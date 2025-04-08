#pragma once
#include <winsock2.h> // for SOCKET
#include <string>

struct ConnectionInfo {
    SOCKET uSocket = INVALID_SOCKET;
    bool   uIsSocketActive = false;

    std::string uIP;
    unsigned long uPort = 0;
    std::string uHostName;

    unsigned long uCCId = 0;

    // Not used in HTTP version
    std::string uURLPath;
    void*       uSSLSessionInfo = nullptr;
};
