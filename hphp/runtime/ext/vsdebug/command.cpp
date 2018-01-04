/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/command.h"

namespace HPHP {
namespace VSDEBUG {

const folly::dynamic VSCommand::s_emptyArgs = folly::dynamic::object;

VSCommand::VSCommand(Debugger* debugger, folly::dynamic message) :
  m_message(message),
  m_debugger(debugger) {
}

VSCommand::~VSCommand() {
}

bool VSCommand::tryGetBool(
  const folly::dynamic& message,
  const char* key,
  bool defaultValue
) {
  try {
    const auto& val = message[key];
    return val.isBool() ? val.getBool() : defaultValue;
  } catch (std::out_of_range e) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

const std::string& VSCommand::tryGetString(
  const folly::dynamic& message,
  const char* key,
  const std::string& defaultValue
) {
  try {
    const auto& val = message[key];
    return val.isString() ? val.getString() : defaultValue;
  } catch (std::out_of_range e) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

const folly::dynamic& VSCommand::tryGetObject(
  const folly::dynamic& message,
  const char* key,
  const folly::dynamic& defaultValue
) {
  try {
    const auto& val = message[key];
    return val.isObject() ? val : defaultValue;
  } catch (std::out_of_range e) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

int64_t VSCommand::tryGetInt(
  const folly::dynamic& message,
  const char* key,
  const int64_t defaultValue
) {
 try {
   const auto& val = message[key];
   return val.isInt() ? val.asInt() : defaultValue;
 } catch (std::out_of_range e) {
   // Value not present in dynamic.
   return defaultValue;
 }
}

bool VSCommand::parseCommand(
  Debugger* debugger,
  folly::dynamic& clientMessage,
  VSCommand** command
) {
  assert(command != nullptr && *command == nullptr);

  // Only VS Code debug protocol messages of type "request" are expected from
  // the client.
  const std::string& type = tryGetString(clientMessage, "type", "");
  if (type != "request") {
    throw DebuggerCommandException("Invalid message type.");
  }

  const std::string& cmdString = tryGetString(clientMessage, "command", "");
  if (cmdString.empty()) {
    throw DebuggerCommandException("Invalid command.");
  }

  if (cmdString == "initialize") {

    *command = new InitializeCommand(debugger, clientMessage);

  } else if (cmdString == "configurationDone") {

    *command = new ConfigurationDoneCommand(debugger, clientMessage);

  } else if (cmdString == "launch" || cmdString == "attach") {

    *command = new LaunchAttachCommand(debugger, clientMessage);

  } else if (cmdString == "continue") {

    *command = new ContinueCommand(debugger, clientMessage);

  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "No command implemented to process message: %s",
      folly::toJson(clientMessage).c_str()
    );
  }

  return *command != nullptr;
}

bool VSCommand::execute() {
  assert(m_debugger != nullptr);
  return m_debugger->executeClientCommand(
    this,
    [&](folly::dynamic& responseMsg) {
      return executeImpl(&responseMsg);
    });
}

int64_t VSCommand::defaultGetTargetThreadId() {
  const folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  return tryGetInt(args, "threadId", -1);
}

}
}
