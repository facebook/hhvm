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

#include "hphp/runtime/debugger/debugger.h"
#include <set>
#include <stack>
#include <vector>
#include "hphp/runtime/debugger/debugger_server.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/text-color.h"
#include "hphp/util/logger.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

Debugger Debugger::s_debugger;
bool Debugger::s_clientStarted = false;

bool Debugger::StartServer() {
  TRACE(2, "Debugger::StartServer\n");
  return DebuggerServer::Start();
}

DebuggerProxyPtr Debugger::StartClient(const DebuggerClientOptions &options) {
  TRACE(2, "Debugger::StartClient\n");
  SmartPtr<Socket> localProxy = DebuggerClient::Start(options);
  if (localProxy.get()) {
    s_clientStarted = true;
    return CreateProxy(localProxy, true);
  }
  return DebuggerProxyPtr();
}

void Debugger::Stop() {
  TRACE(2, "Debugger::Stop\n");
  LogShutdown(ShutdownKind::Normal);
  s_debugger.m_proxyMap.clear();
  DebuggerServer::Stop();
  CleanupRetiredProxies();
  if (s_clientStarted) {
    DebuggerClient::Stop();
  }
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::RegisterSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::RegisterSandbox\n");
  s_debugger.registerSandbox(sandbox);
}

void Debugger::UnregisterSandbox(const String& id) {
  TRACE(2, "Debugger::UnregisterSandbox\n");
  s_debugger.unregisterSandbox(id.get());
}

DebuggerProxyPtr Debugger::CreateProxy(SmartPtr<Socket> socket, bool local) {
  TRACE(2, "Debugger::CreateProxy\n");
  return s_debugger.createProxy(socket, local);
}

void Debugger::RemoveProxy(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::RemoveProxy\n");
  s_debugger.removeProxy(proxy);
}

int Debugger::CountConnectedProxy() {
  TRACE(7, "Debugger::CountConnectedProxy\n");
  return s_debugger.countConnectedProxy();
}

DebuggerProxyPtr Debugger::GetProxy() {
  TRACE(7, "Debugger::GetProxy\n");
  const String& sandboxId = g_context->getSandboxId();
  return s_debugger.findProxy(sandboxId.get());
}

bool Debugger::SwitchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
  TRACE(2, "Debugger::SwitchSandbox\n");
  return s_debugger.switchSandbox(proxy, newId, force);
}

void Debugger::GetRegisteredSandboxes(
    std::vector<DSandboxInfoPtr> &sandboxes) {
  TRACE(2, "Debugger::GetRegisteredSandboxes\n");
  s_debugger.getSandboxes(sandboxes);
}

bool Debugger::IsThreadDebugging(int64_t id) {
  TRACE(2, "Debugger::IsThreadDebugging\n");
  return s_debugger.isThreadDebugging(id);
}

void Debugger::RequestInterrupt(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::RequestInterrupt\n");
  s_debugger.requestInterrupt(proxy);
}

void Debugger::RetireProxy(DebuggerProxyPtr proxy) {
  s_debugger.retireProxy(proxy);
}

void Debugger::CleanupRetiredProxies() {
  s_debugger.cleanupRetiredProxies();
}

void Debugger::DebuggerSession(const DebuggerClientOptions& options,
                                bool restart) {
  TRACE(2, "Debugger::DebuggerSession: restart=%d\n", restart);
  if (options.extension.empty()) {
    // even if it's empty, still need to call for warmup
    hphp_invoke_simple("", true); // not to run the 1st file if compiled
  } else {
    hphp_invoke_simple(options.extension);
  }
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setDebugger(true);
  if (!restart) {
    DebuggerDummyEnv dde;
    Debugger::InterruptSessionStarted(options.fileName.c_str());
  }
  if (!options.fileName.empty()) {
    hphp_invoke_simple(options.fileName);
  }
  {
    DebuggerDummyEnv dde;
    Debugger::InterruptSessionEnded(options.fileName.c_str());
  }
}

