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
  static void StartServer();
  static void StartClient(const std::string &host, int port);
  static void Stop();

  /**
   * Add a new sandbox a debugger can connect to.
   */
  static void RegisterSandbox(const SandboxInfo &sandbox);
  static void GetRegisteredSandboxes(StringVec &ids);

  /**
   * Add/remove/change DebuggerProxy.
   */
  static void RegisterProxy(SmartPtr<Socket> socket, bool dummy, bool local);
  static void RemoveProxy(DebuggerProxyPtr proxy);
  static void SwitchSandbox(DebuggerProxyPtr proxy,
                            const SandboxInfo &sandbox);

  /**
   * Called from differnt time point of execution thread.
   */
  static void InterruptSessionStarted(const char *file);
  static void InterruptSessionEnded(const char *file);
  static void InterruptRequestStarted(const char *url);
  static void InterruptRequestEnded(const char *url);
  static void InterruptPSPEnded(const char *url);

  /**
   * A new line of PHP code is reached from execution thread.
   */
  static void InterruptFileLine(InterruptSite &site);
  static void InterruptException(InterruptSite &site);

  /**
   * Surround text with color, if set.
   */
  static void SetTextColors();
  static String ColorStdout(CStrRef s);
  static String ColorStderr(CStrRef s);

private:
  static Debugger s_debugger;

  static DebuggerProxyPtrSet GetProxies();
  static void Interrupt(int type, const char *program,
                        InterruptSite *site = NULL);

  ReadWriteMutex m_mutex;
  StringToDebuggerProxyPtrSetMap m_proxies;

  void stop();

  void addSandbox(const SandboxInfo &sandbox);
  void getSandboxes(StringVec &ids);

  void addProxy(SmartPtr<Socket> socket, bool dummy, bool local);
  void removeProxy(DebuggerProxyPtr proxy);

  DebuggerProxyPtrSet findProxies(const SandboxInfo &sandbox);
  void switchSandbox(DebuggerProxyPtr proxy, const SandboxInfo &sandbox);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_H__
