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
  client->helpCmds(
    "Session Commands", "",
    "[m]achine",    "connects to an HPHPi server",
    "[t]hread",     "switches between different threads",
    "[q]uit",       "quits debugger",

    "Program Flow Control", "",
    "[b]reak",      "sets/clears/displays breakpoints",
    "[e]xception",  "catches/clears exceptions",
    "[r]un",        "starts over a program",
    "<Ctrl-C>",     "breaks program execution",
    "[c]ontinue *", "continues program execution",
    "[s]tep     *", "steps into a function call or an expression",
    "[n]ext     *", "steps over a function call or a line",
    "[o]ut      *", "steps out a function call",
    "[j]ump",       "jumps to specified line of code for execution",

    "Display Commands", "",
    "[p]rint",      "prints a variable's value",
    "[w]here",      "displays stacktrace",
    "[u]p",         "goes up by frame(s)",
    "[d]own",       "goes down by frame(s)",
    "[f]rame",      "goes to a frame",
    "[v]ariable",   "lists all local variables",
    "[g]lobal",     "lists all global variables",
    "[k]onstant",   "lists all constants",

    "Evaluation Commands", "",
    "=",            "prints right-hand-side's value",
    "${name}=",     "assigns a value to left-hand-side",
    "[<?]php",      "starts input of a block of PHP code",
    "?>",           "ends and evaluates a block a PHP code",
    "[a]bort",      "aborts input of a block of PHP code",
    "[z]end",       "evaluates the last snippet in Zend PHP",

    "Documentation and Source Code", "",
    "[i]nfo",       "displays documentations and other information",
    "[l]ist     *", "displays source codes",
    "[h]elp",       "displays this help",
    "?",            "displays this help",

    "Shell and Extended Commands", "",
    "! {cmd}",      "executes a shell command",
    "& {cmd}",      "records and replays macros",
    "x {cmd}",      "extended commands",
    "y {cmd}",      "user extended commands",

    NULL
  );

  client->helpBody("* These commands are replayable by just hitting return.");
}

void CmdHelp::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    client->addCompletion(DebuggerClient::GetCommands());
    client->addCompletion("tutorial");
  } else if (client->arg(1, "tutorial")) {
    client->addCompletion("on");
    client->addCompletion("off");
    client->addCompletion("auto");
  }
}

bool CmdHelp::help(DebuggerClient *client) {
  client->helpTitle("Help Command");
  client->helpCmds(
    "[h]elp [t]utorial on|off|auto", "changing tutorial modes",
    NULL
  );
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
    if (!processTutorial(client)) {
      return help(client);
    }
  }

  client->swapHelp();
  if (client->process()) {
    return true;
  }

  return help(client);
}

bool CmdHelp::processTutorial(DebuggerClient *client) {
  string mode = client->argValue(2);
  if (mode == "off") {
    client->setTutorial(-1);
  } else if (mode == "on") {
    client->setTutorial(1);
  } else if (mode == "auto") {
    client->setTutorial(0);
  } else {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
