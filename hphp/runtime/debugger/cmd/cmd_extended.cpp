/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <memory>
#include <vector>
#include "hphp/runtime/debugger/cmd/all.h"
#include "hphp/util/logger.h"

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdExtended::list(DebuggerClient &client) {
  if (client.argCount() == 0) {
    const ExtendedCommandMap &cmds = getCommandMap();
    for (auto& pair : cmds) {
      client.addCompletion(pair.first.c_str());
    }
  } else {
    auto matches = match(client, 1);
    if (matches.size() == 1) {
      invokeList(client, matches.begin()->second);
    }
  }
}

static std::string format_unique_prefix(const std::string &cmd,
                                        const std::string &prev,
                                        const std::string &next) {
  for (size_t i = 1; i < cmd.size(); i++) {
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
  client.helpCmds((std::string(name) + " " + cmd).c_str(), help,
                   (std::string(name) + cmd).c_str(), help,
                   nullptr);

  auto const& cmds = getCommandMap();

  if (cmds.empty()) return;

  client.help("%s", "");
  client.help("where {cmd} can be:");
  client.help("%s", "");
  std::vector<std::string> vcmds;
  for (auto& pair : cmds) {
    vcmds.push_back(pair.first);
  }
  for (size_t i = 0; i < vcmds.size(); i++) {
    client.help("\t%s", format_unique_prefix
                (vcmds[i], i ? vcmds[i-1] : "",
                 i < vcmds.size() - 1 ? vcmds[i+1] : "").c_str());
  }
  client.help("%s", "");
  client.help("Type '%s [h]elp|? {cmd} to read their usages.", name);
}

ExtendedCommandMap CmdExtended::match(DebuggerClient &client, int argIndex) {
  ExtendedCommandMap matches;
  auto const& cmds = getCommandMap();
  for (auto& pair : cmds) {
    if (client.arg(argIndex, pair.first.c_str())) {
      matches[pair.first] = pair.second;
    }
  }
  if (matches.empty()) {
    client.error("Cannot find the specified user command: %s",
                  client.argValue(argIndex).c_str());
  }
  return matches;
}

void CmdExtended::helpCommands(DebuggerClient& client,
                               const ExtendedCommandMap& matches) {
  for (auto& pair : matches) {
    invokeHelp(client, pair.second);
  }
}

void CmdExtended::onClient(DebuggerClient &client) {
  if (client.arg(1, "help") || client.arg(1, "?")) {
    if (client.argCount() == 1) {
      help(client);
      return;
    }
    auto matches = match(client, 2);
    if (matches.empty()) {
      help(client);
    } else {
      helpCommands(client, matches);
    }
    return;
  }

  auto matches = match(client, 1);
  if (matches.empty()) {
    help(client);
  } else if (matches.size() > 1) {
    client.error("Need more letters to tell which one of these:");
    for (auto& pair : matches) {
      client.error("\t%s", pair.first.c_str());
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

void CmdExtended::invokeList(DebuggerClient &client, const std::string &cls) {
  auto cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->list(client);
  }
}

bool CmdExtended::invokeHelp(DebuggerClient &client, const std::string &cls) {
  auto cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->help(client);
    return true;
  }
  return false;
}

bool CmdExtended::invokeClient(DebuggerClient &client, const std::string &cls) {
  client.usageLogCommand("extended", cls);
  auto cmd = CreateExtendedCommand(cls);
  if (cmd) {
    cmd->onClient(client);
    return true;
  }
  return false;
}

bool CmdExtended::onServer(DebuggerProxy& /*proxy*/) {
  assertx(false);
  return false;
}

///////////////////////////////////////////////////////////////////////////////

const ExtendedCommandMap &CmdExtended::GetExtendedCommandMap() {
  static ExtendedCommandMap s_command_map = {
    { "ample"    , "CmdExample" },
  };
  return s_command_map;
}

DebuggerCommandPtr CmdExtended::CreateExtendedCommand(const std::string &cls) {
  std::shared_ptr<CmdExtended> ret;
  if (cls == "CmdExample") {
    ret = std::make_shared<CmdExample>();
  }

  if (ret) {
    ret->m_class = cls;
  } else {
    Logger::Error("Unable to create %s extended command", cls.c_str());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
