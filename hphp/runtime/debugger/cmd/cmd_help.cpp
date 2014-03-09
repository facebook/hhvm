/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/cmd/cmd_help.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdHelp::HelpAll(DebuggerClient &client) {
  client.helpCmds(
    "Session Commands", "",
    "[m]achine",    "connects to an HHVM server",
    "[t]hread",     "switches between different threads",
    "[s]et",        "various configuration options for hphpd",
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
    "@",            "evaluates one line of PHP code",
    "=",            "prints right-hand-side's value, assigns to $_",
    "${name}=",     "assigns a value to left-hand-side",
    "[<?]php",      "starts input of a block of PHP code",
    "?>",           "ends and evaluates a block a PHP code",
    "[a]bort",      "aborts input of a block of PHP code",
    "[z]end",       "evaluates the last snippet in Zend PHP",

    "Documentation and Source Code", "",
    "[i]nfo",       "displays documentations and other information",
    "[l]ist     *", "displays source codes",
    "[h]elp    **", "displays this help",
    "?",            "displays this help",

    "Shell and Extended Commands", "",
    "! {cmd}",      "executes a shell command",
    "& {cmd}",      "records and replays macros",
    "x {cmd}",      "extended commands",
    "y {cmd}",      "user extended commands",

    nullptr
  );

  client.helpBody("* These commands are replayable by just hitting return.\n"
      "** Type \"help help\" to get more help.");
}

void CmdHelp::HelpStarted(DebuggerClient &client) {
  client.helpTitle("Getting Started with Debugger");

  client.helpBody(
    "1. Quick Overview\n"
    "\n"
    "(1) from A to Z\n"
    "\n"
    "All built-in debugger commands are un-ambiguous with their first "
    "letters. Therefore, a single letter is sufficient to issue the "
    "command.\n"
    "\n"
    "(2) tab, tab, tab\n"
    "\n"
    "Use TAB to auto-complete.\n"
    "\n"
    "(3) input PHP code\n"
    "\n"
    "For single line of PHP code, use \"=\" to print an expression's value, "
    "OR, use \"@\" to execute an expression or statement without printing "
    "return values, OR, start an assignment with \"$\" variable name.\n\n"
    "For multi-line PHP code, type \"<\" then TAB. Now you can type or paste "
    "multiple lines of code. Hit return to start a new line, then TAB. That "
    "will auto-complete \"?>\" to finish the block. Hit return to execute.\n"
    "\n"
    "(4) help\n"
    "\n"
    "Use \"help\" to read more about command details.\n"
    "\n"
    "(5) info and list\n"
    "\n"
    "Use \"info\" and \"list\" commands to read more about source code.\n"
    "\n"
    "(6) readline\n"
    "\n"
    "Debugger is written with readline library, which has rich feature set, "
    "including switching between emacs and vi editing mode. Please read its "
    "[[ http://cnswww.cns.cwru.edu/php/chet/readline/readline.html#SEC1 | "
    "documentation]] for more details."
  );

  client.helpBody(
    "2. Debugging local script\n"
    "\n"
    "The command to run a script normally looks like this,\n"
    "\n"
    "  hhvm myscript.php\n"
    "\n"
    "Simply add \"-m debug\" to run the script in debugger,\n\n"
    "\n"
    "  hhvm -m debug myscript.php\n"
    "\n"
    "Once started, set breakpoints like this,\n"
    "\n"
    "  hphpd> break myscript.php:10\n"
    "  hphpd> break foo()\n"
    "\n"
    "Then let it run, until it hits the breakpoints,\n"
    "\n"
    "  hphpd> run\n"
    "\n"
    "The debugger will highlight current statement or expression that is "
    "just about to evaluate. Sometimes a statement is highlighted first, then "
    "sub-expressions inside the statement are highlighted one after another "
    "while repeating step commands.\n"
    "\n"
    "At any breakpoints, examine variables or evaluate expressions,\n"
    "\n"
    "  hphpd> variable\n"
    "  hphpd> print $a\n"
    "  hphpd> =$a\n"
    "  hphpd> <?php print $a; ?>\n"
    "  hphpd> <?php\n"
    "   ..... print $a;\n"
    "   ..... ?>\n"
    "\n"
    "Optionally, modify variables like this,\n"
    "\n"
    "  hphpd> $a = 10\n"
    "  hphpd> <?php $a = 10; ?>\n"
    "  hphpd> <?php\n"
    "   ..... $a = 10;\n"
    "   ..... ?>\n"
    "\n"
    "Then let it continue, until it hits more breakpoints,\n"
    "\n"
    "  hphpd> continue\n"
    "\n"
    "Finally, quit debugger,\n"
    "\n"
    "  hphpd> quit"
  );

  client.helpBody(
    "3. Debugging sandbox\n"
    "\n"
    "Connect to an HHVM server from command line,\n"
    "\n"
    "  hhvm -m debug -h mymachine.com\n"
    "\n"
    "Or, connect from within debugger,\n"
    "\n"
    "  hphpd> machine connect mymachine.com\n"
    "\n"
    "This will try to attach to a default sandbox on that machine. "
    "\"Attaching\" means it will only debug web requests hitting that "
    "sandbox. To switch to a different sandbox,\n"
    "\n"
    "  mymachine> machine list\n"
    "  mymachine> machine attach 2\n"
    "\n"
    "In remote debugging mode, a breakpoint can be specific about an URL,\n"
    "\n"
    "  mymachine> break myscript.php:10@index.php\n"
    "  mymachine> break foo()@index.php\n"
    "\n"
    "You may connect to more than one machine and breakpoints will be "
    "shared by all of them."
  );

  client.helpBody(
    "4. Understanding dummy sandbox\n"
    "\n"
    "When a web request hits a breakpoint, debugger will run in a "
    "\"Web Request\" thread. Use \"thread\" command to display this "
    "information,\n"
    "\n"
    "  mymachine> thread\n"
    "\n"
    "What will debugger use when there is no web request thread that's "
    "active, but we need to set a breakpoint? We created so-called "
    "\"dummy sandbox\", purely for taking debugger commands when there is "
    "no active web request. When there is no active request, hit Ctrl-C to "
    "break debugger, and use \"thread\" to display dummy sandbox thread's "
    "information.\n"
    "\n"
    "  Ctrl-C\n"
    "  mymachine> thread\n"
    "\n"
    "In dummy sandbox, a PHP file can be pre-loaded, so that we can "
    "\"info\" functions and classes and execute certain code. This file is "
    "specified on server side by\n"
    "\n"
    "  Eval.Debugger.StartupDocument = scripts/startup.php\n"
    "\n"
    "Dummy sandbox will always use currently attached sandbox's PHP files. "
    "When files are modified, simply reload them by\n"
    "\n"
    "  mymachine> continue\n"
    "  Ctrl-C"
  );

  client.helpBody(
    "5. Colors and Configuration\n"
    "\n"
    "By default, it will use emacs colors for dark background. To change "
    "them, run debugger at least once, then look for ~/.hphpd.ini file. If you "
    "want it to look more like vim, you can use:\n"
    "hhvm.color.code.keyword = MAGENTA\n"
    "hhvm.color.code.comment = BLUE\n"
    "hhvm.color.code.string = RED\n"
    "hhvm.color.code.variable = CYAN\n"
    "hhvm.color.code.html = GRAY\n"
    "hhvm.color.code.tag = MAGENTA\n"
    "hhvm.color.code.declaration = WHITE\n"
    "hhvm.color.code.constant = WHITE\n"
    "hhvm.color.code.line_no = GRAY\n"
    "\n"
    "You can also set:\n"
    "hhvm.color.help = BROWN\n"
    "hhvm.color.info = GREEN\n"
    "hhvm.color.output = CYAN\n"
    "hhvm.color.error = RED\n"
    "hhvm.color.item_name = GRAY\n"
    "hhvm.color.highlight_foreground = RED\n"
    "hhvm.color.highlight_background = GRAY\n"
  );
}

