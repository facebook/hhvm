/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdPrint : DebuggerCommand {
  static std::string FormatResult(const char* format, const Variant& ret);

  CmdPrint(): DebuggerCommand(KindOfPrint) {}

  void list(DebuggerClient&) override;
  void help(DebuggerClient&) override;

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

  Variant processWatch(
    DebuggerClient& client,
    const char* format,
    const std::string& php
  );

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  req::root<Variant> m_ret;
  req::root<String> m_output;
  int m_frame;
  int m_printLevel;
  bool m_bypassAccessCheck{false};
  bool m_isForWatch{false};
  bool m_noBreak{false};

  void processList(DebuggerClient&);
  void processClear(DebuggerClient&);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_PRINT_H_
