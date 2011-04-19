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

#include <runtime/eval/debugger/cmd/cmd_zend.h>
#include <util/process.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdZend::help(DebuggerClient *client) {
  client->helpTitle("Zend Command");
  client->helpCmds(
    "[z]end", "running the most recent code snippet in Zend PHP",
    NULL
  );
  client->helpBody(
    "This is mainly for comparing results from PHP vs. HipHop. After you type "
    "in some PHP code, it will be evaluated immediately in HipHop. Then you "
    "can type '[z]end' command to re-run the same script in Zend PHP. Please "
    "note that only the most recent block of code you manually typed in was "
    "evaluated, not any earlier ones, nor the ones from a PHP file."
  );
  return true;
}

bool CmdZend::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() == 0) {
    const std::string &code = client->getCode();
    string out;
    Process::Exec("php", NULL, code.c_str(), out, &out, true);
    client->print(out);
    return true;
  }

  return help(client);
}

///////////////////////////////////////////////////////////////////////////////
}}
