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
    int opt = 1;
    /*La sous-routine setsockopt fournit à un programme 
    d'application les moyens de contrôler une 
    communication de socket. 
    La fonction setsockopt définit la valeur actuelle 
    d'une option de socket associée à un socket de tout type et dans n'importe quel état*/
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        close(server_fd);
        throw std::runtime_error("Error: setsockpot failed");
    }
    sockaddr_in address;
    //ADDRESS CONFIGURATION
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;// -> je travaille en IPv4
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 -> j'accepte les connexions partout ->j'ecoute partout
    address.sin_port = htons(server.getPort()); // Host TO Network Short -> port défini par la config

    //BIND -> attach socket to the address
    // Added comma between server_fd and (struct sockaddr *)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: bind failed");
    }
    //LISTEN
    if (listen(server_fd, 10) < 0)//-> waiting for client to connect, with max 10 pending connexioons
    {
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: listen failed");
    }
    setNB(server_fd);
    return server_fd;
}

/*le but est d’initialiser tous les sockets d’écoute à partir de la liste de configurations _servers. 
Pour chaque serveur, le code crée un socket avec createSocket(_servers[i]), 
stocke son descripteur dans _listenFds, 
puis construit une structure pollfd pour dire au système: 
surveille ce socket pour savoir quand une connexion entrante arrive.
Le message affiché sert juste à confirmer sur quel port le serveur écoute. 
Le rôle global est donc: préparer le serveur avant de lancer la boucle d’événements. */
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

/*Dans run(), le serveur entre dans une boucle infinie.
À chaque tour, il appelle poll pour attendre qu’un des sockets surveillés devienne prêt.
Quand un événement arrive, il parcourt tous les pollfd et teste si l’événement
correspond à une nouvelle entrée sur un socket d’écoute. 
Si oui, il accepte la connexion du client avec accept, 
passe le descripteur en non-bloquant via setNB, affiche que le client est connecté, 
puis ferme le socket client. 
cette fonction sert à détecter une connexion entrante, pas à dialoguer avec le client.*/
void ServerManager::run(){
    while(true){
        if (_pollfds.empty())
            return;
        int ready = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ready < 0)
            throw std::runtime_error("Error: poll failed");
        for (size_t i = 0; i < _pollfds.size(); i++){
            if (_pollfds[i].revents & POLLIN){
                int client_fd = accept(_pollfds[i].fd, NULL, NULL);
                if (client_fd < 0){
                    std::cout << "Error: accept failed"<< std::endl;
                    continue ;
                }
                setNB(client_fd);
                std::cout << "Client connected: "<< client_fd << std::endl;
                close(client_fd);
            }
        }
    }
}