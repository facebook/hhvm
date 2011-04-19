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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_BREAK_H__
#define __HPHP_EVAL_DEBUGGER_CMD_BREAK_H__

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdBreak);
class CmdBreak : public DebuggerCommand {
public:
  CmdBreak() : DebuggerCommand(KindOfBreak), m_breakpoints(NULL) {}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
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

  bool processList(DebuggerClient *client);
  bool processUpdate(DebuggerClient *client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_BREAK_H__
