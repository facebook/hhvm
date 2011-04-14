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
#include <runtime/eval/debugger/cmd/cmd_jump.h>
#include <runtime/eval/debugger/cmd/cmd_signal.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>
#include <runtime/eval/eval.h>
#include <util/process.h>
#include <util/logger.h>

using namespace std;
using namespace boost;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket, bool local)
    : m_stopped(false), m_local(local), m_hasBreakPoints(false),
      m_threadMode(Normal), m_thread(0),
      m_signalThread(this, &DebuggerProxy::pollSignal),
      m_signum(CmdSignal::SignalNone) {
  m_thrift.create(socket);
}

DebuggerProxy::~DebuggerProxy() {
  m_stopped = true;
  m_signalThread.waitForEnd();

  if (m_dummySandbox) {
    m_dummySandbox->stop();
  }
}

void DebuggerProxy::startDummySandbox() {
  m_dummySandbox = DummySandboxPtr
    (new DummySandbox(this, RuntimeOption::DebuggerDefaultSandboxPath,
                      RuntimeOption::DebuggerStartupDocument));
  m_dummySandbox->start();
}

void DebuggerProxy::startSignalThread() {
  m_signalThread.start();
}

DSandboxInfo DebuggerProxy::getSandbox() const {
  Lock lock(m_mutex);
  return m_sandbox;
}

std::string DebuggerProxy::getSandboxId() const {
  Lock lock(m_mutex);
  return m_sandbox.id();
}

void DebuggerProxy::switchSandbox(const std::string &newId) {
  Debugger::SwitchSandbox(shared_from_this(), newId);
}

void DebuggerProxy::updateSandbox(DSandboxInfoPtr sandbox) {
  Lock lock(m_mutex);
  if (sandbox) {
    if (m_sandbox.id() != sandbox->id()) {
      m_sandbox = *sandbox;
    } else {
      m_sandbox.update(*sandbox);
    }
  }
}

void DebuggerProxy::setBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  Lock lock(m_mutex);
  m_breakpoints = breakpoints;
  m_hasBreakPoints = !m_breakpoints.empty();
}

bool DebuggerProxy::checkBreakPoints(CmdInterrupt &cmd) {
  Lock lock(m_mutex);
  return cmd.shouldBreak(m_breakpoints);
}

bool DebuggerProxy::switchThread(DThreadInfoPtr thread) {
  Lock lock(this);
  if (m_threads.find(thread->m_id) != m_threads.end()) {
    m_newThread = thread;
    return true;
  }
  return false;
}

void DebuggerProxy::switchThreadMode(ThreadMode mode,
                                     int64 threadId /* = 0 */) {
  Lock lock(this);
  m_threadMode = mode;
  if (threadId) {
    m_thread = threadId;
    m_newThread.reset();
    notifyAll(); // since threadId != 0, we're still waking up just one thread
  } else if (mode == Normal) {
    m_thread = 0;
    notify();
  } else {
    m_thread = Process::GetThreadId();
  }
  if (mode == Normal) {
    m_jump.reset();
    m_flow.reset();
  }
}

void DebuggerProxy::getThreads(DThreadInfoPtrVec &threads) {
  Lock lock(this);
  std::stack<void *> &interrupts =
    ThreadInfo::s_threadInfo->m_reqInjectionData.interrupts;
  ASSERT(!interrupts.empty());
  if (!interrupts.empty()) {
    CmdInterrupt *tint = (CmdInterrupt*)interrupts.top();
    ASSERT(tint);
    if (tint) {
      threads.push_back(createThreadInfo(tint->desc()));
    }
  }
  for (std::map<int64, DThreadInfoPtr>::const_iterator iter =
         m_threads.begin(); iter != m_threads.end(); ++iter) {
    DThreadInfoPtr thread(new DThreadInfo());
    *thread = *iter->second;
    threads.push_back(thread);
  }
}

