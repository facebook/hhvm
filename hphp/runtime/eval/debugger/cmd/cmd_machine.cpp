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

#include "hphp/runtime/eval/debugger/cmd/cmd_machine.h"
#include "hphp/runtime/eval/debugger/cmd/cmd_signal.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/util/libevent_http_client.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdMachine::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_sandboxes);
  thrift.write(m_rpcConfig);
  thrift.write(m_force);
  thrift.write(m_succeed);
}

void CmdMachine::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_sandboxes);
  thrift.read(m_rpcConfig);
  thrift.read(m_force);
  thrift.read(m_succeed);
}

void CmdMachine::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    static const char *keywords[] =
      { "disconnect", "connect", "rpc", "list", "attach", nullptr };
    client->addCompletion(keywords);
  }
}

bool CmdMachine::help(DebuggerClient *client) {
  client->helpTitle("Machine Command");
  client->helpCmds(
    "[m]achine [c]onnect {host}",         "debugging remote server natively",
    "[m]achine [c]onnect {host}:{port}",  "debugging remote server natively",
    "[m]achine [r]pc {host}",             "debugging remote server with RPC",
    "[m]achine [r]pc {host}:{port}",      "debugging remote server with RPC",
    "[m]achine [d]isconnect",             "debugging local script",
    "[m]achine [l]ist",                   "list all sandboxes",
    "[m]achine [a]ttach {index}",         "attach to a sandbox",
    "[m]achine [a]ttach {sandbox}",       "attach to my sandbox by name",
    "[m]achine [a]ttach {user} {sandbox}",
    "attach to a sandbox by user and name",
    nullptr
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
    "from your browser. Then run '[m]achine [l]ist' command again.\n"
    "\n"
    "If a HipHop server has RPC port open, one can also debug the server in "
    "a very special RPC mode. In this mode, one can type in PHP scripts to "
    "run, but all functions will be executed on server through RPC. Because "
    "states are still maintained locally and only functions are executed "
    "remotely, it may not work with functions or scripts that depend on "
    "global variables or low-level raw resource pointers. As a simple rule, "
    "stateless functions will work just fine. This is true to objects and "
    "method calls as well, except classes will have to be loaded on client "
    "side by '=include(\"file-containing-the-class.php\")'."
  );
  return true;
}

bool CmdMachine::processList(DebuggerClient *client,
                             bool output /* = true */) {
  m_body = "list";
  CmdMachinePtr res = client->xend<CmdMachine>(this);
  client->updateSandboxes(res->m_sandboxes);
  if (!output) return true;

  if (res->m_sandboxes.empty()) {
    client->info("(no sandbox was found)");
    client->tutorial(
      "Please hit the sandbox from browser at least once. Then run "
      "'[m]achine [l]ist' again."
    );
  } else {
    for (int i = 0; i < (int)res->m_sandboxes.size(); i++) {
      client->print("  %d\t%s", i + 1,
                    res->m_sandboxes[i]->desc().c_str());
    }
    client->tutorial(
      "Use '[m]achine [a]ttach {index}' to attach to one sandbox. For "
      "example, 'm a 1'. If desired sandbox is not on the list, please "
      "hit the sandbox from browser once. Then run '[m]achine [l]ist' "
      "again."
    );
  }

  return true;
}

bool CmdMachine::AttachSandbox(DebuggerClient *client,
                               const char *user /* = NULL */,
                               const char *name /* = NULL */,
                               bool force /* = false */) {
  string login;
  if (user == nullptr) {
    login = client->getCurrentUser();
    user = login.c_str();
  }
  if (client->isApiMode()) {
    force = true;
  }

  DSandboxInfoPtr sandbox(new DSandboxInfo());
  sandbox->m_user = user ? user : "";
  sandbox->m_name = (name && *name) ? name : "default";
  return AttachSandbox(client, sandbox, force);
}

bool CmdMachine::AttachSandbox(DebuggerClient *client,
                               DSandboxInfoPtr sandbox,
                               bool force /* = false */) {
  if (client->isLocal()) {
    client->error("Local script doesn't have sandbox to attach to.");
    return false;
  }

  CmdMachine cmd;
  cmd.m_body = "attach";
  cmd.m_sandboxes.push_back(sandbox);
  cmd.m_force = force;

  client->info("Attaching to %s and pre-loading, please wait...",
               sandbox->desc().c_str());
  CmdMachinePtr cmdMachine = client->xend<CmdMachine>(&cmd);
  if (cmdMachine->m_succeed) {
    client->playMacro("startup");
  } else {
    client->error("failed to attach to sandbox, maybe another client is "
                  "debugging, \nattach to another sandbox, exit the "
                  "attached hphpd client, or try \n"
                  "[m]achine [a]ttach [f]orce [%s] [%s]",
                  sandbox->m_user.c_str(), sandbox->m_name.c_str());
  }
  return cmdMachine->m_succeed;
}

