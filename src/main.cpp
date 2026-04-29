#include "../includes/inc.hpp"

int main(int argc, char **argv){
    if (argc > 2){
        std::cerr << "Error: wrong number of arguments." << std::endl; 
        std::cerr << "Usage : ./webserv [conf_file]" << std::endl; 
        return 1;
    }
    std::string res;
    if (argc == 1)
        res = "conf/def.conf";
    else
        res = argv[1];
    std::cout << "Starting Webserv with configuration file : "<< res << std::endl;
    return 0;
}