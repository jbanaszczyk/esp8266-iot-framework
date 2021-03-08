#include "Dashboard.h"
#include "WebServer.h"

void Dashboard::begin(int sampleTimeMs) {
	getWebServer()->getWs()->onEvent(onWsEvent);
	loopRate = sampleTimeMs;
}

void Dashboard::loop() {
	if (loopPrevious == 0 || (millis() - loopPrevious > loopRate)) {
		loopPrevious = millis();

		send();
	}
}

void Dashboard::send() {
	//send data, first 32bit timestamp and then the binary data structure
	uint8_t buffer[sizeof(data) + 8];

	unsigned long now = millis();
	memcpy(buffer, reinterpret_cast<uint8_t *>(&now), 8);

	memcpy(buffer + 8, reinterpret_cast<uint8_t *>(&data), sizeof(data));

	getWebServer()->getWs()->binaryAll(buffer, sizeof(buffer));
}

void Dashboard::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *dataIn, size_t len) {
	/* initialize new client */
	if (type == WS_EVT_CONNECT) {
		Serial.println("New WS client");
	} else if (type == WS_EVT_DISCONNECT) {
		Serial.println("Lost WS client");
	}
}

Dashboard dash;