void CmdHelp::list(DebuggerClient &client) {
  if (client.argCount() == 0) {
    client.addCompletion(DebuggerClient::GetCommands());
    client.addCompletion("tutorial");
    client.addCompletion("start");
  } else if (client.arg(1, "tutorial")) {
    client.addCompletion("on");
    client.addCompletion("off");
    client.addCompletion("auto");
  }
}

void CmdHelp::help(DebuggerClient &client) {
  client.helpTitle("Help Command");
  client.helpCmds(
    "[h]elp [s]tart", "displays material for getting started",
    "[h]elp [t]utorial on|off|auto", "changing tutorial modes",
    nullptr
  );
  client.helpBody(
    "Please read \"Getting Started\" material with '[h]elp [s]tart' for "
    "first time use to get yourself familiar with basics.\n"
    "\n"
    "Tutorial mode displays extra information when something didn't work "
    "as you expected. \"auto\" mode will display the same information just "
    "once. \"on\" mode will display it as long as you run into the same "
    "situation. \"off\" mode completely turns off all tutorial texts.\n"
    "\n"
    "To get detailed information of a command, type '{cmd} [h]elp' or '{cmd} "
    "?' or 'help {cmd}' or '? {cmd}'."
  );
}

void CmdHelp::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() == 0) {
    HelpAll(client);
   } else if (client.arg(1, "start")) {
    HelpStarted(client);
  } else if (client.arg(1, "tutorial")) {
    if (!processTutorial(client)) {
      help(client);
    }
  } else {
    client.swapHelp();
    if (!client.process()) {
      help(client);
    }
  }
}

bool CmdHelp::processTutorial(DebuggerClient &client) {
  std::string mode = client.argValue(2);
  if (mode == "off") {
    client.setTutorial(-1);
  } else if (mode == "on") {
    client.setTutorial(1);
  } else if (mode == "auto") {
    client.setTutorial(0);
  } else {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
