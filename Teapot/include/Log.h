#pragma once

#ifdef LOG_INFO

#include <chrono>
#include <cstdio>

#define LOG_MSG(a, ...) printMsg(a, "MESSAGE", __VA_ARGS__)
#define LOG_WARNING(a, ...) printMsg(a, "WARNING", __VA_ARGS__)
#define LOG_CRITICAL(a, ...) printMsg(a, "ERROR", __VA_ARGS__)

#ifdef DEBUG_LOG
#define DEBUG_MSG(a, ...) printMsg(a, "DEBUG", __VA_ARGS__);
#else
#define DEBUG_MSG(a, ...)
#endif // !DEBUG_LOG

template<typename ...Args>
inline void printMsg(const char *msg, const char *type, Args... args)
{
	char buffer[365];
	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	ctime_s(buffer, 365, &time);
	buffer[std::strlen(buffer) - 1] = '\0';

	std::printf("[%s][%s] ", buffer, type);
	std::printf(msg, args...);
	std::printf("\n");
}

#else

#define LOG_MSG(a, ...)
#define LOG_WARNING(a, ...)
#define LOG_CRITICAL(a, ...)
#define DEBUG_MSG(a, ...)

#endif // !LOG_INFO