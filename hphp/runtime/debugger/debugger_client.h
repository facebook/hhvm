/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EVAL_DEBUGGER_CLIENT_H_
#define incl_HPHP_EVAL_DEBUGGER_CLIENT_H_

#include <boost/smart_ptr/shared_array.hpp>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_client_settings.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/util/text-color.h"
#include "hphp/util/hdf.h"
#include "hphp/util/mutex.h"

namespace HPHP {

class StringBuffer;

namespace Eval {

///////////////////////////////////////////////////////////////////////////////

struct DebuggerCommand;
struct CmdInterrupt;

using DebuggerCommandPtr = std::shared_ptr<DebuggerCommand>;

class DebuggerClient {
public:
  static int LineWidth;
  static int CodeBlockSize;
  static int ScrollBlockSize;
  static const char *LineNoFormat;
  static const char *LineNoFormatWithStar;
  static const char *LocalPrompt;
  static const char *ConfigFileName;
  static const char *LegacyConfigFileName;
  static const char *HistoryFileName;
  static std::string HomePrefix;
  static std::string SourceRoot;

  static bool UseColor;
  static bool NoPrompt;
  static const char *HelpColor;
  static const char *InfoColor;
  static const char *OutputColor;
  static const char *ErrorColor;
  static const char *ItemNameColor;
  static const char *HighlightForeColor;
  static const char *HighlightBgColor;
  static const char *DefaultCodeColors[];
  static const int MinPrintLevel = 1;

public:
  static void LoadColors(Hdf hdf);
  static const char *LoadColor(Hdf hdf, const char *defaultName);
  static const char *LoadBgColor(Hdf hdf, const char *defaultName);
  static void LoadCodeColor(CodeColor index, Hdf hdf, const char *defaultName);

  /**
   * Starts/stops a debugger client.
   */
  static SmartPtr<Socket> Start(const DebuggerClientOptions &options);
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

  typedef std::vector<std::string> LiveList;
  typedef boost::shared_array<LiveList> LiveListsPtr;
  static LiveListsPtr CreateNewLiveLists() {
    return LiveListsPtr(new LiveList[DebuggerClient::AutoCompleteCount]);
  }
  std::vector<std::string> getAllCompletions(std::string const &text);

  /**
   * Helpers
   */
  static void AdjustScreenMetrics();
  static bool Match(const char *input, const char *cmd);
  static bool IsValidNumber(const std::string &arg);
  static String FormatVariable(const Variant& v, int maxlen = 80,
                               char format = 'd');
  static String FormatInfoVec(const IDebuggable::InfoVec &info,
                              int *nameLen = nullptr);
  static String FormatTitle(const char *title);

public:
  explicit DebuggerClient();
  ~DebuggerClient();

  /**
   * Main processing functions.
   */
  void console();
  // Carries out the current command and returns true if the command completed.
  bool process();
  void quit();
  void onSignal(int sig);
  int pollSignal();

