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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdBreak);
class CmdBreak : public DebuggerCommand {
public:
  CmdBreak() : DebuggerCommand(KindOfBreak), m_breakpoints(nullptr) {}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
  virtual void setClientOutput(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  static bool SendClientBreakpointListToServer(DebuggerClient *client);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);
  bool addToBreakpointListAndUpdateServer(
      DebuggerClient *client, BreakPointInfoPtr bpi, int index);

private:
  // Either points to the breakpoint collection of a debugger client
  // or points to m_bps. In the former case the client frees the
  // memory. In the latter case the destructor for CmdBreak frees
  // the memory. (The base class destructor is only invoked for instances
  // that point to the collection in the client.)
  BreakPointInfoPtrVec *m_breakpoints;

  // Holds the breakpoint collection of a CmdBreak received via Thrift.
  BreakPointInfoPtrVec m_bps;

  bool updateServer(DebuggerClient *client);
  bool processList(DebuggerClient *client);
  bool processStatusChange(DebuggerClient *client);

  bool hasStatusChangeArg(DebuggerClient *client);
  bool hasEnableArg(DebuggerClient *client);
  bool hasDisableArg(DebuggerClient *client);
  bool hasClearArg(DebuggerClient *client);
  bool hasToggleArg(DebuggerClient *client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_
