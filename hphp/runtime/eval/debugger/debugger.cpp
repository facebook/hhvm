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

#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/debugger_server.h>
#include <runtime/eval/debugger/debugger_client.h>
#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/base/hphp_system.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>
#include <util/text_color.h>
#include <util/util.h>
#include <util/logger.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static const Trace::Module TRACEMOD = Trace::debugger;

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
  if (s_clientStarted) {
    DebuggerClient::Stop();
  }
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::RegisterSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::RegisterSandbox\n");
  s_debugger.registerSandbox(sandbox);
}

void Debugger::UnregisterSandbox(CStrRef id) {
  TRACE(2, "Debugger::UnregisterSandbox\n");
  s_debugger.unregisterSandbox(id.get());
}

void Debugger::RegisterThread() {
  TRACE(2, "Debugger::RegisterThread\n");
  s_debugger.registerThread();
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
  TRACE(2, "Debugger::CountConnectedProxy\n");
  return s_debugger.countConnectedProxy();
}

DebuggerProxyPtr Debugger::GetProxy() {
  TRACE(2, "Debugger::GetProxy\n");
  CStrRef sandboxId = g_context->getSandboxId();
  return s_debugger.findProxy(sandboxId.get());
}

bool Debugger::SwitchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
  TRACE(2, "Debugger::SwitchSandbox\n");
  return s_debugger.switchSandbox(proxy, newId, force);
}

void Debugger::GetRegisteredSandboxes(DSandboxInfoPtrVec &sandboxes) {
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

void Debugger::RetireDummySandboxThread(DummySandbox* toRetire) {
  TRACE(2, "Debugger::RetireDummySandboxThread\n");
  s_debugger.retireDummySandboxThread(toRetire);
}

void Debugger::CleanupDummySandboxThreads() {
  TRACE(7, "Debugger::CleanupDummySandboxThreads\n");
  s_debugger.cleanupDummySandboxThreads();
}

void Debugger::DebuggerSession(const DebuggerClientOptions& options,
                               const std::string& file, bool restart) {
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
    Debugger::InterruptSessionStarted(file.c_str());
  }
  hphp_invoke_simple(file);
  {
    DebuggerDummyEnv dde;
    Debugger::InterruptSessionEnded(file.c_str());
  }
}

void Debugger::LogShutdown(ShutdownKind shutdownKind) {
  int proxyCount = s_debugger.countConnectedProxy();
  if (proxyCount > 0) {
    Logger::Warning(DEBUGGER_LOG_TAG "%s with connected debuggers!",
                    shutdownKind == ShutdownKind::Normal ?
                      "Normal shutdown" : "Unexpected crash",
                    proxyCount);

    for (const auto& proxyEntry: s_debugger.m_proxyMap) {
      auto sid = proxyEntry.first;
      auto proxy = proxyEntry.second;
      auto dummySid = StringData::GetStaticString(proxy->getDummyInfo().id());
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
  Interrupt(SessionStarted, file, nullptr, error);
}

void Debugger::InterruptSessionEnded(const char *file) {
  TRACE(2, "Debugger::InterruptSessionEnded\n");
  Interrupt(SessionEnded, file);
}

void Debugger::InterruptRequestStarted(const char *url) {
  TRACE(2, "Debugger::InterruptRequestStarted\n");
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    Interrupt(RequestStarted, url);
  }
}

void Debugger::InterruptRequestEnded(const char *url) {
  TRACE(2, "Debugger::InterruptRequestEnded: url=%s\n", url);
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    Interrupt(RequestEnded, url);
  }
  CStrRef sandboxId = g_context->getSandboxId();
  s_debugger.unregisterSandbox(sandboxId.get());
}

void Debugger::InterruptPSPEnded(const char *url) {
  TRACE(2, "Debugger::InterruptPSPEnded\n");
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebugger()) {
    Interrupt(PSPEnded, url);
  }
}

