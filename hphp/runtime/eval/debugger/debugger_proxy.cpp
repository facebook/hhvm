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
#include <runtime/eval/debugger/cmd/cmd_signal.h>
#include <runtime/eval/debugger/cmd/cmd_machine.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/runtime_option.h>
#include <runtime/vm/debugger_hook.h>
#include <util/process.h>
#include <util/logger.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static const Trace::Module TRACEMOD = Trace::debugger;

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket, bool local)
    : m_stopped(false), m_local(local), m_dummySandbox(nullptr),
      m_hasBreakPoints(false), m_threadMode(Normal), m_thread(0),
      m_signalThread(this, &DebuggerProxy::pollSignal),
      m_signum(CmdSignal::SignalNone), m_injTables(nullptr) {
  TRACE(2, "DebuggerProxy::DebuggerProxy\n");
  m_thrift.create(socket);
  m_dummyInfo = DSandboxInfo::CreateDummyInfo((int64_t)this);
}

DebuggerProxy::~DebuggerProxy() {
  TRACE(2, "DebuggerProxy::~DebuggerProxy\n");
  m_stopped = true;
  m_signalThread.waitForEnd();

  if (m_dummySandbox) {
    m_dummySandbox->stop();
  }

  delete m_injTables;
}

const char *DebuggerProxy::getThreadType() const {
  TRACE(2, "DebuggerProxy::getThreadType\n");
  return isLocal() ? "Command Line Script" : "Dummy Sandbox";
}

DSandboxInfo DebuggerProxy::getSandbox() const {
  TRACE(2, "DebuggerProxy::getSandbox\n");
  Lock lock(m_mutex);
  return m_sandbox;
}

std::string DebuggerProxy::getSandboxId() const {
  TRACE(2, "DebuggerProxy::getSandboxId\n");
  Lock lock(m_mutex);
  return m_sandbox.id();
}

void DebuggerProxy::getThreads(DThreadInfoPtrVec &threads) {
  TRACE(2, "DebuggerProxy::getThreads\n");
  Lock lock(this);
  std::stack<void *> &interrupts =
    ThreadInfo::s_threadInfo->m_reqInjectionData.interrupts;
  assert(!interrupts.empty());
  if (!interrupts.empty()) {
    CmdInterrupt *tint = (CmdInterrupt*)interrupts.top();
    assert(tint);
    if (tint) {
      threads.push_back(createThreadInfo(tint->desc()));
    }
  }
  for (std::map<int64_t, DThreadInfoPtr>::const_iterator iter =
         m_threads.begin(); iter != m_threads.end(); ++iter) {
    DThreadInfoPtr thread(new DThreadInfo());
    *thread = *iter->second;
    threads.push_back(thread);
  }
}

bool DebuggerProxy::switchSandbox(const std::string &newId, bool force) {
  TRACE(2, "DebuggerProxy::switchSandbox\n");
  return Debugger::SwitchSandbox(shared_from_this(), newId, force);
}

void DebuggerProxy::updateSandbox(DSandboxInfoPtr sandbox) {
  TRACE(2, "DebuggerProxy::updateSandbox\n");
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
  TRACE(2, "DebuggerProxy::switchThread\n");
  Lock lock(this);
  if (m_threads.find(thread->m_id) != m_threads.end()) {
    m_newThread = thread;
    return true;
  }
  return false;
}

void DebuggerProxy::switchThreadMode(ThreadMode mode,
                                     int64_t threadId /* = 0 */) {
  TRACE(2, "DebuggerProxy::switchThreadMode\n");
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
    m_thread = (int64_t)Process::GetThreadId();
  }
  if (mode == Normal) {
    m_flow.reset();
  }
}

void DebuggerProxy::startDummySandbox() {
  TRACE(2, "DebuggerProxy::startDummySandbox\n");
  m_dummySandbox =
    new DummySandbox(this, RuntimeOption::DebuggerDefaultSandboxPath,
                     RuntimeOption::DebuggerStartupDocument);
  m_dummySandbox->start();
}

void DebuggerProxy::notifyDummySandbox() {
  TRACE(2, "DebuggerProxy::notifyDummySandbox\n");
  m_dummySandbox->notifySignal(CmdSignal::SignalBreak);
}

// Hold the entire set of breakpoints, and sift breakpoints by function and
// class name into separate containers for later.
void DebuggerProxy::setBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  TRACE(2, "DebuggerProxy::setBreakPoints\n");
  // Hold the break mutex while we update the proxy's state. There's no need
  // to hold it over the longer operation to set breakpoints in each file later.
  {
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
  VM::phpSetBreakPointsInAllFiles(this); // Apply breakpoints to the code.
}

