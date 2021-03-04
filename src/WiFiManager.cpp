// inspired by https://github.com/tzapu/WiFiManager but
// with more flexibility to add your own web server setup
// state machine for changing wifi settings on the fly

#include <ESP8266WiFi.h>
#include "WiFiManager.h"
#include "configManager.h"

#include <user_interface.h>

#if defined(DEBUG_IOT_WIFI_MANAGER) && defined(DEBUG_IOT_PORT)
#define LOG_WIFI(...) DEBUG_IOT_PORT.printf_P( "[WIFI] " __VA_ARGS__ )
#define LOG_WIFI_IP(format) LOG_WIFI(format, WiFi.localIP().toString().c_str())
#else
#define LOG_WIFI(...)
#define LOG_WIFI_IP(format)

#endif

void WifiManager::begin(char const *apName) {
	portalName = apName;

	restoreFromEEPROM();

	ETS_UART_INTR_DISABLE();
	wifi_station_disconnect();
	useStaticConfig();
	WiFi.hostname(portalName);
	WiFi.persistent(true);
	WiFi.setAutoReconnect(true);
	WiFi.setAutoConnect(true);
	ETS_UART_INTR_ENABLE();
	WiFi.mode(WIFI_STA);
	WiFi.begin();


	WiFi.waitForConnectResult();

	if (WiFi.isConnected()) {
		LOG_WIFI_IP("Connected to WiFi, as %s\n");
	} else {
		LOG_WIFI("Not connected, status %d\n", WiFi.status());
	}
}

void WifiManager::startApMode() {
	if (!apMode) {
		apMode = true;
		LOG_WIFI("startApMode\n");

		WiFi.mode(WIFI_STA);
		bool canReconnect = !WiFi.SSID().isEmpty();
		WiFi.mode(canReconnect ? WIFI_AP_STA : WIFI_AP);
		WiFi.softAP(portalName, WiFi.softAPPSK());

		/* Setup the DNS server redirecting all the domains to the apIP */
		delete dnsServer;
		dnsServer = new DNSServer();
		dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer->start(53, "*", WiFi.softAPIP());

		LOG_WIFI("Opened AP mode portal\nSSID: %s\nIP:   %s\n", WiFi.softAPSSID().c_str(), WiFi.softAPIP().toString().c_str());
	}
}

void WifiManager::stopApMode() {
	if (apMode) {
		apMode = false;
		LOG_WIFI("stopApMode\n");
		WiFi.mode(WIFI_STA);
		delete dnsServer;
		dnsServer = nullptr;
	}
}

void WifiManager::forget() {
	LOG_WIFI("forget WiFi.\n");

	storageNewWfi.localIP = IPAddress();
	useStaticConfig();
	storeToEEPROM();

	WiFi.persistent(false);
	WiFi.disconnect();
	WiFi.persistent(true);
}

void WifiManager::connectNewWifi() {
	auto oldSSID = WiFi.SSID();
	auto oldPSK = WiFi.psk();

	auto oldIP = WiFi.localIP();
	auto oldGatewayIP = WiFi.gatewayIP();
	auto oldSubnetMask = WiFi.subnetMask();
	auto oldDns = WiFi.dnsIP();

	LOG_WIFI("connectNewWiFi\n");

	if ((WiFi.getMode() & WIFI_STA) != 0 && !storageNewWfi.ssid.isEmpty() && oldSSID != storageNewWfi.ssid) {
		LOG_WIFI("WiFi force disconnect\n");
		WiFi.persistent(false);
		WiFi.setAutoReconnect(false);
		WiFi.disconnect();
	}

	useStaticConfig();
	WiFi.hostname(portalName);
	WiFi.mode(WIFI_OFF);
	WiFi.mode(WIFI_STA);

	storageNewWfi.ssid.isEmpty()
	? WiFi.begin()
	: WiFi.begin(storageNewWfi.ssid, storageNewWfi.pass);

	if (WiFi.waitForConnectResult() == WL_CONNECTED) {
		storeToEEPROM();
	} else {
		LOG_WIFI("Error connecting to %s\n", ssid.c_str());
		WiFi.config(oldIP, oldGatewayIP, oldSubnetMask, oldDns);
		WiFi.begin(oldSSID, oldPSK);
		if (WiFi.waitForConnectResult() != WL_CONNECTED) {
			LOG_WIFI("Old credentials failed\n");
		}
	}

	WiFi.persistent(true);
	WiFi.setAutoReconnect(true);

	LOG_WIFI("connectNewWiFi done, status: %d\n", WiFi.status());
}

