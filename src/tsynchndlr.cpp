#include "tasynchndlr.hpp"

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
    vThData = (tThreadData*)calloc(1, sizeof(tThreadData));
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

    delete vReqQueue;
    vReqQueue = nullptr;
}

DWORD WINAPI TAsyncHndlr::ProcessQRequest(LPVOID lpParam) {
    auto* thdata = (tThreadData*)lpParam;
    auto* async = thdata->uAsyncHndlr;

    while (true) {
        DWORD wr = WaitForMultipleObjects(3, thdata->uEvent, FALSE, INFINITE);

        if (wr == WAIT_OBJECT_0) {
            void* msg = nullptr;
            while ((msg = async->GetRequest()) != nullptr) {
                auto* tpdata = (tThreadPoolData*)calloc(1, sizeof(tThreadPoolData));
                tpdata->uProcessor = thdata->uReqProcessor;
                tpdata->uData = ((VoidElem*)msg)->uInfo;

                async->GetThreadPoolHndlr()->ExecuteRequest(tpdata);
                delete (VoidElem*)msg;
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
