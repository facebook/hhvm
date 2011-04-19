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

#include <runtime/eval/debugger/cmd/cmd_abort.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdAbort::help(DebuggerClient *client) {
  client->helpTitle("Abort Command");
  client->helpCmds(
    "[a]bort", "aborts current PHP code input",
    NULL
  );
  client->helpBody(
    "You will have to type this command on a new line, while you're typing "
    "ad-hoc PHP code to evaluate. In other words, it only works when you see "
    "continuation prompt like \">>>>\"."
  );
  return true;
}

bool CmdAbort::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() == 0) {
    client->tutorial(
      "This command only works when you started typing ad-hoc PHP code with "
      "\"<?\" then decided to abort the input. So it only makes sense when "
      "you see continuation prompt like \">>>>\"."
    );
    return true;
  }

  return help(client);
}

///////////////////////////////////////////////////////////////////////////////
}}
