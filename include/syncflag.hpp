// syncflag.hpp
#pragma once

struct SyncFlag {
    volatile short vHoldFlag;
    volatile short vHoldCount;

    SyncFlag();
};

extern "C" void SyncHoldForRead(SyncFlag* pFlag);
extern "C" void SyncReleaseRead(SyncFlag* pFlag);
extern "C" void SyncHoldForWrite(SyncFlag* pFlag);
extern "C" void SyncReleaseWrite(SyncFlag* pFlag);

extern "C" void SyncSleepForRead();
extern "C" void SyncSleepForWrite();
extern "C" void SyncSleepForReadDecay();
extern "C" void SyncUnderflow();

class SyncReadHold {
public:
    explicit SyncReadHold(SyncFlag* f) : flag(f) { SyncHoldForRead(flag); }
    ~SyncReadHold() { SyncReleaseRead(flag); }
private:
    SyncFlag* flag;
};

class SyncWriteHold {
public:
    explicit SyncWriteHold(SyncFlag* f) : flag(f) { SyncHoldForWrite(flag); }
    ~SyncWriteHold() { SyncReleaseWrite(flag); }
private:
    SyncFlag* flag;
};