#pragma once

#include "IPAddress.h"
#include "generated/config.h"

typedef uint8_t configManagerChecksum;

class ConfigManager;

ConfigManager *getConfigManager();

class ConfigManager {

public:
	class InternalData {
		uint32_t localIP;
		uint32_t subnetMask;
		uint32_t gatewayIP;
		uint32_t dnsIP;
	public:
		IPAddress getLocalIP() const { return IPAddress(localIP); }
		IPAddress getSubnetMask() const { return IPAddress(subnetMask); }
		IPAddress getGatewayIP() const { return IPAddress(gatewayIP); }
		IPAddress getDnsIP() const { return IPAddress(dnsIP); }
		void setLocalIP(IPAddress localIP) { InternalData::localIP = localIP.v4(); getConfigManager()->setDirty(); }
		void setSubnetMask(IPAddress subnetMask) { InternalData::subnetMask = subnetMask.v4(); getConfigManager()->setDirty(); }
		void setGatewayIP(IPAddress gatewayIP) { InternalData::gatewayIP = gatewayIP.v4(); getConfigManager()->setDirty(); }
		void setDnsIP(IPAddress dnsIP) { InternalData::dnsIP = dnsIP.v4(); getConfigManager()->setDirty(); }
	};

	class StoredData {
		InternalData internalData;
		ConfigData configData;
	public:
		InternalData *getMutableInternalData() { return &internalData; }
		ConfigData *getMutableConfigData() { return &configData; }
		const InternalData &getInternalData() const { return internalData; }
		const ConfigData &getConfigData() const { return configData; }
	};

	class ControlData {
		configManagerChecksum checksum;
		uint32_t version;
	public:
		configManagerChecksum getChecksum() const { return checksum; }
		void setChecksum(configManagerChecksum checksum) { ControlData::checksum = checksum; }
		uint32_t getVersion() const { return version; }
		void setVersion(const uint32_t version) { ControlData::version = version; }
	};

	class EepromData {
		StoredData storedData;
		ControlData controlData;
	public:
		StoredData *getMutableStoredData() { return &storedData; }
		ControlData *getMutableControlData() { return &controlData; }
		const StoredData &getStoredData() const { return storedData; }
		const ControlData &getControlData() const { return controlData; }
	};

	const EepromData &getEepromData() const { return eepromData; }
	EepromData *getMutableEepromData() { return &eepromData; }

	ConfigManager();

	void reset();

	void saveConfig(ConfigData *configData);

	void saveEeprom();

	void loop();

	void setConfigSaveCallback(const std::function<void()> &saveCallBack) { configSaveCallback = saveCallBack; }
	void setDirty() { dirty = true; }
	void clrDirty() { dirty = false; }
	bool isDirty() const { return dirty; }

private:
	bool dirty = false;

	EepromData eepromData;

	static configManagerChecksum hash(configManagerChecksum value, uint8_t c);

	static configManagerChecksum checksumHelper(uint8_t *byteArray, unsigned long length, configManagerChecksum result = 0);

	template<typename T>
	configManagerChecksum checksumHelper(T value, configManagerChecksum result = 0) {
		return checksumHelper(reinterpret_cast<uint8_t *>(&value), sizeof(value), result);
	}

	template<typename T>
	configManagerChecksum checksum( T data) {
		auto result = checksumHelper(sizeof(data), 0);
		result = checksumHelper(data, result);
		return result;
	}

	std::function<void()> configSaveCallback = nullptr;

};
