#pragma once

#include "treqprocessor.hpp" // include the interface

class LLMReqProcessor : public TReqProcessor {
public:
    void ProcessRequest(void* ctx) override;
};
