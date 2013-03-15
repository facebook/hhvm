/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_debugger.h>
#include <runtime/ext/ext_string.h>
#include <runtime/eval/debugger/cmd/cmd_user.h>
#include <runtime/eval/debugger/cmd/cmd_interrupt.h>
#include <runtime/vm/debugger_hook.h>
#include <runtime/vm/translator/translator-inline.h>
#include <tbb/concurrent_hash_map.h>
#include <util/logger.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using namespace Eval;
using HPHP::VM::Transl::CallerFrame;

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
  return CmdUser::InstallCommand(cmd, clsname);
}

Array f_hphpd_get_user_commands() {
  return CmdUser::GetCommands();
}

static const Trace::Module TRACEMOD = Trace::bcinterp;

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
    throw VMSwitchModeException(true);
  }
  TRACE(5, "out f_hphpd_break()\n");
}

typedef tbb::concurrent_hash_map<std::string, DebuggerClient*> DbgCltMap;
static DbgCltMap s_dbgCltMap;

// if the DebuggerClient with the same name is already in use, return null
Variant f_hphpd_get_client(CStrRef name /* = null */) {
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

c_DebuggerProxyCmdUser::c_DebuggerProxyCmdUser(VM::Class* cb) : ExtObjectData(cb) {
}

c_DebuggerProxyCmdUser::~c_DebuggerProxyCmdUser() {
}

void c_DebuggerProxyCmdUser::t___construct() {
}

bool c_DebuggerProxyCmdUser::t_islocal() {
  return m_proxy->isLocal();
}

Variant c_DebuggerProxyCmdUser::t_send(CObjRef cmd) {
  CmdUser cmdUser(cmd);
  return m_proxy->send(&cmdUser);
}

///////////////////////////////////////////////////////////////////////////////

c_DebuggerClientCmdUser::c_DebuggerClientCmdUser(VM::Class* cb) : ExtObjectData(cb) {
}

c_DebuggerClientCmdUser::~c_DebuggerClientCmdUser() {
}

void c_DebuggerClientCmdUser::t___construct() {
}

void c_DebuggerClientCmdUser::t_quit() {
  m_client->quit();
}

static String format_string(DebuggerClient *client,
                            int _argc, CStrRef format, CArrRef _argv) {
  Variant ret = f_sprintf(_argc, format, _argv);
  if (ret.isString()) {
    return ret;
  }
  client->error("Debugger extension failed to format string: %s",
                 format.data());
  return "";
}

void c_DebuggerClientCmdUser::t_print(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  m_client->print(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_help(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  m_client->help(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_info(int _argc, CStrRef format,
                              CArrRef _argv /* = null_array */) {
  m_client->info(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_output(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  m_client->output(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_error(int _argc, CStrRef format,
                               CArrRef _argv /* = null_array */) {
  m_client->error(format_string(m_client, _argc, format, _argv));
}

void c_DebuggerClientCmdUser::t_code(CStrRef source, int highlight_line /* = 0 */,
                              int start_line_no /* = 0 */,
                              int end_line_no /* = 0 */) {
  m_client->code(source, highlight_line, start_line_no, end_line_no);
}

Variant c_DebuggerClientCmdUser::t_ask(int _argc, CStrRef format,
                                CArrRef _argv /* = null_array */) {
  String ret = format_string(m_client, _argc, format, _argv);
  return String::FromChar(m_client->ask("%s", ret.data()));
}

String c_DebuggerClientCmdUser::t_wrap(CStrRef str) {
  return m_client->wrap(str.data());
}

void c_DebuggerClientCmdUser::t_helptitle(CStrRef str) {
  m_client->helpTitle(str.data());
}

void c_DebuggerClientCmdUser::t_helpcmds(int _argc, CStrRef cmd, CStrRef desc,
                                  CArrRef _argv /* = null_array */) {
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
  m_client->helpBody(str.data());
}

void c_DebuggerClientCmdUser::t_helpsection(CStrRef str) {
  m_client->helpSection(str.data());
}

void c_DebuggerClientCmdUser::t_tutorial(CStrRef str) {
  m_client->tutorial(str.data());
}

String c_DebuggerClientCmdUser::t_getcode() {
  return m_client->getCode();
}

String c_DebuggerClientCmdUser::t_getcommand() {
  return m_client->getCommand();
}

bool c_DebuggerClientCmdUser::t_arg(int index, CStrRef str) {
  return m_client->arg(index + 1, str.data());
}

int64_t c_DebuggerClientCmdUser::t_argcount() {
  return m_client->argCount() - 1;
}

String c_DebuggerClientCmdUser::t_argvalue(int index) {
  return m_client->argValue(index + 1);
}

String c_DebuggerClientCmdUser::t_linerest(int index) {
  return m_client->lineRest(index + 1);
}

Array c_DebuggerClientCmdUser::t_args() {
  StringVec *args = m_client->args();
  Array ret(Array::Create());
  for (unsigned int i = 1; i < args->size(); i++) {
    ret.append(String(args->at(i)));
  }
  return ret;
}

Variant c_DebuggerClientCmdUser::t_send(CObjRef cmd) {
  CmdUser cmdUser(cmd);
  m_client->send(&cmdUser);
  return true;
}

Variant c_DebuggerClientCmdUser::t_xend(CObjRef cmd) {
  CmdUser cmdUser(cmd);
  CmdUserPtr ret = m_client->xend<CmdUser>(&cmdUser);
  return ret->getUserCommand();
}

Variant c_DebuggerClientCmdUser::t_getcurrentlocation() {
  BreakPointInfoPtr bpi = m_client->getCurrentLocation();
  Array ret(Array::Create());
  if (bpi) {
    ret.set("file",      String(bpi->m_file));
    ret.set("line",      (int64_t)bpi->m_line1);
    ret.set("namespace", String(bpi->getNamespace()));
    ret.set("class",     String(bpi->getClass()));
    ret.set("function",  String(bpi->getFunction()));
    ret.set("text",      String(bpi->site()));
  }
  return ret;
}

Variant c_DebuggerClientCmdUser::t_getstacktrace() {
  return m_client->getStackTrace();
}

int64_t c_DebuggerClientCmdUser::t_getframe() {
  return m_client->getFrame();
}

void c_DebuggerClientCmdUser::t_printframe(int index) {
  m_client->printFrame(index, m_client->getStackTrace()[index]);
}

void c_DebuggerClientCmdUser::t_addcompletion(CVarRef list) {
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

c_DebuggerClient::c_DebuggerClient(VM::Class* cb) : ExtObjectData(cb) {
  m_client = NULL;
}

c_DebuggerClient::~c_DebuggerClient() {
  sweep();
}

void c_DebuggerClient::t___construct() {
}

int64_t c_DebuggerClient::t_getstate() {
  if (!m_client) {
    return q_DebuggerClient$$STATE_INVALID;
  }
  return m_client->getClientState();
}

Variant c_DebuggerClient::t_init(CVarRef options) {
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
  if (opsArr.exists("user")) {
    ops.user = opsArr.rvalAtRef("user").toString().data();
  } else {
    raise_warning("must specify user in options");
    return false;
  }

  if (opsArr.exists("configFName")) {
    ops.configFName = opsArr.rvalAtRef("configFName").toString().data();
    FILE *f = fopen(ops.configFName.c_str(), "r");
    if (!f) {
      raise_warning("cannot access config file %s", ops.configFName.c_str());
      return false;
    }
    fclose(f);
  }

  if (opsArr.exists("host")) {
    ops.host = opsArr.rvalAtRef("host").toString().data();
  }
  if (opsArr.exists("port")) {
    ops.port = opsArr.rvalAtRef("port").toInt32();
  }
  if (opsArr.exists("sandbox")) {
    ops.sandbox = opsArr.rvalAtRef("sandbox").toString().data();
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
  if (!ret) {
    raise_warning("failed to initialize machine info");
    return false;
  }

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
    "break", "continue", "down", "exception", "frame", "global",
    "help", "info", "konstant", "next", "out", "print", "quit", "step",
    "up", "variable", "where", "bt", "set", "inst", "=", "@", NULL
  };

  bool allowed = false;
  for (int i = 0; ; i++) {
    const char *cmd = s_allowedCmds[i];
    if (cmd == NULL) {
      break;
    }
    if (cmdName.same(cmd)) {
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
    // Flow-control command goes here
    Logger::Info("wait for debugger client to stop");
    m_client->setTakingInterrupt();
    m_client->setClientState(DebuggerClient::StateBusy);
    DebuggerCommandPtr cmd = m_client->waitForNextInterrupt();
    if (!cmd) {
      raise_warning("not getting a command");
    } else if (cmd->is(DebuggerCommand::KindOfInterrupt)) {
      CmdInterruptPtr cmdInterrupt = dynamic_pointer_cast<CmdInterrupt>(cmd);
      cmdInterrupt->onClientD(m_client);
    } else {
      // Previous pending commands
      cmd->handleReply(m_client);
      cmd->setClientOutput(m_client);
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
