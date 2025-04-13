#include "tthreadpoolhandler.hpp"



TThreadPoolHndlr::TThreadPoolHndlr()
    : vReqProcessThPool(nullptr)
{}

TThreadPoolHndlr::~TThreadPoolHndlr() {
    Finalize();
}
void TThreadPoolHndlr::Initialize(uint16_t threadCount) {
    assert(vReqProcessThPool == nullptr); // Make sure it’s not already initialized

    vReqProcessThPool = new TThreader(threadCount);
    vReqProcessThPool->SetWorkFunc(&TThreadPoolHndlr::ProcessRequest);
}
void TThreadPoolHndlr::Finalize() {
    if (vReqProcessThPool) {
        delete vReqProcessThPool;
        vReqProcessThPool = nullptr;
    }
}
void TThreadPoolHndlr::ExecuteRequest(void * pMessage) {
    if (!pMessage || !vReqProcessThPool) return;

    vReqProcessThPool->ASyncExecute(pMessage);
}

void TThreadPoolHndlr::ProcessRequest(void * pData) {
    printf("[TThreadPoolHndlr] Processing request...\n");

    tThreadPoolData* tpdata = static_cast<tThreadPoolData*>(pData);
    if (!tpdata->uProcessor || !tpdata->uData) {
        delete tpdata;
        return;
    }

    static_cast<LLMReqProcessor*>(tpdata->uProcessor)->ProcessRequest(tpdata->uData);

    delete tpdata;
}
