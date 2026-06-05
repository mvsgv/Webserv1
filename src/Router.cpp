/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mariamevissargova <mariamevissargova@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/31 14:08:18 by stephen           #+#    #+#             */
/*   Updated: 2026/06/05 16:22:18 by mariameviss      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/cgi.hpp"

HttpResponse Router::route(const HttpRequest& request)	//new
{
	if (request.getMethod() == "GET")
		return handleGet(request);

	if (request.getMethod() == "POST")
		return handlePost(request);

	if (request.getMethod() == "DELETE") //how can we delete file without unlink or delete ?
		return handleDelete(request);

	return errorResponse(400);
}

HttpResponse handleGet(const HttpRequest& request)	//new
{
	HttpResponse response;

	std::string path;

	if(request.getUri() == "/")
		path = "www/index.html";	//default page
	else
		path = "www" + request.getUri();		//will need to update bc uri not always = path
	if (path.find(".py") != std::string::npos) {
        CgiHandler cgi;
        response.body = cgi.executeCGI(path);
        response.statusCode = 200;
        // Si votre script Python renvoie déjà "Content-type: text/html\n\n", 
        // ca s'ajoutera au corps . classique en CGI basique.
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

HttpResponse handlePost(const HttpRequest& request){	//new
	HttpResponse response;

	std::string path;
	if(request.getUri() == "/")
		path = "www/index.html";	//default page
	else
		path = "www" + request.getUri();
	if (path.find(".py") != std::string::npos) {
        CgiHandler cgi;
        response.body = cgi.executeCGI(path);
        response.statusCode = 200;
        return response;
	}
	int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);	//0644 here is in octal and means rw-r--r-- (owner : 6 = 4 + 2 = r&w | group : 4 = r | others : 4 = r)
	if(fd < 0)
		return errorResponse(404);
	write(fd, request.getBody().c_str(), request.getBody().size()); //might need to check write
	close(fd);

	response.statusCode = 201;
	response.headers["Content-Type"] = "text/html";
	response.body = "<h1>File Created</h1>";

	return response;
}

HttpResponse handleDelete(const HttpRequest& request)	//new
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
	{
		response.statusCode = 500;
		response.headers["Content-Type"] = "text/html";
		response.body = "<h1>500 Internal Server Error</h1>";
		return response;
	}
	response.statusCode = 200;
	response.headers["Content-Type"] = "text/html";
	response.body = "<h1>File Deleted</h1>";

	return response;
}

HttpResponse errorResponse(int status){	//new
	HttpResponse response;

	response.statusCode = status;
	response.headers["Content-Type"] = "text/html";
	response.body = "<h1>404 Not Found</h1>";
	return response;
}