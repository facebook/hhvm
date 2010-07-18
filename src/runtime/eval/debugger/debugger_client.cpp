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

#include <runtime/eval/debugger/debugger_client.h>
#include <runtime/eval/debugger/debugger_command.h>
#include <runtime/eval/debugger/cmd/all.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/string_util.h>
#include <runtime/ext/ext_socket.h>
#include <util/logger.h>
#include <util/text_color.h>
#include <util/text_art.h>

#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
using namespace HPHP::Util::TextArt;

#define LINE_WIDTH 76

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class DebuggerProtocolException : public Exception {};

///////////////////////////////////////////////////////////////////////////////

DebuggerClient s_debugger_client;

const char **DebuggerClient::AUTO_COMPLETE_FILENAMES = (const char **)1;
const char **DebuggerClient::AUTO_COMPLETE_VARIABLES = (const char **)2;
const char **DebuggerClient::AUTO_COMPLETE_CONSTANTS = (const char **)3;
const char **DebuggerClient::AUTO_COMPLETE_CLASSES   = (const char **)4;
const char **DebuggerClient::AUTO_COMPLETE_FUNCTIONS = (const char **)5;

SmartPtr<Socket> DebuggerClient::Start(const std::string &host, int port) {
  SmartPtr<Socket> ret = s_debugger_client.connectLocal();
  if (!host.empty()) {
    s_debugger_client.connectRemote(host, port);
  }
  Debugger::SetTextColors();
  s_debugger_client.start();
  return ret;
}

void DebuggerClient::Stop() {
  s_debugger_client.stop();
}

///////////////////////////////////////////////////////////////////////////////

DebuggerClient::DebuggerClient()
    : m_thread(this, &DebuggerClient::run), m_stopped(false),
      m_color(true), m_inputState(TakingCommand), m_runState(NotYet) {
}

SmartPtr<Socket> DebuggerClient::connectLocal() {
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    throw Exception("unable to create socket pair for local debugging");
  }
  SmartPtr<Socket> socket1(new Socket(fds[0], AF_UNIX));
  SmartPtr<Socket> socket2(new Socket(fds[1], AF_UNIX));

  MachineInfoPtr machine(new MachineInfo());
  machine->name = "local";
  machine->thrift.create(socket1);
  m_machines.push_back(machine);
  m_machine = machine;
  return socket2;
}

void DebuggerClient::connectRemote(const std::string &host, int port) {
  if (port <= 0) {
    port = RuntimeOption::DebuggerServerPort;
  }
  Socket *sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET,
                            String(host), port);
  Object obj(sock);
  if (!f_socket_connect(sock, String(host), port)) {
    throw Exception("unable to connect to %s:%d", host.c_str(), port);
  }
  MachineInfoPtr machine(new MachineInfo());
  machine->name = host;
  machine->thrift.create(SmartPtr<Socket>(sock));
  m_machines.push_back(machine);
  m_machine = machine;
}

std::string DebuggerClient::getPrompt() {
  if (m_inputState == TakingCode) {
    string prompt = " ";
    for (unsigned int i = 2; i < m_machine->name.size() + 2; i++) {
      prompt += '.';
    }
    prompt += ' ';
    return prompt;
  }
  return m_machine->name + "> ";
}

void DebuggerClient::start() {
  m_thread.start();
}

void DebuggerClient::stop() {
  m_stopped = true;
  m_thread.waitForEnd();
}

void DebuggerClient::run() {
  hphp_session_init();
  ExecutionContext *context = hphp_context_init();
  runImpl();
  hphp_context_exit(context, false);
  hphp_session_exit();
}

///////////////////////////////////////////////////////////////////////////////

