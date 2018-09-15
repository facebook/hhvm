/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/debugger/debugger_command.h"

#include <folly/portability/Sockets.h>

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/cmd/all.h"
#include "hphp/util/logger.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

// Resets the buffer, serializes this command into the buffer and then
// flushes the buffer.
// Returns false if an exception occurs during these steps.
bool DebuggerCommand::send(DebuggerThriftBuffer& thrift) {
  TRACE(5, "DebuggerCommand::send cmd of type %d\n", m_type);
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

bool DebuggerCommand::recv(DebuggerThriftBuffer& thrift) {
  TRACE(5, "DebuggerCommand::recv\n");
  try {
    recvImpl(thrift);
  } catch (...) {
    Logger::Error("DebuggerCommand::recv(): a socket error has happened");
    return false;
  }
  return true;
}

// Always called from send and must implement the subclass specific
// logic for serializing a command to send via Thrift.
void DebuggerCommand::sendImpl(DebuggerThriftBuffer& thrift) {
  TRACE(5, "DebuggerCommand::sendImpl\n");
  thrift.write((int32_t)m_type);
  thrift.write(m_class);
  thrift.write(m_body);
  thrift.write(m_version);
}

// Always called from recv and must implement the subclass specific
// logic for deserializing a command received via Thrift.
void DebuggerCommand::recvImpl(DebuggerThriftBuffer& thrift) {
  TRACE(5, "DebuggerCommand::recvImpl\n");
  thrift.read(m_body);
  thrift.read(m_version);
}

// Returns false on timeout, true when data has been read even if that data
// didn't form a usable command. Is there is no usable command, cmd is null.
bool DebuggerCommand::Receive(DebuggerThriftBuffer& thrift,
                              DebuggerCommandPtr& cmd, const char* caller) {
  TRACE(5, "DebuggerCommand::Receive\n");
  cmd.reset();

  constexpr auto POLLING_SECONDS = 1;
  struct pollfd fds[1];
  fds[0].fd = thrift.getSocket()->fd();
  fds[0].events = POLLIN|POLLERR|POLLHUP;
  auto const ret = poll(fds, 1, POLLING_SECONDS * 1000);

  // Timeout.
  if (ret == 0) return false;

  if (ret == -1) {
    auto const errorNumber = errno; // Just in case TRACE_RB changes errno
    TRACE_RB(1, "DebuggerCommand::Receive: error %d\n", errorNumber);
    return errorNumber != EINTR; // Treat signals as timeouts
  }

  // If we don't have any data to read (POLLIN) then we're done. If we
  // do have data we'll attempt to read and decode it below, even if
  // there are other error bits set.
  if (!(fds[0].revents & POLLIN)) {
    TRACE_RB(1, "DebuggerCommand::Receive: revents %d\n", fds[0].revents);
    return true;
  }

  int32_t type;
  std::string clsname;
  try {
    thrift.reset(true);
    thrift.read(type);
    thrift.read(clsname);
  } catch (...) {
    // Note: this error case is difficult to test. But, it's exactly the same
    // as the error noted below. Make sure to keep handling of both of these
    // errors in sync.
    TRACE_RB(1, "%s: socket error receiving command", caller);
    return true;
  }

  TRACE(1, "DebuggerCommand::Receive: got cmd of type %d\n", type);

  // Not all commands are here, as not all commands need to be sent over wire.
  switch (type) {
    case KindOfBreak    : cmd = std::make_shared<CmdBreak>(); break;
    case KindOfContinue : cmd = std::make_shared<CmdContinue>(); break;
    case KindOfDown     : cmd = std::make_shared<CmdDown>(); break;
    case KindOfException: cmd = std::make_shared<CmdException>(); break;
    case KindOfFrame    : cmd = std::make_shared<CmdFrame>(); break;
    case KindOfGlobal   : cmd = std::make_shared<CmdGlobal>(); break;
    case KindOfInfo     : cmd = std::make_shared<CmdInfo>(); break;
    case KindOfConstant : cmd = std::make_shared<CmdConstant>(); break;
    case KindOfList     : cmd = std::make_shared<CmdList>(); break;
    case KindOfMachine  : cmd = std::make_shared<CmdMachine>(); break;
    case KindOfNext     : cmd = std::make_shared<CmdNext>(); break;
    case KindOfOut      : cmd = std::make_shared<CmdOut>(); break;
    case KindOfPrint    : cmd = std::make_shared<CmdPrint>(); break;
    case KindOfQuit     : cmd = std::make_shared<CmdQuit>(); break;
    case KindOfRun      : cmd = std::make_shared<CmdRun>(); break;
    case KindOfStep     : cmd = std::make_shared<CmdStep>(); break;
    case KindOfThread   : cmd = std::make_shared<CmdThread>(); break;
    case KindOfUp       : cmd = std::make_shared<CmdUp>(); break;
    case KindOfVariable : cmd = std::make_shared<CmdVariable>(); break;
    case KindOfWhere    : cmd = std::make_shared<CmdWhere>(); break;
    case KindOfEval     : cmd = std::make_shared<CmdEval>(); break;
    case KindOfInterrupt: cmd = std::make_shared<CmdInterrupt>(); break;
    case KindOfSignal   : cmd = std::make_shared<CmdSignal>(); break;
    case KindOfAuth     : cmd = std::make_shared<CmdAuth>(); break;
    case KindOfShell    : cmd = std::make_shared<CmdShell>(); break;
    case KindOfInternalTesting :
      cmd = std::make_shared<CmdInternalTesting>();
      break;

    case KindOfExtended: {
      assertx(!clsname.empty());
      cmd = CmdExtended::CreateExtendedCommand(clsname);
      assertx(cmd);
      break;
    }

    default:
      TRACE_RB(1, "%s: received bad cmd type: %d", caller, type);
      cmd.reset();
      return true;
  }
  if (!cmd->recv(thrift)) {
    // Note: this error case is easily tested, and we have a test for it. But
    // the error case noted above is quite difficult to test. Keep these two
    // in sync.
    TRACE_RB(1, "%s: socket error receiving command", caller);
    cmd.reset();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// default handlers

// Informs the client of all argument strings that may follow this command
// name. Used for auto completion. The client uses the prefix of the argument
// following the command name to narrow down the list displayed to the user.
void DebuggerCommand::list(DebuggerClient& /*client*/) {
  TRACE(2, "DebuggerCommand::list\n");
}

// The text to display when the debugger client
// processes "help <this command name>".
void DebuggerCommand::help(DebuggerClient& /*client*/) {
  TRACE(2, "DebuggerCommand::help\n");
  not_reached();
}

// If the first argument of the command is "help" or "?"
// this displays help text for the command and returns true.
// Otherwise it returns false.
bool DebuggerCommand::displayedHelp(DebuggerClient& client) {
  TRACE(2, "DebuggerCommand::displayedHelp\n");
  if (client.arg(1, "help") || client.arg(1, "?")) {
    help(client);
    return true;
  }
  return false;
}

// Server-side work for a command. Returning false indicates a failure to
// communicate with the client (for commands that do so).
bool DebuggerCommand::onServer(DebuggerProxy& /*proxy*/) {
  TRACE(2, "DebuggerCommand::onServer\n");
  assertx(false);
  Logger::Error("DebuggerCommand::onServer(): bad cmd type: %d", m_type);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
