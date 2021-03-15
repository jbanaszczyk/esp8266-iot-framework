#include <ESP8266WiFi.h>
#include "WiFiManager.h"
#include "ConfigManager.h"

WiFiManager::WiFiManager(char const *apName) {
	apMode = false;
	portalName = (apName != nullptr && apName[0] != '\0') ? apName : "ESP8266";
	logger->debug.printf("local hostname: %s\n", portalName.c_str());

	ETS_UART_INTR_DISABLE();
	wifi_station_disconnect();
	auto wiFiConfig = std::unique_ptr<WiFiConfig>(WiFiConfig::fromConfigManager());
	wiFiConfig->use();
	WiFi.hostname(portalName);
	WiFi.persistent(true);
	WiFi.setAutoReconnect(true);
	WiFi.setAutoConnect(true);
	ETS_UART_INTR_ENABLE();
	WiFi.mode(WIFI_STA);
	WiFi.begin();
	logger->debug.printf("WiFi begun, ssid:%s\n", WiFi.SSID().c_str());
}

void WiFiManager::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		aScheduler = scheduler;
		tApStartStop = new Task(
				0,
				1,
				[this]() -> void {
					apStartStop();
				},
				scheduler,
				false
		);
		tRedirectDNS = new Task(
				0,
				-1,
				[this]() -> void {
					if (apMode && dnsServer != nullptr) {
						dnsServer->processNextRequest();
					}
				},
				scheduler,
				false,
				[this]() -> bool {
					logger->debug.print("RedirectDNS start\n");
					return true;
				},
				[this]() -> void {
					logger->debug.print("RedirectDNS stop\n");
				}
		);
		tInitialConnect = new Task(
				InitialConnectTimeout,
				2,
				nullptr,
				scheduler,
				!WiFi.isConnected(),
				nullptr,
				[this]() -> void {
					tApStartStop->enable();
				}
		);
		tChangeWifi = new Task(
				0,
				1,
				[this]() -> void {
					connectNewWifi();
				},
				scheduler,
				false
		);
		tChangeWifiTimeOut = new Task(
				InitialConnectTimeout,
				2,
				nullptr,
				scheduler,
				false,
				nullptr,
				[this]() -> void {
					connectNewWifiCheck();
				}
		);
		tChangeApPsk = new Task(
				0,
				1,
				[this]() -> void {
					changeApPsk();
				},
				scheduler,
				false
		);

		handlerStationModeGotIP = WiFi.onStationModeGotIP(std::bind(&WiFiManager::onStationModeGotIP, this, std::placeholders::_1));
		handlerStationModeDisconnected = WiFi.onStationModeDisconnected(std::bind(&WiFiManager::onStationModeDisconnected, this, std::placeholders::_1));
	}
}

void WiFiManager::onStationModeGotIP(const WiFiEventStationModeGotIP &evt) const {
	logger->info.printf("onStationModeGotIP ssid:%s IP:%s mask:%s gateway:%s\n", WiFi.SSID().c_str(), evt.ip.toString().c_str(), evt.mask.toString().c_str(), evt.gw.toString().c_str());
	tInitialConnect->disable();
	if (!tChangeWifiTimeOut->isEnabled()) {
		tApStartStop->restart();
	} else {
		tChangeWifiTimeOut->disable();
	}
}

void WiFiManager::onStationModeDisconnected(const WiFiEventStationModeDisconnected &evt) const {
	logger->info.printf("onStationModeDisconnected ssid %s reason %d\n", evt.ssid.c_str(), evt.reason);
	tInitialConnect->disable();
	tApStartStop->restart();
}

void WiFiManager::apStartStop() {
	if (apMode == !WiFi.isConnected()) {
		return;
	}

	apMode
	? apStop()
	: apStart();
}

void WiFiManager::apStart() {
	logger->debug.print("ApMode start\n");
	WiFi.mode(static_cast<WiFiMode_t>(WiFi.getMode() | WIFI_STA));
	bool canReconnect = !WiFi.SSID().isEmpty();
	WiFi.mode(canReconnect ? WIFI_AP_STA : WIFI_AP);
	WiFi.softAP(portalName, WiFi.softAPPSK());

	/* Setup the DNS server redirecting all the domains to the apIP */
	delete dnsServer;
	dnsServer = new DNSServer();
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(53, "*", WiFi.softAPIP());

	if (tRedirectDNS != nullptr) {
		tRedirectDNS->enable();
	}

	apMode = true;

	logger->info.printf("Opened AP mode portal SSID: %s IP: %s\n", WiFi.softAPSSID().c_str(), WiFi.softAPIP().toString().c_str());
}

