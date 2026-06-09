#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse() : statusCode(200){}

HttpResponse::~HttpResponse(){}

std::string HttpResponse::buildResponse()
{
    std::stringstream res;

    headers["Content-Length"] = std::to_string(body.size());

    res << "HTTP/1.1 " << statusCode << " "
        << status_message(statusCode)
        << "\r\n";

    for (std::map<std::string, std::string>::iterator it = headers.begin();
         it != headers.end();
         ++it)
    {
        res << it->first << ": " << it->second << "\r\n";
    }

    res << "\r\n";
    res << body;

    return res.str();
}


std::string status_message(int code){
	if (code == 200)
		return "OK";
	else if (code == 201)
		return "Created";
	else if (code == 400)
		return "Bad Request";
	else if (code == 403)
		return "Forbidden";
	else if (code == 404)
		return "Not Found";
	else if (code == 500)
		return "Internal Server Error";
    else if (code == 502)
        return "Bad Gateway";
    else if (code == 504)
        return "Gateway Timeout";
	return "Unknown";
}