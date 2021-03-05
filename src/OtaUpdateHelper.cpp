#include "OtaUpdateHelper.h"

#include "LittleFS.h"

void OtaUpdateHelper::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		loopTask = new Task(
				0,
				1,
				[this]() -> void {
					flash(filename);
				},
				scheduler,
				requestFlag);
	}
}

void OtaUpdateHelper::requestStart(String filenameIn) {
	status = 254;
	filename = std::move(filenameIn);
	requestFlag = true;
	if (loopTask != nullptr) {
		loopTask->setIterations(1);
		loopTask->enable();
	}
}

void OtaUpdateHelper::loop() {
	if (requestFlag == true) {
		flash(filename);
	}
}

uint8_t OtaUpdateHelper::getStatus() const {
	return status;
}

void OtaUpdateHelper::flash(const String &filename) {
	requestFlag = false;
	bool answer = false;
	File file = LittleFS.open(filename, "r");

	if (!file) {
		Serial.println(PSTR("Failed to open file for reading"));
		answer = false;
	} else {
		Serial.println(PSTR("Starting update.."));

		size_t fileSize = file.size();

		if (!Update.begin(fileSize)) {
			Serial.println(PSTR("Not enough space for update"));
		} else {
			Update.writeStream(file);

			if (Update.end()) {
				Serial.println(PSTR("Successful update"));
				answer = true;
			} else {
				Serial.println(PSTR("Error Occurred: ") + String(Update.getError()));
			}
		}
		file.close();
	}
	status = static_cast<uint8_t>(answer);
}

OtaUpdateHelper otaUpdateHelper;
