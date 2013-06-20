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
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/debugger/cmd/cmd_user.h"
#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"
#include "hphp/runtime/vm/debugger_hook.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unwind.h"
#include "tbb/concurrent_hash_map.h"
#include "hphp/util/logger.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

using namespace Eval;
using HPHP::Transl::CallerFrame;

const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_FILENAMES =
  DebuggerClient::AutoCompleteFileNames;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_VARIABLES =
  DebuggerClient::AutoCompleteVariables;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CONSTANTS =
  DebuggerClient::AutoCompleteConstants;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CLASSES   =
  DebuggerClient::AutoCompleteClasses;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_FUNCTIONS =
  DebuggerClient::AutoCompleteFunctions;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CLASS_METHODS =
  DebuggerClient::AutoCompleteClassMethods;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CLASS_PROPERTIES =
  DebuggerClient::AutoCompleteClassProperties;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CLASS_CONSTANTS =
  DebuggerClient::AutoCompleteClassConstants;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_KEYWORDS =
  DebuggerClient::AutoCompleteKeyword;
const int64_t q_DebuggerClientCmdUser$$AUTO_COMPLETE_CODE =
  DebuggerClient::AutoCompleteCode;

///////////////////////////////////////////////////////////////////////////////

bool f_hphpd_install_user_command(CStrRef cmd, CStrRef clsname) {
  TRACE(5, "in f_hphpd_install_user_command()\n");
  return CmdUser::InstallCommand(cmd, clsname);
}

Array f_hphpd_get_user_commands() {
  TRACE(5, "in f_hphpd_get_user_commands()\n");
  return CmdUser::GetCommands();
}

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
  if (RuntimeOption::EvalJit && !g_vmContext->m_interpreting &&
      DEBUGGER_FORCE_INTR) {
    TRACE(5, "switch mode\n");
    throw VMSwitchModeBuiltin();
  }
  TRACE(5, "out f_hphpd_break()\n");
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

c_DebuggerProxyCmdUser::c_DebuggerProxyCmdUser(Class* cb) : ExtObjectData(cb) {
  TRACE(5, "c_DebuggerProxyCmdUser::c_DebuggerProxyCmdUser\n");
}

c_DebuggerProxyCmdUser::~c_DebuggerProxyCmdUser() {
  TRACE(5, "c_DebuggerProxyCmdUser::~c_DebuggerProxyCmdUser\n");
}

void c_DebuggerProxyCmdUser::t___construct() {
  TRACE(5, "c_DebuggerProxyCmdUser::t___construct\n");
}

bool c_DebuggerProxyCmdUser::t_islocal() {
  TRACE(5, "c_DebuggerProxyCmdUser::t_islocal\n");
  return m_proxy->isLocal();
}

Variant c_DebuggerProxyCmdUser::t_send(CObjRef cmd) {
  TRACE(5, "c_DebuggerProxyCmdUser::t_send\n");
  CmdUser cmdUser(cmd);
  return m_proxy->sendToClient(&cmdUser);
}

///////////////////////////////////////////////////////////////////////////////

c_DebuggerClientCmdUser::c_DebuggerClientCmdUser(Class* cb) : ExtObjectData(cb) {
  TRACE(5, "c_DebuggerClientCmdUser::c_DebuggerClientCmdUser\n");
}

c_DebuggerClientCmdUser::~c_DebuggerClientCmdUser() {
  TRACE(5, "c_DebuggerClientCmdUser::~c_DebuggerClientCmdUser\n");
}

void c_DebuggerClientCmdUser::t___construct() {
  TRACE(5, "c_DebuggerClientCmdUser::t___construct\n");
}

void c_DebuggerClientCmdUser::t_quit() {
  TRACE(5, "c_DebuggerClientCmdUser::t_quit\n");
  m_client->quit();
}

static String format_string(DebuggerClient &client,
                            int _argc, CStrRef format, CArrRef _argv) {
  TRACE(5, "c_DebuggerClientCmdUser::format_string\n");
  Variant ret = f_sprintf(_argc, format, _argv);
  if (ret.isString()) {
    return ret;
  }
  client.error("Debugger extension failed to format string: %s",
                 format.data());
  return "";
}

