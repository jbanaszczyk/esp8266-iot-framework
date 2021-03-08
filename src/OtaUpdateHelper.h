#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

class OtaUpdateHelper {

private:
	String filename;
	bool requestFlag = false;
	uint8_t status = 255;
	void flash(const String &filename);
	Task *tLoop = nullptr;

public:
	void requestStart(String filename);
	void loop();
	uint8_t getStatus() const;
	void addScheduler(Scheduler *scheduler);
};

extern OtaUpdateHelper otaUpdateHelper;
