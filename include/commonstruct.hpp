#pragma once
#include <string>
#include "commonenum.hpp"

struct tBuffer{
    char * uBuffer;
    unsigned long uBuffSize;
    unsigned long uUtilSize;
};

struct tHTTPHeaderInfo {
    std::string uMethod;
    std::string uUri;
    std::string uHTTPVersion;

    unsigned long uStatusCode = 0;
    std::string uStatusCodeDesc;

    bool uEncrypt = false;
    bool uCompress = false;
    bool uUnicode = false;
    bool uContentLenPresent = false;
    unsigned long uContentLen = 0;

    eTrival uKeepAlive = eTrival::UNKNOWN;

    std::string uCookie;
    std::string uCookie1;

    eHTTPAuthType uAuthType = eHTTPAuthType::HAT_UNKNOWN;
    unsigned long uUnauthCount = 0;

    bool uCompressResp = false;
};
