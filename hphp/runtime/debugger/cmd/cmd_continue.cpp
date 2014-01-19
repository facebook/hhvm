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

#include "hphp/runtime/debugger/cmd/cmd_continue.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdContinue::help(DebuggerClient &client) {
  client.helpTitle("Continue Command");
  client.helpCmds(
    "[c]ontinue {count=1}", "continues program execution",
    nullptr
  );
  client.helpBody(
    "Use this command at break to resume program execution. Specify a "
    "count to repeat the same command many times."
  );
}

void CmdContinue::onSetup(DebuggerProxy &proxy, CmdInterrupt &interrupt) {
  assert(!m_complete); // Complete cmds should not be asked to do work.
  // If there's a remaining count on this cmd then we want it left installed
  // in the proxy.
  m_complete = (decCount() == 0);
}

void CmdContinue::onBeginInterrupt(DebuggerProxy &proxy,
                                   CmdInterrupt &interrupt) {
  assert(!m_complete); // Complete cmds should not be asked to do work.
  m_complete = (decCount() == 0);
}

///////////////////////////////////////////////////////////////////////////////
}}
