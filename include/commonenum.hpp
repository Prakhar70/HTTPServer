//commonenum.hpp
#pragma once

enum class MsgState {
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

enum class Protocol {
    UNKNOWN_PROTOCOL = -1,
    HTTP_PROTOCOL = 0,
    HTTPS_PROTOCOL = 1
};

enum class ReqRespType{
    REQUEST = 1,
    RESPONSE,
};

enum class Trival{
    UNKNOWN = 0,
    IS_TRUE = 1,
    IS_FALSE = 2
};