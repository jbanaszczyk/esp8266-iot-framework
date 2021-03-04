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
		reset();
	}
}

void ConfigManager::reset() {
	auto internal = eepromData.getMutableStoredData()->getMutableInternalData();
	internal->setDnsIP(IPAddress());
	internal->setSubnetMask(IPAddress());
	internal->setGatewayIP(IPAddress());
	internal->setDnsIP(IPAddress());
	static_assert(sizeof(ConfigData) == sizeof(configDefaults), "Wrong configDefaults");
	memcpy_P(eepromData.getMutableStoredData()->getMutableConfigData(), &configDefaults, sizeof(ConfigData));
	setDirty();
}

void ConfigManager::saveConfig(ConfigData *configData) {
	memcpy_P(eepromData.getMutableStoredData()->getMutableConfigData(), configData, sizeof(ConfigData));
	setDirty();
}

void ConfigManager::saveEeprom() {
	clrDirty();

	auto control = getMutableEepromData()->getMutableControlData();
	control->setVersion(configVersion);
	control->setChecksum(checksum(eepromData.getStoredData()));

	EEPROM.put(0, eepromData);
	EEPROM.commit();

	if (configSaveCallback != nullptr) {
		configSaveCallback();
	}
}

void ConfigManager::loop() {
	if (isDirty()) {
		saveEeprom();
	}
}

configManagerChecksum ConfigManager::checksumHelper(uint8_t *byteArray, unsigned long length, configManagerChecksum result) {
	for (decltype(length) counter = 0; counter < length; counter++) {
		result = hash(result, *(byteArray++));
	}
	return result;
}

configManagerChecksum ConfigManager::hash(configManagerChecksum value, uint8_t c) {
	value ^= c;
	return value << 1 | value >> 7;
}

ConfigManager *getConfigManager() {
	static ConfigManager configManager = ConfigManager();
	return &configManager;
}
