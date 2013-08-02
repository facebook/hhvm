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
#include "hphp/runtime/debugger/debugger_proxy.h"
#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"
#include "hphp/runtime/debugger/cmd/cmd_flow_control.h"
#include "hphp/runtime/debugger/cmd/cmd_signal.h"
#include "hphp/runtime/debugger/cmd/cmd_machine.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/util/process.h"
#include "hphp/util/logger.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket, bool local)
    : m_stopped(false), m_local(local), m_dummySandbox(nullptr),
      m_hasBreakPoints(false), m_threadMode(Normal), m_thread(0),
      m_signalThread(this, &DebuggerProxy::pollSignal),
      m_okayToPoll(true), m_signum(CmdSignal::SignalNone) {
  TRACE(2, "DebuggerProxy::DebuggerProxy\n");
  m_thrift.create(socket);
  m_dummyInfo = DSandboxInfo::CreateDummyInfo((int64_t)this);
  Variant address;
  Variant port;
  std::string clientDetails;
  if (!local) {
    if (getClientConnectionInfo(ref(address), ref(port))) {
      clientDetails = folly::stringPrintf("From %s:%d",
                                          address.toString().data(),
                                          port.toInt32());
    } else {
      clientDetails = "Failed to get client connection details";
    }
  } else {
    clientDetails = "Local script connection";
  }
  Debugger::UsageLog("server", m_dummyInfo.id(), "connect", clientDetails);
}

// Cleanup all resources owned by this proxy, including any threads it
// owns. If a thread doesn't stop, return false so we can try again
// later. This can be called multiple times, and this function leaves
// the proxy usable by request threads which may still be handling an
// interrupt.
bool DebuggerProxy::cleanup(int timeout) {
  TRACE_RB(2, "DebuggerProxy::cleanup starting\n");
  // If we're not already marked as stopping then there may be other
  // threads still attempting to use this object!
  assert(m_stopped);
  // No more client operation is possible, so drop the connection.
  m_thrift.close();
  TRACE(2, "Stopping signal thread...\n");
  if (!m_signalThread.waitForEnd(timeout)) return false;
  TRACE(2, "Stopping dummy sandbox...\n");
  if (m_dummySandbox) {
    if (!m_dummySandbox->stop(timeout)) return false;
    delete m_dummySandbox;
    m_dummySandbox = nullptr;
  }
  TRACE_RB(2, "DebuggerProxy::cleanup complete\n");
  return true;
}

// Stop the proxy and ensure that no new uses of it can
// occur. Schedule it for final cleanup on another thread. This may be
// called from any thread wishing to stop the proxy, for any reason.
void DebuggerProxy::stop() {
  // Shared ref to keep us alive. Often the only reference to a proxy
  // is in the debugger's proxy map, which we're about to alter below.
  auto this_ = shared_from_this();
  Debugger::UsageLog("server", getSandboxId(), "disconnect");

  // After this we'll no longer be able to get this_ from
  // Debugger::findProxy(), which means no new interrupts for this
  // proxy. There may still be threads in existing interrupts, though.
  Debugger::RemoveProxy(this_);

  // Pop the signal polling thread, and any interrupting threads, out
  // of their event loops and cause them to exit.
  DSandboxInfo invalid;
  {
    Lock lock(this);
    m_sandbox = invalid;
    m_stopped = true;
  }

  // Retire the proxy, which will schedule it for cleanup and
  // destruction on another thread. Threads for the dummy sandbox and
  // signal polling can't destroy the proxy directly, since the proxy
  // owns them and will want to destroy them.
  Debugger::RetireProxy(this_);
}

// Stop the proxy, and stop execution of the current request.
void DebuggerProxy::stopAndThrow() {
  stop();
  throw DebuggerClientExitException();
}

const char *DebuggerProxy::getThreadType() const {
  TRACE(2, "DebuggerProxy::getThreadType\n");
  return isLocal() ? "Command Line Script" : "Dummy Sandbox";
}

