#include <Arduino.h>
#include <SerialCommands.h>

void doHello(SerialCommands *sender) {
	if (sender->getStream() != nullptr) {
		sender->getStream()->println("Ello, ello.");
	}
}

void doMore(SerialCommands *sender) {
	if (sender->getStream() != nullptr) {
		sender->getStream()->println("more");
	}
}

void doLess(SerialCommands *sender) {
	if (sender->getStream() != nullptr) {
		sender->getStream()->println("less");
	}
}

void doNothing(SerialCommands *sender) {
	if (sender->getStream() != nullptr) {
		sender->getStream()->println("nothing");
	}
}

void doHelp(SerialCommands *sender) {
	if (sender->getStream() != nullptr) {
		sender->printHelp();
	}
}

void doSthWithArgs(SerialCommands *sender) {
	auto stream = sender->getStream();
	if (stream != nullptr) {
		stream->printf("Got command\n");
		decltype(sender->Next()) argument;
		while ((argument = sender->Next()) != nullptr) {
			stream->printf("\targument: %s\n", argument);
		}
		stream->printf("=================\n");
	}
}

void iDontKnowWhatShouldIDo(SerialCommands *sender, const char *cmd) {
	auto stream = sender->getStream();
	if (stream != nullptr) {
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
}

static SerialCommands *serialCommands = nullptr;

void setup() {
	Serial.begin(74880);
	while (!Serial) {}
	Serial.println();

	serialCommands = new SerialCommands();

	serialCommands->setDefaultHandler(iDontKnowWhatShouldIDo);

	serialCommands->AddCommand("x", "Example command with arguments", doSthWithArgs);

	serialCommands->AddCommand("Hello", nullptr, doHello);

	serialCommands->AddCommand('+', "Do more", doMore);
	serialCommands->AddCommand('-', "Do less", doLess);
	serialCommands->AddCommand('0', nullptr, doNothing);

	serialCommands->AddCommand("help", "Show help", doHelp);
	serialCommands->AddCommand('?', "Show help", doHelp);

	Serial.printf("Press ? to get help\n");
}

void loop() {
	if (serialCommands != nullptr) {
		serialCommands->ReadSerial();
	}
}
