#pragma once

#include <Arduino.h>
#include <memory>

class INTPSync {
public:
	virtual time_t getNow() = 0;

	virtual void setFormat(const char *format, size_t bufferSize) = 0;

	virtual std::unique_ptr<char[]> timeStr(const time_t &someTime) = 0;
};

class NTPSync : public INTPSync {
	String timeFormat;
	size_t strftimeBufferSize;

public:
	NTPSync();

	time_t getNow() override;

	void setFormat(const char *format, size_t bufferSize) override;

	std::unique_ptr<char[]> timeStr(const time_t &someTime) override;
};

INTPSync *getNTPSync();
