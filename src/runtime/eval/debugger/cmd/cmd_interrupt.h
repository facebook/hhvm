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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__
#define __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdInterrupt);
class CmdInterrupt : public DebuggerCommand {
public:
  enum SubType {
    SessionStarted,
    SessionEnded,
    RequestStarted,
    RequestEnded,
    PSPEnded,
    BreakPointReached,
    ExceptionThrown,
  };

  static const char *GetBreakpointName(SubType subtype);

public:
  CmdInterrupt()
      : DebuggerCommand(KindOfInterrupt),
        m_subtype(-1) {
  }
  CmdInterrupt(SubType subtype, const char *file, int line, const char *cls)
      : DebuggerCommand(KindOfInterrupt),
        m_subtype(subtype), m_exceptionClass(cls ? cls : "") {
    m_bpi.file = file ? file : "";
    m_bpi.line = line;
  }

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  bool shouldBreak(const BreakPointInfoMap &bps);

private:
  int32 m_subtype;
  BreakPointInfo m_bpi;
  std::string m_exceptionClass;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_INTERRUPT_H__
