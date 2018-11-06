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

ContinueCommand::ContinueCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

ContinueCommand::~ContinueCommand() {
}

ContinueCommand* ContinueCommand::createInstance(Debugger* debugger) {
  return new ContinueCommand(debugger, folly::dynamic::object);
}

bool ContinueCommand::executeImpl(DebuggerSession* /*session*/,
                                  folly::dynamic* responseMsg) {
  const folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  request_id_t threadId = tryGetInt(args, "threadId", -1);

  if (threadId >= 0) {
    // If a thread ID was specified, check if the thread with that ID is in
    // the middle of an evaluation - if it was evaluating and hit another bp,
    // resume just that thread. Otherwise, resume all threads.
    DebuggerRequestInfo* ri = m_debugger->getRequestInfo(threadId);
    if (ri != nullptr && ri->m_pauseRecurseCount > 1) {
      VSCommand* resumeCommand = ContinueCommand::createInstance(m_debugger);
      ri->m_commandQueue.dispatchCommand(resumeCommand);
      return false;
    } else {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelWarning,
        "Client asked to resume thread from eval, but thread was not evaluating"
      );
    }
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Client sent continue command."
  );

  folly::dynamic body = folly::dynamic::object;
  body["allThreadsContinued"] = false;
  (*responseMsg)["body"] = body;

  phpDebuggerContinue();
  return true;
}

}
}
