// LLMServer.hpp
#pragma once

#include "syncflag.hpp"
#include "connection_context.hpp"
#include "socket_manager.hpp"

#include <cstdio>
#include <atomic>
#include <vector>
#include <thread>

class ConnectionContext;

class LLMServer {
public:
    static LLMServer& Instance(USHORT port);
    static LLMServer& Instance(); // No port after first init

    bool Initialize();
    void RunMainLoop();  // actual server logic
    HANDLE GetStopEvent() const;

    void SetKeepAlive(bool pState);
    bool KeepConnection(bool pState);
    bool KeepConnection() const;
    bool IsKeepAliveSet() const;
    Protocol GetProtocol() const;
    ConnectionContext* CreateConnectionContext(const ConnectionInfo& clientInfo, LLMServer * pServer);
    HANDLE GetIOCP() const;
    
private:
    LLMServer(USHORT port);
    
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

    WORD vThreadCount;
    std::vector<std::thread> vThreads;
    ULONG vConnectionContextCount;

    SocketManager* vSocketMgr;
    
    bool vPerformKeepAlive;
    bool vKeepConnection;
    static LLMServer* s_instance;
    static USHORT s_initPort;
};
