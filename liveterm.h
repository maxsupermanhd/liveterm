#pragma once

#include <string>

#if !defined(unix) && !defined(__unix__) && !defined(__unix)
#define LIVETERM_UNIX 1
#endif

// false - success
bool LivetermInit(void (*Commander)(std::string cmd));

void LivetermSetCommander(void (*Commander)(std::string cmd));

void LivetermSetPromt(std::string p);

void LTPromtUpdate();

void LTPrintf(const char* format, ...);

// false - success
bool LivetermShutdown();
