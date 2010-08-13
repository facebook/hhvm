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

void Debugger::StartServer() {
  DebuggerServer::Start();
}

void Debugger::StartClient(const std::string &host, int port,
                           const std::string &extension,
                           const StringVec &cmds) {
  SmartPtr<Socket> localProxy = DebuggerClient::Start(host, port, extension,
                                                      cmds);
  if (localProxy.get()) {
    RegisterProxy(localProxy, true);
  }
}

void Debugger::OnServerShutdown() {
  s_debugger.clearThreadInfos();
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

void Debugger::GetRegisteredSandboxes(StringVec &ids) {
  s_debugger.getSandboxes(ids);
}

void Debugger::RegisterProxy(SmartPtr<Socket> socket, bool local) {
  s_debugger.addProxy(socket, local);
}

void Debugger::RemoveProxy(DebuggerProxyPtr proxy) {
  s_debugger.removeProxy(proxy);
}

void Debugger::SwitchSandbox(DebuggerProxyPtr proxy,
                             const DSandboxInfo &sandbox) {
  s_debugger.switchSandbox(proxy, sandbox);
}

///////////////////////////////////////////////////////////////////////////////

DebuggerProxyPtrSet Debugger::GetProxies() {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  Variant &server = g->gv__SERVER;
  String id = server["HPHP_SANDBOX_ID"];
  if (id.empty()) {
    DSandboxInfo sandbox;
    sandbox.m_user = server["HPHP_SANDBOX_USER"].toString().data();
    sandbox.m_name = server["HPHP_SANDBOX_NAME"].toString().data();
    sandbox.m_path = server["HPHP_SANDBOX_PATH"].toString().data();
    id = String(sandbox.id());
    server.set("HPHP_SANDBOX_ID", id);
  }
  return s_debugger.findProxies(id.data());
}

void Debugger::InterruptSessionStarted(const char *file) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.debugger = true;
  Interrupt(SessionStarted, file);
}

void Debugger::InterruptSessionEnded(const char *file) {
  Interrupt(SessionEnded, file);
}

void Debugger::InterruptRequestStarted(const char *url) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.get();
  if (ti->m_reqInjectionData.debugger) {
    Interrupt(RequestStarted, url);
  }
}

void Debugger::InterruptRequestEnded(const char *url) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.get();
  if (ti->m_reqInjectionData.debugger) {
    Interrupt(RequestEnded, url);
  }
}

void Debugger::InterruptPSPEnded(const char *url) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.get();
  if (ti->m_reqInjectionData.debugger) {
    Interrupt(PSPEnded, url);
  }
}

void Debugger::InterruptFileLine(InterruptSite &site) {
  Interrupt(BreakPointReached, NULL, &site);
}

void Debugger::InterruptException(InterruptSite &site) {
  Interrupt(ExceptionThrown, NULL, &site);
}

void Debugger::Interrupt(int type, const char *program,
                         InterruptSite *site /* = NULL */) {
  ASSERT(RuntimeOption::EnableDebugger);

  DebuggerProxyPtrSet proxies = GetProxies();
  if (proxies.empty()) {
    // debugger clients are disconnected abnormally
    if (type == SessionStarted || type == SessionEnded) {
      // for command line programs, we need this exception to exit from
      // the infinite execution loop
      throw DebuggerClientExitException();
    }
    return;
  }

  for (DebuggerProxyPtrSet::const_iterator iter = proxies.begin();
       iter != proxies.end(); ++iter) {
    CmdInterrupt cmd((InterruptType)type, program, site);
    (*iter)->interrupt(cmd);
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

void Debugger::clearThreadInfos() {
  WriteLock lock(m_mutex);
  m_threadInfos.clear();
}

void Debugger::stop() {
  WriteLock lock(m_mutex);
  m_proxies.clear();
  m_threadInfos.clear();
}

void Debugger::addSandbox(const DSandboxInfo &sandbox) {
  WriteLock lock(m_mutex);
  ThreadInfo *ti = ThreadInfo::s_threadInfo.get();
  string id = sandbox.id();
  if (!m_proxies[id].empty()) {
    ti->m_reqInjectionData.debugger = true;
    m_threadInfos[id].erase(ti);
  } else {
    m_threadInfos[id].insert(ti);
  }
}

void Debugger::addProxy(SmartPtr<Socket> socket, bool local) {
  DebuggerProxyPtr proxy(new DebuggerProxy(socket, local));
  {
    WriteLock lock(m_mutex);
    string id = proxy->getSandboxId();
    m_proxies[id].insert(proxy);
    flagDebugger(id);
  }
  if (!local) {
    proxy->startDummySandbox();
  }
}

void Debugger::removeProxy(DebuggerProxyPtr proxy) {
  WriteLock lock(m_mutex);
  m_proxies[proxy->getSandboxId()].erase(proxy);
}

void Debugger::getSandboxes(StringVec &ids) {
  ReadLock lock(m_mutex);
  ids.reserve(m_proxies.size());
  for (StringToDebuggerProxyPtrSetMap::const_iterator iter =
         m_proxies.begin(); iter != m_proxies.end(); ++iter) {
    if (!iter->first.empty()) {
      ids.push_back(iter->first);
    }
  }
}

DebuggerProxyPtrSet Debugger::findProxies(const std::string &id) {
  ReadLock lock(m_mutex);
  return m_proxies[id];
}

void Debugger::switchSandbox(DebuggerProxyPtr proxy,
                             const DSandboxInfo &sandbox) {
  string oldId = proxy->getSandboxId();
  string newId = sandbox.id();
  if (newId != oldId) {
    WriteLock lock(m_mutex);
    m_proxies[oldId].erase(proxy);
    m_proxies[newId].insert(proxy);
    flagDebugger(newId);
  }
}

void Debugger::flagDebugger(const std::string &id) {
  ThreadInfoSet &infos = m_threadInfos[id];
  for (ThreadInfoSet::iterator iter = infos.begin();
       iter != infos.end(); ++iter) {
    (*iter)->m_reqInjectionData.debugger = true;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
