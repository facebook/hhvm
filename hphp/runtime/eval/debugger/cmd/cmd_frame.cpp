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

#include "hphp/runtime/eval/debugger/cmd/cmd_frame.h"
#include "hphp/runtime/eval/debugger/cmd/cmd_up.h"
#include "hphp/runtime/eval/debugger/cmd/cmd_where.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

bool CmdFrame::help(DebuggerClient *client) {
  client->helpTitle("Frame Command");
  client->helpCmds(
    "[f]rame {index}",  "jumps to one particular frame",
    nullptr
  );
  client->helpBody(
    "Use '[w]here' command to find out the frame number. Use 'f 0' to jump "
    "back to the most recent frame or the innermost frame. Use 'f 999' or "
    "some big number to jump to the outermost frame."
  );
  return true;
}

bool CmdFrame::onClientImpl(DebuggerClient *client) {
  if (DebuggerCommand::onClientImpl(client)) return true;
  if (client->argCount() != 1) {
    return help(client);
  }

  CmdWhere().fetchStackTrace(client);
  client->moveToFrame(CmdUp::ParseNumber(client));
  return true;
}

void CmdFrame::setClientOutput(DebuggerClient *client) {
  client->setOutputType(DebuggerClient::OTStacktrace);
}

///////////////////////////////////////////////////////////////////////////////
}}
