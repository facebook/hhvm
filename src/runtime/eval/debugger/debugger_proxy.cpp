/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/eval/debugger/cmd/cmd_flow_control.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>

using namespace std;
using namespace boost;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket, bool local)
    : m_local(local) {
  m_thrift.create(socket);
}

void DebuggerProxy::startDummySandbox() {
  m_dummySandbox = DummySandboxPtr
    (new DummySandbox(m_sandbox, RuntimeOption::DebuggerStartupDocument));
  m_dummySandbox->start();
}

void DebuggerProxy::switchSandbox(const std::string &id) {
  SandboxInfo sandbox(id);
  Debugger::SwitchSandbox(shared_from_this(), sandbox);

  // This has to be done after Debugger::SwitchSandbox() who still needs the
  // old m_sandbox value.
  m_sandbox = sandbox;
}

void DebuggerProxy::interrupt(CmdInterrupt &cmd) {
  // Note that even if fcShouldBreak, we still need to test bpShouldBreak
  // so to correctly report breakpoint reached + evaluating watchpoints;
  // vice versa, so to decCount() on m_burner.
  bool fcShouldBreak = false; // should I break according to flow control?
  bool bpShouldBreak = false; // should I break according to breakpoints?

  if (cmd.getInterruptType() == BreakPointReached && m_burner) {
    fcShouldBreak = breakByFlowControl(cmd);
  }
  bpShouldBreak = cmd.shouldBreak(m_breakpoints);
  if (!fcShouldBreak && !bpShouldBreak) {
    return; // this is done before KindOfContinue testing
  }
  if (cmd.getFrame()) {
    cmd.getFrame()->setBreakPointHit();
  }

  if (cmd.getInterruptType() == BreakPointReached && m_burner) {
    if (m_burner->is(DebuggerCommand::KindOfContinue)) {
      if (!m_burner->decCount()) m_burner.reset();
      return;
    }
  }

  if (!cmd.onServer(this)) {
    Debugger::RemoveProxy(shared_from_this()); // on socket error
    return;
  }

  while (true) {
    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::onCommand()")) {
      // we will wait forever until DebuggerClient sends us something
    }
    if (res) {
      m_burner = dynamic_pointer_cast<CmdFlowControl>(res);
      if (m_burner) {
        processFlowControl(cmd);
        return;
      }
    }
    if (!res || res->is(DebuggerCommand::KindOfQuit) || !res->onServer(this)) {
      Debugger::RemoveProxy(shared_from_this());
      if (res && res->is(DebuggerCommand::KindOfQuit)) {
        throw DebuggerClientExitException();
      }
      return;
    }
  }
}

void DebuggerProxy::processFlowControl(CmdInterrupt &cmd) {
  switch (m_burner->getType()) {
    case DebuggerCommand::KindOfContinue:
      if (!m_burner->decCount()) m_burner.reset();
      break;
    case DebuggerCommand::KindOfStep:
      break;
    case DebuggerCommand::KindOfNext:
      if (cmd.getFrame()) {
        m_burner->setFrame(cmd.getFrame());
        m_burner->setFileLine(cmd.getFileLine());
      }
      break;
    case DebuggerCommand::KindOfOut: {
      FrameInjection *frame = cmd.getFrame();
      if (frame) {
        m_burner->setFrame(frame->getPrev());
      }
      break;
    }
    default:
      ASSERT(false);
      break;
  }
}

bool DebuggerProxy::breakByFlowControl(CmdInterrupt &cmd) {
  switch (m_burner->getType()) {
    case DebuggerCommand::KindOfStep:
      if (!m_burner->decCount()) m_burner.reset();
      return true;

    case DebuggerCommand::KindOfNext: {
      FrameInjection *last = m_burner->getFrame();
      FrameInjection *frame = cmd.getFrame();
      bool over = true;
      if (last == frame) {
        over = (m_burner->getFileLine() != cmd.getFileLine());
      } else if (last == m_burner->getNegativeFrame()) {
        over = false;
      } else {
        FrameInjection *prev;
        while (prev = frame->getPrev()) {
          if (last == prev) {
            over = false;
            m_burner->setNegativeFrame(frame); // to avoid re-calculation
            break;
          }
        }
      }
      if (over) {
        if (!m_burner->decCount()) {
          m_burner.reset();
        } else {
          if (last != frame) {
            m_burner->setFrame(frame);
            m_burner->setNegativeFrame(NULL);
          }
          m_burner->setFileLine(cmd.getFileLine());
        }
        return true;
      }
      break;
    }
    case DebuggerCommand::KindOfOut: {
      FrameInjection *frame = cmd.getFrame();
      if (m_burner->getFrame() == frame) {
        if (!m_burner->decCount()) {
          m_burner.reset();
        } else {
          if (frame) {
            m_burner->setFrame(frame->getPrev());
          } else {
            m_burner.reset();
          }
        }
        return true;
      }
      break;
    }
    default:
      break;
  }

  return false;
}

bool DebuggerProxy::send(DebuggerCommand *cmd) {
  return cmd->send(m_thrift);
}

///////////////////////////////////////////////////////////////////////////////
}}