static const StaticString s_host("host");
static const StaticString s_port("port");
static const StaticString s_auth("auth");
static const StaticString s_timeout("timeout");

void CmdMachine::UpdateIntercept(DebuggerClient *client,
                                 const std::string &host, int port) {
  CmdMachine cmd;
  cmd.m_body = "rpc";
  cmd.m_rpcConfig = CREATE_MAP4
    (s_host, String(host),
     s_port, port ? port : RuntimeOption::DebuggerDefaultRpcPort,
     s_auth, String(RuntimeOption::DebuggerDefaultRpcAuth),
     s_timeout, RuntimeOption::DebuggerDefaultRpcTimeout);
  client->xend<CmdMachine>(&cmd);
}

bool CmdMachine::onClientImpl(DebuggerClient *client) {
  if (DebuggerCommand::onClientImpl(client)) return true;
  if (client->argCount() == 0) return help(client);

  bool rpc = client->arg(1, "rpc");
  if (rpc || client->arg(1, "connect")) {
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

    if (rpc) {
      if (client->connectRPC(host, port)) {
        throw DebuggerConsoleExitException();
      }
    } else {
      if (client->connect(host, port)) {
        throw DebuggerConsoleExitException();
      }
    }
    client->initializeMachine();
    return true;
  }

  if (client->arg(1, "disconnect")) {
    if (client->disconnect()) {
      throw DebuggerConsoleExitException();
    }
    client->initializeMachine();
    return true;
  }

  if (client->arg(1, "list")) {
    processList(client);
    return true;
  }

  if (client->arg(1, "attach")) {
    DSandboxInfoPtr sandbox;

    string snum = client->argValue(2);
    if (DebuggerClient::IsValidNumber(snum)) {
      int num = atoi(snum.c_str());
      sandbox = client->getSandbox(num);
      if (!sandbox) {
        processList(client, false);
        sandbox = client->getSandbox(num);
        if (!sandbox) {
          client->error("\"%s\" is not a valid sandbox index. Choose one from "
                        "this list:", snum.c_str());
          processList(client);
          return true;
        }
      }
    } else {
      int argBase = 2;
      if (client->argCount() >= 2 && client->arg(2, "force")) {
        m_force = true;
        argBase++;
      }
      sandbox = DSandboxInfoPtr(new DSandboxInfo());
      if (client->argCount() < argBase) {
        sandbox->m_user = client->getCurrentUser();
        sandbox->m_name = "default";
      } else if (client->argCount() == argBase) {
        sandbox->m_user = client->getCurrentUser();
        sandbox->m_name = client->argValue(argBase);
      } else if (client->argCount() == argBase + 1) {
        sandbox->m_user = client->argValue(argBase);
        sandbox->m_name = client->argValue(argBase + 1);
      } else {
        return help(client);
      }
    }
    if (AttachSandbox(client, sandbox, m_force)) {
      // Attach succeed, wait for next interrupt
      throw DebuggerConsoleExitException();
    }
    return true;
  }

  return help(client);
}

bool CmdMachine::onServer(DebuggerProxy *proxy) {
  if (m_body == "rpc") {
    String host = m_rpcConfig[s_host].toString();
    if (host.empty()) {
      register_intercept("", false, uninit_null());
    } else {
      int port = m_rpcConfig[s_port].toInt32();
      LibEventHttpClient::SetCache(host.data(), port, 1);
      register_intercept("", "fb_rpc_intercept_handler", m_rpcConfig);
    }
    return proxy->sendToClient(this);
  }
  if (m_body == "list") {
    Debugger::GetRegisteredSandboxes(m_sandboxes);
    return proxy->sendToClient(this);
  }
  if (m_body == "attach" && !m_sandboxes.empty()) {
    m_succeed = proxy->switchSandbox(m_sandboxes[0]->id(), m_force);
    if (m_succeed) {
      proxy->notifyDummySandbox();
      m_exitInterrupt = true;
    }
    return proxy->sendToClient(this);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
