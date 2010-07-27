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
class DebuggerProxy : public Synchronizable,
                      public boost::enable_shared_from_this<DebuggerProxy> {
public:
  DebuggerProxy(SmartPtr<Socket> socket, bool local);

  bool isLocal() const { return m_local;}
  std::string getSandboxId() const { return m_sandbox.id();}

  void startDummySandbox();
  void switchSandbox(const std::string &id);

  void interrupt(CmdInterrupt &cmd);
  bool send(DebuggerCommand *cmd);

  void setBreakPoints(BreakPointInfoPtrVec &breakpoints) {
    m_breakpoints = breakpoints;
  }

private:
  bool m_local;
  DebuggerThriftBuffer m_thrift;
  SandboxInfo m_sandbox;
  DummySandboxPtr m_dummySandbox;

  BreakPointInfoPtrVec m_breakpoints;
  CmdFlowControlPtr m_burner; // c, s, n, o commands that can burn breakpoints

  // helpers
  void processFlowControl(CmdInterrupt &cmd);
  bool breakByFlowControl(CmdInterrupt &cmd);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_PROXY_H__
