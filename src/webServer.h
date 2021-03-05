#ifndef SERVER_H
#define SERVER_H

#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class webServer
{

private:    
    static void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static void serveProgmem(AsyncWebServerRequest *request);
    void bindAll();

public:
    AsyncWebServer server = AsyncWebServer(80);
    AsyncWebSocket ws = AsyncWebSocket("/ws");
    void begin();
    void loop();
	void addScheduler(Scheduler *scheduler);
};

extern webServer GUI;

#endif
