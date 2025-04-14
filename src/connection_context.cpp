#include "connection_context.hpp"

ConnectionContext::ConnectionContext(const ConnectionInfo& info, LLMServer * pServer)
    : clientInfo(info),
      isKeepAlive(true),
      vIOMsgState(MsgState::STATE_MSG_START),
      vHTTPHeaderLen(0),
      vMsgSize(0),
      vServer(pServer),
      vWsabuf{},
      vReqRespType(ReqRespType::REQUEST),
      vBytesTrnfs(0),
    vContentAlreadyRead(0),
    vOverlapped{},
    vPrevIOBytes(0)
{
    // Allocate request buffer 
    vRequest = new tBuffer();
    vRequest->uBuffer = nullptr;
    vRequest->uBuffSize = 0;
    vRequest->uUtilSize = 0;

    // Allocate response buffer
    

    vResponse = new tBuffer();
    vResponse->uBuffer = new char[RESPONSE_BUFFER_LENGTH];
    vResponse->uBuffSize = RESPONSE_BUFFER_LENGTH;
    vResponse->uUtilSize = 0;


    // Allocate request header

    vReqHeader = new tBuffer();
    vReqHeader->uBuffer = new char[INITIAL_HEADER_LENGTH];
    vReqHeader->uBuffSize = INITIAL_HEADER_LENGTH;
    vReqHeader->uUtilSize = 0;

    // Allocate response header

    vRespHeader = new tBuffer();
    vRespHeader->uBuffer = new char[INITIAL_HEADER_LENGTH];
    vRespHeader->uBuffSize = INITIAL_HEADER_LENGTH;
    vRespHeader->uUtilSize = 0;

    // HTTP Header info structure which contains information about HTTP method, version, etc.
    vHTTPHeaderInfo = new tHTTPHeaderInfo();
}

ConnectionContext::~ConnectionContext(){
    
    if(vHTTPHeaderInfo){
        delete vHTTPHeaderInfo;
        vHTTPHeaderInfo = nullptr;
    }
        
    if(vRequest->uBuffSize != 0){
        delete[] vRequest->uBuffer;
    }
    delete (vRequest);

    if(vResponse->uBuffSize != 0){
        delete[] vResponse->uBuffer;
    }
    delete vResponse;

    if(vReqHeader->uBuffSize != 0){
        delete[] vReqHeader->uBuffer;
    }
    delete vReqHeader;

    if(vRespHeader->uBuffSize != 0){
        delete[] vRespHeader->uBuffer;
    }
    delete vRespHeader;
}

void ConnectionContext::InitializeHTTPHeaderInfo() {

    // Sanity check: HTTP/1.0 or HTTP/1.1 must be present
    if (strstr(vReqHeader->uBuffer, "HTTP/1") == nullptr) {
        // HTTP version missing → reject
        return;
    }

    // Temporarily null-terminate at the end of header to safely tokenize
    char backupChar = vReqHeader->uBuffer[vHTTPHeaderLen];
    vReqHeader->uBuffer[vHTTPHeaderLen] = '\0';

    std::string_view headerView(vReqHeader->uBuffer);
    std::istringstream stream(std::string(headerView.substr(0, vHTTPHeaderLen)));

    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (firstLine) {
            // Like ProcessFirstLine()
            std::istringstream lineStream(line);
            std::string part1, part2, part3;
            lineStream >> part1 >> part2 >> part3;

            if (part1.rfind("HTTP/", 0) == 0) {
                vHTTPHeaderInfo->uHTTPVersion = part1;
                vHTTPHeaderInfo->uStatusCode = std::stoul(part2);
                vHTTPHeaderInfo->uStatusCodeDesc = part3;
            } else {
                vHTTPHeaderInfo->uMethod = part1;
                vHTTPHeaderInfo->uUri = part2;
                vHTTPHeaderInfo->uHTTPVersion = part3;
            }

            firstLine = false;
        } else {
            // Like AddKeyValueToList()
            auto colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            while (!value.empty() && value.front() == ' ') value.erase(value.begin());

            vReqKeyValueList.emplace_back(std::move(key), std::move(value));
        }
    }
    UpdateHeaderInfo();

    vReqHeader->uBuffer[vHTTPHeaderLen] = backupChar;
}
void ConnectionContext::UpdateHeaderInfo() {
    for (const auto& [key, value] : vReqKeyValueList) {
        std::string lowerKey = key;
        std::string lowerValue = value;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

        if (lowerKey == "unicode" && lowerValue == "yes") {
            vHTTPHeaderInfo->uUnicode = true;
        } else if (lowerKey == "content-type" && lowerValue.find("utf-16") != std::string::npos) {
            vHTTPHeaderInfo->uUnicode = true;
        } else if (lowerKey == "encrypted" && lowerValue == "yes") {
            vHTTPHeaderInfo->uEncrypt = true;
        } else if (lowerKey == "compressed" && lowerValue == "yes") {
            vHTTPHeaderInfo->uCompress = true;
        } else if (lowerKey == "accept-encoding" && lowerValue.find("gzip") != std::string::npos) {
            vHTTPHeaderInfo->uCompressResp = true;
        } else if (lowerKey == "content-length") {
            try {
                vHTTPHeaderInfo->uContentLen = std::stoul(lowerValue);
                vHTTPHeaderInfo->uContentLenPresent = true;
            } catch (...) {
                // Invalid number — ignore
            }
        } else if (lowerKey == "connection") {
            if (lowerValue == "keep-alive") {
                vHTTPHeaderInfo->uKeepAlive = Trival::IS_TRUE;
            } else {
                vHTTPHeaderInfo->uKeepAlive = Trival::IS_FALSE;
                SetKeepConnectionAlive(false);
            }
        }
    }
}

