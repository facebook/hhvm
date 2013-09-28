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

#include "hphp/runtime/debugger/cmd/cmd_zend.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdZend::help(DebuggerClient &client) {
  client.helpTitle("Zend Command");
  client.helpCmds(
    "[z]end", "running the most recent code snippet in Zend PHP",
    nullptr
  );
  client.helpBody(
    "This is mainly for comparing results from PHP vs. HipHop. After you type "
    "in some PHP code, it will be evaluated immediately in HipHop. Then you "
    "can type '[z]end' command to re-run the same script with your "
    "system-default PHP. Please note that only the most recent block of code "
    "you manually typed in is evaluated, not any earlier ones, nor the ones "
    "from a PHP file."
  );
}

void CmdZend::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  if (client.argCount() == 0) {
    const std::string &code = client.getCode();
    if (!code.empty()) {
      const std::string zendExe = client.getZendExecutable();
      client.info("Executing last PHP block with \"%s\"...", zendExe.c_str());
      string out;
      Process::Exec(zendExe.c_str(), nullptr, code.c_str(), out, &out, true);
      client.print(out);
      return;
    }
  }
  help(client);
}

///////////////////////////////////////////////////////////////////////////////
}}
