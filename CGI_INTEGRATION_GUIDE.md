# CGI Integration Guide for Webserv

## Overview

The new CGI handler (`CgiHandler` class) is designed to execute external scripts (PHP, Python, etc.) in a **non-blocking, event-driven manner** that integrates seamlessly with your `poll()`-based server architecture.

## Key Design Principles

1. **Non-Blocking**: CGI processes run asynchronously; execution doesn't block the main event loop
2. **Resource Safe**: Automatic timeout (5s), output size limits (1MB), cleanup of zombie processes
3. **Poll Integration**: CGI output pipes are monitored via poll() just like client sockets
4. **Process Management**: Proper tracking, cleanup, and error handling

---

## Architecture

### CgiProcess Structure

```cpp
struct CgiProcess {
    pid_t pid;              // Child process ID
    int stdout_fd;          // File descriptor for reading CGI output
    int stdin_fd;           // File descriptor for writing request body
    std::string output;     // Accumulated output buffer
    std::string contentType; // Content-Type from CGI headers
    time_t startTime;       // Process start time (for timeout detection)
    bool headersParsed;     // Flag for CGI response header parsing
    int statusCode;         // HTTP status code (default 200)
};
```

### CgiHandler Methods

| Method | Purpose |
|--------|----------|
| `executeCGI()` | Fork and execute a CGI script; returns PID |
| `readCgiOutput()` | Non-blocking read from CGI stdout; returns true if running |
| `finalizeCgi()` | Collect final output and build HttpResponse |
| `getAllCgiFds()` | Get all CGI output FDs for poll() monitoring |
| `getPidByOutputFd()` | Map a file descriptor back to process PID |
| `terminateCgi()` | Kill a CGI process (timeout/cleanup) |
| `checkZombies()` | Reap zombie child processes |
| `cleanupCgi()` | Release resources for a finished process |

---

## Integration Steps

### Step 1: Add CgiHandler to ServerManager

In `includes/ServerManager.hpp`:

```cpp
#pragma once
#include "cgi.hpp"

class ServerManager {
private:
    CgiHandler _cgiHandler;
    std::map<int, pid_t> _cgiByFd;  // Map CGI output fd → PID
    
public:
    // ... existing methods ...
};
```

### Step 2: Extend HttpRequest (Optional but Recommended)

Your `HttpRequest` class needs these methods for CGI:

```cpp
// In includes/HTTPRequest.hpp
class HttpRequest {
private:
    // ... existing members ...
    
public:
    // Add these getters:
    std::map<std::string, std::string> getHeaders() const;
    std::string getQueryString() const;
    
    // ... existing methods ...
};
```

Implementation in `src/HTTPRequest.cpp`:

```cpp
std::map<std::string, std::string> HttpRequest::getHeaders() const {
    return headers;
}

std::string HttpRequest::getQueryString() const {
    // Extract query string from URI (after ?)
    size_t pos = uri.find('?');
    if (pos != std::string::npos) {
        return uri.substr(pos + 1);
    }
    return "";
}
```

### Step 3: Detect CGI Scripts in Router

In `src/Router.cpp`, modify handler functions:

```cpp
// Simplified example - detect .py files
if (path.find(".py") != std::string::npos || 
    path.find(".php") != std::string::npos) {
    // This is a CGI script
    return true;
}
return false;
```

### Step 4: Execute CGI in Request Handler

Modify `ServerManager::run()` to handle CGI:

```cpp
void ServerManager::run() {
    while (true) {
        // Poll with timeout for zombie detection
        int ready = poll(&_pollfds[0], _pollfds.size(), 100);  // 100ms timeout
        
        if (ready > 0) {
            // ... existing poll handling ...
            
            // When processing a client request that needs CGI:
            if (isCgiScript(path)) {
                pid_t cgi_pid = _cgiHandler.executeCGI(request, path);
                if (cgi_pid > 0) {
                    // Get the stdout fd from CgiHandler's internal map
                    std::vector<int> cgi_fds = _cgiHandler.getAllCgiFds();
                    if (!cgi_fds.empty()) {
                        int cgi_fd = cgi_fds.back();  // Most recently added
                        _cgiByFd[cgi_fd] = cgi_pid;
                        
                        // Add CGI output fd to poll array
                        struct pollfd cgi_pfd;
                        cgi_pfd.fd = cgi_fd;
                        cgi_pfd.events = POLLIN;
                        cgi_pfd.revents = 0;
                        _pollfds.push_back(cgi_pfd);
                    }
                }
                continue;  // Don't wait for response; handle async
            }
        }
        
        // Periodically check for zombie processes
        _cgiHandler.checkZombies();
    }
}
```

### Step 5: Monitor CGI Output in Poll Loop

Add to your main poll handling:

