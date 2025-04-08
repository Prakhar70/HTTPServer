// LLMServer.hpp
#pragma once

class ConnectionContext;

#include "syncflag.hpp"
#include "pch.hpp"
#include "connection_context.hpp"
#include "socket_manager.hpp"

#include <cstdio>
#include <atomic>
#include <vector>
#include <thread>


class LLMServer {
public:
    LLMServer(USHORT port);
    bool Initialize();
    void RunMainLoop();  // actual server logic
    HANDLE GetStopEvent() const;

    void SetKeepAlive(bool pState);
    bool KeepConnection(bool pState);
    bool KeepConnection() const;
    bool IsKeepAliveSet() const;
    eProtocol GetProtocol() const;
    ConnectionContext* CreateConnectionContext(const ConnectionInfo& clientInfo);
    

private:
    void SetAcceptNewConnection(bool state);
    bool InitializeBaseServer();
    bool GetAcceptNewConnection() const;
    bool CreateThreadPool();
    void Finalize();
    void FinalizeBaseServer();
    static void ProcessClientRequest(LLMServer* pThis);
    void CleanupThreadPool();
    bool HandleAcceptEvent();

    mutable SyncFlag vSyncFlag;
    bool vIsAcceptNewConns;

    HANDLE vStopEvent;
    HANDLE vIOCompletionPort;

    DWORD vThreadCount;
    std::vector<std::thread> vThreads;
    ULONG vConnectionContextCount;

    SocketManager* vSocketMgr;
    
    bool vPerformKeepAlive;
    bool vKeepConnection;

};
