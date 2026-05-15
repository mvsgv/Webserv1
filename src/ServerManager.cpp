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
    /* O_NONBLOCK fait que si une fonction n'a rien à faire, 
    au lieu de figer, elle renvoie une erreur 
    (type EWOULDBLOCK) et le code continue.*/
}
/*HyperText Transfer Protocol
 est le fondement de l'échange de données sur le World Wide Web, 
 permettant de transmettre des ressources (HTML, images, vidéos)
  entre un serveur et un client (navigateur). 
  Fonctionnant sur un modèle requête-réponse, 
  il est extensible et évolue avec le web.
  HTTP transmet les données en clair, 
  tandis que HTTPS (HTTP Secure) chiffre la communication 
  pour garantir la confidentialité et la sécurité des données.*/
/*Un socket est un point d'accès logiciel qui permet la
 communication bidirectionnelle entre deux processus, 
 que ce soit sur la même machine ou à travers un réseau*/
int ServerManager::createSocket(const ServerConfig &server){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Error: socket failed");
    int opt = 1;
    /*La sous-routine setsockopt fournit à un programme 
    d'application les moyens de contrôler une 
    communication de socket. 
    permet de définir les options associées à un socket, 
    quel que soit son type ou son état, pour contrôler son 
    comportement (ex: timeout, tampons, diffusion)*/
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

    //BIND -> attach socket to the local address
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
                
                int current_fd = _pollfds[i].fd;
                bool is_server = false;
                for (size_t s = 0; s < _listenFds.size(); s++){
                    if (current_fd == _listenFds[s])
                        is_server = true;
                            break;
                }
                if (is_server){ /*si cest un nv client*/

                    int client_fd = accept(_pollfds[i].fd, NULL, NULL);
                    if (client_fd < 0){
                        std::cout << "Error: accept failed"<< std::endl;
                        continue ;
                    }
                    setNB(client_fd);
                    //creer le client et le stocker dans map pour garder trace
                    _clients.insert(std::make_pair(client_fd, Client(client_fd)));
                    struct pollfd client_pfd;
                    client_pfd.fd = client_fd;
                    client_pfd.events = POLLIN;
                    client_pfd.revents = 0;
                    _pollfds.push_back(client_pfd);
                    std::cout << "Client connected: "<< client_fd << std::endl;
                } 
                else{
                    char buff[4096]; //pour lire ce que je vais recevoir
                    std::memset(buff, 0, sizeof(buff));
                    ssize_t b_read = recv(current_fd, buff, sizeof(buff), 0);
                    if (b_read <= 0){
                        std::cout << "Client disconnected: " <<  current_fd << std::endl;
                        close(current_fd);
                        _clients.erase(current_fd);
                    }
                    else {
                        Client &myClient = _clients.at(current_fd);
                        myClient.appendreadRequest(buff, b_read);
                        std::cout << "Reveived " << b_read << "bytes from client " << current_fd << std::endl;
                    }
                }
            }
        }
    }
}
/*Poll() est une variation de select(2) : 
il attend que l'un des descripteurs de fichier parmi un ensemble 
soit prêt pour effectuer des entrées-sorties.
L'ensemble des descripteurs de fichier à surveiller 
est indiqué dans l'argument fds qui est un tableau de 
structures nfds du type :
struct pollfd {
    int   fd;          Descripteur de fichier (n de socket\ moi qui parle)
    short events;      Événements attendus(ce que je veux savoir, previens moi quand t as les donnes)
    short revents;     Événements détectés(ce qui s'est reelement passe)
    
    POLLIN = Il y a des données en attente de lecture. */