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

#include "hphp/runtime/debugger/cmd/cmd_macro.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdMacro::list(DebuggerClient &client) {
  if (client.argCount() == 0) {
    static const char *keywords[] =
      { "start", "end", "replay", "list", "clear", nullptr};
    client.addCompletion(keywords);
  }
}

void CmdMacro::help(DebuggerClient &client) {
  client.helpTitle("Macro Command");
  client.helpCmds(
    "& [s]tart",            "starts recording of default macro",
    "& [s]tart {name}",     "starts recording of a named macro",
    "& [e]nd",              "stops and saves recorded macro",
    "& [r]eplay",           "replays default macro",
    "& [r]eplay {name}",    "replays a named macro",
    "& [l]ist",             "lists all macros",
    "& [c]lear {index}",    "deletes a macro",
    nullptr
  );
  client.helpBody(
    "Macro command allows you to record a series of debugger command, so "
    "you can replay later by its name. When name is not specified, it will "
    "use \"default\" as the name.\n"
    "\n"
    "There is also a special macro \"startup\" that will be replayed "
    "every time when debugger is just started. Use startup macro to load "
    "certain PHP files or perform certain debugging environment setup.\n"
    "\n"
    "The space between & and command is not needed. '&s' works as well."
  );
}

void CmdMacro::processList(DebuggerClient &client) {
  const MacroPtrVec &macros = client.getMacros();
  for (unsigned int i = 0; i < macros.size(); i++) {
    MacroPtr macro = macros[i];
    client.output("%4d  %s", i + 1, macro->m_name.c_str());
    client.print("%s", macro->desc("     > ").c_str());
  }
}

void CmdMacro::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() == 0) {
    help(client);
    return;
  }

  if (client.arg(1, "start")) {
    client.startMacro(client.argValue(2));
  } else if (client.arg(1, "end")) {
    client.endMacro();
  } else if (client.arg(1, "replay")) {
    if (!client.playMacro(client.argValue(2))) {
      client.error("Unable to find specified macro.");
      processList(client);
    }
  } else if (client.arg(1, "list")) {
    processList(client);
  } else if (client.arg(1, "clear")) {
    string snum = client.argValue(2);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client.error("'& [c]lear' needs an {index} argument.");
      client.tutorial(
        "You will have to run '& [l]ist' first to see a list of valid "
        "numbers or indices to specify."
      );
      return;
    }

    int num = atoi(snum.c_str());
    if (!client.deleteMacro(num)) {
      client.error("\"%s\" is not a valid macro index. Choose one from "
                    "this list:", snum.c_str());
      processList(client);
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