void WiFiManager::apStop() {
	apMode = false;
	logger->debug.print("ApMode stop\n");
	WiFi.mode(WIFI_STA);
	if (tRedirectDNS != nullptr) {
		tRedirectDNS->disable();
	}
	delete dnsServer;
	dnsServer = nullptr;
	logger->info.printf("ApMode closed\n");
}

void WiFiManager::forgetWiFi() {
	logger->debug.print("WiFi forget\n");
	WiFiConfig wiFiConfig{};
	wiFiConfig.use();
	wiFiConfig.storeToConfigManager();
#ifdef ESP32
	WiFi.disconnect(true,true);
#else
	WiFi.persistent(true);
	WiFi.disconnect(true);
	WiFi.persistent(false);
#endif
	logger->info.print("WiFi forgotten\n");
}

void WiFiManager::connectNewWifiCheck() {
	logger->debug.print("connectNewWifiCheck\n");

	auto *oldWiFiConfig = static_cast<WiFiConfig *>(aScheduler->currentLts());
	if (WiFi.isConnected()) {
		logger->debug.printf("Connected to %s\n", WiFi.SSID().c_str());
		std::unique_ptr<WiFiConfig> currentConfig = std::unique_ptr<WiFiConfig>(WiFiConfig::fromWiFi());
		currentConfig->storeToConfigManager();
	} else {
		logger->notice.printf("Error connecting to %s\n", WiFi.SSID().c_str());
		oldWiFiConfig->use();
		WiFi.begin(oldWiFiConfig->getSsid(), oldWiFiConfig->getPass());
	}

	delete oldWiFiConfig;

	WiFi.persistent(true);
	WiFi.setAutoReconnect(true);
	if (WiFi.isConnected()) {
		logger->info.print("connectNewWiFi done ok\n");
	} else {
		logger->notice.printf("connectNewWiFi done, status: %d\n", WiFi.status());
	}
}

void WiFiManager::connectNewWifi() {
	auto *wiFiConfig = static_cast<WiFiConfig *>(aScheduler->currentLts());
	if (wiFiConfig == nullptr) {
		forgetWiFi();
		return;
	}

	logger->debug.print("connectNewWifi\n");

	auto oldConfig = WiFiConfig::fromWiFi();

	if ((WiFi.getMode() & WIFI_STA) != 0 && !wiFiConfig->getSsid().isEmpty() && oldConfig->getSsid() != wiFiConfig->getSsid()) {
		logger->debug.print("WiFi force disconnect\n");
		WiFi.persistent(false);
		WiFi.setAutoReconnect(false);
		WiFi.disconnect();
	}

	wiFiConfig->use();
	WiFi.hostname(portalName);
	WiFi.mode(WIFI_OFF);
	WiFi.mode(WIFI_STA);
	if (apMode) {
		apStop();
	}

	tChangeWifiTimeOut->setLtsPointer(oldConfig);
	tChangeWifiTimeOut->restart();

	WiFi.persistent(true);
	WiFi.setAutoReconnect(true);

	wiFiConfig->getSsid().isEmpty()
	? WiFi.begin()
	: WiFi.begin(wiFiConfig->getSsid(), wiFiConfig->getPass());

	delete wiFiConfig;
}

void WiFiManager::changeApPsk() {
	logger->debug.print("changeApPsk\n");

	auto oldMode = WiFi.getMode();
	if (oldMode & WIFI_AP) {
		WiFi.mode(static_cast<WiFiMode_t>(WIFI_AP | oldMode));
	}

	auto *newApPsk = static_cast<NewApPsk *>(aScheduler->currentLts());
	auto result = WiFi.softAP(portalName, newApPsk->getApPsk());
	delete newApPsk;

	WiFi.mode(oldMode);
	if (result ) {
		logger->debug.print("changeApPsk: ok\n");
	}  else {
		logger->notice.printf("changeApPsk: Failed\n");
	}
}

void WiFiManager::prepareWiFi_STA_forget() {
	logger->debug.print("prepareWiFi_STA_forget\n");
	if (tChangeWifi != nullptr) {
		tChangeWifi->setLtsPointer(nullptr);
		tChangeWifi->restart();
	} else {
		logger->notice.print("WARN: tForgetWifi not set\n");
	}
}

void WiFiManager::prepareWiFi_STA(String newSSID, String newPass) {
	logger->debug.printf("prepareWiFi_STA ssid:%s\n", newSSID.c_str());
	if (tChangeWifi != nullptr) {
		auto newConfig = new WiFiConfig(newSSID, newPass);
		tChangeWifi->setLtsPointer(newConfig);
		tChangeWifi->restart();
	}
}