void DebuggerClient::phpCompletion(const char *text) {
  if (!*text || strchr(text, '/')) {
    addCompletion(AUTO_COMPLETE_FILENAMES);
    return;
  }

  if (*text == '$') {
    if (strcmp(text, "$this")) {
      addCompletion("->");
    } else {
      addCompletion("this");
      addCompletion(AUTO_COMPLETE_VARIABLES);
    }
    return;
  }

  if (text[strlen(text) - 1] == '(') {
    string token = text;
    token.resize(token.size() - 1);
    rl_save_prompt();
    rl_message("\n%s%s\n", "prototype of ", token.c_str());
    rl_restore_prompt();
    return;
  }

  // generic bare word that can be any of these
  addCompletion(AUTO_COMPLETE_CONSTANTS);
  addCompletion(AUTO_COMPLETE_CLASSES);
  addCompletion(AUTO_COMPLETE_FUNCTIONS);
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

void DebuggerClient::addCompletion(const char **list) {
  m_acLists.push_back(list);
}

void DebuggerClient::addCompletion(const char *name) {
  m_acStrings.push_back(name);
}

char *DebuggerClient::getCompletion(const char *text, int state) {
  if (state == 0) {
    m_acLen = strlen(text);
    m_acIndex = 0;
    m_acPos = -1;
    m_acStrings.clear();
    m_acLists.clear();
    if (m_inputState == TakingCommand) {
      if (m_command.empty()) {
        addCompletion(GetCommands());
      } else {
        DebuggerCommand *cmd = createCommand();
        if (cmd) {
          DebuggerCommandPtr deleter(cmd);
          cmd->list(this);
        }
      }
    } else {
      ASSERT(m_inputState == TakingCode);
      if (!*rl_line_buffer) {
        addCompletion("?>"); // so we tab, we're done
      } else {
        phpCompletion(text); // context-sensitive help with PHP
      }
    }
  }

  for (; m_acIndex < (int)m_acLists.size(); m_acIndex++) {
    const char **list = m_acLists[m_acIndex];
    if (list == AUTO_COMPLETE_FILENAMES) {
      char *p = rl_filename_completion_function(text, ++m_acPos);
      if (p) return p;

    } else if (list == AUTO_COMPLETE_VARIABLES) {
    } else if (list == AUTO_COMPLETE_CONSTANTS) {
    } else if (list == AUTO_COMPLETE_CLASSES) {
    } else if (list == AUTO_COMPLETE_FUNCTIONS) {

    } else {
      for (const char *p = list[++m_acPos]; p; p = list[++m_acPos]) {
        if (m_acLen == 0 || strncasecmp(p, text, m_acLen) == 0) {
          return strdup(p);
        }
      }
    }
    m_acPos = -1;
  }

  while (++m_acPos < (int)m_acStrings.size()) {
    const char *name = m_acStrings[m_acPos].c_str();
    if (strncasecmp(name, text, m_acLen) == 0) {
      return strdup(name);
    }
  }

  return NULL;
}

static char* debugger_generator(const char* text, int state) {
  return s_debugger_client.getCompletion(text, state);
}

static char **debugger_completion(const char *text, int start, int end) {
  if (s_debugger_client.setCompletion(text, start, end)) {
    return rl_completion_matches((char*)text, &debugger_generator);
  }
  return NULL;
}

void DebuggerClient::runImpl() {
  const char *func = "DebuggerClient::runImpl()";

  info("Welcome to HipHop Debugger!");
  info("Type \"help\" or \"?\" for a complete list of commands.");

  rl_attempted_completion_function = debugger_completion;
  rl_basic_word_break_characters = " ";

  try {
    while (!m_stopped) {
      DebuggerCommandPtr cmd;
      if (DebuggerCommand::Receive(m_machine->thrift, cmd, func)) {
        if (!cmd) {
          Logger::Error("%s: debugger error", func);
          return;
        }
        if (!cmd->is(DebuggerCommand::KindOfInterrupt)) {
          Logger::Error("%s: bad cmd type: %d", func, cmd->getType());
          return;
        }
        if (!console()) {
          return;
        }
      }
    }
  } catch (DebuggerExitException &e) { /* normal exit */ }
}

bool DebuggerClient::console() {
  while (true) {
    char *line = readline(getPrompt().c_str());
    if (line && *line) {
      add_history(line);
    }
    if (line) {
      if (*line) {
        bool cmd = parse(line);
        free(line);
        if (cmd) {
          try {
            if (!process()) {
              error("command not found");
              m_command.clear();
            } else if (m_inputState == ShouldQuit) {
              return false;
            }
          } catch (DebuggerProtocolException &e) {
            return false;
          }
        }
      } else if (m_inputState == TakingCommand) {
        switch (m_command[0]) {
          case 'c': // continue
          case 's': // step
          case 'n': // next
            try {
              process(); // replay the same command
            } catch (DebuggerProtocolException &e) {
              return false;
            }
            break;
        }
      }
    } else {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void DebuggerClient::print(const char *fmt, ...) {
  string msg;
  va_list ap;
  va_start(ap, fmt);
  Logger::VSNPrintf(msg, fmt, ap); va_end(ap);
  msg += "\n";
  fwrite(msg.data(), 1, msg.length(), stdout);
  fflush(stdout);
}

void DebuggerClient::print(const std::string &s) {
  print("%s", s.c_str());
}

#define IMPLEMENT_COLOR_OUTPUT(name, where, color)                      \
  void DebuggerClient::name(const char *fmt, ...) {                     \
    string msg;                                                         \
    va_list ap;                                                         \
    va_start(ap, fmt);                                                  \
    Logger::VSNPrintf(msg, fmt, ap); va_end(ap);                        \
    if (m_color) {                                                      \
      msg = color + msg + ANSI_COLOR_END;                               \
    }                                                                   \
    msg += "\n";                                                        \
                                                                        \
    fwrite(msg.data(), 1, msg.length(), where);                         \
    fflush(where);                                                      \
  }                                                                     \
  void DebuggerClient::name(const std::string &s) {                     \
    name("%s", s.c_str());                                              \
  }                                                                     \

IMPLEMENT_COLOR_OUTPUT(help,   stdout, ANSI_COLOR_BROWN);
IMPLEMENT_COLOR_OUTPUT(info,   stdout, ANSI_COLOR_GREEN);
IMPLEMENT_COLOR_OUTPUT(output, stdout, Util::s_stdout_color);
IMPLEMENT_COLOR_OUTPUT(error,  stderr, Util::s_stderr_color);

string DebuggerClient::wrap(const std::string &s) {
  String ret = StringUtil::WordWrap(String(s.c_str(), s.size(), AttachLiteral),
                                    LINE_WIDTH - 4, "\n", true);
  return string(ret.data(), ret.size());
}

void DebuggerClient::helpTitle(const char *title) {
  help(string("\n===== ") + title + " =====\n");
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
  String ret = StringUtil::WordWrap(String(text), LINE_WIDTH - 4, "\n", true);
  Array lines = StringUtil::Explode(ret, "\n").toArray();

  StringBuffer sb;
  String header = "  Tutorial - '[h]elp [t]utorial [off]' to turn off  ";
  String hr = StringUtil::Repeat(BOX_H, LINE_WIDTH - 2);

  sb.append(BOX_UL); sb.append(hr); sb.append(BOX_UR); sb.append("\n");

  int wh = (LINE_WIDTH - 2 - header.size()) / 2;
  sb.append(BOX_V);
  sb.append(StringUtil::Repeat(" ", wh));
  sb.append(header);
  sb.append(StringUtil::Repeat(" ", wh));
  sb.append(BOX_V);
  sb.append("\n");

  sb.append(BOX_VL); sb.append(hr); sb.append(BOX_VR); sb.append("\n");
  for (ArrayIter iter(lines); iter; ++iter) {
    sb.append(BOX_V); sb.append(' ');
    sb.append(StringUtil::Pad(iter.second(), LINE_WIDTH - 4));
    sb.append(' '); sb.append(BOX_V); sb.append("\n");
  }
  sb.append(BOX_LL); sb.append(hr); sb.append(BOX_LR); sb.append("\n");

  help("%s", sb.data());
}

///////////////////////////////////////////////////////////////////////////////
// command processing

const char **DebuggerClient::GetCommands() {
  static const char *cmds[] = {
    "abort",
    "break",
    "continue",
    "down",
    "exception",
    "frame",
    "global",
    "help",
    "info",
    "jump",
    "konstant",
    "list",
    "machine",
    "next",
    "out",
    "print",
    "quit",
    "run",
    "step",
    "thread",
    "up",
    "variable",
    "where",
    "x",
    "y",
    "zend",
    "!",
    "=",
    "$",
    "<?php",
    "?>",
    NULL
  };
  return cmds;
}

DebuggerCommand *DebuggerClient::createCommand() {
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
    case 'x': return (match("x")         ) ? new CmdExtended () : NULL;
    case 'y': return (match("y")         ) ? new CmdUser     () : NULL;
    case 'z': return (match("zend")      ) ? new CmdZend     () : NULL;
    case '!': return (match("!")         ) ? new CmdShell    () : NULL;
  }
  return NULL;
}

bool DebuggerClient::process() {
  switch (tolower(m_command[0])) {
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
        if (!quote && !token.empty()) {
          addToken(token);
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

///////////////////////////////////////////////////////////////////////////////
// comunication with DebuggerProxy

DebuggerCommandPtr DebuggerClient::xend(DebuggerCommand *cmd) {
  return send(cmd, cmd->getType());
}

DebuggerCommandPtr DebuggerClient::send(DebuggerCommand *cmd, int expected) {
  if (cmd->send(m_machine->thrift)) {
    DebuggerCommandPtr res;
    if (expected) {
      while (!DebuggerCommand::Receive(m_machine->thrift, res,
                                       "DebuggerClient::send()")) {
        if (m_stopped) throw DebuggerExitException();
      }
      if (res) {
        if (res->is((DebuggerCommand::Type)expected)) {
          return res;
        }
        Logger::Error("DebuggerClient::send(): unexpected return: %d",
                      res->getType());
      } else {
        Logger::Error("DebuggerClient::send(): unexpected return: null");
      }
    } else {
      return res;
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
  if (first == '=') {
    m_code = string("<?php $_") + m_line + "; var_export($_);";
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
  return true;
}

void DebuggerClient::swapHelp() {
  ASSERT(m_args.size() > 0);
  m_command = m_args[0];
  m_args[0] = "help";
}

void DebuggerClient::quit() {
  m_inputState = ShouldQuit;
}

std::string DebuggerClient::getSandbox(int index) const {
  if (index > 0) {
    --index;
    if (index >= 0 && index < (int)m_sandboxes.size()) {
      return m_sandboxes[index];
    }
  }
  return "";
}

///////////////////////////////////////////////////////////////////////////////
}}
