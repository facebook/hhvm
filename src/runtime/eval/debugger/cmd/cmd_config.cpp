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

#include <runtime/eval/debugger/cmd/cmd_config.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdConfig::help(DebuggerClient *client) {
  client->helpTitle("Config Command");
  client->helpCmds("set <var> <value>", "set variable <var> to be <value>",
                   "set", "list current values of variables",
                   NULL);
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
      client->setBypassAccessCheck(true);
    } else if (value == "off") {
      client->print("BypassAccessCheck(bac) set to off");
      client->setBypassAccessCheck(false);
    } else {
      return help(client);
    }
    return true;
  }

  listVars(client);
  return true;
}

void CmdConfig::listVars(DebuggerClient *client) {
  client->print("BypassAccessCheck(bac) %s", client->getBypassAccessCheck() ?
                                             "on" : "off");
}

///////////////////////////////////////////////////////////////////////////////
}}
