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

#ifndef incl_HPHP_EVAL_DEBUGGER_PROXY_H_
#define incl_HPHP_EVAL_DEBUGGER_PROXY_H_

#include <util/base.h>
#include <util/synchronizable.h>
#include <util/async_func.h>
#include <runtime/base/file/socket.h>
#include <runtime/eval/debugger/dummy_sandbox.h>
#include <runtime/vm/instrumentation.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
// A DebuggerProxy provides a conection thru which a client may talk to a VM
// which is being debugged. The VM can also send messages to the client via the
// proxy, either in reponse to messages from the client, or to poll the client
// for information.
//
// In an basic scenario where a client is debugging a remote VM, the VM will
// create a proxy when the client connects (via DebuggerServer) and listen for
// commands via this proxy. It will use this proxy when completing control flow
// commands to interrupt the client. The client sends and receives messages over
// a socket directly to this proxy. Thus we have:
//
//   Client <---> Proxy <---> VM
//
// The client always creates its own "local proxy", which allows debugging any
// code running on the VM within the client. The two are easily confused.
//

class CmdInterrupt;
DECLARE_BOOST_TYPES(DebuggerProxy);
DECLARE_BOOST_TYPES(DebuggerCommand);
DECLARE_BOOST_TYPES(CmdFlowControl);

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
  ~DebuggerProxy();

  bool isLocal() const { return m_local;}

  const char *getThreadType() const;
  DSandboxInfo getSandbox() const;
  std::string getSandboxId() const;
  const DSandboxInfo& getDummyInfo() const { return m_dummyInfo; }

  void getThreads(DThreadInfoPtrVec &threads);
  bool switchSandbox(const std::string &newId, bool force);
  void updateSandbox(DSandboxInfoPtr sandbox);
  bool switchThread(DThreadInfoPtr thread);
  void switchThreadMode(ThreadMode mode, int64_t threadId = 0);

  void startDummySandbox();
  void notifyDummySandbox();

  void setBreakPoints(BreakPointInfoPtrVec &breakpoints);
  void getBreakPoints(BreakPointInfoPtrVec &breakpoints);
  bool couldBreakEnterClsMethod(const StringData* className);
  bool couldBreakEnterFunc(const StringData* funcFullName);
  void getBreakClsMethods(std::vector<const StringData*>& classNames);
  void getBreakFuncs(std::vector<const StringData*>& funcFullNames);

  bool needInterrupt();
  bool needInterruptForNonBreak();
  void interrupt(CmdInterrupt &cmd);
  bool sendToClient(DebuggerCommand *cmd);

  void startSignalThread();
  void pollSignal(); // for signal polling thread

  void checkStop();
  void forceQuit();

  // For instrumentation
  HPHP::VM::InjectionTables* getInjTables() const { return m_injTables; }
  void setInjTables(HPHP::VM::InjectionTables* tables) { m_injTables = tables;}
  void readInjTablesFromThread();
  void writeInjTablesToThread();

private:
  bool blockUntilOwn(CmdInterrupt &cmd, bool check);
  bool checkBreakPoints(CmdInterrupt &cmd);
  bool checkFlowBreak(CmdInterrupt &cmd);
  bool processFlowBreak(CmdInterrupt &cmd);
  void processInterrupt(CmdInterrupt &cmd);
  void processFlowControl(CmdInterrupt &cmd);
  bool breakByFlowControl(CmdInterrupt &cmd);

  DThreadInfoPtr createThreadInfo(const std::string &desc);

  void changeBreakPointDepth(CmdInterrupt& cmd);
  BreakPointInfoPtr getBreakPointAtCmd(CmdInterrupt& cmd);

  int getRealStackDepth();
  int getStackDepth();

  bool m_stopped;

  bool m_local;
  DebuggerThriftBuffer m_thrift;
  DummySandbox* m_dummySandbox;

  mutable Mutex m_mutex;
  ReadWriteMutex m_breakMutex;
  bool m_hasBreakPoints;
  BreakPointInfoPtrVec m_breakpoints;
  DSandboxInfo m_sandbox;
  DSandboxInfo m_dummyInfo;

  ThreadMode m_threadMode;
  int64_t m_thread;
  DThreadInfoPtr m_newThread;
  std::map<int64_t, DThreadInfoPtr> m_threads;

  typedef tbb::concurrent_hash_map<const StringData*, void*,
                                   StringDataHashCompare> StringDataMap;
  StringDataMap m_breaksEnterClsMethod;
  StringDataMap m_breaksEnterFunc;

  CmdFlowControlPtr m_flow; // c, s, n, o commands that can skip breakpoints

  Mutex m_signalMutex; // who can talk to client
  AsyncFunc<DebuggerProxy> m_signalThread; // polling signals from client

  Mutex m_signumMutex;
  int m_signum;

  // For instrumentation
  HPHP::VM::InjectionTables* m_injTables;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_PROXY_H_
