#include "llmserver.hpp"

LLMServer* LLMServer::s_instance = nullptr;
USHORT LLMServer::s_initPort = 0;

LLMServer::LLMServer(USHORT port)
    : vIsAcceptNewConns(true),
    vStopEvent(nullptr),
    vIOCompletionPort(nullptr),
    vThreadCount(IOTHREADS_COUNT),
    vConnectionContextCount(0),
    vKeepConnection(true),
    vPerformKeepAlive(false) {
    vSocketMgr = new SocketManager(port);
}

LLMServer& LLMServer::Instance(USHORT port) {
    if (!s_instance) {
        s_instance = new LLMServer(port);
        s_initPort = port;
    } else if (port != s_initPort) {
        printf("[LLMServer] Warning: Instance already initialized with port %d. Ignoring new value %d.\n", s_initPort, port);
    }
    return *s_instance;
}

LLMServer& LLMServer::Instance() {
    if (!s_instance) {
        printf("[LLMServer] FATAL: Instance() called before initialization.\n");
        std::terminate();
    }
    return *s_instance;
}


bool LLMServer::InitializeBaseServer() {

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("[SocketManager] WSAStartup failed with error: %d\n", result);
        return false;
    }

    vStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    // manual reset event(2nd parameter) and initially non-signalled(3rd parameter).

    if (!vStopEvent) {
        printf("[LLMServer] Failed to create stop event.\n");
        return false;
    }

    vIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, vThreadCount);

    if (!vIOCompletionPort) {
        printf("[LLMServer] Failed to create IOCP.\n");
        return false;
    }

    if (!CreateThreadPool()) {
        printf("[LLMServer] Failed to create thread pool.\n");
        return false;
    }
    return true;
}

void LLMServer::Finalize() {
    SetAcceptNewConnection(false);
    if (vSocketMgr) {
        vSocketMgr->Finalize();
    }
    FinalizeBaseServer();
}

void LLMServer::CleanupThreadPool() {
    printf("[LLMServer] Cleaning up thread pool...\n");

    if (!vIOCompletionPort) {
        printf("[LLMServer] IOCP not initialized, skipping cleanup.\n");
        return;
    }

    for (WORD i = 0; i < vThreadCount; ++i) {
        if (!PostQueuedCompletionStatus(vIOCompletionPort, 0, NULL, NULL)) {
            DWORD error = WSAGetLastError();
            printf("[LLMServer] PostQueuedCompletionStatus failed: %lu\n", error);
        }
    }

    int index = 0;
    for (auto& t : vThreads) {
        if (t.joinable()) {
            printf("[LLMServer] Joining thread #%d...\n", index);
            t.join();
            printf("[LLMServer] Thread #%d joined successfully.\n", index);
        }
        index++;
    }

    vThreads.clear();
    printf("[LLMServer] Thread pool cleaned up.\n");
}

void LLMServer::FinalizeBaseServer() {
    CleanupThreadPool();

    if (vStopEvent) {
        CloseHandle(vStopEvent);
        vStopEvent = 0;
    }

    if (vIOCompletionPort) {
        CloseHandle(vIOCompletionPort);
        vIOCompletionPort = 0;
    }

    WSACleanup();
}

bool LLMServer::Initialize() {

    SetAcceptNewConnection(true);

    if (InitializeBaseServer() && vSocketMgr->Initialize()) {
        printf("[LLMServer] Initialization successful.\n");
        return true;
    }

    Finalize();
    return false;
}

bool LLMServer::CreateThreadPool() {
    for (WORD i = 0; i < vThreadCount; ++i) {
        vThreads.emplace_back(&LLMServer::ProcessClientRequest, this);
    }
    return true;
}

void LLMServer::ProcessClientRequest(LLMServer* pThis) {
    printf("[LLMServer] Worker thread started.\n");

    DWORD bytes;
    ConnectionContext * cc;
    OVERLAPPED * olap;
    bool rc;

    while (true) {

        bytes = 0;
        olap = nullptr;
        cc = nullptr;

        rc = static_cast<bool>(GetQueuedCompletionStatus(
            pThis->vIOCompletionPort,
            &bytes,
            reinterpret_cast<PULONG_PTR>(&cc),
            &olap,
            INFINITE
        ));

        if (!rc) {
            if(!cc && !olap){

                printf("[LLMServer] error = %lu\n", WSAGetLastError());
                printf("[LLMServer] olap is null and function doesnt dequeue a completion packet from a completion port, return value is zero\n");
                continue;
            }
            if(cc && olap && bytes){

                printf("[LLMServer] error = %lu\n", WSAGetLastError());
                printf("[LLMServer] olap is not null and function  dequeue a completion packet for failed I/O operation from a completion port, return value is zero\n");
                continue;
            }
        }else{
            if(!cc && !olap){

                break;
            }
        }

        //case: graceful closure of client
        if(rc == 0 || (cc && !bytes && olap)){
            cc->SetState(MsgState::STATE_MSG_END);
        }

        MsgState result;
        result = cc->ProcessIO(pThis, bytes);

        if(result == MsgState::STATE_MSG_END){
            if(cc->Close() == true){
                //TODO : close connect
                delete cc;
            }
        }
    }
    
}

