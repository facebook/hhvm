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

#include <runtime/eval/debugger/cmd/cmd_example.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdExample::sendImpl(DebuggerThriftBuffer &thrift) {
  CmdExtended::sendImpl(thrift);
  thrift.write(m_input);
  thrift.write(m_output);
}

void CmdExample::recvImpl(DebuggerThriftBuffer &thrift) {
  CmdExtended::recvImpl(thrift);
  thrift.read(m_input);
  thrift.read(m_output);
}

void CmdExample::list(DebuggerClient *client) {
  client->addCompletion("tic-tac-toe");
  client->addCompletion("hip-hop-roll");
}

bool CmdExample::help(DebuggerClient *client) {
  client->helpTitle("Example Command");
  client->helpCmds(
    "xample {string}",      "it will tell you how long it is!",
    "x ample {string}",     "it will tell you how long it is!",
    NULL
  );
  client->helpBody(
    "This is just an example of extending debugger commands with C++. "
    "To add a new command, simply run \"php new_cmd.php {name}\" under "
    "runtime/eval/debugger/cmd, and it will generate two files to start with. "
    "Modify command registration code at bottom of runtime/eval/debugger/cmd/"
    "cmd_extended.cpp and modify your new command by following this example."
  );
  return true;
}

bool CmdExample::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 1) {
    return help(client);
  }

  m_input = client->argRest(2);
  CmdExamplePtr res = client->xend<CmdExample>(this);
  client->output("%d", res->m_output);
  return true;
}

bool CmdExample::onServer(DebuggerProxy *proxy) {
  m_output = m_input.size();
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
