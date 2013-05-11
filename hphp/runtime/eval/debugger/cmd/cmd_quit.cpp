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

#include <runtime/eval/debugger/cmd/cmd_quit.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdQuit::help(DebuggerClient *client) {
  client->helpTitle("Quit Command");
  client->helpCmds(
    "[q]uit", "quits this program",
    nullptr
  );
  client->helpBody(
    "After you type this command, you will not see me anymore."
  );
  return true;
}

bool CmdQuit::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() == 0) {
    client->sendToServer(this);
    client->quit();
    return true;
  }

  return help(client);
}

///////////////////////////////////////////////////////////////////////////////
}}
