/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/base.h"
#include "hphp/util/synchronizable.h"
#include "hphp/util/async-func.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/debugger/dummy_sandbox.h"

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
                      public std::enable_shared_from_this<DebuggerProxy> {
public:
  enum ThreadMode {
    Normal,
    Sticky,
    Exclusive
  };

  static std::string MakePHP(const std::string &php);
  static std::string MakePHPReturn(const std::string &php);

public:
  DebuggerProxy(SmartPtr<Socket> socket, bool local);

  bool isLocal() const { return m_local;}

  const char *getThreadType() const;
  DSandboxInfo getSandbox();
  std::string getSandboxId();
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

  void setBreakableForBreakpointsNotMatching(CmdInterrupt& cmd);
  void unsetBreakableForBreakpointsMatching(CmdInterrupt& cmd);

  bool needInterrupt();
  bool needVMInterrupts();
  void interrupt(CmdInterrupt &cmd);
  bool sendToClient(DebuggerCommand *cmd);
  CmdInterrupt& currentInterruptCmd();

  int getStackDepth();
  int getRealStackDepth();

  void startSignalThread();
  void stop();
  bool cleanup(int timeout);

  bool getClientConnectionInfo(VRefParam address, VRefParam port);

  enum ExecutePHPFlags {
    ExecutePHPFlagsNone = 0x0, // No logging, not at an interrupt
    ExecutePHPFlagsLog = 0x1, // Add logs to the output string
    ExecutePHPFlagsAtInterrupt = 0x02 // Called when stopped at an interrupt
  };

  Variant ExecutePHP(const std::string &php, String &output, int frame,
                     bool &failed, int flags);

private:
  bool blockUntilOwn(CmdInterrupt &cmd, bool check);
  bool checkBreakPoints(CmdInterrupt &cmd);
  bool checkFlowBreak(CmdInterrupt &cmd);
  void processInterrupt(CmdInterrupt &cmd);
  void enableSignalPolling();
  void disableSignalPolling();
  void checkStop();
  void pollSignal(); // for signal polling thread
  void stopAndThrow();

  DThreadInfoPtr createThreadInfo(const std::string &desc);

  SmartPtr<Socket> getSocket() { return m_thrift.getSocket(); }

  bool m_stopped;

  bool m_local;
  DebuggerThriftBuffer m_thrift;
  DummySandbox* m_dummySandbox;

  ReadWriteMutex m_breakMutex;
  bool m_hasBreakPoints;
  BreakPointInfoPtrVec m_breakpoints;
  DSandboxInfo m_sandbox;
  DSandboxInfo m_dummyInfo;

  ThreadMode m_threadMode;
  int64_t m_thread; // Thread allowed to process interrupts
  DThreadInfoPtr m_newThread; // Used by CmdThread to switch threads
  std::map<int64_t, DThreadInfoPtr> m_threads; // Threads in blockUntilOwn

  CmdFlowControlPtr m_flow; // c, s, n, o commands that can skip breakpoints

  AsyncFunc<DebuggerProxy> m_signalThread; // polling signals from client

  // This mutex gates who can talk to the client (signal polling
  // thread vs. other threads who want to send an interrupt). Protects
  // m_signum, m_okayToPoll.
  Mutex m_signalMutex;
  bool m_okayToPoll; // whether the polling thread can send polls to the client
  int m_signum;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_PROXY_H_
