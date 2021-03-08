#include "NTPSync.h"

#include <coredecls.h>

NTPSync::NTPSync() : synced(false), startTime(0), lastSyncTime(0), strftimeBufferSize(80) {

	setFormat("%Y.%m.%d %H.%M.%S", 24);

	addSyncCallback(
			[this]() -> void {
				time(&lastSyncTime);
				if (startTime == 0) {
					startTime = lastSyncTime;
				}
				synced = true;
			}
	);

	settimeofday_cb([this]() -> void {
		for (const auto &syncCallback : syncCallbacks) {
			syncCallback();
		}
	});
}

bool NTPSync::isSynced() const {
	return synced;
}

void NTPSync::setFormat(const char *format, size_t bufferSize) {
	timeFormat = format;
	strftimeBufferSize = bufferSize;
}

std::unique_ptr<char[]> NTPSync::timeStr(time_t someTime) {
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

void NTPSync::addSyncCallback(std::function<void()> cb) {
	syncCallbacks.push_back(cb);
}

INTPSync *getNTPSync() {
	static NTPSync ntpSync{};
	return &ntpSync;
}
