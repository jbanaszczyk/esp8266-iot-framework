#pragma once

#include <Arduino.h>

class NTPSync {

public:
	void begin(const char *tz);
	void begin(const char *tz, const char *server1, const char *server2 = nullptr, const char *server3 = nullptr);

	bool isSynced() const;
	bool waitForSyncResult(unsigned long timeoutLength = 10000) const;

private :
	bool synced = false;
	static void setTime(const char *tz, const char *server1, const char *server2, const char *server3);
};

extern NTPSync timeSync;