void WifiManager::changeApPsk() {
	LOG_WIFI("changeApPsk\n");

	auto oldMode = WiFi.getMode();
	if (oldMode & WIFI_AP) {
		WiFi.mode(static_cast<WiFiMode_t>(WIFI_AP | oldMode));
	}
	auto result = WiFi.softAP(portalName, storageApPassword.ApPass);
	WiFi.mode(oldMode);

	LOG_WIFI("changeApPsk: %s\n", result ? "Ok" : "Failed");
}

void WifiManager::prepareWiFi_forget() {
	reconnect = reconnect_t::wifiForget;
}

void WifiManager::prepareWiFi_STA(String newSSID, String newPass) {
	storageNewWfi.ssid = std::move(newSSID);
	storageNewWfi.pass = std::move(newPass);
	storageNewWfi.localIP = IPAddress();
	storageNewWfi.subnetMask = IPAddress();
	storageNewWfi.gatewayIP = IPAddress();
	storageNewWfi.dnsIP = IPAddress();
	reconnect = reconnect_t::wifiConnect;
}

void WifiManager::prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP) {
	storageNewWfi.ssid = std::move(newSSID);
	storageNewWfi.pass = std::move(newPass);
	storageNewWfi.localIP.fromString(newLocalIP);
	storageNewWfi.subnetMask.fromString(newSubnetMask);
	storageNewWfi.gatewayIP.fromString(newGatewayIP);
	storageNewWfi.dnsIP.fromString(newDnsIP);
	reconnect = reconnect_t::wifiConnect;
}

void WifiManager::prepareWiFi_AP(String newPass) {
	storageApPassword.ApPass = std::move(newPass);
	reconnect = reconnect_t::changeApPSK;
}

bool WifiManager::useStaticConfig() {
	if (!storageNewWfi.localIP.isSet()) {
		storageNewWfi.subnetMask = IPAddress();
		storageNewWfi.gatewayIP = IPAddress();
		storageNewWfi.dnsIP = IPAddress();
	}
	return WiFi.config(storageNewWfi.localIP, storageNewWfi.gatewayIP, storageNewWfi.subnetMask, storageNewWfi.dnsIP);
}

bool WifiManager::isApMode() const {
	return apMode;
}

String WifiManager::getSSID() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.SSID();
}

bool WifiManager::isDHCP() {
	return (WiFi.getMode() & WIFI_STA) == WIFI_STA && wifi_station_dhcpc_status() == DHCP_STARTED;
}

String WifiManager::getLocalIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.localIP().toString();
}

String WifiManager::getSubnetMask() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.subnetMask().toString();
}

String WifiManager::getGatewayIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.gatewayIP().toString();
}

String WifiManager::getDnsIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.dnsIP().toString();
}

void WifiManager::loop() {
	if (apMode && dnsServer != nullptr) {
		dnsServer->processNextRequest();
	}

	switch (reconnect) {
		case reconnect_t::wifiConnect: {
			connectNewWifi();
			reconnect = reconnect_t::doNothing;
			break;
		}

		case reconnect_t::wifiForget: {
			forget();
			reconnect = reconnect_t::doNothing;
			break;
		}

		case reconnect_t::changeApPSK: {
			changeApPsk();
			reconnect = reconnect_t::doNothing;
			break;
		}

		default: {
			break;
		}
	}

	if (WiFi.isConnected() != !apMode) {
		WiFi.isConnected()
		? stopApMode()
		: startApMode();
	}
}

void WifiManager::restoreFromEEPROM() {
	auto internalData = getConfigManager()->getEepromData().getStoredData().getInternalData();
	storageNewWfi.localIP = internalData.getLocalIP();
	storageNewWfi.subnetMask = internalData.getSubnetMask();
	storageNewWfi.gatewayIP = internalData.getGatewayIP();
	storageNewWfi.dnsIP = internalData.getDnsIP();
}

void WifiManager::storeToEEPROM() const {
	auto internalData = getConfigManager()->getMutableEepromData()->getMutableStoredData()->getMutableInternalData();
	internalData->setLocalIP(storageNewWfi.localIP);
	internalData->setSubnetMask(storageNewWfi.subnetMask);
	internalData->setGatewayIP(storageNewWfi.gatewayIP);
	internalData->setDnsIP(storageNewWfi.dnsIP);
}

WifiManager wiFiManager;
