#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Уровни логирования
enum LogLevel
{
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_VERBOSE = 5
};

class Logger
{
public:
	static void begin(unsigned long baudRate = 115200);
	static void setLevel(LogLevel level);
	static LogLevel getLevel();

	// Основные методы логирования
	static void error(const char *tag, const char *format, ...);
	static void warning(const char *tag, const char *format, ...);
	static void info(const char *tag, const char *format, ...);
	static void debug(const char *tag, const char *format, ...);
	static void verbose(const char *tag, const char *format, ...);

// Для удобства: макросы
#define LOG_ERROR(tag, ...) Logger::error(tag, __VA_ARGS__)
#define LOG_WARNING(tag, ...) Logger::warning(tag, __VA_ARGS__)
#define LOG_INFO(tag, ...) Logger::info(tag, __VA_ARGS__)
#define LOG_DEBUG(tag, ...) Logger::debug(tag, __VA_ARGS__)
#define LOG_VERBOSE(tag, ...) Logger::verbose(tag, __VA_ARGS__)

private:
	static LogLevel _level;
	static void log(LogLevel level, const char *tag, const char *format, va_list args);
	static const char *levelToString(LogLevel level);
};

#endif