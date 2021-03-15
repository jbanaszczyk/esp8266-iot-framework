
#pragma once

#include <Arduino.h>
#include <Print.h>
#include <forward_list>
#include <utility>
#include <list>

class SimpleLogging {
public:
	enum Level : uint8_t {
		CRITICAL = 0x1, // 1
		ERROR = CRITICAL << 1, // 2
		WARNING = ERROR << 1, // 4
		NOTICE = WARNING << 1, // 7
		INFO = NOTICE << 1, // 16
		DEBUG = INFO << 1, // 32
		TRACE = DEBUG << 1, // 64
		NOLOG = TRACE << 1 // 128
	};

	SimpleLogging() = default;

	SimpleLogging(const SimpleLogging &) = delete;

	SimpleLogging(const SimpleLogging &&) = delete;

	class Logger {
		const String tag;
		Level minimumLevel;
		SimpleLogging *logging;

	public:
		Logger(const char *tag, Level minimumLevel, SimpleLogging *logging) : tag(tag), minimumLevel(minimumLevel), logging(logging) {}

		Logger(const Logger &) = delete;

		Logger(const Logger &&) = delete;

		void setMinimumLevel(Level minimumLevel) { Logger::minimumLevel = minimumLevel; }

		class LogTarget : public Print {
			Level level;
			Logger *logger;
			SimpleLogging *logging{};
		public:
			LogTarget(const Level level, Logger *logger, SimpleLogging *logging) : level(level), logger(logger), logging(logging) {}

			LogTarget(const LogTarget &) = delete;

			LogTarget(const LogTarget &&) = delete;

			size_t write(uint8_t) override;

			size_t write(const uint8_t *buffer, size_t size) override;

			size_t lazyLog(const std::function<const char *()> &log_creator);
		};

		LogTarget critical{CRITICAL, this, logging};
		LogTarget error{ERROR, this, logging};
		LogTarget warning{WARNING, this, logging};
		LogTarget notice{NOTICE, this, logging};
		LogTarget info{INFO, this, logging};
		LogTarget debug{DEBUG, this, logging};
		LogTarget trace{TRACE, this, logging};
		LogTarget nolog{NOLOG, this, logging};
	};

	Logger *getLogger(const char *tag, Level minimumLevel);

	size_t log(Level level, const String &tag, const uint8_t *buffer, size_t size);

	size_t log(Level level, uint8_t);

	size_t lazyLog(Level level, const String &tag, const std::function<const char *()> &log_creator);

	void addHandler(Print &stream, Level level);

	void addSpecificHandler(Print &stream, Level level);

	void removeHandlers(Print &stream);

private:
	class LogHandler {
	public:
		Print &stream;
		const uint8_t mask;

		explicit LogHandler(Print &stream, uint8_t mask) : stream(stream), mask(mask) {}
	};

	static uint8_t makeMask(Level level);

	std::forward_list<LogHandler> handlers;

	static String levelToString(Level level);

	static String levelToPrefix(Level level);

	static Level stringToLevel(const String &levelName);

	std::list<std::pair<String, Logger *> > tagsAndLoggers{};

	static std::pair<SimpleLogging::Level, const char *> LevelNames[];
	static std::pair<SimpleLogging::Level, const char *> LevelPrefixes[];
	bool gotCRLF = true;
	uint8_t allHandlersMask = 0;

	static const int timeStringSize = 30;

	static const char *const timeFormat;
};

extern SimpleLogging Logging;
