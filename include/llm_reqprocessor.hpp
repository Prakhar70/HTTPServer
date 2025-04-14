#pragma once

#include "treqprocessor.hpp" // include the interface
#include "connection_context.hpp"

class LLMReqProcessor : public TReqProcessor {
public:
    void ProcessRequest(void* ctx) override;
};
