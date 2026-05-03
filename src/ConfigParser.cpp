#include "../includes/ConfigParser.hpp"

std::string ConfigParser::parseValue(std::istringstream &iss, const std::string &word){
    std::string value;
    if (!(iss >> value)) // verifier que jai une valeur dedans
        throw std::runtime_error("Error: missing argument for '" + word + "'");
    bool semicolon = false;
    if(!value.empty() && value[value.size() - 1] == ';'){ // ; attache a la valeur ou pas 8080;
        value.erase(value.size() - 1);
        semicolon = true;
    }
    std::string otherwords;
    if (iss >> otherwords){
        if (!semicolon && otherwords == ";")
            semicolon = true;
        if(iss >> otherwords)
            throw std::runtime_error ("Error : unexpected argument after  '" + word + "'");
        else 
            throw std::runtime_error("Error : unexpected argument after  '" + word + "'");
    }
    if(!semicolon)
        throw std::runtime_error("Error: missing ';' after  '" + word + "'");
    return value;
}

void    ConfigParser::parseLocation(std::ifstream &file, location &l){
    std::string line;
    while (std::getline(file, line)){
        std::istringstream iss(line);
        std::string word;
        if(!(iss >> word))
            return ;
        if (word == "}"){
            break;
        }   
        else if (word == "allow_methods"){
            std::string method;
            bool semicolon = false;
            bool methadd = false;
            
            while (!semicolon && (iss >> method)){
                if (method == ";"){// ';' separe par la meth
                    semicolon = true;
                    break;
                }
            
            }
            if (!method.empty() && method[method.size() - 1] == ';'){
                method.erase(method.size() - 1); // ';' est attache
                semicolon = true;
            }
            if (method != "GET" && method != "DELETE" && method != "POST"){
                throw std::runtime_error("Error: invalid method '" + method + "' in allow_methods");
            }
            if (!method.empty()){
                l.addMethods(method);
                methadd = true;
            }
            if(!methadd)
                throw std::runtime_error("Error: allow_methods mus at least have one method");
            if (!semicolon)
                throw std::runtime_error("Error: missing ';' after allow_methods");
            std::string extra;
            if (iss >> extra)   
                throw std::runtime_error("Error: unexpected argument after allow_methods");
            
        }
        else if (word == "root"){
            std::string path = parseValue(iss, "root");
            l.setRoot(path);
        }
        else if (word == "index"){
            std::string index = parseValue(iss, "index");
            l.setIndex(index);
        }
        else if(word == "autoindex"){
            std::string autoindex = parseValue(iss, "autoindex");
            if (autoindex != "on" && autoindex != "off")
                throw std::runtime_error("Error: autoindex must be 'on' or 'off'");
            l.setAutoindex(autoindex == "on");
        }
    }
}

void    ConfigParser::parseListen(std::istringstream &iss, ServerConfig &ns){
    std::string value = parseValue(iss, "listen");
    for (size_t i = 0; i < value.length(); i++){
        if (!std::isdigit(value[i]))
            throw std::runtime_error("Error: listen port must only contain digits");
    }
    ns.setPort(std::atoi(value.c_str()));
}
void    ConfigParser::parseServerName(std::istringstream &iss, ServerConfig &ns){
    std::string value = parseValue(iss, "server_name");
    ns.setServer(value);
}
void    ConfigParser::parseClientMaxBodySize(std::istringstream &iss, ServerConfig &ns){
    std::string value = parseValue(iss, "clien_max_body_size");
    ns.setMaxBodySize(std::atoi(value.c_str()));
}
void    ConfigParser::parseErrorPage(std::istringstream &iss, ServerConfig &ns){
    std::string code, path;
    if (!(iss >> code))
        throw std::runtime_error("Error: missing code for 'error_page'");
    for (size_t i = 0; i < code.length(); i++){
        if (!std::isdigit(code[i]))
            throw std::runtime_error("Error: error_page must be numeric");
    }
    if (!(iss >> path))
        throw std::runtime_error("Error: missing path for 'error_page'");
    bool semicolon = false;
    std::string ow;
    if(!path.empty() && path[path.size() - 1] == ';'){
        path.erase(path.size() - 1); 
        semicolon = true;
    } 

    if (iss >> ow) {
        if (!semicolon && ow == ";") {
            semicolon = true;
            if (iss >> ow) {
                throw std::runtime_error("Error: unexpected argument after 'error_page'");
            }
        } else {
            throw std::runtime_error("Error: unexpected argument after 'error_page'");
        }
    }

    if (!semicolon)
        throw std::runtime_error("Error: missing ';' after 'error_page'");         
    ns.setErrors(std::atoi(code.c_str()), path);
}

void ConfigParser::parseLine(const std::string &line, ServerConfig &ns, std::ifstream &file){
    std::istringstream iss(line);
    std::string word;
    if (!(iss >> word))
        return ;
    if (word == "}")
        return ;
    else if (word =="listen")
        parseListen(iss, ns);
    else if(word == "server_name")
        parseServerName(iss, ns);
    else if(word == "client_max_body_size")
        parseClientMaxBodySize(iss, ns);
    else if (word == "error_page")
        parseErrorPage(iss, ns);
    else if (word == "location"){
        location l;
        std::string path;
        iss >> path;
        if (!path.empty() && path[path.size() - 1] == '{')
            path.erase(path.size() - 1);
        l.setPath(path);
        parseLocation(file, l);
        ns.setLocations(l);
    }
    else if (word != "}")
    {
        throw std::runtime_error("Error: unknown directive '" + word + "'");
    }
}


void ConfigParser::parseServer(std::ifstream &file){
    std::string line;
    ServerConfig ns;
    while(std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string word;
        if (!(iss >> word))
            continue ;
        if (word == "}"){
            _servers.push_back(ns);
            break;
        }
        parseLine(line, ns, file);
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
        std::istringstream iss(line);
        std::string word;
        if (!(iss >> word)) {continue ;}
        if (word == "server") {
            std::string parseBracket;
            if (iss >> parseBracket && parseBracket != "{")
                throw std::runtime_error("Error: missing { after server");
            parseServer(file);
        } else {
             throw std::runtime_error("Error: unknown directive outside server");
        }
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