bool ConnectionContext::ProcessMessage(LLMServer * pServer){
    ExpandBuffer(vResponse, RESPONSE_BUFFER_LENGTH);
    vResponse->uUtilSize = 0;
    return ProcessHTTPMessage(pServer);
}

bool ConnectionContext::SendHeader(MsgState& pCurState){
    vBytesTrnfs += vPrevIOBytes;
    vPrevIOBytes =0;
    bool result = false;
    if(vMsgSize == vBytesTrnfs){
        vIOMsgState = MsgState::STATE_SEND_MESSAGE;
        pCurState=vIOMsgState;
        vMsgSize = vResponse->uUtilSize;
        vBytesTrnfs = 0;
        vPrevIOBytes = 0;
        return true;
    }
    vWsabuf.len = vMsgSize - vBytesTrnfs;
    vWsabuf.buf=vRespHeader->uBuffer + vBytesTrnfs;
    result = SendMessageOnSocket();
    if(result == false){
        vIOMsgState = MsgState::STATE_IO_ERROR;
        pCurState=MsgState::STATE_IO_ERROR;
    }
    return false;
} 

bool ConnectionContext::SendBody(MsgState& pCurState){
    vBytesTrnfs += vPrevIOBytes;
    vPrevIOBytes =0;
    bool result = false;
    if(vMsgSize == vBytesTrnfs){
        vIOMsgState = MsgState::STATE_MSG_END;
        pCurState=vIOMsgState;
        return true;
    }
    vWsabuf.len = vMsgSize - vBytesTrnfs;
    vWsabuf.buf=vResponse->uBuffer + vBytesTrnfs;
    result = SendMessageOnSocket();
    if(result == false){
        vIOMsgState = MsgState::STATE_IO_ERROR;
        pCurState=MsgState::STATE_IO_ERROR;
    }
    return false;
}  

bool ConnectionContext::SendMessageOnSocket(){
    unsigned long bytes = 0;
    unsigned long flag = 0;
    int rc = 0;
    int err = 0;

    memset(&vOverlapped, 0, sizeof(WSAOVERLAPPED));

    rc = WSASend(clientInfo.uSocket, &vWsabuf, 1, &bytes, flag, &vOverlapped, NULL);

    if(rc != 0){
        err = WSAGetLastError();
    }

    if(err == WSA_IO_PENDING){
        return true;
    }

    if(err == SOCKET_ERROR && err != WSA_IO_PENDING){
        return false;
    }
    return true;
}

bool ConnectionContext::ProcessHTTPMessage(LLMServer * pServer){

    if (!g_AsyncHndlr) {
        printf("[ConnectionContext] ERROR: g_AsyncHndlr not initialized\n");
        return false;
    }

    g_AsyncHndlr->Write(this);

    vIOMsgState = MsgState::STATE_AWAITING_RESPONSE;
    return false;
}

tBuffer* ConnectionContext::GetResponseHeader(){
    return vRespHeader;
}
tBuffer* ConnectionContext::GetResponseBody(){
    return vResponse;
}
void ConnectionContext::ResponseReady(){
    vMsgSize = vRespHeader->uUtilSize;
    vBytesTrnfs = 0;
    vPrevIOBytes = 0;
    SetState(MsgState::STATE_SEND_HEADER);
    PostQueuedCompletionStatus(vServer->GetIOCP(), 0, reinterpret_cast<ULONG_PTR>(this), NULL);
}

LLMServer* ConnectionContext::GetServer(){
    return vServer;
}

