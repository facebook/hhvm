/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/debugger/cmd/cmd_auth.h"

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/util/configs/debugger.h"
#include "hphp/util/process-exec.h"
#include <string>

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdAuth::sendImpl(DebuggerThriftBuffer& thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_token);
  thrift.write(m_session);
  thrift.write(m_sandboxPath);
}

void CmdAuth::recvImpl(DebuggerThriftBuffer& thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_token);
  thrift.read(m_session);
  thrift.read(m_sandboxPath);
}

void CmdAuth::onClient(DebuggerClient& client) {
  auto const token_path = Cfg::Debugger::AuthTokenScriptBin;
  auto const session_path = Cfg::Debugger::AuthSessionAuthScriptBin;
  const char *argv[] = { nullptr };
  if (token_path.empty() ||
      !proc::exec(token_path.data(), argv, nullptr, m_token, nullptr)) {
    m_token.clear();
  }
  if (session_path.empty() ||
      !proc::exec(session_path.data(), argv, nullptr, m_session, nullptr)) {
    m_session.clear();
  }

  client.sendToServer(this);
}

bool CmdAuth::onServer(DebuggerProxy& proxy) {
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}
