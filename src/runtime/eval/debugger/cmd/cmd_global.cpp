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

#include <runtime/eval/debugger/cmd/cmd_global.h>
#include <runtime/eval/debugger/cmd/cmd_variable.h>
#include <runtime/eval/runtime/eval_frame_injection.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdGlobal::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_globals);
}

void CmdGlobal::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_globals);
}

bool CmdGlobal::help(DebuggerClient *client) {
  client->helpTitle("Global Command");
  client->helpCmds(
    "[g]lobal",           "lists all global variables",
    "[g]lobal {text}",    "full-text search global variables",
    NULL
  );
  client->helpBody(
    "This will print names and values of all global variables, if {text} is "
    "not speified. Otherwise, it will print global variables that contain the "
    "text in their names or values. The search is case-insensitive and "
    "string-based."
  );
  return true;
}

bool CmdGlobal::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  String text;
  if (client->argCount() == 1) {
    text = client->argValue(1);
  } else if (client->argCount() != 0) {
    return help(client);
  }

  CmdGlobalPtr cmd = client->xend<CmdGlobal>(this);
  if (cmd->m_globals.empty()) {
    client->info("(no global variable was found)");
  } else {
    CmdVariable::PrintVariables(client, cmd->m_globals, true, text);
  }

  return true;
}

bool CmdGlobal::onServer(DebuggerProxy *proxy) {
  m_globals = CmdVariable::GetGlobalVariables();
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