DSandboxInfo DebuggerProxy::getSandbox() {
  TRACE(2, "DebuggerProxy::getSandbox\n");
  Lock lock(this);
  return m_sandbox;
}

std::string DebuggerProxy::getSandboxId() {
  TRACE(2, "DebuggerProxy::getSandboxId\n");
  Lock lock(this);
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

// Switch this proxy to debug the given sandbox.
bool DebuggerProxy::switchSandbox(const std::string &newId, bool force) {
  TRACE(2, "DebuggerProxy::switchSandbox\n");
  return Debugger::SwitchSandbox(shared_from_this(), newId, force);
}

// Callback made by Debugger::SwitchSandbox() when the switch is successful.
// NB: this is called with a read lock on the corresponding entry in the sandbox
// map.
void DebuggerProxy::updateSandbox(DSandboxInfoPtr sandbox) {
  TRACE(2, "DebuggerProxy::updateSandbox\n");
  Lock lock(this);
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
  if (m_dummySandbox) m_dummySandbox->notifySignal(CmdSignal::SignalBreak);
}

void DebuggerProxy::setBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  TRACE(2, "DebuggerProxy::setBreakPoints\n");
  // Hold the break mutex while we update the proxy's state. There's no need
  // to hold it over the longer operation to set breakpoints in each file later.
  {
    WriteLock lock(m_breakMutex);
    m_breakpoints = breakpoints;
    m_hasBreakPoints = !m_breakpoints.empty();
  }
  phpSetBreakPoints(this);
}

void DebuggerProxy::getBreakPoints(BreakPointInfoPtrVec &breakpoints) {
  TRACE(2, "DebuggerProxy::getBreakPoints\n");
  ReadLock lock(m_breakMutex);
  breakpoints = m_breakpoints;
}

// Proxy needs to be interrupted because it has something setup which may need
// processing; breakpoints, flow control commands, a signal.
bool DebuggerProxy::needInterrupt() {
  TRACE(2, "DebuggerProxy::needInterrupt\n");
  return m_hasBreakPoints || m_flow || m_signum != CmdSignal::SignalNone;
}

// We need VM interrupts if we're in a state that requires the debugger to be
// interrupted for every opcode.
bool DebuggerProxy::needVMInterrupts() {
  TRACE(2, "DebuggerProxy::needVMInterrupts\n");
  bool flowNeedsInterrupt = (m_flow && m_flow->needsVMInterrupt());
  return flowNeedsInterrupt || m_signum != CmdSignal::SignalNone;
}