bool ConnectionContext::RecvMessage(MsgState &pCurState){
    bool result;

    vBytesTrnfs += vPrevIOBytes;
    vPrevIOBytes = 0;

    if(vMsgSize == vBytesTrnfs){

        vIOMsgState = MsgState::STATE_PROCESS_MESSAGE;
        pCurState = vIOMsgState;

        vRequest->uUtilSize = vBytesTrnfs;
        if(!vHTTPHeaderInfo->uCompress){
            if(vRequest->uUtilSize <= vRequest->uBuffSize){
                //expand the buffer by the given size
                ExpandBuffer(vRequest, vRequest->uUtilSize + 1);
            }
            vRequest->uBuffer[vRequest->uUtilSize] = '\0';
        }
        return true;
    }

    vWsabuf.len = vMsgSize - vBytesTrnfs;

    vWsabuf.buf = vRequest->uBuffer + vBytesTrnfs;

    result = ReadMessageOnSocket();

    if(result == false){
        vIOMsgState = MsgState::STATE_IO_ERROR;
        pCurState = MsgState::STATE_MSG_END;
    }

    return false;
}

void ConnectionContext::ResetHTTPHeaderInfo(){
    vHTTPHeaderInfo->uMethod.clear();
    vHTTPHeaderInfo->uUri.clear();
    vHTTPHeaderInfo->uHTTPVersion.clear();
    vHTTPHeaderInfo->uStatusCodeDesc.clear();
    vHTTPHeaderInfo->uEncrypt = false;
    vHTTPHeaderInfo->uCompress = false;
    vHTTPHeaderInfo->uUnicode = false;
    vHTTPHeaderInfo->uCompressResp = false;
    vHTTPHeaderInfo->uContentLen = 0;
    vHTTPHeaderInfo->uKeepAlive = Trival::UNKNOWN;
}

void ConnectionContext::SetKeepConnectionAlive(bool pState){
    isKeepAlive = pState;
}

void ConnectionContext::SetState(MsgState pState){
    
    if(vIOMsgState == MsgState::STATE_IO_ERROR){
        //TODO : add exception here
        printf("Debug error\n");
        return;
    }
    if(vIOMsgState == MsgState::STATE_MSG_END && pState == MsgState::STATE_MSG_END){
        vIOMsgState = MsgState::STATE_IO_ERROR;
    }else{
        vIOMsgState = pState;
    }

}
void ConnectionContext::ReIntializeBufs(LLMServer * pServer){

    vBytesTrnfs = 0;

    //Reset buffer
    vReqHeader->uUtilSize = 0;

    RefreshReqResBuffer(pServer);

    vIOMsgState = MsgState::STATE_READ_HEADER;

    vMsgSize = INITIAL_HEADER_LENGTH - 1;
}

void ConnectionContext::RefreshReqResBuffer(LLMServer * pServer){
    
    if(vRequest->uBuffSize > REQUEST_BUFFER_LENGTH){
        delete vRequest->uBuffer;
        vRequest->uBuffer = nullptr;
        vRequest->uBuffSize = 0;
        vRequest->uUtilSize = 0;
    }

    if(vResponse->uBuffSize > RESPONSE_BUFFER_LENGTH){
        delete vResponse->uBuffer;
        vResponse->uBuffer = nullptr;
        vResponse->uBuffSize = 0;
        vResponse->uUtilSize = 0;
    }
    return;
}


bool ConnectionContext::ReadMessageOnSocket(){
    unsigned long bytes = 0;
    unsigned long flag = 0;
    int rc = 0;
    int err = 0;

    memset(&vOverlapped, 0, sizeof(WSAOVERLAPPED));

    rc = WSARecv(clientInfo.uSocket, &vWsabuf, 1, &bytes, &flag, &vOverlapped, NULL);

    if(rc != 0){
        err = WSAGetLastError();
    }

    if(err == WSA_IO_PENDING){
        return true;
    }

    if(err == SOCKET_ERROR && err != WSA_IO_PENDING){
        return false;
    }
    return true;
    
}

void ConnectionContext::ExpandBuffer(tBuffer * pBuffer, unsigned short pReqBytes){

    char * temptr;

    if(pReqBytes > pBuffer->uBuffSize){
        char* temptr = new char[pReqBytes]();
        if (!temptr) {
            printf("[ExpandBuffer] ERROR: Out of memory while expanding buffer to %u bytes\n", pReqBytes);
            return;
        }

        if(pBuffer->uBuffer){
            memcpy(temptr, pBuffer->uBuffer, pBuffer->uUtilSize);
            delete[] pBuffer->uBuffer;
        }else{
            pBuffer->uUtilSize = 0;
        }
        pBuffer->uBuffer = temptr;
        pBuffer->uBuffSize = pReqBytes;
    }
}

char * ConnectionContext::FindEndOfHTTPHeader(){
    char * tempbuf;

    vReqHeader->uBuffer[vReqHeader->uUtilSize] = '\0';
    //*(vReqHeader->uBuffer + vReqHeader->uUtilSize) = '\0';

    tempbuf = strstr(vReqHeader->uBuffer, "\r\n\r\n");

    if(tempbuf != nullptr){
        tempbuf += strlen("\r\n\r\n");
    }

    return tempbuf;
}

