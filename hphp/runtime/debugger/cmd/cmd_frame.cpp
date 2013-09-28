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

#include "hphp/runtime/debugger/cmd/cmd_frame.h"
#include "hphp/runtime/debugger/cmd/cmd_up.h"
#include "hphp/runtime/debugger/cmd/cmd_where.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdFrame::help(DebuggerClient &client) {
  client.helpTitle("Frame Command");
  client.helpCmds(
    "[f]rame {index}",  "jumps to one particular frame",
    nullptr
  );
  client.helpBody(
    "Use '[w]here' command to find out the frame number. Use 'f 0' to jump "
    "back to the most recent frame or the innermost frame. Use 'f 999' or "
    "some big number to jump to the outermost frame."
  );
}

void CmdFrame::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() != 1) {
    help(client);
  } else {
    CmdWhere().fetchStackTrace(client);
    client.moveToFrame(CmdUp::ParseNumber(client));
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
