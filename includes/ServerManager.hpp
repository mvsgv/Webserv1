#pragma once
#include "../includes/ServerConfig.hpp"
#include <iostream>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

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
};