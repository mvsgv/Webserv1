#include "../includes/ConfigParser.hpp"

void    ConfigParser::parse_location(std::ifstream &file, location &l){
    std::string line;
    std::istringstera(line);
    std::string word, path;
    
    while (std::getline(file, line)){
        if (line.find("}") != std::string::npos){
            break;
        }   
        else if (line.find("allow_methods") != std::string::npos){

        }

    }
}

void ConfigParser::parseServer(std::ifstream &file){
    std::string line;
    ServerConfig ns;
    while(std::getline(file, line))
    {
        if (line.find("listen") != std::string::npos){
            ns.setPort(8080);
            std::istringstream iss(line);
            std::string word, value;
            iss >> word;
            iss >> value;
            value.erase(value.size() - 1);

            int port = std::atoi(value.c_str());
            ns.setPort(port);
        }
        else if (line.find("server_name") != std::string::npos){
            std::istringstream iss(line);
            std::string word, value;
            iss >> word;
            iss >> value;
            value.erase(value.size() - 1);
            ns.setServer(value);
        }
        else if (line.find("lient_max_body_size") != std::string::npos){
            std::istringstream iss(line);
            std::string word, value;
            iss >> word;
            iss >> value;
            value.erase(value.size() - 1);
            size_t mbs = std::atoi(value.c_str());
            ns.setMaxBodySize(mbs);
        }
        else if(line.find("location") != std::string::npos){
            location l;
            std::istringstream iss(line);
            std::string word, path;
            iss >> word >> path;
            l.setPath(path);
            parse_location(file, l);
            ns.setLocations(l);
        }
        else if (line.find("}") != std::string::npos){
            _servers.push_back(ns);
            break;
        }
    }
}

ConfigParser::ConfigParser(const std::string &filename){
    std::ifstream file(filename.c_str());

    if (!file.is_open()){
        std::cerr << "Error: can't open file" << filename<< std::endl;
        return;
    }
    std::string line;
    while(std::getline(file, line)){
        if (line.find("server") != std::string::npos)
            parseServer(file);
    }

}

ConfigParser::ConfigParser(const ConfigParser &copy) : _servers(copy._servers){}
ConfigParser &ConfigParser::operator=(const ConfigParser &target){
    if (this != &target)
        _servers = target._servers;
    return *this;
}
ConfigParser::~ConfigParser(){}

const std::vector<ServerConfig> &ConfigParser::getServers()const{
    return _servers;
}