void WiFiManager::prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP) {
	logger->debug.printf("prepareWiFi_STA_Config ssid:%s\n", newSSID.c_str());
	if (tChangeWifi != nullptr) {
		auto newConfig = new WiFiConfig(newSSID, newPass,
		                                IPAddress().fromString(newLocalIP),
		                                IPAddress().fromString(newSubnetMask),
		                                IPAddress().fromString(newGatewayIP),
		                                IPAddress().fromString(newDnsIP));
		tChangeWifi->setLtsPointer(newConfig);
		tChangeWifi->restart();
	}
}

void WiFiManager::prepareWiFi_AP_PSK(String newPass) {
	logger->debug.print("prepareWiFi_AP_PSK\n");
	if (tChangeApPsk != nullptr) {
		auto newPsk = new NewApPsk(newPass);
		tChangeApPsk->setLtsPointer(newPsk);
		tChangeApPsk->restart();
	}
}

bool WiFiManager::isApMode() const {
	return apMode;
}

String WiFiManager::getSSID() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.SSID();
}

bool WiFiManager::isDHCP() {
	return (WiFi.getMode() & WIFI_STA) == WIFI_STA && wifi_station_dhcpc_status() == DHCP_STARTED;
}

String WiFiManager::getLocalIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.localIP().toString();
}

String WiFiManager::getSubnetMask() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.subnetMask().toString();
}

String WiFiManager::getGatewayIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.gatewayIP().toString();
}

String WiFiManager::getDnsIP() {
	return (WiFi.getMode() & WIFI_STA) != WIFI_STA ? "" : WiFi.dnsIP().toString();
}

void WiFiManager::WiFiConfig::storeToConfigManager() {
	ConfigManager::InternalData *internalData = getConfigManager()->getMutableEepromData()->getMutableStoredData()->getMutableInternalData();
	internalData->setLocalIP(localIP);
	internalData->setSubnetMask(subnetMask);
	internalData->setGatewayIP(gatewayIP);
	internalData->setDnsIP(dnsIP);
	getConfigManager()->setDirty();
}

WiFiManager::WiFiConfig::WiFiConfig() :
		ssid(""), pass(""),
		localIP(), gatewayIP(), subnetMask(), dnsIP() {}

WiFiManager::WiFiConfig::WiFiConfig(String ssid, String pass) :
		ssid(std::move(ssid)), pass(std::move(pass)),
		localIP(), gatewayIP(), subnetMask(), dnsIP() {}

WiFiManager::WiFiConfig::WiFiConfig(const IPAddress &localIp, const IPAddress &gatewayIp, const IPAddress &subnetMask, const IPAddress &dnsIp) :
		ssid(""), pass(""),
		localIP(localIp), gatewayIP(gatewayIp), subnetMask(subnetMask), dnsIP(dnsIp) {
	fix();
}

WiFiManager::WiFiConfig::WiFiConfig(String ssid, String pass, const IPAddress &localIp, const IPAddress &gatewayIp, const IPAddress &subnetMask, const IPAddress &dnsIp) :
		ssid(std::move(ssid)), pass(std::move(pass)),
		localIP(localIp), gatewayIP(gatewayIp), subnetMask(subnetMask), dnsIP(dnsIp) {}

bool WiFiManager::WiFiConfig::use() {
	fix();
	return WiFi.config(localIP, gatewayIP, subnetMask, dnsIP);
}

void WiFiManager::WiFiConfig::fix() {
	if (!localIP.isSet()) {
		gatewayIP = IPAddress();
		subnetMask = IPAddress();
		dnsIP = IPAddress();
	}
}

WiFiManager::WiFiConfig *WiFiManager::WiFiConfig::fromConfigManager() {
	static ConfigManager::InternalData internalData = getConfigManager()->getEepromData().getStoredData().getInternalData();
	auto wiFiConfig = new WiFiConfig{};
	wiFiConfig->ssid = "";
	wiFiConfig->pass = "";
	wiFiConfig->localIP = internalData.getLocalIP();
	wiFiConfig->gatewayIP = internalData.getGatewayIP();
	wiFiConfig->subnetMask = internalData.getSubnetMask();
	wiFiConfig->dnsIP = internalData.getDnsIP();
	wiFiConfig->fix();
	return wiFiConfig;
}

WiFiManager::WiFiConfig *WiFiManager::WiFiConfig::fromWiFi() {
	return wifi_station_dhcpc_status() == DHCP_STARTED
	       ? new WiFiConfig(
					WiFi.SSID(), WiFi.psk())
	       : new WiFiConfig(
					WiFi.SSID(), WiFi.psk(),
					WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), WiFi.dnsIP()
			);
}

IWiFiManager *getWiFiManager(const char *apName) {
	static WiFiManager wiFiManager = WiFiManager(apName);
	return &wiFiManager;
}
