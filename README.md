*This project has been created as part of the 42 curriculum by **mavissar** and **scesar**.*

# Webserv

## Description

**Webserv** is a custom-built HTTP/1.1 server written from scratch in **C++98**. The goal of this project is to dive deep into network programming, socket management, and the HTTP protocol by recreating the core functionalities of a web server such as NGINX.

This server is entirely non-blocking and relies on a single I/O multiplexing mechanism (`poll()`, `select()`, or equivalent) to handle multiple client connections efficiently and securely. It parses a configuration file, manages multiple virtual servers on different ports or hostnames, serves static files, and executes dynamic content through CGI.

---

## Features

* **I/O Multiplexing:** Handles multiple simultaneous connections without blocking.
* **HTTP Methods:** Supports `GET`, `POST`, and `DELETE`.
* **Configuration Parsing:** Reads and validates an NGINX-style configuration file.
* **CGI Execution:** Executes dynamic scripts (Python, PHP, etc.) with proper environment variables and timeout handling.
* **File Uploads:** Supports file uploads through `POST` requests.
* **File Deletion:** Supports resource deletion through `DELETE` requests.
* **Error Handling:** Robust handling of malformed requests and unexpected situations.
* **Memory Safety:** Designed to avoid memory leaks and crashes.

---

# Architecture

## HTTP

The **Hypertext Transfer Protocol (HTTP)** is an application-layer protocol used for communication between clients and servers.

When browsing a website, the browser acts as the **client**, sending requests to a **server**, which processes them and returns responses.

HTTP communication is based on a request-response model:

1. The client sends an HTTP request.
2. The server processes the request.
3. The server returns an HTTP response.

![HTTP Message Anatomy](https://mdn.github.io/shared-assets/images/diagrams/http/messages/http-message-anatomy.svg)

### Supported HTTP Methods

#### GET

Requests a resource from the server without modifying it.

```http
GET /index.html HTTP/1.1
```

#### POST

Sends data to the server, usually to create or update a resource.

```http
POST /upload HTTP/1.1
```

#### DELETE

Requests the deletion of a resource.

```http
DELETE /file.txt HTTP/1.1
```

### Common HTTP Status Codes

#### 2xx — Success

| Code | Meaning    |
| ---- | ---------- |
| 200  | OK         |
| 201  | Created    |
| 202  | Accepted   |
| 204  | No Content |

#### 3xx — Redirection

| Code | Meaning           |
| ---- | ----------------- |
| 301  | Moved Permanently |
| 302  | Found             |
| 304  | Not Modified      |

#### 4xx — Client Errors

| Code | Meaning            |
| ---- | ------------------ |
| 400  | Bad Request        |
| 401  | Unauthorized       |
| 403  | Forbidden          |
| 404  | Not Found          |
| 405  | Method Not Allowed |

#### 5xx — Server Errors

| Code | Meaning               |
| ---- | --------------------- |
| 500  | Internal Server Error |
| 501  | Not Implemented       |
| 503  | Service Unavailable   |

---

## Server & Sockets

HTTP communication generally uses **TCP sockets**.

A typical HTTP session follows these steps:

1. The server creates a socket and starts listening.
2. The client establishes a TCP connection.
3. The client sends an HTTP request.
4. The server processes the request.
5. The server sends an HTTP response.
6. The connection is closed or reused.

![TCP Socket Communication](https://camo.githubusercontent.com/94bb4c4642cceac8ca1a835c57e54c2a2bacd902db5f19eaf01b98d384e05e47/68747470733a2f2f646c2e64726f70626f782e636f6d2f73636c2f66692f766c66736431787073657630337a3271396a30676c2f736f636b65742e706e673f726c6b65793d697565656a717a6b7230737471613271326d32377978356a7726646c3d30)

### Server Workflow

```text
Create Socket
      ↓
Bind Port
      ↓
Listen
      ↓
Accept Connections
      ↓
Receive Requests
      ↓
Process Requests
      ↓
Send Responses
      ↓
Close Connection
```

---

## CGI

**CGI (Common Gateway Interface)** allows a web server to execute external programs and return their output as HTTP responses.

CGI is independent of:

* Programming language
* Operating system
* Web server implementation

For this project, CGI execution is configured through dedicated routes such as:

```text
/cgi-bin/script.py
/cgi-bin/script.php
```

When such a resource is requested:

1. The server detects the CGI route.
2. The script is executed in a child process.
3. Environment variables are prepared.
4. The script output is captured.
5. The generated response is sent back to the client.

---

# Installation

## Prerequisites

* Linux or macOS
* `make`
* C++ compiler (`c++` or `g++`)

## Compilation

```bash
git clone <repository_url>
cd webserv
make
```

---

# Usage

Run the server with a configuration file:

```bash
./webserv conf/default.conf
```

---

# Testing

### Browser

Open:

```text
http://localhost:8080
```

### GET Request

```bash
curl -i http://localhost:8080/
```

### Error Page Test

```bash
curl -i http://localhost:8080/non_existent_page
```

### POST Upload Test

```bash
curl -i -X POST \
--data-binary "@test.txt" \
http://localhost:8080/upload/test.txt
```

### DELETE Test

```bash
curl -i -X DELETE \
http://localhost:8080/upload/test.txt
```

---

# References

## Networking & Sockets

* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/#client-server-background)
* [TCP Sockets](https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/TCPSockets.html)
* [Blocking vs Non-Blocking Sockets](http://dwise1.net/pgm/sockets/blocking.html)
* [Non-Blocking I/O with select()](https://www.ibm.com/docs/en/i/7.2.0?topic=designs-example-nonblocking-io-select)

## HTTP

* [HTTP Requests and Responses Explained](https://codefinity.com/blog/HTTP-Requests-and-Responses-Explained)
* [C++ Web Programming](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm)

## Web Server Development

* [Building a Simple Server with C++](https://ncona.com/2019/04/building-a-simple-server-with-cpp/)

## CGI

* [CGI Programming](https://www.tutorialspoint.com/python/python_cgi_programming.htm)

---

## Authors

* **mavissar**
* **scesar**
