#include "../includes/cgi.hpp"
#include <fcntl.h>
#include <cstring>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <signal.h>
#include <errno.h>

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {
    // Clean up any remaining CGI processes
    for (std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.begin(); 
         it != _cgiProcesses.end(); ++it) {
        terminateCgi(it->first);
    }
}

// Helper: Set file descriptor to non-blocking
void CgiHandler::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl F_GETFL failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed");
}

// Helper: Build CGI environment variables
char** CgiHandler::buildCgiEnv(const HttpRequest& request, const std::string& script_path) {
    std::vector<std::string> env_vars;
    
    // Standard CGI variables
    env_vars.push_back("REQUEST_METHOD=" + request.getMethod());
    env_vars.push_back("REQUEST_URI=" + request.getUri());
    env_vars.push_back("SCRIPT_NAME=" + script_path);
    env_vars.push_back("SCRIPT_FILENAME=" + script_path);
    env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_vars.push_back("SERVER_SOFTWARE=Webserv/1.0");
    env_vars.push_back("SERVER_NAME=localhost");
    env_vars.push_back("SERVER_PORT=8080");
    
    // Content headers
    std::string content_length = request.getHeader("Content-Length");
    if (!content_length.empty()) {
        env_vars.push_back("CONTENT_LENGTH=" + content_length);
    }
    
    std::string content_type = request.getHeader("Content-Type");
    if (!content_type.empty()) {
        env_vars.push_back("CONTENT_TYPE=" + content_type);
    }
    
    env_vars.push_back("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
    env_vars.push_back("");  // Null terminator
    
    // Allocate and populate the environment array
    char** env = new char*[env_vars.size()];
    for (size_t i = 0; i < env_vars.size(); ++i) {
        env[i] = new char[env_vars[i].length() + 1];
        std::strcpy(env[i], env_vars[i].c_str());
    }
    
    return env;
}

void CgiHandler::freeCgiEnv(char** env) {
    if (!env) return;
    for (int i = 0; env[i] != NULL; ++i) {
        delete[] env[i];
    }
    delete[] env;
}

// Execute CGI script - non-blocking
pid_t CgiHandler::executeCGI(const HttpRequest& request, const std::string& script_path) {
    int stdout_pipe[2];
    int stdin_pipe[2];
    
    // Create pipes for stdout (CGI output) and stdin (request body)
    if (pipe(stdout_pipe) == -1 || pipe(stdin_pipe) == -1) {
        std::cerr << "Error creating pipes for CGI" << std::endl;
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        std::cerr << "Error forking CGI process" << std::endl;
        return -1;
    }
    
    if (pid == 0) {
        // --- CHILD PROCESS ---
        // Close read end of stdout pipe and write end of stdin pipe
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        
        // Redirect stdout to pipe
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[1]);
        
        // Redirect stdin from pipe
        dup2(stdin_pipe[0], STDIN_FILENO);
        close(stdin_pipe[0]);
        
        // Build arguments
        char* argv[] = {
            const_cast<char*>("/usr/bin/python3"),
            const_cast<char*>(script_path.c_str()),
            NULL
        };
        
        // Build environment
        char** envp = buildCgiEnv(request, script_path);
        
        // Execute the CGI script
        execve(argv[0], argv, envp);
        
        // If execve returns, there was an error
        freeCgiEnv(envp);
        std::cerr << "execve failed for CGI: " << strerror(errno) << std::endl;
        exit(127);
    } 
    else {
        // --- PARENT PROCESS ---
        // Close write end of stdout pipe and read end of stdin pipe
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        
        // Make the stdout pipe non-blocking
        setNonBlocking(stdout_pipe[0]);
        
        // Write POST body to stdin if present
        std::string body = request.getBody();
        if (!body.empty()) {
            ssize_t written = write(stdin_pipe[1], body.c_str(), body.length());
            if (written < 0) {
                std::cerr << "Error writing to CGI stdin: " << strerror(errno) << std::endl;
            }
        }
        close(stdin_pipe[1]);  // Close write end to signal EOF to CGI
        
        // Store CGI process info
        CgiProcess proc;
        proc.pid = pid;
        proc.stdout_fd = stdout_pipe[0];
        proc.stdin_fd = -1;  // Already closed
        proc.startTime = std::time(NULL);
        _cgiProcesses[pid] = proc;
        
        return pid;
    }
}

bool CgiHandler::isCgiRunning(pid_t pid) const {
    return _cgiProcesses.find(pid) != _cgiProcesses.end();
}

std::string CgiHandler::getCgiOutput(pid_t pid) const {
    std::map<pid_t, CgiProcess>::const_iterator it = _cgiProcesses.find(pid);
    if (it != _cgiProcesses.end()) {
        return it->second.output;
    }
    return "";
}

