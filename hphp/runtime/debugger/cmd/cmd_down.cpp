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

#include "hphp/runtime/debugger/cmd/cmd_down.h"
#include "hphp/runtime/debugger/cmd/cmd_up.h"
#include "hphp/runtime/debugger/cmd/cmd_where.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdDown::help(DebuggerClient &client) {
  client.helpTitle("Down Command");
  client.helpCmds(
    "[d]own {num=1}", "moves to inner frames (callees) on stacktrace",
    nullptr
  );
  client.helpBody(
    "Use this command to walk down on stacktrace to find out inner callees of "
    "current frame. By default it moves down by one level. Specify a number "
    "to move down several levels a time."
  );
}

void CmdDown::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() > 1) {
    help(client);
  } else {
    if (client.isStackTraceAsync()) {
      CmdWhere(KindOfWhereAsync).fetchStackTrace(client);
    } else {
      CmdWhere().fetchStackTrace(client);
    }

    client.moveToFrame(client.getFrame() - CmdUp::ParseNumber(client));
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
