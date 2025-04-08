// pch.hpp
#pragma once

#define _WINSOCKAPI_
//This ensures windows.h doesn’t bring in winsock.h (which conflicts with winsock2.h we're using).

#include <winsock2.h>
// Use winsock2.h (not winsock.h) for modern socket APIs like WSASocket, WSAAccept, and IOCP required by our async Windows server.

#include <ws2tcpip.h>
// Include ws2tcpip.h for modern TCP/IP features like IPv6, inet_pton, getaddrinfo, and advanced socket options via setsockopt.

#include <windows.h>
// Include windows.h for core Windows API functions like handles, events, threads, and system calls used in our server. CreateEvent, WaitForSingleObject