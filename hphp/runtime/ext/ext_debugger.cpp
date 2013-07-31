/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/ext_debugger.h"
#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unwind.h"
#include "tbb/concurrent_hash_map.h"
#include "hphp/util/logger.h"
#include "hphp/util/network.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

using namespace Eval;
using HPHP::Transl::CallerFrame;

///////////////////////////////////////////////////////////////////////////////

void f_hphpd_break(bool condition /* = true */) {
  TRACE(5, "in f_hphpd_break()\n");
  if (!RuntimeOption::EnableDebugger || !condition ||
      g_vmContext->m_dbgNoBreak) {
    TRACE(5, "bail !%d || !%d || %d\n", RuntimeOption::EnableDebugger,
          condition, g_vmContext->m_dbgNoBreak);
    return;
  }
  CallerFrame cf;
  Debugger::InterruptVMHook(HardBreakPoint);
  if (RuntimeOption::EvalJit && DEBUGGER_FORCE_INTR) {
    TRACE(5, "switch mode\n");
    throw VMSwitchModeBuiltin();
  }
  TRACE(5, "out f_hphpd_break()\n");
}

// Quickly determine if a debugger is attached to the current thread.
bool f_hphp_debugger_attached() {
  return (RuntimeOption::EnableDebugger && (Debugger::GetProxy() != nullptr));
}

const StaticString
  s_clientIP("clientIP"),
  s_clientPort("clientPort");

// Determine if a debugger is attached to the current thread, and
// return information about where it is connected from. The client IP
// and port will be null if the connection is local.
Variant f_hphp_get_debugger_info() {
  Array ret(Array::Create());
  if (!RuntimeOption::EnableDebugger) return ret;
  DebuggerProxyPtr proxy = Debugger::GetProxy();
  if (!proxy) return ret;
  Variant address;
  Variant port;
  if (proxy->getClientConnectionInfo(ref(address), ref(port))) {
    ret.set(s_clientIP, address);
    ret.set(s_clientPort, port);
  }
  return ret;
}

typedef tbb::concurrent_hash_map<std::string, DebuggerClient*> DbgCltMap;
static DbgCltMap s_dbgCltMap;

// if the DebuggerClient with the same name is already in use, return null
Variant f_hphpd_get_client(CStrRef name /* = null */) {
  TRACE(5, "in f_hphpd_get_client()\n");
  if (name.empty()) {
    return uninit_null();
  }
  DebuggerClient *client = NULL;
  std::string nameStr = name->toCPPString();
  {
    DbgCltMap::accessor acc;
    if (s_dbgCltMap.insert(acc, nameStr)) {
      client = new DebuggerClient(nameStr);
      acc->second = client;
    } else {
      client = acc->second;
    }
    if (!client->apiGrab()) {
      // already grabbed by another request
      return uninit_null();
    }
  }
  p_DebuggerClient clt(NEWOBJ(c_DebuggerClient));
  clt->m_client = client;
  return clt;
}

