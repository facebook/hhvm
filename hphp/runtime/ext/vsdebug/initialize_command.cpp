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

InitializeCommand::InitializeCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

InitializeCommand::~InitializeCommand() {
}

bool InitializeCommand::executeImpl(DebuggerSession* /*session*/,
                                    folly::dynamic* responseMsg) {
  const folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  ClientPreferences preferences = {0};

  preferences.linesStartAt1 = tryGetBool(args, "linesStartAt1", true);
  preferences.columnsStartAt1 = tryGetBool(args, "columnsStartAt1", true);

  preferences.supportsVariableType =
    tryGetBool(args, "supportsVariableType", true);

  preferences.supportsVariablePaging =
    tryGetBool(args, "supportsVariablePaging", true);

  preferences.supportsRunInTerminalRequest =
      tryGetBool(args, "supportsRunInTerminalRequest", true);

  const auto& pathFormat = tryGetString(args, "pathFormat", "path");
  if (pathFormat.compare("uri") == 0) {
    preferences.pathFormat = PathFormat::URI;
  } else {
    preferences.pathFormat = PathFormat::Path;
  }

  m_debugger->setClientPreferences(preferences);

  // Prepare a response for the client.
  (*responseMsg)["body"] = getDebuggerCapabilities();

  // Completion of this command does not resume the target.
  return false;
}

}
}