// Handle an interrupt from the VM.
void DebuggerProxy::interrupt(CmdInterrupt &cmd) {
  TRACE_RB(2, "DebuggerProxy::interrupt\n");
  // Make any breakpoints that have passed breakable again.
  setBreakableForBreakpointsNotMatching(cmd);

  // At this point we have an interrupt, but we don't know if we're on the
  // thread the proxy considers "current".
  // NB: BreakPointReached really means we've got control of a VM thread from
  // the opcode hook. This could be for a breakpoint, stepping, etc.

  // Wait until this thread is the one this proxy wants to debug.
  if (!blockUntilOwn(cmd, true)) {
    return;
  }

  // We know we're on the "current" thread, so we can process any active flow
  // command, stop if we're at a breakpoint, handle other interrupts, etc.
  if (checkFlowBreak(cmd)) {
    // We've hit a breakpoint and now need to make sure that breakpoints
    // wont be hit again for this site until control leaves this site.
    // (Breakpoints can still get hit if control reaches this site during
    // a call that is part of this site because the flags are stacked.)
    unsetBreakableForBreakpointsMatching(cmd);
    while (true) {
      try {
        // We're about to send the client an interrupt and start
        // waiting for commands back from it. Disable signal polling
        // during this time, since our protocol requires that only one
        // thread talk to the client at a time.
        disableSignalPolling();
        SCOPE_EXIT { enableSignalPolling(); };
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

// Sends a copy of the given command to the associated client
// using the buffer in m_thrift. Returns false if an exception
// occurs during the send (typically a socket error).
bool DebuggerProxy::sendToClient(DebuggerCommand *cmd) {
  TRACE(2, "DebuggerProxy::sendToClient\n");
  return cmd->send(m_thrift);
}

// Allow the signal polling thread to send CmdSignal messages to the
// client to see if it there is a signal to repond to.
void DebuggerProxy::enableSignalPolling() {
  Lock lock(m_signalMutex);
  m_okayToPoll = true;
}

// Prevent the signal polling thread from sending CmdSignal messages
// to the client. Polling is disabled while a thread is stopped at an
// interrupt and responding to messages from the client.
void DebuggerProxy::disableSignalPolling() {
  Lock lock(m_signalMutex);
  m_okayToPoll = false;
  // Drop any pending signal since we're about to start processing
  // interrupts again anyway.
  m_signum = CmdSignal::SignalNone;
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
  TRACE_RB(2, "DebuggerProxy::pollSignal: starting\n");
  int signalTimeout = RuntimeOption::DebuggerSignalTimeout;
  while (!m_stopped) {
    sleep(1);

    // Block any threads that might be interrupting from communicating with the
    // client until we're done with this poll.
    Lock lock(m_signumMutex);

    // After DebuggerSignalTimeout seconds that no active thread picks
    // up the signal, we send it to dummy sandbox.
    if ((m_signum != CmdSignal::SignalNone) && m_dummySandbox &&
        (--signalTimeout <= 0)) {
      TRACE_RB(2, "DebuggerProxy::pollSignal: sending to dummy sandbox\n");
      m_dummySandbox->notifySignal(m_signum);
      m_signum = CmdSignal::SignalNone;
    }

    // Don't actually poll if another thread is already in a command
    // processing loop with the client.
    if (!m_okayToPoll) continue;

    // Send CmdSignal over to the client and wait for a response.
    CmdSignal cmd;
    if (!cmd.onServer(*this)) {
      TRACE_RB(2, "DebuggerProxy::pollSignal: "
               "Failed to send CmdSignal to client\n");
      break;
    }

    // We've sent the client a command, and we expect an immediate
    // response. Wait 10 times to give it a chance on especially
    // overloaded computers.
    DebuggerCommandPtr res;
    for (int i = 0; i < 10; i++) {
      if (DebuggerCommand::Receive(m_thrift, res,
                                   "DebuggerProxy::pollSignal()")) break;
      if (m_stopped) {
        TRACE_RB(2, "DebuggerProxy::pollSignal: "
                 "signal thread asked to stop while waiting "
                 "for CmdSignal back from the client\n");
        break;
      }
    }
    if (!res) {
      if (!m_stopped) {
        TRACE_RB(2, "DebuggerProxy::pollSignal: "
                 "Failed to get CmdSignal back from client\n");
      }
      break;
    }

    CmdSignalPtr sig = dynamic_pointer_cast<CmdSignal>(res);
    if (!sig) {
      TRACE_RB(2, "DebuggerProxy::pollSignal: "
               "bad response from signal polling: %d", res->getType());
      break;
    }

    auto newSignum = sig->getSignal();

    if (newSignum != CmdSignal::SignalNone) {
      TRACE_RB(2, "DebuggerProxy::pollSignal: "
               "got interrupt signal from client\n");
      m_signum = newSignum;
      signalTimeout = RuntimeOption::DebuggerSignalTimeout;
      Debugger::RequestInterrupt(shared_from_this());
    }
  }
  if (!m_stopped) {
    // We've noticed that the socket has closed. Stop and destory this proxy.
    TRACE_RB(2, "DebuggerProxy::pollSignal: "
             "lost communication with the client, stopping proxy\n");
    stop();
  }
  TRACE_RB(2, "DebuggerProxy::pollSignal: ended\n");
}

// Grab the ip address and port of the client that is connected to this proxy.
bool DebuggerProxy::getClientConnectionInfo(VRefParam address,
                                            VRefParam port) {
  Resource s(getSocket().get());
  return f_socket_getpeername(s, address, port);
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

// Passed to the ExecutionContext during Eval to add writes to stdout
// to the output buffer string.
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

// Passed to the ExecutionContext during Eval to add writes to stderr
// to the output buffer string.
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
// thread.
// NB: if the thread is not the current thread, and we're asked to check
// breakpoints, then if there are no breakpoints which could effect this
// thread we simply return false and stop processing the current interrupt.
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
  }

  // This thread is now the one the proxy considers the current thread.
  if (m_thread == 0) {
    m_thread = self;
  }
  return true;
}

// Checks whether the cmd has any breakpoints that match the current Site.
// Also returns true for cmds that should always break, like SessionStarted,
// and returns true when we have special modes setup for, say, breaking on
// RequestEnded, PSPEnded, etc.
bool DebuggerProxy::checkBreakPoints(CmdInterrupt &cmd) {
  TRACE(2, "DebuggerProxy::checkBreakPoints\n");
  ReadLock lock(m_breakMutex);
  return cmd.shouldBreak(*this, m_breakpoints, getRealStackDepth());
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
  // vice versa, so to give a flow cmd a chance to react.
  // Note: a Continue cmd is a bit special. We need to process it _only_ if
  // we decide to break.
  bool fcShouldBreak = false; // should I break according to flow control?
  bool bpShouldBreak = false; // should I break according to breakpoints?

  if ((cmd.getInterruptType() == BreakPointReached ||
       cmd.getInterruptType() == HardBreakPoint ||
       cmd.getInterruptType() == ExceptionHandler) && m_flow) {
    if (!m_flow->is(DebuggerCommand::KindOfContinue)) {
      m_flow->onBeginInterrupt(*this, cmd);
      if (m_flow->complete()) {
        fcShouldBreak = true;
        m_flow.reset();
      }
    }
  }

  // NB: this also checks whether we should be stopping at special interrupt
  // sites, like SessionStarted, RequestEnded, ExceptionThrown, etc.
  bpShouldBreak = checkBreakPoints(cmd);

  // This is done before KindOfContinue testing.
  if (!fcShouldBreak && !bpShouldBreak) {
    return false;
  }

  if ((cmd.getInterruptType() == BreakPointReached ||
       cmd.getInterruptType() == HardBreakPoint) && m_flow) {
    if (m_flow->is(DebuggerCommand::KindOfContinue)) {
      m_flow->onBeginInterrupt(*this, cmd);
      if (m_flow->complete()) m_flow.reset();
      return false;
    }
  }

  return true;
}

void DebuggerProxy::checkStop() {
  TRACE(5, "DebuggerProxy::checkStop\n");
  if (m_stopped) stopAndThrow();
}

void DebuggerProxy::processInterrupt(CmdInterrupt &cmd) {
  TRACE_RB(2, "DebuggerProxy::processInterrupt\n");
  // Do the server-side work for this interrupt, which just notifies the client.
  if (!cmd.onServer(*this)) {
    TRACE_RB(1, "Failed to send CmdInterrupt to client\n");
    stopAndThrow();
  }

  Debugger::UsageLogInterrupt("server", getSandboxId(), cmd);

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
      TRACE_RB(2, "Proxy got cmd type %d\n", res->getType());
      Debugger::UsageLog("server", getSandboxId(),
                         boost::lexical_cast<string>(res->getType()));
      // Any control flow command gets installed here and we continue execution.
      m_flow = dynamic_pointer_cast<CmdFlowControl>(res);
      if (m_flow) {
        m_flow->onSetup(*this, cmd);
        if (!m_flow->complete()) {
          TRACE_RB(2, "Incomplete flow command %d remaining on proxy for "
                   "further processing\n", m_flow->getType());
          if (m_threadMode == Normal) {
            switchThreadMode(Sticky);
          }
        } else {
          // The flow cmd has determined that it is done with its work and
          // doesn't need to remain for later processing.
          TRACE_RB(2, "Flow command %d completed\n", m_flow->getType());
          m_flow.reset();
        }
        return;
      }
      if (res->is(DebuggerCommand::KindOfQuit)) {
        TRACE_RB(2, "Received quit command\n");
        res->onServer(*this); // acknowledge receipt so that client can quit.
        stopAndThrow();
      }
    }
    bool cmdFailure = false;
    try {
      // Perform the server-side work for this command.
      if (res) {
        if (!res->onServer(*this)) {
          TRACE_RB(1, "Failed to execute cmd %d from client\n", res->getType());
          cmdFailure = true;
        }
      } else {
        TRACE_RB(1, "Failed to receive cmd from client\n");
        cmdFailure = true;
      }
    } catch (const DebuggerException &e) {
      throw;
    } catch (const std::exception& e) {
      Logger::Warning(DEBUGGER_LOG_TAG
       "Cmd type %d onServer() threw exception %s", res->getType(), e.what());
      cmdFailure = true;
    } catch (...) {
      Logger::Warning(DEBUGGER_LOG_TAG
       "Cmd type %d onServer() threw non standard exception", res->getType());
      cmdFailure = true;
    }
    if (cmdFailure) stopAndThrow();
    if (res->shouldExitInterrupt()) return;
  }
}

///////////////////////////////////////////////////////////////////////////////

Variant DebuggerProxy::ExecutePHP(const std::string &php, String &output,
                                  int frame, int flags) {
  TRACE(2, "DebuggerProxy::ExecutePHP\n");
  Variant ret;
  // Wire up stdout and stderr to our own string buffer so we can pass
  // any output back to the client.
  StringBuffer sb;
  StringBuffer *save = g_context->swapOutputBuffer(nullptr);
  g_context->setStdout(append_stdout, &sb);
  if (flags & ExecutePHPFlagsLog) {
    Logger::SetThreadHook(append_stderr, &sb);
  }
  try {
    String code(php.c_str(), php.size(), CopyString);
    // @TODO: enable this once task #2608250 is completed.
#if 0
    // We're about to start executing more PHP. This is typically done
    // in response to commands from the client, and the client expects
    // those commands to send more interrupts since, of course, the
    // user might want to debug the code we're about to run. If we're
    // already processing an interrupt, enable signal polling around
    // the execution of the new PHP to ensure that we can handle
    // signals while doing so.
    if (flags & ExecutePHPFlagsAtInterrupt) enableSignalPolling();
    SCOPE_EXIT {
      if (flags & ExecutePHPFlagsAtInterrupt) disableSignalPolling();
    };
#endif
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
  if (flags & ExecutePHPFlagsLog) {
    Logger::SetThreadHook(nullptr, nullptr);
  }
  output = sb.detach();
  return ret;
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
  ActRec *prev = fp->arGetSfp();
  while (fp != prev) {
    fp = prev;
    prev = fp->arGetSfp();
    depth++;
  }
  return depth;
}

// Allow breaks for previously disabled breakpoints that do not match the site
// of cmd. (Call this when processing an interrupt since this probably means
// that execution has moved away from the previous interrupt site.)
void DebuggerProxy::setBreakableForBreakpointsNotMatching(CmdInterrupt& cmd) {
  TRACE(2, "DebuggerProxy::setBreakableForBreakpointsNotMatching\n");
  auto stackDepth = getRealStackDepth();
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp != nullptr && bp->m_state != BreakPointInfo::Disabled &&
        !bp->match(*this, cmd.getInterruptType(), *cmd.getSite())) {
      bp->setBreakable(stackDepth);
    }
  }
}

// Do not allow further breaks on the site of cmd, except during
// calls made from the current site.
void DebuggerProxy::unsetBreakableForBreakpointsMatching(CmdInterrupt& cmd) {
  TRACE(2, "DebuggerProxy::unsetBreakableForBreakpointsMatching\n");
  auto stackDepth = getRealStackDepth();
  for (unsigned int i = 0; i < m_breakpoints.size(); ++i) {
    BreakPointInfoPtr bp = m_breakpoints[i];
    if (bp != nullptr && bp->m_state != BreakPointInfo::Disabled &&
        bp->match(*this, cmd.getInterruptType(), *cmd.getSite())) {
      bp->unsetBreakable(stackDepth);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
