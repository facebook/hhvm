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
#include <system/gen/sys/system_globals.h>
#include <util/text_color.h>
#include <util/util.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

Debugger Debugger::s_debugger;

bool Debugger::StartServer() {
  return DebuggerServer::Start();
}

void Debugger::StartClient(const DebuggerClientOptions &options) {
  SmartPtr<Socket> localProxy = DebuggerClient::Start(options);
  if (localProxy.get()) {
    RegisterProxy(localProxy, true);
  }
}

void Debugger::Stop() {
  DebuggerServer::Stop();
  DebuggerClient::Stop();
  s_debugger.stop();
}

///////////////////////////////////////////////////////////////////////////////

void Debugger::RegisterSandbox(const DSandboxInfo &sandbox) {
  s_debugger.addSandbox(sandbox);
}

void Debugger::GetRegisteredSandboxes(DSandboxInfoPtrVec &sandboxes) {
  s_debugger.getSandboxes(sandboxes);
}

bool Debugger::IsThreadDebugging(int64 id) {
  return s_debugger.isThreadDebugging(id);
}

void Debugger::RegisterProxy(SmartPtr<Socket> socket, bool local) {
  s_debugger.addProxy(socket, local);
}

void Debugger::RemoveProxy(DebuggerProxyPtr proxy) {
  s_debugger.removeProxy(proxy);
}

void Debugger::SwitchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId) {
  s_debugger.switchSandbox(proxy, newId);
}

///////////////////////////////////////////////////////////////////////////////

DebuggerProxyPtr Debugger::GetProxy() {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  String id = g->GV(_SERVER)["HPHP_SANDBOX_ID"];
  return s_debugger.findProxy(id.data());
}

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
    if (ti->m_top && ti->m_reqInjectionData.debugger) {
      Eval::InterruptSite site(ti->m_top, e);
      Eval::Debugger::Interrupt(ExceptionThrown, NULL, &site);
      if (site.isJumping()) {
        return false;
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
    rjdata.debuggerIdle = proxy->needInterrupt() ? 0 : 1000;
  } else {
    // debugger clients are disconnected abnormally
    if (type == SessionStarted || type == SessionEnded) {
      // for command line programs, we need this exception to exit from
      // the infinite execution loop
      throw DebuggerClientExitException();
    }
  }
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

void Debugger::stop() {
  WriteLock lock(m_mutex);
  m_proxies.clear();
  m_sandboxThreads.clear();
}

bool Debugger::isThreadDebugging(int64 id) {
  ReadLock lock(m_mutex);
  if (id) {
    ThreadInfo *ti = m_threadInfos[id];
    ASSERT(ti);
    if (ti) {
      return ti->m_reqInjectionData.debugger;
    }
  }
  return false;
}

void Debugger::addSandbox(const DSandboxInfo &sandbox) {
  WriteLock lock(m_mutex);
  string id = sandbox.id();
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  m_threadInfos[(int64)pthread_self()] = ti;
  if (m_proxies[id]) {
    ti->m_reqInjectionData.debugger = true;
    m_sandboxThreads[id].erase(ti);
  } else {
    m_sandboxThreads[id].insert(ti);
  }
  DSandboxInfoPtr old = m_sandboxes[id];
  if (old) {
    old->update(sandbox);
  } else {
    DSandboxInfoPtr sb(new DSandboxInfo());
    *sb = sandbox;
    m_sandboxes[id] = sb;
  }
}

void Debugger::addProxy(SmartPtr<Socket> socket, bool local) {
  DebuggerProxyPtr proxy(new DebuggerProxy(socket, local));
  switchSandbox(proxy, proxy->getSandboxId());
  if (!local) {
    proxy->startDummySandbox();
  }
  proxy->startSignalThread();
}

void Debugger::removeProxy(DebuggerProxyPtr proxy) {
  WriteLock lock(m_mutex);
  m_proxies[proxy->getSandboxId()].reset();
}

void Debugger::getSandboxes(DSandboxInfoPtrVec &sandboxes) {
  ReadLock lock(m_mutex);
  sandboxes.reserve(m_sandboxes.size());
  for (StringToDSandboxInfoPtrMap::const_iterator iter =
         m_sandboxes.begin(); iter != m_sandboxes.end(); ++iter) {
    if (!iter->second->m_user.empty()) {
      DSandboxInfoPtr sandbox(new DSandboxInfo());
      *sandbox = *iter->second;
      sandboxes.push_back(sandbox);
    }
  }
}

DebuggerProxyPtr Debugger::findProxy(const std::string &id) {
  ReadLock lock(m_mutex);
  return m_proxies[id];
}

void Debugger::switchSandbox(DebuggerProxyPtr proxy,
                             const std::string &newId) {
  WriteLock lock(m_mutex);

  string oldId = proxy->getSandboxId();
  m_proxies[oldId].reset();
  m_proxies[newId] = proxy;

  // makes sure proxy's sandbox info is complete with path, etc..
  if (m_sandboxes.find(newId) != m_sandboxes.end()) {
    proxy->updateSandbox(m_sandboxes[newId]);
  } else {
    DSandboxInfoPtr sb(new DSandboxInfo(newId));
    m_sandboxes[newId] = sb;
    proxy->updateSandbox(sb);
  }

  flagDebugger(newId);
}

void Debugger::flagDebugger(const std::string &id) {
  ThreadInfoSet &infos = m_sandboxThreads[id];
  for (ThreadInfoSet::iterator iter = infos.begin();
       iter != infos.end(); ++iter) {
    (*iter)->m_reqInjectionData.debugger = true;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
