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

void doHelp(SerialCommands *sender) {
	sender->printHelp();
}

static SerialCommands serialCommands{}; // NOLINT(cppcoreguidelines-interfaces-global-init)

void setup() {
	Serial.begin(74880);
	while (!Serial) {}
	serialCommands.setStream(&Serial); // Avoids problems with early initialized 'Serial' and `serialCommands`
	serialCommands.AddCommand("Hello", nullptr, doHello);
	serialCommands.AddCommand('+', "Do more", doMore);
	serialCommands.AddCommand('-', "Do less", doLess);
	serialCommands.AddCommand('?', "Display help", doHelp);
	serialCommands.AddCommand("help", "Display help", doHelp);
	serialCommands.getStream()->printf("Press '?' for help\n");
}

void loop() {
	serialCommands.ReadSerial();
}
