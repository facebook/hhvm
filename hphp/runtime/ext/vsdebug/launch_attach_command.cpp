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

LaunchAttachCommand::LaunchAttachCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

LaunchAttachCommand::~LaunchAttachCommand() {
}

bool LaunchAttachCommand::executeImpl(DebuggerSession* /*session*/,
                                      folly::dynamic* responseMsg) {
  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const std::string emptyString = std::string("");

  const auto& sandboxUser =
    tryGetString(args, "sandboxUser", emptyString);

  const auto& sandboxName =
    tryGetString(args, "sandboxName", "default");

  const auto& startupDoc =
    tryGetString(args, "startupDocumentPath", emptyString);

  bool displayStartupMsg =
    tryGetBool(args, "displayConsoleStartupMsg", true);

  bool showDummyOnAsyncPause =
     tryGetBool(args, "showDummyOnAsyncPause", false);

  bool warnOnInterceptedFunctions =
    tryGetBool(args, "warnOnInterceptedFunctions", false);

  bool notifyOnBpCalibration =
    tryGetBool(args, "notifyOnBpCalibration", false);

  bool disableUniqueVarRef =
    tryGetBool(args, "disableUniqueVarRef", false);

  bool disablePostDummyEvalHelper =
    tryGetBool(args, "disablePostDummyEvalHelper", false) ||
    // TODO: make clients use the new flag name and remove this legacy version
    tryGetBool(args, "disableDummyPsPs", false);

  bool disableStdoutRedirection =
    tryGetBool(args, "disableStdoutRedirection", false);

  bool disableJit =
    tryGetBool(args, "disableJit", RuntimeOption::EvalJitDisabledByVSDebug);

  const auto& logFilePath =
    tryGetString(args, "logFilePath", emptyString);

  const auto debuggerSessionAuthToken =
    tryGetString(args, "debuggerSessionAuthToken", emptyString);

  auto maxReturnedStringLength =
    tryGetInt(args, "maxReturnedStringLength", 32 * 1024);

  if (!logFilePath.empty()) {
    // Re-open logging using the file path specified by the client.
    int result = VSDebugLogger::InitializeLogging(logFilePath);
    if (result != 0) {
      std::string msg = "Opening log file ";
      msg += logFilePath.c_str();
      msg += " failed: ";
      msg += strerror(result);
      m_debugger->sendUserMessage(
        msg.c_str(),
        DebugTransport::OutputLevelWarning
      );
    }
  }

  // Obviously don't log the auth token itself, but log whether or not we've
  // got one.
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Client provided an auth token? %s",
    debuggerSessionAuthToken.empty() ? "NO" : "YES"
  );

  if (!startupDoc.empty()) {
    m_debugger->startDummyRequest(
      startupDoc,
      sandboxUser,
      sandboxName,
      debuggerSessionAuthToken,
      displayStartupMsg
    );
  } else {
    m_debugger->startDummyRequest(
      emptyString,
      sandboxUser,
      sandboxName,
      debuggerSessionAuthToken,
      displayStartupMsg
    );
  }

  DebuggerOptions options = {0};
  options.showDummyOnAsyncPause = showDummyOnAsyncPause;
  options.warnOnInterceptedFunctions = warnOnInterceptedFunctions;
  options.notifyOnBpCalibration = notifyOnBpCalibration;
  options.disableUniqueVarRef = disableUniqueVarRef;
  options.disablePostDummyEvalHelper = disablePostDummyEvalHelper;
  options.maxReturnedStringLength = maxReturnedStringLength;
  options.disableStdoutRedirection = disableStdoutRedirection;
  options.disableJit = disableJit;
  m_debugger->setDebuggerOptions(options);

  // Send the InitializedEvent to indicate to the front-end that we are up
  // and ready for breakpoint requests.
  folly::dynamic event = folly::dynamic::object;
  m_debugger->sendEventMessage(event, "initialized");

  // Re-send debugger capabilities in the launch / attach command response.
  // Note: while this is not documented in the protocol, VS Code uses this
  // to update capabilities after attaching in case they change after knowing
  // what version of the engine is launched.
  (*responseMsg)["body"] = getDebuggerCapabilities();

  // Completion of this command does not resume the target.
  return false;
}

}
}
