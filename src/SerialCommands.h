#pragma once

#include <Arduino.h>
#include <SimpleLogging.h>
#include <list>
#include <memory>

#ifndef DEBUG_SERIAL_COMMANDS
#define DEBUG_SERIAL_COMMANDS NOTICE
#endif

class SerialCommand;

class SerialCommands {
public:
	enum class Status {
		Success,
		NoSerial,
		BufferFull
	};

	class SerialCommand {
	public:

		SerialCommand(const char *cmd, const char *description, void (*func)(SerialCommands *), bool oneKey = false) :
				func(func),
				command(cmd),
				description(description),
				oneKey(oneKey),
				next(nullptr) {}

		SerialCommand(SerialCommand const &origin) :
				func(origin.func),
				oneKey(origin.oneKey),
				next(nullptr) {
			this->command = origin.command;
			this->description = origin.description;
		}

		void (*func)(SerialCommands *);

		String command;
		String description;
		bool oneKey;
		SerialCommand *next;
	};

	explicit SerialCommands(Stream *serial = &Serial, char *buffer = nullptr, size_t bufferLen = 64, bool caseSensitive = false, bool echo = true, String delimiter = " ") :
			stream(serial),
			ownedBuffer(buffer == nullptr),
			buffer(buffer == nullptr ? new char[bufferLen] : buffer),
			bufferLength(bufferLen - 1), //string termination char '\0'
			caseSensitive(caseSensitive),
			echo(echo),
			delimiter(std::move(delimiter)),
			bufferPosition(0),
			lastToken(nullptr),
			defaultHandler(cmd_unrecognized) {}

	virtual ~SerialCommands();

	/**
	 * \brief Adds a command handler (Uses a linked list)
	 * \param command
	 */
	bool AddCommand(char cmd, const char *description, void (*func)(SerialCommands *));

	bool AddCommand(const char *cmd, const char *description, void (*func)(SerialCommands *));

	/**
	 * \brief Checks the Serial port, reads the input buffer and calls a matching command handler.
	 * \return Success when successful or SerialCommandError on error.
	 */
	Status ReadSerial();

	/**
	 * \brief Returns the source stream (Serial port)
	 * \return 
	 */
	Stream *getStream() { return stream; }

	/**
	 * \brief Attaches a Serial Port to this object 
	 * \param serial 
	 */

	void setStream(Stream *stream) { SerialCommands::stream = stream; }

	/**
	 * \brief Detaches the serial port, if no serial port nothing will be done at ReadSerial
	 */
	void unsetStream() { stream = nullptr; }

	/**
	 * \brief Sets a default handler can be used for a catch all or unrecognized commands
	 * \param func 
	 */

	void setDefaultHandler(void (*defaultHandler)(SerialCommands *, const char *)) { SerialCommands::defaultHandler = defaultHandler; }

	/**
	 * \brief Clears the buffer, and resets the indexes.
	 */
	void ClearBuffer();

	/**
	 * \brief Gets the next argument
	 * \return returns NULL if no argument is available
	 */
	const char *Next();

	static void cmd_unrecognized(SerialCommands *sender, const char *cmd);

	void printHelp();

private:
	constexpr static const char *const commandTemplatePartial = "\t{command}";
	constexpr static const char *const commandTemplateFull = "\t{command}\n\t\t{description}";
	constexpr static const char *const keyCommandTemplatePartial = "\t{command}";
	constexpr static const char *const keyCommandTemplateFull = "\t{command}\t{description}";
	constexpr static const char *const commandTemplateNewLineReplacement = "\n\t\t";

	SimpleLogging::Logger *logger = Logging.getLogger("SerialCommands", Logging.DEBUG_SERIAL_COMMANDS);;

	Stream *stream;
	bool ownedBuffer;
	char *buffer;
	size_t bufferLength;
	bool caseSensitive;
	bool echo;
	String delimiter;
	size_t bufferPosition;
	char *lastToken;
	std::list<std::unique_ptr<SerialCommand>> commands{};
	std::list<std::unique_ptr<SerialCommand>> oneKeyCommands{};

	void (*defaultHandler)(SerialCommands *, const char *);

	bool checkOneKeyCmd(int ch, bool caseSensitive);

	bool AddCommand(SerialCommand *command);

	static int transpose(int ch);

	void doEcho(int ch) const;

	void doEcho(const char *txt) const;

	void printHelpHelper(std::list<std::unique_ptr<SerialCommand>> &commandsList, const char *title, bool oneKeys) const;

	static std::_List_iterator<std::unique_ptr<SerialCommand>> findCommand(std::list<std::unique_ptr<SerialCommand>> *commandsList, const String &commandName, bool caseSensitive);
};
