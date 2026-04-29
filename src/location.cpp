#include "../includes/location.hpp"

location::location():_path(""), _root(""), _index(""), _autoindex(false){}

location::location(const location &copy) : _path(copy._path), _root(copy._root),
_index(copy._index), _autoindex(copy._autoindex), _methods(copy._methods){}

location &location::operator=(const location &target){
    if (this != &target){
        _path = target._path;
        _root = target._root;
        _index = target._index;
        _autoindex = target._autoindex;
        _methods = target._methods;
    }
    return *this;
}
location::~location(){}

std::string location::getPath()const{
    return (_path);
}
std::string location::getIndex()const{
    return (_index);
}
std::string location::getRoot()const{
    return (_root);
}
bool location::getAutoindex()const{
    return (_autoindex);
}
const std::vector<std::string> &location::getMethods()const{
    return (_methods);
}
void location::setPath(const std::string &path){
    _path = path;
}
void    location::setRoot(const std::string &root){
    _root = root;
}
void    location::setIndex(const std::string &index){
    _index = index;
}
void    location::setAutoindex(bool autoindex){
    _autoindex = autoindex;
}
void    location::addMethods(const std::string &methods){
    _methods.push_back(methods);
}