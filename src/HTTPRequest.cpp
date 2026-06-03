#include "../includes/HTTPRequest.hpp"

HTTPRequest::HTTPRequest(const std::string &raw_request){
    std::istringstream stream(raw_request);
    std::string line;

    //Reading line request -> ex GET /index.html ....
    if (std::getline(stream, line) && !line.empty()){
        if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        parseRequest(line);
    }
    //Lire les headers
    parseHeader(stream);

    //Lecture du body -> apres \r\n\r\n
    size_t body = raw_request.find("\r\n\r\n");
    if (body != std::string::npos){
        _body = raw_request.substr(body + 4);
    }
}

HTTPRequest::~HTTPRequest(){}

void   HTTPRequest::parseRequest(const std::string &line){
    std::istringstream linestr(line);
    linestr >> _method >> _uri >> _version;
}

void   HTTPRequest::parseHeader(std::istringstream &stream){
    std::string line;
    while(getline(stream, line) && line != "\r"){
        size_t separator = line.find(":");
        if (separator != std::string::npos){
            std::string head = line.substr(0, separator);
            std::string value = line.substr(separator + 1);
            if (!value.empty() && value[0] == ' ')
            value.erase(0, 1);
            _headers[head] = value;
        }
    }
}


/*Dans une std::map, chaque élément est une paire :
std::pair<const std::string, std::string>
avec :
first  -> clé
second -> valeur
Exemple : ("Host", "localhost:8080")*/
std::string HTTPRequest::getMethod()const{ return _method; }
std::string HTTPRequest::getUri()const{ return _uri; }
std::string HTTPRequest::getVesion()const{ return _version; } 
std::string HTTPRequest::getBody()const{ return _body; }
std::string HTTPRequest::getHeader(const std::string &key)const{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
        return it->second;
    return "";
}