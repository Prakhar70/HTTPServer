#pragma once

#include "tthreadpoolhandler.hpp"
#include "treqprocessor.hpp"
#include "syncflag.hpp"
#include "syncqueue.hpp"
#include <windows.h>

class TAsyncHndlr;
extern TAsyncHndlr* g_AsyncHndlr;

struct tThreadData {
    HANDLE uEvent[3]; // [0] start, [1] stop, [2] prepare shutdown
    TReqProcessor* uReqProcessor;
    TAsyncHndlr* uAsyncHndlr;
};

class TAsyncHndlr {
public:
    TAsyncHndlr();
    ~TAsyncHndlr();

    void Initialize(TReqProcessor* pReqProcessor, uint16_t pThreadCount);
    void Finalize();

    void Write(void* pRequest);
    void WriteFirst(void* pRequest);

    void* GetRequest();
    TThreadPoolHndlr* GetThreadPoolHndlr();

    void StartQProcessing();
    void StopQProcessing();
    void PauseQProcessing();
    void ResumeQProcessing();
    void StartPrepareForShutdown();

private:
    void InitializeThreadPoolHndlr(uint16_t pThreadCount);
    void FinalizeThreadPoolHndlr();

    static DWORD WINAPI ProcessQRequest(LPVOID lpParam);

private:
    SyncFlag vAsyncSyncFlag;

    SyncQueue<void *>* vReqQueue;                  // Use your TSyncQueueList*
    HANDLE vReqThStartEvent;
    HANDLE vReqThStopEvent;
    HANDLE vPrepareShutdownEvent;
    HANDLE vReqThread;

    TThreadPoolHndlr* vThreadPoolHndlr;
    TReqProcessor* vReqProcessor;
    tThreadData* vThData;
};