bool DebuggerProxy::blockUntilOwn(CmdInterrupt &cmd, bool check) {
  int64 self = cmd.getThreadId();

  Lock lock(this);
  if (m_thread && m_thread != self) {
    if (check && (m_threadMode == Exclusive || !checkBreakPoints(cmd))) {
      // jumps and flow control commands only belong to sticky thread
      return false;
    }
    m_threads[self] = createThreadInfo(cmd.desc());
    while (!m_stopped && m_thread && m_thread != self) {
      wait(1);

      // if for whatever reason, m_thread isn't debugging anymore (for example,
      // it runs in Sticky mode, but it finishes running), kick it out.
      if (!Debugger::IsThreadDebugging(m_thread)) {
        m_threadMode = Normal;
        m_thread = self;
        m_newThread.reset();
        m_jump.reset();
        m_flow.reset();
      }
    }
    m_threads.erase(self);
    if (m_stopped) return false;
  } else if (check && !checkJumpFlowBreak(cmd)) {
    return false;
  }

  if (m_thread == 0) {
    m_thread = self;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// main functions

bool DebuggerProxy::needInterrupt() {
  return m_hasBreakPoints || m_signum != CmdSignal::SignalNone;
}

void DebuggerProxy::interrupt(CmdInterrupt &cmd) {
  if (!blockUntilOwn(cmd, true)) {
    return;
  }
  if (processJumpFlowBreak(cmd)) {
    while (true) {
      try {
        Lock lock(m_signalMutex);
        m_signum = CmdSignal::SignalNone;
        processInterrupt(cmd);
      } catch (const DebuggerException &e) {
        switchThreadMode(Normal);
        throw;
      } catch (...) {
        ASSERT(false); // no other exceptions should be seen here
        switchThreadMode(Normal);
        throw;
      }
      if (cmd.getInterruptType() == PSPEnded) {
        switchThreadMode(Normal);
        return; // we are done with this thread
      }
      if (!m_newThread) {
        break; // we're not switching threads
      }

      switchThreadMode(Normal, m_newThread->m_id);
      blockUntilOwn(cmd, false);
    }
  }

  if (m_threadMode == Normal) {
    Lock lock(this);
    m_thread = 0;
    notify();
  } else if (cmd.getInterruptType() == PSPEnded) {
    switchThreadMode(Normal); // we are done with this thread
  }
}

void DebuggerProxy::notifyDummySandbox() {
  m_dummySandbox->notifySignal(CmdSignal::SignalBreak);
}

void DebuggerProxy::pollSignal() {
  while (!m_stopped) {
    sleep(1);

    // after 1 second that no active thread picks up the signal, we send it
    // to dummy sandbox
    if (m_signum != CmdSignal::SignalNone && m_dummySandbox) {
      Lock lock(m_signumMutex);
      if (m_signum != CmdSignal::SignalNone) {
        m_dummySandbox->notifySignal(m_signum);
        m_signum = CmdSignal::SignalNone;
      }
    }

    Lock lock(m_signalMutex);

    CmdSignal cmd;
    if (!cmd.onServer(this)) break; // on socket error

    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::pollSignal()")) {
      // we will wait forever until DebuggerClient sends us something
    }
    if (!res) break;

    CmdSignalPtr sig = dynamic_pointer_cast<CmdSignal>(res);
    if (!sig) {
      Logger::Error("bad response from signal polling: %d", res->getType());
      break;
    }

    m_signum = sig->getSignal();
  }
}

///////////////////////////////////////////////////////////////////////////////

bool DebuggerProxy::checkJumpFlowBreak(CmdInterrupt &cmd) {
  if (m_signum == CmdSignal::SignalBreak) {
    Lock lock(m_signumMutex);
    if (m_signum == CmdSignal::SignalBreak) {
      m_signum = CmdSignal::SignalNone;
      m_jump.reset();
      m_flow.reset();
      return true;
    }
  }

  // jump command skips everything until the file:line or label is reached
  bool jumpBreak = false;
  if (m_jump) {
    InterruptSite *site = cmd.getSite();
    if (site) {
      if (!m_jump->match(site)) {
        site->setJumping();
        return false;
      }
      m_jump.reset();
      jumpBreak = true;
    }
  }

  if (!jumpBreak) {
    // Note that even if fcShouldBreak, we still need to test bpShouldBreak
    // so to correctly report breakpoint reached + evaluating watchpoints;
    // vice versa, so to decCount() on m_flow.
    bool fcShouldBreak = false; // should I break according to flow control?
    bool bpShouldBreak = false; // should I break according to breakpoints?

    if ((cmd.getInterruptType() == BreakPointReached ||
         cmd.getInterruptType() == HardBreakPoint) && m_flow) {
      fcShouldBreak = breakByFlowControl(cmd);
    }

    bpShouldBreak = checkBreakPoints(cmd);
    if (!fcShouldBreak && !bpShouldBreak) {
      return false; // this is done before KindOfContinue testing
    }
  }

  return true;
}

bool DebuggerProxy::processJumpFlowBreak(CmdInterrupt &cmd) {
  if (m_jump) {
    switch (cmd.getInterruptType()) {
      case SessionEnded:
      case RequestEnded:
      case PSPEnded:
        cmd.setPendingJump();
        m_jump.reset();
        break;
      default:
        break;
    }
  }
  if (cmd.getFrame()) {
    cmd.getFrame()->setBreakPointHit();
  }
  if ((cmd.getInterruptType() == BreakPointReached ||
       cmd.getInterruptType() == HardBreakPoint) && m_flow) {
    if (m_flow->is(DebuggerCommand::KindOfContinue)) {
      if (!m_flow->decCount()) m_flow.reset();
      return false;
    }
  }
  return true;
}

void DebuggerProxy::processInterrupt(CmdInterrupt &cmd) {
  if (!cmd.onServer(this)) {
    Debugger::RemoveProxy(shared_from_this()); // on socket error
    return;
  }

  while (true) {
    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::processInterrupt()")) {
      // we will wait forever until DebuggerClient sends us something
    }
    if (res) {
      m_flow = dynamic_pointer_cast<CmdFlowControl>(res);
      if (m_flow) {
        processFlowControl(cmd);
        if (m_flow && m_threadMode == Normal) {
          switchThreadMode(Sticky);
        }
        return;
      }
      if (res->is(DebuggerCommand::KindOfJump)) {
        m_jump = static_pointer_cast<CmdJump>(res);
        if (m_threadMode == Normal) {
          switchThreadMode(Sticky);
        }
        InterruptSite *site = cmd.getSite();
        if (site) {
          site->setJumping();
        }
        return;
      }
      if (res->is(DebuggerCommand::KindOfQuit)) {
        Debugger::RemoveProxy(shared_from_this());
        throw DebuggerClientExitException();
      }
    }
    try {
      if (!res || !res->onServer(this)) {
        Debugger::RemoveProxy(shared_from_this());
        return;
      }
    } catch (const DebuggerException &e) {
      throw;
    } catch (...) {
      Logger::Error("onServer() throws non DebuggerException: %d",
                    res->getType());
      Debugger::RemoveProxy(shared_from_this());
      return;
    }
    if (res->shouldExitInterrupt()) {
      return;
    }
  }
}