void LLMServer::SetAcceptNewConnection(bool state) {
    SyncWriteHold hold(&vSyncFlag); // RAII lock
    vIsAcceptNewConns = state;
    printf("[LLMServer] SetAcceptNewConnection: %d\n", state);
}

bool LLMServer::GetAcceptNewConnection() const {
    SyncReadHold hold(&vSyncFlag);
    return vIsAcceptNewConns;
}

void LLMServer::RunMainLoop() {
    printf("[LLMServer] Server main loop started.\n");

    DWORD rc;
    WSAEVENT event;

    while (true) {
        if (rc = WaitForSingleObject(vStopEvent, 0) == WAIT_OBJECT_0) {
            printf("[LLMServer] Stop event received. Breaking main loop.\n");
            break;
        }

        if (rc == WAIT_FAILED)
            printf("[LLMServer]: WaitForSingleObject failed\n");

        event = vSocketMgr->GetAcceptEvent();
        
        rc = WSAWaitForMultipleEvents(1, &event, TRUE, 100, FALSE);

        if (rc == WSA_WAIT_TIMEOUT)
            continue;

        if (rc == WSA_WAIT_FAILED) {
            printf("[LLMServer] WSAWaitForMultipleEvents failed\n");
            continue;
        }

        if (rc == WAIT_OBJECT_0) {
            if (!HandleAcceptEvent()) {
                printf("[LLMServer] HandleAcceptEvent failed\n");
                break;
            }
        }
    }

    Finalize();
}

void LLMServer::SetKeepAlive(bool pState){
    vPerformKeepAlive = pState;
}

Protocol LLMServer::GetProtocol() const{
    return Protocol::HTTP_PROTOCOL;
}

bool LLMServer::IsKeepAliveSet() const{
    return vPerformKeepAlive;
}

bool LLMServer::KeepConnection(bool pState){
    return vKeepConnection;
}

bool LLMServer::KeepConnection() const{
    return vKeepConnection;
}

bool LLMServer::HandleAcceptEvent() {
    if (!vSocketMgr->IsAcceptEventOccured()) {
        return true;
    }

    ConnectionInfo clientInfo = {};
    ConnectionContext *cc;

    bool acceptSuccess = vSocketMgr->AcceptConnection(
        &clientInfo,
        GetAcceptNewConnection(),
        IsKeepAliveSet()  // keepAlive
    );
    
     if (!vSocketMgr->SetAcceptEvent()) {
         closesocket(clientInfo.uSocket);
         return false;
     }

    if (!acceptSuccess) {
        printf("[LLMServer] AcceptConnection failed.\n");
        return true;
    }

    // Assign connection ID
    clientInfo.uCCId = vConnectionContextCount++;

    // Create connection context
    cc = this->CreateConnectionContext(clientInfo, this);

    if (!cc) {
        
        //stop the application
        printf("[LLMServer] CreateConnectionContext failed.\n");
        closesocket(clientInfo.uSocket);
        return true;
    }

    cc->SetKeepConnectionAlive(KeepConnection());

    printf("Client IP: %s\n", cc->GetClientInfo().uIP.c_str());
    printf("Client port %d\n", cc->GetClientInfo().uPort);

    // Bind socket to IOCP using connection context as key
    if (CreateIoCompletionPort(
            (HANDLE)cc->GetClientInfo().uSocket,
            vIOCompletionPort,
            (ULONG_PTR)cc,
            0) == NULL)
    {
        printf("[LLMServer] CreateIoCompletionPort failed: %lu\n", GetLastError());
        closesocket(cc->GetClientInfo().uSocket);
        delete cc;
        return false;
    }

    // Post a dummy IO completion packet to trigger worker thread to continue processing
    if (!PostQueuedCompletionStatus(vIOCompletionPort, 0, (ULONG_PTR)cc, NULL)) {
        DWORD err = GetLastError();
        printf("[LLMServer] PostQueuedCompletionStatus failed: %lu\n", err);
        // if (gConnectorErrFunc) gConnectorErrFunc(err);
        closesocket(clientInfo.uSocket);
        delete cc;
        return false;
    }

    // Connection accepted and queued for worker thread
    return true;    

}

ConnectionContext* LLMServer::CreateConnectionContext(const ConnectionInfo& clientInfo, LLMServer * pServer) {
    return new ConnectionContext(clientInfo, pServer);
}

HANDLE LLMServer::GetIOCP() const{
    return vIOCompletionPort;
}

HANDLE LLMServer::GetStopEvent() const {
    return vStopEvent;
}