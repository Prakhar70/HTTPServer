#pragma once
#include <string>
#include "common_enum.hpp"

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

    Trival uKeepAlive = Trival::UNKNOWN;

    std::string uCookie;
    std::string uCookie1;

    unsigned long uUnauthCount = 0;

    bool uCompressResp = false;
};

struct tThreadPoolData{
    void * uProcessor;
    void * uData;
};
