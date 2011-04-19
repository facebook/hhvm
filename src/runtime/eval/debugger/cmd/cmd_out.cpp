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

#include <runtime/eval/debugger/cmd/cmd_out.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdOut::help(DebuggerClient *client) {
  client->helpTitle("Out Command");
  client->helpCmds(
    "[o]ut {count=1}", "steps out function calls",
    NULL
  );
  client->helpBody(
    "Use this command at break to step out function calls. Specify a "
    "count to step out more than one level of function calls."
  );
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
