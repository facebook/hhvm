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

#include <runtime/eval/debugger/cmd/cmd_config.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdConfig::help(DebuggerClient *client) {
  client->helpTitle("Config Command");
  client->helpCmds("set <var> <value>", "set variable <var> to be <value>",
                   "set", "list current values of variables",
                   nullptr);
  client->helpBody(
    "Use this command to set up config variable, "
    "e.g. turning on/off a special mode."
  );
  return true;
}

bool CmdConfig::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 0) {
    listVars(client);
    return true;
  }
  std::string var = client->argValue(1);
  if (var == "help" || client->argCount() < 2) {
    return help(client);
  }

  std::string value = client->argValue(2);
  if (var == "BypassAccessCheck" || var == "bac") {
    if (value == "on") {
      client->print("BypassAccessCheck(bac) set to on.\n"
                    "All code executed from debugger is bypassing "
                    "access check!");
      client->setDebuggerBypassCheck(true);
    } else if (value == "off") {
      client->print("BypassAccessCheck(bac) set to off");
      client->setDebuggerBypassCheck(false);
    } else {
      return help(client);
    }
    return true;
  }
  if (var == "LogFile" || var == "lf") {
    // Close the current log file handler
    FILE *f = client->getLogFileHandler();
    if (f != nullptr) {
      fclose(f);
      client->setLogFileHandler(nullptr);
    }

    if (value == "off") {
      value = "";
    } else {
      // Try open the log file and error if it's not working
      f = fopen(value.c_str(), "a");
      if (f == nullptr) {
        client->error("Cannot open log file '%s'",
          value.c_str());
        value = "";
        client->setLogFileHandler(nullptr);
      } else {
        client->setLogFileHandler(f);
      }
    }
    client->print("LogFile(lf) is set to %s", value == "" ? "off"
                                                          : value.c_str());
    client->setLogFile(value);
    return true;
  }
  if (var == "PrintLevel" || var == "pl") {
    int pl = strtol(value.c_str(), nullptr, 10);
    if (pl > 0 && pl < DebuggerClient::MinPrintLevel) {
      client->error("%d is invalid for PrintLevel(pl)", pl);
      return true;
    }
    client->setDebuggerPrintLevel(pl);
    client->print("PrintLevel(pl) is set to %d", pl);
    return true;
  }
  if (var == "SmallStep" || var == "ss") {
    if (value == "on") {
      client->print("SmallStep(ss) set to on.\n");
      client->setDebuggerSmallStep(true);
    } else if (value == "off") {
      client->print("SmallStep(ss) set to off");
      client->setDebuggerSmallStep(false);
    } else {
      return help(client);
    }
    return true;
  }
  if (var == "StackArgs" || var == "sa") {
    if (value == "on") {
      client->print("StackArgs(sa) set to on.\n");
      client->setDebuggerStackArgs(true);
    } else if (value == "off") {
      client->print("StackArgs(sa) set to off");
      client->setDebuggerStackArgs(false);
    } else {
      return help(client);
    }
    return true;
  }
  if (var == "ApiModeSerialize") {
    assert(client->isApiMode());
    if (value == "on") {
      client->setDebuggerClientApiModeSerialize(true);
    } else if (value == "off") {
      client->setDebuggerClientApiModeSerialize(false);
    } else {
      return true;
    }
    return true;
  }
  if (var == "MaxCodeLines" || var == "mcl") {
    // MaxCodeLines: a useful configuration variable for emacs/hphpd-integration
    // to prevent or limit code spew after each breakpoint is hit (since emacs
    // hphpd-mode already loads the source file into a buffer and displays a
    // pointer to the current line).
    int mcl = strtol(value.c_str(), nullptr, 10);
    if (mcl < -1) {
      client->error("%d is invalid for MaxCodeLines(mcl)", mcl);
      return true;
    }
    client->setDebuggerClientMaxCodeLines(mcl);
    client->print("MaxCodeLines(mcl) is set to %d", mcl);
    return true;
  }

  listVars(client);
  return true;
}

void CmdConfig::setClientOutput(DebuggerClient *client) {
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  values.set("BypassAccessCheck", client->getDebuggerBypassCheck());
  values.set("LogFile", client->getLogFile());
  values.set("PrintLevel", client->getDebuggerPrintLevel());
  values.set("SmallStep", client->getDebuggerSmallStep());
  values.set("StackArgs", client->getDebuggerStackArgs());
  values.set("ApiModeSerialize", client->getDebuggerClientApiModeSerialize());
  client->setOTValues(values);
}

void CmdConfig::listVars(DebuggerClient *client) {
  std::string LogFile = client->getLogFile();
  client->print("LogFile(lf) %s", LogFile == "" ?
                                    "off" : LogFile.c_str());
  client->print("BypassAccessCheck(bac) %s",
                client->getDebuggerBypassCheck() ? "on" : "off");
  client->print("PrintLevel(pl) %d", client->getDebuggerPrintLevel());
  client->print("SmallStep(ss) %s",
                client->getDebuggerSmallStep() ? "on" : "off");
  client->print("StackArgs(sa) %s",
                client->getDebuggerStackArgs() ? "on" : "off");
  client->print("ApiModeSerialize %s",
                client->getDebuggerClientApiModeSerialize() ? "on" : "off");
  client->print("MaxCodeLines(mcl) %d",
                client->getDebuggerClientMaxCodeLines());
}

///////////////////////////////////////////////////////////////////////////////
}}