```cpp
// After checking client read/write events:

// Check for CGI output availability
for (std::map<int, pid_t>::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ++it) {
    int cgi_fd = it->first;
    pid_t cgi_pid = it->second;
    
    // Find this fd in poll array
    for (size_t i = 0; i < _pollfds.size(); ++i) {
        if (_pollfds[i].fd == cgi_fd && (_pollfds[i].revents & POLLIN)) {
            // Try to read more data from CGI
            bool still_running = _cgiHandler.readCgiOutput(cgi_pid, cgi_fd);
            
            if (!still_running) {
                // CGI finished or error; collect final output
                HttpResponse response;
                if (_cgiHandler.finalizeCgi(cgi_pid, response)) {
                    // Send response to waiting client
                    Client &client = _clients.at(waiting_client_fd);  // Store on start
                    client.setWriteRequest(response.buildResponse());
                    
                    // Switch client to write mode
                    // ... update that client's poll entry ...
                }
                
                // Cleanup: remove CGI fd from poll
                _pollfds.erase(_pollfds.begin() + i);
                _cgiByFd.erase(cgi_fd);
            }
            break;
        }
    }
}
```

### Step 6: Implement Timeout Handling

In your main poll loop (after `poll()` call):

```cpp
// Check CGI timeouts by attempting finalize on all running processes
for (std::map<int, pid_t>::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ) {
    int cgi_fd = it->first;
    pid_t cgi_pid = it->second;
    
    // Check if finished (with timeout detection internal to finalizeCgi)
    HttpResponse temp_response;
    if (_cgiHandler.finalizeCgi(cgi_pid, temp_response)) {
        // Process finished or timed out
        _pollfds.erase(/* find and remove cgi_fd */);
        _cgiByFd.erase(it++);
    } else {
        ++it;
    }
}
```

---

## Testing

### Test 1: Simple CGI Execution

```bash
# Make test script executable
chmod +x www/test.py

# Compile
make

# Run server
./webserv conf/default.conf

# In another terminal
curl http://localhost:8080/test.py
```

Expected output:
```html
<h1>CGI Test - Hello from Webserv</h1>
<h2>Request Information</h2>
<ul>
  <li>METHOD: GET</li>
  <li>URI: /test.py</li>
  <li>SERVER: Webserv/1.0</li>
</ul>
<p>CGI is working!</p>
```

### Test 2: POST with Body

```bash
curl -X POST -d "name=test&value=123" http://localhost:8080/test.py
```

### Test 3: Timeout Handling

Create `www/timeout.py`:

```python
#!/usr/bin/env python3
import time
time.sleep(10)  # Sleep longer than CGI_TIMEOUT (5s)
print("Content-Type: text/html\n")
print("<h1>This should be killed</h1>")
```

Test:
```bash
curl http://localhost:8080/timeout.py
# Should fail or get no response after ~5 seconds
```

### Test 4: Large Output Handling

Create `www/large.py`:

```python
#!/usr/bin/env python3
print("Content-Type: text/html\n")
for i in range(300000):  # Generate ~3MB output (exceeds 1MB limit)
    print(f"<p>Line {i}</p>")
```

Test:
```bash
curl http://localhost:8080/large.py
# Should be killed when output exceeds CGI_MAX_OUTPUT
```

---

## Configuration Constants

Edit in `includes/cgi.hpp` as needed:

```cpp
static const int CGI_TIMEOUT = 5;              // 5 seconds
static const size_t CGI_MAX_OUTPUT = 1024 * 1024;  // 1MB
```

---

## Troubleshooting

### Issue: CGI scripts not executing

**Solution**: Ensure:
- Python3 is installed: `which python3`
- Script has execute permission: `chmod +x www/test.py`
- Script path is correct: `www/test.py` not `/www/test.py`

### Issue: Output not showing

**Solution**:
- CGI must output headers: `print("Content-Type: text/html")`
- Must have blank line after headers: `print("")`
- Check `response.buildResponse()` is called correctly

### Issue: Zombie processes

**Solution**: Call `_cgiHandler.checkZombies()` periodically in poll loop

### Issue: Memory leaks

**Solution**: Ensure all CGI processes call `finalizeCgi()` or `terminateCgi()` to cleanup

---

## Environment Variables Passed to CGI

The handler automatically sets:

- `REQUEST_METHOD` - GET, POST, DELETE, etc.
- `REQUEST_URI` - The requested path
- `SCRIPT_NAME` - Path to CGI script
- `SCRIPT_FILENAME` - Full path to CGI script
- `SERVER_PROTOCOL` - HTTP/1.1
- `SERVER_SOFTWARE` - Webserv/1.0
- `CONTENT_LENGTH` - Request body size (if POST)
- `CONTENT_TYPE` - Request body type (if POST)
- `PATH` - Standard system PATH

---

## Performance Notes

1. **Poll Timeout**: Use a short timeout (100-200ms) to detect finished CGI processes quickly
2. **Zombie Reaping**: Call `checkZombies()` at least once per poll cycle
3. **Output Buffering**: CGI output is accumulated in memory; increase `CGI_MAX_OUTPUT` if needed for large responses
4. **Concurrent CGI**: Multiple CGI scripts can run concurrently; each has its own process

---

## Compliance with School Requirements

✅ Non-blocking I/O: CGI execution doesn't block poll() loop  
✅ Fork usage: Only used for CGI (requirement met)  
✅ Pipe usage: For CGI stdin/stdout communication  
✅ Signal handling: SIGTERM/SIGKILL for timeout management  
✅ Resource cleanup: Proper file descriptor and memory management  
✅ C++98: All code is C++98 compliant  

