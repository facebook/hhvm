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

#include "hphp/runtime/debugger/cmd/cmd_config.h"
#include "hphp/runtime/base/array-init.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdConfig::help(DebuggerClient &client) {
  client.helpTitle("Set Command");
  client.helpCmds(
    "set bac on/off","on makes debugger bypass access checks on class members",
    "set lf path/off","turn logging on and specify log file",
    "set pl level","if level > 0, only print out object trees to that depth",
    "set cc count","display at most count characters when doing = command",
    "set ss on/off",
      "on makes the debugger take small steps (not entire lines)",
    "set sa on/off","on makes where command display argument values",
    "set mcl limit","display at most limit source lines at breakpoints",
    nullptr);
  client.helpBody(
    "Use this command to change default settings. "
    "The new values are persisted into "
    "the configuration file that normally can be found at ~/.hphpd.ini. "
    "Level, count and limit can be <= 0, in which case they are unlimited."
  );
}

void CmdConfig::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() == 0) {
    listVars(client);
    return;
  }
  std::string var = client.argValue(1);
  if (var == "help" || client.argCount() < 2) {
    help(client);
    return;
  }

  std::string value = client.argValue(2);
  if (var == "BypassAccessCheck" || var == "bac") {
    if (value == "on") {
      client.print("BypassAccessCheck(bac) set to on.\n"
                    "All code executed from debugger is bypassing "
                    "access check!");
      client.setDebuggerClientBypassCheck(true);
    } else if (value == "off") {
      client.print("BypassAccessCheck(bac) set to off");
      client.setDebuggerClientBypassCheck(false);
    } else {
      help(client);
    }
    return;
  }
  if (var == "LogFile" || var == "lf") {
    // Close the current log file handler
    FILE *f = client.getLogFileHandler();
    if (f != nullptr) {
      fclose(f);
      client.setLogFileHandler(nullptr);
    }

    if (value == "off") {
      value = "";
    } else {
      // Try open the log file and error if it's not working
      f = fopen(value.c_str(), "a");
      if (f == nullptr) {
        client.error("Cannot open log file '%s'",
          value.c_str());
        value = "";
        client.setLogFileHandler(nullptr);
      } else {
        client.setLogFileHandler(f);
      }
    }
    client.print("LogFile(lf) is set to %s", value == "" ? "off"
                                                          : value.c_str());
    client.setLogFile(value);
    return;
  }
  if (var == "PrintLevel" || var == "pl") {
    int pl = strtol(value.c_str(), nullptr, 10);
    if (pl > 0 && pl < DebuggerClient::MinPrintLevel) {
      client.error("%d is invalid for PrintLevel(pl)", pl);
      return;
    }
    client.setDebuggerClientPrintLevel(pl);
    client.print("PrintLevel(pl) is set to %d", pl);
    return;
  }
  if (var == "ShortPrintCharCount" || var == "cc") {
    int cc = strtol(value.c_str(), nullptr, 10);
    if (cc < -1) {
      client.error("%d is invalid for ShortPrintCharCount(cc)", cc);
    } else {
      client.setDebuggerClientShortPrintCharCount(cc);
      client.print("ShortPrintCharCount(cc) is set to %d", cc);
    }
    return;
  }
  if (var == "SmallStep" || var == "ss") {
    if (value == "on") {
      client.print("SmallStep(ss) set to on.\n");
      client.setDebuggerClientSmallStep(true);
    } else if (value == "off") {
      client.print("SmallStep(ss) set to off");
      client.setDebuggerClientSmallStep(false);
    } else {
      help(client);
    }
    return;
  }
  if (var == "StackArgs" || var == "sa") {
    if (value == "on") {
      client.print("StackArgs(sa) set to on.\n");
      client.setDebuggerClientStackArgs(true);
    } else if (value == "off") {
      client.print("StackArgs(sa) set to off");
      client.setDebuggerClientStackArgs(false);
    } else {
      help(client);
    }
    return;
  }
  if (var == "MaxCodeLines" || var == "mcl") {
    // MaxCodeLines: a useful configuration variable for emacs/hphpd-integration
    // to prevent or limit code spew after each breakpoint is hit (since emacs
    // hphpd-mode already loads the source file into a buffer and displays a
    // pointer to the current line).
    int mcl = strtol(value.c_str(), nullptr, 10);
    if (mcl < -1) {
      client.error("%d is invalid for MaxCodeLines(mcl)", mcl);
    } else {
      client.setDebuggerClientMaxCodeLines(mcl);
      client.print("MaxCodeLines(mcl) is set to %d", mcl);
    }
    return;
  }

  listVars(client);
}

void CmdConfig::listVars(DebuggerClient &client) {
  std::string LogFile = client.getLogFile();
  client.print("LogFile(lf) %s", LogFile == "" ?
                                    "off" : LogFile.c_str());
  client.print("BypassAccessCheck(bac) %s",
                client.getDebuggerClientBypassCheck() ? "on" : "off");
  client.print("PrintLevel(pl) %d", client.getDebuggerClientPrintLevel());
  client.print("ShortPrintCharCount(cc) %d",
               client.getDebuggerClientShortPrintCharCount());
  client.print("SmallStep(ss) %s",
                client.getDebuggerClientSmallStep() ? "on" : "off");
  client.print("StackArgs(sa) %s",
                client.getDebuggerClientStackArgs() ? "on" : "off");
  client.print("MaxCodeLines(mcl) %d",
                client.getDebuggerClientMaxCodeLines());
}

///////////////////////////////////////////////////////////////////////////////
}}
