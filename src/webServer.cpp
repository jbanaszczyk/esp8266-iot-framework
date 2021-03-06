#include "webServer.h"
#include "ArduinoJson.h"
#include "LittleFS.h"

// Include the header file we create with webpack
#include "generated/html.h"

//Access to other classes for GUI functions
#include "WiFiManager.h"
#include "ConfigManager.h"
#include "OtaUpdateHelper.h"
#include "Dashboard.h"

#include <SimpleLogging.h>

#ifndef DEBUG_IOT_OTA
#define DEBUG_IOT_OTA NOTICE
#endif

WebServer::WebServer() {
	//to enable testing and debugging of the interface
	DefaultHeaders::Instance().addHeader(PSTR("Access-Control-Allow-Origin"), PSTR("*"));

	server.addHandler(&ws);
	server.begin();

	server.serveStatic("/download", LittleFS, "/");

	server.onNotFound(serveProgmem);

	//handle uploads
	server.on(PSTR("/upload"), HTTP_POST, [](AsyncWebServerRequest *request) {}, handleFileUpload);

	bindAll();
}

void WebServer::bindAll() {
	//Restart the ESP
	server.on(PSTR("/api/restart"), HTTP_POST, [](AsyncWebServerRequest *request) {
		request->send(200, PSTR("text/html"), ""); //respond first because of restart
		ESP.restart();
	});

	//update WiFi details
	server.on(PSTR("/api/wifi/set"), HTTP_POST, [](AsyncWebServerRequest *request) {
		request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
		getWiFiManager(nullptr)->prepareWiFi_STA(request->arg("ssid"), request->arg("pass"));
	});

	//update WiFi details with static IP
	server.on(PSTR("/api/wifi/setStatic"), HTTP_POST, [](AsyncWebServerRequest *request) {
		request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
		getWiFiManager(nullptr)->prepareWiFi_STA(request->arg("ssid"), request->arg("pass"), request->arg("localIP"), request->arg("subnetMask"), request->arg("gatewayIP"), request->arg("dnsIP"));
	});

	//forget WiFi details
	server.on(PSTR("/api/wifi/forget"), HTTP_POST, [](AsyncWebServerRequest *request) {
		request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
		getWiFiManager(nullptr)->prepareWiFi_STA_forget();

	});

	//set access point password
	server.on(PSTR("/api/wifi/set_ap"), HTTP_POST, [](AsyncWebServerRequest *request) {
		request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
		getWiFiManager(nullptr)->prepareWiFi_AP_PSK(request->arg("pass"));
	});

	//get WiFi details
	server.on(PSTR("/api/wifi/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
		String JSON;
		StaticJsonDocument<200> jsonBuffer;

		jsonBuffer["apMode"] = getWiFiManager(nullptr)->isApMode();
		jsonBuffer["ssid"] = getWiFiManager(nullptr)->getSSID();
		jsonBuffer["dhcp"] = getWiFiManager(nullptr)->isDHCP();
		jsonBuffer["localIP"] = getWiFiManager(nullptr)->getLocalIP();
		jsonBuffer["subnetMask"] = getWiFiManager(nullptr)->getSubnetMask();
		jsonBuffer["gatewayIP"] = getWiFiManager(nullptr)->getGatewayIP();
		jsonBuffer["dnsIP"] = getWiFiManager(nullptr)->getDnsIP();
		serializeJson(jsonBuffer, JSON);

		request->send(200, PSTR("text/html"), JSON);
	});

	//get file listing
	server.on(PSTR("/api/files/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
		String JSON;
		StaticJsonDocument<1000> jsonBuffer;
		JsonArray files = jsonBuffer.createNestedArray("files");

		//get file listing
		Dir dir = LittleFS.openDir("");
		while (dir.next())
			files.add(dir.fileName());

		//get used and total data
		FSInfo fs_info{};
		LittleFS.info(fs_info);
		jsonBuffer["used"] = String(static_cast<uint32_t>(fs_info.usedBytes));
		jsonBuffer["max"] = String(static_cast<uint32_t>(fs_info.totalBytes));

		serializeJson(jsonBuffer, JSON);

		request->send(200, PSTR("text/html"), JSON);
	});

	//remove file
	server.on(PSTR("/api/files/remove"), HTTP_POST, [](AsyncWebServerRequest *request) {
		LittleFS.remove("/" + request->arg("filename"));
		request->send(200, PSTR("text/html"), "");
	});

	//update from LittleFS
	server.on(PSTR("/api/update"), HTTP_POST, [](AsyncWebServerRequest *request) {
		getOtaUpdateHelper()->requestStart("/" + request->arg("filename"));
		request->send(200, PSTR("text/html"), "");
	});

	//update status
	server.on(PSTR("/api/update-status"), HTTP_GET, [](AsyncWebServerRequest *request) {
		String JSON;
		StaticJsonDocument<200> jsonBuffer;

		jsonBuffer["status"] = getOtaUpdateHelper()->getStatus();
		serializeJson(jsonBuffer, JSON);

		request->send(200, PSTR("text/html"), JSON);
	});

	//send binary configuration data
	server.on(PSTR("/api/config/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
		AsyncResponseStream *response = request->beginResponseStream(PSTR("application/octet-stream"));
		auto configData = getConfigManager()->getEepromData().getStoredData().getConfigData();
		response->write(reinterpret_cast<const char *>(&configData), sizeof(configData));
		request->send(response);
	});

	//receive binary configuration data from body
	server.on(
			PSTR("/api/config/set"), HTTP_POST,
			[this](AsyncWebServerRequest *request) {},
			[](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {},
			[this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
				if (index != 0 || total != len) {
					request->send(500, PSTR("text/html"), "[webServer] not supported handleBody");
				} else if (len != sizeof(ConfigData)) {
					request->send(500, PSTR("text/html"), "[webServer] ConfigData size mismatch");
				} else {
					auto *config = reinterpret_cast<ConfigData *>(data);
					getConfigManager()->saveConfigData(config);
					request->send(200, PSTR("text/html"), "");
				}
			});

	//receive binary configuration data from body
	server.on(
			PSTR("/api/dash/set"), HTTP_POST,
			[this](AsyncWebServerRequest *request) {},
			[](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {},
			[this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
				memcpy(reinterpret_cast<uint8_t *>(getDashboard()->getMutualDashboardData()) + (request->arg("start")).toInt(), data, static_cast<size_t>((request->arg("length")).toInt()));
				request->send(200, PSTR("text/html"), "");
			});
}

// Callback for the html
void WebServer::serveProgmem(AsyncWebServerRequest *request) {
	// Dump the byte array in PROGMEM with a 200 HTTP code (OK)
	AsyncWebServerResponse *response = request->beginResponse_P(200, PSTR("text/html"), html, html_len);

	// Tell the browser the content is Gzipped
	response->addHeader(PSTR("Content-Encoding"), PSTR("gzip"));

	request->send(response);
}

void WebServer::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	static File fsUploadFile;

	if (!index) {
		auto logger = Logging.getLogger("OtaUpdates", Logging.DEBUG_IOT_OTA);
		logger->info.printf_P(PSTR("Start file upload: %s\n"), filename.c_str());

		if (!filename.startsWith("/"))
			filename = "/" + filename;

		fsUploadFile = LittleFS.open(filename, "w");
	}

	for (size_t i = 0; i < len; i++) {
		fsUploadFile.write(data[i]);
	}

	if (final) {
		String JSON;
		StaticJsonDocument<100> jsonBuffer;

		jsonBuffer["success"] = fsUploadFile.isFile();
		serializeJson(jsonBuffer, JSON);

		request->send(200, PSTR("text/html"), JSON);
		fsUploadFile.close();
	}
}

void WebServer::addScheduler(Scheduler *scheduler) {
	getOtaUpdateHelper()->addScheduler(scheduler);
}

IWebServer *getWebServer() {
	static WebServer webServer = WebServer();
	return &webServer;
}
