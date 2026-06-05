/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mariamevissargova <mariamevissargova@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:39:09 by stephen           #+#    #+#             */
/*   Updated: 2026/06/05 16:17:56 by mariameviss      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequest.hpp"

bool HttpRequest::parse(const std::string& rawRequest){
	//first line
	size_t	eol = 0;
	std::string	line = gnl_req(rawRequest, eol);

	std::stringstream	elems(line);
	elems >> this->method >> this->uri >> this->version;
	if(this->check_empty())
		return false;
	//headers
	while (eol != std::string::npos)
	{
		size_t sep;
		line = gnl_req(rawRequest, eol);	//new
		if(line == "\r\n")
			break;
		sep = line.find(':');
		if (sep == std::string::npos)
			return false;
		//put some exceptions to look good maybe ?
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);
		if (!value.empty() && value[0] == ' ')
			value.erase(0, 1);
		this->headers[key] = value;
	}
	this->body = rawRequest.substr(eol, rawRequest.size() - eol);
	//put some exceptions to look good maybe ?
	//deeper check needed like valid method (only 3 here),...
	return true;
}

bool HttpRequest::check_empty(){
	if(this->method.empty() || this->uri.empty() || this->version.empty())
		return true;
	return false;
}

std::string gnl_req(const std::string &rawRequest, size_t &eol){	//new

	size_t n_eol = rawRequest.find("\r\n", eol);
	if (n_eol == std::string::npos)
	{
		eol = std::string::npos;
		return "";
	}
	else if (n_eol == eol)
	{
		eol = n_eol + 2;
		return "\r\n";
	}
	std::string line = rawRequest.substr(eol, n_eol - eol);
	eol = n_eol + 2;

	return line;
}

// bool HttpRequest::parse(const std::string& rawRequest){
// 	//first line
// 	size_t	eol = 0;
// 	std::string	line = gnl_req(rawRequest, eol);

// 	std::stringstream	elems(line);
// 	elems >> this->method >> this->uri >> this->version;

// 	//headers
// 	eol +=2;
// 	size_t	n_eol = eol;
// 	while (n_eol != std::string::npos)
// 	{
// 		size_t sep;
// 		n_eol = rawRequest.find("\r\n", eol);
// 		line = rawRequest.substr(eol, n_eol - eol);
// 		if(line.empty())
// 			break;
// 		sep = line.find(':');
// 		if (sep == std::string::npos)
// 			return false;
// 		//put some exceptions to look good maybe ?
// 		std::string key = line.substr(0, sep);
// 		// std::string value = line.substr(sep + 2, line.size());
// 		std::string value = line.substr(sep + 1); //see if not better than manually trimming spaces as above
// 		if (!value.empty() && value[0] == ' ')
// 			value.erase(0, 1);
// 		this->headers[key] = value;
// 		n_eol +=2;
// 		eol = n_eol;
// 	}
// 	if(!line.empty() && (n_eol != std::string::npos))
// 	{

// 	}
// 	if(this->check_empty())
// 		return false;
// 	//put some exceptions to look good maybe ?
// 	//deeper check needed like valid method (only 3 here),...
// 	return true;
// }

const std::string& HttpRequest::getMethod()const{ return method; }
const std::string& HttpRequest::getUri()const{ return uri; }
const std::string& HttpRequest::getVersion()const{ return version; } 
const std::string& HttpRequest::getBody()const{ return body; }
std::string HttpRequest::getHeader(const std::string &key)const{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return "";
}