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
		logger->error.printf("Failed to open file for reading: %s\n",fileName->c_str());
		answer = false;
	} else {
		logger->info.print("Starting update..\n");

		size_t fileSize = file.size();

		if (!Update.begin(fileSize)) {
			logger->error.print("Not enough file system space\n");
		} else {
			Update.writeStream(file);

			if (Update.end()) {
				logger->error.print("Successful update\n");
				answer = true;
			} else {
				logger->error.printf("Error: %s\n",String(Update.getError()).c_str());
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
