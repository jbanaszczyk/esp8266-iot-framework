#pragma once

#include <ESPAsyncWebServer.h>
#include "generated/dash.h"

class Dashboard {

private:
	static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

	unsigned long loopRate = 0;
	unsigned long loopPrevious = 0;

public:
	void begin(int sampleTimeMs = 1000);

	void loop();

	void send();

	dashboardData data;
};

extern Dashboard dash;
