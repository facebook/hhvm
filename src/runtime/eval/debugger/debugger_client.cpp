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

#include <runtime/eval/debugger/debugger_client.h>
#include <runtime/eval/debugger/debugger_command.h>
#include <runtime/eval/debugger/cmd/all.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/string_util.h>
#include <runtime/base/preg.h>
#include <runtime/ext/ext_json.h>
#include <runtime/ext/ext_socket.h>
#include <runtime/ext/ext_network.h>
#include <util/text_color.h>
#include <util/text_art.h>
#include <util/logger.h>
#include <util/process.h>
#include <boost/format.hpp>

#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
using namespace HPHP::Util::TextArt;

#define PHP_WORD_BREAK_CHARACTERS " \t\n\"\\'`@=;,|{[()]}+*%^!~&"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static DebuggerClient s_debugger_client;
///////////////////////////////////////////////////////////////////////////////
// readline setups

static char* debugger_generator(const char* text, int state) {
  return s_debugger_client.getCompletion(text, state);
}

static char **debugger_completion(const char *text, int start, int end) {
  if (s_debugger_client.setCompletion(text, start, end)) {
    return rl_completion_matches((char*)text, &debugger_generator);
  }
  return NULL;
}

static void debugger_signal_handler(int sig) {
  s_debugger_client.onSignal(sig);
}

void DebuggerClient::onSignal(int sig) {
  if (m_inputState == TakingInterrupt) {
    time_t now = time(0);

    if (m_sigTime) {
      int secWait = 10;
      if (now - m_sigTime > secWait) {
        error("Program is not responding. Please restart debugger to get a "
              "new connection.");
        quit();
        return;
      }
      info("Please wait. If not responding in %d seconds, "
           "press Ctrl-C again to quit.", secWait);
    } else {
      info("Pausing program execution, please wait...");
      m_sigTime = now;
    }
    m_signum = CmdSignal::SignalBreak;
  } else {
    rl_replace_line("", 0);
    rl_redisplay();
  }
}

