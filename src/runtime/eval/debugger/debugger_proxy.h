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

#ifndef __HPHP_EVAL_DEBUGGER_PROXY_H__
#define __HPHP_EVAL_DEBUGGER_PROXY_H__

#include <util/base.h>
#include <util/synchronizable.h>
#include <util/async_func.h>
#include <runtime/base/file/socket.h>
#include <runtime/eval/debugger/dummy_sandbox.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class CmdInterrupt;
DECLARE_BOOST_TYPES(DebuggerProxy);
DECLARE_BOOST_TYPES(DebuggerCommand);
DECLARE_BOOST_TYPES(CmdFlowControl);
DECLARE_BOOST_TYPES(CmdJump);
class DebuggerProxy : public Synchronizable,
                      public boost::enable_shared_from_this<DebuggerProxy> {
public:
  enum ThreadMode {
    Normal,
    Sticky,
    Exclusive,
  };

  static std::string MakePHP(const std::string &php);
  static std::string MakePHPReturn(const std::string &php);
  static Variant ExecutePHP(const std::string &php, String &output);
  static DThreadInfoPtr CreateThreadInfo(const std::string &desc);

public:
  DebuggerProxy(SmartPtr<Socket> socket, bool local);
  ~DebuggerProxy();

  bool isLocal() const { return m_local;}
  const DSandboxInfo &getSandbox() const { return m_sandbox;}
  std::string getSandboxId() const { return m_sandbox.id();}
  void getThreads(DThreadInfoPtrVec &threads);

  void startDummySandbox();
  void switchSandbox(const std::string &id);
  bool switchThread(DThreadInfoPtr thread);
  void switchThreadMode(ThreadMode mode, int64 threadId = 0);
  void setBreakPoints(BreakPointInfoPtrVec &breakpoints);

  void interrupt(CmdInterrupt &cmd);
  bool send(DebuggerCommand *cmd);

  void pollSignal(); // for signal polling thread

private:
  bool m_local;
  DebuggerThriftBuffer m_thrift;
  DSandboxInfo m_sandbox;
  DummySandboxPtr m_dummySandbox;

  Mutex m_bpMutex;
  BreakPointInfoPtrVec m_breakpoints;

  ThreadMode m_threadMode;
  pthread_t m_thread;
  DThreadInfoPtr m_newThread;
  std::map<pthread_t, DThreadInfoPtr> m_threads;
  CmdInterrupt *m_interrupt;

  CmdFlowControlPtr m_flow; // c, s, n, o commands that can skip breakpoints
  CmdJumpPtr m_jump;

  Mutex m_signalMutex;
  AsyncFunc<DebuggerProxy> m_signalThread; // polling signals from client
  int m_signum;

  // helpers
  bool blockUntilOwn(CmdInterrupt &cmd, bool check);
  bool checkBreakPoints(CmdInterrupt &cmd);
  bool checkJumpFlowBreak(CmdInterrupt &cmd);
  bool processJumpFlowBreak(CmdInterrupt &cmd);
  void processInterrupt(CmdInterrupt &cmd);
  void processFlowControl(CmdInterrupt &cmd);
  bool breakByFlowControl(CmdInterrupt &cmd);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_PROXY_H__
