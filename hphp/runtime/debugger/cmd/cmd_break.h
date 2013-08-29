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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdBreak);
class CmdBreak : public DebuggerCommand {
public:
  CmdBreak() : DebuggerCommand(KindOfBreak), m_breakpoints(nullptr) {
    m_version = 1;
  }

  // Informs the client of all strings that may follow a break command.
  // Used for auto completion. The client uses the prefix of the argument
  // following the command to narrow down the list displayed to the user.
  virtual void list(DebuggerClient &client);

  // The text to display when the debugger client processes "help break".
  virtual void help(DebuggerClient &client);

  // Updates the breakpoint list in the proxy with the new list
  // received from the client. Then sends the command back to the
  // client as confirmation. Returns false if the confirmation message
  // send failed.
  virtual bool onServer(DebuggerProxy &proxy);

  // Creates a new CmdBreak instance, sets its breakpoints to the client's
  // list, sends the command to the server and waits for a response.
  static void SendClientBreakpointListToServer(DebuggerClient &client);

  // Carries out the Break command. This always involves an action on the
  // client and usually, but not always, involves the server by sending
  // this command to the server and waiting for its response.
  virtual void onClient(DebuggerClient &client);

protected:
  // Serializes this command into the given Thrift buffer.
  virtual void sendImpl(DebuggerThriftBuffer &thrift);

  // Deserializes a CmdBreak from the given Thrift buffer.
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  // Adds conditional or watch clause to the breakpoint info if needed.
  // Then adds the breakpoint to client's list and sends this command
  // to the server so that it too can update it's list.
  // Returns false if the breakpoint is not well formed.
  bool addToBreakpointListAndUpdateServer(
      DebuggerClient &client, BreakPointInfoPtr bpi, int index);

private:
  // Either points to the breakpoint collection of a debugger client
  // or points to m_bps. In the former case the client frees the
  // memory. In the latter case the destructor for CmdBreak frees
  // the memory. (The base class destructor is only invoked for instances
  // that point to the collection in the client.)
  BreakPointInfoPtrVec *m_breakpoints;

  // Holds the breakpoint collection of a CmdBreak received via Thrift.
  BreakPointInfoPtrVec m_bps;

  // Uses the client to send this command to the server, which
  // will update its breakpoint list with the one in this command.
  // The client will block until the server echoes
  // this command back to it. The echoed command is discarded.
  void updateServer(DebuggerClient &client);

  // Carries out the "break list" command.
  void processList(DebuggerClient &client);

  // Carries out commands that change the status of a breakpoint.
  void processStatusChange(DebuggerClient &client);

  // Returns true if the last command parsed by the client has
  // an argument that changes the status of a breakpoint.
  // I.e. clear, enable, disable or toggle.
  bool hasStatusChangeArg(DebuggerClient &client);

  // Returns true if the last command parsed by the client has
  // the string "enable" in its first argument position.
  bool hasEnableArg(DebuggerClient &client);

  // Returns true if the last command parsed by the client has
  // the string "disable" in its first argument position.
  bool hasDisableArg(DebuggerClient &client);

  // Returns true if the last command parsed by the client has
  // the string "clear" in its first argument position.
  bool hasClearArg(DebuggerClient &client);

  // Returns true if the last command parsed by the client has
  // the string "toggle" in its first argument position.
  bool hasToggleArg(DebuggerClient &client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_
