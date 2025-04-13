#pragma once

class TReqProcessor {
public:
    virtual ~TReqProcessor() = default;

    // Core interface function
    virtual void ProcessRequest(void* ctx) = 0;
};
