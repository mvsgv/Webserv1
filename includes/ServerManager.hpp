#pragma once
#include "../includes/ServerConfig.hpp"
#include <iostream>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include "Client.hpp"

class ServerManager{
    private:
        std::vector<ServerConfig> _servers;
        std::vector<int> _listenFds;
        std::vector<struct pollfd> _pollfds;

        int createSocket(const ServerConfig &server);
        void setNB(int fd);
    public:
        ServerManager(const std::vector <ServerConfig> &servers);
        ~ServerManager();
        void setup();
        void run();
        
        //registre de tout les clients connectes
        std::map<int, Client> _clients;
};