void DebuggerProxy::getBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  TRACE(2, "DebuggerProxy::getBreakPoints\n");
  ReadLock lock(m_breakMutex);
  breakpoints = m_breakpoints;
}

bool DebuggerProxy::couldBreakEnterClsMethod(const StringData* className) {
  TRACE(2, "DebuggerProxy::couldBreakEnterClsMethod\n");
  ReadLock lock(m_breakMutex);
  StringDataMap::const_accessor acc;
  return m_breaksEnterClsMethod.find(acc, className);
}

bool DebuggerProxy::couldBreakEnterFunc(const StringData* funcFullName) {
  TRACE(2, "DebuggerProxy::couldBreakEnterFunc\n");
  ReadLock lock(m_breakMutex);
  StringDataMap::const_accessor acc;
  return m_breaksEnterFunc.find(acc, funcFullName);
}

void DebuggerProxy::getBreakClsMethods(
  std::vector<const StringData*>& classNames) {
  TRACE(2, "DebuggerProxy::getBreakClsMethods\n");
  classNames.clear();
  WriteLock lock(m_breakMutex); // Write lock in case iteration causes a re-hash
  for (StringDataMap::const_iterator iter = m_breaksEnterClsMethod.begin();
       iter != m_breaksEnterClsMethod.end(); ++iter) {
    classNames.push_back(iter->first);
  }
}

void DebuggerProxy::getBreakFuncs(
  std::vector<const StringData*>& funcFullNames) {
  TRACE(2, "DebuggerProxy::getBreakFuncs\n");
  funcFullNames.clear();
  WriteLock lock(m_breakMutex); // Write lock in case iteration causes a re-hash
  for (StringDataMap::const_iterator iter = m_breaksEnterFunc.begin();
       iter != m_breaksEnterFunc.end(); ++iter) {
    funcFullNames.push_back(iter->first);
  }
}

bool DebuggerProxy::needInterrupt() {
  TRACE(2, "DebuggerProxy::needInterrupt\n");
  return m_hasBreakPoints || m_flow ||
         m_signum != CmdSignal::SignalNone;
}

bool DebuggerProxy::needInterruptForNonBreak() {
  TRACE(2, "DebuggerProxy::needInterruptForNonBreak\n");
  return m_flow || m_signum != CmdSignal::SignalNone;
}

