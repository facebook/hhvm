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
#include <boost/scoped_ptr.hpp>

#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>
#include <readline/history.h>

using namespace HPHP::Util::TextArt;

#define PHP_WORD_BREAK_CHARACTERS " \t\n\"\\'`@=;,|{[()]}+*%^!~&"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static boost::scoped_ptr<DebuggerClient> debugger_client;

static DebuggerClient& getStaticDebuggerClient() {
  /*
   * DebuggerClient acquires global mutexes in its constructor, so we
   * allocate s_debugger_client lazily to ensure that all of the
   * global mutexes have been initialized before we enter the
   * constructor.
   *
   * This initialization is thread-safe because program_functions.cpp
   * must call Debugger::StartClient (which ends up here) before any
   * additional threads are created.
   */
  if (!debugger_client) {
    debugger_client.reset(new DebuggerClient);
  }
  return *debugger_client;
}

void shutdown_hphpd() {
  if (debugger_client) {
    debugger_client->resetSmartAllocatedMembers();
    debugger_client.reset();
  }
}

///////////////////////////////////////////////////////////////////////////////
// readline setups

static char* debugger_generator(const char* text, int state) {
  return getStaticDebuggerClient().getCompletion(text, state);
}

static char **debugger_completion(const char *text, int start, int end) {
  if (getStaticDebuggerClient().setCompletion(text, start, end)) {
    return rl_completion_matches((char*)text, &debugger_generator);
  }
  return NULL;
}

static void debugger_signal_handler(int sig) {
  getStaticDebuggerClient().onSignal(sig);
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
      usageLogSignal();
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
std::string DebuggerClient::HomePrefix = "/home";

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
  SmartPtr<Socket> ret = getStaticDebuggerClient().connectLocal();
  getStaticDebuggerClient().start(options);
  return ret;
}

