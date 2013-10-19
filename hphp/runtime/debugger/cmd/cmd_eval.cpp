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
  if (this->m_version == 2) thrift.write(m_failed);
}

void CmdEval::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_output);
  thrift.read(m_frame);
  thrift.read(m_bypassAccessCheck);
  if (this->m_version == 2) thrift.read(m_failed);
  // Old senders will set version to 0. A new sender sets it to 1
  // and then expects an answer using version 2.
  // Note that version 1 is the same format as version 0, so old
  // receivers will not break when receiving a version 1 message.
  // This code ensures that version 2 messages are received only
  // by receivers that previously sent a version 1 message (thus
  // indicating their ability to deal with version 2 messages).
  if (this->m_version == 1) this->m_version = 2;
}

void CmdEval::onClient(DebuggerClient &client) {
  m_body = client.getCode();
  m_frame = client.getFrame();
  m_bypassAccessCheck = client.getDebuggerClientBypassCheck();
  CmdEvalPtr res =
     client.xendWithNestedExecution<CmdEval>(this);
  res->handleReply(client);
  m_failed = res->m_failed;
}

void CmdEval::handleReply(DebuggerClient &client) {
  if (this->failed() && client.unknownCmdReceived()) {
    client.help(
    "Notice: Attempted to interpret unknown debugger command as PHP!\n");
  }

  if (!m_output.empty()) client.print(m_output);
}

// NB: unlike most other commands, the client expects that more interrupts
// can occur while we're doing the server-side work for an eval.
bool CmdEval::onServer(DebuggerProxy &proxy) {
  PCFilter* locSave = g_vmContext->m_lastLocFilter;
  g_vmContext->m_lastLocFilter = new PCFilter();
  g_vmContext->setDebuggerBypassCheck(m_bypassAccessCheck);
  proxy.ExecutePHP(m_body, m_output, m_frame, m_failed,
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
