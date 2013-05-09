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

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  /**
   * Sync breakpoints from client to server.
   */
  bool update(DebuggerClient *client);

protected:
  bool validate(DebuggerClient *client, BreakPointInfoPtr bpi, int index);

private:
  BreakPointInfoPtrVec *m_breakpoints;
  BreakPointInfoPtrVec m_bps;

  bool updateImpl(DebuggerClient *client);
  bool processList(DebuggerClient *client);
  bool processUpdate(DebuggerClient *client);

  bool hasUpdateArg(DebuggerClient *client);
  bool hasEnableArg(DebuggerClient *client);
  bool hasDisableArg(DebuggerClient *client);
  bool hasClearArg(DebuggerClient *client);
  bool hasToggleArg(DebuggerClient *client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_BREAK_H_
