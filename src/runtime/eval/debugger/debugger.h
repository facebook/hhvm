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

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class Debugger {
public:
  /**
   * Start/stop Debugger for remote debugging.
   */
  static bool StartServer();
  static void StartClient(const DebuggerClientOptions &options);
  static void Stop();

  /**
   * Add a new sandbox a debugger can connect to.
   */
  static void RegisterSandbox(const DSandboxInfo &sandbox);
  static void GetRegisteredSandboxes(DSandboxInfoPtrVec &sandboxes);
  static bool IsThreadDebugging(int64 id);

  /**
   * Add/remove/change DebuggerProxy.
   */
  static void RegisterProxy(SmartPtr<Socket> socket, bool local);
  static void RemoveProxy(DebuggerProxyPtr proxy);
  static void SwitchSandbox(DebuggerProxyPtr proxy, const std::string &newId);

  /**
   * Called from differnt time point of execution thread.
   */
  static void InterruptSessionStarted(const char *file,
                                      const char *error = NULL);
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
   * Surround text with color, if set.
   */
  static void SetTextColors();
  static String ColorStdout(CStrRef s);
  static String ColorStderr(CStrRef s);

private:
  static Debugger s_debugger;

  static DebuggerProxyPtr GetProxy();
  static void Interrupt(int type, const char *program,
                        InterruptSite *site = NULL, const char *error = NULL);

  ReadWriteMutex m_mutex;
  StringToDebuggerProxyPtrMap m_proxies;
  StringToDSandboxInfoPtrMap m_sandboxes;

  /**
   * m_sandboxThreads stores threads by sandbox id. These threads were started
   * without finding a matched DebuggerProxy. Newly attached DebuggerProxy
   * can check this set to mark them with "debugger" flag on ThreadInfo's
   * RequestInjectionData. This way, these threads will start to interrupt.
   * The whole purpose of doing this is to make sure threads that don't have
   * debugger attached will not keep checking whether a DebuggerProxy has been
   * attached.
   */
  typedef std::set<ThreadInfo*> ThreadInfoSet;
  typedef std::map<int64, ThreadInfo*> ThreadInfoMap;
  typedef std::map<std::string, ThreadInfoSet> StringToThreadInfoSet;
  ThreadInfoMap m_threadInfos;
  StringToThreadInfoSet m_sandboxThreads;

  void flagDebugger(const std::string &id);
  bool isThreadDebugging(int64 id);

  void stop();

  void addSandbox(const DSandboxInfo &sandbox);
  void getSandboxes(DSandboxInfoPtrVec &sandboxes);

  void addProxy(SmartPtr<Socket> socket, bool local);
  void removeProxy(DebuggerProxyPtr proxy);

  DebuggerProxyPtr findProxy(const std::string &id);
  void switchSandbox(DebuggerProxyPtr proxy, const std::string &newId);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_H__
