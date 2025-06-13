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

#include "hphp/runtime/debugger/cmd/cmd_eval_stream.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger)

void CmdEvalStream::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_output);
  thrift.write(m_frame);
  thrift.write(m_bypassAccessCheck);
  thrift.write(static_cast<int8_t>(m_stream_status));
  if (this->m_version == 2) thrift.write(m_failed);
}

void CmdEvalStream::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_output);
  thrift.read(m_frame);
  thrift.read(m_bypassAccessCheck);
  int8_t streamStatusValue;
  thrift.read(streamStatusValue);
  m_stream_status = static_cast<StreamStatus>(streamStatusValue);
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

StreamStatus CmdEvalStream::getStreamStatus() {
  return m_stream_status;
}

void CmdEvalStream::onClient(DebuggerClient &client) {
  m_body = client.getCode();
  m_frame = client.getFrame();
  m_bypassAccessCheck = client.getDebuggerClientBypassCheck();
  auto res = client.xendWithNestedExecution<CmdEvalStream>(this);
  assertx(res->is(DebuggerCommand::KindOfEvalStream));
  auto eval = std::static_pointer_cast<CmdEvalStream>(res);
  m_failed = eval->m_failed;
}

void CmdEvalStream::handleReply(DebuggerClient &client) {
  if (failed() && client.unknownCmdReceived()) {
    client.help(
    "Notice: Attempted to interpret unknown debugger command as PHP!\n");
  }

  if (!m_output.empty()) client.output(m_output);
}

// NB: unlike most other commands, the client expects that more interrupts
// can occur while we're doing the server-side work for an eval.
bool CmdEvalStream::onServer(DebuggerProxy &proxy) {
  PCFilter locSave;
  RequestInjectionData &rid = RequestInfo::s_requestInfo->m_reqInjectionData;
  locSave.swap(rid.m_flowFilter);
  g_context->debuggerSettings.bypassCheck = m_bypassAccessCheck;
  auto const result = proxy.ExecutePHPWithStreaming(
    m_body, m_output, *this, m_frame, m_stream_status,
    DebuggerProxy::ExecutePHPFlagsAtInterrupt |
      (!proxy.isLocal() ? DebuggerProxy::ExecutePHPFlagsLog :
       DebuggerProxy::ExecutePHPFlagsNone)
  );
  m_failed = result.first;
  g_context->debuggerSettings.bypassCheck = false;
  locSave.swap(rid.m_flowFilter);
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}
