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

TerminateThreadsCommand::TerminateThreadsCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_requestId(-1) {
}

TerminateThreadsCommand::TerminateThreadsCommand(
  Debugger* debugger,
  folly::dynamic message,
  request_id_t requestId
) : VSCommand(debugger, message),
    m_requestId(requestId) {
}

TerminateThreadsCommand::~TerminateThreadsCommand() {
}

TerminateThreadsCommand* TerminateThreadsCommand::createInstance(
  Debugger* debugger,
  folly::dynamic message,
  request_id_t requestId
) {
  return new TerminateThreadsCommand(debugger, message, requestId);
}

CommandTarget TerminateThreadsCommand::commandTarget() {
  if (m_requestId > 0) {
    return CommandTarget::WorkItem;
  }

  return CommandTarget::None;
}

bool TerminateThreadsCommand::executeImpl(DebuggerSession* /*session*/,
                                          folly::dynamic* /*responseMsg*/
) {
  if (m_requestId > 0) {
    DebuggerRequestInfo* ri = m_debugger->getRequestInfo();
    ri->m_flags.terminateRequest = true;
    RID().setDebuggerIntr(true);
    return true;
  }

  const folly::dynamic& message = getMessage();
  const auto dispatchRequest = [&](request_id_t requestId) {
    if (requestId > 0) {
      m_debugger->dispatchCommandToRequest(
        requestId,
        createInstance(m_debugger, message, requestId));
    }
  };

  try {
    const auto& args = tryGetObject(message, "arguments", s_emptyArgs);
    const auto& val = args["threadIds"];
    if (val.isArray()) {
      for (const auto& requestIdVal : val) {
        auto requestId = requestIdVal.isInt() ? requestIdVal.asInt() : -1;
        dispatchRequest(requestId);
      }
    }
  } catch (std::out_of_range& e) {
  }

  // Do not resume target.
  return false;
}

}
}
