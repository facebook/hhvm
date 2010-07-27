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

#include <runtime/eval/debugger/cmd/cmd_extended.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdExtended::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_class);
  m_cmd->sendImpl(thrift);
}

void CmdExtended::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  m_cmd = CreateExtendedCommand(m_class);
  m_cmd->recvImpl(thrift);
}

void CmdExtended::helpImpl(DebuggerClient *client, const char *name) {
  client->helpTitle("User Extended Command");
  client->help("%s {cmd} {arg1} {arg2} ...    invoke specified command", name);
  client->help("%s{cmd} {arg1} {arg2} ...     invoke specified command", name);
  const ExtendedCommandMap &cmds = getCommandMap();
  if (!cmds.empty()) {
    client->help("");
    client->help("where {cmd} can be:");
    client->help("");
    for (ExtendedCommandMap::const_iterator iter = cmds.begin();
         iter != cmds.end(); ++iter) {
      client->help("\t%s", iter->first.c_str());
    }
    client->help("");
    client->help("Type '%s [h]elp|? {cmd} to read their usages.", name);
  }
}

ExtendedCommandMap CmdExtended::match(DebuggerClient *client, int argIndex) {
  ExtendedCommandMap matches;
  const ExtendedCommandMap &cmds = getCommandMap();
  for (ExtendedCommandMap::const_iterator iter = cmds.begin();
       iter != cmds.end(); ++iter) {
    if (client->arg(argIndex, iter->first.c_str())) {
      matches[iter->first] = iter->second;
    }
  }
  if (matches.empty()) {
    client->error("cannot find the specified user command: %s",
                  client->argValue(argIndex).c_str());
  }
  return matches;
}

bool CmdExtended::helpCommands(DebuggerClient *client,
                               const ExtendedCommandMap &matches) {
  for (ExtendedCommandMap::const_iterator iter = matches.begin();
       iter != matches.end(); ++iter) {
    invokeHelp(client, iter->second);
  }
  return true;
}

bool CmdExtended::onClient(DebuggerClient *client) {
  if (client->arg(1, "help") || client->arg(1, "?")) {
    if (client->argCount() == 1) {
      return help(client);
    }
    ExtendedCommandMap matches = match(client, 2);
    if (matches.empty()) {
      return help(client);
    }
    return helpCommands(client, matches);
  }

  ExtendedCommandMap matches = match(client, 1);
  if (matches.empty()) {
    return help(client);
  }
  if (matches.size() > 1) {
    client->error("need more letters to tell which one of these:");
    for (ExtendedCommandMap::const_iterator iter = matches.begin();
         iter != matches.end(); ++iter) {
      client->error("\t%s", iter->first.c_str());
    }
    return true;
  }

  if (!invokeClient(client, matches.begin()->second)) {
    if (client->arg(2, "help") || client->arg(2, "?")) {
      return helpCommands(client, matches);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CmdExtended::help(DebuggerClient *client) {
  helpImpl(client, "x");
  return true;
}

const ExtendedCommandMap &CmdExtended::getCommandMap() {
  return GetExtendedCommandMap();
}

bool CmdExtended::invokeHelp(DebuggerClient *client, const std::string &cls) {
  DebuggerCommandPtr cmd = CreateExtendedCommand(cls);
  if (cmd) {
    return cmd->help(client);
  }
  return false;
}

bool CmdExtended::invokeClient(DebuggerClient *client, const std::string &cls){
  DebuggerCommandPtr cmd = CreateExtendedCommand(cls);
  if (cmd) {
    return cmd->onClient(client);
  }
  return false;
}

bool CmdExtended::onServer(DebuggerProxy *proxy) {
  return m_cmd->onServer(proxy);
}

///////////////////////////////////////////////////////////////////////////////

const ExtendedCommandMap &CmdExtended::GetExtendedCommandMap() {
  static ExtendedCommandMap s_command_map;
  if (s_command_map.empty()) {
    s_command_map["ample"] = "CmdExtendedExample";
  }
  return s_command_map;
}

DebuggerCommandPtr CmdExtended::CreateExtendedCommand(const std::string &cls) {
  Logger::Error("Unable to create %s extended command", cls.c_str());
  return DebuggerCommandPtr();
}

///////////////////////////////////////////////////////////////////////////////
}}
