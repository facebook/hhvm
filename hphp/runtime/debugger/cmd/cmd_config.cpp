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

#include "hphp/runtime/debugger/cmd/cmd_config.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdConfig::help(DebuggerClient &client) {
  client.helpTitle("Config Command");
  client.helpCmds("set <var> <value>", "set variable <var> to be <value>",
                   "set", "list current values of variables",
                   nullptr);
  client.helpBody(
    "Use this command to set up config variable, "
    "e.g. turning on/off a special mode."
  );
}

void CmdConfig::onClientImpl(DebuggerClient &client) {
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
      client.setDebuggerBypassCheck(true);
    } else if (value == "off") {
      client.print("BypassAccessCheck(bac) set to off");
      client.setDebuggerBypassCheck(false);
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
    client.setDebuggerPrintLevel(pl);
    client.print("PrintLevel(pl) is set to %d", pl);
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
      client.setDebuggerStackArgs(true);
    } else if (value == "off") {
      client.print("StackArgs(sa) set to off");
      client.setDebuggerStackArgs(false);
    } else {
      help(client);
    }
    return;
  }
  if (var == "ApiModeSerialize") {
    assert(client.isApiMode());
    if (value == "on") {
      client.setDebuggerClientApiModeSerialize(true);
    } else if (value == "off") {
      client.setDebuggerClientApiModeSerialize(false);
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

static const StaticString
  s_BypassAccessCheck("BypassAccessCheck"),
  s_LogFile("LogFile"),
  s_PrintLevel("PrintLevel"),
  s_SmallStep("SmallStep"),
  s_StackArgs("StackArgs"),
  s_ApiModeSerialize("ApiModeSerialize");

void CmdConfig::setClientOutput(DebuggerClient &client) {
  client.setOutputType(DebuggerClient::OTValues);
  ArrayInit values(6);
  values.set(s_BypassAccessCheck, client.getDebuggerBypassCheck());
  values.set(s_LogFile, client.getLogFile());
  values.set(s_PrintLevel, client.getDebuggerPrintLevel());
  values.set(s_SmallStep, client.getDebuggerClientSmallStep());
  values.set(s_StackArgs, client.getDebuggerStackArgs());
  values.set(s_ApiModeSerialize, client.getDebuggerClientApiModeSerialize());
  client.setOTValues(values.create());
}

void CmdConfig::listVars(DebuggerClient &client) {
  std::string LogFile = client.getLogFile();
  client.print("LogFile(lf) %s", LogFile == "" ?
                                    "off" : LogFile.c_str());
  client.print("BypassAccessCheck(bac) %s",
                client.getDebuggerBypassCheck() ? "on" : "off");
  client.print("PrintLevel(pl) %d", client.getDebuggerPrintLevel());
  client.print("SmallStep(ss) %s",
                client.getDebuggerClientSmallStep() ? "on" : "off");
  client.print("StackArgs(sa) %s",
                client.getDebuggerStackArgs() ? "on" : "off");
  client.print("ApiModeSerialize %s",
                client.getDebuggerClientApiModeSerialize() ? "on" : "off");
  client.print("MaxCodeLines(mcl) %d",
                client.getDebuggerClientMaxCodeLines());
}

///////////////////////////////////////////////////////////////////////////////
}}