int DebuggerClient::pollSignal() {
  int ret = m_signum;
  m_signum = CmdSignal::SignalNone;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Initialization and shutdown.
 */
class ReadlineApp {
public:
  ReadlineApp() {
    DebuggerClient::AdjustScreenMetrics();

    rl_attempted_completion_function = debugger_completion;
    rl_basic_word_break_characters = PHP_WORD_BREAK_CHARACTERS;

    rl_catch_signals = 0;
    signal(SIGINT, debugger_signal_handler);

    read_history((Process::GetHomeDirectory() +
                  DebuggerClient::HistoryFileName).c_str());
  }

  ~ReadlineApp() {
    write_history((Process::GetHomeDirectory() +
                   DebuggerClient::HistoryFileName).c_str());
  }
};

/**
 * Displaying a spinning wait icon.
 */
class ReadlineWaitCursor {
public:
  ReadlineWaitCursor()
      : m_thread(this, &ReadlineWaitCursor::animate), m_waiting(true) {
    m_thread.start();
  }

  ~ReadlineWaitCursor() {
    m_waiting = false;
    m_thread.waitForEnd();
  }

  void animate() {
    m_line = rl_line_buffer;
    while (m_waiting) {
      frame('|'); frame('/'); frame('-'); frame('\\');
      rl_replace_line(m_line.c_str(), 1);
      rl_redisplay();
    }
  }

private:
  AsyncFunc<ReadlineWaitCursor> m_thread;
  bool m_waiting;
  string m_line;

  void frame(char ch) {
    string line = m_line + ch;
    rl_replace_line(line.c_str(), 1);
    rl_redisplay();
    usleep(100000);
  }
};

///////////////////////////////////////////////////////////////////////////////

int DebuggerClient::LineWidth = 76;
int DebuggerClient::CodeBlockSize = 20;
int DebuggerClient::ScrollBlockSize = 20;
const char *DebuggerClient::LineNoFormat = "%4d ";
const char *DebuggerClient::LocalPrompt = "hphpd";
const char *DebuggerClient::ConfigFileName = ".hphpd.hdf";
const char *DebuggerClient::HistoryFileName = ".hphpd.history";
std::string DebuggerClient::SourceRoot;

bool DebuggerClient::UseColor = true;
bool DebuggerClient::NoPrompt = false;

const char *DebuggerClient::HelpColor     = NULL;
const char *DebuggerClient::InfoColor     = NULL;
const char *DebuggerClient::OutputColor   = NULL;
const char *DebuggerClient::ErrorColor    = NULL;
const char *DebuggerClient::ItemNameColor = NULL;
const char *DebuggerClient::HighlightForeColor = NULL;
const char *DebuggerClient::HighlightBgColor = NULL;

const char *DebuggerClient::DefaultCodeColors[] = {
  /* None        */ NULL, NULL,
  /* Keyword     */ NULL, NULL,
  /* Comment     */ NULL, NULL,
  /* String      */ NULL, NULL,
  /* Variable    */ NULL, NULL,
  /* Html        */ NULL, NULL,
  /* Tag         */ NULL, NULL,
  /* Declaration */ NULL, NULL,
  /* Constant    */ NULL, NULL,
  /* LineNo      */ NULL, NULL,
};

void DebuggerClient::LoadColors(Hdf hdf) {
  HelpColor     = LoadColor(hdf["Help"],     "BROWN");
  InfoColor     = LoadColor(hdf["Info"],     "GREEN");
  OutputColor   = LoadColor(hdf["Output"],   "CYAN");
  ErrorColor    = LoadColor(hdf["Error"],    "RED");
  ItemNameColor = LoadColor(hdf["ItemName"], "GRAY");

  HighlightForeColor = LoadColor(hdf["HighlightForeground"], "RED");
  HighlightBgColor = LoadBgColor(hdf["HighlightBackground"], "GRAY");

  Hdf code = hdf["Code"];
  LoadCodeColor(CodeColorKeyword,      code["Keyword"],      "CYAN");
  LoadCodeColor(CodeColorComment,      code["Comment"],      "RED");
  LoadCodeColor(CodeColorString,       code["String"],       "GREEN");
  LoadCodeColor(CodeColorVariable,     code["Variable"],     "BROWN");
  LoadCodeColor(CodeColorHtml,         code["Html"],         "GRAY");
  LoadCodeColor(CodeColorTag,          code["Tag"],          "MAGENTA");
  LoadCodeColor(CodeColorDeclaration,  code["Declaration"],  "BLUE");
  LoadCodeColor(CodeColorConstant,     code["Constant"],     "MAGENTA");
  LoadCodeColor(CodeColorLineNo,       code["LineNo"],       "GRAY");
}

const char *DebuggerClient::LoadColor(Hdf hdf, const char *defaultName) {
  const char *name = hdf.get(defaultName);
  hdf = name;  // for starter
  const char *color = Util::get_color_by_name(name);
  if (color == NULL) {
    Logger::Error("Bad color name %s", name);
    color = Util::get_color_by_name(defaultName);
  }
  return color;
}

const char *DebuggerClient::LoadBgColor(Hdf hdf, const char *defaultName) {
  const char *name = hdf.get(defaultName);
  hdf = name;  // for starter
  const char *color = Util::get_bgcolor_by_name(name);
  if (color == NULL) {
    Logger::Error("Bad color name %s", name);
    color = Util::get_bgcolor_by_name(defaultName);
  }
  return color;
}

void DebuggerClient::LoadCodeColor(CodeColor index, Hdf hdf,
                                   const char *defaultName) {
  const char *color = LoadColor(hdf, defaultName);
  DefaultCodeColors[index * 2] = color;
  DefaultCodeColors[index * 2 + 1] = color ? ANSI_COLOR_END : NULL;
}

SmartPtr<Socket> DebuggerClient::Start(const DebuggerClientOptions &options) {
  Debugger::SetTextColors();
  SmartPtr<Socket> ret = s_debugger_client.connectLocal();
  s_debugger_client.start(options);
  return ret;
}

void DebuggerClient::Stop() {
  s_debugger_client.stop();
}

void DebuggerClient::AdjustScreenMetrics() {
  int rows = 0; int cols = 0;
  rl_get_screen_size(&rows, &cols);
  if (rows > 0 && cols > 0) {
    LineWidth = cols - 4;
    ScrollBlockSize = CodeBlockSize = rows - (rows >> 2);
  }
}

bool DebuggerClient::IsValidNumber(const std::string &arg) {
  if (arg.empty()) return false;
  for (unsigned int i = 0; i < arg.size(); i++) {
    if (!isdigit(arg[i])) {
      return false;
    }
  }
  return true;
}

String DebuggerClient::FormatVariable(CVarRef v, int maxlen /* = 80 */) {
  String value;
  if (maxlen <= 0) {
    try {
      VariableSerializer vs(VariableSerializer::VarExport, 0, 2);
      value = vs.serialize(v, true);
    } catch (NestingLevelTooDeepException &e) {
      VariableSerializer vs(VariableSerializer::VarDump, 0, 2);
      value = vs.serialize(v, true);
    } catch (...) {
      ASSERT(false);
      throw;
    }
  } else {
    VariableSerializer vs(VariableSerializer::DebuggerDump, 0, 2);
    value = vs.serialize(v, true);
  }

  if (maxlen <= 0 || value.length() - maxlen < 30) {
    return value;
  }

  StringBuffer sb;
  sb.append(value.substr(0, maxlen / 2));
  sb.append(" ...(omitted ");
  sb.append(value.size() - maxlen);
  sb.append(" chars)... ");
  sb.append(value.substr(value.size() - maxlen / 2));
  return sb.detach();
}

String DebuggerClient::FormatInfoVec(const IDebuggable::InfoVec &info,
                                     int *nameLen /* = NULL */) {
  // vertical align names
  int maxlen = 0;
  for (unsigned int i = 0; i < info.size(); i++) {
    int len = strlen(info[i].first);
    if (len > maxlen) maxlen = len;
  }

  // print
  StringBuffer sb;
  for (unsigned int i = 0; i < info.size(); i++) {
    if (ItemNameColor) sb.append(ItemNameColor);
    string name = info[i].first;
    name += ":                                                        ";
    sb.append(name.substr(0, maxlen + 4));
    if (ItemNameColor) sb.append(ANSI_COLOR_END);
    if (OutputColor) sb.append(OutputColor);
    sb.append(info[i].second);
    if (OutputColor) sb.append(ANSI_COLOR_END);
    sb.append("\n");
  }
  if (nameLen) *nameLen = maxlen + 4;
  return sb.detach();
}

String DebuggerClient::FormatTitle(const char *title) {
  String dash = StringUtil::Repeat(BOX_H, (LineWidth - strlen(title)) / 2 - 4);

  StringBuffer sb;
  sb.append("\n");
  sb.append("    ");
  sb.append(dash);
  sb.append(" "); sb.append(title); sb.append(" ");
  sb.append(dash);
  sb.append("\n");
  return sb.detach();
}

///////////////////////////////////////////////////////////////////////////////

DebuggerClient::DebuggerClient()
    : m_tutorial(0),
      m_printFunction(""),
      m_mainThread(this, &DebuggerClient::run), m_stopped(false),
      m_inputState(TakingCommand), m_runState(NotYet),
      m_signum(CmdSignal::SignalNone), m_sigTime(0),
      m_acLen(0), m_acIndex(0), m_acPos(0), m_acLiveListsDirty(true),
      m_threadId(0), m_listLine(0), m_listLineFocus(0), m_frame(0) {
}

DebuggerClient::~DebuggerClient() {
}

void DebuggerClient::reset() {
  for (unsigned int i = 0; i < m_machines.size(); i++) {
    m_machines[i]->m_thrift.close();
  }

  m_stacktrace.reset();
  m_acItems.clear();
  m_acLiveLists.reset();
  m_machines.clear();
  m_machine.reset();
}

bool DebuggerClient::isLocal() {
  return m_machines[0] == m_machine;
}

bool DebuggerClient::connect(const std::string &host, int port) {
  ASSERT(!m_machines.empty());
  ASSERT(m_machines[0]->m_name == LocalPrompt);
  for (unsigned int i = 1; i < m_machines.size(); i++) {
    if (f_gethostbyname(m_machines[i]->m_name) ==
        f_gethostbyname(host)) {
      switchMachine(m_machines[i]);
      return false;
    }
  }
  return connectRemote(host, port);
}

bool DebuggerClient::connectRPC(const std::string &host, int port) {
  ASSERT(!m_machines.empty());
  DMachineInfoPtr local = m_machines[0];
  ASSERT(local->m_name == LocalPrompt);
  local->m_rpcHost = host;
  local->m_rpcPort = port;
  switchMachine(local);
  m_rpcHost = "rpc:" + host;
  return !local->m_interrupting;
}

bool DebuggerClient::disconnect() {
  ASSERT(!m_machines.empty());
  DMachineInfoPtr local = m_machines[0];
  ASSERT(local->m_name == LocalPrompt);
  local->m_rpcHost.clear();
  local->m_rpcPort = 0;
  switchMachine(local);
  return !local->m_interrupting;
}

void DebuggerClient::switchMachine(DMachineInfoPtr machine) {
  m_rpcHost.clear();
  machine->m_initialized = false; // even if m_machine == machine

  if (m_machine != machine) {
    m_machine = machine;
    m_sandboxes.clear();
    m_threads.clear();
    m_threadId = 0;
    m_breakpoint.reset();
    m_matched.clear();
    m_listFile.clear();
    m_listLine = 0;
    m_listLineFocus = 0;
    m_stacktrace.reset();
    m_frame = 0;
  }
}

SmartPtr<Socket> DebuggerClient::connectLocal() {
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    throw Exception("unable to create socket pair for local debugging");
  }
  SmartPtr<Socket> socket1(new Socket(fds[0], AF_UNIX));
  SmartPtr<Socket> socket2(new Socket(fds[1], AF_UNIX));

  DMachineInfoPtr machine(new DMachineInfo());
  machine->m_sandboxAttached = true;
  machine->m_name = LocalPrompt;
  machine->m_thrift.create(socket1);
  ASSERT(m_machines.empty());
  m_machines.push_back(machine);
  switchMachine(machine);
  return socket2;
}

