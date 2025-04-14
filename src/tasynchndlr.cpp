#include "tasynchndlr.hpp"

TAsyncHndlr* TAsyncHndlr::s_instance = nullptr;

TAsyncHndlr::TAsyncHndlr()
    : vReqQueue(nullptr),
      vReqThStartEvent(nullptr),
      vReqThStopEvent(nullptr),
      vPrepareShutdownEvent(nullptr),
      vReqThread(nullptr),
      vThreadPoolHndlr(nullptr),
      vReqProcessor(nullptr),
      vThData(nullptr)
{}

TAsyncHndlr& TAsyncHndlr::Instance() {
    if (!s_instance) {
        s_instance = new TAsyncHndlr();
    }
    return *s_instance;
}


TAsyncHndlr::~TAsyncHndlr() {
    Finalize();
}

void TAsyncHndlr::Initialize(TReqProcessor* pReqProcessor, uint16_t pThreadCount) {
    // 1. Save the processor
    vReqProcessor = pReqProcessor;

    // 2. Init thread pool
    InitializeThreadPoolHndlr(pThreadCount);

    // 3. Init main queue
    vReqQueue = new SyncQueue<void *>();

    // 4. Create event handles
    vReqThStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    vReqThStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    vPrepareShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // 5. Prepare shared thread data
    vThData = new tThreadData();
    vThData->uEvent[0] = vReqThStartEvent;
    vThData->uEvent[1] = vReqThStopEvent;
    vThData->uEvent[2] = vPrepareShutdownEvent;
    vThData->uReqProcessor = vReqProcessor;
    vThData->uAsyncHndlr = this;

    // 6. Start thread
    vReqThread = CreateThread(NULL, 0, ProcessQRequest, vThData, 0, NULL);
}
void TAsyncHndlr::Finalize() {
    SetEvent(vReqThStopEvent); // signal shutdown
    WaitForSingleObject(vReqThread, INFINITE);
    CloseHandle(vReqThread);

    CloseHandle(vReqThStartEvent);
    CloseHandle(vReqThStopEvent);
    CloseHandle(vPrepareShutdownEvent);

    FinalizeThreadPoolHndlr();

    delete vThData;
    vThData = nullptr;
    
    delete vReqQueue;
    vReqQueue = nullptr;
}

DWORD WINAPI TAsyncHndlr::ProcessQRequest(LPVOID lpParam) {
    auto* thdata = (tThreadData*)lpParam;
    auto* async = thdata->uAsyncHndlr;

    while (true) {
        DWORD wr = WaitForMultipleObjects(3, thdata->uEvent, FALSE, INFINITE);

        if (wr == WAIT_OBJECT_0) {
            void* cc = nullptr;
            while ((cc = async->GetRequest()) != nullptr) {
                auto* tpdata = new tThreadPoolData();
                tpdata->uProcessor = thdata->uReqProcessor;
                tpdata->uData = cc;

                async->GetThreadPoolHndlr()->ExecuteRequest(tpdata);
            }
        }
        else if (wr == WAIT_OBJECT_0 + 1) {
            delete thdata;
            return 0;
        }
    }
}

void TAsyncHndlr::InitializeThreadPoolHndlr(uint16_t pThreadCount) {
    vThreadPoolHndlr = new TThreadPoolHndlr();
    vThreadPoolHndlr->Initialize(pThreadCount);
}

void TAsyncHndlr::FinalizeThreadPoolHndlr() {
    vThreadPoolHndlr->Finalize();
    delete vThreadPoolHndlr;
    vThreadPoolHndlr = nullptr;
}

void TAsyncHndlr::Write(void* pRequest) {
    {
        SyncWriteHold lock(&vAsyncSyncFlag); // ensure thread-safe
        vReqQueue->Push(pRequest);
    }
    SetEvent(vReqThStartEvent); // wake dispatcher thread
}

void* TAsyncHndlr::GetRequest() {
    void* msg = nullptr;
    {
        SyncWriteHold lock(&vAsyncSyncFlag);
        vReqQueue->Pop(msg);
    }
    return msg;
}

TThreadPoolHndlr* TAsyncHndlr::GetThreadPoolHndlr() {
    return vThreadPoolHndlr;
}
void TAsyncHndlr::StartQProcessing() {
    SetEvent(vReqThStartEvent);
}
void TAsyncHndlr::PauseQProcessing() {
    ResetEvent(vReqThStartEvent); // optional, if you use pause/resume semantics
}
void TAsyncHndlr::ResumeQProcessing() {
    SetEvent(vReqThStartEvent);
}
void TAsyncHndlr::StopQProcessing() {
    SetEvent(vReqThStopEvent);
}
void TAsyncHndlr::StartPrepareForShutdown() {
    SetEvent(vPrepareShutdownEvent);
}


