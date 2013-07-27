/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_client_settings.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/util/text_color.h"
#include "hphp/util/hdf.h"
#include "hphp/util/mutex.h"

namespace HPHP {

class StringBuffer;

namespace Eval {

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DebuggerCommand);
DECLARE_BOOST_TYPES(CmdInterrupt);
class DebuggerClient {
public:
  static int LineWidth;
  static int CodeBlockSize;
  static int ScrollBlockSize;
  static const char *LineNoFormat;
  static const char *LocalPrompt;
  static const char *ConfigFileName;
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
  static String FormatVariable(CVarRef v, int maxlen = 80,
                               bool vardump = false);
  static String FormatInfoVec(const IDebuggable::InfoVec &info,
                              int *nameLen = nullptr);
  static String FormatTitle(const char *title);

public:
  explicit DebuggerClient(std::string name = ""); // name only for api usage
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

  void print  (CStrRef s);
  void help   (CStrRef s);
  void info   (CStrRef s);
  void output (CStrRef s);
  void error  (CStrRef s);

  bool code(CStrRef source, int lineFocus = 0, int line1 = 0, int line2 = 0,
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
  void setCommand(const std::string &cmd) { m_command = cmd;}
  bool arg(int index, const char *s);
  int argCount() { return m_args.size();}
  std::string argValue(int index);
  // The entire line after that argument, un-escaped.
  std::string lineRest(int index);
  StringVec *args() { return &m_args;}

  /**
   * Send the commmand to server's DebuggerProxy and expect same type of command
   * back. The WithNestedExecution version supports commands that cause the
   * server to run PHP on send when we want to be able to debug that PHP before
   * completing the command.
   */
  template<typename T> boost::shared_ptr<T> xend(DebuggerCommand *cmd) {
    return boost::static_pointer_cast<T>(xend(cmd, Nested));
  }
  template<typename T> boost::shared_ptr<T>
  xendWithNestedExecution(DebuggerCommand *cmd) {
    return boost::static_pointer_cast<T>(xend(cmd, NestedWithExecution));
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
  void updateSandboxes(DSandboxInfoPtrVec &sandboxes) {
    m_sandboxes = sandboxes;
  }
  DSandboxInfoPtr getSandbox(int index) const;
  void setSandbox(DSandboxInfoPtr sandbox);
  std::string getSandboxId();

  /**
   * Thread functions.
   */
  void updateThreads(DThreadInfoPtrVec threads);
  DThreadInfoPtr getThread(int index) const;
  int64_t getCurrentThreadId() const { return m_threadId;}

  /**
   * Current source location and breakpoints.
   */
  BreakPointInfoPtr getCurrentLocation() const { return m_breakpoint;}
  BreakPointInfoPtrVec *getBreakPoints() { return &m_breakpoints;}
  void setMatchedBreakPoints(BreakPointInfoPtrVec breakpoints);
  void setCurrentLocation(int64_t threadId, BreakPointInfoPtr breakpoint);
  BreakPointInfoPtrVec *getMatchedBreakPoints() { return &m_matched;}

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
  typedef boost::shared_ptr<Watch> WatchPtr;
  typedef std::vector<WatchPtr> WatchPtrVec;
  WatchPtrVec &getWatches() { return m_watches;}
  void addWatch(const char *fmt, const std::string &php);

  /**
   * Stacktraces.
   */
  Array getStackTrace() { return m_stacktrace;}
  void setStackTrace(CArrRef stacktrace);
  void moveToFrame(int index, bool display = true);
  void printFrame(int index, CArrRef frame);
  void setFrame(int frame) { m_frame = frame; }
  int getFrame() const { return m_frame;}

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

  /**
   * For DebuggerClient API
   */
  enum ClientState {
    StateUninit,
    StateInitializing,
    StateReadyForCommand,
    StateBusy
  };
  enum OutputType {
    OTInvalid,
    OTCodeLoc,
    OTStacktrace,
    OTValues,
    OTText
  };
  bool isApiMode() const { return m_options.apiMode; }
  void setConfigFileName(const std::string& fn) { m_configFileName = fn;}
  ClientState getClientState() const { return m_clientState; }
  void setClientState(ClientState state) { m_clientState = state; }
  void init(const DebuggerClientOptions &options);
  DebuggerCommandPtr waitForNextInterrupt();
  String getPrintString();
  Array getOutputArray();
  void setOutputType(OutputType type) { m_outputType = type; }
  void setOTFileLine(const std::string& file, int line) {
    m_otFile = file;
    m_otLineNo = line;
  }
  void setOTValues(CArrRef values) { m_otValues = values; }
  void clearCachedLocal() {
    m_otFile = "";
    m_otLineNo = 0;
    m_stacktrace = null_array;
    m_otValues = null_array;
  }
  bool apiGrab();
  void apiFree();
  void resetSmartAllocatedMembers();
  const std::string& getNameApi() const { return m_nameForApi; }

  /**
   * Macro functions
   */
  void startMacro(std::string name);
  void endMacro();
  bool playMacro(std::string name);
  const MacroPtrVec &getMacros() const { return m_macros;}
  bool deleteMacro(int index);

  DECLARE_DBG_SETTING_ACCESSORS
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

private:
  enum InputState {
    TakingCommand,
    TakingCode,
    TakingInterrupt
  };

  /*
   * NOTE: be careful about the use of smart-allocated data members
   * here.  They need to be kept in sync with
   * resetSmartAllocatedMembers() or you'll break the php-api to the
   * debugger and shutdown in the CLI client.
   */

  std::string m_configFileName;
  Hdf m_config;
  int m_tutorial;
  std::set<std::string> m_tutorialVisited;
  bool m_scriptMode; // Is this client being scripted by a test?

  DECLARE_DBG_SETTING
  DECLARE_DBG_CLIENT_SETTING

  std::string m_logFile;
  FILE* m_logFileHandler;

  DebuggerClientOptions m_options;
  AsyncFunc<DebuggerClient> m_mainThread;
  bool m_stopped;

  InputState m_inputState;
  int m_signum; // Set when ctrl-c is pressed, used by signal polling
  int m_sigTime; // The last time ctrl-c was recognized

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
  StringVec m_args;
  // m_args[i]'s last character is m_line[m_argIdx[i]]
  std::vector<int> m_argIdx;
  std::string m_code;

  MacroPtrVec m_macros;
  MacroPtr m_macroRecording;
  MacroPtr m_macroPlaying;

  DMachineInfoPtrVec m_machines; // All connected machines. 0th is local.
  DMachineInfoPtr m_machine;     // Current machine
  std::string m_rpcHost;         // Current RPC host

  DSandboxInfoPtrVec m_sandboxes;
  DThreadInfoPtrVec m_threads;
  int64_t m_threadId;
  std::map<int64_t, int> m_threadIdMap; // maps threadId to index

  BreakPointInfoPtrVec m_breakpoints;
  BreakPointInfoPtr m_breakpoint;
  BreakPointInfoPtrVec m_matched;

  // list command's current location, which may be different from m_breakpoint

  // The file currently being listed. Set implicitly by breakpoints and
  // explicitly by list commands issued to the client by a user or via the API.
  std::string m_listFile;

  // The first line to list
  int m_listLine;
  int m_listLineFocus;

  WatchPtrVec m_watches;

  Array m_stacktrace;
  int m_frame;

  ClientState m_clientState;
  std::string m_sourceRoot;
  std::string m_outputBuf;

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
  void processEval();
  DebuggerCommand *createCommand();

  void updateLiveLists();
  void promptFunctionPrototype();
  char *getCompletion(const std::vector<std::string> &items,
                      const char *text);
  char *getCompletion(const std::vector<const char *> &items,
                      const char *text);

  // config and macros
  void defineColors();
  void loadConfig();
  void saveConfig();
  void record(const char *line);

  // connections
  void closeAllConnections();
  void switchMachine(DMachineInfoPtr machine);
  SmartPtr<Socket> connectLocal();
  bool connectRemote(const std::string &host, int port);

  enum EventLoopKind {
    TopLevel, // The top-level event loop, called from run().
    Nested, // A nested loop where we expect a cmd back with no PHP executed.
    NestedWithExecution // A nested loop where more PHP may execute.
  };

  DebuggerCommandPtr xend(DebuggerCommand *cmd, EventLoopKind loopKind);
  DebuggerCommandPtr eventLoop(EventLoopKind loopKind, int expectedCmd,
                               const char *caller);

  // output
  OutputType m_outputType;
  std::string m_otFile;
  int m_otLineNo;
  Array m_otValues;
  std::vector<int> m_pendingCommands;

  Mutex m_inApiUseLck;
  bool m_inApiUse;
  std::string m_nameForApi;

  // usage logging
  const char *getUsageMode();

  // Zend executable for CmdZend, overridable via config.
  std::string m_zendExe;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CLIENT_H_
