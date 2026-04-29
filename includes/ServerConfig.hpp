#pragma once
#include <map>
#include <string>
#include <vector>
#include "location.hpp"

class ServerConfig{
    private:
        int _port;
        std::string _server;
        std::map<int, std::string> _errors;
        std::vector<location> _locations;
        size_t _maxBodySize;
    public:
        ServerConfig();
        ServerConfig(const ServerConfig &copy);
        ServerConfig &operator=(const ServerConfig &target);
        ~ServerConfig();

        int getPort()const;
        std::string getServer()const;
        const std::map<int, std::string>& getErrors()const;
        const std::vector<location> &getLocations()const;
        size_t  getMaxBodySize()const;

        void    setPort(int port);
        void    setServer(const std::string &server);
        void    setErrors(int code, const std::string &path);
        void    setLocations(const location &newLoc);
        void    setMaxBodySize(size_t size);
};