// Handle an interrupt from the VM.
void DebuggerProxy::interrupt(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::interrupt\n");
  changeBreakPointDepth(cmd);

  if (cmd.getInterruptType() == BreakPointReached) {
    if (!needInterrupt()) return;

    // NB: stepping is represented as a BreakPointReached interrupt.

    // Modify m_lastLocFilter to save current location. This will short-circuit
    // the work done up in phpDebuggerOpcodeHook() and ensure we don't break on
    // this line until we're completely off of it.
    InterruptSite *site = cmd.getSite();
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
    // If the breakpoint is not to be processed, we should continue execution.
    BreakPointInfoPtr bp = getBreakPointAtCmd(cmd);
    if (bp) {
      if (!bp->breakable(getRealStackDepth())) {
        return;
      } else {
        bp->unsetBreakable(getRealStackDepth());
      }
    }
  }

  // Wait until this thread is the thread this proxy wants to debug.
  // NB: breakpoints and control flow checks happen here, too, and return false
  // if we're not done with the flow, or not at a breakpoint, etc.
  if (!blockUntilOwn(cmd, true)) {
    return;
  }
  if (processFlowBreak(cmd)) {
    while (true) {
      try {
        Lock lock(m_signalMutex);
        m_signum = CmdSignal::SignalNone;
        processInterrupt(cmd);
      } catch (const DebuggerException &e) {
        switchThreadMode(Normal);
        throw;
      } catch (...) {
        assert(false); // no other exceptions should be seen here
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

bool DebuggerProxy::sendToClient(DebuggerCommand *cmd) {
  TRACE(2, "DebuggerProxy::sendToClient\n");
  return cmd->send(m_thrift);
}

void DebuggerProxy::startSignalThread() {
  TRACE(2, "DebuggerProxy::startSignalThread\n");
  m_signalThread.start();
}

// This gets it's own thread, and polls the client once per second to see if
// there is a signal, i.e., if the user has pressed Ctrl-C, etc. If there is a
// signal, it is passed as an interrupt to the proxy in an attempt to get other
// threads in the sandbox to stop.
//
// If another thread in the sandbox fails to stop and consume the signal then
// it will be passed to the dummy sandbox instead.
void DebuggerProxy::pollSignal() {
  TRACE(2, "DebuggerProxy::pollSignal\n");
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

    // Send CmdSignal over to the client and wait for a response.
    CmdSignal cmd;
    if (!cmd.onServer(this)) break; // on socket error

    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::pollSignal()")) {
      // We will wait forever until DebuggerClient sends us something.
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
  TRACE(2, "DebuggerProxy::forceQuit\n");
  DSandboxInfo invalid;
  Lock l(this);
  m_sandbox = invalid;
  m_stopped = true;
  // the flag will take care of the rest
}

///////////////////////////////////////////////////////////////////////////////
// helpers

std::string DebuggerProxy::MakePHP(const std::string &php) {
  TRACE(2, "DebuggerProxy::MakePHP\n");
  return "<?php " + php + ";";
}

std::string DebuggerProxy::MakePHPReturn(const std::string &php) {
  TRACE(2, "DebuggerProxy::MakePHPReturn\n");
  return "<?php return " + php + ";";
}

static void append_stdout(const char *s, int len, void *data) {
  TRACE(2, "DebuggerProxy::append_stdout\n");
  StringBuffer *sb = (StringBuffer*)data;
  if (s_stdout_color) {
    sb->append(s_stdout_color);
  }
  sb->append(s, len);
  if (s_stdout_color) {
    sb->append(ANSI_COLOR_END);
  }
}

static void append_stderr(const char *header, const char *msg,
                          const char *ending, void *data) {
  TRACE(2, "DebuggerProxy::append_stderr\n");
  StringBuffer *sb = (StringBuffer*)data;
  if (s_stderr_color) {
    sb->append(s_stderr_color);
  }
  sb->append(msg);
  sb->append(ending);
  if (s_stderr_color) {
    sb->append(ANSI_COLOR_END);
  }
}

DThreadInfoPtr DebuggerProxy::createThreadInfo(const std::string &desc) {
  TRACE(2, "DebuggerProxy::createThreadInfo\n");
  DThreadInfoPtr info(new DThreadInfo());
  info->m_id = (int64_t)Process::GetThreadId();
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

// Waits until this thread is the one that the proxy considers the current
// thread. This also check to see if the given cmd has any breakpoints or
// flow control that we should stop for. Note: while stepping, pretty much all
// of the stepping logic is handled below here and this will return false if
// the stepping operation has not completed.
bool DebuggerProxy::blockUntilOwn(CmdInterrupt &cmd, bool check) {
  TRACE(2, "DebuggerProxy::blockUntilOwn\n");
  int64_t self = cmd.getThreadId();

  Lock lock(this);
  if (m_thread && m_thread != self) {
    if (check && (m_threadMode == Exclusive || !checkBreakPoints(cmd))) {
      // Flow control commands only belong to sticky thread
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
        m_flow.reset();
      }
    }
    m_threads.erase(self);
    if (m_stopped) return false;
    if (!checkBreakPoints(cmd)) {
      // The breakpoint might have been removed while I'm waiting
      return false;
    }
  } else if (check && !checkFlowBreak(cmd)) {
    return false;
  }

  if (m_thread == 0) {
    m_thread = self;
  }
  return true;
}

// Checks whether the cmd has any breakpoints that match the current Site.
// Also returns true for cmds that have should always break.
bool DebuggerProxy::checkBreakPoints(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::checkBreakPoints\n");
  ReadLock lock(m_breakMutex);
  return cmd.shouldBreak(m_breakpoints);
}

// Check if we should stop due to flow control, breakpoints, and signals.
bool DebuggerProxy::checkFlowBreak(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::checkFlowBreak\n");
  // If there is an outstanding Ctrl-C from the client, go ahead and break now.
  // Note: this stops any flow control command we might have in-flight.
  if (m_signum == CmdSignal::SignalBreak) {
    Lock lock(m_signumMutex);
    if (m_signum == CmdSignal::SignalBreak) {
      m_signum = CmdSignal::SignalNone;
      m_flow.reset();
      return true;
    }
  }

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


  return true;
}

bool DebuggerProxy::processFlowBreak(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::processFlowBreak\n");
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
  TRACE(2, "DebuggerProxy::checkStop\n");
  if (m_stopped) {
    Debugger::RemoveProxy(shared_from_this());
    m_thrift.close();
    throw DebuggerClientExitException();
  }
}

void DebuggerProxy::processInterrupt(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::processInterrupt\n");
  // Do the server-side work for this cmd, which just notifies the client.
  if (!cmd.onServer(this)) {
    Debugger::RemoveProxy(shared_from_this()); // on socket error
    return;
  }

  // Wait for commands from the debugger client and process them. We'll stay
  // here until we get a command that should cause the thread to continue.
  while (true) {
    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::processInterrupt()")) {
      // we will wait forever until DebuggerClient sends us something
      checkStop();
    }
    checkStop();
    if (res) {
      // Any control flow command gets installed here and we continue execution.
      m_flow = dynamic_pointer_cast<CmdFlowControl>(res);
      if (m_flow) {
        m_flow->onServer(this);
        processFlowControl(cmd);
        if (m_flow && m_threadMode == Normal) {
          switchThreadMode(Sticky);
        }
        return;
      }
      if (res->is(DebuggerCommand::KindOfQuit)) {
        Debugger::RemoveProxy(shared_from_this());
        throw DebuggerClientExitException();
      }
    }
    try {
      // Perform the server-side work for this command.
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

///////////////////////////////////////////////////////////////////////////////

Variant DebuggerProxy::ExecutePHP(const std::string &php, String &output,
                                  bool log, int frame) {
  TRACE(2, "DebuggerProxy::ExecutePHP\n");
  Variant ret;
  StringBuffer sb;
  StringBuffer *save = g_context->swapOutputBuffer(nullptr);
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
  g_context->setStdout(nullptr, nullptr);
  g_context->swapOutputBuffer(save);
  if (log) {
    Logger::SetThreadHook(nullptr, nullptr);
  }
  output = sb.detach();
  return ret;
}

// There could be multiple breakpoints at one place but we can manage this
// with only one breakpoint.
BreakPointInfoPtr DebuggerProxy::getBreakPointAtCmd(CmdInterrupt& cmd) {
  TRACE(2, "DebuggerProxy::getBreakPointAtCmd\n");
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp->m_state != BreakPointInfo::Disabled &&
        bp->match(cmd.getInterruptType(), *cmd.getSite())) {
      return bp;
    }
  }
  return BreakPointInfoPtr();
}

void DebuggerProxy::readInjTablesFromThread() {
  TRACE(2, "DebuggerProxy::readInjTablesFromThread\n");
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.dummySandbox) {
    return;
  }
  if (m_injTables) {
    delete m_injTables;
    m_injTables = nullptr;
  }
  if (!g_vmContext->m_injTables) {
    return;
  }
  m_injTables = g_vmContext->m_injTables->clone();
}

void DebuggerProxy::writeInjTablesToThread() {
  TRACE(2, "DebuggerProxy::writeInjTablesToThread\n");
  if (g_vmContext->m_injTables) {
    delete g_vmContext->m_injTables;
    g_vmContext->m_injTables = nullptr;
  }
  if (!m_injTables) {
    return;
  }
  g_vmContext->m_injTables = m_injTables->clone();
}

int DebuggerProxy::getRealStackDepth() {
  TRACE(2, "DebuggerProxy::getRealStackDepth\n");
  int depth = 0;
  VMExecutionContext* context = g_vmContext;
  ActRec *fp = context->getFP();
  if (!fp) return 0;

  while (fp != nullptr) {
    fp = context->getPrevVMState(fp, nullptr, nullptr);
    depth++;
  }
  return depth;
}

int DebuggerProxy::getStackDepth() {
  TRACE(2, "DebuggerProxy::getStackDepth\n");
  int depth = 0;
  VMExecutionContext* context = g_vmContext;
  ActRec *fp = context->getFP();
  if (!fp) return 0;
  ActRec *prev = context->arGetSfp(fp);
  while (fp != prev) {
    fp = prev;
    prev = context->arGetSfp(fp);
    depth++;
  }
  return depth;
}

// Handle a continue cmd, or setup stepping.
void DebuggerProxy::processFlowControl(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::processFlowControl\n");
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
      assert(false);
      break;
  }
}

/**
 * If a breakpoint is set at that depth,
 * this function clears the current depth information
 * after the breakpoint has passed
 */

void DebuggerProxy::changeBreakPointDepth(CmdInterrupt& cmd) {
  TRACE(2, "DebuggerProxy::changeBreakPointDepth\n");
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    // if the site changes, then update the breakpoint depth
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp->m_state != BreakPointInfo::Disabled &&
        !bp->match(cmd.getInterruptType(), *cmd.getSite())) {
      m_breakpoints[i]->changeBreakPointDepth(getRealStackDepth());
    }
  }
}

// Determine if an outstanding flow control cmd has run it's course and we
// should stop execution.
bool DebuggerProxy::breakByFlowControl(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::breakByFlowControl\n");
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
