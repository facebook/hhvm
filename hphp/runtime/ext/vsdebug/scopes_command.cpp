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

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

ScopesCommand::ScopesCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_frameId{0} {

  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const int frameId = tryGetInt(args, "frameId", -1);
  m_frameId = frameId;
}

ScopesCommand::~ScopesCommand() {
}

FrameObject* ScopesCommand::getFrameObject(DebuggerSession* session) {
  if (m_frameObj != nullptr) {
    return m_frameObj;
  }

  m_frameObj = session->getFrameObject(m_frameId);
  return m_frameObj;
}

request_id_t ScopesCommand::targetThreadId(DebuggerSession* session) {
  FrameObject* frame = getFrameObject(session);
  if (frame == nullptr) {
    return Debugger::kDummyTheadId;
  }

  return frame->m_requestId;
}

bool ScopesCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  // The request thread should not re-enter the debugger while
  // processing this command.
  DebuggerNoBreakContext noBreak(m_debugger);

  folly::dynamic body = folly::dynamic::object;
  folly::dynamic scopes = folly::dynamic::array;

  if (auto const frame = getFrameObject(session)) {
    session->setCurrFrameId(m_frameId);
    scopes.push_back(getScopeDescription(session,
                                         frame,
                                         "Locals",
                                         ScopeType::Locals,
                                         false));
    scopes.push_back(getScopeDescription(session,
                                         frame,
                                         "Superglobals",
                                         ScopeType::Superglobals,
                                         false));
    scopes.push_back(getScopeDescription(session,
                                         frame,
                                         "Constants",
                                         ScopeType::ServerConstants,
                                         true));
  }

  body["scopes"] = scopes;
  (*responseMsg)["body"] = body;

  // Completion of this command does not resume the target.
  return false;
}

folly::dynamic ScopesCommand::getScopeDescription(
  DebuggerSession* session,
  const FrameObject* frame,
  const char* displayName,
  ScopeType type,
  bool expensive
) {
  assert (frame != nullptr);

  request_id_t req = frame->m_requestId;
  assertx(req == m_debugger->getCurrentThreadId());
  int depth = frame->m_frameDepth;

  folly::dynamic scope = folly::dynamic::object;
  unsigned int scopeId = session->generateScopeId(req, depth, type);

  scope["name"] = displayName;
  scope["variablesReference"] = scopeId;
  scope["expensive"] = expensive;

  const ScopeObject* scopeObj = session->getScopeObject(scopeId);
  scope["namedVariables"] =
    VariablesCommand::countScopeVariables(session, m_debugger, scopeObj, req);

  return scope;
}

}
}
