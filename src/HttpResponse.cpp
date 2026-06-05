/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mariamevissargova <mariamevissargova@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 15:01:22 by stephen           #+#    #+#             */
/*   Updated: 2026/06/05 16:18:17 by mariameviss      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpResponse.hpp"

std::string HttpResponse::buildResponse(){
	std::stringstream res;

	//status line
	res << "HTTP/1.1 " << statusCode << " " << status_message(statusCode)
		<< "\r\n";

	//headers
	headers["Content-Length"] = itostr((body.length()));
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		res << it->first << ": " << it->second
		<< "\r\n";
	}
	res << "\r\n";
	res << body;
	return res.str();
}

std::string itostr(size_t n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

//might be better to do some try catch here but good for now
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
	return "Unknown";
}