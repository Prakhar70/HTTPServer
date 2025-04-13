
// SocketManager.cpp
#include "socket_manager.hpp"

SocketManager::SocketManager(USHORT port)
    : vPort(port), vListen(INVALID_SOCKET), vAcceptEvent(0) {}

bool SocketManager::Initialize() {

    if (!CreateAndBindSocket()) {
        printf("[SocketManager] Failed to create and bind listen socket.\n");
        return false;
    }

    if (listen(vListen, SOMAXCONN) == SOCKET_ERROR) {
        printf("[SocketManager] listen() failed with error: %d\n", WSAGetLastError());
        return false;
    }

    // create a accept event
    vAcceptEvent = WSACreateEvent();

    if (vAcceptEvent == WSA_INVALID_EVENT) {
        printf("[SocketManager] WSACreateEvent failed. Error = %d\n", WSAGetLastError());
        return false;
    }

    // associate that socket with event for FD_ACCEPT
    if (!SetAcceptEvent()) {
        printf("[SocketManager] WSAEventSelect failed. Error = %d\n", WSAGetLastError());
        return false;
    }

    printf("[SocketManager] Listening on port %d\n", vPort);
    return true;
}
//TODO:Understanding why shutdown
void SocketManager::CloseSocket(){

    if (vListen != INVALID_SOCKET) {

        // calling a shutdown on a socket that's is non conneted to 
        /*if (shutdown(vListen, SD_RECEIVE) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            printf("[ERROR] WSASocket failed. WSAGetLastError = %d\n", err);
            printf("[SocketManager] shutdown socket failed\n");
        }*/

        if (closesocket(vListen) == SOCKET_ERROR) {
            printf("[SocketManager] closesocket failed\n");
        }
        vListen = INVALID_SOCKET;
    }   
}
void SocketManager::Finalize(){

    if (vListen != INVALID_SOCKET) {
        CloseSocket();
    }
    if (vAcceptEvent) {
        if (WSACloseEvent(vAcceptEvent)==FALSE) {
            printf("[SocketManager] WSACloseEvent::closesocket failed\n");
        }
        vAcceptEvent = 0;
    }

}

WSAEVENT SocketManager::GetAcceptEvent() {

    return vAcceptEvent;
}

bool SocketManager::IsAcceptEventOccured(){

        WSANETWORKEVENTS    netevents;

    // Query any network event occured
    if (WSAEnumNetworkEvents (vListen, vAcceptEvent, &netevents) == SOCKET_ERROR) {

        // standard API failed so log the system error code to known error
        printf("[SocketManager]  Error = %d\n", WSAGetLastError());

        // log the function name which failed
        printf("[SocketManager] SocketManager::WSAEnumNetworkEvents function failed\n");
        

        // return FALSE to indicate that processing failed
        return false;
    }

    // Check whether AcceptEvent only occured
    if ((netevents.lNetworkEvents & FD_ACCEPT) == 0) {

        // standard API failed so log the system error code to known error
        printf("[SocketManager] ServerSock::Unknown event has occured\n");

        // return FALSE to indicate that processing failed
        return false;               
    }

    // AcceptEvent occured with some error
    if (netevents.iErrorCode[FD_ACCEPT_BIT] != 0) {

        // log the function name which failed
        printf("[SocketManager] ServerSock::Unknown event has occured\n");
        return false;
    }

    return true;
}

bool SocketManager::SetAcceptEvent() {

    if(WSAEventSelect(vListen, vAcceptEvent, FD_ACCEPT) == SOCKET_ERROR){
        return false;
    }
    return true;
}

bool SocketManager::ResetAcceptEvent() {

    if(WSAEventSelect(vListen, vAcceptEvent, 0) == SOCKET_ERROR){
        return false;
    }
    return true;
}


