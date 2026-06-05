#pragma once

#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <stdlib.h>
class CgiHandler {
public:
    CgiHandler();
    ~CgiHandler();

    std::string executeCGI(const std::string& script_path);
};