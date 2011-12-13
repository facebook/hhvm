/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/debugger/cmd/cmd_eval.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

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

bool CmdEval::onClient(DebuggerClient *client) {
  m_body = client->getCode();
  m_frame = client->getFrame();
  m_bypassAccessCheck = client->getDebuggerBypassCheck();
  CmdEvalPtr res = client->xend<CmdEval>(this);
  m_output = res->m_output;
  client->print(res->m_output);
  return true;
}

void CmdEval::setClientOutput(DebuggerClient *client) {
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  values.set("body", m_body);
  values.set("value", m_output);
  client->setOTValues(values);
}

bool CmdEval::onServer(DebuggerProxy *proxy) {
  g_context->setDebuggerBypassCheck(m_bypassAccessCheck);
  DebuggerProxy::ExecutePHP(m_body, m_output, !proxy->isLocal(), m_frame);
  g_context->setDebuggerBypassCheck(false);
  return proxy->send(this);
}

bool CmdEval::onServerVM(DebuggerProxy *proxy) {
  const HPHP::VM::SourceLoc *saveLoc = g_context->m_debuggerLastBreakLoc;
  g_context->setDebuggerBypassCheck(m_bypassAccessCheck);
  DebuggerProxyVM::ExecutePHP(m_body, m_output, !proxy->isLocal(), m_frame);
  g_context->setDebuggerBypassCheck(false);
  g_context->m_debuggerLastBreakLoc = saveLoc;
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
