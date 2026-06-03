#include "../includes/Client.hpp"

Client::Client(int fd): _fd(fd), _port(0), _maxBodySize(1048576){}
Client::Client(const Client &copy){
    *this = copy;
}
Client &Client::operator=(const Client &target){
    if (this != &target){
        this->_fd = target._fd;
        this->_port = target._port;
        this->_maxBodySize = target._maxBodySize;
        this->_readRequest = target._readRequest;
        this->_writeRequest = target._writeRequest;
    }
    return *this;
}
Client::~Client(){}

void Client::setMaxBodySize(size_t size){
    _maxBodySize = size;
}

int Client::getFd()const{
    return _fd;
}

//On ajoute une marge arbitraire sécurisée d'environ 8 Ko (8192 octets) 
//pour tolérer la taille des en-têtes HTTP de la requête brute
bool Client::appendreadRequest(const char *buff, ssize_t bytes){
    if (bytes > 0){
        if (_readRequest.size() + bytes > _maxBodySize + 8192){
                return false;
        }
        _readRequest.append(buff, bytes);
    }
    return true;
}
 //chercher la sequence de caracter \r\n pour etre sur que c est la fin de la requette
 //si la sequence est trouvee je retourne true
bool Client::isRequestComplete() const {
    size_t head_end = _readRequest.find("\r\n\r\n");

    // Si les en-têtes ne sont pas encore complètement reçus
    if (head_end == std::string::npos)
        return false;

    std::string headers = _readRequest.substr(0, head_end);
    if (headers.find("Transfer-Encoding: chunked") != std::string::npos)
        return _readRequest.find("0\r\n\r\n") != std::string::npos;

    return true;
}

void Client::setWriteRequest(const std::string &response){
    _writeRequest = response;
}

const std::string &Client::getWriteRequest()const{
    return _writeRequest;
}

void Client::clearWriteRequest(){
    _writeRequest.clear();
}
