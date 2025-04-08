// syncflag.cpp
#include "syncflag.hpp"
#include "pch.hpp"
#include <cstdio>

SyncFlag::SyncFlag() {
    vHoldFlag = 0;
    vHoldCount = 0;
}

void SyncHoldForRead(SyncFlag* pFlag) {
#if !defined(_M_IX86)
#error "Only 32-bit x86 supported in this implementation"
#endif
    __asm {
        mov ebx, pFlag
    GetHold:
        mov ax, 1
        xchg [ebx] SyncFlag.vHoldFlag, ax
        test ax, ax
        jnz Retry
        lock inc [ebx] SyncFlag.vHoldCount
        mov [ebx] SyncFlag.vHoldFlag, ax
        jmp Done
    Retry:
        call SyncSleepForRead
        jmp GetHold
    Done:
    }
}

void SyncReleaseRead(SyncFlag* pFlag) {
#if !defined(_M_IX86)
#error "Only 32-bit x86 supported in this implementation"
#endif
    __asm {
        mov ebx, pFlag
#ifdef _DEBUG
        cmp [ebx] SyncFlag.vHoldCount, 0
        jnz CountOK
        call SyncUnderflow
    CountOK:
#endif
        lock dec [ebx] SyncFlag.vHoldCount
    }
}

void SyncHoldForWrite(SyncFlag* pFlag) {
#if !defined(_M_IX86)
#error "Only 32-bit x86 supported in this implementation"
#endif
    __asm {
        mov ebx, pFlag
    GetHold:
        mov ax, 1
        xchg [ebx] SyncFlag.vHoldFlag, ax
        test ax, ax
        jnz RetryHold
    CountCheck:
        cmp [ebx] SyncFlag.vHoldCount, ax
        jz Done
        call SyncSleepForReadDecay
        jmp CountCheck
    RetryHold:
        call SyncSleepForWrite
        jmp GetHold
    Done:
    }
}

void SyncReleaseWrite(SyncFlag* pFlag) {
#if !defined(_M_IX86)
#error "Only 32-bit x86 supported in this implementation"
#endif
    __asm {
        mov ebx, pFlag
        mov [ebx] SyncFlag.vHoldFlag, 0
    }
}

extern "C" void SyncSleepForRead() {
    Sleep(0);
}

extern "C" void SyncSleepForWrite() {
    Sleep(0);
}

extern "C" void SyncSleepForReadDecay() {
    Sleep(0);
}

extern "C" void SyncUnderflow() {
    printf("[SyncUnderflow] ERROR: Reader count went negative!\n");
    ExitProcess(1);
}

