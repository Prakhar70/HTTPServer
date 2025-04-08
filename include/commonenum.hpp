//commonenum.hpp
#pragma once

enum class eMsgState {
    STATE_MSG_UNKNOWN = 0,
    STATE_MSG_START,
    STATE_READ_PROTOCOL,
    STATE_PROCESS_PROTOCOL,
    STATE_READ_HEADER,
    STATE_PROCESS_HEADER,
    STATE_RECV_MESSAGE,
    STATE_PROCESS_MESSAGE,
    STATE_SEND_HEADER,
    STATE_SEND_MESSAGE,
    STATE_MSG_END,
    STATE_AWAITING_RESPONSE,
    STATE_IO_ERROR
};

enum class eProtocol {
    UNKNOWN_PROTOCOL = -1,
    HTTP_PROTOCOL = 0,
    HTTPS_PROTOCOL = 1
};

enum class eReqRespType{
    REQUEST = 1,
    RESPONSE,
};

enum class eTrival{
    UNKNOWN = 0,
    True = 1,
    False = 2
};

enum class eHTTPAuthType{
    HAT_UNKNOWN = 0,
    HAT_BASIC,
    HAT_DIGEST,
    HAT_NTLM
};