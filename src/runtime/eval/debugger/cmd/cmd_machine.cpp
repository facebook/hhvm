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

bool CmdMachine::help(DebuggerClient *client) {
  client->helpTitle("Machine Command");
  client->help("[m]achine [c]onnect local          debugging local script");
  client->help("[m]achine [c]onnect {host}         debugging remote server");
  client->help("[m]achine [c]onnect {host}:{port}  debugging remote server");
  client->help("[m]achine [l]ist                   list all sandboxes");
  client->help("[m]achine [a]ttach {index}         attach to a sandbox");
  client->helpBody(
    "Use this command to switch between different machines or "
    "sandboxes.\n"
    "\n"
    "\"local\" is a special host name, and when connecting to local, "
    "all evaluation of PHP code happens locally within the debugger. "
    "This is the mode when debugger is started without a remote server "
    "name. No user libraries are pre-loaded in this mode.\n"
    "\n"
    "When connecting to a remote server, it will automatically attach "
    "to \"default\" sandbox under current user. If \"default\" sandbox "
    "does not exist, it will attach to a random sandbox under current "
    "user. In sandbox mode, a file specified in server's configuration "
    "of \"Eval.Debugger.StartupDocument\" is pre-loaded.\n"
    "\n"
    "If there is no sandbox available, it will create a \"dummy\" "
    "sandbox and attach to it. This \"dummy\" sandbox is not "
    "associated with any PHP files, hence not pre-loading any user "
    "libraries.\n"
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

bool CmdMachine::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 0) return help(client);

  if (client->arg(1, "list")) {
    processList(client);
    return true;
  }

  if (client->arg(1, "attach")) {
    string snum = client->argValue(2);
    if (snum.empty() || !DebuggerClient::IsValidNumber(snum)) {
      client->error("'[m]achine [a]attach' needs an {index} argument.");
      client->tutorial(
        "You will have to run '[m]achine [l]ist' first to see a list of valid "
        "numbers or indices to specify."
      );
      return true;
    }

    int num = atoi(snum.c_str());
    string sandbox = client->getSandbox(num);
    if (sandbox.empty()) {
      client->error("\"%s\" is not a valid sandbox index. Choose one from "
                    "this list:", snum.c_str());
      processList(client);
      return true;
    }

    m_body = "attach";
    m_sandboxes.push_back(sandbox);
    client->send(this);
    client->info("attached to sandbox %s", sandbox.c_str());
    return true;
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
