#pragma once

#include <Arduino.h>
#include <memory>
#include <DNSServer.h>
//#include <TaskScheduler.h>

class WifiManager {

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

private:
	DNSServer *dnsServer = nullptr;

	enum class reconnect_t {
		doNothing, wifiConnect, wifiForget, changeApPSK
	};

	struct StorageNewWfi {
		String ssid;
		String pass;
		IPAddress localIP;
		IPAddress gatewayIP;
		IPAddress subnetMask;
		IPAddress dnsIP;
	};

	struct StorageApPassword {
		String ApPass;
	};

	StorageNewWfi storageNewWfi;
	StorageApPassword storageApPassword;

	volatile reconnect_t reconnect = reconnect_t::doNothing;

	bool apMode = false;
	String portalName;

	void startApMode();
	void stopApMode();

	void forget();
	void connectNewWifi();
	void changeApPsk();

	void restoreFromEEPROM();
	void storeToEEPROM() const;

	bool useStaticConfig();

};

extern WifiManager wiFiManager;
