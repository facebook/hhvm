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
  if (!message.isObject()) {
    return defaultValue;
  }

  try {
    const auto& val = message[key];
    return val.isBool() ? val.getBool() : defaultValue;
  } catch (std::out_of_range&) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

const std::string VSCommand::tryGetString(
  const folly::dynamic& message,
  const char* key,
  const std::string& defaultValue
) {
  if (!message.isObject()) {
    return defaultValue;
  }

  try {
    const auto& val = message[key];
    return val.isString() ? val.getString() : defaultValue;
  } catch (std::out_of_range&) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

const folly::dynamic& VSCommand::tryGetObject(
  const folly::dynamic& message,
  const char* key,
  const folly::dynamic& defaultValue
) {
  if (!message.isObject()) {
    return defaultValue;
  }

  try {
    const auto& val = message[key];
    return val.isObject() ? val : defaultValue;
  } catch (std::out_of_range&) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

static const folly::dynamic s_emptyArray = folly::dynamic::array;
const folly::dynamic& VSCommand::tryGetArray(
  const folly::dynamic& message,
  const char* key
) {
  if (message.isObject()) {
    try {
      const auto& val = message[key];
      if (val.isArray()) {
        return val;
      }
    } catch (std::out_of_range&) {
    }
  }

  return s_emptyArray;
}

int64_t VSCommand::tryGetInt(
  const folly::dynamic& message,
  const char* key,
  const int64_t defaultValue
) {
  if (!message.isObject()) {
    return defaultValue;
  }

  try {
    const auto& val = message[key];
    return val.isInt() ? val.asInt() : defaultValue;
  } catch (std::out_of_range&) {
    // Value not present in dynamic.
    return defaultValue;
  }
}

std::string VSCommand::trimString(const std::string str) {
  auto firstPos = std::find_if_not(
    str.begin(),
    str.end(),
    [](int c){
      return std::isspace(c);
    }
  );

  auto lastPos = std::find_if_not(
    str.rbegin(),
    str.rend(),
    [](int c){
      return std::isspace(c);
    }
  ).base();

  std::string trimmed = firstPos > lastPos
    ? std::string()
    : std::string(firstPos, lastPos);

  return trimmed;
}

std::string VSCommand::removeVariableNamePrefix(const std::string& str) {
  if (str.find("$") == 0) {
    return str.substr(1);
  } else if (str.find("::$") == 0) {
    return str.substr(3);
  } else {
    return str;
  }
}

bool VSCommand::parseCommand(
  Debugger* debugger,
  folly::dynamic& clientMessage,
  VSCommand** command
) {
  assertx(command != nullptr && *command == nullptr);

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

  if (cmdString == "attach" || cmdString == "launch") {

    *command = new LaunchAttachCommand(debugger, clientMessage);

  } else if (cmdString == "completions") {

    *command = new CompletionsCommand(debugger, clientMessage);

  } else if (cmdString == "configurationDone") {

    *command = new ConfigurationDoneCommand(debugger, clientMessage);

  } else if (cmdString == "continue") {

    *command = new ContinueCommand(debugger, clientMessage);

  } else if (cmdString == "evaluate") {

    *command = new EvaluateCommand(debugger, clientMessage);

  } else if (cmdString == "fb_continueToLocation" ||
             cmdString == "continueToLocation") {

    // NOTE: fb_continueToLocation is a Facebook addition to the VS Code
    // debug protocol. Other clients are not expected to send this message
    // since it's not standard, but Nuclide can send it.
    *command = new RunToLocationCommand(debugger, clientMessage);

  } else if (cmdString == "terminateThreads") {

    *command = new TerminateThreadsCommand(debugger, clientMessage);

  } else if (cmdString == "info") {

    *command = new InfoCommand(debugger, clientMessage);

  } else if (cmdString == "initialize") {

    *command = new InitializeCommand(debugger, clientMessage);

  } else if (cmdString == "next" ||
             cmdString == "stepIn" ||
             cmdString == "stepOut") {

    *command = new StepCommand(debugger, clientMessage);

  } else if (cmdString == "pause") {

    *command = new PauseCommand(debugger, clientMessage);

  } else if (cmdString == "scopes") {

    *command = new ScopesCommand(debugger, clientMessage);

  } else if (cmdString == "setBreakpoints" ||
             cmdString == "setFunctionBreakpoints") {

    *command = new SetBreakpointsCommand(debugger, clientMessage);

  } else if (cmdString == "setExceptionBreakpoints") {

    *command = new SetExceptionBreakpointsCommand(debugger, clientMessage);

  } else if (cmdString == "setVariable") {

    *command = new SetVariableCommand(debugger, clientMessage);

  } else if (cmdString == "stackTrace") {

    *command = new StackTraceCommand(debugger, clientMessage);

  } else if (cmdString == "threads") {

    *command = new ThreadsCommand(debugger, clientMessage);

  } else if (cmdString == "variables") {

    *command = new VariablesCommand(debugger, clientMessage);

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
  assertx(m_debugger != nullptr);
  return m_debugger->executeClientCommand(
    this,
    [&](DebuggerSession* session, folly::dynamic& responseMsg) {
      return executeImpl(session, &responseMsg);
    });
}

request_id_t VSCommand::targetThreadId(DebuggerSession* /*session*/) {
  if (commandTarget() != CommandTarget::Request) {
    return -1;
  }

  const folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  return tryGetInt(args, "threadId", -1);
}

const folly::dynamic VSCommand::getDebuggerCapabilities() {
  folly::dynamic capabilities = folly::dynamic::object;

  capabilities["supportsCompletionsRequest"] = true;
  capabilities["supportsConfigurationDoneRequest"] = true;
  capabilities["supportsConditionalBreakpoints"] = true;
  capabilities["supportsFunctionBreakpoints"] = true;
  capabilities["supportsHitConditionalBreakpoints"] = false;
  capabilities["supportsEvaluateForHovers"] = true;
  capabilities["supportsStepBack"] = false;
  capabilities["supportsSetVariable"] = true;
  capabilities["supportsRestartFrame"] = false;
  capabilities["supportsGotoTargetsRequest"] = false;
  capabilities["supportsStepInTargetsRequest"] = false;
  capabilities["supportsModulesRequest"] = false;
  capabilities["supportsRestartRequest"] = false;
  capabilities["supportsExceptionOptions"] = true;
  capabilities["supportsValueFormattingOptions"] = true;
  capabilities["supportsExceptionInfoRequest"] = false;
  capabilities["supportTerminateDebuggee"] = false;
  capabilities["supportsDelayedStackTraceLoading"] = true;
  capabilities["supportsLoadedSourcesRequest"] = false;
  capabilities["supportsTerminateThreadsRequest"] = true;
  capabilities["supportsBreakpointIdOnStop"] = true;

  // Non-standard capability to indicate we send a custom event when
  // the startup doc for the console REPL is complete, so that a client
  // can wait for this event before offering the console.
  capabilities["supportsReadyForEvaluationsEvent"] = true;

  // Non-standard capability to indicate we support the "info" command.
  capabilities["supportsInfo"] = true;

  folly::dynamic exceptionBreakpointFilters = folly::dynamic::array;

  // Filter for all breakpoints.
  folly::dynamic allBreakpointsFilter = folly::dynamic::object;
  allBreakpointsFilter["filter"] = "all";
  allBreakpointsFilter["label"] = "All Exceptions";
  allBreakpointsFilter["default"] = false;
  exceptionBreakpointFilters.push_back(allBreakpointsFilter);

  capabilities["exceptionBreakpointFilters"] =
    std::move(exceptionBreakpointFilters);

  return capabilities;
}

}
}