void c_DebuggerClientCmdUser::t_print(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_print\n");
  m_client->print(format_string(*m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_help(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_help\n");
  m_client->help(format_string(*m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_info(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_info\n");
  m_client->info(format_string(*m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_output(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_output\n");
  m_client->output(format_string(*m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_error(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_error\n");
  m_client->error(format_string(*m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_code(CStrRef source, int highlight_line /* = 0 */,
                              int start_line_no /* = 0 */,
                              int end_line_no /* = 0 */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_code\n");
  m_client->code(source, start_line_no, end_line_no, highlight_line);
}

Variant c_DebuggerClientCmdUser::t_ask(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_ask\n");
  String ret = format_string(*m_client, _argc, format, _argv);
  return String::FromChar(m_client->ask("%s", ret.data()));
}

String c_DebuggerClientCmdUser::t_wrap(CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_wrap\n");
  return m_client->wrap(str.data());
}

void c_DebuggerClientCmdUser::t_helptitle(CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_helptitle\n");
  m_client->helpTitle(str.data());
}

void c_DebuggerClientCmdUser::t_helpcmds(int _argc, CStrRef cmd, CStrRef desc,
                                  CArrRef _argv /* = null_array */) {
  TRACE(5, "c_DebuggerClientCmdUser::t_helpcmds\n");
  std::vector<String> holders;
  std::vector<const char *> cmds;
  cmds.push_back(cmd.data());
  cmds.push_back(desc.data());
  for (int i = 0; i < _argv.size(); i++) {
    String s = _argv[i].toString();
    holders.push_back(s);
    cmds.push_back(s.data());
  }
  m_client->helpCmds(cmds);
}

void c_DebuggerClientCmdUser::t_helpbody(CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_helpbody\n");
  m_client->helpBody(str.data());
}

void c_DebuggerClientCmdUser::t_helpsection(CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_helpsection\n");
  m_client->helpSection(str.data());
}

void c_DebuggerClientCmdUser::t_tutorial(CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_tutorial\n");
  m_client->tutorial(str.data());
}

String c_DebuggerClientCmdUser::t_getcode() {
  TRACE(5, "c_DebuggerClientCmdUser::t_getcode\n");
  return m_client->getCode();
}

String c_DebuggerClientCmdUser::t_getcommand() {
  TRACE(5, "c_DebuggerClientCmdUser::t_getcommand\n");
  return m_client->getCommand();
}

bool c_DebuggerClientCmdUser::t_arg(int index, CStrRef str) {
  TRACE(5, "c_DebuggerClientCmdUser::t_arg\n");
  return m_client->arg(index + 1, str.data());
}

int64_t c_DebuggerClientCmdUser::t_argcount() {
  TRACE(5, "c_DebuggerClientCmdUser::t_argcount\n");
  return m_client->argCount() - 1;
}

String c_DebuggerClientCmdUser::t_argvalue(int index) {
  TRACE(5, "c_DebuggerClientCmdUser::t_argvalue\n");
  return m_client->argValue(index + 1);
}

String c_DebuggerClientCmdUser::t_linerest(int index) {
  TRACE(5, "c_DebuggerClientCmdUser::t_linerest\n");
  return m_client->lineRest(index + 1);
}

Array c_DebuggerClientCmdUser::t_args() {
  TRACE(5, "c_DebuggerClientCmdUser::t_args\n");
  StringVec *args = m_client->args();
  Array ret(Array::Create());
  for (unsigned int i = 1; i < args->size(); i++) {
    ret.append(String(args->at(i)));
  }
  return ret;
}

Variant c_DebuggerClientCmdUser::t_send(CObjRef cmd) {
  TRACE(5, "c_DebuggerClientCmdUser::t_send\n");
  CmdUser cmdUser(cmd);
  m_client->sendToServer(&cmdUser);
  return true;
}

Variant c_DebuggerClientCmdUser::t_xend(CObjRef cmd) {
  TRACE(5, "c_DebuggerClientCmdUser::t_xend\n");
  CmdUser cmdUser(cmd);
  CmdUserPtr ret = m_client->xend<CmdUser>(&cmdUser);
  return ret->getUserCommand();
}

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

Variant c_DebuggerClientCmdUser::t_getcurrentlocation() {
  TRACE(5, "c_DebuggerClientCmdUser::t_getcurrentlocation\n");
  BreakPointInfoPtr bpi = m_client->getCurrentLocation();
  if (!bpi) return Array::Create();
  ArrayInit ret(6);
  ret.set(s_file,      String(bpi->m_file));
  ret.set(s_line,      (int64_t)bpi->m_line1);
  ret.set(s_namespace, String(bpi->getNamespace()));
  ret.set(s_class,     String(bpi->getClass()));
  ret.set(s_function,  String(bpi->getFunction()));
  ret.set(s_text,      String(bpi->site()));
  return ret.create();
}

Variant c_DebuggerClientCmdUser::t_getstacktrace() {
  TRACE(5, "c_DebuggerClientCmdUser::t_getstacktrace\n");
  return m_client->getStackTrace();
}

int64_t c_DebuggerClientCmdUser::t_getframe() {
  TRACE(5, "c_DebuggerClientCmdUser::t_getframe\n");
  return m_client->getFrame();
}

void c_DebuggerClientCmdUser::t_printframe(int index) {
  TRACE(5, "c_DebuggerClientCmdUser::t_printframe\n");
  m_client->printFrame(index, m_client->getStackTrace()[index]);
}

void c_DebuggerClientCmdUser::t_addcompletion(CVarRef list) {
  TRACE(5, "c_DebuggerClientCmdUser::t_addcompletion\n");
  if (list.isInteger()) {
    m_client->addCompletion((DebuggerClient::AutoComplete)list.toInt64());
  } else {
    Array arr = list.toArray(); // handles string, array and iterators
    std::vector<std::string> items;
    for (ArrayIter iter(arr); iter; ++iter) {
      items.push_back(iter.second().toString()->toCPPString());
    }
    m_client->addCompletion(items);
  }
}

///////////////////////////////////////////////////////////////////////////////

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
