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

#include <runtime/eval/debugger/cmd/cmd_machine.h>
#include <runtime/base/runtime_option.h>
#include <util/process.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdMachine::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_sandboxes);
}

void CmdMachine::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_sandboxes);
}

void CmdMachine::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    static const char *keywords[] =
      { "disconnect", "connect", "list", "attach", NULL };
    client->addCompletion(keywords);
  }
}

bool CmdMachine::help(DebuggerClient *client) {
  client->helpTitle("Machine Command");
  client->helpCmds(
    "[m]achine [c]onnect {host}",         "debugging remote server",
    "[m]achine [c]onnect {host}:{port}",  "debugging remote server",
    "[m]achine [d]isconnect",             "debugging local script",
    "[m]achine [l]ist",                   "list all sandboxes",
    "[m]achine [a]ttach {index}",         "attach to a sandbox",
    NULL
  );
  client->helpBody(
    "Use this command to switch between different machines or "
    "sandboxes.\n"
    "\n"
    "If command prompt says \"hphpd\", all evaluation of PHP code happens "
    "locally within the debugger. This is the mode when debugger is started "
    "without a remote server name. No user libraries are pre-loaded in this "
    "mode.\n"
    "\n"
    "When connecting to a remote server, it will automatically attach "
    "to \"default\" sandbox under current user. If \"default\" sandbox "
    "does not exist, it will attach to a random sandbox under current "
    "user. In sandbox mode, a file specified in server's configuration "
    "of \"Eval.Debugger.StartupDocument\" is pre-loaded.\n"
    "\n"
    "If there is no sandbox available, it will create a \"dummy\" "
    "sandbox and attach to it.\n"
    "\n"
    "When your sandbox is not available, please hit it at least once "
    "from your browser. Then run '[m]achine [l]ist' command again."
  );
  return true;
}

bool CmdMachine::processList(DebuggerClient *client) {
  m_body = "list";
  CmdMachinePtr res = client->xend<CmdMachine>(this);
  if (res->m_sandboxes.empty()) {
    client->info("(no sandbox was found)");
    client->tutorial(
      "Please hit the sandbox from browser at least once. Then run "
      "'[m]achine [l]ist' again."
    );
  } else {
    for (int i = 0; i < (int)res->m_sandboxes.size(); i++) {
      client->print("  %d\t%s", i + 1, res->m_sandboxes[i].c_str());
    }
    client->tutorial(
      "Use '[m]achine [a]ttach {index}' to attach to one sandbox. For "
      "example, 'm a 1'. If desired sandbox is not on the list, please "
      "hit the sandbox from browser once. Then run '[m]achine [l]ist' "
      "again."
    );
  }
  client->updateSandboxes(res->m_sandboxes);
  return true;
}

bool CmdMachine::AttachSandbox(DebuggerClient *client,
                               const char *user /* = NULL */,
                               const char *name /* = NULL */,
                               const char *path /* = NULL */) {
  string login;
  if (user == NULL) {
    login = Process::GetCurrentUser();
    user = login.c_str();
  }

  DSandboxInfo sandbox;
  sandbox.m_user = user ? user : "";
  sandbox.m_name = name ? name : "default";
  sandbox.m_path = path ? path : "";

  return AttachSandbox(client, sandbox);
}

bool CmdMachine::AttachSandbox(DebuggerClient *client,
                               const DSandboxInfo &sandbox) {
  CmdMachine cmd;
  cmd.m_body = "attach";
  cmd.m_sandboxes.push_back(sandbox.id());

  client->send(&cmd);
  client->info("Attached to %s's %s sandbox.", sandbox.m_user.c_str(),
               sandbox.m_name.c_str());
  throw DebuggerConsoleExitException();
}

bool CmdMachine::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 0) return help(client);

  if (client->arg(1, "connect")) {
    if (client->argCount() != 2) {
      return help(client);
    }
    string host = client->argValue(2);
    int port = 0;
    size_t pos = host.find(":");
    if (pos != string::npos) {
      if (!DebuggerClient::IsValidNumber(host.substr(pos + 1))) {
        client->error("Port needs to be a number");
        return help(client);
      }
      port = atoi(host.substr(pos + 1).c_str());
      host = host.substr(0, pos);
    }
    client->connect(host, port);
    return true;
  }

  if (client->arg(1, "disconnect")) {
    client->disconnect();
    return true;
  }

  if (client->arg(1, "list")) {
    processList(client);
    return true;
  }

  if (client->arg(1, "attach")) {
    string snum = client->argValue(2);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client->error("'[m]achine [a]attach' needs an {index} argument.");
      client->tutorial(
        "You will have to run '[m]achine [l]ist' first to see a list of valid "
        "numbers or indices to specify."
      );
      return true;
    }

    int num = atoi(snum.c_str());
    string id = client->getSandbox(num);
    if (id.empty()) {
      client->error("\"%s\" is not a valid sandbox index. Choose one from "
                    "this list:", snum.c_str());
      processList(client);
      return true;
    }
    DSandboxInfo sandbox(id);
    return AttachSandbox(client, sandbox);
  }

  return help(client);
}

bool CmdMachine::onServer(DebuggerProxy *proxy) {
  if (m_body == "list") {
    Debugger::GetRegisteredSandboxes(m_sandboxes);
    return proxy->send(this);
  }
  if (m_body == "attach" && !m_sandboxes.empty()) {
    proxy->switchSandbox(m_sandboxes[0]);
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
