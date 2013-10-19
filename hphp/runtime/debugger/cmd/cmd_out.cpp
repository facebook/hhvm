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

#include "hphp/runtime/debugger/cmd/cmd_out.h"
#include "hphp/runtime/vm/hhbc.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

void CmdOut::help(DebuggerClient &client) {
  client.helpTitle("Out Command");
  client.helpCmds(
    "[o]ut {count=1}", "steps out function calls",
    nullptr
  );
  client.helpBody(
    "Use this command at break to step out function calls. Specify a "
    "count to step out more than one level of function calls."
  );
}

void CmdOut::onSetup(DebuggerProxy &proxy, CmdInterrupt &interrupt) {
  TRACE(2, "CmdOut::onSetup\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.
  m_stackDepth = proxy.getStackDepth();
  m_vmDepth = g_vmContext->m_nesting;

  // Simply setup a "step out breakpoint" and let the program run.
  setupStepOuts();
}

void CmdOut::onBeginInterrupt(DebuggerProxy &proxy, CmdInterrupt &interrupt) {
  TRACE(2, "CmdOut::onBeginInterrupt\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.

  m_needsVMInterrupt = false;

  if (m_skippingOverPopR) {
    m_complete = true;
    return;
  }

  int currentVMDepth = g_vmContext->m_nesting;
  int currentStackDepth = proxy.getStackDepth();

  // Deeper or same depth? Keep running.
  if ((currentVMDepth > m_vmDepth) ||
      ((currentVMDepth == m_vmDepth) && (currentStackDepth >= m_stackDepth))) {
    TRACE(2, "CmdOut: deeper, keep running...\n");
    return;
  }

  if (interrupt.getInterruptType() == ExceptionHandler) {
    // If we're about to enter an exception handler we turn interrupts on to
    // ensure we stop when control reaches the handler. The normal logic below
    // will decide if we're done at that point or not.
    TRACE(2, "CmdOut: exception thrown\n");
    removeLocationFilter();
    m_needsVMInterrupt = true;
    return;
  }

  TRACE(2, "CmdOut: shallower stack depth, done.\n");
  cleanupStepOuts();
  int depth = decCount();
  if (depth == 0) {
    PC pc = g_vmContext->getPC();
    // Step over PopR following a call
    if (toOp(*pc) == OpPopR) {
      m_skippingOverPopR = true;
      m_needsVMInterrupt = true;
    } else {
      m_complete = true;
    }
    return;
  } else {
    TRACE(2, "CmdOut: not complete, step out again.\n");
    onSetup(proxy, interrupt);
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