  /**
   * Output functions
   */
  void print  (const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  void help   (const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  void info   (const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  void output (const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  void error  (const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

  void print  (const std::string &s);
  void help   (const std::string &s);
  void info   (const std::string &s);
  void output (const std::string &s);
  void error  (const std::string &s);

  void print  (const String& s);
  void help   (const String& s);
  void info   (const String& s);
  void output (const String& s);
  void error  (const String& s);

  bool code(const String& source, int lineFocus = 0, int line1 = 0,
            int line2 = 0,
            int charFocus0 = 0, int lineFocus1 = 0, int charFocus1 = 0);
  void shortCode(BreakPointInfoPtr bp);
  char ask(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

  std::string wrap(const std::string &s);
  void helpTitle(const char *title);
  void helpCmds(const char *cmd, const char *desc, ...);
  void helpCmds(const std::vector<const char *> &cmds);
  void helpBody(const std::string &s);
  void helpSection(const std::string &s);

  void tutorial(const char *text);
  void setTutorial(int mode);

  // Returns the source code string that the debugger is currently
  // evaluating.
  const std::string &getCode() const { return m_code;}
  void swapHelp();

  /**
   * Test if argument matches specified. "index" is 1-based.
   */
  const std::string &getCommand() const { return m_command;}
  bool arg(int index, const char *s);
  int argCount() { return m_args.size();}
  std::string argValue(int index);
  // The entire line after that argument, un-escaped.
  std::string lineRest(int index);
  std::vector<std::string> *args() { return &m_args;}

  /**
   * Send the commmand to server's DebuggerProxy and expect same type of command
   * back. The WithNestedExecution version supports commands that cause the
   * server to run PHP on send when we want to be able to debug that PHP before
   * completing the command.
   */
  template<typename T> std::shared_ptr<T> xend(DebuggerCommand *cmd) {
    return std::static_pointer_cast<T>(xend(cmd, Nested));
  }
  template<typename T> std::shared_ptr<T>
  xendWithNestedExecution(DebuggerCommand *cmd) {
    return std::static_pointer_cast<T>(xend(cmd, NestedWithExecution));
  }

  void sendToServer(DebuggerCommand *cmd);

  /**
   * Machine functions. True if we're switching to a machine that's not
   * interrupting, therefore, we need to throw DebuggerConsoleExitException
   * to pump more interrupts. False if we're switching to a machine that
   * was already interrupting, OR, there was a failure to switch. We then
   * need to call initializeMachine() immediately without waiting.
   */
  bool connect(const std::string &host, int port);
  bool connectRPC(const std::string &host, int port);
  bool reconnect();
  bool disconnect();
  bool initializeMachine();
  bool isLocal();

  /**
   * Sandbox functions.
   */
  void updateSandboxes(std::vector<DSandboxInfoPtr> &sandboxes) {
    m_sandboxes = sandboxes;
  }
  DSandboxInfoPtr getSandbox(int index) const;
  void setSandbox(DSandboxInfoPtr sandbox);
  std::string getSandboxId();

  /**
   * Thread functions.
   */
  void updateThreads(std::vector<DThreadInfoPtr> threads);
  DThreadInfoPtr getThread(int index) const;
  int64_t getCurrentThreadId() const { return m_threadId;}

  /**
   * Current source location and breakpoints.
   */
  BreakPointInfoPtr getCurrentLocation() const { return m_breakpoint;}
  std::vector<BreakPointInfoPtr> *getBreakPoints() { return &m_breakpoints;}
  void setMatchedBreakPoints(std::vector<BreakPointInfoPtr> breakpoints);
  void setCurrentLocation(int64_t threadId, BreakPointInfoPtr breakpoint);
  std::vector<BreakPointInfoPtr> *getMatchedBreakPoints() { return &m_matched;}

  // Retrieves a source location that is the current focus of the
  // debugger. The current focus is initially determined by the
  // breakpoint where the debugger is currently stopped and can
  // thereafter be modified by list commands and by switching the
  // the stack frame.
  void getListLocation(std::string &file, int &line, int &lineFocus0,
                       int &charFocus0, int &lineFocus1, int &charFocus1);

  void setListLocation(const std::string &file, int line, bool center);
  void setSourceRoot(const std::string &sourceRoot);

  /**
   * Watch expressions.
   */
  typedef std::pair<const char *, std::string> Watch;
  typedef std::shared_ptr<Watch> WatchPtr;
  typedef std::vector<WatchPtr> WatchPtrVec;
  WatchPtrVec &getWatches() { return m_watches;}
  void addWatch(const char *fmt, const std::string &php);

  /**
   * Stacktraces.
   */
  Array getStackTrace() { return m_stacktrace; }
  void setStackTrace(const Array& stacktrace, bool isAsync);
  bool isStackTraceAsync() { return m_stacktraceAsync; }
  void moveToFrame(int index, bool display = true);
  void printFrame(int index, const Array& frame);
  void setFrame(int frame) { m_frame = frame; }
  int getFrame() const { return m_frame; }

  /**
   * Auto-completion.
   */
  bool setCompletion(const char *text, int start, int end);
  char *getCompletion(const char *text, int state);
  void addCompletion(AutoComplete type);
  void addCompletion(const char **list);
  void addCompletion(const char *name);
  void addCompletion(const std::vector<std::string> &items);
  void setLiveLists(LiveListsPtr liveLists) { m_acLiveLists = liveLists; }

  void init(const DebuggerClientOptions &options);
  void clearCachedLocal() {
    m_stacktrace = null_array;
  }

  /**
   * Macro functions
   */
  void startMacro(std::string name);
  void endMacro();
  bool playMacro(std::string name);
  const std::vector<std::shared_ptr<Macro>> &getMacros() const {
    return m_macros;
  }
  bool deleteMacro(int index);

  DECLARE_DBG_CLIENT_SETTING_ACCESSORS

  std::string getLogFile () const { return m_logFile; }
  void setLogFile (std::string inLogFile) { m_logFile = inLogFile; }
  FILE* getLogFileHandler () const { return m_logFileHandler; }
  void setLogFileHandler (FILE* inLogFileHandler) {
    m_logFileHandler = inLogFileHandler;
  }
  std::string getCurrentUser() const { return m_options.user; }

  // Usage logging
  void usageLogCommand(const std::string &cmd, const std::string &data);
  void usageLogEvent(const std::string &eventName,
                     const std::string &data = "");

  std::string getZendExecutable() const { return m_zendExe; }

  // Internal testing helpers. Only used by internal tests!!!
  bool internalTestingIsClientStopped() const { return m_stopped; }

  bool unknownCmdReceived() const { return m_unknownCmd; }
private:
  enum InputState {
    TakingCommand,
    TakingCode,
    TakingInterrupt
  };

  std::string m_configFileName;
  int m_tutorial;
  std::set<std::string> m_tutorialVisited;
  bool m_scriptMode; // Is this client being scripted by a test?
  bool m_neverSaveConfig; // So that tests can avoid clobbering the config file
  bool m_neverSaveConfigOverride;

  DECLARE_DBG_CLIENT_SETTING

  std::string m_logFile;
  FILE* m_logFileHandler;

  DebuggerClientOptions m_options;
  AsyncFunc<DebuggerClient> m_mainThread;
  bool m_stopped;

  InputState m_inputState;
  int m_sigNum; // Set when ctrl-c is pressed, used by signal polling
  int m_sigCount; // Number of times ctrl-c pressed since last interrupt

  // auto-completion states
  int m_acLen;
  int m_acIndex;
  int m_acPos;
  std::vector<const char **> m_acLists;
  std::vector<const char *> m_acStrings;
  std::vector<std::string> m_acItems;
  bool m_acLiveListsDirty;
  LiveListsPtr m_acLiveLists;
  bool m_acProtoTypePrompted;

  std::string m_line;
  // The current command to process.
  std::string m_command;
  std::string m_commandCanonical;
  std::string m_prevCmd;
  std::vector<std::string> m_args;
  // m_args[i]'s last character is m_line[m_argIdx[i]]
  std::vector<int> m_argIdx;
  std::string m_code;

  std::vector<std::shared_ptr<Macro>> m_macros;
  std::shared_ptr<Macro> m_macroRecording;
  std::shared_ptr<Macro> m_macroPlaying;

  std::vector<std::shared_ptr<DMachineInfo>>
    m_machines; // All connected machines. 0th is local.
  std::shared_ptr<DMachineInfo> m_machine; // Current machine
  std::string m_rpcHost; // Current RPC host

  std::vector<DSandboxInfoPtr> m_sandboxes;
  std::vector<DThreadInfoPtr> m_threads;
  int64_t m_threadId;
  std::map<int64_t, int> m_threadIdMap; // maps threadId to index

  std::vector<BreakPointInfoPtr> m_breakpoints;
  BreakPointInfoPtr m_breakpoint;
  std::vector<BreakPointInfoPtr> m_matched;

  // list command's current location, which may be different from m_breakpoint

  // The file currently being listed. Set implicitly by breakpoints and
  // explicitly by list commands issued to the client by a user.
  std::string m_listFile;

  // The first line to list
  int m_listLine;
  int m_listLineFocus;

  WatchPtrVec m_watches;

  Array m_stacktrace;
  bool m_stacktraceAsync;
  int m_frame;

  std::string m_sourceRoot;

  void start(const DebuggerClientOptions &options);
  void run();

  // helpers
  std::string getPrompt();
  void addToken(std::string &token, int idx);
  void parseCommand(const char *line);
  void shiftCommand();
  bool parse(const char *line);
  bool match(const char *cmd);
  int  checkEvalEnd();
  void processTakeCode();
  bool processEval();
  DebuggerCommand *createCommand();

  void updateLiveLists();
  void promptFunctionPrototype();
  char *getCompletion(const std::vector<std::string> &items,
                      const char *text);
  char *getCompletion(const std::vector<const char *> &items,
                      const char *text);

  // config and macros
  void defineColors(const Hdf &config);
  void loadConfig();
  void saveConfig();
  void record(const char *line);

  // connections
  void closeAllConnections();
  void switchMachine(std::shared_ptr<DMachineInfo> machine);
  SmartPtr<Socket> connectLocal();
  bool connectRemote(const std::string &host, int port);
  bool tryConnect(const std::string &host, int port, bool clearmachines);

  enum EventLoopKind {
    TopLevel, // The top-level event loop, called from run().
    Nested, // A nested loop where we expect a cmd back with no PHP executed.
    NestedWithExecution // A nested loop where more PHP may execute.
  };

  DebuggerCommandPtr xend(DebuggerCommand *cmd, EventLoopKind loopKind);
  DebuggerCommandPtr eventLoop(EventLoopKind loopKind, int expectedCmd,
                               const char *caller);

  // Zend executable for CmdZend, overridable via config.
  std::string m_zendExe = "php";

  bool m_unknownCmd;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CLIENT_H_
