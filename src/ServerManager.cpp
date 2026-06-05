#include "../includes/ServerManager.hpp"
#include <cstring>
#include <stdexcept>

ServerManager::ServerManager(const std::vector <ServerConfig> &servers) : _servers(servers){}
ServerManager::~ServerManager(){
    for (size_t i = 0; i < _listenFds.size(); i++)
        close(_listenFds[i]);
}
// Rend un descripteur de fichier (fd) non-bloquant
// O_NONBLOCK permet aux fonctions d'I/O (recv, send, accept) de retourner 
// immédiatement sans figer le programme si aucune donnée n'est prête.
void    ServerManager::setNB(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Error: F_GETFL failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Error: F_SETFL failed");
}

// Crée et configure un socket serveur pour une configuration donnée
int ServerManager::createSocket(const ServerConfig &server){
     // 1. Création du socket (IPv4, TCP)
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

    //BIND -> attach socket to the local address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: bind failed");
    }
    //5. LISTEN : Prépare le socket à accepter des connexions entrantes (file d'attente de 10)
    if (listen(server_fd, 10) < 0)
    {
        close(server_fd); // Clean up on error
        throw std::runtime_error("Error: listen failed");
    }
    // 6. Rend le socket d'écoute non-bloquant
    setNB(server_fd);
    return server_fd;
}

/*initialise tous les sockets d’écoute à partir de la liste de configurations _servers. 
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

        // Crée l'instruction pour poll() : surveille ce socket pour la LECUTRE (POLLIN)
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

        // poll() bloque jusqu'à ce qu'un événement survienne sur l'un des sockets surveillés
        int ready = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ready < 0)
            throw std::runtime_error("Error: poll failed");

        // On parcourt tous les sockets surveillés pour voir s'il y a de l'action
        for (size_t i = 0; i < _pollfds.size(); i++){

            // Evénement de LECTURE détecté
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

                //creer le client avec maxbodysize
                Client newC(client_fd);
                newC.setMaxBodySize(_servers[server_index].getMaxBodySize()); // Applique la limite configurée
                _clients.insert(std::make_pair(client_fd, newC));

                // Ajoute le client à poll pour surveiller ses futures requêtes (POLLIN)
                struct pollfd client_pfd;
                client_pfd.fd = client_fd;
                client_pfd.events = POLLIN;
                client_pfd.revents = 0;
                _pollfds.push_back(client_pfd);
                std::cout << "Client connected: "<< client_fd << std::endl;
                } 
                else{
                // CLIENT EXISTANT : des données sont arrivées
                char buff[4096]; //pour lire ce que je vais recevoir
                std::memset(buff, 0, sizeof(buff));
                ssize_t b_read = recv(current_fd, buff, sizeof(buff), 0);
                if (b_read <= 0){
                    std::cout << "Client disconnected: " <<  current_fd << std::endl;
                    close(current_fd);
                    _clients.erase(current_fd);
                    _pollfds.erase(_pollfds.begin());
            }
            else {
                // Stocke les données reçues dans le buffer du client
                Client &myClient = _clients.at(current_fd);
                // VERIFICATION DE SECURITE OBLIGATOIRE
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
                        // 2. On donne la requête au routeur de votre mate
                        Router router;
                        // Attention: Le routeur aura besoin de connaître la vraie Config (ServerConfig / location)
                        HttpResponse response = router.route(request);
                        // 3. On stocke la string brute dans le buffer d'écriture du client
                        myClient.setWriteRequest(response.buildResponse());
                    }
                    // On bascule la surveillance du client de LECTURE vers ÉCRITURE
                    _pollfds[i].events = POLLOUT; //demande a ecouter la disponibilite pour ecrire
                    }
                }
            }
            }
            // Événement en ÉCRITURE détecté (le socket est prêt à envoyer des données)
            //pollout -> output available
            if (_pollfds[i].revents & POLLOUT){
                int current_fd = _pollfds[i].fd;
                Client &myClient = _clients.at(current_fd);
                const std::string &response = myClient.getWriteRequest();

                // On envoie la réponse stockée
                ssize_t b_sent = send(current_fd, response.c_str(), response.length(), 0);
                if (b_sent <= 0){
                    std::cout << "Send error or client disconnected : " << current_fd << std::endl;
                }else{
                    std::cout << "Sent "<< b_sent << " bytes to client "<< current_fd << std::endl;
                }
                // Pour l'instant, connexion fermée après envoi 
                close(current_fd);
                _clients.erase(current_fd);
                _pollfds.erase(_pollfds.begin() + i);
                i--;// Ajuste l'index car on vient de supprimer un élément du vecteur
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