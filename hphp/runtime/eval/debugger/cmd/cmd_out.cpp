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

#include <runtime/eval/debugger/cmd/cmd_out.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

bool CmdOut::help(DebuggerClient *client) {
  client->helpTitle("Out Command");
  client->helpCmds(
    "[o]ut {count=1}", "steps out function calls",
    nullptr
  );
  client->helpBody(
    "Use this command at break to step out function calls. Specify a "
    "count to step out more than one level of function calls."
  );
  return true;
}

void CmdOut::onSetup(DebuggerProxy *proxy, CmdInterrupt &interrupt) {
  TRACE(2, "CmdOut::onSetup\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.
  CmdFlowControl::onSetup(proxy, interrupt);
  m_stackDepth = proxy->getStackDepth();
  m_vmDepth = g_vmContext->m_nesting;

  // Simply setup a "step out breakpoint" and let the program run.
  setupStepOut();
}

void CmdOut::onBeginInterrupt(DebuggerProxy *proxy, CmdInterrupt &interrupt) {
  TRACE(2, "CmdNext::onBeginInterrupt\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.
  if (interrupt.getInterruptType() == ExceptionThrown) {
    // If an exception is thrown we turn interrupts on to ensure we stop when
    // control reaches the first catch clause.
    removeLocationFilter();
    m_needsVMInterrupt = true;
    return;
  }

  int currentVMDepth = g_vmContext->m_nesting;
  int currentStackDepth = proxy->getStackDepth();
  if (currentVMDepth < m_vmDepth) {
    // Cut corner here, just break when cross VM boundary no matter how
    // many levels we want to go out of
    TRACE(2, "CmdOut: shallower VM depth, done.\n");
    cleanupStepOut();
    m_complete = true;
  } else if ((currentVMDepth == m_vmDepth) &&
             (currentStackDepth < m_stackDepth)) {
    TRACE(2, "CmdOut: same VM depth, shallower stack depth, done.\n");
    cleanupStepOut();
    m_complete = (decCount() == 0);
    if (!m_complete) {
      TRACE(2, "CmdOut: not complete, step out again.\n");
      setupStepOut();
    }
  }
  m_needsVMInterrupt = false;
}

///////////////////////////////////////////////////////////////////////////////
}}