bool DebuggerClient::connectRemote(const std::string &host, int port) {
  if (port <= 0) {
    port = RuntimeOption::DebuggerServerPort;
  }
  info("Connecting to %s:%d...", host.c_str(), port);
  Socket *sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET,
                            String(host), port);
  Object obj(sock);
  if (f_socket_connect(sock, String(host), port)) {
    DMachineInfoPtr machine(new DMachineInfo());
    machine->m_name = host;
    machine->m_port = port;
    machine->m_thrift.create(SmartPtr<Socket>(sock));
    m_machines.push_back(machine);
    switchMachine(machine);
    return true;
  }
  error("Unable to connect to %s:%d.", host.c_str(), port);
  return false;
}

bool DebuggerClient::reconnect() {
  ASSERT(m_machine);
  string &host = m_machine->m_name;
  int port = m_machine->m_port;
  if (port) {
    info("Re-connecting to %s:%d...", host.c_str(), port);
    Socket *sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET,
                              String(host), port);
    Object obj(sock);
    if (f_socket_connect(sock, String(host), port)) {
      for (unsigned int i = 0; i < m_machines.size(); i++) {
        if (m_machines[i] == m_machine) {
          m_machines.erase(m_machines.begin() + i);
          break;
        }
      }
      DMachineInfoPtr machine(new DMachineInfo());
      machine->m_name = host;
      machine->m_port = port;
      machine->m_thrift.create(SmartPtr<Socket>(sock));
      switchMachine(machine);
      return true;
    }
    error("Still unable to connect to %s:%d.", host.c_str(), port);
  }
  return false;
}

std::string DebuggerClient::getPrompt() {
  if (NoPrompt) {
    return "";
  }
  string *name = &m_machine->m_name;
  if (!m_rpcHost.empty()) {
    name = &m_rpcHost;
  }
  if (m_inputState == TakingCode) {
    string prompt = " ";
    for (unsigned int i = 2; i < name->size() + 2; i++) {
      prompt += '.';
    }
    prompt += ' ';
    return prompt;
  }
  return *name + "> ";
}

void DebuggerClient::start(const DebuggerClientOptions &options) {
  loadConfig();
  if (!options.cmds.empty()) {
    UseColor = false;
    s_use_utf8 = false;
    NoPrompt = true;
  }

  if (!NoPrompt) {
    info("Welcome to HipHop Debugger!");
    info("Type \"help\" or \"?\" for a complete list of commands.\n");
  }

  if (!options.host.empty()) {
    connectRemote(options.host, options.port);
  }

  m_options = options;
  m_mainThread.start();
}

void DebuggerClient::stop() {
  m_stopped = true;
  m_mainThread.waitForEnd();
}

void DebuggerClient::run() {
  ReadlineApp app;
  playMacro("startup");

  if (!m_options.cmds.empty()) {
    m_macroPlaying = MacroPtr(new Macro());
    m_macroPlaying->m_cmds = m_options.cmds;
    m_macroPlaying->m_cmds.push_back("q");
    m_macroPlaying->m_index = 0;
  }

  hphp_session_init();
  ExecutionContext *context = hphp_context_init();
  hphp_invoke_simple(m_options.extension);
  while (true) {
    try {
      runImpl();
    } catch (DebuggerServerLostException &e) {
      if (reconnect()) {
        continue;
      }
    } catch (...) {
      Logger::Error("Unhandled exception from DebuggerClient::runImpl().");
    }
    break;
  }
  reset();
  hphp_context_exit(context, false);
  hphp_session_exit();
}

///////////////////////////////////////////////////////////////////////////////
// auto-complete

void DebuggerClient::updateLiveLists() {
  ReadlineWaitCursor waitCursor;
  CmdInfo::UpdateLiveLists(this);
  m_acLiveListsDirty = false;
}

void DebuggerClient::promptFunctionPrototype() {
  if (m_acProtoTypePrompted) return;
  m_acProtoTypePrompted = true;

  const char *p0 = rl_line_buffer;
  int len = strlen(p0);
  if (len < 2) return;

  const char *pLast = p0 + len - 1;
  while (pLast > p0 && isspace(*pLast)) --pLast;
  if (pLast == p0 || *pLast-- != '(') return;
  while (pLast > p0 && isspace(*pLast)) --pLast;

  const char *p = pLast;
  while (p >= p0 && (isalnum(*p) || *p == '_')) --p;
  if (p == pLast) return;

  string cls;
  string func(p + 1, pLast - p);
  if (p > p0 && *p-- == ':' && *p-- == ':') {
    pLast = p;
    while (p >= p0 && (isalnum(*p) || *p == '_')) --p;
    if (pLast > p) {
      cls = string(p + 1, pLast - p);
    }
  }

  String output = highlight_code(CmdInfo::GetProtoType(this, cls, func));
  print("\n%s", output.data());
  rl_forced_update_display();
}

bool DebuggerClient::setCompletion(const char *text, int start, int end) {
  if (m_inputState == TakingCommand) {
    parseCommand(rl_line_buffer);
    if (*text) {
      if (!m_args.empty()) {
        m_args.resize(m_args.size() - 1);
      } else {
        m_command.clear();
      }
    }
  }
  return true;
}

void DebuggerClient::addCompletion(AutoComplete type) {
  if (type < 0 || type >= AutoCompleteCount) {
    Logger::Error("Invalid auto completion enum: %d", type);
    return;
  }

  if (type == AutoCompleteCode) {
    addCompletion(AutoCompleteVariables);
    addCompletion(AutoCompleteConstants);
    addCompletion(AutoCompleteClasses);
    addCompletion(AutoCompleteFunctions);
    addCompletion(AutoCompleteClassMethods);
    addCompletion(AutoCompleteClassProperties);
    addCompletion(AutoCompleteClassConstants);
    addCompletion(AutoCompleteKeyword);
  } else if (type == AutoCompleteKeyword) {
    addCompletion(PHP_KEYWORDS);
  } else {
    m_acLists.push_back((const char **)type);
  }

  if (type == AutoCompleteFunctions || type == AutoCompleteClassMethods) {
    rl_completion_suppress_append = 1;
    promptFunctionPrototype();
  }
}

void DebuggerClient::addCompletion(const char **list) {
  m_acLists.push_back(list);
}

void DebuggerClient::addCompletion(const char *name) {
  m_acStrings.push_back(name);
}

void DebuggerClient::addCompletion(const std::vector<String> &items) {
  m_acItems.insert(m_acItems.end(), items.begin(), items.end());
}

char *DebuggerClient::getCompletion(const std::vector<String> &items,
                                    const char *text) {
  while (++m_acPos < (int)items.size()) {
    const char *p = items[m_acPos].data();
    if (m_acLen == 0 || strncasecmp(p, text, m_acLen) == 0) {
      return strdup(p);
    }
  }
  m_acPos = -1;
  return NULL;
}

