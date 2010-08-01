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
#include <runtime/base/debuggable.h>
#include <util/text_color.h>
#include <util/hdf.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DebuggerCommand);
class DebuggerClient {
public:
  static int LineWidth;
  static int CodeBlockSize;
  static int ScrollBlockSize;
  static const char *LineNoFormat;
  static const char *LocalPrompt;
  static const char *ConfigFileName;
  static const char *HistoryFileName;

  static bool UseColor;
  static const char *HelpColor;
  static const char *InfoColor;
  static const char *OutputColor;
  static const char *ErrorColor;
  static const char *ItemNameColor;
  static const char *DefaultCodeColors[];

public:
  /**
   * Starts/stops a debugger client.
   */
  static SmartPtr<Socket> Start(const std::string &host, int port,
                                const std::string &extension);
  static void Stop();

  /**
   * Pre-defined auto-complete lists. Append-only, as they will be used in
   * binary communication protocol.
   */
  enum AutoComplete {
    AutoCompleteFileNames,
    AutoCompleteVariables,
    AutoCompleteConstants,
    AutoCompleteClasses,
    AutoCompleteFunctions,
    AutoCompleteClassMethods,
    AutoCompleteClassProperties,
    AutoCompleteClassConstants,
    AutoCompleteKeyword,
    AutoCompleteCode,

    AutoCompleteCount
  };
  static const char **GetCommands();

  typedef std::vector<String> LiveLists[DebuggerClient::AutoCompleteCount];
  typedef boost::shared_ptr<LiveLists> LiveListsPtr;
  static LiveListsPtr CreateNewLiveLists() {
    return LiveListsPtr(new LiveLists[DebuggerClient::AutoCompleteCount]());
  }

  /**
   * Helpers
   */
  static void AdjustScreenMetrics();
  static bool Match(const char *input, const char *cmd);
  static bool IsValidNumber(const std::string &arg);
  static String FormatVariable(CVarRef v, int maxlen = 80);
  static String FormatInfoVec(const IDebuggable::InfoVec &info,
                              int *nameLen = NULL);
  static String FormatTitle(const char *title);

public:
  DebuggerClient();
  ~DebuggerClient();
  void reset();

  /**
   * Thread functions.
   */
  void start(const std::string &extension);
  void stop();
  void run();

  /**
   * Main processing functions.
   */
  bool console();
  bool process();
  void quit();
  void onSignal(int sig);
  int pollSignal();

  /**
   * Output functions
   */
  void print  (const char *fmt, ...);
  void help   (const char *fmt, ...);
  void info   (const char *fmt, ...);
  void output (const char *fmt, ...);
  void error  (const char *fmt, ...);

  void print  (const std::string &s);
  void help   (const std::string &s);
  void info   (const std::string &s);
  void output (const std::string &s);
  void error  (const std::string &s);

  void print  (CStrRef s);
  void help   (CStrRef s);
  void info   (CStrRef s);
  void output (CStrRef s);
  void error  (CStrRef s);

  void code(CStrRef source, int line1 = 0, int line2 = 0);
  char ask(const char *fmt, ...);

  std::string wrap(const std::string &s);
  void helpTitle(const char *title);
  void helpCmds(const char *cmd, const char *desc, ...);
  void helpCmds(const std::vector<const char *> &cmds);
  void helpBody(const std::string &s);
  void helpSection(const std::string &s);

  void tutorial(const char *text);
  void setTutorial(int mode);

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
   * Machine functions.
   */
  void connect(const std::string &host, int port);
  void disconnect();

  /**
   * Sandbox functions.
   */
  void updateSandboxes(const StringVec &sandboxes) {
    m_sandboxes = sandboxes;
  }
  std::string getSandbox(int index) const;

  /**
   * Thread functions.
   */
  void updateThreads(DThreadInfoPtrVec threads);
  DThreadInfoPtr getThread(int index) const;
  int64 getCurrentThreadId() const { return m_threadId;}

  /**
   * Current source location and breakpoints.
   */
  BreakPointInfoPtr getCurrentLocation() const { return m_breakpoint;}
  BreakPointInfoPtrVec *getBreakPoints() { return &m_breakpoints;}
  void setMatchedBreakPoints(BreakPointInfoPtrVec breakpoints);
  void setCurrentLocation(int64 threadId, BreakPointInfoPtr breakpoint);
  BreakPointInfoPtrVec *getMatchedBreakPoints() { return &m_matched;}
  void getListLocation(std::string &file, int &line);
  void setListLocation(const std::string &file, int line);

  /**
   * Watch expressions.
   */
  typedef std::pair<const char *, std::string> Watch;
  typedef boost::shared_ptr<Watch> WatchPtr;
  typedef std::vector<WatchPtr> WatchPtrVec;
  WatchPtrVec &getWatches() { return m_watches;}
  void addWatch(const char *fmt, const std::string &php);

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
  void addCompletion(AutoComplete type);
  void addCompletion(const char **list);
  void addCompletion(const char *name);
  void addCompletion(const std::vector<String> &items);
  void phpCompletion(const char *text);
  void setLiveLists(LiveListsPtr liveLists) { m_acLiveLists = liveLists;}

  /**
   * Macro functions
   */
  void startMacro(std::string name);
  void endMacro();
  bool playMacro(std::string name);
  const MacroPtrVec &getMacros() const { return m_macros;}
  bool deleteMacro(int index);

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

  std::string m_configFileName;
  Hdf m_config;
  int m_tutorial;
  std::set<std::string> m_tutorialVisited;

  std::string m_extension;
  AsyncFunc<DebuggerClient> m_mainThread;
  bool m_stopped;

  InputState m_inputState;
  RunState m_runState;
  int m_signum;

  // auto-completion states
  int m_acLen;
  int m_acIndex;
  int m_acPos;
  std::vector<const char **> m_acLists;
  std::vector<const char *> m_acStrings;
  std::vector<String> m_acItems;
  bool m_acLiveListsDirty;
  LiveListsPtr m_acLiveLists;

  std::string m_line;
  std::string m_command;
  std::string m_prevCmd;
  StringVec m_args;
  std::string m_code;

  MacroPtrVec m_macros;
  MacroPtr m_macroRecording;
  MacroPtr m_macroPlaying;

  DMachineInfoPtrVec m_machines; // all connected ones
  DMachineInfoPtr m_machine;     // current

  StringVec m_sandboxes;
  DThreadInfoPtrVec m_threads;
  int64 m_threadId;
  std::map<int64, int> m_threadIdMap; // maps threadId to index

  BreakPointInfoPtrVec m_breakpoints;
  BreakPointInfoPtr m_breakpoint;
  BreakPointInfoPtrVec m_matched;

  // list command's current location, which may be different from m_breakpoint
  std::string m_listFile;
  int m_listLine;

  WatchPtrVec m_watches;

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

  void updateLiveLists();
  char *getCompletion(const std::vector<String> &items,
                      const char *text);
  char *getCompletion(const std::vector<const char *> &items,
                      const char *text);

  // config and macros
  void defineColors();
  const char *loadColor(Hdf hdf, const char *defaultName);
  void loadCodeColor(CodeColor index, Hdf hdf, const char *defaultName);
  void loadConfig();
  void saveConfig();
  void record(const char *line);

  // communications
  void switchMachine(DMachineInfoPtr machine);
  SmartPtr<Socket> connectLocal();
  void connectRemote(const std::string &host, int port);

  DebuggerCommandPtr send(DebuggerCommand *cmd, int expected);
  DebuggerCommandPtr xend(DebuggerCommand *cmd);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CLIENT_H__
