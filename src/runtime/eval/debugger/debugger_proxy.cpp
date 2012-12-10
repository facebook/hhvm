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
#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/eval/debugger/cmd/cmd_flow_control.h>
#include <runtime/eval/debugger/cmd/cmd_jump.h>
#include <runtime/eval/debugger/cmd/cmd_signal.h>
#include <runtime/eval/debugger/cmd/cmd_machine.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>
#include <runtime/eval/eval.h>
#include <runtime/vm/debugger_hook.h>
#include <util/process.h>
#include <util/logger.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket, bool local)
    : m_stopped(false), m_local(local), m_dummySandbox(NULL),
      m_hasBreakPoints(false), m_threadMode(Normal), m_thread(0),
      m_signalThread(this, &DebuggerProxy::pollSignal),
      m_signum(CmdSignal::SignalNone) {
  m_thrift.create(socket);
  m_dummyInfo = DSandboxInfo::CreateDummyInfo((int64)this);
}

DebuggerProxy::~DebuggerProxy() {
  m_stopped = true;
  m_signalThread.waitForEnd();

  if (m_dummySandbox) {
    m_dummySandbox->stop();
  }
}

const char *DebuggerProxy::getThreadType() const {
  return isLocal() ? "Command Line Script" : "Dummy Sandbox";
}

DSandboxInfo DebuggerProxy::getSandbox() const {
  Lock lock(m_mutex);
  return m_sandbox;
}

std::string DebuggerProxy::getSandboxId() const {
  Lock lock(m_mutex);
  return m_sandbox.id();
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

bool DebuggerProxy::switchSandbox(const std::string &newId, bool force) {
  return Debugger::SwitchSandbox(shared_from_this(), newId, force);
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
    m_thread = (int64)Process::GetThreadId();
  }
  if (mode == Normal) {
    m_jump.reset();
    m_flow.reset();
  }
}

void DebuggerProxy::startDummySandbox() {
  m_dummySandbox =
    new DummySandbox(this, RuntimeOption::DebuggerDefaultSandboxPath,
                     RuntimeOption::DebuggerStartupDocument);
  m_dummySandbox->start();
}

void DebuggerProxy::notifyDummySandbox() {
  m_dummySandbox->notifySignal(CmdSignal::SignalBreak);
}

void DebuggerProxy::setBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  WriteLock lock(m_breakMutex);
  m_breakpoints = breakpoints;
  m_hasBreakPoints = !m_breakpoints.empty();
  m_breaksEnterFunc.clear();
  m_breaksEnterClsMethod.clear();
  for (unsigned int i = 0; i < m_breakpoints.size(); i++) {
    BreakPointInfoPtr bp = m_breakpoints[i];
    std::string funcFullName = bp->getFuncName();
    if (funcFullName.empty()) {
      continue;
    }
    {
      StringDataMap::accessor acc;
      const StringData* sd = StringData::GetStaticString(funcFullName);
      m_breaksEnterFunc.insert(acc, sd);
    }
    std::string clsName = bp->getClass();
    if (!clsName.empty()) {
      StringDataMap::accessor acc;
      const StringData* sd = StringData::GetStaticString(clsName);
      m_breaksEnterClsMethod.insert(acc, sd);
    }
  }
}

void DebuggerProxy::getBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  ReadLock lock(m_breakMutex);
  breakpoints = m_breakpoints;
}

bool DebuggerProxy::couldBreakEnterClsMethod(const StringData* className) {
  ReadLock lock(m_breakMutex);
  StringDataMap::const_accessor acc;
  return m_breaksEnterClsMethod.find(acc, className);
}

bool DebuggerProxy::couldBreakEnterFunc(const StringData* funcFullName) {
  ReadLock lock(m_breakMutex);
  StringDataMap::const_accessor acc;
  return m_breaksEnterFunc.find(acc, funcFullName);
}

