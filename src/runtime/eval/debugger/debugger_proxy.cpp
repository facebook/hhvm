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

#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DebuggerProxy::DebuggerProxy(SmartPtr<Socket> socket) {
  m_thrift.create(socket);
}

void DebuggerProxy::startDummySandbox() {
  m_dummySandbox = DummySandboxPtr
    (new DummySandbox(m_sandbox, RuntimeOption::DebuggerStartupDocument));
  m_dummySandbox->start();
}

void DebuggerProxy::switchSandbox(const std::string &id) {
  SandboxInfo sandbox(id);
  Debugger::SwitchSandbox(shared_from_this(), sandbox);

  // This has to be done after Debugger::SwitchSandbox() who still needs the
  // old m_sandbox value.
  m_sandbox = sandbox;
}

void DebuggerProxy::interrupt(CmdInterrupt &cmd) {
  if (!cmd.shouldBreak(m_breakpoints)) {
    return;
  }
  if (!cmd.onServer(this)) {
    Debugger::RemoveProxy(shared_from_this()); // on socket error
    return;
  }
  while (true) {
    DebuggerCommandPtr res;
    while (!DebuggerCommand::Receive(m_thrift, res,
                                     "DebuggerProxy::onCommand()")) {
      // we will wait forever until DebuggerClient sends us something
    }
    if (res && res->is(DebuggerCommand::KindOfContinue)) {
      return;
    }
    if (!res || res->is(DebuggerCommand::KindOfQuit) || !res->onServer(this)) {
      Debugger::RemoveProxy(shared_from_this()); // on socket error
      return;
    }
  }
}

bool DebuggerProxy::send(DebuggerCommand *cmd) {
  return cmd->send(m_thrift);
}

///////////////////////////////////////////////////////////////////////////////
}}