void DebuggerProxy::processFlowControl(CmdInterrupt &cmd) {
  switch (m_flow->getType()) {
    case DebuggerCommand::KindOfContinue:
      if (!m_flow->decCount()) m_flow.reset();
      break;
    case DebuggerCommand::KindOfStep:
      break;
    case DebuggerCommand::KindOfNext:
      if (cmd.getFrame()) {
        m_flow->setFrame(cmd.getFrame());
        m_flow->setFileLine(cmd.getFileLine());
      }
      break;
    case DebuggerCommand::KindOfOut: {
      FrameInjection *frame = cmd.getFrame();
      if (frame) {
        m_flow->setFrame(frame->getPrev());
      }
      break;
    }
    default:
      ASSERT(false);
      break;
  }
}

bool DebuggerProxy::breakByFlowControl(CmdInterrupt &cmd) {
  switch (m_flow->getType()) {
    case DebuggerCommand::KindOfStep:
      if (!m_flow->decCount()) m_flow.reset();
      return true;

    case DebuggerCommand::KindOfNext: {
      FrameInjection *last = m_flow->getFrame();
      FrameInjection *frame = cmd.getFrame();
      bool over = true;
      if (last == frame) {
        over = (m_flow->getFileLine() != cmd.getFileLine());
      } else if (last == m_flow->getNegativeFrame()) {
        over = false;
      } else {
        FrameInjection *prev;
        while (prev = frame->getPrev()) {
          if (last == prev) {
            over = false;
            m_flow->setNegativeFrame(frame); // to avoid re-calculation
            break;
          }
          frame = prev;
        }
      }
      if (over) {
        if (!m_flow->decCount()) {
          m_flow.reset();
        } else {
          if (last != frame) {
            m_flow->setFrame(frame);
            m_flow->setNegativeFrame(NULL);
          }
          m_flow->setFileLine(cmd.getFileLine());
        }
        return true;
      }
      break;
    }
    case DebuggerCommand::KindOfOut: {
      FrameInjection *frame = cmd.getFrame();
      if (m_flow->getFrame() == frame) {
        if (!m_flow->decCount()) {
          m_flow.reset();
        } else {
          if (frame) {
            m_flow->setFrame(frame->getPrev());
          } else {
            m_flow.reset();
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
// helpers

std::string DebuggerProxy::MakePHP(const std::string &php) {
  return "<?php " + php + ";";
}

std::string DebuggerProxy::MakePHPReturn(const std::string &php) {
  return "<?php return " + php + ";";
}

static void append_stdout(const char *s, int len, void *data) {
  StringBuffer *sb = (StringBuffer*)data;
  if (Util::s_stdout_color) {
    sb->append(Util::s_stdout_color);
  }
  sb->append(s, len);
  if (Util::s_stdout_color) {
    sb->append(ANSI_COLOR_END);
  }
}

static void append_stderr(const char *header, const char *msg,
                          const char *ending, void *data) {
  StringBuffer *sb = (StringBuffer*)data;
  if (Util::s_stderr_color) {
    sb->append(Util::s_stderr_color);
  }
  sb->append(msg);
  sb->append(ending);
  if (Util::s_stderr_color) {
    sb->append(ANSI_COLOR_END);
  }
}

Variant DebuggerProxy::ExecutePHP(const std::string &php, String &output,
                                  bool log, int frame) {
  Variant ret;
  StringBuffer sb;
  g_context->setStdout(append_stdout, &sb);
  if (log) {
    Logger::SetThreadHook(append_stderr, &sb);
  }
  try {
    LVariableTable *vars = get_variable_table();
    FrameInjection *f = FrameInjection::GetStackFrame(frame);
    if (f && f->isEvalFrame()) {
      EvalFrameInjection *eframe = static_cast<EvalFrameInjection*>(f);
      vars = &eframe->getEnv();
    }

    String code(php.c_str(), php.size(), AttachLiteral);
    ret = eval(vars, Object(), code, false);

  } catch (Exception &e) {
    sb.append(Debugger::ColorStderr(String(e.what())));
  } catch (Object &e) {
    sb.append(Debugger::ColorStderr(e.toString()));
  } catch (...) {
    sb.append(Debugger::ColorStderr(String("(unknown exception was thrown")));
  }
  g_context->setStdout(NULL, NULL);
  if (log) {
    Logger::SetThreadHook(NULL, NULL);
  }
  output = sb.detach();
  return ret;
}

const char *DebuggerProxy::getThreadType() const {
  return isLocal() ? "Command Line Script" : "Dummy Sandbox";
}

DThreadInfoPtr DebuggerProxy::createThreadInfo(const std::string &desc) {
  DThreadInfoPtr info(new DThreadInfo());
  info->m_id = Process::GetThreadId();
  info->m_desc = desc;
  Transport *transport = g_context->getTransport();
  if (transport) {
    info->m_type = transport->getThreadTypeName();
    info->m_url = transport->getCommand();
  } else {
    info->m_type = getThreadType();
  }
  return info;
}

///////////////////////////////////////////////////////////////////////////////
}}
