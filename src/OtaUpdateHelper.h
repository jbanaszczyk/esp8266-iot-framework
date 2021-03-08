#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

class IOtaUpdateHelper {
public:
	virtual ~IOtaUpdateHelper() = default;
	virtual void requestStart(String filename) = 0;
	virtual uint8_t getStatus() const = 0;
	virtual void addScheduler(Scheduler *scheduler) = 0;
};

class OtaUpdateHelper : public IOtaUpdateHelper {

private:
	Scheduler *aScheduler = nullptr;
	bool requestFlag = false;
	uint8_t status = 255;
	Task *tLoop = nullptr;
	void flash();

public:
	OtaUpdateHelper() = default;
	void requestStart(String filename) override;
	uint8_t getStatus() const override;
	void addScheduler(Scheduler *scheduler) override;
};

IOtaUpdateHelper *getOtaUpdateHelper();
