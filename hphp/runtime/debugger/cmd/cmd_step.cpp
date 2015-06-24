/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/cmd/cmd_step.h"

#include "hphp/runtime/debugger/debugger_client.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

void CmdStep::help(DebuggerClient &client) {
  client.helpTitle("Step Command");
  client.helpCmds(
    "[s]tep {count=1}", "steps into lines of code",
    nullptr
  );
  client.helpBody(
    "Use this command at break to step into lines of code. Specify a "
    "count to step more than once."
  );
}

void CmdStep::onSetup(DebuggerProxy &proxy, CmdInterrupt &interrupt) {
  assert(!m_complete); // Complete cmds should not be asked to do work.
  installLocationFilterForLine(interrupt.getSite());
  m_needsVMInterrupt = true;
}

void CmdStep::onBeginInterrupt(DebuggerProxy &proxy, CmdInterrupt &interrupt) {
  // Step doesn't care about this interrupt... we just stay the course and
  // keep stepping.
  if (interrupt.getInterruptType() == ExceptionHandler) return;
  // Don't step into generated or builtin functions, keep looking.
  if (interrupt.getSite()->getLine0() == 0) return;
  if (interrupt.getSite()->isBuiltin()) return;
  m_complete = (decCount() == 0);
  if (!m_complete) {
    installLocationFilterForLine(interrupt.getSite());
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
