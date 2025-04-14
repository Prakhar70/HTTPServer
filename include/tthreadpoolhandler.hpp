#pragma once

#include "tthreader.hpp"  // Your thread pool
#include "common_struct.hpp" // Include the header where tThreadPoolData is defined
#include "llm_reqprocessor.hpp"

#include <cstdint>
#include <cassert>

class TThreadPoolHndlr {
public:
    TThreadPoolHndlr();
    ~TThreadPoolHndlr();

    void Initialize(uint16_t threadCount);
    void Finalize();
    void ExecuteRequest(void * pMessage);     // Enqueue connection context
    static void ProcessRequest(void * pData); // Actual worker

private:
    TThreader* vReqProcessThPool; // Internal thread pool
};