bool SocketManager::CreateAndBindSocket() {

    vListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (vListen == INVALID_SOCKET) {
        int err = WSAGetLastError();
        printf("[ERROR] WSASocket failed. WSAGetLastError = %d\n", err);
        printf("[SocketManager] WSASocket failed with error: %d\n", WSAGetLastError());
        return false;
    }

    // Enable keep-alive
    BOOL keepAlive = TRUE;
    if (setsockopt(vListen, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive)) == SOCKET_ERROR) {
        printf("[SocketManager] setsockopt SO_KEEPALIVE failed with error: %d\n", WSAGetLastError());
    }

    sockaddr_in service = {};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY);
    service.sin_port = htons(vPort);

    if (bind(vListen, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        printf("[SocketManager] bind failed with error: %d\n", WSAGetLastError());
        closesocket(vListen);
        vListen = INVALID_SOCKET;
        return false;
    }

    return true;
}

int CALLBACK FilterConnectionConditionProc(
    LPWSABUF /* lpCallerId */,
    LPWSABUF /* lpCallerData */,
    LPQOS    /* lpSQOS */,
    LPQOS    /* lpGQOS */,
    LPWSABUF /* lpCalleeId */,
    LPWSABUF /* lpCalleeData */,
    GROUP*   /* g */,
    DWORD_PTR dwCallbackData
) {

    if (dwCallbackData) {
        return CF_ACCEPT; //Accept the connection
    }
    printf("[SocketManager]conditionally reject the connection");
    return CF_REJECT;  // Reject the connection
}

bool SocketManager::AcceptConnection(ConnectionInfo* pClient, bool acceptNewConnections, bool setKeepAlive) {
    sockaddr_storage addrStorage = {};
    int addrLen = sizeof(addrStorage);
    char ipBuf[NI_MAXHOST] = {};
    SOCKET clientSock = INVALID_SOCKET;

    // Accept connection
    clientSock = WSAAccept(
        vListen,
        reinterpret_cast<sockaddr*>(&addrStorage),
        &addrLen,
        FilterConnectionConditionProc,
        static_cast<DWORD>(acceptNewConnections)
    );

    if (clientSock == INVALID_SOCKET) {
        printf("[SocketManager] WSAAccept failed. Error: %d\n", WSAGetLastError());
        return false;
    }

    // Optional keep-alive setup
    if (setKeepAlive) {
        tcp_keepalive ka = {};
        ka.onoff = 1;
        ka.keepalivetime = 60000;     // 60 sec
        ka.keepaliveinterval = 1000;  // 1 sec

        DWORD bytesReturned = 0;
        WSAIoctl(
            clientSock,
            SIO_KEEPALIVE_VALS,
            &ka,
            sizeof(ka),
            nullptr,
            0,
            &bytesReturned,
            nullptr,
            nullptr
        );

        BOOL keepaliveOpt = TRUE;
        setsockopt(clientSock, IPPROTO_TCP, SO_KEEPALIVE, reinterpret_cast<const char*>(&keepaliveOpt), sizeof(keepaliveOpt));
    }

    // Determine IP version & extract IP and Port
    if (addrLen == sizeof(sockaddr_in)) {
        const sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(&addrStorage);
        pClient->uPort = ntohs(addr4->sin_port);
    } else {
        const sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(&addrStorage);
        pClient->uPort = ntohs(addr6->sin6_port);
    }

    // Extract IP address
    if (getnameinfo(
            reinterpret_cast<sockaddr*>(&addrStorage),
            addrLen,
            ipBuf,
            sizeof(ipBuf),
            nullptr,
            0,
            NI_NUMERICHOST) != 0)
    {
        printf("[SocketManager] getnameinfo failed. Error: %d\n", WSAGetLastError());
        closesocket(clientSock);
        return false;
    }

    // Fill connection info
    pClient->uSocket = clientSock;
    pClient->uIsSocketActive = true;
    pClient->uIP = ipBuf;
    pClient->uHostName = ipBuf;

    return true;
}




