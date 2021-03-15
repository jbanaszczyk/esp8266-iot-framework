#include <Arduino.h>
#include <SerialCommands.h>

void doHello(SerialCommands *sender) {
	sender->getStream()->println("Ello, ello.");
}

void doMore(SerialCommands *sender) {
	sender->getStream()->println("more");
}

void doLess(SerialCommands *sender) {
	sender->getStream()->println("less");
}

void doNothing(SerialCommands *sender) {
	sender->getStream()->println("nothing");
}

void doHelp(SerialCommands *sender) {
	sender->printHelp();
}

void doSthWithArgs(SerialCommands *sender) {
	Stream *stream = sender->getStream();
	stream->printf("Got command\n");
	decltype(sender->Next()) argument;
	while ((argument = sender->Next()) != nullptr) {
		stream->printf("\targument: %s\n", argument);
	}
	stream->printf("=================\n");
}

void iDontKnowWhatShouldIDo(SerialCommands *sender, const char *cmd) {
	Stream *stream = sender->getStream();
	stream->printf("Got unrecognized command [%s]", cmd);
	auto counter = 0U;
	decltype(sender->Next()) argument;
	while ((argument = sender->Next()) != nullptr) {
		if (counter == 0) {
			stream->printf("\tArguments:");
		}
		stream->printf(" #%d:{%s}", counter, argument);
		++counter;
	}
	stream->printf("\n");
}

void setup() {
	Serial.begin(74880);
	while (!Serial) {}
	auto memBefore = ESP.getFreeHeap();

	auto processor = new SerialCommands();
	processor->setDefaultHandler(nullptr);
	processor->AddCommand("Hello", nullptr, doHello);
	processor->AddCommand("x", "Example command with arguments", doSthWithArgs);
	processor->AddCommand('+', "Do more", doMore);
	processor->AddCommand('=', "Do more", doMore);
	processor->AddCommand('-', "Do less", doLess);
	processor->AddCommand('_', "Do less", doLess);
	processor->AddCommand('0', nullptr, doNothing);
	processor->AddCommand("help", "Show help", doHelp);
	processor->AddCommand('?', "Show help", doHelp);
	delete processor;

	auto memAfter = ESP.getFreeHeap();

	if ((memBefore - memAfter) != 0) {
		Serial.printf("==[ Memory lost ]======= getFreeHeap: before:%u after:%u lost:%u\n", memBefore, memAfter, memBefore - memAfter);
	} else {
		Serial.printf("==[ No memory leak ]======= getFreeHeap: before:%u and after:%u\n", memBefore, memAfter);
	}
}

void loop() {
	// There is no active command processor
}