void DebuggerProxy::getBreakClsMethods(
  std::vector<const StringData*>& classNames) {
  classNames.clear();
  WriteLock lock(m_breakMutex);
  for (StringDataMap::const_iterator iter = m_breaksEnterClsMethod.begin();
       iter != m_breaksEnterClsMethod.end(); ++iter) {
    classNames.push_back(iter->first);
  }
}

void DebuggerProxy::getBreakFuncs(
  std::vector<const StringData*>& funcFullNames) {
  funcFullNames.clear();
  WriteLock lock(m_breakMutex);
  for (StringDataMap::const_iterator iter = m_breaksEnterFunc.begin();
       iter != m_breaksEnterFunc.end(); ++iter) {
    funcFullNames.push_back(iter->first);
  }
}

bool DebuggerProxy::needInterrupt() {
  return m_hasBreakPoints || m_flow ||
         m_signum != CmdSignal::SignalNone;
}

bool DebuggerProxy::needInterruptForNonBreak() {
  return m_flow || m_signum != CmdSignal::SignalNone;
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

bool DebuggerProxy::send(DebuggerCommand *cmd) {
  return cmd->send(m_thrift);
}

void DebuggerProxy::startSignalThread() {
  m_signalThread.start();
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
    if (!cmd.onServerD(this)) break; // on socket error

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

    if (m_signum != CmdSignal::SignalNone) {
      Debugger::RequestInterrupt(shared_from_this());
    }
  }
}

void DebuggerProxy::forceQuit() {
  DSandboxInfo invalid;
  Lock l(this);
  m_sandbox = invalid;
  m_stopped = true;
  // the flag will take care of the rest
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
  StringBuffer *save = g_context->swapOutputBuffer(NULL);
  g_context->setStdout(append_stdout, &sb);
  if (log) {
    Logger::SetThreadHook(append_stderr, &sb);
  }
  ret = null;
  g_context->setStdout(NULL, NULL);
  g_context->swapOutputBuffer(save);
  if (log) {
    Logger::SetThreadHook(NULL, NULL);
  }
  output = sb.detach();
  return ret;
}

DThreadInfoPtr DebuggerProxy::createThreadInfo(const std::string &desc) {
  DThreadInfoPtr info(new DThreadInfo());
  info->m_id = (int64)Process::GetThreadId();
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
    if (!checkBreakPoints(cmd)) {
      // The breakpoint might have been removed while I'm waiting
      return false;
    }
  } else if (check && !checkJumpFlowBreak(cmd)) {
    return false;
  }

  if (m_thread == 0) {
    m_thread = self;
  }
  return true;
}

bool DebuggerProxy::checkBreakPoints(CmdInterrupt &cmd) {
  ReadLock lock(m_breakMutex);
  return cmd.shouldBreak(m_breakpoints);
}

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

void DebuggerProxy::checkStop() {
  if (m_stopped) {
    Debugger::RemoveProxy(shared_from_this());
    m_thrift.close();
    throw DebuggerClientExitException();
  }
}

