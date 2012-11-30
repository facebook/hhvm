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

Debugger Debugger::s_debugger;
bool Debugger::s_clientStarted = false;

bool Debugger::StartServer() {
  return DebuggerServer::Start();
}

DebuggerProxyPtr Debugger::StartClient(const DebuggerClientOptions &options) {
  SmartPtr<Socket> localProxy = DebuggerClient::Start(options);
  if (localProxy.get()) {
    s_clientStarted = true;
    return CreateProxy(localProxy, true);
  }
  return DebuggerProxyPtr();
}

void Debugger::Stop() {
  s_debugger.m_proxyMap.clear();
  DebuggerServer::Stop();
  if (s_clientStarted) {
    DebuggerClient::Stop();
  }
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::RegisterSandbox(const DSandboxInfo &sandbox) {
  s_debugger.registerSandbox(sandbox);
}

void Debugger::UnregisterSandbox(CStrRef id) {
  s_debugger.unregisterSandbox(id.get());
}

void Debugger::RegisterThread() {
  s_debugger.registerThread();
}

DebuggerProxyPtr Debugger::CreateProxy(SmartPtr<Socket> socket, bool local) {
  return s_debugger.createProxy(socket, local);
}

void Debugger::RemoveProxy(DebuggerProxyPtr proxy) {
  s_debugger.removeProxy(proxy);
}

int Debugger::CountConnectedProxy() {
  return s_debugger.countConnectedProxy();
}

DebuggerProxyPtr Debugger::GetProxy() {
  CStrRef sandboxId = g_context->getSandboxId();
  return s_debugger.findProxy(sandboxId.get());
}

bool Debugger::SwitchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
  return s_debugger.switchSandbox(proxy, newId, force);
}

void Debugger::GetRegisteredSandboxes(DSandboxInfoPtrVec &sandboxes) {
  s_debugger.getSandboxes(sandboxes);
}

bool Debugger::IsThreadDebugging(int64 id) {
  return s_debugger.isThreadDebugging(id);
}

void Debugger::RequestInterrupt(DebuggerProxyPtr proxy) {
  s_debugger.requestInterrupt(proxy);
}

void Debugger::RetireDummySandboxThread(DummySandbox* toRetire) {
  s_debugger.retireDummySandboxThread(toRetire);
}

void Debugger::CleanupDummySandboxThreads() {
  s_debugger.cleanupDummySandboxThreads();
}


void Debugger::DebuggerSession(const DebuggerClientOptions& options,
                               const std::string& file, bool restart) {
  if (options.extension.empty()) {
    // even if it's empty, still need to call for warmup
    hphp_invoke_simple("", true); // not to run the 1st file if compiled
  } else {
    hphp_invoke_simple(options.extension);
  }
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.debugger = true;
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

///////////////////////////////////////////////////////////////////////////////

void Debugger::InterruptSessionStarted(const char *file,
                                       const char *error /* = NULL */) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.debugger = true;
  Interrupt(SessionStarted, file, NULL, error);
}

void Debugger::InterruptSessionEnded(const char *file) {
  Interrupt(SessionEnded, file);
}

void Debugger::InterruptRequestStarted(const char *url) {
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.debugger) {
    Interrupt(RequestStarted, url);
  }
}

void Debugger::InterruptRequestEnded(const char *url) {
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.debugger) {
    Interrupt(RequestEnded, url);
  }
  CStrRef sandboxId = g_context->getSandboxId();
  s_debugger.unregisterSandbox(sandboxId.get());
}

void Debugger::InterruptPSPEnded(const char *url) {
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.debugger) {
    Interrupt(PSPEnded, url);
  }
}

void Debugger::InterruptFileLine(InterruptSite &site) {
  Interrupt(BreakPointReached, NULL, &site);
}

void Debugger::InterruptHard(InterruptSite &site) {
  Interrupt(HardBreakPoint, NULL, &site);
}

