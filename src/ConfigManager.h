#pragma once

#include "IPAddress.h"
#include "generated/config.h"
#include <TaskSchedulerDeclarations.h>
#include <EEPROM_Rotate.h>

typedef uint8_t configManagerChecksum;

class ConfigManager {

public:
	class InternalData {
	public:
		InternalData() {
			localIP = IPAddress();
			subnetMask = IPAddress();
			gatewayIP = IPAddress();
			dnsIP = IPAddress();
		}

	private:
		uint32_t localIP{};
		uint32_t subnetMask{};
		uint32_t gatewayIP{};
		uint32_t dnsIP{};
	public:
		IPAddress getLocalIP() const { return IPAddress(localIP); }
		IPAddress getSubnetMask() const { return IPAddress(subnetMask); }
		IPAddress getGatewayIP() const { return IPAddress(gatewayIP); }
		IPAddress getDnsIP() const { return IPAddress(dnsIP); }
		void setLocalIP(IPAddress localIP) { InternalData::localIP = localIP.v4(); }
		void setSubnetMask(IPAddress subnetMask) { InternalData::subnetMask = subnetMask.v4(); }
		void setGatewayIP(IPAddress gatewayIP) { InternalData::gatewayIP = gatewayIP.v4(); }
		void setDnsIP(IPAddress dnsIP) { InternalData::dnsIP = dnsIP.v4(); }
	};

private:

	class StoredData {
		InternalData internalData;
		ConfigData configData{};
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
		StoredData storedData{};
		ControlData controlData{};
	public:
		StoredData *getMutableStoredData() { return &storedData; }
		ControlData *getMutableControlData() { return &controlData; }
		const StoredData &getStoredData() const { return storedData; }
		const ControlData &getControlData() const { return controlData; }
	};


	//  using EEPROM_Rotate: EepromReservedAreaSize should be at least 3
	const unsigned int EepromReservedAreaSize = 8;
	const unsigned int EepromRotate_size = 2;

	EEPROM_Rotate eeprom{};
	EepromData eepromData{};
	Task *tLoop = nullptr;
	bool dirty = false;
	void writeEeprom();

public:
	ConfigManager();
	const EepromData &getEepromData() const { return eepromData; }

	EepromData *getMutableEepromData() { return &eepromData; }

	void addScheduler(Scheduler *scheduler);
	void setDirty();
	void saveInternalData(const InternalData *internalData);
	void saveConfigData(const ConfigData *configData);
	void setConfigSaveCallback(const std::function<void()> &saveCallBack) { configSaveCallback = saveCallBack; }

private:

	static configManagerChecksum hash(configManagerChecksum value, configManagerChecksum c);

	static configManagerChecksum checksumHelper(configManagerChecksum *byteArray, unsigned long length, configManagerChecksum result = 0);

	template<typename T>
	configManagerChecksum checksumHelper(T value, configManagerChecksum result = 0) {
		return checksumHelper(reinterpret_cast<configManagerChecksum *>(&value), sizeof(value), result);
	}

	template<typename T>
	configManagerChecksum checksum( T data) {
		auto result = checksumHelper(sizeof(data), 0);
		result = checksumHelper(data, result);
		return result;
	}

	std::function<void()> configSaveCallback = nullptr;

};

ConfigManager *getConfigManager();
