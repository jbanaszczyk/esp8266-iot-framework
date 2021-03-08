#include "OtaUpdateHelper.h"

#include "LittleFS.h"

void OtaUpdateHelper::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		aScheduler = scheduler;
		tLoop = new Task(
				0,
				1,
				[this]() -> void {
					flash();
				},
				scheduler,
				requestFlag);
	}
}

void OtaUpdateHelper::requestStart(String filenameIn) {
	status = 254;
	auto *filename = new String(filenameIn);
	requestFlag = true;
	if (tLoop != nullptr) {
		tLoop->setLtsPointer(filename);
		tLoop->restart();
	}
}

uint8_t OtaUpdateHelper::getStatus() const {
	return status;
}

void OtaUpdateHelper::flash() {
	auto *fileName = static_cast<String *>(aScheduler->currentLts());
	requestFlag = false;
	bool answer = false;
	File file = LittleFS.open(*fileName, "r");

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

IOtaUpdateHelper *getOtaUpdateHelper() {
	static OtaUpdateHelper otaUpdateHelper = OtaUpdateHelper();
	return &otaUpdateHelper;
}
