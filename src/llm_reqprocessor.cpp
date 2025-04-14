#include "llm_reqprocessor.hpp"

void LLMReqProcessor::ProcessRequest(void* pData) {
    ConnectionContext* cc = static_cast<ConnectionContext*>(pData);

    // Write response body first
    const char* msg = "Mare jigar ka tokda";
    size_t msgLen = strlen(msg);

    // Prepare body buffer
    cc->ExpandBuffer(cc->GetResponseBody(), msgLen + 1);
    memcpy(cc->GetResponseBody()->uBuffer, msg, msgLen);
    cc->GetResponseBody()->uBuffer[msgLen] = '\0';
    cc->GetResponseBody()->uUtilSize = msgLen;

    // Build header with correct Content-Length
    char header[256];
    int headerLen = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n", msgLen);

    // Prepare header buffer
    cc->ExpandBuffer(cc->GetResponseHeader(), headerLen + 1);
    memcpy(cc->GetResponseHeader()->uBuffer, header, headerLen);
    cc->GetResponseHeader()->uBuffer[headerLen] = '\0';
    cc->GetResponseHeader()->uUtilSize = headerLen;

    cc->ResponseReady();
}