void DebuggerProxy::processInterrupt(CmdInterrupt &cmd) {
  if (!cmd.onServerD(this)) {
    Debugger::RemoveProxy(shared_from_this()); // on socket error
    return;
  }

  // Once we sent an CmdInterrupt to client side, we should be considered idle
  RequestInjectionData &rjdata = ThreadInfo::s_threadInfo->m_reqInjectionData;
  rjdata.debuggerIdle = 0;

  while (true) {
    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::processInterrupt()")) {
      // we will wait forever until DebuggerClient sends us something
      checkStop();
    }
    checkStop();
    if (res) {
      m_flow = dynamic_pointer_cast<CmdFlowControl>(res);
      if (m_flow) {
        m_flow->onServerD(this);
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
      if (!res || !res->onServerD(this)) {
        Debugger::RemoveProxy(shared_from_this());
        return;
      }
    } catch (const DebuggerException &e) {
      throw;
    } catch (...) {
      Logger::Error("onServerD() throws non DebuggerException: %d",
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
        while ((prev = frame->getPrev())) {
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

///////////////////////////////////////////////////////////////////////////////
// DebuggerProxyVM

Variant DebuggerProxyVM::ExecutePHP(const std::string &php, String &output,
                                    bool log, int frame) {
  Variant ret;
  StringBuffer sb;
  StringBuffer *save = g_context->swapOutputBuffer(NULL);
  g_context->setStdout(append_stdout, &sb);
  if (log) {
    Logger::SetThreadHook(append_stderr, &sb);
  }
  try {
    String code(php.c_str(), php.size(), CopyString);
    g_vmContext->evalPHPDebugger((TypedValue*)&ret, code.get(), frame);
  } catch (InvalidFunctionCallException &e) {
    sb.append(Debugger::ColorStderr(String(e.what())));
    sb.append(Debugger::ColorStderr(
              "You may also need to connect to a host "
              "(e.g., machine connect localhost)."));
  } catch (Exception &e) {
    sb.append(Debugger::ColorStderr(String(e.what())));
  } catch (Object &e) {
    try {
      sb.append(Debugger::ColorStderr(e.toString()));
    } catch (BadTypeConversionException &e) {
      sb.append(Debugger::ColorStderr
                (String("(object without __toString() is thrown)")));
    }
  } catch (...) {
    sb.append(Debugger::ColorStderr(String("(unknown exception was thrown)")));
  }
  g_context->setStdout(NULL, NULL);
  g_context->swapOutputBuffer(save);
  if (log) {
    Logger::SetThreadHook(NULL, NULL);
  }
  output = sb.detach();
  return ret;
}

// There could be multiple breakpoints at one place but we can manage this
// with only one breakpoint.
BreakPointInfoPtr DebuggerProxyVM::getBreakPointAtCmd(CmdInterrupt& cmd) {
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp->m_state != BreakPointInfo::Disabled &&
        bp->match(cmd.getInterruptType(), *cmd.getSite())) {
      return bp;
    }
  }
  return BreakPointInfoPtr();
}


void DebuggerProxyVM::interrupt(CmdInterrupt &cmd) {
  changeBreakPointDepth(cmd);
  if (cmd.getInterruptType() != BreakPointReached &&
      cmd.getInterruptType() != HardBreakPoint) {
    DebuggerProxy::interrupt(cmd);
    return;
  }

  if (cmd.getInterruptType() != HardBreakPoint) {
    if (!needInterrupt()) return;
    // Modify m_lastLocFilter to save current location
    InterruptSiteVM *site = (InterruptSiteVM*)cmd.getSite();
    if (g_vmContext->m_lastLocFilter) {
      g_vmContext->m_lastLocFilter->clear();
    } else {
      g_vmContext->m_lastLocFilter = new VM::PCFilter();
    }
    if (debug && Trace::moduleEnabled(Trace::bcinterp, 5)) {
      Trace::trace("prepare source loc filter\n");
      const VM::OffsetRangeVec& offsets = site->getCurOffsetRange();
      for (VM::OffsetRangeVec::const_iterator it = offsets.begin();
           it != offsets.end(); ++it) {
        Trace::trace("block source loc in %s:%d: unit %p offset [%d, %d)\n",
                     site->getFile(), site->getLine0(),
                     site->getUnit(), it->m_base, it->m_past);
      }
    }
    g_vmContext->m_lastLocFilter->addRanges(site->getUnit(),
                                            site->getCurOffsetRange());
    // if the breakpoint is not to be processed, we should continue execution
    BreakPointInfoPtr bp = getBreakPointAtCmd(cmd);
    if (bp) {
      if (!bp->breakable(getRealStackDepth())) {
        return;
      } else {
        bp->unsetBreakable(getRealStackDepth());
      }
    }
  }

  DebuggerProxy::interrupt(cmd);
}

void DebuggerProxyVM::setBreakPoints(BreakPointInfoPtrVec& breakpoints) {
  DebuggerProxy::setBreakPoints(breakpoints);
  VM::phpBreakPointHook(this);
}

void DebuggerProxyVM::readInjTablesFromThread() {
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.dummySandbox) {
    return;
  }
  if (m_injTables) {
    delete m_injTables;
    m_injTables = NULL;
  }
  if (!g_vmContext->m_injTables) {
    return;
  }
  m_injTables = g_vmContext->m_injTables->clone();
}

void DebuggerProxyVM::writeInjTablesToThread() {
  if (g_vmContext->m_injTables) {
    delete g_vmContext->m_injTables;
    g_vmContext->m_injTables = NULL;
  }
  if (!m_injTables) {
    return;
  }
  g_vmContext->m_injTables = m_injTables->clone();
}

int DebuggerProxyVM::getRealStackDepth() {
  int depth = 0;
  VMExecutionContext* context = g_vmContext;
  HPHP::VM::ActRec *fp = context->getFP();
  if (!fp) return 0;

  while (fp != NULL) {
    fp = context->getPrevVMState(fp, NULL, NULL);
    depth++;
  }
  return depth;
}

int DebuggerProxyVM::getStackDepth() {
  int depth = 0;
  VMExecutionContext* context = g_vmContext;
  HPHP::VM::ActRec *fp = context->getFP();
  if (!fp) return 0;
  HPHP::VM::ActRec *prev = context->arGetSfp(fp);
  while (fp != prev) {
    fp = prev;
    prev = context->arGetSfp(fp);
    depth++;
  }
  return depth;
}

void DebuggerProxyVM::processFlowControl(CmdInterrupt &cmd) {
  switch (m_flow->getType()) {
    case DebuggerCommand::KindOfContinue:
      if (!m_flow->decCount()) {
        m_flow.reset();
      }
      break;
    case DebuggerCommand::KindOfStep:
      {
        // allows the breakpoint to be hit again when returns
        // from function call
        BreakPointInfoPtr bp = getBreakPointAtCmd(cmd);
        if (bp) {
          bp->setBreakable(getRealStackDepth());
        }
        break;
      }
    case DebuggerCommand::KindOfOut:
    case DebuggerCommand::KindOfNext:
      m_flow->setStackDepth(getStackDepth());
      m_flow->setVMDepth(g_vmContext->m_nesting);
      m_flow->setFileLine(cmd.getFileLine());
      break;
    default:
      ASSERT(false);
      break;
  }
}

/**
 * If a breakpoint is set at that depth,
 * this function clears the current depth information
 * after the breakpoint has passed
 */

void DebuggerProxyVM::changeBreakPointDepth(CmdInterrupt& cmd) {
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    // if the site changes, then update the breakpoint depth
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp->m_state != BreakPointInfo::Disabled &&
        !bp->match(cmd.getInterruptType(), *cmd.getSite())) {
      m_breakpoints[i]->changeBreakPointDepth(getRealStackDepth());
    }
  }
}

