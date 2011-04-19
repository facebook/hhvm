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
}

void CmdEval::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_output);
  thrift.read(m_frame);
}

bool CmdEval::onClient(DebuggerClient *client) {
  m_body = client->getCode();
  m_frame = client->getFrame();
  CmdEvalPtr res = client->xend<CmdEval>(this);
  client->print(res->m_output);
  return true;
}

bool CmdEval::onServer(DebuggerProxy *proxy) {
  DebuggerProxy::ExecutePHP(m_body, m_output, !proxy->isLocal(), m_frame);
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
