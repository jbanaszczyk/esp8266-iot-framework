
#include "SimpleLogging.h"

size_t SimpleLogging::Logger::LogTarget::write(uint8_t byte) {
	return level <= logger->minimumLevel
	       ? logging->log(level, byte)
	       : 0;
}

size_t SimpleLogging::Logger::LogTarget::write(const uint8_t *buffer, size_t size) {
	return level <= logger->minimumLevel
	       ? logging->log(level, logger->tag, buffer, size)
	       : 0;
}

size_t SimpleLogging::Logger::LogTarget::lazyLog(const std::function<const char *()> &log_creator) {
	return level <= logger->minimumLevel
	       ? logging->lazyLog(level, logger->tag, log_creator)
	       : 0;
}

const char *const SimpleLogging::timeFormat = "%Y.%m.%d %H.%M.%S ";

size_t SimpleLogging::log(SimpleLogging::Level level, const String &tag, const uint8_t *buffer, size_t size) {
	if ((allHandlersMask & level) == 0) {
		return 0;
	}

	static Level lastLevel = NOLOG;
	if (!gotCRLF && lastLevel != level) {
		log(lastLevel, '\n');
	}
	lastLevel = level;

	static char nowString[timeStringSize];
	nowString[0] = '\0';
	String levelName{};

	if (gotCRLF) {
		levelName = levelToPrefix(level);
		time_t now = 0;
		time(&now);
		struct tm *info = localtime(&now);
		strftime(nowString, sizeof(nowString), timeFormat, info);
	}

	for (auto &handler : handlers) {
		if ((handler.mask & level) != 0) {
			if (gotCRLF) {
				handler.stream.write(nowString, strlen(nowString));
				handler.stream.write(levelName.c_str(), levelName.length());
				handler.stream.write(' ');
				handler.stream.write(tag.c_str(), tag.length());
				handler.stream.write(' ');
			}
			handler.stream.write(buffer, size);
		}
	}
	gotCRLF = size >= 1 && buffer[size - 1] == '\n';
	return size;
}

size_t SimpleLogging::log(SimpleLogging::Level level, uint8_t byte) {
	if ((allHandlersMask & level) == 0) {
		return 0;
	}
	for (auto &handler : handlers) {
		if ((handler.mask & level) != 0) {
			handler.stream.write(byte);
		}
	}
	gotCRLF = byte == '\n';
	return 1;
}

size_t SimpleLogging::lazyLog(SimpleLogging::Level level, const String &tag, const std::function<const char *()> &log_creator) {
	if ((allHandlersMask & level) == 0) {
		return 0;
	}
	auto message = log_creator();
	return log(level, tag, reinterpret_cast<const uint8_t *>(message), strlen(message));
}

uint8_t SimpleLogging::makeMask(const SimpleLogging::Level level) {
	uint8_t mask = 0x0;
	while (mask < level) {
		mask = (mask << 1) | uint8_t{0x01};
	}
	return mask;
}

void SimpleLogging::addSpecificHandler(Print &stream, Level level) {
	allHandlersMask |= level;
	handlers.emplace_front(stream, level);
}

void SimpleLogging::addHandler(Print &stream, SimpleLogging::Level level) {
	auto mask = makeMask(level);
	allHandlersMask |= mask;
	handlers.emplace_front(stream, mask);
}

void SimpleLogging::removeHandlers(Print &stream) {
	handlers.remove_if([&stream](const LogHandler &handler) {
		return &(handler.stream) == &stream;
	});
	allHandlersMask = 0;
	for (auto &handler : handlers) {
		allHandlersMask |= handler.mask;
	}
}

std::pair<SimpleLogging::Level, const char *>SimpleLogging::LevelNames[] = {
		std::make_pair(SimpleLogging::CRITICAL, "CRITICAL"),
		std::make_pair(SimpleLogging::ERROR, "ERROR"),
		std::make_pair(SimpleLogging::WARNING, "WARNING"),
		std::make_pair(SimpleLogging::NOTICE, "NOTICE"),
		std::make_pair(SimpleLogging::INFO, "INFO"),
		std::make_pair(SimpleLogging::DEBUG, "DEBUG"),
		std::make_pair(SimpleLogging::TRACE, "TRACE"),
};

std::pair<SimpleLogging::Level, const char *>SimpleLogging::LevelPrefixes[] = {
		std::make_pair(SimpleLogging::CRITICAL, "[CRITICAL]"),
		std::make_pair(SimpleLogging::ERROR, "[ERROR   ]"),
		std::make_pair(SimpleLogging::WARNING, "[WARNING ]"),
		std::make_pair(SimpleLogging::NOTICE, "[NOTICE  ]"),
		std::make_pair(SimpleLogging::INFO, "[INFO    ]"),
		std::make_pair(SimpleLogging::DEBUG, "[DEBUG   ]"),
		std::make_pair(SimpleLogging::TRACE, "[TRACE   ]"),
		std::make_pair(SimpleLogging::NOLOG, "[NOLOG   ]"),
};

SimpleLogging::Level SimpleLogging::stringToLevel(const String &levelName) {
	for (const auto &levelAndName : LevelNames) {
		if (levelName.equalsIgnoreCase(levelAndName.second)) {
			return levelAndName.first;
		}
	}
	return NOLOG;
}

String SimpleLogging::levelToString(SimpleLogging::Level level) {
	for (const auto &levelAndName : LevelNames) {
		if (level == levelAndName.first) {
			return levelAndName.second;
		}
	}
	return "???";
}

String SimpleLogging::levelToPrefix(SimpleLogging::Level level) {
	for (const auto &levelAndName : LevelPrefixes) {
		if (level == levelAndName.first) {
			return levelAndName.second;
		}
	}
	return "???";
}

SimpleLogging::Logger *SimpleLogging::getLogger(const char *tag, SimpleLogging::Level minimumLevel) {
	for (const auto &tagAndLogger : tagsAndLoggers) {
		if (strcmp(tag, tagAndLogger.first.c_str()) == 0) {
			return tagAndLogger.second;
		}
	}
	auto *logger = new Logger(tag, minimumLevel, this);
	tagsAndLoggers.emplace_back(String(tag), logger);
	return logger;
}

SimpleLogging Logging{};
