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

#include "hphp/runtime/debugger/cmd/cmd_run.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdRun::sendImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "CmdRun::sendImpl\n");
  DebuggerCommand::sendImpl(thrift);
  thrift.write(*m_args);
}

void CmdRun::recvImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "CmdRun::recvImpl\n");
  DebuggerCommand::recvImpl(thrift);
  m_args = StringVecPtr(new StringVec());
  thrift.read(*m_args);
}

void CmdRun::list(DebuggerClient &client) {
  TRACE(2, "CmdRun::list\n");
  client.addCompletion(DebuggerClient::AutoCompleteFileNames);
}

void CmdRun::help(DebuggerClient &client) {
  TRACE(2, "CmdRun::help\n");
  client.helpTitle("Run Command");
  client.helpCmds(
    "[r]un",                             "restarts program",
    "[r]un {file} {arg1} {arg2} ...",    "starts a new program",
    nullptr
  );
  client.helpBody(
    "Aborts current execution and restarts program with specified arguments. "
    "If no arguments are specified, it will reuse the PHP file and old "
    "arguments. If arguments are to be changed, please include file name, "
    "even if it is the same, as the first one.\n"
    "\n"
    "In server mode, this command will simply abort current page handling "
    "without restarting anything."
  );
}

void CmdRun::onClient(DebuggerClient &client) {
  TRACE(2, "CmdRun::onClient\n");
  if (DebuggerCommand::displayedHelp(client)) return;

  m_args = StringVecPtr(client.args(), null_deleter());
  client.sendToServer(this);
  client.clearCachedLocal();
  client.setFrame(0);
  throw DebuggerConsoleExitException();
}

bool CmdRun::onServer(DebuggerProxy &proxy) {
  throw DebuggerRestartException(m_args);
}

///////////////////////////////////////////////////////////////////////////////
}}
