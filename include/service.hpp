// service.hpp

#pragma once
#include "LLMServer.hpp"
#include "macros.hpp"

#include <thread>
#include <cstdio>
#include <windows.h>

void runAsService();
void runAsConsoleFallback();
