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

#include "hphp/runtime/eval/debugger/debugger_command.h"
#include "hphp/runtime/eval/debugger/debugger.h"
#include "hphp/runtime/eval/debugger/cmd/all.h"
#include "hphp/util/logger.h"

#define POLLING_SECONDS 1

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

// send/recv
bool DebuggerCommand::send(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::send\n");
  try {
    thrift.reset(false);
    sendImpl(thrift);
    thrift.flush();
  } catch (...) {
    Logger::Error("DebuggerCommand::send(): a socket error has happened");
    return false;
  }
  return true;
}

bool DebuggerCommand::recv(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::recv\n");
  try {
    recvImpl(thrift);
  } catch (...) {
    Logger::Error("DebuggerCommand::recv(): a socket error has happened");
    return false;
  }
  return true;
}

void DebuggerCommand::sendImpl(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::sendImpl\n");
  thrift.write((int32_t)m_type);
  thrift.write(m_class);
  thrift.write(m_body);
  thrift.write(m_version);
}

void DebuggerCommand::recvImpl(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::recvImpl\n");
  thrift.read(m_body);
  thrift.read(m_version);
}

bool DebuggerCommand::Receive(DebuggerThriftBuffer &thrift,
                              DebuggerCommandPtr &cmd, const char *caller) {
  TRACE(5, "DebuggerCommand::Receive\n");
  cmd.reset();

  struct pollfd fds[1];
  fds[0].fd = thrift.getSocket()->fd();
  fds[0].events = POLLIN|POLLERR|POLLHUP;
  int ret = poll(fds, 1, POLLING_SECONDS * 1000);
  if (ret == 0) {
    return false;
  }
  if (ret == -1 || !(fds[0].revents & POLLIN)) {
    return errno != EINTR; // treat signals as timeouts
  }

  int32_t type;
  string clsname;
  try {
    thrift.reset(true);
    thrift.read(type);
    thrift.read(clsname);
  } catch (...) {
    Logger::Error("%s => DebuggerCommand::Receive(): socket error", caller);
    return true;
  }

  TRACE(1, "DebuggerCommand::Receive: got cmd of type %d\n", type);

  // not all commands are here, as not all commands need to be sent over wire
  switch (type) {
    case KindOfBreak    :  cmd = DebuggerCommandPtr(new CmdBreak    ()); break;
    case KindOfContinue :  cmd = DebuggerCommandPtr(new CmdContinue ()); break;
    case KindOfDown     :  cmd = DebuggerCommandPtr(new CmdDown     ()); break;
    case KindOfException:  cmd = DebuggerCommandPtr(new CmdException()); break;
    case KindOfFrame    :  cmd = DebuggerCommandPtr(new CmdFrame    ()); break;
    case KindOfGlobal   :  cmd = DebuggerCommandPtr(new CmdGlobal   ()); break;
    case KindOfInfo     :  cmd = DebuggerCommandPtr(new CmdInfo     ()); break;
    case KindOfConstant :  cmd = DebuggerCommandPtr(new CmdConstant ()); break;
    case KindOfList     :  cmd = DebuggerCommandPtr(new CmdList     ()); break;
    case KindOfMachine  :  cmd = DebuggerCommandPtr(new CmdMachine  ()); break;
    case KindOfNext     :  cmd = DebuggerCommandPtr(new CmdNext     ()); break;
    case KindOfOut      :  cmd = DebuggerCommandPtr(new CmdOut      ()); break;
    case KindOfPrint    :  cmd = DebuggerCommandPtr(new CmdPrint    ()); break;
    case KindOfQuit     :  cmd = DebuggerCommandPtr(new CmdQuit     ()); break;
    case KindOfRun      :  cmd = DebuggerCommandPtr(new CmdRun      ()); break;
    case KindOfStep     :  cmd = DebuggerCommandPtr(new CmdStep     ()); break;
    case KindOfThread   :  cmd = DebuggerCommandPtr(new CmdThread   ()); break;
    case KindOfUp       :  cmd = DebuggerCommandPtr(new CmdUp       ()); break;
    case KindOfVariable :  cmd = DebuggerCommandPtr(new CmdVariable ()); break;
    case KindOfWhere    :  cmd = DebuggerCommandPtr(new CmdWhere    ()); break;
    case KindOfUser     :  cmd = DebuggerCommandPtr(new CmdUser     ()); break;
    case KindOfEval     :  cmd = DebuggerCommandPtr(new CmdEval     ()); break;
    case KindOfInterrupt:  cmd = DebuggerCommandPtr(new CmdInterrupt()); break;
    case KindOfSignal   :  cmd = DebuggerCommandPtr(new CmdSignal   ()); break;
    case KindOfShell    :  cmd = DebuggerCommandPtr(new CmdShell    ()); break;

    case KindOfExtended: {
      assert(!clsname.empty());
      cmd = CmdExtended::CreateExtendedCommand(clsname);
      assert(cmd);
      break;
    }

    default:
      assert(false);
      Logger::Error("%s => DebuggerCommand::Receive(): bad cmd type: %d",
                    caller, type);
      return true;
  }
  if (!cmd->recv(thrift)) {
    Logger::Error("%s => DebuggerCommand::Receive(): socket error", caller);
    cmd.reset();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// default handlers

void DebuggerCommand::list(DebuggerClient *client) {
  TRACE(2, "DebuggerCommand::list\n");
}

bool DebuggerCommand::help(DebuggerClient *client) {
  TRACE(2, "DebuggerCommand::help\n");
  assert(false);
  return true;
}

// If the first argument of the command is "help" or "?"
// this displays help text for the command and returns true.
// Otherwise it returns false.
bool DebuggerCommand::onClientImpl(DebuggerClient *client) {
  TRACE(2, "DebuggerCommand::onClientImpl\n");
  if (client->arg(1, "help") || client->arg(1, "?")) {
    return help(client);
  }
  return false;
}

bool DebuggerCommand::onClient(DebuggerClient *client) {
  TRACE(2, "DebuggerCommand::onClient\n");
  bool ret = onClientImpl(client);
  if (client->isApiMode() && !m_incomplete) {
    setClientOutput(client);
  }
  return ret;
}

void DebuggerCommand::setClientOutput(DebuggerClient *client) {
  TRACE(2, "DebuggerCommand::setClientOutput\n");
  // Just default to text
  client->setOutputType(DebuggerClient::OTText);
}

bool DebuggerCommand::onServer(DebuggerProxy *proxy) {
  TRACE(2, "DebuggerCommand::onServer\n");
  assert(false);
  Logger::Error("DebuggerCommand::onServer(): bad cmd type: %d", m_type);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
