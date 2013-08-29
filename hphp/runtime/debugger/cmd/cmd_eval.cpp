/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/cmd/cmd_eval.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/base/array-init.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdEval::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_output);
  thrift.write(m_frame);
  thrift.write(m_bypassAccessCheck);
}

void CmdEval::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_output);
  thrift.read(m_frame);
  thrift.read(m_bypassAccessCheck);
}

void CmdEval::onClient(DebuggerClient &client) {
  m_body = client.getCode();
  m_frame = client.getFrame();
  m_bypassAccessCheck = client.getDebuggerBypassCheck();
  DebuggerCommandPtr res =
    client.xendWithNestedExecution<DebuggerCommand>(this);
  res->handleReply(client);
}

void CmdEval::handleReply(DebuggerClient &client) {
  if (!m_output.empty()) client.print(m_output);
}

// NB: unlike most other commands, the client expects that more interrupts
// can occur while we're doing the server-side work for an eval.
bool CmdEval::onServer(DebuggerProxy &proxy) {
  PCFilter* locSave = g_vmContext->m_lastLocFilter;
  g_vmContext->m_lastLocFilter = new PCFilter();
  g_vmContext->setDebuggerBypassCheck(m_bypassAccessCheck);
  proxy.ExecutePHP(m_body, m_output, m_frame,
                   DebuggerProxy::ExecutePHPFlagsAtInterrupt |
                   (!proxy.isLocal() ? DebuggerProxy::ExecutePHPFlagsLog :
                    DebuggerProxy::ExecutePHPFlagsNone));
  g_vmContext->setDebuggerBypassCheck(false);
  delete g_vmContext->m_lastLocFilter;
  g_vmContext->m_lastLocFilter = locSave;
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
