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

#include <runtime/eval/debugger/cmd/cmd_up.h>
#include <runtime/eval/debugger/cmd/cmd_where.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdUp::help(DebuggerClient *client) {
  client->helpTitle("Up Command");
  client->helpCmds(
    "[u]p {num=1}", "moves to outer frames (callers) on stacktrace",
    NULL
  );
  client->helpBody(
    "Use this command to walk up on stacktrace to find out outer callers of "
    "current frame. By default it moves up by one level. Specify a number "
    "to move up several levels a time."
  );
  return true;
}

int CmdUp::ParseNumber(DebuggerClient *client) {
  if (client->argCount() == 1) {
    string snum = client->argValue(1);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client->error("Please specify a number.");
      client->tutorial(
        "Run '[w]here' command to see the entire stacktrace."
      );
      return true;
    }
    return atoi(snum.c_str());
  }
  return 1;
}

bool CmdUp::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() > 1) {
    return help(client);
  }

  CmdWhere().fetchStackTrace(client);
  client->moveToFrame(client->getFrame() + ParseNumber(client));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
