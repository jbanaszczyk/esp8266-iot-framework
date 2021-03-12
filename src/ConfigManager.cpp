#include <Arduino.h>

#include "ConfigManager.h"

ConfigManager::ConfigManager() {
	eeprom.size(EepromRotate_size);
	eeprom.begin(EepromReservedAreaSize + sizeof(EepromData));
	eeprom.get(EepromReservedAreaSize, eepromData);

	if (eepromData.getControlData().getVersion() != configVersion || checksum(eepromData.getStoredData()) != eepromData.getControlData().getChecksum()) {
		Serial.println(PSTR("EEPROM data invalid"));
		Serial.printf("Version: expected %#08x, was read %#02x\n", configVersion, eepromData.getControlData().getVersion());
		Serial.printf("Checksum  expected %#02x, was read %#02x\n", eepromData.getControlData().getChecksum(), checksum(eepromData.getStoredData()));
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

	eeprom.put(EepromReservedAreaSize, eepromData);
	eeprom.commit();

	if (configSaveCallback != nullptr) {
		configSaveCallback();
	}
}

void ConfigManager::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		tLoop = new Task(
				0,
				1,
				[this]() -> void {
					writeEeprom();
				},
				scheduler,
				dirty);
	}
}

void ConfigManager::setDirty() {
	dirty = true;
	if (tLoop != nullptr) {
		tLoop->setIterations(1);
		tLoop->enable();
	}
}

configManagerChecksum ConfigManager::checksumHelper(configManagerChecksum *byteArray, unsigned long length, configManagerChecksum result) {
	for (decltype(length) counter = 0; counter < length; counter++) {
		result = hash(result, *byteArray);
		byteArray++;
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
