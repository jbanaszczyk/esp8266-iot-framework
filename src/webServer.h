#pragma once

#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class IWebServer {
public:
	virtual ~IWebServer() = default;

	virtual AsyncWebSocket *getWs() = 0;

	virtual AsyncWebServer *getServer() = 0;

	virtual void addScheduler(Scheduler *scheduler) = 0;
};

class WebServer : public IWebServer {
	static void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

	static void serveProgmem(AsyncWebServerRequest *request);

	void bindAll();

	AsyncWebSocket ws = AsyncWebSocket("/ws");
	AsyncWebServer server = AsyncWebServer(80);

public:
	WebServer();

	AsyncWebSocket *getWs() override { return &ws; }

	AsyncWebServer *getServer() override { return &server; }

	void addScheduler(Scheduler *scheduler) override;
};

IWebServer *getWebServer();
