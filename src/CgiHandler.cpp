#include "../includes/CgiHandler.hpp"
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

char** CgiHandler::buildCgiEnv(const HttpRequest& request, const std::string& script_path)
{
    std::vector<std::string> env_vars;

    env_vars.push_back("REQUEST_METHOD=" + request.getMethod());
    env_vars.push_back("REQUEST_URI=" + request.getUri());
    env_vars.push_back("SCRIPT_NAME=" + script_path);
    env_vars.push_back("SCRIPT_FILENAME=" + script_path);
    env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_vars.push_back("SERVER_SOFTWARE=Webserv/1.0");
    env_vars.push_back("SERVER_NAME=localhost");
    env_vars.push_back("SERVER_PORT=8080");

    std::string content_length = request.getHeader("Content-Length");
    if (!content_length.empty())
        env_vars.push_back("CONTENT_LENGTH=" + content_length);

    std::string content_type = request.getHeader("Content-Type");
    if (!content_type.empty())
        env_vars.push_back("CONTENT_TYPE=" + content_type);

    env_vars.push_back("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");

    // +1 for NULL terminator
    char** env = new char*[env_vars.size() + 1];

    for (size_t i = 0; i < env_vars.size(); i++)
    {
        env[i] = new char[env_vars[i].size() + 1];
        std::strcpy(env[i], env_vars[i].c_str());
    }
    env[env_vars.size()] = NULL;
    return env;
}

void CgiHandler::freeCgiEnv(char** env)
{
    if (!env)
        return;

    for (size_t i = 0; env[i] != NULL; i++)
        delete[] env[i];

    delete[] env;
}

pid_t CgiHandler::executeCGI(const HttpRequest& request, const std::string& script_path) {
    int stdout_pipe[2];
    int stdin_pipe[2];
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
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[1]);
        
        dup2(stdin_pipe[0], STDIN_FILENO);
        close(stdin_pipe[0]);
        
        char* argv[] = {
            const_cast<char*>("/usr/bin/python3"),
            const_cast<char*>(script_path.c_str()),
            NULL
        };
        
        char** envp = buildCgiEnv(request, script_path);
        
        execve(argv[0], argv, envp);

        freeCgiEnv(envp);
        std::cerr << "execve failed: " << strerror(errno) << std::endl;
        _exit(127); 
    } 
    else {
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        
        setNonBlocking(stdout_pipe[0]);
        
        std::string body = request.getBody();
        if (!body.empty()) {
            ssize_t written = write(stdin_pipe[1], body.c_str(), body.length());
            if (written < 0) {
                std::cerr << "Error writing to CGI stdin: " << strerror(errno) << std::endl;
            }
        }
        close(stdin_pipe[1]); 
        
        // Store CGI process info
        CgiProcess proc;
        proc.pid = pid;
        proc.stdout_fd = stdout_pipe[0];
        proc.stdin_fd = -1;
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
        return false;
    }
    
    return true;  
}
bool CgiHandler::finalizeCgi(pid_t pid, HttpResponse& response)
{
    std::map<pid_t, CgiProcess>::iterator it = _cgiProcesses.find(pid);
    if (it == _cgiProcesses.end())
        return false;

    CgiProcess& proc = it->second;

    int status;
    pid_t result = waitpid(pid, &status, 0);

    if (result == -1)
    {
        cleanupCgi(pid);
        return true;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(proc.stdout_fd, buffer, sizeof(buffer))) > 0)
        proc.output.append(buffer, bytes_read);

    std::string headers;
    std::string body;

    size_t header_end = proc.output.find("\r\n\r\n");
    if (header_end != std::string::npos)
    {
        headers = proc.output.substr(0, header_end);
        body = proc.output.substr(header_end + 4);
    }
    else
    {
        header_end = proc.output.find("\n\n");
        if (header_end != std::string::npos)
        {
            headers = proc.output.substr(0, header_end);
            body = proc.output.substr(header_end + 2);
        }
        else
        {
            body = proc.output;
        }
    }

    proc.statusCode = 200;

    if (!headers.empty())
    {
        std::istringstream stream(headers);
        std::string line;

        while (std::getline(stream, line))
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            size_t pos = line.find(':');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            value.erase(0, value.find_first_not_of(" \t"));

            if (key == "Status")
            {
                proc.statusCode = std::atoi(value.c_str());
            }
            else if (key == "Content-Type")
            {
                proc.contentType = value;
            }
        }
    }

    if (proc.statusCode < 100 || proc.statusCode > 599)
        response.statusCode = 500;
    else
        response.statusCode = proc.statusCode;

    response.headers["Content-Type"] = proc.contentType;
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
        usleep(100000); 
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
