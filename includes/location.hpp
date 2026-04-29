#pragma once 

#include <string>
#include <vector>

class location{
    private:
        std::string              _path;
        std::string              _root;
        std::string              _index;
        bool                     _autoindex;
        std::vector<std::string> _methods;
    public:
        location();
        location(const location &copy);
        ~location();
        location &operator=(const location &target);

        std::string getPath()const ;
        std::string getIndex()const ;
        std::string getRoot()const ;
        bool getAutoindex()const ;
        const std::vector<std::string> &getMethods() const;

        void    setPath(const std::string &path);
        void    setRoot(const std::string &root);
        void    setIndex(const std::string &index);
        void    setAutoindex(bool autoindex);
        void    addMethods(const std::string &methods);
};