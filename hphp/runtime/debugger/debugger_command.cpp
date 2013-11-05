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

#include "hphp/runtime/debugger/debugger_command.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/cmd/all.h"
#include "hphp/util/logger.h"

#define POLLING_SECONDS 1

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

// Resets the buffer, serializes this command into the buffer and then
// flushes the buffer.
// Returns false if an exception occurs during these steps.
bool DebuggerCommand::send(DebuggerThriftBuffer &thrift) {
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

// Always called from send and must implement the subclass specific
// logic for serializing a command to send via Thrift.
void DebuggerCommand::sendImpl(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::sendImpl\n");
  thrift.write((int32_t)m_type);
  thrift.write(m_class);
  thrift.write(m_body);
  thrift.write(m_version);
}

// Always called from recv and must implement the subclass specific
// logic for deserializing a command received via Thrift.
void DebuggerCommand::recvImpl(DebuggerThriftBuffer &thrift) {
  TRACE(5, "DebuggerCommand::recvImpl\n");
  thrift.read(m_body);
  thrift.read(m_version);
}

// Returns false on timeout, true when data has been read even if that data
// didn't form a usable command. Is there is no usable command, cmd is null.
bool DebuggerCommand::Receive(DebuggerThriftBuffer &thrift,
                              DebuggerCommandPtr &cmd, const char *caller) {
  TRACE(5, "DebuggerCommand::Receive\n");
  cmd.reset();

  struct pollfd fds[1];
  fds[0].fd = thrift.getSocket()->fd();
  fds[0].events = POLLIN|POLLERR|POLLHUP;
  int ret = poll(fds, 1, POLLING_SECONDS * 1000);
  if (ret == 0) return false; // Timeout
  if (ret == -1) {
    auto errorNumber = errno; // Just in case TRACE_RB changes errno
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
  string clsname;
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
    case KindOfVariableAsync :
      cmd = DebuggerCommandPtr(new CmdVariable (KindOfVariableAsync)); break;
    case KindOfWhere    :  cmd = DebuggerCommandPtr(new CmdWhere    ()); break;
    case KindOfWhereAsync:
      cmd = DebuggerCommandPtr(new CmdWhere(KindOfWhereAsync)); break;
    case KindOfEval     :  cmd = DebuggerCommandPtr(new CmdEval     ()); break;
    case KindOfInterrupt:  cmd = DebuggerCommandPtr(new CmdInterrupt()); break;
    case KindOfSignal   :  cmd = DebuggerCommandPtr(new CmdSignal   ()); break;
    case KindOfShell    :  cmd = DebuggerCommandPtr(new CmdShell    ()); break;
    case KindOfInternalTesting :
      cmd = DebuggerCommandPtr(new CmdInternalTesting()); break;

    case KindOfExtended: {
      assert(!clsname.empty());
      cmd = CmdExtended::CreateExtendedCommand(clsname);
      assert(cmd);
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
void DebuggerCommand::list(DebuggerClient &client) {
  TRACE(2, "DebuggerCommand::list\n");
}

// The text to display when the debugger client
// processes "help <this command name>".
void DebuggerCommand::help(DebuggerClient &client) {
  TRACE(2, "DebuggerCommand::help\n");
  assert(false);
}

// If the first argument of the command is "help" or "?"
// this displays help text for the command and returns true.
// Otherwise it returns false.
bool DebuggerCommand::displayedHelp(DebuggerClient &client) {
  TRACE(2, "DebuggerCommand::displayedHelp\n");
  if (client.arg(1, "help") || client.arg(1, "?")) {
    help(client);
    return true;
  }
  return false;
}

// Server-side work for a command. Returning false indicates a failure to
// communicate with the client (for commands that do so).
bool DebuggerCommand::onServer(DebuggerProxy &proxy) {
  TRACE(2, "DebuggerCommand::onServer\n");
  assert(false);
  Logger::Error("DebuggerCommand::onServer(): bad cmd type: %d", m_type);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
