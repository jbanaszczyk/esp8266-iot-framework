#include "Dashboard.h"
#include <WebServer.h>

void Dashboard::send() {
	auto now = millis();
	static_assert(sizeof(now) == 4, "Used in index.js, function wsMessage");

	//send dashboardData, first timestamp and then the binary dashboardData structure
	uint8_t buffer[sizeof(dashboardData) + sizeof(now)];
	memcpy(buffer, reinterpret_cast<uint8_t *>(&now), sizeof(now));
	memcpy(buffer + sizeof(now), reinterpret_cast<uint8_t *>(&dashboardData), sizeof(dashboardData));

	getWebServer()->getWs()->binaryAll(buffer, sizeof(buffer));
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