bool DebuggerProxyVM::breakByFlowControl(CmdInterrupt &cmd) {
  switch (m_flow->getType()) {
    case DebuggerCommand::KindOfStep: {
      if (!m_flow->decCount()) {
        // if the line changes and the stack depth is the same
        // pop the breakpoint depth stack
        m_flow.reset();
        return true;
      }
      break;
    }
    case DebuggerCommand::KindOfNext: {
      int currentVMDepth = g_vmContext->m_nesting;
      int currentStackDepth = getStackDepth();

      if (currentVMDepth <= m_flow->getVMDepth() &&
          currentStackDepth <= m_flow->getStackDepth() &&
          m_flow->getFileLine() != cmd.getFileLine()) {
            if (!m_flow->decCount()) {
              m_flow.reset();
              return true;
            }
      }

      break;
    }
    case DebuggerCommand::KindOfOut: {
      int currentVMDepth = g_vmContext->m_nesting;
      int currentStackDepth = getStackDepth();
      if (currentVMDepth < m_flow->getVMDepth()) {
        // Cut corner here, just break when cross VM boundary no matter how
        // many levels we want to go out of
        m_flow.reset();
        return true;
      } else if (m_flow->getStackDepth() - currentStackDepth >=
                 m_flow->getCount()) {
        m_flow.reset();
        return true;
      }
      break;
    }
    default:
      break;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
