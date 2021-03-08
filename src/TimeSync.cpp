#include "TimeSync.h"

#include <TZ.h>
#include <coredecls.h>
#include <PolledTimeout.h>

void NTPSync::begin(const char *tz, const char *server1, const char *server2, const char *server3) {
	settimeofday_cb([this]() { synced = true; });
	configTime(tz, server1, server2, server3);
}

void NTPSync::begin(const char *tz) {
	begin(tz, "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
}

void NTPSync::begin() {
	begin(TZ_Etc_UTC);
}

bool NTPSync::isSynced() const {
	return synced;
}

bool NTPSync::waitForSyncResult(unsigned long timeoutLength) const {
	if (synced) {
		return true;
	}

	esp8266::polledTimeout::oneShot timeout(timeoutLength);
	while (!timeout) {
		yield();
		if (synced) {
			return true;
		}
	}
	return false;
}


NTPSync timeSync;
