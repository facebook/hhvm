/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/cmd/cmd_help.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdHelp::HelpAll(DebuggerClient *client) {
  client->helpTitle("Session Commands");
  client->help("[m]achine      connects to an HPHPi server");
  client->help("[t]hread       switches between different threads");
  client->help("[q]uit         quits debugger");
  client->help("<Ctrl-D>       quits debugger");

  client->helpTitle("Program Flow Control");
  client->help("[b]reak        sets/clears/displays breakpoints");
  client->help("[e]xception    catches/clears exceptions");
  client->help("[r]un          starts over a program");
  client->help("<Ctrl-C>       breaks program execution");
  client->help("[c]ontinue *   continues program execution");
  client->help("[s]tep     *   steps into a function call or an expression");
  client->help("[n]ext     *   steps over a function call or a line");
  client->help("[o]ut      *   steps out a function call");
  client->help("[j]ump         jumps to specified line of code for execution");

  client->helpTitle("Display Commands");
  client->help("[p]rint        prints a variable's value");
  client->help("[w]here        displays stacktrace");
  client->help("[u]p           goes up by frame(s)");
  client->help("[d]own         goes down by frame(s)");
  client->help("[f]rame        goes to a frame");
  client->help("[v]ariable     lists all local variables");
  client->help("[g]lobal       lists all global variables");
  client->help("[k]onstant     lists all constants");

  client->helpTitle("Evaluation Commands");
  client->help("=              prints right-hand-side's value");
  client->help("${name}=       assigns a value to left-hand-side");
  client->help("[<?]php        starts input of a block of PHP code");
  client->help("?>             ends and evaluates a block a PHP code");
  client->help("[a]bort        aborts input of a block of PHP code");
  client->help("[z]end         evaluates the last snippet in Zend PHP");

  client->helpTitle("Documentation and Source Code");
  client->help("[i]nfo         displays documentations and other information");
  client->help("[l]ist         displays source codes");
  client->help("[h]elp         displays this help");
  client->help("?              displays this help");

  client->helpTitle("Shell and Extended Commands");
  client->help("! {cmd}        executes a shell command");
  client->help("x {cmd}        extended commands");
  client->help("y {cmd}        user extended commands");

  client->helpBody("* These commands are replayable by just hitting return.");
}

bool CmdHelp::help(DebuggerClient *client) {
  client->helpTitle("Help Command");
  client->help("[h]elp [t]utorial on|off|auto      changing tutorial modes");
  client->helpBody(
    "Tutorial mode displays extra information when something didn't work "
    "as you expected. \"auto\" mode will display the same information just "
    "once. \"on\" mode will display it as long as you run into the same "
    "situation. \"off\" mode completely turns off all tutorial texts.\n"
    "\n"
    "To get detailed information of a command, type '{cmd} [h]elp' or '{cmd} "
    "?' or 'help {cmd}' or '? {cmd}'."
  );
  return true;
}

bool CmdHelp::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() == 0) {
    HelpAll(client);
    return true;
  }

  if (client->arg(1, "tutorial")) {
    return processTutorial(client);
  }

  client->swapHelp();
  if (client->process()) {
    return true;
  }

  return help(client);
}

bool CmdHelp::processTutorial(DebuggerClient *client) {
  client->error("Not implemented yet. It's always on for now.");
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
