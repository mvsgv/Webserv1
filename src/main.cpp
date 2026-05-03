#include "../includes/inc.hpp"
#include "../includes/ConfigParser.hpp"
#include <vector>

static void	printServers(const ConfigParser &parser)
{
    const std::vector<ServerConfig> &servers = parser.getServers();
    for (size_t i = 0; i < servers.size(); ++i)
    {
        std::cout << "Server " << i << std::endl;
        std::cout << "  port: " << servers[i].getPort() << std::endl;
        std::cout << "  server_name: " << servers[i].getServer() << std::endl;
        std::cout << "  max_body_size: " << servers[i].getMaxBodySize() << std::endl;

        const std::vector<location> &locations = servers[i].getLocations();
        for (size_t j = 0; j < locations.size(); ++j)
        {
            std::cout << "  Location " << j << std::endl;
            std::cout << "    path: " << locations[j].getPath() << std::endl;
            std::cout << "    root: " << locations[j].getRoot() << std::endl;
            std::cout << "    index: " << locations[j].getIndex() << std::endl;
            std::cout << "    autoindex: " << locations[j].getAutoindex() << std::endl;

            const std::vector<std::string> &methods = locations[j].getMethods();
            for (size_t k = 0; k < methods.size(); ++k)
                std::cout << "    method: " << methods[k] << std::endl;
        }
    }
}

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
    
    try {
        ConfigParser parser(res);
        printServers(parser);
    } catch(const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}