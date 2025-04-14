#pragma once

#include "tthreadpoolhandler.hpp"
#include "treqprocessor.hpp"
#include "syncflag.hpp"
#include "syncqueue.hpp"
#include <windows.h>

class TAsyncHndlr;
class TReqProcessor;
class TThreadPoolHndlr;

struct tThreadData {
    HANDLE uEvent[3]; // [0] start, [1] stop, [2] prepare shutdown
    TReqProcessor* uReqProcessor;
    TAsyncHndlr* uAsyncHndlr;
};

class TAsyncHndlr {
public:
    
    static TAsyncHndlr& Instance();

    void Initialize(TReqProcessor* pReqProcessor, uint16_t pThreadCount);
    void Finalize();

    void Write(void* pRequest);

    void* GetRequest();
    TThreadPoolHndlr* GetThreadPoolHndlr();

    void StartQProcessing();
    void StopQProcessing();
    void PauseQProcessing();
    void ResumeQProcessing();
    void StartPrepareForShutdown();

private:
    TAsyncHndlr();
    ~TAsyncHndlr();
    void InitializeThreadPoolHndlr(uint16_t pThreadCount);
    void FinalizeThreadPoolHndlr();
    static DWORD WINAPI ProcessQRequest(LPVOID lpParam);
    SyncFlag vAsyncSyncFlag;

    SyncQueue<void *>* vReqQueue;
    HANDLE vReqThStartEvent;
    HANDLE vReqThStopEvent;
    HANDLE vPrepareShutdownEvent;
    HANDLE vReqThread;

    TThreadPoolHndlr* vThreadPoolHndlr;
    TReqProcessor* vReqProcessor;
    tThreadData* vThData;
    static TAsyncHndlr* s_instance;
};
