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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdList);
class CmdList : public DebuggerCommand {
public:
  CmdList() : DebuggerCommand(KindOfList) {}

  static Variant GetSourceFile(DebuggerClient *client,
                               const std::string &file);

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  std::string m_file;
  int32_t m_line1;
  int32_t m_line2;
  Variant m_code;

  bool listCurrent(DebuggerClient *client, int &line,
                   int &charFocus0, int &lineFocus1,
                   int &charFocus1);
  bool listFileRange(DebuggerClient *client, int line,
                     int charFocus0, int lineFocus1,
                     int charFocus1);
  bool listFunctionOrClass(DebuggerClient *client);

};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_
