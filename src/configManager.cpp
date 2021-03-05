#include <EEPROM.h>
#include <Arduino.h>

#include "configManager.h"

ConfigManager::ConfigManager() {
	EEPROM.begin(sizeof(EepromData));
	EEPROM.get(0, eepromData);

	if (eepromData.getControlData().getVersion() != configVersion || checksum(eepromData.getStoredData()) != eepromData.getControlData().getChecksum()) {
		Serial.println(PSTR("EEPROM data invalid"));
		Serial.printf("Version %d %d\n", configVersion, eepromData.getControlData().getVersion());
		Serial.printf("Checksum %d %d\n", eepromData.getControlData().getChecksum(), checksum(eepromData.getStoredData()));
		InternalData internalData{};
		saveInternalData(&internalData);
		saveConfigData(&configDefaults);
	}
}

void ConfigManager::saveInternalData(const InternalData *internalData) {
	memcpy_P(eepromData.getMutableStoredData()->getMutableConfigData(), internalData, sizeof(InternalData));
	setDirty();
}

void ConfigManager::saveConfigData(const ConfigData *configData) {
	memcpy_P(eepromData.getMutableStoredData()->getMutableConfigData(), configData, sizeof(ConfigData));
	setDirty();
}

void ConfigManager::writeEeprom() {
	Serial.printf("[EEPROM] Write %zu bytes\n", sizeof(eepromData));
	dirty = false;

	auto control = getMutableEepromData()->getMutableControlData();
	control->setVersion(configVersion);
	control->setChecksum(checksum(eepromData.getStoredData()));

	EEPROM.put(0, eepromData);
	EEPROM.commit();

	if (configSaveCallback != nullptr) {
		configSaveCallback();
	}
}

void ConfigManager::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		loopTask = new Task(
				0,
				1,
				[this]() -> void {
					writeEeprom();
				},
				scheduler,
				false);
	}
	if (dirty) {
		loopTask->enable();
	}
}

void ConfigManager::setDirty() {
	dirty = true;
	if (loopTask != nullptr) {
		loopTask->setIterations(1);
		loopTask->enable();
	}
}

configManagerChecksum ConfigManager::checksumHelper(configManagerChecksum *byteArray, unsigned long length, configManagerChecksum result) {
	for (decltype(length) counter = 0; counter < length; counter++) {
		result = hash(result, *(byteArray++));
		byteArray ++ ;
	}
	return result;
}

configManagerChecksum ConfigManager::hash(configManagerChecksum value, configManagerChecksum c) {
	value ^= c;
	return value << 1 | value >> 7;
}

ConfigManager *getConfigManager() {
	static ConfigManager configManager = ConfigManager();
	return &configManager;
}
