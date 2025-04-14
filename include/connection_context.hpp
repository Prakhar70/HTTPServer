#pragma once

#include "common_struct.hpp"
#include "connection_info.hpp"
#include "tasynchndlr.hpp" // gives you access to g_AsyncHndlr
#include "llmserver.hpp"
#include "macros.hpp"

#include <vector>
#include <string>
#include <algorithm> // For std::transform
#include <sstream>   // For std::istringstream

class LLMServer;

class ConnectionContext {
    public:
        explicit ConnectionContext(const ConnectionInfo& info, LLMServer * pServer);
        ~ConnectionContext();
    
        const ConnectionInfo& GetClientInfo() const { return clientInfo; }
        void SetKeepConnectionAlive(bool pState);
        void SetState(MsgState pState);
        MsgState ProcessIO(LLMServer* pServer, DWORD pBytes);
        bool Close();
        void ReIntializeBufs(LLMServer *pServer);
        void RefreshReqResBuffer(LLMServer* pServer);
        bool ReadMessageOnSocket();
        bool ReadHTTPHeader(MsgState &pCurState);
        void ExpandBuffer(tBuffer * pBuffer, unsigned short pReqBytes);
        char * FindEndOfHTTPHeader();
        bool ProcessHTTPHeader();
        void ResetHTTPHeaderInfo();
        void InitializeHTTPHeaderInfo();
        void UpdateHeaderInfo();
        bool RecvMessage(MsgState &pCurState);
        bool ProcessMessage(LLMServer * pServer);
        bool ProcessHTTPMessage(LLMServer * pServer);
        tBuffer* GetResponseHeader();
        tBuffer* GetResponseBody();
        LLMServer* GetServer();
        void ResponseReady();
        bool SendHeader(MsgState& pCurState);
        bool SendBody(MsgState& pCurState);
        bool SendMessageOnSocket();


    private:
        ConnectionInfo clientInfo;
        bool isKeepAlive;
    
        // Buffers
        tBuffer * vRequest;
        tBuffer * vResponse;
        tBuffer * vReqHeader;
        tBuffer * vRespHeader;

        MsgState vIOMsgState;
        DWORD vPrevIOBytes;
        DWORD vBytesTrnfs;
        WSABUF vWsabuf;
        WSAOVERLAPPED vOverlapped; 
        unsigned short vMsgSize;
        ReqRespType vReqRespType;
        unsigned long vHTTPHeaderLen;
        unsigned long vContentAlreadyRead;
        tHTTPHeaderInfo * vHTTPHeaderInfo;
        std::vector<std::pair<std::string, std::string>> vReqKeyValueList;
        LLMServer * vServer;
 };

 



    