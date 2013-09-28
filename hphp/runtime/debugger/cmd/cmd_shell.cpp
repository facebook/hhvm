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

#include "hphp/runtime/debugger/cmd/cmd_shell.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdShell::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_args);
  thrift.write(m_out);
}

void CmdShell::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_args);
  thrift.read(m_out);
}

void CmdShell::list(DebuggerClient &client) {
  client.addCompletion(DebuggerClient::AutoCompleteFileNames);
}

void CmdShell::help(DebuggerClient &client) {
  client.helpTitle("Shell Command");
  client.help("! {cmd} {arg1} {arg2} ...    remotely executes shell command");
  client.helpBody(
    "Executes the shell command on connected machine.\n"
    "\n"
    "The space between ! and command is not needed. '!ls' works as well."
  );
}

void CmdShell::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() == 0) {
    help(client);
    return;
  }
  m_args = *client.args();
  CmdShellPtr cmd = client.xend<CmdShell>(this);
  client.print(cmd->m_out);
}

bool CmdShell::onServer(DebuggerProxy &proxy) {
  const char **argv =
    (const char **)malloc((m_args.size() + 1) * sizeof(char*));
  for (unsigned int i = 0; i < m_args.size(); i++) {
    argv[i] = (char*)m_args[i].c_str();
  }
  argv[m_args.size()] = nullptr;
  Process::Exec(argv[0], argv, nullptr, m_out, &m_out, true);
  free(argv);
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
