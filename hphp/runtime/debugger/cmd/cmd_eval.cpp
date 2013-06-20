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
#include "hphp/runtime/vm/debugger_hook.h"

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

void CmdEval::onClientImpl(DebuggerClient &client) {
  m_body = client.getCode();
  m_frame = client.getFrame();
  m_bypassAccessCheck = client.getDebuggerBypassCheck();
  DebuggerCommandPtr res =
    client.xendWithNestedExecution<DebuggerCommand>(this);
  if (!res->is(m_type)) {
    assert(client.isApiMode());
    m_incomplete = true;
    res->setClientOutput(client);
  } else {
    res->handleReply(client);
  }
}

void CmdEval::handleReply(DebuggerClient &client) {
  client.print(m_output);
}

const StaticString
  s_body("body"),
  s_value("value");

void CmdEval::setClientOutput(DebuggerClient &client) {
  client.setOutputType(DebuggerClient::OTValues);
  ArrayInit values(2);
  values.set(s_body, m_body);
  values.set(s_value, m_output);
  client.setOTValues(values.create());
}

bool CmdEval::onServer(DebuggerProxy &proxy) {
  PCFilter* locSave = g_vmContext->m_lastLocFilter;
  g_vmContext->m_lastLocFilter = new PCFilter();
  g_vmContext->setDebuggerBypassCheck(m_bypassAccessCheck);
  DebuggerProxy::ExecutePHP(m_body, m_output, !proxy.isLocal(), m_frame);
  g_vmContext->setDebuggerBypassCheck(false);
  delete g_vmContext->m_lastLocFilter;
  g_vmContext->m_lastLocFilter = locSave;
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