char *DebuggerClient::getCompletion(const std::vector<const char *> &items,
                                    const char *text) {
  while (++m_acPos < (int)items.size()) {
    const char *p = items[m_acPos];
    if (m_acLen == 0 || strncasecmp(p, text, m_acLen) == 0) {
      return strdup(p);
    }
  }
  m_acPos = -1;
  return NULL;
}

static char first_non_whitespace(const char *s) {
  while (*s && isspace(*s)) s++;
  return *s;
}

char *DebuggerClient::getCompletion(const char *text, int state) {
  if (state == 0) {
    m_acLen = strlen(text);
    m_acIndex = 0;
    m_acPos = -1;
    m_acLists.clear();
    m_acStrings.clear();
    m_acItems.clear();
    m_acProtoTypePrompted = false;
    if (m_inputState == TakingCommand) {
      switch (first_non_whitespace(rl_line_buffer)) {
        case '<':
          if (strncasecmp(m_command.substr(0, 5).c_str(), "<?php", 5)) {
            addCompletion("<?php");
            break;
          }
        case '@':
        case '=':
        case '$': {
          addCompletion(AutoCompleteCode);
          break;
        }
        default: {
          if (m_command.empty()) {
            addCompletion(GetCommands());
            addCompletion("@");
            addCompletion("=");
            addCompletion("<?php");
            addCompletion("?>");
          } else {
            DebuggerCommand *cmd = createCommand();
            if (cmd) {
              DebuggerCommandPtr deleter(cmd);
              cmd->list(this);
            }
          }
          break;
        }
      }
    } else {
      ASSERT(m_inputState == TakingCode);
      if (!*rl_line_buffer) {
        addCompletion("?>"); // so we tab, we're done
      } else {
        addCompletion(AutoCompleteCode);
      }
    }
  }

  for (; m_acIndex < (int)m_acLists.size(); m_acIndex++) {
    const char **list = m_acLists[m_acIndex];
    if ((int64)list == AutoCompleteFileNames) {
      char *p = rl_filename_completion_function(text, ++m_acPos);
      if (p) return p;
    } else if ((int64)list >= 0 && (int64)list < AutoCompleteCount) {
      if (m_acLiveListsDirty) {
        updateLiveLists();
        ASSERT(!m_acLiveListsDirty);
      }
      char *p = getCompletion((*m_acLiveLists)[(int64)list], text);
      if (p) return p;
    } else {
      for (const char *p = list[++m_acPos]; p; p = list[++m_acPos]) {
        if (m_acLen == 0 || strncasecmp(p, text, m_acLen) == 0) {
          return strdup(p);
        }
      }
    }
    m_acPos = -1;
  }

  char *p = getCompletion(m_acStrings, text);
  if (p) return p;

  return getCompletion(m_acItems, text);
}

///////////////////////////////////////////////////////////////////////////////
// main

bool DebuggerClient::initializeMachine() {
  if (!m_machine->m_initialized) {
    // set/clear intercept for RPC thread
    if (!m_machines.empty() && m_machine == m_machines[0]) {
      CmdMachine::UpdateIntercept(this, m_machine->m_rpcHost,
                                  m_machine->m_rpcPort);
    }

    // upload breakpoints
    if (!m_breakpoints.empty()) {
      info("Updating breakpoints...");
      CmdBreak().update(this);
    }

    // attaching to default sandbox
    if (!m_machine->m_sandboxAttached) {
      try {
        CmdMachine::AttachSandbox(this, NULL, m_options.sandbox.c_str());
      } catch (DebuggerConsoleExitException &e) {
        m_machine->m_sandboxAttached = true;
      }
      if (!m_machine->m_sandboxAttached) {
        Logger::Error("Unable to communicate with default sandbox.");
        return false;
      }

      m_machine->m_initialized = true;
      throw DebuggerConsoleExitException();
    }

    m_machine->m_initialized = true;
  }
  return true;
}

void DebuggerClient::runImpl() {
  const char *func = "DebuggerClient::runImpl()";

  try {
    while (!m_stopped) {
      DebuggerCommandPtr cmd;
      if (DebuggerCommand::Receive(m_machine->m_thrift, cmd, func)) {
        if (!cmd) {
          Logger::Error("Unable to communicate with server. Server's down?");
          throw DebuggerServerLostException();
        }
        if (cmd->is(DebuggerCommand::KindOfSignal)) {
          if (!cmd->onClient(this)) {
            Logger::Error("%s: unable to poll signal", func);
            return;
          }
          continue;
        }
        if (!cmd->is(DebuggerCommand::KindOfInterrupt)) {
          Logger::Error("%s: bad cmd type: %d", func, cmd->getType());
          return;
        }
        m_sigTime = 0;
        if (!cmd->onClient(this)) {
          Logger::Error("%s: unable to process %d", func, cmd->getType());
          return;
        }
        m_machine->m_interrupting = true;

        m_inputState = TakingCommand;
        if (!m_machine->m_initialized) {
          try {
            if (!initializeMachine()) {
              return;
            }
          } catch (DebuggerConsoleExitException &e) {
            continue;
          }
        }
        if (!console()) {
          return;
        }
        m_inputState = TakingInterrupt;
      }
    }
  } catch (DebuggerClientExitException &e) { /* normal exit */ }
}

