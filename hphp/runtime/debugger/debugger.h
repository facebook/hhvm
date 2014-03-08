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

#ifndef incl_HPHP_EVAL_DEBUGGER_H_
#define incl_HPHP_EVAL_DEBUGGER_H_

#include <vector>

#include "hphp/util/lock.h"
#include "hphp/runtime/debugger/debugger_proxy.h"
#include "hphp/runtime/base/program-functions.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <set>

namespace HPHP {
struct ThreadInfo;
}

namespace HPHP { namespace Eval {

// Tag used on server log messages to highlight that they are likely useful to
// show to users.
#define DEBUGGER_LOG_TAG "[DBGINFO] "

///////////////////////////////////////////////////////////////////////////////
// The Debugger generally manages proxies, sandboxes, and is the inital handler
// of interrupts from the VM. It associates VM threads with sandboxes, and
// sandboxes with proxies. Interrupts get minimal handling before being handed
// off to the proper proxy.

struct CmdInterrupt;

class Debugger {
public:
  Debugger() : m_usageLogger(nullptr) {}

  // Start/stop Debugger for remote debugging.
  static bool StartServer();
  static DebuggerProxyPtr StartClient(const DebuggerClientOptions &options);
  static void Stop();

  // Add a new sandbox a debugger can connect to.
  static void RegisterSandbox(const DSandboxInfo &sandbox);
  static void UnregisterSandbox(const String& id);

  //  Add/remove/change DebuggerProxy.
  static DebuggerProxyPtr CreateProxy(SmartPtr<Socket> socket, bool local);
  static void RemoveProxy(DebuggerProxyPtr proxy);
  static bool SwitchSandbox(DebuggerProxyPtr proxy, const std::string &newId,
                            bool force);
  static int CountConnectedProxy();
  static DebuggerProxyPtr GetProxy();

  static void GetRegisteredSandboxes(std::vector<DSandboxInfoPtr> &sandboxes);
  static bool IsThreadDebugging(int64_t tid);

  static void RetireProxy(DebuggerProxyPtr proxy);
  static void CleanupRetiredProxies();

  // Request interrupt on threads that a proxy is attached to
  static void RequestInterrupt(DebuggerProxyPtr proxy);

  // Debugger session to be called in a loop
  static void DebuggerSession(const DebuggerClientOptions& options,
                              bool restart);

  // Called from differnt time point of execution thread.
  static void InterruptSessionStarted(const char *file,
                                      const char *error = nullptr);
  static void InterruptSessionEnded(const char *file);
  static void InterruptRequestStarted(const char *url);
  static void InterruptRequestEnded(const char *url);
  static void InterruptPSPEnded(const char *url);

  // Interrupt from VM
  static void InterruptVMHook(int type = BreakPointReached,
                              const Variant& e = null_variant);

  // Surround text with color, if set.
  static void SetTextColors();
  static String ColorStdout(const String& s);
  static String ColorStderr(const String& s);

  // Log debugging state when we're shutting the server down.
  enum ShutdownKind {
    Normal,
    Abnormal
  };

  static void LogShutdown(ShutdownKind shutdownKind);

  // Usage logging
  static void SetUsageLogger(DebuggerUsageLogger *usageLogger);
  static void InitUsageLogging();
  static void UsageLog(const std::string &mode,
                       const std::string &sandboxId,
                       const std::string &cmd,
                       const std::string &data = "");
  static void UsageLogInterrupt(const std::string &mode,
                                const std::string &sandboxId,
                                CmdInterrupt &cmd);

private:
  static Debugger s_debugger;
  static bool s_clientStarted;

  static void Interrupt(int type, const char *program,
                        InterruptSite *site = nullptr, const char *error = nullptr);
  static void InterruptWithUrl(int type, const char *url);

  static const char *InterruptTypeName(CmdInterrupt &cmd);

  // The following maps use a "sandbox id", or sid, which is a string containing
  // the user's name, a tab, and the sandbox name. I.e., "mikemag\tdefault".

  // Map of "sandbox id"->proxy. The map contains an entry for every active
  // proxy attached to this process, keyed by the sandbox being debugged. It
  // also contains an entry for the proxy's dummy sandbox, keyed by the dummy
  // sandbox id which is basically the proxy pointer rendered as a string.
  typedef tbb::concurrent_hash_map<const StringData*, DebuggerProxyPtr,
                                   StringDataHashCompare> ProxyMap;
  ProxyMap m_proxyMap;

  // Map of "sandbox id"->"sandbox info" for any sandbox the process has seen.
  // Entries are made when a request is received for a sandbox, or when a switch
  // to a new sandbox id is made via CmdMachine. The latter alows a debugger to
  // "pre-attach" to a sandbox before the first request is every received.
  // Entries are never removed.
  typedef tbb::concurrent_hash_map<const StringData*, DSandboxInfoPtr,
                                   StringDataHashCompare> SandboxMap;
  SandboxMap m_sandboxMap;

  // Map of "sandbox id"->"set of threads executing requests in the sandbox".
  typedef std::set<ThreadInfo*> ThreadInfoSet;
  typedef tbb::concurrent_hash_map<const StringData*, ThreadInfoSet,
                                   StringDataHashCompare> SandboxThreadInfoMap;
  SandboxThreadInfoMap m_sandboxThreadInfoMap;

  // "thread id"->"thread info". Each thread which is being debugged is
  // added to this map.
  typedef tbb::concurrent_hash_map<int64_t, ThreadInfo*> ThreadInfoMap;
  ThreadInfoMap m_threadInfos;

  typedef tbb::concurrent_queue<DebuggerProxyPtr> RetiredProxyQueue;
  RetiredProxyQueue m_retiredProxyQueue;

  bool isThreadDebugging(int64_t id);
  void registerThread();
  void addOrUpdateSandbox(const DSandboxInfo &sandbox);
  DSandboxInfoPtr getSandbox(const StringData* sid);
  void getSandboxes(std::vector<DSandboxInfoPtr> &sandboxes);
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
  void retireProxy(DebuggerProxyPtr proxy);
  void cleanupRetiredProxies();

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
