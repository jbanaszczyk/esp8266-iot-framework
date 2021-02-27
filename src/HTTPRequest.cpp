#include "HTTPRequest.h"

#include <WiFiClientSecureBearSSL.h>

HTTPRequest::HTTPRequest(const String &url) : HTTPRequest() {
	begin(url);
}

HTTPRequest::~HTTPRequest() {
	end();
}

void HTTPRequest::begin(const String &url) {
	if (httpClient == nullptr) {

		httpClient = new HTTPClient();

		if (url != nullptr && url.startsWith(PSTR("https://"))) {

			httpsClient = new BearSSL::WiFiClientSecure();

#ifdef forceUseMFLN
			if (BearSSL::WiFiClientSecure::probeMaxFragmentLength(url, 443, 512)) {
				httpsClient->setBufferSizes(512, 512);
			}
#endif

			httpsClient->setCertStore(&certStore);
			httpClient->begin(*httpsClient, url);
			wifiClient = httpsClient;
		} else {
			wifiClient = new WiFiClient();
			httpClient->begin(*wifiClient, url);
		}
		httpClient->setReuse(false);
	}
}

void HTTPRequest::end() {
	delete httpClient;
	if (wifiClient != httpsClient) {
		delete wifiClient;
	}
	delete httpsClient;

	httpsClient = nullptr;
	httpClient = nullptr;
	wifiClient = nullptr;
}

void HTTPRequest::setAuthorization(const char *user, const char *password) {
	if (httpClient != nullptr) {
		httpClient->setAuthorization(user, password);
	}
}

void HTTPRequest::setAuthorization(const char *auth) {
	if (httpClient != nullptr) {
		httpClient->setAuthorization(auth);
	}
}

void HTTPRequest::addHeader(const String &name, const String &value) {
	httpClient->addHeader(name, value);
}

int HTTPRequest::GET(const String &url) {
	begin(url);
	return httpClient->GET();
}

int HTTPRequest::POST(const String &url, const String &body) {
	begin(url);
	return httpClient->POST(body);
}

int HTTPRequest::PUT(const String &url, const String &body) {
	begin(url);
	return httpClient->PUT(body);
}

int HTTPRequest::PATCH(const String &url, const String &body) {
	begin(url);
	return httpClient->PATCH(body);
}

int HTTPRequest::DELETE(const String &url) {
	begin(url);
	return httpClient->sendRequest("DELETE");
}

int HTTPRequest::GET() {
	if (httpClient == nullptr) {
		return HTTP_CODE_BAD_REQUEST;
	}
	return httpClient->GET();
}

int HTTPRequest::POST(const String &body) {
	if (httpClient == nullptr) {
		return HTTP_CODE_BAD_REQUEST;
	}
	return httpClient->POST(body);
}

int HTTPRequest::PUT(const String &body) {
	if (httpClient == nullptr) {
		return HTTP_CODE_BAD_REQUEST;
	}
	return httpClient->PUT(body);
}

int HTTPRequest::PATCH(const String &body) {
	if (httpClient == nullptr) {
		return HTTP_CODE_BAD_REQUEST;
	}
	return httpClient->PATCH(body);
}

int HTTPRequest::DELETE() {
	if (httpClient == nullptr) {
		return HTTP_CODE_BAD_REQUEST;
	}
	return httpClient->sendRequest("DELETE");
}

bool HTTPRequest::busy() {
	return wifiClient == nullptr || wifiClient->connected() || wifiClient->available();
}

bool HTTPRequest::available() {
	return ((wifiClient != nullptr) && static_cast<bool>(wifiClient->available()));
}

int HTTPRequest::read() {
	return (wifiClient != nullptr) ? wifiClient->read() : -1;
}

String HTTPRequest::readString() {
	return (wifiClient != nullptr) ? wifiClient->readString() : "";
}
