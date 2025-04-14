// main.cpp

// Entry point for the HTTP server; uses service.hpp to decide how to launch the server
#include "service.hpp"

int main() {
    
    RunAsService();
    return 0;
}
