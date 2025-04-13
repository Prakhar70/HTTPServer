#include "LLMReqProcessor.hpp"

void LLMReqProcessor::ProcessRequest(void * pData){

    ConnectionContext* cc = static_cast<ConnectionContext*>(pData);

    // Write custom header

    const char* header =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 13\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n";

    size_t headerLen = strlen(header);

    cc->ExpandBuffer(cc->GetResponseHeader(), headerLen + 1);

    memcpy(cc->GetResponseHeader()->uBuffer, header, headerLen);
    cc->GetResponseHeader()->uBuffer[headerLen] = '\0';
    cc->GetResponseHeader()->uUtilSize = headerLen;
    

    //write response message
    
    const char* msg = "Hello, world!";
    size_t msgLen = strlen(msg);

    // Make sure buffer is large enough
    cc->ExpandBuffer(cc->GetResponseBody(), msgLen + 1); // +1 for null-terminator

    memcpy(cc->GetResponseBody()->uBuffer, msg, msgLen);
    cc->GetResponseBody()->uBuffer[msgLen] = '\0';
    cc->GetResponseBody()->uUtilSize = msgLen;

    cc->ResponseReady();

}