#include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig(): _port(0), _server(""), _maxBodySize(0) {}
ServerConfig::ServerConfig(const ServerConfig &copy): _port(copy._port),
_server(copy._server), _errors(copy._errors), _locations(copy._locations),
_maxBodySize(copy._maxBodySize) {
}
ServerConfig &ServerConfig::operator=(const ServerConfig &target){
    if (this != &target){
        _port = target._port;
        _server = target._server;
        _errors = target._errors;
        _locations = target._locations;
        _maxBodySize = target._maxBodySize;
    }
    return *this;
}
ServerConfig::~ServerConfig(){}

int ServerConfig::getPort()const{
    return (_port);
}
std::string ServerConfig::getServer()const{
    return (_server);
}
const std::map<int, std::string>& ServerConfig::getErrors()const{
    return _errors;
}
const std::vector<location> &ServerConfig::getLocations()const{
    return _locations;
}
size_t  ServerConfig::getMaxBodySize()const{
    return _maxBodySize;
}

void    ServerConfig::setPort(int port){
    _port = port;
}
void    ServerConfig::setServer(const std::string &server){
    _server = server;
}
void    ServerConfig::setErrors(int code, const std::string &path){
    _errors[code] = path;
}
void    ServerConfig::setLocations(const location &newLoc){
    _locations.push_back(newLoc);
}
void    ServerConfig::setMaxBodySize(size_t size){
    _maxBodySize = size;
}