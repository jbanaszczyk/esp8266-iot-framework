#pragma once

#include <Arduino.h>

class OtaUpdateHelper {

private:
	String filename;
	bool requestFlag = false;
	uint8_t status = 255;
	void flash(const String &filename);

public:
	void requestStart(String filename);
	void loop();
	uint8_t getStatus() const;
};

extern OtaUpdateHelper otaUpdateHelper;
