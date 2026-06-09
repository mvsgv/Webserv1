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

		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);
		if (!value.empty() && value[0] == ' ')
			value.erase(0, 1);
		this->headers[key] = value;
	}
	this->body = rawRequest.substr(eol, rawRequest.size() - eol);
	return true;
}

bool HttpRequest::check_empty(){
	if(this->method.empty() || this->uri.empty() || this->version.empty())
		return true;
	return false;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const {
    return headers;
}

std::string HttpRequest::getQueryString() const {
    size_t pos = uri.find('?');
    if (pos != std::string::npos) {
        return uri.substr(pos + 1);
    }
    return "";
}
std::string gnl_req(const std::string &rawRequest, size_t &eol){

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