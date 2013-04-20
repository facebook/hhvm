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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_PRINT_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_PRINT_H_

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdPrint);
class CmdPrint : public DebuggerCommand {
public:
  static const char *Formats[];
  static std::string FormatResult(const char *format, CVarRef ret);

public:
  CmdPrint() : DebuggerCommand(KindOfPrint), m_bypassAccessCheck(false),
               m_isForWatch(false), m_noBreak(false) {}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
  virtual void setClientOutput(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);
  virtual bool onServerVM(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  Variant processWatch(DebuggerClient *client, const char *format,
                    const std::string &php);

  virtual void handleReply(DebuggerClient *client);

private:
  Variant m_ret;
  String m_output;
  int m_frame;
  bool m_bypassAccessCheck;
  int m_printLevel;
  bool m_isForWatch;
  bool m_noBreak;

  bool processList(DebuggerClient *client);
  bool processClear(DebuggerClient *client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_PRINT_H_
