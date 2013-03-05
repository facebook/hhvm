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

#ifndef __HPHP_EVAL_DEBUGGER_H__
#define __HPHP_EVAL_DEBUGGER_H__

#include <util/lock.h>
#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/base/program_functions.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class Debugger {
public:
  /**
   * Start/stop Debugger for remote debugging.
   */
  static bool StartServer();
  static DebuggerProxyPtr StartClient(const DebuggerClientOptions &options);
  static void Stop();

  /**
   * Add a new sandbox a debugger can connect to.
   */
  static void RegisterSandbox(const DSandboxInfo &sandbox);
  static void UnregisterSandbox(CStrRef id);
  static void RegisterThread();

  /**
   * Add/remove/change DebuggerProxy.
   */
  static DebuggerProxyPtr CreateProxy(SmartPtr<Socket> socket, bool local);
  static void RemoveProxy(DebuggerProxyPtr proxy);
  static bool SwitchSandbox(DebuggerProxyPtr proxy, const std::string &newId,
                            bool force);
  static int CountConnectedProxy();
  static DebuggerProxyPtr GetProxy();

  static void GetRegisteredSandboxes(DSandboxInfoPtrVec &sandboxes);
  static bool IsThreadDebugging(int64_t tid);

  static void RetireDummySandboxThread(DummySandbox* toRetire);
  static void CleanupDummySandboxThreads();

  // Request interrupt on threads that a proxy is attached to
  static void RequestInterrupt(DebuggerProxyPtr proxy);

  // Debugger session to be called in a loop
  static void DebuggerSession(const DebuggerClientOptions& options,
                              const std::string& file, bool restart);

  /**
   * Called from differnt time point of execution thread.
   */
  static void InterruptSessionStarted(const char *file,
                                      const char *error = nullptr);
  static void InterruptSessionEnded(const char *file);
  static void InterruptRequestStarted(const char *url);
  static void InterruptRequestEnded(const char *url);
  static void InterruptPSPEnded(const char *url);

  /**
   * A new line of PHP code is reached from execution thread.
   */
  static void InterruptFileLine(InterruptSite &site);
  static void InterruptHard(InterruptSite &site);
  static bool InterruptException(CVarRef e);

  /**
   * Interrupt from VM
   */
  static void InterruptVMHook(int type = BreakPointReached,
                              CVarRef e = null_variant);

  /**
   * Surround text with color, if set.
   */
  static void SetTextColors();
  static String ColorStdout(CStrRef s);
  static String ColorStderr(CStrRef s);

private:
  static Debugger s_debugger;
  static bool s_clientStarted;

  static void Interrupt(int type, const char *program,
                        InterruptSite *site = nullptr, const char *error = nullptr);

  typedef tbb::concurrent_hash_map<const StringData*, DebuggerProxyPtr,
                                   StringDataHashCompare> ProxyMap;
  ProxyMap m_proxyMap;

  typedef tbb::concurrent_hash_map<const StringData*, DSandboxInfoPtr,
                                   StringDataHashCompare> SandboxMap;
  SandboxMap m_sandboxMap;

  typedef std::set<ThreadInfo*> ThreadInfoSet;
  typedef tbb::concurrent_hash_map<const StringData*, ThreadInfoSet,
                                   StringDataHashCompare> SandboxThreadInfoMap;
  SandboxThreadInfoMap m_sandboxThreadInfoMap;

  typedef tbb::concurrent_hash_map<int64_t, ThreadInfo*> ThreadInfoMap;
  ThreadInfoMap m_threadInfos; // tid => ThreadInfo*

  typedef tbb::concurrent_queue<DummySandbox*> DummySandboxQ;
  DummySandboxQ m_cleanupDummySandboxQ;

  bool isThreadDebugging(int64_t id);
  void registerThread();
  void updateSandbox(const DSandboxInfo &sandbox);
  DSandboxInfoPtr getSandbox(const StringData* sid);
  void getSandboxes(DSandboxInfoPtrVec &sandboxes);
  void registerSandbox(const DSandboxInfo &sandbox);
  void unregisterSandbox(const StringData* sandboxId);

  void getSandboxThreads(const DSandboxInfo &sandbox,
                         std::set<ThreadInfo*>& set);

  void requestInterrupt(DebuggerProxyPtr proxy);
  void setDebuggerFlag(const StringData* sandboxId, bool flag);

  DebuggerProxyPtr createProxy(SmartPtr<Socket> socket, bool local);
  void removeProxy(DebuggerProxyPtr proxy);
  DebuggerProxyPtr findProxy(const StringData* sandboxId);
  int countConnectedProxy() { return m_proxyMap.size(); } ;

  void updateProxySandbox(DebuggerProxyPtr proxy,
                          const StringData* sandboxId);
  bool switchSandboxImpl(DebuggerProxyPtr proxy,
                         const StringData* newSid,
                         bool force);
  bool switchSandbox(DebuggerProxyPtr proxy, const std::string &newId,
                     bool force);
  void retireDummySandboxThread(DummySandbox* toRetire);
  void cleanupDummySandboxThreads();
};

class DebuggerDummyEnv {
public:
  DebuggerDummyEnv();
  ~DebuggerDummyEnv();
};

class EvalBreakControl {
public:
  EvalBreakControl(bool noBreak);
  ~EvalBreakControl();
private:
  bool m_noBreakSave;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_H__
