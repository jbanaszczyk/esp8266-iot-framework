#include <Arduino.h>
#include <SimpleLogging.h>

void doSomeLogs() {
	Serial.println("Logging logger for level ERROR ...");
	static SimpleLogging::Logger *logger = Logging.getLogger("SomeLogs", Logging.ERROR);

	// Expectation: message will be shown
	logger->critical.println("Message: critical");

	// Expectation: message will be shown
	logger->error.println("Message: error");

	// Expectation: message will not be shown due to minimumLevel in logger (above)
	logger->warning.println("Message: warning");

	// Expectation: message will not be shown due to minimumLevel in logger (above)
	logger->notice.println("Message: notice");

	// Expectation: message will not be shown due to minimumLevel in logger (above)
	logger->info.println("Message: info");

	// Expectation: message will not be shown due to minimumLevel in logger (above)
	logger->debug.println("Message: debug");

	// Expectation: message will not be shown due to minimumLevel in logger and lack of handler in Logging (below)
	logger->trace.println("Message: trace");

	// Expectation: message will be shown
	// Serial.println inside lambda will be executed
	logger->error.lazyLog([]() -> const char * {
		Serial.println("Inside lazy evaluated logCreator for ERROR");
		return "Error: Lazy evaluation\n";
	});

	// Expectation: message will not be shown due to minimumLevel in logger (above)
	// Important Serial.println inside lambda will not be executed
	logger->debug.lazyLog([]() -> const char * {
		Serial.println("Inside lazy evaluated logCreator for DEBUG");
		return "Debug: Lazy evaluation\n";
	});
}

void setupSerial(unsigned long baud = 74880) {
	Serial.begin(baud);
	while (!Serial) {}
	Serial.println();
}

void setup() {
	setupSerial();

	Serial.println("------------------------");
	Serial.println("Logging handler for port Serial, level DEBUG ...");
	Logging.addHandler(Serial, Logging.DEBUG);
	doSomeLogs();
	Serial.println("------------------------");
}

void loop() {}
