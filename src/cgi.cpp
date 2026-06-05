#include "../includes/cgi.hpp"


CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

std::string CgiHandler::executeCGI(const std::string& script_path) {
    int pipe_fd[2];
    
    if (pipe(pipe_fd) == -1) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    }

    pid_t pid = fork();
    if (pid == -1) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    }

    if (pid == 0) {
        // --- PROCESSUS ENFANT ---
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        
        char* argv[] = {
            const_cast<char*>("/usr/bin/python3"),
            const_cast<char*>(script_path.c_str()),
            NULL
        };
        
        char* envp[] = {
            const_cast<char*>("REQUEST_METHOD=GET"),
            const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"),
            NULL
        };

        execve(argv[0], argv, envp);
        exit(1); 
    } 
    else {
        // --- PROCESSUS PARENT ---
        close(pipe_fd[1]); 
        
        int status;
        waitpid(pid, &status, 0);

        char buffer[4096];
        std::string cgi_result = "";
        int bytes_read;
        
        while ((bytes_read = read(pipe_fd[0], buffer, 4096)) > 0) {
            cgi_result.append(buffer, bytes_read);
        }
        close(pipe_fd[0]);

        // Construction de la réponse finale
        std::string final_response = "HTTP/1.1 200 OK\r\n" + cgi_result;
        return final_response;
    }
}