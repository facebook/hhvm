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

#include <runtime/eval/debugger/cmd/cmd_flow_control.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdFlowControl::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_count);
}

void CmdFlowControl::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_count);
}

bool CmdFlowControl::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() > 1) {
    return help(client);
  }

  if (client->argCount() == 1) {
    string snum = client->argValue(1);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client->error("Count needs to be a number.");
      return true;
    }

    m_count = atoi(snum.c_str());
    if (m_count < 1) {
      client->error("Count needs to be a positive number.");
      return true;
    }
  }

  client->send(this);
  throw DebuggerConsoleExitException();
}

bool CmdFlowControl::onServer(DebuggerProxy *proxy) {
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