// Called directly from exception handling to indicate a user error handler
// failed to handle an exeption. NB: this is quite distinct from the hook called
// from iopThrow named phpDebuggerExceptionHook().
bool Debugger::InterruptException(CVarRef e) {
  TRACE(2, "Debugger::InterruptException\n");
  if (RuntimeOption::EnableDebugger) {
    ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
    if (ti->m_reqInjectionData.getDebugger()) {
      HPHP::VM::Transl::VMRegAnchor _;
      InterruptVMHook(ExceptionThrown, e);
    }
  }
  return true;
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
  TRACE(2, "Debugger::Interrupt\n");

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
      // for command line programs, we need this exception to exit from
      // the infinite execution loop
      throw DebuggerClientExitException();
    }
  }
}

// Primary entrypoint from the set of "debugger hooks", which the VM calls in
// response to various events. While this function is quite general wrt. the
// type of interrupt, practically the type will be one of the following:
//   - ExceptionThrown
//   - BreakPointReached
//   - HardBreakPoint
//
// Note: it is indeed a bit odd that interrupts due to single stepping come in
// as "BreakPointReached". Currently this results in spurious work in the
// debugger.
void Debugger::InterruptVMHook(int type /* = BreakPointReached */,
                               CVarRef e /* = null_variant */) {
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

String Debugger::ColorStdout(CStrRef s) {
  TRACE(2, "Debugger::ColorStdout\n");
  if (s_stdout_color) {
    return String(s_stdout_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

String Debugger::ColorStderr(CStrRef s) {
  TRACE(2, "Debugger::ColorStderr\n");
  if (s_stderr_color) {
    return String(s_stderr_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

///////////////////////////////////////////////////////////////////////////////

bool Debugger::isThreadDebugging(int64_t tid) {
  TRACE(2, "Debugger::isThreadDebugging\n");
  ThreadInfoMap::const_accessor acc;
  if (m_threadInfos.find(acc, tid)) {
    ThreadInfo* ti = acc->second;
    return (ti->m_reqInjectionData.getDebugger());
  }
  return false;
}

void Debugger::registerThread() {
  TRACE(2, "Debugger::registerThread\n");
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  int64_t tid = (int64_t)Process::GetThreadId();
  ThreadInfoMap::accessor acc;
  m_threadInfos.insert(acc, tid);
  acc->second = ti;
}

void Debugger::updateSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::updateSandbox\n");
  const StringData* sd = StringData::GetStaticString(sandbox.id());
  SandboxMap::accessor acc;
  if (m_sandboxMap.insert(acc, sd)) {
    DSandboxInfoPtr sb(new DSandboxInfo());
    *sb = sandbox;
    acc->second = sb;
  } else {
    acc->second->update(sandbox);
  }
}

void Debugger::getSandboxes(DSandboxInfoPtrVec &sandboxes) {
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

void Debugger::registerSandbox(const DSandboxInfo &sandbox) {
  TRACE(2, "Debugger::registerSandbox\n");
  // update sandbox first
  updateSandbox(sandbox);

  // add thread do m_sandboxThreadInfoMap
  const StringData* sid = StringData::GetStaticString(sandbox.id());
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
    proxy->writeInjTablesToThread();
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

// Ask every thread in this proxy's sandbox and the dummy sandbox to "stop".
// Gaining control of these threads is the intention... the mechanism is to
// force them all to start interpreting all of their code in an effort to gain
// control in phpDebuggerOpcodeHook().
void Debugger::requestInterrupt(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::requestInterrupt\n");
  const StringData* sid = StringData::GetStaticString(proxy->getSandboxId());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
    ti->m_reqInjectionData.setDebuggerIntr(true);
  FOREACH_SANDBOX_THREAD_END()

  sid = StringData::GetStaticString(proxy->getDummyInfo().id());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
    ti->m_reqInjectionData.setDebuggerIntr(true);
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
  const StringData* sid =
    StringData::GetStaticString(proxy->getDummyInfo().id());
  {
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

void Debugger::retireDummySandboxThread(DummySandbox* toRetire) {
  TRACE(2, "Debugger::retireDummySandboxThread\n");
  m_cleanupDummySandboxQ.push(toRetire);
}

void Debugger::cleanupDummySandboxThreads() {
  TRACE(7, "Debugger::cleanupDummySandboxThreads\n");
  DummySandbox* ptr = nullptr;
  while (m_cleanupDummySandboxQ.try_pop(ptr)) {
    ptr->notify();
    try {
      // we can't block the server for waiting
      if (ptr->waitForEnd(1)) {
        delete ptr;
      } else {
        Logger::Error("Dummy sandbox %p refused to stop", ptr);
      }
    } catch (Exception &e) {
      Logger::Error("Dummy sandbox exception: " + e.getMessage());
    }
  }
}

void Debugger::removeProxy(DebuggerProxyPtr proxy) {
  TRACE(2, "Debugger::removeProxy\n");
  if (proxy->getSandbox().valid()) {
    const StringData* sid = StringData::GetStaticString(proxy->getSandboxId());
    setDebuggerFlag(sid, false);
    m_proxyMap.erase(sid);
  }
  const StringData* dummySid =
    StringData::GetStaticString(proxy->getDummyInfo().id());
  m_proxyMap.erase(dummySid);
  // Clear the debugger blacklist PC upon last detach if JIT is used
  if (RuntimeOption::EvalJit && countConnectedProxy() == 0) {
    VM::Transl::Translator::Get()->clearDbgBL();
  }
}

DebuggerProxyPtr Debugger::findProxy(const StringData* sandboxId) {
  TRACE(2, "Debugger::findProxy\n");
  ProxyMap::const_accessor acc;
  if (m_proxyMap.find(acc, sandboxId)) {
    return acc->second;
  }
  return DebuggerProxyPtr();
}

bool Debugger::switchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
  TRACE(2, "Debugger::switchSandbox\n");
  const StringData* newSid = StringData::GetStaticString(newId);
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
  TRACE(2, "Debugger::switchSandboxImpln");
  // Take the new sandbox
  DebuggerProxyPtr dpp;
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
      dpp = acc->second;
      acc->second = proxy;
    }
  }

  if (proxy->getSandbox().valid()) {
    // Detach from the old sandbox
    const StringData* oldSid =
      StringData::GetStaticString(proxy->getSandboxId());
    setDebuggerFlag(oldSid, false);
    m_proxyMap.erase(oldSid);
  }
  setDebuggerFlag(newSid, true);

  if (dpp) {
    dpp->forceQuit();
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
    DSandboxInfoPtr sb(new DSandboxInfo(sandboxId->toCPPString()));
    proxy->updateSandbox(sb);
  }
}

///////////////////////////////////////////////////////////////////////////////

DebuggerDummyEnv::DebuggerDummyEnv() {
  TRACE(2, "DebuggerDummyEnv::DebuggerDummyEnv\n");
  g_vmContext->enterDebuggerDummyEnv();
}

DebuggerDummyEnv::~DebuggerDummyEnv() {
  TRACE(2, "DebuggerDummyEnv::~DebuggerDummyEnv\n");
  g_vmContext->exitDebuggerDummyEnv();
}

///////////////////////////////////////////////////////////////////////////////

EvalBreakControl::EvalBreakControl(bool noBreak) {
  TRACE(2, "EvalBreakControl::EvalBreakControl\n");
  m_noBreakSave = g_vmContext->m_dbgNoBreak;
  g_vmContext->m_dbgNoBreak = noBreak;
}

EvalBreakControl::~EvalBreakControl() {
  TRACE(2, "EvalBreakControl::~EvalBreakControl\n");
  g_vmContext->m_dbgNoBreak = m_noBreakSave;
}

///////////////////////////////////////////////////////////////////////////////
}}
