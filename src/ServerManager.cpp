#include "../includes/ServerManager.hpp"
#include <cstring>
#include <stdexcept>

ServerManager::ServerManager(const std::vector <ServerConfig> &servers) : _servers(servers){}
ServerManager::~ServerManager(){
    for (size_t i = 0; i < _listenFds.size(); i++)
        close(_listenFds[i]);
}

void    ServerManager::setNB(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Error: F_GETFL failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Error: F_SETFL failed");
}

int ServerManager::createSocket(const ServerConfig &server){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Error: socket failed");

    // 2. SO_REUSEADDR permet de redémarrer le serveur et réutiliser le port 
    // immédiatement sans erreur "Address already in use"
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        close(server_fd);
        throw std::runtime_error("Error: setsockpot failed");
    }
    //3. ADDRESS CONFIGURATION
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;// -> je travaille en IPv4
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 -> j'accepte les connexions partout ->j'ecoute partout
    address.sin_port = htons(server.getPort()); // Host TO Network Short -> port défini par la config

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: bind failed");
    }
    if (listen(server_fd, 10) < 0)
    {
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: listen failed");
    }
    setNB(server_fd);
    return server_fd;
}

void    ServerManager::setup(){
    for (size_t i = 0 ; i < _servers.size(); i++){
        int fd = createSocket(_servers[i]);
        _listenFds.push_back(fd);

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollfds.push_back(pfd);
        std::cout << "Listen on port" << _servers[i].getPort() << std::endl;
    }
}

void ServerManager::run(){
    while(true){
        if (_pollfds.empty())
            return;

        int ready = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ready < 0)
            throw std::runtime_error("Error: poll failed");

        for (size_t i = 0; i < _pollfds.size(); i++){

            if (_pollfds[i].revents & POLLIN){
                int current_fd = _pollfds[i].fd;
                bool is_server = false;
                size_t server_index = 0;

                // On vérifie si l'événement concerne un socket serveur (nouvelle connexion)
                for (size_t s = 0; s < _listenFds.size(); s++){
                    if (current_fd == _listenFds[s]) {
                        is_server = true;
                        server_index = s;
                        break;
                    }
                }

            // NOUVEAU CLIENT : on accepte la connexion
            if (is_server){
                int client_fd = accept(_pollfds[i].fd, NULL, NULL);
                if (client_fd < 0){
                    std::cout << "Error: accept failed"<< std::endl;
                    continue ;
                }
                setNB(client_fd);

                Client newC(client_fd);
                newC.setMaxBodySize(_servers[server_index].getMaxBodySize()); // Applique la limite configurée
                _clients.insert(std::make_pair(client_fd, newC));

                struct pollfd client_pfd;
                client_pfd.fd = client_fd;
                client_pfd.events = POLLIN;
                client_pfd.revents = 0;
                _pollfds.push_back(client_pfd);
                
                std::cout << "Client connected: "<< client_fd << std::endl;
                } 
                else{
                char buff[4096];
                std::memset(buff, 0, sizeof(buff));
                ssize_t b_read = recv(current_fd, buff, sizeof(buff), 0);
                if (b_read <= 0){
                    std::cout << "Client disconnected: " <<  current_fd << std::endl;
                    close(current_fd);
                    _clients.erase(current_fd);
                    _pollfds.erase(_pollfds.begin() + i);
            }
            else {
                Client &myClient = _clients.at(current_fd);
                if (myClient.appendreadRequest(buff, b_read) == false) {
                    std::cout << "Rejet 413 : Payload Trop Large" << std::endl;
                    std::string error413 = "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/plain\r\nContent-Length: 21\r\nConnection: close\r\n\r\n413 Payload Too Large";
                    myClient.setWriteRequest(error413);
                    _pollfds[i].events = POLLOUT;
                    continue; // On arrête la lecture pour ce client, on passe à l'envoi de l'erreur
                }                
                std::cout << "Received " << b_read << "bytes from client " << current_fd << std::endl;
            
                // Si la requête HTTP est entièrement reçue
                if (myClient.isRequestComplete()){
                    // 1. On parse la requête brute du client
                    HttpRequest request;
                    if (!request.parse(myClient.getReadRequest())) {
                        HttpResponse response = errorResponse(400); // Bad Request
                        myClient.setWriteRequest(response.buildResponse());
                    } else {
                        Router router;
                        HttpResponse response = router.route(request);
                        myClient.setWriteRequest(response.buildResponse());
                    }
                    _pollfds[i].events = POLLOUT; //demande a ecouter la disponibilite pour ecrire
                    }
                }
            }
            }
            if (_pollfds[i].revents & POLLOUT){
                int current_fd = _pollfds[i].fd;
                Client &myClient = _clients.at(current_fd);
                const std::string &response = myClient.getWriteRequest();
                ssize_t b_sent = send(current_fd, response.c_str(), response.length(), 0);
                if (b_sent <= 0){
                    std::cout << "Send error or client disconnected : " << current_fd << std::endl;
                    close(current_fd);
                    continue;
                }else{
                    std::cout << "Sent "<< b_sent << " bytes to client "<< current_fd << std::endl;
                }
                close(current_fd);
                _clients.erase(current_fd);
                _pollfds.erase(_pollfds.begin() + i);
                i--;
            }
        }
    }
}
