#include "Logger.h"
#include <stdarg.h>

LogLevel Logger::_level = LOG_LEVEL_INFO;

void Logger::begin(unsigned long baudRate)
{
	Serial.begin(baudRate);
	// while (!Serial)
	// 	delay(10);
	LOG_INFO("Logger", "Logger initialized");
}

void Logger::setLevel(LogLevel level)
{
	_level = level;
	LOG_INFO("Logger", "Log level set to %s", levelToString(level));
}

LogLevel Logger::getLevel()
{
	return _level;
}

void Logger::error(const char *tag, const char *format, ...)
{
	if (_level >= LOG_LEVEL_ERROR)
	{
		va_list args;
		va_start(args, format);
		log(LOG_LEVEL_ERROR, tag, format, args);
		va_end(args);
	}
}

void Logger::warning(const char *tag, const char *format, ...)
{
	if (_level >= LOG_LEVEL_WARNING)
	{
		va_list args;
		va_start(args, format);
		log(LOG_LEVEL_WARNING, tag, format, args);
		va_end(args);
	}
}

void Logger::info(const char *tag, const char *format, ...)
{
	if (_level >= LOG_LEVEL_INFO)
	{
		va_list args;
		va_start(args, format);
		log(LOG_LEVEL_INFO, tag, format, args);
		va_end(args);
	}
}

void Logger::debug(const char *tag, const char *format, ...)
{
	if (_level >= LOG_LEVEL_DEBUG)
	{
		va_list args;
		va_start(args, format);
		log(LOG_LEVEL_DEBUG, tag, format, args);
		va_end(args);
	}
}

void Logger::verbose(const char *tag, const char *format, ...)
{
	if (_level >= LOG_LEVEL_VERBOSE)
	{
		va_list args;
		va_start(args, format);
		log(LOG_LEVEL_VERBOSE, tag, format, args);
		va_end(args);
	}
}

void Logger::log(LogLevel level, const char *tag, const char *format, va_list args)
{
	char buffer[256];
	int len = snprintf(buffer, sizeof(buffer), "[%lu] [%s] [%s] ", millis(), levelToString(level), tag);
	vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
	Serial.println(buffer);
}

const char *Logger::levelToString(LogLevel level)
{
	switch (level)
	{
	case LOG_LEVEL_ERROR:
		return "ERROR";
	case LOG_LEVEL_WARNING:
		return "WARN";
	case LOG_LEVEL_INFO:
		return "INFO";
	case LOG_LEVEL_DEBUG:
		return "DEBUG";
	case LOG_LEVEL_VERBOSE:
		return "VERBOSE";
	default:
		return "UNKNOWN";
	}
}