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
    public:
        ConfigParser(const std::string &filename);
        ConfigParser(const ConfigParser &copy);
        ConfigParser &operator=(const ConfigParser &target);
        ~ConfigParser();

        const std::vector<ServerConfig> &getServers()const;

};