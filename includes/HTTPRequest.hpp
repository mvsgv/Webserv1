#pragma once 

#include <sstream>
#include <map>
#include <string>

class HTTPRequest{
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        std::string _body;
        std::map<std::string, std::string> _headers;

        void    parseRequest(const std::string &line);
        void    parseHeader(std::istringstream &stream);

    public:
        HTTPRequest(const std::string &raw_request);
        ~HTTPRequest();

        std::string getMethod()const;
        std::string getUri()const;
        std::string getVesion()const;
        std::string getBody()const;
        std::string getHeader(const std::string &key)const;
};