void Debugger::LogShutdown(ShutdownKind shutdownKind) {
  int proxyCount = s_debugger.countConnectedProxy();
  if (proxyCount > 0) {
    Logger::Warning(DEBUGGER_LOG_TAG "%s with connected debuggers!",
                    shutdownKind == ShutdownKind::Normal ?
                      "Normal shutdown" : "Unexpected crash");

    for (const auto& proxyEntry: s_debugger.m_proxyMap) {
      auto sid = proxyEntry.first;
      auto proxy = proxyEntry.second;
      auto dummySid = makeStaticString(proxy->getDummyInfo().id());
      if (sid != dummySid) {
        auto sandbox = proxy->getSandbox();
        if (sandbox.valid()) {
          Logger::Warning(DEBUGGER_LOG_TAG "Debugging %s\n",
                          sandbox.desc().c_str());
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::InterruptSessionStarted(const char *file,
                                       const char *error /* = NULL */) {
  TRACE(2, "Debugger::InterruptSessionStarted\n");
  ThreadInfo::s_threadInfo->m_reqInjectionData.setDebugger(true);
  s_debugger.registerThread(); // Register this thread as being debugged
  Interrupt(SessionStarted, file, nullptr, error);
}

void Debugger::InterruptSessionEnded(const char *file) {
  TRACE(2, "Debugger::InterruptSessionEnded\n");
  Interrupt(SessionEnded, file);
}

void Debugger::InterruptWithUrl(int type, const char *url) {
  // Build a site to represent the URL. Note it won't have any source info
  // in it, because this event is raised with no PHP on the stack.
  InterruptSite site(false, null_variant);
  site.url() = url ? url : "";
  Interrupt(type, url, &site);
}

void Debugger::InterruptRequestStarted(const char *url) {
  TRACE(2, "Debugger::InterruptRequestStarted\n");
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    s_debugger.registerThread(); // Register this thread as being debugged
    InterruptWithUrl(RequestStarted, url);
  }
}

void Debugger::InterruptRequestEnded(const char *url) {
  TRACE(2, "Debugger::InterruptRequestEnded: url=%s\n", url);
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    InterruptWithUrl(RequestEnded, url);
  }
  const String& sandboxId = g_context->getSandboxId();
  s_debugger.unregisterSandbox(sandboxId.get());
}

void Debugger::InterruptPSPEnded(const char *url) {
  TRACE(2, "Debugger::InterruptPSPEnded\n");
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    InterruptWithUrl(PSPEnded, url);
  }
}

// Primary entrypoint for the debugger from the VM. Called in response to a host
// of VM events that the debugger is interested in. The debugger will execute
// any logic needed to handle the event, and will block below this to wait for
// and process more commands from the debugger client. This function will only
// return when the debugger is letting the thread continue execution, e.g., for
// flow control command like continue, next, etc.
void Debugger::Interrupt(int type, const char *program,
                         InterruptSite *site /* = NULL */,
                         const char *error /* = NULL */) {
  assert(RuntimeOption::EnableDebugger);
  TRACE_RB(2, "Debugger::Interrupt type %d\n", type);

  DebuggerProxyPtr proxy = GetProxy();
  if (proxy) {
    TRACE(3, "proxy != null\n");
    RequestInjectionData &rjdata = ThreadInfo::s_threadInfo->m_reqInjectionData;
    // The proxy will only service an interrupt if we've previously setup some
    // form of flow control command (steps, breakpoints, etc.) or if it's
    // an interrupt related to something like the session or request.
    if (proxy->needInterrupt() || type != BreakPointReached) {
      // Interrupts may execute some PHP code, causing another interruption.
      std::stack<void *> &interrupts = rjdata.interrupts;

      CmdInterrupt cmd((InterruptType)type, program, site, error);
      interrupts.push(&cmd);
      proxy->interrupt(cmd);
      interrupts.pop();
    }
    // Some cmds require us to interpret all instructions until the cmd
    // completes. Setting this will ensure we stay out of JIT code and in the
    // interpreter so phpDebuggerOpcodeHook has a chance to work.
    rjdata.setDebuggerIntr(proxy->needVMInterrupts());
  } else {
    TRACE(3, "proxy == null\n");
    // Debugger clients are disconnected abnormally, or this sandbox is not
    // being debugged.
    if (type == SessionStarted || type == SessionEnded) {
      // For command line programs, we need this exception to exit from
      // the infinite execution loop.
      throw DebuggerClientExitException();
    }
  }
}

// Primary entrypoint from the set of "debugger hooks", which the VM calls in
// response to various events. While this function is quite general wrt. the
// type of interrupt, practically the type will be one of the following:
//   - ExceptionThrown
//   - ExceptionHandler
//   - BreakPointReached
//   - HardBreakPoint
//
// Note: it is indeed a bit odd that interrupts due to single stepping come in
// as "BreakPointReached". Currently this results in spurious work in the
// debugger.
void Debugger::InterruptVMHook(int type /* = BreakPointReached */,
                               const Variant& e /* = null_variant */) {
  TRACE(2, "Debugger::InterruptVMHook\n");
  // Computing the interrupt site here pulls in more data from the Unit to
  // describe the current execution point.
  InterruptSite site(type == HardBreakPoint, e);
  if (!site.valid()) {
    // An invalid site is missing something like an ActRec, a func, or a
    // Unit. Currently the debugger has no action to take at such sites.
    return;
  }
  Interrupt(type, nullptr, &site);
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::SetTextColors() {
  TRACE(2, "Debugger::SetTextColors\n");
  s_stdout_color = ANSI_COLOR_CYAN;
  s_stderr_color = ANSI_COLOR_RED;
}

String Debugger::ColorStdout(const String& s) {
  TRACE(2, "Debugger::ColorStdout\n");
  if (s_stdout_color) {
    return String(s_stdout_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

String Debugger::ColorStderr(const String& s) {
  TRACE(2, "Debugger::ColorStderr\n");
  if (s_stderr_color) {
    return String(s_stderr_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

///////////////////////////////////////////////////////////////////////////////

// The flag for this is in the VM's normal ThreadInfo, but we don't
// have a way to get that given just a tid. Use our own map to find it
// and answer the question.
bool Debugger::isThreadDebugging(int64_t tid) {
  TRACE(2, "Debugger::isThreadDebugging tid=%" PRIx64 "\n", tid);
  ThreadInfoMap::const_accessor acc;
  if (m_threadInfos.find(acc, tid)) {
    ThreadInfo* ti = acc->second;
    auto isDebugging = ti->m_reqInjectionData.getDebugger();
    TRACE(2, "Is thread debugging? %d\n", isDebugging);
    return isDebugging;
  }
  TRACE(2, "Thread was never registered, assuming it is not debugging\n");
  return false;
}

// Remeber this thread's VM ThreadInfo so we can find it later via
// isThreadDebugging(). This is called when a thread interrupts for
// either session- or request-started, as these each signal the start
// of debugging for request and other threads.
void Debugger::registerThread() {
  TRACE(2, "Debugger::registerThread\n");
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  int64_t tid = (int64_t)Process::GetThreadId();
  ThreadInfoMap::accessor acc;
  m_threadInfos.insert(acc, tid);
  acc->second = ti;
}

void Debugger::addOrUpdateSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::addOrUpdateSandbox\n");
  const StringData* sd = makeStaticString(sandbox.id());
  SandboxMap::accessor acc;
  if (m_sandboxMap.insert(acc, sd)) {
    DSandboxInfoPtr sb(new DSandboxInfo());
    *sb = sandbox;
    acc->second = sb;
  } else {
    acc->second->update(sandbox);
  }
}

void Debugger::getSandboxes(std::vector<DSandboxInfoPtr> &sandboxes) {
  TRACE(2, "Debugger::getSandboxes\n");
  sandboxes.reserve(m_sandboxMap.size());
  for (SandboxMap::const_iterator iter =
       m_sandboxMap.begin(); iter != m_sandboxMap.end(); ++iter) {
    if (!iter->second->m_user.empty()) {
      DSandboxInfoPtr sandbox(new DSandboxInfo());
      *sandbox = *iter->second;
      sandboxes.push_back(sandbox);
    }
  }
}

// Notify the debugger that this thread is executing a request in the given
// sandbox. This ensures that the debugger knows about the sandbox, and adds
// the thread to the set of threads currently active in the sandbox.
void Debugger::registerSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::registerSandbox\n");
  // update sandbox first
  addOrUpdateSandbox(sandbox);

  // add thread to m_sandboxThreadInfoMap
  const StringData* sid = makeStaticString(sandbox.id());
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  {
    SandboxThreadInfoMap::accessor acc;
    m_sandboxThreadInfoMap.insert(acc, sid);
    ThreadInfoSet& set = acc->second;
    set.insert(ti);
  }

  // find out whether this sandbox is being debugged
  DebuggerProxyPtr proxy = findProxy(sid);
  if (proxy) {
    ti->m_reqInjectionData.setDebugger(true);
  }
}

void Debugger::unregisterSandbox(const StringData* sandboxId) {
  TRACE(2, "Debugger::unregisterSandbox\n");
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  SandboxThreadInfoMap::accessor acc;
  if (m_sandboxThreadInfoMap.find(acc, sandboxId)) {
    ThreadInfoSet& set = acc->second;
    set.erase(ti);
  }
}

#define FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)  {                       \
  SandboxThreadInfoMap::const_accessor acc;                            \
  if (m_sandboxThreadInfoMap.find(acc, sid)) {                         \
    const ThreadInfoSet& set = acc->second;                            \
    for (std::set<ThreadInfo*>::iterator iter = set.begin();           \
         iter != set.end(); ++iter) {                                  \
      ThreadInfo* ti = (*iter);                                        \
      assert(ThreadInfo::valid(ti));                                   \

#define FOREACH_SANDBOX_THREAD_END()    } } }                          \

// Ask every thread in this proxy's sandbox and the dummy sandbox to
// "stop".  Gaining control of these threads is the intention... the
// mechanism is to force them all to start interpreting all of their
// code in an effort to gain control in phpDebuggerOpcodeHook(). We
// set the "debugger interrupt" flag to ensure we interpret code
// rather than entering translated code, and we set the "debugger
// signal" surprise flag to pop out of loops in translated code.
void Debugger::requestInterrupt(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::requestInterrupt\n");
  const StringData* sid = makeStaticString(proxy->getSandboxId());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
    ti->m_reqInjectionData.setDebuggerIntr(true);
    ti->m_reqInjectionData.setDebuggerSignalFlag();
  FOREACH_SANDBOX_THREAD_END()

  sid = makeStaticString(proxy->getDummyInfo().id());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
    ti->m_reqInjectionData.setDebuggerIntr(true);
    ti->m_reqInjectionData.setDebuggerSignalFlag();
  FOREACH_SANDBOX_THREAD_END()
}

void Debugger::setDebuggerFlag(const StringData* sandboxId, bool flag) {
  TRACE(2, "Debugger::setDebuggerFlag\n");
  FOREACH_SANDBOX_THREAD_BEGIN(sandboxId, ti)
    ti->m_reqInjectionData.setDebugger(flag);
  FOREACH_SANDBOX_THREAD_END()
}

#undef FOREACH_SANDBOX_THREAD_BEGIN
#undef FOREACH_SANDBOX_THREAD_END

DebuggerProxyPtr Debugger::createProxy(SmartPtr<Socket> socket, bool local) {
  TRACE(2, "Debugger::createProxy\n");
  // Creates a proxy and threads needed to handle it. At this point, there is
  // not enough information to attach a sandbox.
  DebuggerProxyPtr proxy(new DebuggerProxy(socket, local));
  {
    // Place this new proxy into the proxy map keyed on the dummy sandbox id.
    // This keeps the proxy alive in the server case, which drops the result of
    // this function on the floor. It also makes the proxy findable when we a
    // dummy sandbox thread needs to interrupt.
    const StringData* sid =
      makeStaticString(proxy->getDummyInfo().id());
    assert(sid);
    ProxyMap::accessor acc;
    m_proxyMap.insert(acc, sid);
    acc->second = proxy;
  }
  if (!local) {
    proxy->startDummySandbox();
  }
  proxy->startSignalThread();
  return proxy;
}

// Place the proxy onto the retired proxy queue. It will be cleaned up
// and destroyed later by another thread.
void Debugger::retireProxy(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::retireProxy %p\n", proxy.get());
  m_retiredProxyQueue.push(proxy);
}

// Cleanup any proxies in our retired proxy queue. We'll pull a proxy
// out of the queue, ask it to cleanup which will wait for threads it
// owns to exit, then drop our shared reference to it. That may
// destroy the proxy here, or it may remain alive if there is a thread
// still processing an interrupt with it since such threads have their
// own shared reference.
void Debugger::cleanupRetiredProxies() {
  TRACE(7, "Debugger::cleanupRetiredProxies\n");
  DebuggerProxyPtr proxy;
  while (m_retiredProxyQueue.try_pop(proxy)) {
    try {
      // We give the proxy a short period of time to wait for any
      // threads it owns. If it doesn't succeed, we put it back and
      // try again later.
      TRACE(2, "Cleanup proxy %p\n", proxy.get());
      if (!proxy->cleanup(1)) {
        TRACE(2, "Proxy %p has not stopped yet\n", proxy.get());
        m_retiredProxyQueue.push(proxy);
      }
    } catch (Exception &e) {
      Logger::Error("Exception during proxy %p retirement: %s",
                    proxy.get(), e.getMessage().c_str());
    }
  }
}

// NB: when this returns, the Debugger class no longer has any references to the
// given proxy. It will likely be destroyed when the caller's reference goes out
// of scope.
void Debugger::removeProxy(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::removeProxy\n");
  if (proxy->getSandbox().valid()) {
    const StringData* sid = makeStaticString(proxy->getSandboxId());
    setDebuggerFlag(sid, false);
    m_proxyMap.erase(sid);
  }
  const StringData* dummySid =
    makeStaticString(proxy->getDummyInfo().id());
  m_proxyMap.erase(dummySid);
  // Clear the debugger blacklist PC upon last detach if JIT is used
  if (RuntimeOption::EvalJit && countConnectedProxy() == 0) {
    JIT::tx->clearDbgBL();
  }
}

DebuggerProxyPtr Debugger::findProxy(const StringData* sandboxId) {
  TRACE(7, "Debugger::findProxy\n");
  if (sandboxId) {
    ProxyMap::const_accessor acc;
    if (m_proxyMap.find(acc, sandboxId)) {
      return acc->second;
    }
  }
  return DebuggerProxyPtr();
}

bool Debugger::switchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
  TRACE(2, "Debugger::switchSandbox\n");
  const StringData* newSid = makeStaticString(newId);
  // Lock the proxy during the switch
  Lock l(proxy.get());
  if (proxy->getSandboxId() != newId) {
    if (!switchSandboxImpl(proxy, newSid, force)) {
      // failed to switch
      return false;
    }
  }

  updateProxySandbox(proxy, newSid);
  return true;
}

bool Debugger::switchSandboxImpl(DebuggerProxyPtr proxy,
                                 const StringData* newSid,
                                 bool force) {
  TRACE(2, "Debugger::switchSandboxImpl\n");
  // Take the new sandbox
  DebuggerProxyPtr otherProxy;
  {
    ProxyMap::accessor acc;
    if (m_proxyMap.insert(acc, newSid)) {
      acc->second = proxy;
    } else {
      // the sandbox indicated by newId is already attached by another proxy
      if (!force) {
        return false;
      }
      // Delay the destruction of the proxy originally attached to the sandbox
      // to avoid calling a DebuggerProxy destructor (which can sleep) while
      // holding m_mutex.
      otherProxy = acc->second;
      acc->second = proxy;
    }
  }

  if (proxy->getSandbox().valid()) {
    // Detach from the old sandbox
    const StringData* oldSid =
      makeStaticString(proxy->getSandboxId());
    setDebuggerFlag(oldSid, false);
    m_proxyMap.erase(oldSid);
  }
  setDebuggerFlag(newSid, true);

  if (otherProxy) {
    otherProxy->stop();
  }

  return true;
}

void Debugger::updateProxySandbox(DebuggerProxyPtr proxy,
                                  const StringData* sandboxId) {
  TRACE(2, "Debugger::updateProxySandbox\n");
  // update proxy's sandbox info from what is on file
  SandboxMap::const_accessor acc;
  if (m_sandboxMap.find(acc, sandboxId)) {
    proxy->updateSandbox(acc->second);
  } else {
    // don't have the sandbox on file yet. create a sandbox info with
    // no path for now. Sandbox path will be updated upon first request
    // with that sandbox arrives
    DSandboxInfoPtr sb(new DSandboxInfo(sandboxId->toCppString()));
    proxy->updateSandbox(sb);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Helpers for usage logging

// NB: the usage logger is not owned by the Debugger. The caller will call this
// again with nullptr before destroying the given usage logger.
void Debugger::SetUsageLogger(DebuggerUsageLogger *usageLogger) {
  TRACE(1, "Debugger::SetUsageLogger\n");
  s_debugger.m_usageLogger = usageLogger;
}

void Debugger::InitUsageLogging() {
  TRACE(1, "Debugger::InitUsageLogging\n");
  if (s_debugger.m_usageLogger) s_debugger.m_usageLogger->init();
}

void Debugger::UsageLog(const std::string &mode, const std::string &sandboxId,
                        const std::string &cmd, const std::string &data) {
  if (s_debugger.m_usageLogger) s_debugger.m_usageLogger->log(mode, sandboxId,
                                                              cmd, data);
}

const char *Debugger::InterruptTypeName(CmdInterrupt &cmd) {
  switch (cmd.getInterruptType()) {
    case SessionStarted: return "SessionStarted";
    case SessionEnded: return "SessionEnded";
    case RequestStarted: return "RequestStarted";
    case RequestEnded: return "RequestEnded";
    case PSPEnded: return "PSPEnded";
    case HardBreakPoint: return "HardBreakPoint";
    case BreakPointReached: return "BreakPointReached";
    case ExceptionThrown: return "ExceptionThrown";
    case ExceptionHandler: return "ExceptionHandler";
    default:
      return "unknown";
  }
}

void Debugger::UsageLogInterrupt(const std::string &mode,
                                 const std::string &sandboxId,
                                 CmdInterrupt &cmd) {
  UsageLog(mode, sandboxId, "interrupt", InterruptTypeName(cmd));
}

///////////////////////////////////////////////////////////////////////////////

DebuggerDummyEnv::DebuggerDummyEnv() {
  TRACE(2, "DebuggerDummyEnv::DebuggerDummyEnv\n");
  g_context->enterDebuggerDummyEnv();
}

DebuggerDummyEnv::~DebuggerDummyEnv() {
  TRACE(2, "DebuggerDummyEnv::~DebuggerDummyEnv\n");
  g_context->exitDebuggerDummyEnv();
}

///////////////////////////////////////////////////////////////////////////////

EvalBreakControl::EvalBreakControl(bool noBreak) {
  TRACE(2, "EvalBreakControl::EvalBreakControl\n");
  m_noBreakSave = g_context->m_dbgNoBreak;
  g_context->m_dbgNoBreak = noBreak;
}

EvalBreakControl::~EvalBreakControl() {
  TRACE(2, "EvalBreakControl::~EvalBreakControl\n");
  g_context->m_dbgNoBreak = m_noBreakSave;
}

///////////////////////////////////////////////////////////////////////////////
}}
