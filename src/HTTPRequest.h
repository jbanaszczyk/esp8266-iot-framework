#pragma once

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

#include "CertStore.h"

class HTTPRequest {

public:

	HTTPRequest() = default;
	explicit HTTPRequest(const String &url);
	virtual ~HTTPRequest();

    void begin(const String &url);
	void end();

	void setAuthorization(const char *user, const char *password);
	void setAuthorization(const char *auth);
	void addHeader(const String &name, const String &value);

	int GET(const String &url);
    int GET();
    int POST(const String &url, const String &body);
    int POST(const String &body);
    int PUT(const String &url, const String &body);
    int PUT(const String &body);
    int PATCH(const String &url, const String &body);
    int PATCH(const String &body);
    int DELETE(const String &url);
    int DELETE();

    bool busy();
    bool available();
    int read();
    String readString();


private :
    BearSSL::CertStore certStore;
	HTTPClient *httpClient = nullptr;
	WiFiClient *wifiClient = nullptr;
	BearSSL::WiFiClientSecure *httpsClient = nullptr;
};
