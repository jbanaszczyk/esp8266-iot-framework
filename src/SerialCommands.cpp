
#include "SerialCommands.h"

std::_List_iterator<std::unique_ptr<SerialCommands::SerialCommand>> SerialCommands::findCommand(std::list<std::unique_ptr<SerialCommands:: SerialCommand>> *commandsList, const String &commandName, bool caseSensitive) {
	return std::find_if(
			std::begin(*commandsList),
			std::end(*commandsList),
			[&](std::unique_ptr<SerialCommand> const &p) { return caseSensitive ? p->command.equals(commandName) : p->command.equalsIgnoreCase(commandName); });
}

bool SerialCommands::AddCommand(SerialCommand *command) {
	logger->debug.printf("Adding cmd=[%s] %s\n", command->command.c_str(), command->oneKey ? ("as one-key") : "");
	auto *selectedCommands = command->oneKey ? &oneKeyCommands : &commands;
	selectedCommands->push_back(std::unique_ptr<SerialCommand>(new SerialCommand(*command)));
	delete command;
	return true;
}

bool SerialCommands::AddCommand(char cmd, const char *description, void (*func)(SerialCommands *)) {
	char fullCmd[] = " ";
	fullCmd[0] = cmd;
	return AddCommand(new SerialCommand(fullCmd, description, func, true));
}

bool SerialCommands::AddCommand(const char *cmd, const char *description, void (*func)(SerialCommands *)) {
	return AddCommand(new SerialCommand(cmd, description, func, false));
}

SerialCommands::Status SerialCommands::ReadSerial() {
	if (stream == nullptr) {
		return Status::NoSerial;
	}

	while (stream->available() > 0) {
		int ch = stream->read();
		logger->debug.printf("Read: bufPos=%zu bufLen=%zu", bufferPosition, bufferLength);
		if (ch < ' ') {
			logger->debug.printf(" code=#%d", ch);
		} else {
			logger->debug.printf(" ascii=[%c]", static_cast<char>(ch));
		}
		logger->debug.println();
		if (ch <= 0) {
			continue;
		}

		ch = transpose(ch);

		if (ch == '\0' && bufferPosition == 0) {
			continue;   // LF from previous CRLF
		}

		if (ch == '\b') {
			if (bufferPosition > 0) {
				doEcho(ch);
				bufferPosition--;
			}
			return Status::Success;
		}

		if (ch == '\e') {
			doEcho(ch);
			ClearBuffer();
			return Status::Success;
		}

		if (bufferPosition == 0 && checkOneKeyCmd(static_cast<char>(ch), caseSensitive)) {
			return Status::Success;
		}

		if (ch == ' ' && bufferPosition == 0) {
			continue;
		}

		doEcho(ch);

		if (bufferPosition < bufferLength) {
			buffer[bufferPosition++] = static_cast<char>(ch);
		} else {
			logger->notice.print("Buffer full\n");
			doEcho("!!!\r");
			ClearBuffer();
			return Status::BufferFull;
		}

		if (ch == '\0') {
			logger->debug.printf("Checking command [%s]\n", buffer);
			char *command = strtok_r(buffer, delimiter.c_str(), &lastToken);
			if (command != nullptr) {
				auto found = findCommand(&commands, command, caseSensitive);
				if (found != commands.end()) {
					logger->debug.printf("Found command [%s]\n", command);
					found->get()->func(this);
				} else if (defaultHandler != nullptr) {
					(*defaultHandler)(this, command);
				}
			}
			ClearBuffer();
		}
	}
	return Status::Success;
}

void SerialCommands::doEcho(const char *txt) const {
	if (echo && stream != nullptr) {
		stream->print(txt);
	}
}

void SerialCommands::doEcho(int ch) const {
	if (echo && stream != nullptr) {
		switch (ch) {
			case '\0': {
				stream->print("\n");
				break;
			}
			case '\e': {
				stream->print('\r');
				break;
			}
			default: {
				stream->print(static_cast<char>(ch));
				break;
			}
		}
	}
}

int SerialCommands::transpose(int ch) {
	switch (ch) {
		case '\r':
		case '\n':
			return '\0';
		case '\t':
			return ' ';
		case '\x7f':
			return '\e';
		default:
			return ch;
	}
}

bool SerialCommands::checkOneKeyCmd(int ch, bool caseSensitive) {
	logger->debug.printf("Checking oneKey command [%c]\n", ch);
	char cmd[] = " ";
	cmd[0] = static_cast<char>(ch);
	auto found = findCommand(&oneKeyCommands, cmd, caseSensitive);
	if (found == oneKeyCommands.end()) {
		return false;
	}
	logger->debug.printf("Found oneKey command [%s]\n", cmd);
	found->get()->func(this);
	return true;
}

void SerialCommands::ClearBuffer() {
	buffer[0] = '\0';
	bufferPosition = 0;
}

const char *SerialCommands::Next() {
	return strtok_r(nullptr, delimiter.c_str(), &lastToken);
}

SerialCommands::~SerialCommands() {
	unsetStream();
	if (ownedBuffer) {
		logger->debug.print("Disposing command buffer\n");
		delete[] buffer;
	}
}

void SerialCommands::cmd_unrecognized(SerialCommands *sender, const char *cmd) {
	auto stream = sender->getStream();
	if (stream != nullptr) {
		stream->printf("Unrecognized command [%s]\n", cmd);
		decltype(sender->Next()) argument;
		while ((argument = sender->Next()) != nullptr) {
			stream->printf("\targument: [%s]\n", argument);
		}
	}
}

void SerialCommands::printHelpHelper(std::list<std::unique_ptr<SerialCommand>> &commandsList, const char *title, bool oneKeys) const {
	if (stream != nullptr) {
		stream->printf("%s:\n", title);
		String templateFull = oneKeys ? keyCommandTemplateFull : commandTemplateFull;
		String templatePartial = oneKeys ? keyCommandTemplatePartial : commandTemplatePartial;
		for (auto &it : commandsList) {
			String command = it->command;
			if (!caseSensitive) {
				command.toLowerCase();
			}
			String description = it->description;
			description.replace("\n", commandTemplateNewLineReplacement);
			String selectedTemplate = (description != nullptr && description[0] != '\0') ? templateFull : templatePartial;
			selectedTemplate.replace("{command}", command);
			selectedTemplate.replace("{description}", description);
			stream->printf("%s\n", selectedTemplate.c_str());
		}
	}
}

void SerialCommands::printHelp() {
	if (stream != nullptr) {
		printHelpHelper(commands, "Commands", false);
		printHelpHelper(oneKeyCommands, "OneKey commands", true);
	}
}
