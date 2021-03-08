#include "Dashboard.h"
#include "WebServer.h"

#if defined(DEBUG_IOT_DASHBOARD) && defined(DEBUG_IOT_PORT)
#define LOGING_DASH 1
#define LOG_DASH(...) DEBUG_IOT_PORT.printf_P( "[DASH] " __VA_ARGS__ )
#else
#define LOGING_DASH 0
#define LOG_DASH(...)
#endif

Dashboard::Dashboard() : sendRepeatInterval(defaultSendRepeatInterval) {
#if LOGING_DASH
	getWebServer()->getWs()->onEvent(onWsEvent);
#endif
}

void Dashboard::send() {
	auto now = millis();
	static_assert(sizeof(now) == 4, "Used in index.js, function wsMessage");

	//send dashboardData, first timestamp and then the binary dashboardData structure
	uint8_t buffer[sizeof(dashboardData) + sizeof(now)];
	memcpy(buffer, reinterpret_cast<uint8_t *>(&now), sizeof(now));
	memcpy(buffer + sizeof(now), reinterpret_cast<uint8_t *>(&dashboardData), sizeof(dashboardData));

	getWebServer()->getWs()->binaryAll(buffer, sizeof(buffer));
}

void Dashboard::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *dataIn, size_t len) {
#if LOGING_DASH
	static const char *messages[] = {
			"WS_EVT_CONNECT",
			"WS_EVT_DISCONNECT",
			"WS_EVT_PONG",
			"WS_EVT_ERROR",
			"WS_EVT_DATA"
	};
	const char *message = type >= 0 && type <= WS_EVT_DATA ? messages[type] : "WS_EVT_???";
	LOG_DASH("%s\n", message);
#endif
}

void Dashboard::addScheduler(Scheduler *scheduler) {
	if (scheduler != nullptr) {
		tLoop = new Task(
				sendRepeatInterval,
				-1,
				[this]() -> void {
					send();
				},
				scheduler,
				true);
	}
}

void Dashboard::setSendRepeatInterval(unsigned long sendRepeatInterval) {
	Dashboard::sendRepeatInterval = sendRepeatInterval;
	tLoop->setInterval(sendRepeatInterval);
}

IDashboard *getDashboard() {
	static Dashboard dashboard = Dashboard();
	return &dashboard;
}
