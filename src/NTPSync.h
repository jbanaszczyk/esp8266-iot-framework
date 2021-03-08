#pragma once

#include <Arduino.h>
#include <memory>
#include <list>

class INTPSync {
public:
	virtual bool isSynced() const = 0;

	virtual void clrSynced() = 0;

	virtual time_t getNow() = 0;

	virtual time_t getStartTime() const = 0;

	virtual time_t getLastSyncTime() const = 0;

	virtual void setFormat(const char *format, size_t bufferSize) = 0;

	virtual std::unique_ptr<char[]> timeStr(time_t someTime) = 0;

	virtual void addSyncCallback(std::function<void()> cb) = 0;
};

class NTPSync : public INTPSync {
	bool synced = false;
	time_t startTime = 0;
	time_t lastSyncTime = 0;
	String timeFormat;
	size_t strftimeBufferSize;

public:
	NTPSync();

	std::list<std::function<void()>> syncCallbacks{};

	void addSyncCallback(std::function<void()> cb) override;

	void clrSynced() override { synced = false; }

	time_t getNow() override;

	time_t getStartTime() const override { return startTime; }

	time_t getLastSyncTime() const override { return lastSyncTime; }

	bool isSynced() const override;

	void setFormat(const char *format, size_t bufferSize) override;

	std::unique_ptr<char[]> timeStr(time_t someTime) override;
};

INTPSync *getNTPSync();
