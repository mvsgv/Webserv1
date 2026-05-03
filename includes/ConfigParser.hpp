#pragma once
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include "ServerConfig.hpp"
class ConfigParser{
    private:
        std::vector<ServerConfig> _servers;
        void parseServer(std::ifstream &file);
        void parseLocation(std::ifstream &file, location &l);
        void parseListen(std::istringstream &iss, ServerConfig &ns);
        void parseServerName(std::istringstream &iss, ServerConfig &ns);
        void parseClientMaxBodySize(std::istringstream &iss, ServerConfig &ns);
        void parseErrorPage(std::istringstream &iss, ServerConfig &ns);
        void parseLine(const std::string &line, ServerConfig &ns, std::ifstream &file);
        std::string parseValue(std::istringstream &iss, const std::string &word);    
    public:
        ConfigParser(const std::string &filename);
        ConfigParser(const ConfigParser &copy);
        ConfigParser &operator=(const ConfigParser &target);
        ~ConfigParser();

        const std::vector<ServerConfig> &getServers()const;

};