Variant f_hphpd_client_ctrl(CStrRef name, CStrRef op) {
  TRACE(5, "in f_hphpd_client_ctrl()\n");
  DebuggerClient *client = NULL;
  std::string nameStr = name->toCPPString();
  {
    DbgCltMap::const_accessor acc;
    if (!s_dbgCltMap.find(acc, nameStr)) {
      if (op.equal("getstate")) {
        return q_DebuggerClient$$STATE_INVALID;
      } else {
        raise_warning("client %s does not exist", name.data());
        return uninit_null();
      }
    }
    client = acc->second;
  }
  if (op.equal("interrupt")) {
    if (client->getClientState() < DebuggerClient::StateReadyForCommand) {
      raise_warning("client is not initialized");
      return uninit_null();
    }
    if (client->getClientState() != DebuggerClient::StateBusy) {
      raise_warning("client is not in a busy state");
      return uninit_null();
    }
    client->onSignal(SIGINT);
    return uninit_null();
  } else if (op.equal("getstate")) {
    return client->getClientState();
  } else if (op.equal("reset")) {
    // To handle the case when client is in a bad state, e.g. the grabbing
    // request encountered error and did not get chance to destruct or call
    // sweep. It will remove the client from the map. Here we'd rather take
    // the risk of leaking the client than the risk of chasing dangling
    // pointers.
    //
    // FIXME: it's unclear why it should be possible that we would not
    // get a chance to destruct or call sweep.
    return s_dbgCltMap.erase(nameStr);
  }

  raise_warning("unknown op %s", op.data());

  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_file("file"),
  s_line("line"),
  s_namespace("namespace"),
  s_class("class"),
  s_function("function"),
  s_text("text"),
  s_user("user"),
  s_configFName("configFName"),
  s_host("host"),
  s_port("port"),
  s_sandbox("sandbox");

const int64_t q_DebuggerClient$$STATE_INVALID = -1;
const int64_t q_DebuggerClient$$STATE_UNINIT
  = DebuggerClient::StateUninit;
const int64_t q_DebuggerClient$$STATE_INITIALIZING
  = DebuggerClient::StateInitializing;
const int64_t q_DebuggerClient$$STATE_READY_FOR_COMMAND
  = DebuggerClient::StateReadyForCommand;
const int64_t q_DebuggerClient$$STATE_BUSY
  = DebuggerClient::StateBusy;

c_DebuggerClient::c_DebuggerClient(Class* cb) : ExtObjectData(cb) {
  TRACE(5, "c_DebuggerClient::c_DebuggerClient(Class* cb)\n");
  m_client = NULL;
}

c_DebuggerClient::~c_DebuggerClient() {
  TRACE(5, "c_DebuggerClient::~c_DebuggerClient()\n");
  sweep();
}

void c_DebuggerClient::t___construct() {
  TRACE(5, "c_DebuggerClient::t___construct\n");
}

int64_t c_DebuggerClient::t_getstate() {
  TRACE(5, "c_DebuggerClient::t_getstate\n");
  if (!m_client) {
    return q_DebuggerClient$$STATE_INVALID;
  }
  return m_client->getClientState();
}

Variant c_DebuggerClient::t_init(CVarRef options) {
  TRACE(5, "c_DebuggerClient::t_init\n");
  if (!m_client) {
    raise_warning("invalid client");
    return false;
  }
  if (m_client->getClientState() != DebuggerClient::StateUninit) {
    return m_client->getClientState() == DebuggerClient::StateReadyForCommand;
  }
  if (!options.isArray()) {
    raise_warning("options must be an array");
    return false;
  }
  m_client->setClientState(DebuggerClient::StateInitializing);

  DebuggerClientOptions ops;
  ops.apiMode = true;

  Array opsArr = options.toArray();
  if (opsArr.exists(s_user)) {
    ops.user = opsArr.rvalAtRef(s_user).toString().data();
  } else {
    raise_warning("must specify user in options");
    return false;
  }

  if (opsArr.exists(s_configFName)) {
    ops.configFName = opsArr.rvalAtRef(s_configFName).toString().data();
    FILE *f = fopen(ops.configFName.c_str(), "r");
    if (!f) {
      raise_warning("cannot access config file %s", ops.configFName.c_str());
      return false;
    }
    fclose(f);
  }

  if (opsArr.exists(s_host)) {
    ops.host = opsArr.rvalAtRef(s_host).toString().data();
  }
  if (opsArr.exists(s_port)) {
    ops.port = opsArr.rvalAtRef(s_port).toInt32();
  }
  if (opsArr.exists(s_sandbox)) {
    ops.sandbox = opsArr.rvalAtRef(s_sandbox).toString().data();
  }

  m_client->init(ops);

  if (ops.host.empty()) {
    ops.host = "localhost";
  }
  if (ops.port < 0) {
    ops.port = RuntimeOption::DebuggerServerPort;
  }
  bool ret = m_client->connect(ops.host, ops.port);
  if (!ret) {
    raise_warning("failed to connect to hhvm %s:%d", ops.host.c_str(),
                  ops.port);
    return false;
  }

  // To wait for the session start interrupt
  DebuggerCommandPtr cmd = m_client->waitForNextInterrupt();
  if (!cmd->is(DebuggerCommand::KindOfInterrupt) ||
      dynamic_pointer_cast<CmdInterrupt>(cmd)->getInterruptType() !=
      SessionStarted) {
    raise_warning("failed to load sandbox");
    return false;
  }

  ret = m_client->initializeMachine();
  assert(ret); // Always returns true in API mode.

  // To wait for the machine loading sandbox
  cmd = m_client->waitForNextInterrupt();
  if (!cmd->is(DebuggerCommand::KindOfInterrupt) ||
      dynamic_pointer_cast<CmdInterrupt>(cmd)->getInterruptType() !=
      SessionStarted) {
    raise_warning("failed to load sandbox");
    return false;
  }

  m_client->setClientState(DebuggerClient::StateReadyForCommand);

  return true;
}

Variant c_DebuggerClient::t_processcmd(CVarRef cmdName, CVarRef args) {
  TRACE(5, "c_DebuggerClient::t_processcmd\n");
  if (!m_client ||
      m_client->getClientState() < DebuggerClient::StateReadyForCommand) {
    raise_warning("client is not initialized");
    return uninit_null();
  }
  if (m_client->getClientState() != DebuggerClient::StateReadyForCommand) {
    raise_warning("client is not ready to take command");
    return uninit_null();
  }
  if (!cmdName.isString()) {
    raise_warning("cmdName must be string");
    return uninit_null();
  }
  if (!args.isNull() && !args.isArray()) {
    raise_warning("args must be null or array");
    return uninit_null();
  }

  static const char *s_allowedCmds[] = {
    "break", "bt", "continue", "down", "exception", "frame", "global",
    "help", "info", "inst", "konstant", "next", "out", "print", "quit",
    "set", "step", "up", "variable", "where", "=", "@", nullptr
  };

  bool allowed = false;
  for (int i = 0; ; i++) {
    const char *cmd = s_allowedCmds[i];
    if (cmd == NULL) {
      break;
    }
    if (same(cmdName, cmd)) {
      allowed = true;
      break;
    }
  }
  if (!allowed) {
    raise_warning("unsupported command %s", cmdName.toString().data());
    return uninit_null();
  }

  m_client->setCommand(cmdName.toString().data());
  StringVec *clientArgs = m_client->args();
  clientArgs->clear();
  if (!args.isNull()) {
    for (ArrayIter iter(args.toArray()); iter; ++iter) {
      CStrRef arg = iter.second().toString();
      clientArgs->push_back(std::string(arg.data(), arg.size()));
    }
  }
  try {
    if (!m_client->process()) {
      raise_warning("command \"%s\" not found", cmdName.toString().data());
    }
  } catch (DebuggerConsoleExitException &e) {
    TRACE(4, "Command raised DebuggerConsoleExitException\n");
    // Flow-control command goes here
    Logger::Info("wait for debugger client to stop");
    m_client->setClientState(DebuggerClient::StateBusy);
    DebuggerCommandPtr cmd = m_client->waitForNextInterrupt();
    TRACE(4, "waitForNextInterrupt() came back as ");
    if (!cmd) {
      TRACE(4, "null\n");
      raise_warning("not getting a command");
    } else if (cmd->is(DebuggerCommand::KindOfInterrupt)) {
      TRACE(4, "an interrupt\n");
      CmdInterruptPtr cmdInterrupt = dynamic_pointer_cast<CmdInterrupt>(cmd);
      cmdInterrupt->onClient(*m_client);
    } else {
      TRACE(4, "an previous pending command\n");
      // Previous pending commands
      cmd->handleReply(*m_client);
      cmd->setClientOutput(*m_client);
    }
    Logger::Info("debugger client ready for command");
  } catch (DebuggerClientExitException &e) {
    const std::string& nameStr = m_client->getNameApi();
    Logger::Info("client %s disconnected", nameStr.c_str());
    s_dbgCltMap.erase(nameStr);
    delete m_client;
    m_client = NULL;
    return true;
  } catch (DebuggerProtocolException &e) {
    raise_warning("DebuggerProtocolException");
    return uninit_null();
  }

  return m_client->getOutputArray();
}

void c_DebuggerClient::sweep() {
  TRACE(5, "c_DebuggerClient::sweep\n");
  if (m_client) {
    // Note: it's important that resetSmartAllocatedMembers happens
    // before clearCachedLocal(), because the smart allocated pointers
    // are already invalid.
    m_client->resetSmartAllocatedMembers();
    m_client->clearCachedLocal();
    m_client->apiFree();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
