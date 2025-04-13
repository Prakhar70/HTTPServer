#include "tthreader.hpp"


DWORD WINAPI TThreader::WinThreadFunc(LPVOID lpParam) {
    TThreadControlData* tcd = static_cast<TThreadControlData*>(lpParam);
    HANDLE* events = tcd->uEvents;

    while (true) {
        DWORD waitResult = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (waitResult == WAIT_OBJECT_0 + 1) break; // Close
        if (waitResult == WAIT_OBJECT_0) {
            if (tcd->uThreader->vWorkFunc)
                tcd->uThreader->vWorkFunc(tcd->uData);

            ResetEvent(tcd->uEvents[0]);
            tcd->uThreader->rThreadQueue->Push(tcd);
        }
    }
    return 0;
}


TThreader::TThreader(WORD pNumThreads, LONG pStackSize)
    : rNumThreads(0), vStackSize(pStackSize), vWorkFunc(nullptr) {
    rThreadQueue = new SyncQueue<TThreadControlData*>();
    vCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    assert(vCloseEvent != NULL);
    InitializeThreads(pNumThreads);
}

TThreader::~TThreader() {
    SetEvent(vCloseEvent);
    FinalizeThreads();
    CloseHandle(vCloseEvent);
    delete rThreadQueue;
    rThreadQueue = nullptr;
}

void TThreader::SetWorkFunc(void (*pFunc)(void*)) {
    vWorkFunc = pFunc;
}

void* TThreader::ASyncExecute(void* pData) {
    TThreadControlData* tcd = nullptr;
    while (!rThreadQueue->Pop(tcd)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    tcd->uData = pData;
    SetEvent(tcd->uEvents[0]);
    return tcd;
}

void TThreader::InitializeThreads(WORD pCount) {
    for (WORD i = 0; i < pCount; ++i) {
        TThreadControlData* tcd = new TThreadControlData();
        tcd->uEvents[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
        tcd->uEvents[1] = vCloseEvent;
        tcd->uThreader = this;
        tcd->uResult = false;
        tcd->uThreadHandle = CreateThread(nullptr, vStackSize, WinThreadFunc, tcd, 0, nullptr);

        assert(tcd->uThreadHandle != NULL);

        {
            SyncWriteHold lock(&rSyncFlag);
            ++rNumThreads;
        }
        
        rThreadQueue->Push(tcd);
    }
}

void TThreader::FinalizeThreads() {
    for (WORD i = rNumThreads; i > 0; --i) {
        TThreadControlData* tcd = nullptr;
        while (!rThreadQueue->Pop(tcd)) {
            Sleep(0);
        }
        CloseHandle(tcd->uThreadHandle);
        CloseHandle(tcd->uEvents[0]);
        delete tcd;
    }
    rNumThreads = 0;
}