bool ConnectionContext::ReadHTTPHeader(MsgState &pCurState){

    char * endofheader;

    vBytesTrnfs += vPrevIOBytes;
    vReqHeader->uUtilSize += vPrevIOBytes;

    vPrevIOBytes = 0;

    bool res = false;

    //found the complete header
    if((endofheader = FindEndOfHTTPHeader()) != nullptr){
        
        vHTTPHeaderLen = (endofheader - vReqHeader->uBuffer);
        vContentAlreadyRead = vBytesTrnfs - vHTTPHeaderLen;

        vReqHeader->uUtilSize = vBytesTrnfs;
        vIOMsgState = MsgState::STATE_PROCESS_HEADER;
        pCurState = vIOMsgState;
        return true;
    }

    //bUffer full..
    if(vBytesTrnfs == vMsgSize){
        vReqHeader->uUtilSize = vMsgSize;

        vMsgSize += INITIAL_HEADER_LENGTH;

        ExpandBuffer(vReqHeader, vMsgSize + INCREMENT_MESSAGE_SIZE + 1);
    }

    vWsabuf.len = vMsgSize - vBytesTrnfs;
    vWsabuf.buf = vReqHeader->uBuffer + vBytesTrnfs;

    res = ReadMessageOnSocket();

    if(res == false){
        vIOMsgState = MsgState::STATE_IO_ERROR;
        pCurState = MsgState::STATE_MSG_END;
    }

    return false;
 
}

bool ConnectionContext::ProcessHTTPHeader(){
    ResetHTTPHeaderInfo();
    InitializeHTTPHeaderInfo();
    
    vBytesTrnfs = 0;
    vPrevIOBytes = 0;

    vMsgSize = vHTTPHeaderInfo->uContentLen;

    if(!vMsgSize){
        ExpandBuffer(vRequest, REQUEST_BUFFER_LENGTH);
        vIOMsgState = MsgState::STATE_PROCESS_MESSAGE;
        vRequest->uUtilSize = 0;
    }else{
        if(vMsgSize < vContentAlreadyRead){
            //HandleInvalidResponse();
            return true;
        }
        ExpandBuffer(vRequest, vMsgSize);
        if(vContentAlreadyRead){
            memcpy(vRequest->uBuffer,vReqHeader+vHTTPHeaderLen, vContentAlreadyRead);
            vBytesTrnfs = vContentAlreadyRead;
        }
        vIOMsgState = MsgState::STATE_RECV_MESSAGE;
    }
    return true;
};

MsgState ConnectionContext::ProcessIO(LLMServer* pServer, DWORD pBytes){
    
    MsgState curstate;
    bool rc = false;

    //TODO:UnderstandingPending
    if(!clientInfo.uIsSocketActive){
        //TODO:log error "connection context is not active as it is already shutdown
        return MsgState::STATE_IO_ERROR;
    }

    //TODO:UnderstandingPending
    if(vIOMsgState == MsgState::STATE_MSG_UNKNOWN){
        //TODO debug error
        Close();
        return MsgState::STATE_IO_ERROR;
    
    }   

    vPrevIOBytes = pBytes;
    curstate = vIOMsgState;

    do{
        switch(vIOMsgState){
            case MsgState::STATE_MSG_START: //1
                ReIntializeBufs (pServer);
                curstate = vIOMsgState;
                rc = true;
                break;

            case MsgState::STATE_READ_HEADER://4
                rc = ReadHTTPHeader(curstate);
                break;
            
            case MsgState::STATE_PROCESS_HEADER://5
                rc = ProcessHTTPHeader();
                curstate = vIOMsgState;
                break;
            
            case MsgState::STATE_RECV_MESSAGE://6
                rc = RecvMessage(curstate);
                break;

            case MsgState::STATE_PROCESS_MESSAGE://7
                rc = ProcessMessage(pServer);
                curstate = vIOMsgState;
                break;

            case MsgState::STATE_SEND_HEADER://8
                rc = SendHeader(curstate);
                break;

            case MsgState::STATE_SEND_MESSAGE://9
                rc = SendBody(curstate);
                break;

            case MsgState::STATE_MSG_END://10
                curstate = vIOMsgState;
                rc = false;
                break;
            case MsgState::STATE_AWAITING_RESPONSE://11
                curstate = MsgState::STATE_AWAITING_RESPONSE;
                rc = false;
                break;
        }
    }while(rc == true);

    return curstate;
};

bool ConnectionContext::Close(){

    int err;

    if(clientInfo.uIsSocketActive){
        clientInfo.uIsSocketActive = false;
        err = shutdown(clientInfo.uSocket, SD_RECEIVE);
        err = closesocket(clientInfo.uSocket);
        return true;
    }
    return false;
}