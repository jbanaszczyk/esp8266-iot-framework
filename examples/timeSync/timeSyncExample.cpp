#include <Arduino.h>
#include <LittleFS.h>

#include <WiFiManager.h>
#include <WebServer.h>
#include <OtaUpdateHelper.h>
#include <configManager.h>
#include <timeSync.h>
#include <TZ.h>

void showNow() {
	if (timeSync.isSynced()) {
		time_t now;
		struct tm *info;
		time(&now);
		info = localtime(&now);
		char buffer[80];

		strftime(buffer, sizeof(buffer), "%Y.%m.%d %H.%M.%S", info);
		Serial.printf_P("Current local time: %s\n", buffer);
	} else {
		Serial.printf_P("Time is not ready\n");
	}
}

void setup() {
	Serial.begin(115200);
	while (!Serial) {}
	Serial.println();

	LittleFS.begin();
	GUI.begin();
	configManager.begin();
	WiFiManager.begin(configManager.data.projectName);

	//Set the timezone
	timeSync.begin(TZ_Europe_Warsaw);

	//Wait for connection
	timeSync.waitForSync(10000);

	showNow();
	Serial.printf_P("==[ Setup done ]============\n");
}

void loop() {
	//software interrupts
	WiFiManager.loop();
	otaUpdateHelper.loop();
	configManager.loop();

	delay(1000);
	showNow();
}