void DebuggerClient::Stop() {
  getStaticDebuggerClient().stop();
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

String DebuggerClient::FormatVariable(CVarRef v, int maxlen /* = 80 */,
                                      bool vardump /* = false */) {
  String value;
  if (maxlen <= 0) {
    try {
      VariableSerializer::Type t = vardump ?
                                   VariableSerializer::VarDump :
                                   VariableSerializer::PrintR;
      VariableSerializer vs(t, 0, 2);
      value = vs.serialize(v, true);
    } catch (StringBufferLimitException &e) {
      value = "Serialization limit reached";
    } catch (...) {
      ASSERT(false);
      throw;
    }
  } else {
    VariableSerializer vs(VariableSerializer::DebuggerDump, 0, 2);
    value = vs.serializeWithLimit(v, maxlen);
  }

  if (maxlen <= 0 || value.length() - maxlen < 30) {
    return value;
  }

  StringBuffer sb;
  sb.append(value.substr(0, maxlen));
  sb.append(" ...(omitted)");
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

DebuggerClient::DebuggerClient(std::string name /* = "" */)
    : m_tutorial(0), m_printFunction(""),
      m_logFile(""), m_logFileHandler(NULL),
      m_mainThread(this, &DebuggerClient::run), m_stopped(false),
      m_quitting(false),
      m_inputState(TakingCommand), m_runState(NotYet),
      m_signum(CmdSignal::SignalNone), m_sigTime(0),
      m_acLen(0), m_acIndex(0), m_acPos(0), m_acLiveListsDirty(true),
      m_threadId(0), m_listLine(0), m_listLineFocus(0), m_frame(0),
      m_clientState(StateUninit), m_inApiUse(false),
      m_nameForApi(name), m_usageLogFP(NULL) {
  initUsageLogging();
}

DebuggerClient::~DebuggerClient() {
  m_stopped = true;
  m_mainThread.waitForEnd();
  FILE *f = getLogFileHandler();
  if (f != NULL) {
    fclose(f);
    setLogFileHandler(NULL);
  }
  finiUsageLogging();
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
  return !isApiMode() && m_machines[0] == m_machine;
}

bool DebuggerClient::connect(const std::string &host, int port) {
  ASSERT(isApiMode() ||
         (!m_machines.empty() && m_machines[0]->m_name == LocalPrompt));
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

  socket1->unregister();
  socket2->unregister();
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
  // Ensure the socket is not swept---it is cached across requests in
  // API mode, and in client mode we expect to destruct it ourselves
  // when ~DebuggerClient runs.
  sock->unregister();
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
    sock->unregister();
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

void DebuggerClient::init(const DebuggerClientOptions &options) {
  m_options = options;

  if (isApiMode()) {
    if (options.user.empty()) {
      return;
    }
    m_configFileName = options.configFName;
    if (m_configFileName.empty()) {
      m_configFileName = HomePrefix + "/" + options.user + "/" +
                         ConfigFileName;
    }
  } else {
    if (!options.configFName.empty()) {
      m_configFileName = options.configFName;
    }
    m_options.user = Process::GetCurrentUser();
  }

  usageLogInit();

  loadConfig();

  if (!options.cmds.empty()) {
    UseColor = false;
    s_use_utf8 = false;
    NoPrompt = true;
  }

  if (options.apiMode) {
    UseColor = false;
    NoPrompt = true;
    m_outputBuf.clear();
  }

  if (!NoPrompt) {
    info("Welcome to HipHop Debugger!");
    info("Type \"help\" or \"?\" for a complete list of commands.\n");
  }

  if (!options.host.empty()) {
    connectRemote(options.host, options.port);
  }
}

void DebuggerClient::start(const DebuggerClientOptions &options) {
  init(options);
  m_mainThread.start();
}

void DebuggerClient::stop() {
  m_stopped = true;
  m_mainThread.waitForEnd();
}

void DebuggerClient::run() {
  // Make sure we don't run the interface thread for API mode
  ASSERT(!isApiMode());

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
  if (m_options.extension.empty()) {
    hphp_invoke_simple("", true); // warm-up only
  } else {
    hphp_invoke_simple(m_options.extension);
  }
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
  // We are about to exit from client and idealy we should cleanup,
  // but we the reset() where will try to cleanup Socket under DMachineInfo,
  // which is created by another thread. If it's cleaned up here, later we'll
  // have a SEGV when trying to sweep the object from the other thread.
  // We should refactor the code to avoid Sweepable object being passed across
  // threads later.
  // reset();
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

void DebuggerClient::addCompletion(const std::vector<std::string> &items) {
  m_acItems.insert(m_acItems.end(), items.begin(), items.end());
}

char *DebuggerClient::getCompletion(const std::vector<std::string> &items,
                                    const char *text) {
  while (++m_acPos < (int)items.size()) {
    const char *p = items[m_acPos].c_str();
    if (m_acLen == 0 || strncasecmp(p, text, m_acLen) == 0) {
      return strdup(p);
    }
  }
  m_acPos = -1;
  return NULL;
}

std::vector<std::string> DebuggerClient::getAllCompletions(
  std::string const &text) {
  std::vector<std::string> res;

  if (m_acLiveListsDirty) {
    updateLiveLists();
  }

  for (int i = 0; i < AutoCompleteCount; ++i) {
    const std::vector<std::string> &items = (*m_acLiveLists)[i];
    for (size_t j = 0; j < items.size(); ++j) {
      const char *p = items[j].c_str();
      if (strncasecmp(p, text.c_str(), text.length()) == 0) {
        res.push_back(std::string(p));
      }
    }
  }
  return res;
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
              if (cmd->is(DebuggerCommand::KindOfRun)) playMacro("startup");
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
    int waitForgSandbox = false;
    if (!m_machine->m_sandboxAttached) {
      const char *user = m_options.user.empty() ?
                         NULL : m_options.user.c_str();
      m_machine->m_sandboxAttached = (waitForgSandbox =
        CmdMachine::AttachSandbox(this, user, m_options.sandbox.c_str()));
      if (!m_machine->m_sandboxAttached) {
        Logger::Error("Unable to communicate with default sandbox.");
      }
    }

    m_machine->m_initialized = true;
    if (!isApiMode() && waitForgSandbox) {
      // Throw exception here to wait for next interrupt from server
      throw DebuggerConsoleExitException();
    }
  }
  return true;
}

DebuggerCommandPtr DebuggerClient::waitForNextInterrupt() {
  const char *func = "DebuggerClient::waitForNextInterrupt()";
  while (!m_stopped) {
    DebuggerCommandPtr cmd;
    if (DebuggerCommand::Receive(m_machine->m_thrift, cmd, func)) {
      if (!cmd) {
        Logger::Error("Unable to communicate with server.");
        return DebuggerCommandPtr();
      }
      if (cmd->is(DebuggerCommand::KindOfSignal)) {
        if (!cmd->onClientD(this)) {
          Logger::Error("Unable to handle signal command.");
          return DebuggerCommandPtr();
        }
        continue;
      }
      if (!cmd->is(DebuggerCommand::KindOfInterrupt)) {
        int expected = DebuggerCommand::KindOfInterrupt;
        if (!m_pendingCommands.empty() &&
            cmd->is((DebuggerCommand::Type)m_pendingCommands.back())) {
          m_pendingCommands.pop_back();
          // found a match, fall through
        } else {
          Logger::Error("Bad cmd type: %d expecting %d", cmd->getType(),
                        expected);
          return DebuggerCommandPtr();
        }
      }
      m_sigTime = 0;
      m_machine->m_interrupting = true;
      setClientState(StateReadyForCommand);
      m_inputState = TakingCommand;
      usageLogInterrupt(cmd);
      return cmd;
    }
  }
  return DebuggerCommandPtr();
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
          if (!cmd->onClientD(this)) {
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
        usageLogInterrupt(cmd);
        {
          if (!cmd->onClientD(this)) {
            Logger::Error("%s: unable to process %d", func, cmd->getType());
            return;
          }
        }
        m_machine->m_interrupting = true;
        setClientState(StateReadyForCommand);
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
        setClientState(StateBusy);
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
        // treat ^D as quit
        try {
          print("quit");
          quit();
        } catch (DebuggerClientExitException &e) {
          return false;
        }
      }
    } else if (!NoPrompt) {
      print("%s%s", getPrompt().c_str(), line);
    }
    if (*line && !m_macroPlaying) {
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

String DebuggerClient::getPrintString() {
  String s(m_outputBuf); // makes a copy;
  m_outputBuf.clear();
  return s;
}

Array DebuggerClient::getOutputArray() {
  Array ret;
  switch (m_outputType) {
    case OTCodeLoc:
      ret.set("output_type", "code_loc");
      ret.set("file", m_otFile);
      ret.set("line_no", m_otLineNo);
      ret.set("watch_values", m_otValues);
      break;
    case OTStacktrace:
      ret.set("output_type", "stacktrace");
      ret.set("stacktrace", m_stacktrace);
      ret.set("frame", m_frame);
      break;
    case OTValues:
      ret.set("output_type", "values");
      ret.set("values", m_otValues);
      break;
    case OTText:
      ret.set("output_type", "text");
      break;
    default:
      ret.set("output_type", "invalid");
  }
  ret.set("cmd", m_command);
  ret.set("text", getPrintString());
  return ret;
}

//
// Called when a breakpoint is reached, to produce the console
// spew showing the code around the breakpoint.
//
void DebuggerClient::shortCode(BreakPointInfoPtr bp) {
  if (bp && !bp->m_file.empty() && bp->m_line1) {
    Variant source = CmdList::GetSourceFile(this, bp->m_file);
    if (source.isString()) {
      // Line and column where highlight should start and end
      int beginHighlightLine = bp->m_line1;
      int beginHighlightColumn = bp->m_char1;
      int endHighlightLine = bp->m_line2;
      int endHighlightColumn = bp->m_char2;
      // Lines where source listing should start and end
      int firstLine = std::max(beginHighlightLine - 1, 1);
      int lastLine = endHighlightLine + 1;
      int maxLines = getDebuggerClientMaxCodeLines();

      // If MaxCodeLines == 0: don't spew any code after a [s]tep or [n]ext
      // command.
      if (maxLines == 0) {
        return;
      }

      // If MaxCodeLines > 0: limit spew to a maximum of # lines.
      if (maxLines > 0) {
        int numHighlightLines = endHighlightLine - beginHighlightLine + 1;
        if (numHighlightLines > maxLines) {
          // If there are too many highlight lines, truncate spew
          // by setting lastLine ...
          lastLine = beginHighlightLine + maxLines - 1;
          // ... and set endHighlightLine/Column so that it is just past the end
          // of the spew, and all code up to the truncation will be highlighted.
          endHighlightLine = lastLine + 1;
          endHighlightColumn = 1;
        }
      }

      code(source, beginHighlightLine,
           firstLine, lastLine,
           beginHighlightColumn,
           endHighlightLine,
           endHighlightColumn);
    }
  }
}

bool DebuggerClient::code(CStrRef source, int lineFocus, int line1 /* = 0 */,
                          int line2 /* = 0 */, int charFocus0 /* = 0 */,
                          int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  if (line1 == 0 && line2 == 0) {
    String highlighted;
    if (isApiMode()) {
      highlighted = source;
    } else {
      highlighted = highlight_code(source, 0, lineFocus, charFocus0,
                                   lineFocus1, charFocus1);
    }
    if (!highlighted.empty()) {
      print(highlighted);
      return true;
    }
    return false;
  }

  String highlighted;
  if (isApiMode()) {
    highlighted = source;
  } else {
    highlighted = highlight_php(source, 1, lineFocus, charFocus0,
                                lineFocus1, charFocus1);
  }
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
    print("%s%s", sb.data(), isApiMode() ? "\0" : ANSI_COLOR_END);
    return true;
  }
  return false;
}

char DebuggerClient::ask(const char *fmt, ...) {
  ASSERT(!isApiMode());
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

#define FWRITE(ptr, size, nmemb, stream)                                \
do {                                                                    \
  if (isApiMode()) {                                                    \
    m_outputBuf.append(ptr, size * nmemb);                             \
  }                                                                     \
                                                                        \
  /* LogFile debugger setting */                                        \
  FILE *f = getLogFileHandler();                                        \
  if (f != NULL) {                                                      \
    fwrite(ptr, size, nmemb, f);                                        \
  }                                                                     \
                                                                        \
  /* For debugging, still output to stdout */                           \
  fwrite(ptr, size, nmemb, stream);                                     \
} while (0)                                                             \

void DebuggerClient::print(const char *fmt, ...) {
  string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap); va_end(ap);
  print(msg);
}

void DebuggerClient::print(const std::string &s) {
  FWRITE(s.data(), 1, s.length(), stdout);
  FWRITE("\n", 1, 1, stdout);
  fflush(stdout);
}

void DebuggerClient::print(CStrRef msg) {
  FWRITE(msg.data(), 1, msg.length(), stdout);
  FWRITE("\n", 1, 1, stdout);
  fflush(stdout);
}

#define IMPLEMENT_COLOR_OUTPUT(name, where, color)                      \
  void DebuggerClient::name(CStrRef msg) {                              \
    if (UseColor && color) {                                            \
      FWRITE(color, 1, strlen(color), where);                           \
    }                                                                   \
    FWRITE(msg.data(), 1, msg.length(), where);                         \
    if (UseColor && color) {                                            \
      FWRITE(ANSI_COLOR_END, 1, strlen(ANSI_COLOR_END), where);         \
    }                                                                   \
    FWRITE("\n", 1, 1, where);                                          \
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

#undef FWRITE
#undef IMPLEMENT_COLOR_OUTPUT

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
    m_argIdx.insert(m_argIdx.begin(), 1);
    m_command = m_command.substr(0, 1);
  }
}

DebuggerCommand *DebuggerClient::createCommand() {
#define MATCH_CMD(name, cmd)                 \
do {                                         \
  if (match(name)) {                         \
    m_commandCanonical = name;               \
    return new cmd();                        \
  }                                          \
  return NULL;                               \
} while(0)                                   \

#define NEW_CMD_NAME(name, cmd)              \
do {                                         \
  m_commandCanonical = name;                 \
  return new cmd();                          \
} while(0)                                   \

  // give gdb users some love
  if (m_command == "bt") NEW_CMD_NAME("where", CmdWhere);
  if (m_command == "set") NEW_CMD_NAME("config", CmdConfig);
  if (m_command == "inst") NEW_CMD_NAME("instrument", CmdInstrument);
  if (m_command == "complete") NEW_CMD_NAME("complete", CmdComplete);

  switch (tolower(m_command[0])) {
    case 'a': MATCH_CMD("abort"    , CmdAbort    );
    case 'b': MATCH_CMD("break"    , CmdBreak    );
    case 'c': MATCH_CMD("continue" , CmdContinue );
    case 'd': MATCH_CMD("down"     , CmdDown     );
    case 'e': MATCH_CMD("exception", CmdException);
    case 'f': MATCH_CMD("frame"    , CmdFrame    );
    case 'g': MATCH_CMD("global"   , CmdGlobal   );
    case 'h': MATCH_CMD("help"     , CmdHelp     );
    case 'i': MATCH_CMD("info"     , CmdInfo     );
    case 'j': MATCH_CMD("jump"     , CmdJump     );
    case 'k': MATCH_CMD("konstant" , CmdConstant );
    case 'l': MATCH_CMD("list"     , CmdList     );
    case 'm': MATCH_CMD("machine"  , CmdMachine  );
    case 'n': MATCH_CMD("next"     , CmdNext     );
    case 'o': MATCH_CMD("out"      , CmdOut      );
    case 'p': MATCH_CMD("print"    , CmdPrint    );
    case 'q': MATCH_CMD("quit"     , CmdQuit     );
    case 'r': MATCH_CMD("run"      , CmdRun      );
    case 's': MATCH_CMD("step"     , CmdStep     );
    case 't': MATCH_CMD("thread"   , CmdThread   );
    case 'u': MATCH_CMD("up"       , CmdUp       );
    case 'v': MATCH_CMD("variable" , CmdVariable );
    case 'w': MATCH_CMD("where"    , CmdWhere    );
    case 'z': MATCH_CMD("zend"     , CmdZend     );

    // these single lettter commands allow "x{cmd}" and "x {cmd}"
    case 'x': shiftCommand(); NEW_CMD_NAME("extended", CmdExtended);
    case 'y': shiftCommand(); NEW_CMD_NAME("user", CmdUser);
    case '!': shiftCommand(); NEW_CMD_NAME("shell", CmdShell);
    case '&': shiftCommand(); NEW_CMD_NAME("macro", CmdMacro);
  }
  return NULL;
#undef MATCH_CMD
#undef NEW_CMD_NAME
}

bool DebuggerClient::process() {
  clearCachedLocal();
  if (isApiMode()) {
    // construct m_line based on m_command and m_args
    m_line = m_command;
    m_argIdx.clear();
    m_argIdx.push_back(m_line.size());
    for (unsigned int i = 0; i < m_args.size(); i++) {
      m_line += " " + m_args[i];
      m_argIdx.push_back(m_line.size());
    }
  }
  switch (tolower(m_command[0])) {
    case '@':
    case '=':
    case '$': {
      return processTakeCode();
    }
    case '<': {
      return match("<?php") && processTakeCode();
    }
    case '?': {
      if (match("?")) {
        usageLog("help", m_line);
        return CmdHelp().onClientD(this);
      }
      if (match("?>")) return processEval();
      break;
    }
    default: {
      DebuggerCommand *cmd = createCommand();
      if (cmd) {
        usageLog(m_commandCanonical, m_line);
        if (cmd->is(DebuggerCommand::KindOfRun)) playMacro("startup");
        DebuggerCommandPtr deleter(cmd);
        return cmd->onClientD(this);
      } else {
        return processTakeCode();
      }
      break;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void DebuggerClient::addToken(std::string &token, int idx) {
  m_argIdx.push_back(idx);
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
  m_argIdx.clear();
  int i = 0;
  for (i = 0; line[i]; i++) {
    char ch = line[i];
    char next = line[i+1];
    switch (ch) {
      case ' ':
        if (!quote) {
          if (!token.empty()) {
            addToken(token, i);
          }
        } else {
          token += ch;
        }
        break;
      case '"':
      case '\'':
        if (quote == 0) {
          quote = ch;
          token += ch;
          break;
        }
        if (quote == ch && (next == ' ' || next == 0)) {
          token += ch;
          addToken(token, i);
          quote = 0;
          break;
        }
        token += ch;
        break;
      case '\\':
        if ((next == ' ' || next == '"' || next == '\'' || next == '\\')) {
          if (quote == '\'') {
            token += ch;
          }
          i++;
          token += next;
          break;
        }
        // fall through
      default:
        token += ch;
        break;
    }
  }
  if (!token.empty()) {
    addToken(token, i);
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

std::string DebuggerClient::lineRest(int index) {
  ASSERT(index > 0);
  return m_line.substr(m_argIdx[index - 1] + 1);
}

///////////////////////////////////////////////////////////////////////////////
// comunication with DebuggerProxy

DebuggerCommandPtr DebuggerClient::xend(DebuggerCommand *cmd) {
  send(cmd);
  return recv(cmd->getType());
}

void DebuggerClient::send(DebuggerCommand *cmd) {
  if (!cmd->send(m_machine->m_thrift)) {
    throw DebuggerProtocolException();
  }
}

DebuggerCommandPtr DebuggerClient::recv(int expected) {
  const char *func = "DebuggerClient::recv()";

  DebuggerCommandPtr res;
  while (true) {
    while (!DebuggerCommand::Receive(m_machine->m_thrift, res, func)) {
      if (m_stopped) throw DebuggerClientExitException();
    }
    if (!res) {
      Logger::Error("Unable to communicate with server. Server's down?");
      throw DebuggerServerLostException();
    }
    if (!res->getWireError().empty()) {
      error("wire error: %s", res->getWireError().data());
    }
    if (res->is((DebuggerCommand::Type)expected)) {
      usageLogDone(boost::lexical_cast<string>(expected));
      break;
    }

    if (!res->is(DebuggerCommand::KindOfInterrupt)) {
      Logger::Error("%s: unexpected return: %d", func, res->getType());
      throw DebuggerProtocolException();
    }

    usageLogInterrupt(res);

    if (isApiMode()) {
      // Hit breakpoint during eval
      // Could also happen with CmdPrint, but since CmdPrint is also used
      // for handling watch, it could potentially lead to infinite recursion
      // Do not support KindOfPrint until we separate watch from it.
      if (expected == DebuggerCommand::KindOfEval ||
          expected == DebuggerCommand::KindOfPrint) {
        m_pendingCommands.push_back(expected);
      } else {
        continue;
      }
      break;
    }

    // eval() can cause more breakpoints
    if (!res->onClientD(this) || !console()) {
      if (m_quitting) {
        throw DebuggerClientExitException();
      } else {
        Logger::Error("%s: unable to process %d", func, res->getType());
        throw DebuggerProtocolException();
      }
    }
  }

  return res;
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
    usageLog("@", m_line);
    m_code = string("<?php ") + (m_line.c_str() + 1) + ";";
    return processEval();
  } else if (first == '=') {
    usageLog("=", m_line);
    while (m_line.at(m_line.size() - 1) == ';') {
      // strip the trailing ;
      m_line = m_line.substr(0, m_line.size() - 1);
    }
    m_code = string("<?php $_=(") + m_line.substr(1) + "); " + m_printFunction;
    return processEval();
  } else if (first != '<') {
    usageLog("eval", m_line);
    // User entered something that did not start with @, =, or <
    // and also was not a debugger command. Interpret it as PHP.
    m_code = "<?php ";
    m_code += m_line + ";";
    return processEval();
  }
  usageLog("<?php", m_line);
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
  m_inputState = TakingCommand;
  m_acLiveListsDirty = true;
  CmdEval().onClientD(this);
  return true;
}

void DebuggerClient::swapHelp() {
  ASSERT(m_args.size() > 0);
  m_command = m_args[0];
  m_args[0] = "help";
}

void DebuggerClient::quit() {
  m_quitting = true;
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
    std::map<int64, int>::const_iterator iter =
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
  if (!m_listFile.empty() && m_listFile[0] != '/' && !m_sourceRoot.empty()) {
    if (m_sourceRoot[m_sourceRoot.size() - 1] != '/') {
      m_sourceRoot += "/";
    }
    m_listFile = m_sourceRoot + m_listFile;
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
  m_config["SourceRoot"] = m_sourceRoot = sourceRoot;
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
// helpers for server API

bool DebuggerClient::apiGrab() {
  Lock l(m_inApiUseLck);
  if (m_inApiUse) {
    return false;
  }
  m_inApiUse = true;
  return true;
}

void DebuggerClient::apiFree() {
  Lock l(m_inApiUseLck);
  m_inApiUse = false;
}

void DebuggerClient::resetSmartAllocatedMembers() {
  // Essentially sets these to null: so it's safe to run their
  // destructors at sweep time or after the SmartAllocators are torn
  // down.
  new (&m_stacktrace) Array();
  new (&m_otValues) Array();
}

///////////////////////////////////////////////////////////////////////////////
// helpers for usage logging

void DebuggerClient::initUsageLogging() {
  // Usage log file is a runtime option instead of debugger config because
  // it is for internal monitoring and we don't want users to modify this.
  if (RuntimeOption::DebuggerUsageLogFile.empty()) {
    return;
  }
  mode_t old = umask(0);
  m_usageLogFP = fopen(RuntimeOption::DebuggerUsageLogFile.c_str(), "a");
  if (!m_usageLogFP) {
    Logger::Error("failed to open debugger usage log file %s",
                  RuntimeOption::DebuggerUsageLogFile.c_str());
  }
  umask(old);
}

void DebuggerClient::finiUsageLogging() {
  if (m_usageLogFP) {
    fclose(m_usageLogFP);
    m_usageLogFP = NULL;
  }
}

void DebuggerClient::usageLog(const std::string& cmd, const std::string& line) {
  if (!m_usageLogFP) {
    return;
  }

  if (m_usageLogHeader.empty()) {
    std::string login = m_options.user;
    std::string host = Process::GetHostName();
    std::string clientMode = isApiMode() ? "api" : "terminal";
    m_usageLogHeader = login + " " + host + " hhvm " + clientMode;
  }

  struct timespec tp;
  gettime(CLOCK_REALTIME, &tp);

#define MAX_LINE 1024
  char buf[MAX_LINE];
  int len = snprintf(buf, MAX_LINE, "%s %lld %lld %s #### %s",
                     m_usageLogHeader.c_str(),
                     (int64)tp.tv_sec, (int64)tp.tv_nsec,
                     cmd.c_str(), line.c_str());
  len = len >= MAX_LINE ? MAX_LINE - 1: len;
  buf[len] = '\n';
#undef MAX_LINE
  fwrite(buf, len + 1, 1, m_usageLogFP);
  fflush(m_usageLogFP);
}

void DebuggerClient::usageLogInterrupt(DebuggerCommandPtr cmd) {
  CmdInterruptPtr intr = dynamic_pointer_cast<CmdInterrupt>(cmd);
  if (!intr) {
    return;
  }
  switch (intr->getInterruptType()) {
    case SessionStarted: usageLog("interrupt", "SessionStarted"); break;
    case SessionEnded: usageLog("interrupt", "SessionEnded"); break;
    case RequestStarted: usageLog("interrupt", "RequestStarted"); break;
    case RequestEnded: usageLog("interrupt", "RequestEnded"); break;
    case PSPEnded: usageLog("interrupt", "PSPEnded"); break;
    case HardBreakPoint: usageLog("interrupt", "HardBreakPoint"); break;
    case BreakPointReached: usageLog("interrupt", "BreakPointReached"); break;
    case ExceptionThrown: usageLog("interrupt", "ExceptionThrown"); break;
    default:
      usageLog("interrupt", "unknown");
  }
}

///////////////////////////////////////////////////////////////////////////////
// configuration

void DebuggerClient::loadConfig() {
  if (m_configFileName.empty()) {
    m_configFileName = Process::GetHomeDirectory() + ConfigFileName;
  }

  // make sure file exists
  FILE *f = fopen(m_configFileName.c_str(), "a");
  if (f) {
    fclose(f);
  } else {
    m_configFileName.clear();
    return;
  }

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
  setDebuggerBypassCheck(m_config["BypassAccessCheck"].getBool());
  setDebuggerSmallStep(m_config["SmallStep"].getBool());
  int printLevel = m_config["PrintLevel"].getInt16(3);
  if (printLevel > 0 && printLevel < MinPrintLevel) {
    printLevel = MinPrintLevel;
  }
  setDebuggerPrintLevel(printLevel);

  int maxCodeLines = m_config["MaxCodeLines"].getInt16(-1);
  m_config["MaxCodeLines"] = maxCodeLines;
  setDebuggerClientMaxCodeLines(maxCodeLines);

  m_printFunction = (boost::format(
    "(function_exists(\"%s\") ? %s($_) : print_r(print_r($_, true)));")
    % pprint % pprint).str();

  m_config["Tutorial"]["Visited"].get(m_tutorialVisited);

  for (Hdf node = m_config["Macros"].firstChild(); node.exists();
       node = node.next()) {
    MacroPtr macro(new Macro());
    macro->load(node);
    m_macros.push_back(macro);
  }

  m_sourceRoot = m_config["SourceRoot"].getString();

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
  for (std::set<string>::const_iterator iter = m_tutorialVisited.begin();
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
