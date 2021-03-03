#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <memory>

class WifiManager {

private:
	DNSServer *dnsServer = nullptr;

	enum class reconnect_t {
		doNothing, wifiConnect, wifiForget, changeApPSK
	};
	volatile reconnect_t reconnect = reconnect_t::doNothing;
	String ssid;
	String pass;
	String ApPass;
	IPAddress localIP;
	IPAddress gatewayIP;
	IPAddress subnetMask;
	IPAddress dnsIP;

	bool apMode = false;
	char const *portalName;

	void startApMode();
	void stopApMode();

	void forget();
	void connectNewWifi();
	void changeApPsk();

	void restoreFromEEPROM();
	void storeToEEPROM() const;

	bool useStaticConfig();

public:
	void begin(char const *apName);
	void loop();

	void prepareWiFi_forget();
	void prepareWiFi_STA(String newSSID, String newPass);
	void prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP);
	void prepareWiFi_AP(String newPass);

	bool isApMode() const;
	static bool isDHCP();
	static String getSSID();
	static String getLocalIP();
	static String getSubnetMask();
	static String getGatewayIP();
	static String getDnsIP();
};

extern WifiManager WiFiManager;
