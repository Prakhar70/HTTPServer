#pragma once

#include "syncqueue.hpp"
#include "syncflag.hpp"
#include <Windows.h>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>


using WorkFunc = void(*)(void*);

struct TThreadControlData;

class TThreader {

public:
    TThreader(WORD pNumThreads, LONG pStackSize = 0);
    ~TThreader();

    void SetWorkFunc(void (*pFunc)(void*));
    void* ASyncExecute(void* pData);
    WorkFunc vWorkFunc;
    SyncQueue<TThreadControlData*>* rThreadQueue;
    
    
private:
    void FinalizeThreads();
    void InitializeThreads(WORD pCount);

    SyncFlag rSyncFlag;
    HANDLE vCloseEvent;
    LONG vStackSize;
    
    WORD rNumThreads;
    static DWORD WINAPI WinThreadFunc(LPVOID lpParam);
    
};

struct TThreadControlData {
    HANDLE uThreadHandle;
    HANDLE uEvents[2]; // [0] = execute event, [1] = close event
    TThreader* uThreader;
    void* uData;
    bool uResult;
};
