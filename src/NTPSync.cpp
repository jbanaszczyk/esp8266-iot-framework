#include "NTPSync.h"

NTPSync::NTPSync() : strftimeBufferSize(80) {
	setFormat("%Y.%m.%d %H.%M.%S", 24);
}

void NTPSync::setFormat(const char *format, size_t bufferSize) {
	timeFormat = format;
	strftimeBufferSize = bufferSize;
}

std::unique_ptr<char[]> NTPSync::timeStr(const time_t &someTime) {
	struct tm *info = localtime(&someTime);
	std::unique_ptr<char[]> buffer(new char[strftimeBufferSize]);
	strftime(buffer.get(), strftimeBufferSize, timeFormat.c_str(), info);
	return buffer;
}

time_t NTPSync::getNow() {
	time_t result = 0;
	time(&result);
	return result;
}

INTPSync *getNTPSync() {
	static NTPSync ntpSync{};
	return &ntpSync;
}
