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

#include "hphp/runtime/debugger/cmd/cmd_extended.h"
#include "hphp/runtime/debugger/cmd/all.h"
#include "hphp/util/logger.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdExtended::list(DebuggerClient &client) {
  if (client.argCount() == 0) {
    const ExtendedCommandMap &cmds = getCommandMap();
    for (ExtendedCommandMap::const_iterator iter = cmds.begin();
         iter != cmds.end(); ++iter) {
      client.addCompletion(iter->first.c_str());
    }
  } else {
    ExtendedCommandMap matches = match(client, 1);
    if (matches.size() == 1) {
      invokeList(client, matches.begin()->second);
    }
  }
}

static string format_unique_prefix(const std::string &cmd,
                                   const std::string &prev,
                                   const std::string &next) {
  for (unsigned int i = 1; i < cmd.size(); i++) {
    if (strncasecmp(cmd.c_str(), prev.c_str(), i) &&
        strncasecmp(cmd.c_str(), next.c_str(), i)) {
      return "[" + cmd.substr(0, i) + "]" + cmd.substr(i);
    }
  }
  return cmd + " (ambiguous command)";
}

void CmdExtended::helpImpl(DebuggerClient &client, const char *name) {
  const char *cmd = "{cmd} {arg1} {arg2} ...";
  const char *help = "invoke specified command";
  client.helpCmds((string(name) + " " + cmd).c_str(), help,
                   (string(name) + cmd).c_str(), help,
                   nullptr);

  const ExtendedCommandMap &cmds = getCommandMap();
  if (!cmds.empty()) {
    client.help("%s", "");
    client.help("where {cmd} can be:");
    client.help("%s", "");
    vector<string> vcmds;
    for (ExtendedCommandMap::const_iterator iter = cmds.begin();
         iter != cmds.end(); ++iter) {
      vcmds.push_back(iter->first);
    }
    for (unsigned int i = 0; i < vcmds.size(); i++) {
      client.help("\t%s", format_unique_prefix
                   (vcmds[i], i ? vcmds[i-1] : "",
                    i < vcmds.size() - 1 ? vcmds[i+1] : "").c_str());
    }
    client.help("%s", "");
    client.help("Type '%s [h]elp|? {cmd} to read their usages.", name);
  }
}

ExtendedCommandMap CmdExtended::match(DebuggerClient &client, int argIndex) {
  ExtendedCommandMap matches;
  const ExtendedCommandMap &cmds = getCommandMap();
  for (ExtendedCommandMap::const_iterator iter = cmds.begin();
       iter != cmds.end(); ++iter) {
    if (client.arg(argIndex, iter->first.c_str())) {
      matches[iter->first] = iter->second;
    }
  }
  if (matches.empty()) {
    client.error("Cannot find the specified user command: %s",
                  client.argValue(argIndex).c_str());
  }
  return matches;
}

void CmdExtended::helpCommands(DebuggerClient &client,
                               const ExtendedCommandMap &matches) {
  for (ExtendedCommandMap::const_iterator iter = matches.begin();
       iter != matches.end(); ++iter) {
    invokeHelp(client, iter->second);
  }
}

void CmdExtended::onClient(DebuggerClient &client) {
  if (client.arg(1, "help") || client.arg(1, "?")) {
    if (client.argCount() == 1) {
      help(client);
      return;
    }
    ExtendedCommandMap matches = match(client, 2);
    if (matches.empty()) {
      help(client);
    } else {
      helpCommands(client, matches);
    }
    return;
  }

  ExtendedCommandMap matches = match(client, 1);
  if (matches.empty()) {
    help(client);
  } else if (matches.size() > 1) {
    client.error("Need more letters to tell which one of these:");
    for (ExtendedCommandMap::const_iterator iter = matches.begin();
         iter != matches.end(); ++iter) {
      client.error("\t%s", iter->first.c_str());
    }
  } else if (!invokeClient(client, matches.begin()->second)) {
    if (client.arg(2, "help") || client.arg(2, "?")) {
      helpCommands(client, matches);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void CmdExtended::help(DebuggerClient &client) {
  client.helpTitle("Extended Command");
  helpImpl(client, "x");
}

const ExtendedCommandMap &CmdExtended::getCommandMap() {
  return GetExtendedCommandMap();
}

void CmdExtended::invokeList(DebuggerClient &client, const std::string &cls){
  DebuggerCommandPtr cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->list(client);
  }
}

bool CmdExtended::invokeHelp(DebuggerClient &client, const std::string &cls) {
  DebuggerCommandPtr cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->help(client);
    return true;
  }
  return false;
}

bool CmdExtended::invokeClient(DebuggerClient &client, const std::string &cls){
  client.usageLogCommand("extended", cls);
  DebuggerCommandPtr cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->onClient(client);
    return true;
  }
  return false;
}

bool CmdExtended::onServer(DebuggerProxy &proxy) {
  assert(false);
  return false;
}

///////////////////////////////////////////////////////////////////////////////

const ExtendedCommandMap &CmdExtended::GetExtendedCommandMap() {
  static ExtendedCommandMap s_command_map = {
    { "ample"    , "CmdExample" },
    { "tension"  , "CmdExtension" },
    { "heaptrace", "CmdHeaptrace" }
  };
  return s_command_map;
}

#define ELSE_IF_CMD(name) \
  } else if (cls == "Cmd" #name) { ret = CmdExtendedPtr(new Cmd ## name());

DebuggerCommandPtr CmdExtended::CreateExtendedCommand(const std::string &cls) {
  CmdExtendedPtr ret;
  if (cls.empty()) {

    // add one line for each command
    ELSE_IF_CMD(Example);
    ELSE_IF_CMD(Extension);
    ELSE_IF_CMD(Heaptrace);
  }

  if (ret) {
    ret->m_class = cls;
  } else {
    Logger::Error("Unable to create %s extended command", cls.c_str());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}}
