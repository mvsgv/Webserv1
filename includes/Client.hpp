#pragma once
#include <string>
#include "HttpRequest.hpp"
#include <sys/types.h>
class Client{
    private:
        int _fd;
        int _port;
        size_t _maxBodySize;
        std::string _readRequest;
        std::string _writeRequest;
    public:
        Client(int fd);
        Client(const Client &copy);
        Client &operator=(const Client &target);
        ~Client();

        int getFd()const;
        bool isRequestComplete()const;
        void setMaxBodySize(size_t size);
        void setWriteRequest(const std::string &response);
        const std::string &getWriteRequest()const;
        void clearWriteRequest();
        bool appendreadRequest(const char *buff, ssize_t bytes);
        const std::string &getReadRequest() const;
};