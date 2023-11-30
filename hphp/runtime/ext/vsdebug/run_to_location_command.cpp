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

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/debugger-request-info.h"
#include "hphp/runtime/ext/vsdebug/command.h"

namespace HPHP {
namespace VSDEBUG {

RunToLocationCommand::RunToLocationCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

RunToLocationCommand::~RunToLocationCommand() {
}

bool RunToLocationCommand::executeImpl(DebuggerSession* session,
                                       folly::dynamic* /*responseMsg*/
) {
  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const folly::dynamic& source = tryGetObject(args, "source", s_emptyArgs);
  const std::string& filePath = tryGetString(source, "path", "");

  if (filePath.empty()) {
    throw DebuggerCommandException(
      "Run to location source path not specified."
    );
  }

  const ClientPreferences& prefs = m_debugger->getClientPreferences();
  const std::string& path = File::TranslatePath(String(filePath)).toCppString();
  BreakpointManager* bpMgr = session->getBreakpointManager();

  int line = tryGetInt(args, "line", -1);
  if (!prefs.linesStartAt1) {
    // If client lines start at 0, make them 1 based.
    line++;
  }

  if (line <= 0) {
    throw DebuggerCommandException(
      "Invalid continue to line specified."
    );
  }

  // See if there's already a breakpoint at this file + line.
  const auto bpIds = bpMgr->getBreakpointIdsForPath(path);
  for (auto it = bpIds.begin(); it != bpIds.end(); it++) {
    Breakpoint* bp = bpMgr->getBreakpointById(*it);
    if (bp->m_line == line) {
      // There's already a breakpoint installed at the run to location.
      // Just resume the request thread and let it hit.
      return true;
    }
  }

  // Find a compilation unit to place a temp bp in.
  HPHP::String unitPath(path.c_str());
  const auto compilationUnit = lookupUnit(unitPath.get(), "", nullptr, nullptr,
                                          false);
  if (compilationUnit == nullptr) {
    throw DebuggerCommandException(
      "Could not find a loaded compilation unit to run to location in!"
    );
  }

  std::pair<int, int> runLocation =
    m_debugger->calibrateBreakpointLineInUnit(compilationUnit, line);

  if (runLocation.first < 0) {
    throw DebuggerCommandException(
      "Could not find a suitable location in the compilation unit to run to!"
    );
  }

  if (!phpAddBreakPointLine(compilationUnit, runLocation.first)) {
    throw DebuggerCommandException(
      "Failed to install temporary breakpoint at location."
    );
  }

  DebuggerRequestInfo* ri = m_debugger->getRequestInfo();
  ri->m_runToLocationInfo.path = path;
  ri->m_runToLocationInfo.line = line;

  // Tell the user where we're running to. Resolving the file path and
  // calibrating the source line could have modified things a bit.
  std::string userMsg = "Resuming request ";
  userMsg += std::to_string(targetThreadId(session));
  userMsg += " and running to resolved location ";
  userMsg += path;
  userMsg += ":";
  userMsg += std::to_string(runLocation.second);
  m_debugger->sendUserMessage(userMsg.c_str(), DebugTransport::OutputLevelInfo);

  // Resume only this request thread.
  return true;
}

}
}
