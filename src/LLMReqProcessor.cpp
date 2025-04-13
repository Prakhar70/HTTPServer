#include "LLMReqProcessor.hpp"

void LLMReqProcessor::ProcessRequest(void * pData){

    ConnectionContext* cc = (ConnectionContext*)pData;
    cc->GetResponseHeader()->uBuffer =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 13\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n";
    cc->GetResponseHeader()->uBuffSize=strlen(cc->GetResponseHeader()->uBuffer);
    cc->GetResponseHeader()->uUtilSize=strlen(cc->GetResponseHeader()->uBuffer);

    cc->GetResponseBody()->uBuffer="Hello, world!";
    cc->GetResponseBody()->uBuffSize=strlen(cc->GetResponseBody()->uBuffer);
    cc->GetResponseBody()->uUtilSize=strlen(cc->GetResponseBody()->uBuffer);

    cc->ResponseReady();

}