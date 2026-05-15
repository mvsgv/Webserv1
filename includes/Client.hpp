#pragma once
#include <string>
#include <sys/types.h>
class Client{
    private:
        int _fd;
        int _port;
        std::string _readRequest;
        std::string _writeRequest;
    public:
        Client(int fd);
        Client(const Client &copy);
        Client &operator=(const Client &target);
        ~Client();

        int getFd()const;
        void appendreadRequest(const char *buff, ssize_t bytes);
};