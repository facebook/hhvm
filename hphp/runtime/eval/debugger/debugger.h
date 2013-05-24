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

#ifndef incl_HPHP_EVAL_DEBUGGER_H_
#define incl_HPHP_EVAL_DEBUGGER_H_

#include "hphp/util/lock.h"
#include "hphp/runtime/eval/debugger/debugger_proxy.h"
#include "hphp/runtime/base/program_functions.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_queue.h"

namespace HPHP { namespace Eval {

// Tag used on server log messages to highlight that they are likely useful to
// show to users.
#define DEBUGGER_LOG_TAG "[DBGINFO] "

///////////////////////////////////////////////////////////////////////////////
// The Debugger generally manages proxies, sandboxes, and is the inital handler
// of interrupts from the VM. It associates VM threads with sandboxes, and
// sandboxes with proxies. Interrupts get minimal handling before being handed
// off to the proper proxy.

DECLARE_BOOST_TYPES(CmdInterrupt);

class Debugger {
public:
  Debugger() : m_usageLogger(nullptr) {}

  // Start/stop Debugger for remote debugging.
  static bool StartServer();
  static DebuggerProxyPtr StartClient(const DebuggerClientOptions &options);
  static void Stop();

  // Add a new sandbox a debugger can connect to.
  static void RegisterSandbox(const DSandboxInfo &sandbox);
  static void UnregisterSandbox(CStrRef id);
  static void RegisterThread();

  //  Add/remove/change DebuggerProxy.
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

  // Called from differnt time point of execution thread.
  static void InterruptSessionStarted(const char *file,
                                      const char *error = nullptr);
  static void InterruptSessionEnded(const char *file);
  static void InterruptRequestStarted(const char *url);
  static void InterruptRequestEnded(const char *url);
  static void InterruptPSPEnded(const char *url);

  // Called when a user handler fails to handle an exception.
  static bool InterruptException(CVarRef e);

  // Interrupt from VM
  static void InterruptVMHook(int type = BreakPointReached,
                              CVarRef e = null_variant);

  // Surround text with color, if set.
  static void SetTextColors();
  static String ColorStdout(CStrRef s);
  static String ColorStderr(CStrRef s);

  // Log debugging state when we're shutting the server down.
  enum ShutdownKind {
    Normal,
    Abnormal
  };

  static void LogShutdown(ShutdownKind shutdownKind);

  // Usage logging
  static void SetUsageLogger(DebuggerUsageLogger *usageLogger);
  static void InitUsageLogging();
  static void UsageLog(const std::string &mode, const std::string &cmd,
                       const std::string &data = "");
  static void UsageLogInterrupt(const std::string &mode, CmdInterrupt &cmd);

private:
  static Debugger s_debugger;
  static bool s_clientStarted;

  static void Interrupt(int type, const char *program,
                        InterruptSite *site = nullptr, const char *error = nullptr);
  static void InterruptWithUrl(int type, const char *url);

  static const char *InterruptTypeName(CmdInterrupt &cmd);

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

  // A usage logger which is set by a provider to an implementation-specific
  // subclass if usage logging is enabled.
  DebuggerUsageLogger *m_usageLogger;
};

class DebuggerDummyEnv {
public:
  DebuggerDummyEnv();
  ~DebuggerDummyEnv();
};

// Suppress the debugger's ability to get interrupted while executing PHP.
// Primarily used to suppress debugger events while evaling PHP in response
// to commands like print, or for expressions in conditional breakpoints.
class EvalBreakControl {
public:
  explicit EvalBreakControl(bool noBreak);
  ~EvalBreakControl();
private:
  bool m_noBreakSave;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_H_
