// SocketManager.hpp
#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>
#include <connection_info.hpp>

class SocketManager {
public:
    SocketManager(USHORT port);
    bool Initialize();
    void Finalize();
    void CloseSocket();

    bool CreateAndBindSocket();
    WSAEVENT GetAcceptEvent();
    bool IsAcceptEventOccured();
    bool SetAcceptEvent();
    bool AcceptConnection(ConnectionInfo* pClient, bool acceptNewConnections, bool setKeepAlive);

private:
    USHORT vPort;
    SOCKET vListen;
    WSAEVENT vAcceptEvent;
};

