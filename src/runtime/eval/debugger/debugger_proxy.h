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

#ifndef __HPHP_EVAL_DEBUGGER_PROXY_H__
#define __HPHP_EVAL_DEBUGGER_PROXY_H__

#include <util/base.h>
#include <util/synchronizable.h>
#include <util/async_func.h>
#include <runtime/base/file/socket.h>
#include <runtime/eval/debugger/dummy_sandbox.h>
#include <runtime/vm/instrumentation.h>

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
    Exclusive
  };

  static std::string MakePHP(const std::string &php);
  static std::string MakePHPReturn(const std::string &php);
  static Variant ExecutePHP(const std::string &php, String &output, bool log,
                            int frame);

public:
  DebuggerProxy(SmartPtr<Socket> socket, bool local);
  virtual ~DebuggerProxy();

  bool isLocal() const { return m_local;}

  const char *getThreadType() const;
  DSandboxInfo getSandbox() const;
  std::string getSandboxId() const;

  void getThreads(DThreadInfoPtrVec &threads);
  void switchSandbox(const std::string &newId);
  void updateSandbox(DSandboxInfoPtr sandbox);
  bool switchThread(DThreadInfoPtr thread);
  void switchThreadMode(ThreadMode mode, int64 threadId = 0);

  void startDummySandbox();
  void notifyDummySandbox();

  void setBreakPoints(BreakPointInfoPtrVec &breakpoints);

  bool needInterrupt();
  virtual void interrupt(CmdInterrupt &cmd);
  bool send(DebuggerCommand *cmd);

  void startSignalThread();
  void pollSignal(); // for signal polling thread

protected:
  bool m_stopped;

  bool m_local;
  DebuggerThriftBuffer m_thrift;
  DummySandboxPtr m_dummySandbox;

  mutable Mutex m_mutex;
  bool m_hasBreakPoints;
  BreakPointInfoPtrVec m_breakpoints;
  DSandboxInfo m_sandbox;

  ThreadMode m_threadMode;
  int64 m_thread;
  DThreadInfoPtr m_newThread;
  std::map<int64, DThreadInfoPtr> m_threads;

  CmdFlowControlPtr m_flow; // c, s, n, o commands that can skip breakpoints
  CmdJumpPtr m_jump;

  Mutex m_signalMutex; // who can talk to client
  AsyncFunc<DebuggerProxy> m_signalThread; // polling signals from client

  Mutex m_signumMutex;
  int m_signum;

  // helpers
  bool blockUntilOwn(CmdInterrupt &cmd, bool check);
  bool checkBreakPoints(CmdInterrupt &cmd);
  bool checkJumpFlowBreak(CmdInterrupt &cmd);
  virtual bool processJumpFlowBreak(CmdInterrupt &cmd);
  void processInterrupt(CmdInterrupt &cmd);
  virtual void processFlowControl(CmdInterrupt &cmd);
  virtual bool breakByFlowControl(CmdInterrupt &cmd);

  DThreadInfoPtr createThreadInfo(const std::string &desc);
};

class DebuggerProxyVM : public DebuggerProxy {
public:
  static Variant ExecutePHP(const std::string &php, String &output, bool log,
                            int frame);

public:
  DebuggerProxyVM(SmartPtr<Socket> socket, bool local)
    : DebuggerProxy(socket, local), m_injTables(NULL) {
  }
  virtual ~DebuggerProxyVM() {
    delete m_injTables;
  }
  virtual void interrupt(CmdInterrupt &cmd);

  // For instrumentation
  HPHP::VM::InjectionTables* getInjTables() const { return m_injTables; }
  void setInjTables(HPHP::VM::InjectionTables* tables) { m_injTables = tables;}
  void readInjTablesFromThread();
  void writeInjTablesToThread();

private:
  int getStackDepth();

  virtual bool processJumpFlowBreak(CmdInterrupt &cmd);
  virtual void processFlowControl(CmdInterrupt &cmd);
  virtual bool breakByFlowControl(CmdInterrupt &cmd);

  // For instrumentation
  HPHP::VM::InjectionTables* m_injTables;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_PROXY_H__