bool CgiHandler::readCgiOutput(pid_t pid, int fd, size_t maxReadSize) {
    std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.find(pid);
    if (it == _cgiProcesses.end())
        return false;
    
    CgiProcess& proc = it->second;
    
    // Check for timeout
    time_t now = std::time(NULL);
    if (now - proc.startTime > CGI_TIMEOUT) {
        std::cerr << "CGI process timeout" << std::endl;
        terminateCgi(pid);
        return false;
    }
    
    // Check size limit
    if (proc.output.length() >= CGI_MAX_OUTPUT) {
        std::cerr << "CGI output exceeded maximum size" << std::endl;
        terminateCgi(pid);
        return false;
    }
    
    // Try to read data
    char buffer[4096];
    size_t read_size = (maxReadSize > sizeof(buffer)) ? sizeof(buffer) : maxReadSize;
    ssize_t bytes_read = read(fd, buffer, read_size);
    
    if (bytes_read > 0) {
        proc.output.append(buffer, bytes_read);
    } else if (bytes_read == 0) {
        // EOF: process finished
        return false;
    }
    // bytes_read < 0: EAGAIN/EWOULDBLOCK - no data available (non-blocking)
    
    return true;  // Process still running
}

bool CgiHandler::finalizeCgi(pid_t pid, HttpResponse& response) {
    std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.find(pid);
    if (it == _cgiProcesses.end())
        return false;
    
    CgiProcess& proc = it->second;
    
    // Check if process has finished
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    
    if (result == 0) {
        // Process still running
        return false;
    }
    
    if (result == -1) {
        std::cerr << "waitpid error" << std::endl;
        cleanupCgi(pid);
        return true;  // Consider it done
    }
    
    // Process finished - read any remaining data
    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(proc.stdout_fd, buffer, sizeof(buffer))) > 0) {
        proc.output.append(buffer, bytes_read);
    }
    
    // Parse CGI output
    std::string cgi_output = proc.output;
    std::string headers;
    std::string body;
    
    // Split headers and body by blank line
    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = cgi_output.find("\n\n");
        if (header_end != std::string::npos) {
            headers = cgi_output.substr(0, header_end);
            body = cgi_output.substr(header_end + 2);
        } else {
            body = cgi_output;
        }
    } else {
        headers = cgi_output.substr(0, header_end);
        body = cgi_output.substr(header_end + 4);
    }
    
    // Parse headers
    response.statusCode = proc.statusCode;
    response.headers["Content-Type"] = "text/html";
    
    if (!headers.empty()) {
        std::istringstream header_stream(headers);
        std::string header_line;
        while (std::getline(header_stream, header_line)) {
            if (header_line.empty()) break;
            
            // Remove trailing \r if present
            if (!header_line.empty() && header_line[header_line.length() - 1] == '\r') {
                header_line = header_line.substr(0, header_line.length() - 1);
            }
            
            size_t colon_pos = header_line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = header_line.substr(0, colon_pos);
                std::string value = header_line.substr(colon_pos + 1);
                
                // Trim leading/trailing spaces
                size_t start = value.find_first_not_of(" \t\r\n");
                size_t end = value.find_last_not_of(" \t\r\n");
                if (start != std::string::npos) {
                    value = value.substr(start, end - start + 1);
                }
                
                response.headers[key] = value;
                
                if (key == "Content-Type") {
                    proc.contentType = value;
                }
            }
        }
    }
    
    response.body = body;
    cleanupCgi(pid);
    return true;
}

void CgiHandler::cleanupCgi(pid_t pid) {
    std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.find(pid);
    if (it != _cgiProcesses.end()) {
        if (it->second.stdout_fd >= 0) {
            close(it->second.stdout_fd);
        }
        if (it->second.stdin_fd >= 0) {
            close(it->second.stdin_fd);
        }
        _cgiProcesses.erase(it);
    }
}

std::vector<int> CgiHandler::getAllCgiFds() const {
    std::vector<int> fds;
    for (std::map<pid_t, CgiProcess>::const_iterator it = _cgiProcesses.begin();
         it != _cgiProcesses.end(); ++it) {
        if (it->second.stdout_fd >= 0) {
            fds.push_back(it->second.stdout_fd);
        }
    }
    return fds;
}

pid_t CgiHandler::getPidByOutputFd(int fd) const {
    for (std::map<pid_t, CgiProcess>::const_iterator it = _cgiProcesses.begin();
         it != _cgiProcesses.end(); ++it) {
        if (it->second.stdout_fd == fd) {
            return it->first;
        }
    }
    return -1;
}

void CgiHandler::terminateCgi(pid_t pid) {
    std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.find(pid);
    if (it != _cgiProcesses.end()) {
        kill(pid, SIGTERM);
        usleep(100000);  // Give it 100ms to die gracefully
        kill(pid, SIGKILL);  // Force kill if needed
        
        int status;
        waitpid(pid, &status, WNOHANG);
        cleanupCgi(pid);
    }
}

void CgiHandler::checkZombies() {
    int status;
    pid_t pid;
    
    // Reap any zombie CGI processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (_cgiProcesses.find(pid) != _cgiProcesses.end()) {
            cleanupCgi(pid);
        }
    }
}