bool Debugger::InterruptException(CVarRef e) {
  if (RuntimeOption::EnableDebugger) {
    ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
    if (ti->m_reqInjectionData.debugger) {
      if (hhvm) {
        HPHP::VM::Transl::VMRegAnchor _;
        InterruptVMHook(ExceptionThrown, e);
      } else {
        if (ti->m_top) {
          Eval::InterruptSiteFI site(ti->m_top, e);
          Eval::Debugger::Interrupt(ExceptionThrown, NULL, &site);
          if (site.isJumping()) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void Debugger::Interrupt(int type, const char *program,
                         InterruptSite *site /* = NULL */,
                         const char *error /* = NULL */) {
  ASSERT(RuntimeOption::EnableDebugger);

  RequestInjectionData &rjdata = ThreadInfo::s_threadInfo->m_reqInjectionData;
  if (rjdata.debuggerIdle > 0 && type == BreakPointReached) {
    --rjdata.debuggerIdle;
    return;
  }

  DebuggerProxyPtr proxy = GetProxy();
  if (proxy) {
    if (proxy->needInterrupt() || type != BreakPointReached) {
      // Interrupts may execute some PHP code, causing another interruption.
      std::stack<void *> &interrupts = rjdata.interrupts;

      CmdInterrupt cmd((InterruptType)type, program, site, error);
      interrupts.push(&cmd);
      proxy->interrupt(cmd);
      interrupts.pop();
    }
    rjdata.debuggerIntr = proxy->needInterruptForNonBreak();
    if (!hhvm) {
      rjdata.debuggerIdle = proxy->needInterrupt() ? 0 : 1000;
    }
  } else {
    // debugger clients are disconnected abnormally
    if (type == SessionStarted || type == SessionEnded) {
      // for command line programs, we need this exception to exit from
      // the infinite execution loop
      throw DebuggerClientExitException();
    }
  }
}

void Debugger::InterruptVMHook(int type /* = BreakPointReached */,
                               CVarRef e /* = null_variant */) {
  const_assert(hhvm);
  InterruptSiteVM site(type == HardBreakPoint, e);
  if (!site.valid()) {
    return;
  }
  Interrupt(type, NULL, &site);
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::SetTextColors() {
  Util::s_stdout_color = ANSI_COLOR_CYAN;
  Util::s_stderr_color = ANSI_COLOR_RED;
}

String Debugger::ColorStdout(CStrRef s) {
  if (Util::s_stdout_color) {
    return String(Util::s_stdout_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

String Debugger::ColorStderr(CStrRef s) {
  if (Util::s_stderr_color) {
    return String(Util::s_stderr_color) + s + String(ANSI_COLOR_END);
  }
  return s;
}

///////////////////////////////////////////////////////////////////////////////

bool Debugger::isThreadDebugging(int64 tid) {
  ThreadInfoMap::const_accessor acc;
  if (m_threadInfos.find(acc, tid)) {
    ThreadInfo* ti = acc->second;
    return (ti->m_reqInjectionData.debugger);
  }
  return false;
}

void Debugger::registerThread() {
  ThreadInfo* ti = ThreadInfo::s_threadInfo.getNoCheck();
  int64 tid = (int64)Process::GetThreadId();
  ThreadInfoMap::accessor acc;
  m_threadInfos.insert(acc, tid);
  acc->second = ti;
}

void Debugger::updateSandbox(const DSandboxInfo &sandbox) {
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
    ti->m_reqInjectionData.debugger = true;
    if (hhvm) {
      DebuggerProxyVM* proxyVM = (DebuggerProxyVM*)proxy.get();
      proxyVM->writeInjTablesToThread();
    }
  }
}

void Debugger::unregisterSandbox(const StringData* sandboxId) {
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
      ASSERT(ThreadInfo::valid(ti));                                   \

#define FOREACH_SANDBOX_THREAD_END()    } } }                          \

void Debugger::requestInterrupt(DebuggerProxyPtr proxy) {
  const StringData* sid = StringData::GetStaticString(proxy->getSandboxId());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
  ti->m_reqInjectionData.debuggerIntr = true;
  FOREACH_SANDBOX_THREAD_END()
  sid = StringData::GetStaticString(proxy->getDummyInfo().id());
  FOREACH_SANDBOX_THREAD_BEGIN(sid, ti)
  ti->m_reqInjectionData.debuggerIntr = true;
  FOREACH_SANDBOX_THREAD_END()
}

void Debugger::setDebuggerFlag(const StringData* sandboxId, bool flag) {
  FOREACH_SANDBOX_THREAD_BEGIN(sandboxId, ti)
  ti->m_reqInjectionData.debugger = flag;
  FOREACH_SANDBOX_THREAD_END()
}

#undef FOREACH_SANDBOX_THREAD_BEGIN
#undef FOREACH_SANDBOX_THREAD_END

DebuggerProxyPtr Debugger::createProxy(SmartPtr<Socket> socket, bool local) {
  // Creates a proxy and threads needed to handle it. At this point, there is
  // not enough information to attach a sandbox.
  DebuggerProxyPtr proxy(hhvm
                         ? (DebuggerProxy*) new DebuggerProxyVM(socket, local)
                         : new DebuggerProxy(socket, local));
  const StringData* sid =
    StringData::GetStaticString(proxy->getDummyInfo().id());
  {
    ASSERT(sid);
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
  m_cleanupDummySandboxQ.push(toRetire);
}

void Debugger::cleanupDummySandboxThreads() {
  DummySandbox* ptr = NULL;
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
  ProxyMap::const_accessor acc;
  if (m_proxyMap.find(acc, sandboxId)) {
    return acc->second;
  }
  return DebuggerProxyPtr();
}

bool Debugger::switchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId,
                             bool force) {
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

DebuggerDummyEnv::DebuggerDummyEnv() {
  if (hhvm) {
    g_vmContext->enterDebuggerDummyEnv();
  }
}

DebuggerDummyEnv::~DebuggerDummyEnv() {
  if (hhvm) {
    g_vmContext->exitDebuggerDummyEnv();
  }
}

EvalBreakControl::EvalBreakControl(bool noBreak) {
  m_noBreakSave = g_vmContext->m_dbgNoBreak;
  g_vmContext->m_dbgNoBreak = noBreak;
}

EvalBreakControl::~EvalBreakControl() {
  g_vmContext->m_dbgNoBreak = m_noBreakSave;
}

///////////////////////////////////////////////////////////////////////////////
}}
