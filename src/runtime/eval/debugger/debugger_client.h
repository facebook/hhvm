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

#ifndef __HPHP_EVAL_DEBUGGER_CLIENT_H__
#define __HPHP_EVAL_DEBUGGER_CLIENT_H__

#include <runtime/eval/debugger/debugger.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DebuggerCommand);
class DebuggerClient {
public:
  /**
   * Starts/stops a debugger client.
   */
  static SmartPtr<Socket> Start(const std::string &host, int port);
  static void Stop();

  /**
   * Pre-defined auto-complete lists.
   */
  static const char **AUTO_COMPLETE_FILENAMES;
  static const char **AUTO_COMPLETE_VARIABLES;
  static const char **AUTO_COMPLETE_CONSTANTS;
  static const char **AUTO_COMPLETE_CLASSES;
  static const char **AUTO_COMPLETE_FUNCTIONS;
  static const char **GetCommands();

  /**
   * Helpers
   */
  static bool Match(const char *input, const char *cmd);
  static bool IsValidNumber(const std::string &arg);
  static String PrintVariable(CVarRef v, int maxlen = 80);

public:
  DebuggerClient();
  void reset();

  /**
   * Thread functions.
   */
  void start();
  void stop();
  void run();

  /**
   * Main processing functions.
   */
  bool console();
  bool process();
  void quit();

  /**
   * Output functions
   */
  void print  (const char *fmt, ...);
  void help   (const char *fmt, ...);
  void info   (const char *fmt, ...);
  void output (const char *fmt, ...);
  void error  (const char *fmt, ...);
  void comment(const char *fmt, ...);

  void print  (const std::string &s);
  void help   (const std::string &s);
  void info   (const std::string &s);
  void output (const std::string &s);
  void error  (const std::string &s);
  void comment(const std::string &s);

  void code(const char *str);
  char ask(const char *fmt, ...);

  std::string wrap(const std::string &s);
  void helpTitle(const char *title);
  void helpBody(const std::string &s);
  void helpSection(const std::string &s);
  void tutorial(const char *text);

  /**
   * Input functions.
   */
  const std::string &getCode() const { return m_code;}
  void swapHelp();

  /**
   * Test if argument matches specified. "index" is 1-based.
   */
  const std::string &getCommand() const { return m_command;}
  bool arg(int index, const char *s);
  int argCount() { return m_args.size();}
  std::string argValue(int index);
  std::string argRest(int index);
  StringVec *args() { return &m_args;}

  /**
   * Send the commmand to DebuggerProxy and expect same type of command back.
   */
  template<typename T> boost::shared_ptr<T> xend(DebuggerCommand *cmd) {
    return boost::static_pointer_cast<T>(xend(cmd));
  }
  void send(DebuggerCommand *cmd) { send(cmd, 0);}

  /**
   * Sandbox functions.
   */
  void updateSandboxes(const StringVec &sandboxes) {
    m_sandboxes = sandboxes;
  }
  std::string getSandbox(int index) const;

  /**
   * Current source location and breakpoints.
   */
  BreakPointInfoPtr getCurrentLocation() const { return m_breakpoint;}
  BreakPointInfoPtrVec *getBreakPoints() { return &m_breakpoints;}
  void setMatchedBreakPoints(BreakPointInfoPtrVec breakpoints);
  void setCurrentLocation(BreakPointInfoPtr breakpoint);
  BreakPointInfoPtrVec *getMatchedBreakPoints() { return &m_matched;}

  /**
   * Stacktraces.
   */
  Array getStackTrace() { return m_stacktrace;}
  void setStackTrace(CArrRef stacktrace);
  void moveToFrame(int index);
  void printFrame(int index, CArrRef frame);
  int getFrame() const { return m_frame;}

  /**
   * Auto-completion.
   */
  bool setCompletion(const char *text, int start, int end);
  char *getCompletion(const char *text, int state);
  void addCompletion(const char **list);
  void addCompletion(const char *name);
  void phpCompletion(const char *text);

private:
  enum InputState {
    TakingCommand,
    TakingCode,
  };
  enum RunState {
    NotYet,
    Running,
    Stopped,
  };

  AsyncFunc<DebuggerClient> m_thread;
  bool m_stopped;

  bool m_color;
  InputState m_inputState;
  RunState m_runState;

  // auto-completion states
  int m_acLen;
  int m_acIndex;
  int m_acPos;
  std::vector<const char **> m_acLists;
  StringVec m_acStrings;

  std::string m_line;
  std::string m_command;
  StringVec m_args;
  std::string m_code;

  MachineInfoPtrVec m_machines; // all connected ones
  MachineInfoPtr m_machine;     // current

  StringVec m_sandboxes;

  BreakPointInfoPtrVec m_breakpoints;
  BreakPointInfoPtr m_breakpoint;
  BreakPointInfoPtrVec m_matched;

  Array m_stacktrace;
  int m_frame;

  // helpers
  void runImpl();
  std::string getPrompt();
  void addToken(std::string &token);
  void parseCommand(const char *line);
  void shiftCommand();
  bool parse(const char *line);
  bool match(const char *cmd);
  int  checkEvalEnd();
  bool processTakeCode();
  bool processEval();
  DebuggerCommand *createCommand();

  // communications
  SmartPtr<Socket> connectLocal();
  void connectRemote(const std::string &host, int port);

  DebuggerCommandPtr send(DebuggerCommand *cmd, int expected);
  DebuggerCommandPtr xend(DebuggerCommand *cmd);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CLIENT_H__
