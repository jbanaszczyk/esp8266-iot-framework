#include <Arduino.h>
#include <LittleFS.h>

#include <WiFiManager.h>
#include <WebServer.h>
#include <OtaUpdateHelper.h>
#include <HTTPRequest.h>
#include <configManager.h>
#include <timeSync.h>

struct task {
	unsigned long rate;
	unsigned long previous;
};

task taskA = {.rate = 5000, .previous = 0};

void setup() {
	Serial.begin(115200);
	LittleFS.begin();
	GUI.begin();
	configManager.begin();
	WiFiManager.begin(configManager.data.projectName);
	timeSync.begin();
}

void loop() {
	//software interrupts
	WiFiManager.loop();
	otaUpdateHelper.loop();
	configManager.loop();

	//task A
	if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate)) {
		taskA.previous = millis();

		//do task
		Serial.println(ESP.getFreeHeap());

		HTTPRequest httpRequest("https://www.google.com");
//		HTTPRequest httpRequest("http://www.google.com");
//		HTTPRequest httpRequest("https://api.ipgeolocation.io/timezone?apiKey=a8d4b9360d214b32bf057aaf1a6907ec");   // requires -D DOMAIN_LIST=google.com,api.ipgeolocation.io

		auto result = httpRequest.GET();
		Serial.printf("HTTP status: %d\n", result);

		while (httpRequest.busy()) {
			if (httpRequest.available()) {
				Serial.write(httpRequest.read());
			} else {
				delay(1);
			}
		}

		httpRequest.end();
	}
}
