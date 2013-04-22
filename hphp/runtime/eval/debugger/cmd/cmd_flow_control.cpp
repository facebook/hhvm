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
#include <runtime/vm/debugger_hook.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

CmdFlowControl::~CmdFlowControl() {
  // Remove any location filter that may have been setup by this cmd.
  removeLocationFilter();
}

void CmdFlowControl::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_count);
  thrift.write(m_smallStep);
}

void CmdFlowControl::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_count);
  thrift.read(m_smallStep);
}

bool CmdFlowControl::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  client->setFrame(0);

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
  m_smallStep = client->getDebuggerSmallStep();
  client->sendToServer(this);
  throw DebuggerConsoleExitException();
}

bool CmdFlowControl::onServer(DebuggerProxy *proxy) {
  // Flow control cmds do their work in onSetup() and onBeginInterrupt(), so
  // there is no real work to do in here.
  return true;
}

void CmdFlowControl::onSetup(DebuggerProxy *proxy, CmdInterrupt &interrupt) {
  // Should only do setting and nothing else
  g_context->setDebuggerSmallStep(m_smallStep);
}

// Setup the last location filter on the VM context for all offsets covered by
// the current source line. This will short-circuit the work done in
// phpDebuggerOpcodeHook() and ensure we don't interrupt on this source line.
void CmdFlowControl::installLocationFilterForLine(InterruptSite *site) {
  if (!site) return; // We may be stopped at a place with not source info.
  if (g_vmContext->m_lastLocFilter) {
    g_vmContext->m_lastLocFilter->clear();
  } else {
    g_vmContext->m_lastLocFilter = new VM::PCFilter();
  }
  auto offsets = site->getCurOffsetRange();
  if (Trace::moduleEnabled(Trace::debugger, 5)) {
    Trace::trace("prepare source loc filter\n");
    for (auto it = offsets.begin();
         it != offsets.end(); ++it) {
      Trace::trace("block source loc in %s:%d: unit %p offset [%d, %d)\n",
                   site->getFile(), site->getLine0(),
                   site->getUnit(), it->m_base, it->m_past);
    }
  }
  g_vmContext->m_lastLocFilter->addRanges(site->getUnit(), offsets);
}

void CmdFlowControl::removeLocationFilter() {
  if (g_vmContext->m_lastLocFilter) {
    delete g_vmContext->m_lastLocFilter;
    g_vmContext->m_lastLocFilter = nullptr;
  }
}

void CmdFlowControl::setupStepOut() {
  ActRec *fp = g_vmContext->getFP();
  assert(fp);
  ActRec *fpPrev = g_vmContext->getPrevVMState(fp, &m_stepOutOffset);
  assert(fpPrev);
  TRACE(2, "CmdFlowControl: step out to offset %d\n", m_stepOutOffset);
  m_stepOutUnit = fpPrev->m_func->unit();
  phpAddBreakPoint(m_stepOutUnit, m_stepOutOffset);
}

void CmdFlowControl::cleanupStepOut() {
  if (m_stepOutUnit) {
    phpRemoveBreakPoint(m_stepOutUnit, m_stepOutOffset);
    m_stepOutUnit = nullptr;
    m_stepOutOffset = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
