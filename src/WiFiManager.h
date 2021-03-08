#include <algorithm>

#pragma once

#include <Arduino.h>
#include <memory>
#include <utility>
#include <DNSServer.h>
#include <TaskSchedulerDeclarations.h>
#include <ESP8266WiFi.h>

class IWiFiManager {
public:
	virtual ~IWiFiManager() = default;

	virtual void addScheduler(Scheduler *scheduler) = 0;

	virtual void prepareWiFi_STA_forget() = 0;

	virtual void prepareWiFi_STA(String newSSID, String newPass) = 0;

	virtual void prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP) = 0;

	virtual void prepareWiFi_AP_PSK(String newPass) = 0;

	virtual bool isApMode() const = 0;

	virtual bool isDHCP() = 0;

	virtual String getSSID() = 0;

	virtual String getLocalIP() = 0;

	virtual String getSubnetMask() = 0;

	virtual String getGatewayIP() = 0;

	virtual String getDnsIP() = 0;
};

IWiFiManager *getWiFiManager(char const *apName);

class WiFiManager : public IWiFiManager {
public:
	unsigned long InitialConnectTimeout = 60000U;

	explicit WiFiManager(char const *apName);

	void addScheduler(Scheduler *scheduler) override;

	Task *tInitialConnect = nullptr;
	Task *tApStartStop = nullptr;
	Task *tRedirectDNS = nullptr;
	Task *tChangeWifi = nullptr;
	Task *tChangeWifiTimeOut = nullptr;
	Task *tChangeApPsk = nullptr;

	void prepareWiFi_STA_forget() override;
	void prepareWiFi_STA(String newSSID, String newPass) override;
	void prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP) override;

	void prepareWiFi_AP_PSK(String newPass) override;

	bool isApMode() const override;
	bool isDHCP() override;
	String getSSID() override;
	String getLocalIP() override;
	String getSubnetMask() override;
	String getGatewayIP() override;
	String getDnsIP() override;

private:
	String portalName;
	bool apMode = false;
	DNSServer *dnsServer = nullptr;
	Scheduler *aScheduler = nullptr;

	WiFiEventHandler handlerStationModeGotIP;
	WiFiEventHandler handlerStationModeDisconnected;

	void onStationModeGotIP(const WiFiEventStationModeGotIP &evt) const;
	void onStationModeDisconnected(const WiFiEventStationModeDisconnected &evt) const;

	class WiFiConfig {
	public:
		WiFiConfig();

		WiFiConfig(String ssid, String pass);
		WiFiConfig(const IPAddress &localIp, const IPAddress &gatewayIp, const IPAddress &subnetMask, const IPAddress &dnsIp);
		WiFiConfig(String ssid, String pass, const IPAddress &localIp, const IPAddress &gatewayIp, const IPAddress &subnetMask, const IPAddress &dnsIp);

		static WiFiConfig *fromConfigManager();
		static WiFiConfig *fromWiFi();

		void storeToConfigManager();

		bool use();

	private:
		String ssid;

		String pass;
		IPAddress localIP;
		IPAddress gatewayIP;
		IPAddress subnetMask;
		IPAddress dnsIP;

		void fix();

	public:
		const String &getSsid() const { return ssid; }
		const String &getPass() const { return pass; }

	};

	class NewApPsk {
		String apPsk;
	public:
		explicit NewApPsk(String apPsk) : apPsk(std::move(apPsk)) {}
		const String &getApPsk() const {return apPsk; }
	};

	static void forgetWiFi();
	void connectNewWifi();
	void connectNewWifiCheck();
	void changeApPsk();

	void apStartStop();
	void apStop();
	void apStart();
};
