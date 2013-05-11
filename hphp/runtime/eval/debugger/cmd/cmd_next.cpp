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

#include <runtime/eval/debugger/cmd/cmd_next.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

bool CmdNext::help(DebuggerClient *client) {
  client->helpTitle("Next Command");
  client->helpCmds(
    "[n]ext {count=1}", "steps over lines of code",
    nullptr
  );
  client->helpBody(
    "Use this command at break to step over lines of code. Specify a "
    "count to step over more than one line of code."
  );
  return true;
}

void CmdNext::onSetup(DebuggerProxy *proxy, CmdInterrupt &interrupt) {
  TRACE(2, "CmdNext::onSetup\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.
  CmdFlowControl::onSetup(proxy, interrupt);
  m_stackDepth = proxy->getStackDepth();
  m_vmDepth = g_vmContext->m_nesting;
  m_loc = interrupt.getFileLine();

  // Start by single-stepping the current line.
  installLocationFilterForLine(interrupt.getSite());
  m_needsVMInterrupt = true;
}

void CmdNext::onBeginInterrupt(DebuggerProxy *proxy, CmdInterrupt &interrupt) {
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

  if ((currentVMDepth < m_vmDepth) ||
      ((currentVMDepth == m_vmDepth) && (currentStackDepth <= m_stackDepth))) {
    // We're at the same depth as when we started, or perhaps even shallower, so
    // there's no need for any step out breakpoint anymore.
    cleanupStepOut();

    if (m_loc != interrupt.getFileLine()) {
      TRACE(2, "CmdNext: same depth, off original line.\n");
      m_complete = (decCount() == 0);
    }
    if (!m_complete) {
      TRACE(2, "CmdNext: not complete, filter new line and keep stepping.\n");
      m_loc = interrupt.getFileLine();
      installLocationFilterForLine(interrupt.getSite());
      m_needsVMInterrupt = true;
    }
  } else {
    // Deeper, so let's setup a step out operation and turn interrupts off.
    if (!m_stepOutUnit) {
      // We can nuke the entire location filter here since we'll re-install it
      // when we get back to the old level. Keeping it installed may be more
      // efficient if we were on a large line, but there is a penalty for every
      // opcode executed while it's installed and that's bad if there's a lot of
      // code called from that line.
      removeLocationFilter();
      setupStepOut();
    }
    m_needsVMInterrupt = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
