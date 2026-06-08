#pragma once

#include <string>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

// Represents a running CGI process state
struct CgiProcess {
    pid_t pid;              // Process ID
    int stdout_fd;          // Read end of stdout pipe
    int stdin_fd;           // Write end of stdin pipe (for POST)
    std::string output;     // Accumulated output from CGI
    std::string contentType; // Content-Type from CGI headers
    time_t startTime;       // Start time for timeout detection
    bool headersParsed;     // Flag to know if we've read CGI response headers
    int statusCode;         // HTTP status code from CGI (if provided)
    
    CgiProcess() : pid(0), stdout_fd(-1), stdin_fd(-1), 
                   startTime(0), headersParsed(false), statusCode(200) {}
};

class CgiHandler {
public:
    CgiHandler();
    ~CgiHandler();

    // Stores process info internally
    pid_t executeCGI(const HttpRequest& request, const std::string& script_path);
    
    // Check if a CGI process is running
    bool isCgiRunning(pid_t pid) const;
    
    // Get accumulated output from a running CGI process
    std::string getCgiOutput(pid_t pid) const;
    
    // Try to read more data from a running CGI process (non-blocking)
    // Returns true if process is still running
    bool readCgiOutput(pid_t pid, int fd, size_t maxReadSize = 4096);
    
    // Check if CGI has finished and collect final result
    // Returns true if process finished, false if still running
    bool finalizeCgi(pid_t pid, HttpResponse& response);
    
    // Clean up a finished CGI process
    void cleanupCgi(pid_t pid);
    
    // Get all running CGI file descriptors for poll monitoring
    std::vector<int> getAllCgiFds() const;
    
    // Get PID for a given output fd
    pid_t getPidByOutputFd(int fd) const;
    
    // Terminate a CGI process (timeout or client disconnect)
    void terminateCgi(pid_t pid);
    
    // Check and reap zombie processes
    void checkZombies();
    
private:
    std::map<pid_t, CgiProcess> _cgiProcesses;  // Track all running CGI processes
    
    // Helper: Build environment variables for CGI
    char** buildCgiEnv(const HttpRequest& request, const std::string& script_path);
    
    // Helper: Free environment array
    void freeCgiEnv(char** env);
    
    // Helper: Set file descriptor to non-blocking
    void setNonBlocking(int fd);
    
    // Constants
    static const int CGI_TIMEOUT = 5;        // 5 seconds timeout for CGI scripts
    static const size_t CGI_MAX_OUTPUT = 1024 * 1024; // 1MB max output per CGI
};
