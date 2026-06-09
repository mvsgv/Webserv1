#include "../includes/Router.hpp"
#include "../includes/CgiHandler.hpp"
#include <cstring>
HttpResponse Router::route(const HttpRequest& request)	//new
{
	if (request.getMethod() == "GET")
		return handleGet(request);

	if (request.getMethod() == "POST")
		return handlePost(request);

	if (request.getMethod() == "DELETE") 
		return handleDelete(request);

	return errorResponse(400);
}

HttpResponse handleGet(const HttpRequest& request)
{
    HttpResponse response;

    std::string path;

    if (request.getUri() == "/")
        path = "www/index.html";
    else
        path = "www" + request.getUri();

    //  CGI 
    if (path.find(".py") != std::string::npos)
    {
        CgiHandler cgi;

        pid_t pid = cgi.executeCGI(request, path);
        if (pid < 0)
            return errorResponse(500);

        while (cgi.isCgiRunning(pid))
        {
            bool isStillRunning = cgi.readCgiOutput(pid, cgi.getAllCgiFds()[0], 4096);
            if (!isStillRunning)
                break;
        }

        cgi.finalizeCgi(pid, response);

        return response;
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return errorResponse(404);

    response.statusCode = 200;
    response.headers["Content-Type"] = "text/html";

    char buffer[1024];
    ssize_t bytes;

    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
        response.body.append(buffer, bytes);

    close(fd);

    return response;
}
HttpResponse handlePost(const HttpRequest& request)
{
    HttpResponse response;

    std::string path;

    if (request.getUri() == "/")
        path = "www/index.html";
    else
        path = "www" + request.getUri();

    if (path.find(".py") != std::string::npos)
    {
        CgiHandler cgi;

        pid_t pid = cgi.executeCGI(request, path);
        if (pid < 0)
            return errorResponse(500);

        while (cgi.isCgiRunning(pid))
        {
            cgi.readCgiOutput(pid,
                cgi.getAllCgiFds()[0],
                4096);
        }

        cgi.finalizeCgi(pid, response);
        return response;
    }

    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return errorResponse(404);

    write(fd, request.getBody().c_str(), request.getBody().size());
    close(fd);

    response.statusCode = 201;
    response.headers["Content-Type"] = "text/html";
    response.body = "<h1>File Created</h1>";

    return response;
}
HttpResponse handleDelete(const HttpRequest& request)
{
    HttpResponse response;

    std::string path;

    if (request.getUri() == "/")
        path = "www/index.html";
    else
        path = "www" + request.getUri();

    if (access(path.c_str(), F_OK) != 0)
        return errorResponse(404);

    if (std::remove(path.c_str()) != 0)
        return errorResponse(500);

    response.statusCode = 200;
    response.headers["Content-Type"] = "text/html";
    response.body = "<h1>File Deleted</h1>";

    return response;
}
HttpResponse errorResponse(int status)
{
    HttpResponse response;

    response.statusCode = status;
    response.headers["Content-Type"] = "text/html";

    if (status == 404)
        response.body = "<h1>404 Not Found</h1>";
    else if (status == 400)
        response.body = "<h1>400 Bad Request</h1>";
    else if (status == 500)
        response.body = "<h1>500 Internal Server Error</h1>";
    else
        response.body = "<h1>Error</h1>";

    return response;
}
