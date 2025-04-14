// service.hpp
#pragma once
#include "LLMServer.hpp"

#include <thread>
#include <cstdio>
#include <windows.h>

void RunAsService();
void RunAsConsoleFallback();