bool DebuggerClient::console() {
  while (true) {
    const char *line = NULL;

    string holder;
    if (m_macroPlaying) {
      if (m_macroPlaying->m_index < m_macroPlaying->m_cmds.size()) {
        holder = m_macroPlaying->m_cmds[m_macroPlaying->m_index++];
        line = holder.c_str();
      } else {
        m_macroPlaying.reset();
      }
    }
    if (line == NULL) {
      line = readline(getPrompt().c_str());
      if (line == NULL) {
        return false;
      }
    } else if (!NoPrompt) {
      print("%s%s", getPrompt().c_str(), line);
    }
    if (*line) {
      // even if line is bad command, we still want to remember it, so
      // people can go back and fix typos
      add_history(line);
    }

    AdjustScreenMetrics();

    if (*line) {
      if (parse(line)) {
        try {
          record(line);
          m_prevCmd = m_command;
          if (!process()) {
            error("command \"" + m_command + "\" not found");
            m_command.clear();
          }
        } catch (DebuggerClientExitException &e) {
          return false;
        } catch (DebuggerConsoleExitException &e) {
          return true;
        } catch (DebuggerProtocolException &e) {
          return false;
        }
      }
    } else if (m_inputState == TakingCommand) {
      switch (m_prevCmd[0]) {
        case 'l': // list
          m_args.clear(); // as if just "list"
          // fall through
        case 'c': // continue
        case 's': // step
        case 'n': // next
        case 'o': // out
          try {
            record(line);
            m_command = m_prevCmd;
            process(); // replay the same command
          } catch (DebuggerConsoleExitException &e) {
            return true;
          } catch (DebuggerProtocolException &e) {
            return false;
          }
          break;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void DebuggerClient::shortCode(BreakPointInfoPtr bp) {
  if (bp && !bp->m_file.empty() && bp->m_line1) {
    Variant source = CmdList::GetSourceFile(this, bp->m_file);
    if (source.isString()) {
      code(source, bp->m_line1,
           bp->m_line1 > 1 ? bp->m_line1 - 1 : bp->m_line1,
           bp->m_line2 + 1,
           bp->m_char1, bp->m_line2, bp->m_char2);
    }
  }
}

bool DebuggerClient::code(CStrRef source, int lineFocus, int line1 /* = 0 */,
                          int line2 /* = 0 */, int charFocus0 /* = 0 */,
                          int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  if (line1 == 0 && line2 == 0) {
    String highlighted = highlight_code(source, 0, lineFocus, charFocus0,
                                        lineFocus1, charFocus1);
    if (!highlighted.empty()) {
      print(highlighted);
      return true;
    }
    return false;
  }

  String highlighted = highlight_php(source, 1, lineFocus, charFocus0,
                                     lineFocus1, charFocus1);
  int line = 1;
  const char *begin = highlighted.data();
  StringBuffer sb;
  for (const char *p = begin; *p; p++) {
    if (*p == '\n') {
      if (line >= line1) {
        sb.append(begin, p - begin + 1);
      }
      if (++line > line2) break;
      begin = p + 1;
    }
  }
  if (!sb.empty()) {
    print("%s%s", sb.data(), ANSI_COLOR_END);
    return true;
  }
  return false;
}

char DebuggerClient::ask(const char *fmt, ...) {
  string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap); va_end(ap);
  if (UseColor && InfoColor) {
    msg = InfoColor + msg + ANSI_COLOR_END;
  }
  fwrite(msg.data(), 1, msg.length(), stdout);
  fflush(stdout);
  return tolower(getchar());
}

void DebuggerClient::print(const char *fmt, ...) {
  string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap); va_end(ap);
  print(msg);
}

void DebuggerClient::print(const std::string &s) {
  fwrite(s.data(), 1, s.length(), stdout);
  fwrite("\n", 1, 1, stdout);
  fflush(stdout);
}

void DebuggerClient::print(CStrRef msg) {
  fwrite(msg.data(), 1, msg.length(), stdout);
  fwrite("\n", 1, 1, stdout);
  fflush(stdout);
}

#define IMPLEMENT_COLOR_OUTPUT(name, where, color)                      \
  void DebuggerClient::name(CStrRef msg) {                              \
    if (UseColor && color) {                                            \
      fwrite(color, 1, strlen(color), where);                           \
    }                                                                   \
    fwrite(msg.data(), 1, msg.length(), where);                         \
    if (UseColor && color) {                                            \
      fwrite(ANSI_COLOR_END, 1, strlen(ANSI_COLOR_END), where);         \
    }                                                                   \
    fwrite("\n", 1, 1, where);                                          \
    fflush(where);                                                      \
  }                                                                     \
  void DebuggerClient::name(const char *fmt, ...) {                     \
    string msg;                                                         \
    va_list ap;                                                         \
    va_start(ap, fmt);                                                  \
    Util::string_vsnprintf(msg, fmt, ap); va_end(ap);                   \
    name(msg);                                                          \
  }                                                                     \
  void DebuggerClient::name(const std::string &msg) {                   \
    name(String(msg.data(), msg.length(), AttachLiteral));              \
  }                                                                     \

IMPLEMENT_COLOR_OUTPUT(help,     stdout,  HelpColor);
IMPLEMENT_COLOR_OUTPUT(info,     stdout,  InfoColor);
IMPLEMENT_COLOR_OUTPUT(output,   stdout,  OutputColor);
IMPLEMENT_COLOR_OUTPUT(error,    stderr,  ErrorColor);

string DebuggerClient::wrap(const std::string &s) {
  String ret = StringUtil::WordWrap(String(s.c_str(), s.size(), AttachLiteral),
                                    LineWidth - 4, "\n", true);
  return string(ret.data(), ret.size());
}

void DebuggerClient::helpTitle(const char *title) {
  help(FormatTitle(title));
}

void DebuggerClient::helpCmds(const char *cmd, const char *desc, ...) {
  std::vector<const char *> cmds;
  cmds.push_back(cmd);
  cmds.push_back(desc);

  va_list ap;
  va_start(ap, desc);
  const char *s = va_arg(ap, const char *);
  while (s) {
    cmds.push_back(s);
    s = va_arg(ap, const char *);
  }
  va_end(ap);

  helpCmds(cmds);
}

void DebuggerClient::helpCmds(const std::vector<const char *> &cmds) {
  int left = 0; int right = 0;
  for (unsigned int i = 0; i < cmds.size(); i++) {
    int &width = (i % 2 ? right : left);
    int len = strlen(cmds[i]);
    if (width < len) width = len;
  }

  int margin = 8;
  int leftMax = LineWidth / 3 - margin;
  int rightMax = LineWidth * 3 / 3 - margin;
  if (left > leftMax) left = leftMax;
  if (right > rightMax) right = rightMax;

  StringBuffer sb;
  for (unsigned int i = 0; i < cmds.size() - 1; i += 2) {
    String cmd(cmds[i], AttachLiteral);
    String desc(cmds[i+1], AttachLiteral);

    // two special formats
    if (cmd.empty() && desc.empty()) {
      sb.append("\n");
      continue;
    }
    if (desc.empty()) {
      sb.append(FormatTitle(cmd.data()));
      sb.append("\n");
      continue;
    }

    cmd = StringUtil::WordWrap(cmd, left, "\n", true);
    desc = StringUtil::WordWrap(desc, right, "\n", true);
    Array lines1 = StringUtil::Explode(cmd, "\n").toArray();
    Array lines2 = StringUtil::Explode(desc, "\n").toArray();
    for (int n = 0; n < lines1.size() || n < lines2.size(); n++) {
      StringBuffer line;
      line.append("    ");
      if (n) line.append("  ");
      line.append(StringUtil::Pad(lines1[n], leftMax));
      if (n == 0) line.append("  ");
      line.append("  ");
      line.append(lines2[n].toString());

      sb.append(StringUtil::Trim(line.detach(), StringUtil::TrimRight));
      sb.append("\n");
    }
  }

  help(sb.detach());
}

void DebuggerClient::helpBody(const std::string &s) {
  help("");
  help(wrap(s));
  help("");
}

void DebuggerClient::helpSection(const std::string &s) {
  help(wrap(s));
}

void DebuggerClient::tutorial(const char *text) {
  if (m_tutorial < 0) return;

  String ret = String(text).replace("\t", "    ");
  ret = StringUtil::WordWrap(ret, LineWidth - 4, "\n", true);
  Array lines = StringUtil::Explode(ret, "\n").toArray();

  StringBuffer sb;
  String header = "  Tutorial - '[h]elp [t]utorial off' to turn off  ";
  String hr = StringUtil::Repeat(BOX_H, LineWidth - 2);

  sb.append(BOX_UL); sb.append(hr); sb.append(BOX_UR); sb.append("\n");

  int wh = (LineWidth - 2 - header.size()) / 2;
  sb.append(BOX_V);
  sb.append(StringUtil::Repeat(" ", wh));
  sb.append(header);
  sb.append(StringUtil::Repeat(" ", wh));
  sb.append(BOX_V);
  sb.append("\n");

  sb.append(BOX_VL); sb.append(hr); sb.append(BOX_VR); sb.append("\n");
  for (ArrayIter iter(lines); iter; ++iter) {
    sb.append(BOX_V); sb.append(' ');
    sb.append(StringUtil::Pad(iter.second(), LineWidth - 4));
    sb.append(' '); sb.append(BOX_V); sb.append("\n");
  }
  sb.append(BOX_LL); sb.append(hr); sb.append(BOX_LR); sb.append("\n");

  String content = sb.detach();

  if (m_tutorial == 0) {
    String hash = StringUtil::MD5(content);
    if (m_tutorialVisited.find(hash.data()) != m_tutorialVisited.end()) {
      return;
    }
    m_tutorialVisited.insert(hash.data());
    saveConfig();
  }

  help(content);
}

void DebuggerClient::setTutorial(int mode) {
  if (m_tutorial != mode) {
    m_tutorial = mode;
    m_tutorialVisited.clear();
    saveConfig();
  }
}

///////////////////////////////////////////////////////////////////////////////
// command processing

const char **DebuggerClient::GetCommands() {
  static const char *cmds[] = {
    "abort",    "break",    "continue",   "down",    "exception",
    "frame",    "global",   "help",       "info",    "jump",
    "konstant", "list",     "machine",    "next",    "out",
    "print",    "quit",     "run",        "step",    "thread",
    "up",       "variable", "where",      "x",       "y",
    "zend",     "!",        "&",
    NULL
  };
  return cmds;
}

void DebuggerClient::shiftCommand() {
  if (m_command.size() > 1) {
    m_args.insert(m_args.begin(), m_command.substr(1));
    m_command = m_command.substr(0, 1);
  }
}

DebuggerCommand *DebuggerClient::createCommand() {
  // give gdb users some love
  if (m_command == "bt") return new CmdWhere();

  switch (tolower(m_command[0])) {
    case 'a': return (match("abort")     ) ? new CmdAbort    () : NULL;
    case 'b': return (match("break")     ) ? new CmdBreak    () : NULL;
    case 'c': return (match("continue")  ) ? new CmdContinue () : NULL;
    case 'd': return (match("down")      ) ? new CmdDown     () : NULL;
    case 'e': return (match("exception") ) ? new CmdException() : NULL;
    case 'f': return (match("frame")     ) ? new CmdFrame    () : NULL;
    case 'g': return (match("global")    ) ? new CmdGlobal   () : NULL;
    case 'h': return (match("help")      ) ? new CmdHelp     () : NULL;
    case 'i': return (match("info")      ) ? new CmdInfo     () : NULL;
    case 'j': return (match("jump")      ) ? new CmdJump     () : NULL;
    case 'k': return (match("konstant")  ) ? new CmdConstant () : NULL;
    case 'l': return (match("list")      ) ? new CmdList     () : NULL;
    case 'm': return (match("machine")   ) ? new CmdMachine  () : NULL;
    case 'n': return (match("next")      ) ? new CmdNext     () : NULL;
    case 'o': return (match("out")       ) ? new CmdOut      () : NULL;
    case 'p': return (match("print")     ) ? new CmdPrint    () : NULL;
    case 'q': return (match("quit")      ) ? new CmdQuit     () : NULL;
    case 'r': return (match("run")       ) ? new CmdRun      () : NULL;
    case 's': return (match("step")      ) ? new CmdStep     () : NULL;
    case 't': return (match("thread")    ) ? new CmdThread   () : NULL;
    case 'u': return (match("up")        ) ? new CmdUp       () : NULL;
    case 'v': return (match("variable")  ) ? new CmdVariable () : NULL;
    case 'w': return (match("where")     ) ? new CmdWhere    () : NULL;
    case 'z': return (match("zend")      ) ? new CmdZend     () : NULL;

    // these single lettter commands allow "x{cmd}" and "x {cmd}"
    case 'x': shiftCommand(); return new CmdExtended();
    case 'y': shiftCommand(); return new CmdUser();
    case '!': shiftCommand(); return new CmdShell();
    case '&': shiftCommand(); return new CmdMacro();
  }
  return NULL;
}

bool DebuggerClient::process() {
  switch (tolower(m_command[0])) {
    case '@':
    case '=':
    case '$': return processTakeCode();
    case '<': return match("<?php") && processTakeCode();
    case '?':
      if (match("?"))  return CmdHelp().onClient(this);
      if (match("?>")) return processEval();
      break;
    default: {
      DebuggerCommand *cmd = createCommand();
      if (cmd) {
        DebuggerCommandPtr deleter(cmd);
        return cmd->onClient(this);
      }
      break;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void DebuggerClient::addToken(std::string &token) {
  if (m_command.empty()) {
    m_command = token;
  } else {
    m_args.push_back(token);
  }
  token.clear();
}

void DebuggerClient::parseCommand(const char *line) {
  m_command.clear();
  m_args.clear();

  char quote = 0;
  string token;
  for (const char *p = line; *p; p++) {
    char ch = *p;
    switch (ch) {
      case ' ':
        if (!quote) {
          if (!token.empty()) {
            addToken(token);
          }
        } else {
          token += ch;
        }
        break;
      case '"':
      case '\'':
        if (token.empty() && quote == 0) {
          quote = ch;
          break;
        }
        if (quote == ch && (p[1] == ' ' || p[1] == 0)) {
          addToken(token);
          quote = 0;
          break;
        }
        token += ch;
        break;
      case '\\':
        if (p[1]) {
          p++;
          token += *p;
        }
        break;
      default:
        token += ch;
        break;
    }
  }
  if (!token.empty()) {
    addToken(token);
  }
}

bool DebuggerClient::parse(const char *line) {
  if (m_inputState != TakingCode) {
    while (isspace(*line)) line++;
  }
  m_line = line;

  if (m_inputState == TakingCommand) {
    parseCommand(line);
    return true;
  }

  if (m_inputState == TakingCode) {
    int pos = checkEvalEnd();
    if (pos >= 0) {
      if (pos > 0) {
        m_code += m_line.substr(0, pos);
      }
      processEval();
    } else {
      if (!strncasecmp(m_line.c_str(), "abort", m_line.size())) {
        m_code.clear();
        m_inputState = TakingCommand;
        return false;
      }
      m_code += m_line + "\n";
    }
  }

  return false;
}

bool DebuggerClient::match(const char *cmd) {
  ASSERT(cmd && *cmd);
  return !strncasecmp(m_command.c_str(), cmd, m_command.size());
}

bool DebuggerClient::Match(const char *input, const char *cmd) {
  return !strncasecmp(input, cmd, strlen(input));
}

bool DebuggerClient::arg(int index, const char *s) {
  ASSERT(s && *s);
  ASSERT(index > 0);
  --index;
  return (int)m_args.size() > index &&
    !strncasecmp(m_args[index].c_str(), s, m_args[index].size());
}

std::string DebuggerClient::argValue(int index) {
  ASSERT(index > 0);
  --index;
  if (index >= 0 && index < (int)m_args.size()) {
    return m_args[index];
  }
  return "";
}

std::string DebuggerClient::argRest(int index) {
  ASSERT(index > 0);
  --index;
  if (index >= 0 && index < (int)m_args.size()) {
    string ret = m_args[index];
    while (++index < (int)m_args.size()) {
      ret += " " + m_args[index];
    }
    return ret;
  }
  return "";
}

///////////////////////////////////////////////////////////////////////////////
// comunication with DebuggerProxy

DebuggerCommandPtr DebuggerClient::xend(DebuggerCommand *cmd) {
  return send(cmd, cmd->getType());
}

DebuggerCommandPtr DebuggerClient::send(DebuggerCommand *cmd, int expected) {
  const char *func = "DebuggerClient::send()";

  if (cmd->send(m_machine->m_thrift)) {
    DebuggerCommandPtr res;
    if (!expected) {
      return res;
    }

    while (true) {
      while (!DebuggerCommand::Receive(m_machine->m_thrift, res, func)) {
        if (m_stopped) throw DebuggerClientExitException();
      }
      if (!res) {
        Logger::Error("Unable to communicate with server. Server's down?");
        throw DebuggerServerLostException();
      }
      if (res->is((DebuggerCommand::Type)expected)) {
        return res;
      }

      if (!res->is(DebuggerCommand::KindOfInterrupt)) {
        Logger::Error("%s: unexpected return: %d", func, res->getType());
        throw DebuggerProtocolException();
      }

      // eval() can cause more breakpoints
      if (!res->onClient(this) || !console()) {
        Logger::Error("%s: unable to process %d", func, res->getType());
        throw DebuggerProtocolException();
      }
    }
  }

  throw DebuggerProtocolException();
}

///////////////////////////////////////////////////////////////////////////////
// helpers

int DebuggerClient::checkEvalEnd() {
  size_t pos = m_line.rfind("?>");
  if (pos == string::npos) {
    return -1;
  }

  for (size_t p = pos + 2; p < m_line.size(); p++) {
    if (!isspace(m_line[p])) {
      return -1;
    }
  }

  return pos;
}

bool DebuggerClient::processTakeCode() {
  ASSERT(m_inputState == TakingCommand);

  char first = m_line[0];
  if (first == '@') {
    m_code = string("<?php ") + (m_line.c_str() + 1) + ";";
    return processEval();
  }
  if (first == '=') {
    m_code = string("<?php $_") + m_line + "; " + m_printFunction;
    return processEval();
  }
  if (first == '$') {
    m_code = "<?php ";
    m_code += m_line + ";";
    return processEval();
  }
  ASSERT(first == '<');
  m_code = "<?php ";
  m_code += m_line.substr(m_command.length()) + "\n";
  m_inputState = TakingCode;

  int pos = checkEvalEnd();
  if (pos >= 0) {
    m_code.resize(m_code.size() - m_line.size() + pos - 1);
    return processEval();
  }
  return true;
}

bool DebuggerClient::processEval() {
  m_runState = Running;
  CmdEval().onClient(this);
  m_inputState = TakingCommand;
  m_acLiveListsDirty = true;
  return true;
}

void DebuggerClient::swapHelp() {
  ASSERT(m_args.size() > 0);
  m_command = m_args[0];
  m_args[0] = "help";
}

void DebuggerClient::quit() {
  for (unsigned int i = 0; i < m_machines.size(); i++) {
    m_machines[i]->m_thrift.close();
  }
  throw DebuggerClientExitException();
}

DSandboxInfoPtr DebuggerClient::getSandbox(int index) const {
  if (index > 0) {
    --index;
    if (index >= 0 && index < (int)m_sandboxes.size()) {
      return m_sandboxes[index];
    }
  }
  return DSandboxInfoPtr();
}

void DebuggerClient::updateThreads(DThreadInfoPtrVec threads) {
  m_threads = threads;
  for (unsigned int i = 0; i < m_threads.size(); i++) {
    DThreadInfoPtr thread = m_threads[i];
    map<int64, int>::const_iterator iter =
      m_threadIdMap.find(thread->m_id);
    if (iter != m_threadIdMap.end()) {
      m_threads[i]->m_index = iter->second;
    } else {
      int index = m_threadIdMap.size() + 1;
      m_threadIdMap[thread->m_id] = index;
      m_threads[i]->m_index = index;
    }
  }
}

DThreadInfoPtr DebuggerClient::getThread(int index) const {
  for (unsigned int i = 0; i < m_threads.size(); i++) {
    if (m_threads[i]->m_index == index) {
      return m_threads[i];
    }
  }
  return DThreadInfoPtr();
}

void DebuggerClient::getListLocation(std::string &file, int &line,
                                     int &lineFocus0, int &charFocus0,
                                     int &lineFocus1, int &charFocus1) {
  lineFocus0 = charFocus0 = lineFocus1 = charFocus1 = 0;
  if (m_listFile.empty() && m_breakpoint) {
    setListLocation(m_breakpoint->m_file, m_breakpoint->m_line1, true);
    lineFocus0 = m_breakpoint->m_line1;
    charFocus0 = m_breakpoint->m_char1;
    lineFocus1 = m_breakpoint->m_line2;
    charFocus1 = m_breakpoint->m_char2;
  } else if (m_listLineFocus) {
    lineFocus0 = m_listLineFocus;
  }
  file = m_listFile;
  line = m_listLine;
}

void DebuggerClient::setListLocation(const std::string &file, int line,
                                     bool center) {
  m_listFile = file;
  if (!m_listFile.empty() && m_listFile[0] != '/' && !SourceRoot.empty()) {
    if (SourceRoot[SourceRoot.size() - 1] != '/') {
      SourceRoot += "/";
    }
    m_listFile = SourceRoot + m_listFile;
  }

  m_listLine = line;
  if (center && m_listLine) {
    m_listLineFocus = m_listLine;
    m_listLine -= CodeBlockSize / 2;
    if (m_listLine < 0) {
      m_listLine = 0;
    }
  }
}

void DebuggerClient::setSourceRoot(const std::string &sourceRoot) {
  m_config["SourceRoot"] = SourceRoot = sourceRoot;
  saveConfig();

  // apply change right away
  setListLocation(m_listFile, m_listLine, true);
}

void DebuggerClient::setMatchedBreakPoints(BreakPointInfoPtrVec breakpoints) {
  m_matched = breakpoints;
}

void DebuggerClient::setCurrentLocation(int64 threadId,
                                        BreakPointInfoPtr breakpoint) {
  m_threadId = threadId;
  m_breakpoint = breakpoint;
  m_stacktrace.reset();
  m_listFile.clear();
  m_listLine = 0;
  m_listLineFocus = 0;
  m_acLiveListsDirty = true;
}

void DebuggerClient::addWatch(const char *fmt, const std::string &php) {
  WatchPtr watch(new Watch());
  watch->first = fmt;
  watch->second = php;
  m_watches.push_back(watch);
}

void DebuggerClient::setStackTrace(CArrRef stacktrace) {
  m_stacktrace = stacktrace;
  m_frame = 0;
}

void DebuggerClient::moveToFrame(int index, bool display /* = true */) {
  m_frame = index;
  if (m_frame >= m_stacktrace.size()) {
    m_frame = m_stacktrace.size() - 1;
  }
  if (m_frame < 0) {
    m_frame = 0;
  }
  CArrRef frame = m_stacktrace[m_frame];
  if (!frame.isNull()) {
    String file = frame["file"];
    int line = frame["line"].toInt32();
    if (!file.empty() && line) {
      if (m_frame == 0) {
        m_listFile.clear();
        m_listLine = 0;
        m_listLineFocus = 0;
      } else {
        setListLocation(file.data(), line, true);
      }
    }
    if (display) {
      printFrame(m_frame, frame);
    }
  }
}

void DebuggerClient::printFrame(int index, CArrRef frame) {
  StringBuffer args;
  for (ArrayIter iter(frame["args"]); iter; ++iter) {
    if (!args.empty()) args.append(", ");
    String value = FormatVariable(iter.second());
    args.append(value);
  }

  StringBuffer func;
  if (frame.exists("namespace")) {
    func.append(frame["namespace"].toString());
    func.append("::");
  }
  if (frame.exists("class")) {
    func.append(frame["class"].toString());
    func.append("::");
  }
  func.append(frame["function"].toString());

  String sindex(index);
  print("#%s  %s (%s)\n %s  at %s:%d",
        sindex.data(),
        func.data() ? func.data() : "",
        args.data() ? args.data() : "",
        String("           ").substr(0, sindex.size()).data(),
        frame["file"].toString().data(),
        (int)frame["line"].toInt32());
}

void DebuggerClient::startMacro(std::string name) {
  if (m_macroRecording &&
      ask("We are recording a macro. Do you want to save? [Y/n]") != 'n') {
    endMacro();
  }

  if (name.empty()) {
    name = "default";
  } else {
    for (unsigned int i = 0; i < m_macros.size(); i++) {
      if (m_macros[i]->m_name == name) {
        if (ask("This will overwrite existing one. Proceed? [y/N]") != 'y') {
          return;
        }
      }
      break;
    }
  }
  m_macroRecording = MacroPtr(new Macro());
  m_macroRecording->m_name = name;
}

void DebuggerClient::endMacro() {
  if (!m_macroRecording) {
    error("There is no ongoing recording.");
    tutorial("Use '& [s]tart {name}' or '& [s]tart' command to start "
             "macro recording first.");
    return;
  }

  bool found = false;
  for (unsigned int i = 0; i < m_macros.size(); i++) {
    if (m_macros[i]->m_name == m_macroRecording->m_name) {
      m_macros[i] = m_macroRecording;
      found = true;
      break;
    }
  }
  if (!found) {
    m_macros.push_back(m_macroRecording);
  }
  saveConfig();
  m_macroRecording.reset();
}

bool DebuggerClient::playMacro(std::string name) {
  if (name.empty()) {
    name = "default";
  }
  for (unsigned int i = 0; i < m_macros.size(); i++) {
    if (m_macros[i]->m_name == name) {
      m_macroPlaying = m_macros[i];
      m_macroPlaying->m_index = 0;
      return true;
    }
  }
  return false;
}

bool DebuggerClient::deleteMacro(int index) {
  --index;
  if (index >= 0 && index < (int)m_macros.size()) {
    if (ask("Are you sure you want to delete the macro? [y/N]") != 'y') {
      return true;
    }
    m_macros.erase(m_macros.begin() + index);
    saveConfig();
    return true;
  }
  return false;
}

void DebuggerClient::record(const char *line) {
  ASSERT(line);
  if (m_macroRecording && line[0] != '&') {
    m_macroRecording->m_cmds.push_back(line);
  }
}

///////////////////////////////////////////////////////////////////////////////
// configuration

void DebuggerClient::loadConfig() {
  m_configFileName = Process::GetHomeDirectory() + ConfigFileName;

  // make sure file exists
  FILE *f = fopen(m_configFileName.c_str(), "a");
  if (f) fclose(f);

  try {
    m_config.open(m_configFileName);
  } catch (const HdfException &e) {
    Logger::Error("Unable to load configuration file: %s", e.what());
    m_configFileName.clear();
  }

  s_use_utf8 = m_config["UTF8"].getBool(true);
  m_config["UTF8"] = s_use_utf8; // for starter

  Hdf color = m_config["Color"];
  UseColor = color.getBool(true);
  color = UseColor; // for starter
  if (UseColor) {
    defineColors(); // (1) no one can overwrite, (2) for starter
    LoadColors(color);
  }

  m_tutorial = m_config["Tutorial"].getInt32(0);
  std::string pprint = m_config["PrettyPrint"].getString("hphpd_print_value");

  m_printFunction = (boost::format(
    "(function_exists(\"%s\") ? %s($_) : print_r($_));")  % pprint % pprint)
    .str();

  m_config["Tutorial"]["Visited"].get(m_tutorialVisited);

  for (Hdf node = m_config["Macros"].firstChild(); node.exists();
       node = node.next()) {
    MacroPtr macro(new Macro());
    macro->load(node);
    m_macros.push_back(macro);
  }

  SourceRoot = m_config["SourceRoot"].getString();

  saveConfig(); // so to generate a starter for people
}

void DebuggerClient::saveConfig() {
  if (m_configFileName.empty()) {
    // we are not touching a file that was not successfully loaded earlier
    return;
  }

  m_config["Tutorial"] = m_tutorial;
  Hdf visited = m_config["Tutorial"]["Visited"];
  unsigned int i = 0;
  for (set<string>::const_iterator iter = m_tutorialVisited.begin();
       iter != m_tutorialVisited.end(); ++iter) {
    visited[i++] = *iter;
  }

  m_config.remove("Macros");
  Hdf macros = m_config["Macros"];
  for (i = 0; i < m_macros.size(); i++) {
    m_macros[i]->save(macros[i]);
  }

  m_config.write(m_configFileName);
}

void DebuggerClient::defineColors() {
  vector<string> names;
  Util::get_supported_colors(names);
  Hdf support = m_config["Color"]["SupportedNames"];
  for (unsigned int i = 0; i < names.size(); i++) {
    support[i+1] = names[i];
  }

  Hdf emacs = m_config["Color"]["Palette"]["emacs"];
  emacs["Keyword"]     = "CYAN";
  emacs["Comment"]     = "RED";
  emacs["String"]      = "GREEN";
  emacs["Variable"]    = "BROWN";
  emacs["Html"]        = "GRAY";
  emacs["Tag"]         = "MAGENTA";
  emacs["Declaration"] = "BLUE";
  emacs["Constant"]    = "MAGENTA";
  emacs["LineNo"]      = "GRAY";

  Hdf vim = m_config["Color"]["Palette"]["vim"];
  vim["Keyword"]       = "MAGENTA";
  vim["Comment"]       = "BLUE";
  vim["String"]        = "RED";
  vim["Variable"]      = "CYAN";
  vim["Html"]          = "GRAY";
  vim["Tag"]           = "MAGENTA";
  vim["Declaration"]   = "WHITE";
  vim["Constant"]      = "WHITE";
  vim["LineNo"]        = "GRAY";
}

///////////////////////////////////////////////////////////////////////////////
}}
