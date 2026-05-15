#include "../includes/Client.hpp"

Client::Client(int fd): _fd(fd), _port(0){}
Client::Client(const Client &copy){
    *this = copy;
}
Client &Client::operator=(const Client &target){
    if (this != &target){
        this->_fd = target._fd;
        this->_port = target._port;
        this->_readRequest = target._readRequest;
        this->_writeRequest = target._writeRequest;
    }
    return *this;
}
Client::~Client(){}

int Client::getFd()const{
    return _fd;
}

void Client::appendreadRequest(const char *buff, ssize_t bytes){
    if (bytes > 0){
        _readRequest.append(buff, bytes);
    }
}