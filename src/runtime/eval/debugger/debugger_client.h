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

#ifndef __HPHP_EVAL_DEBUGGER_CLIENT_H__
#define __HPHP_EVAL_DEBUGGER_CLIENT_H__

#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/debugger_client_settings.h>
#include <runtime/eval/debugger/inst_point.h>
#include <runtime/base/debuggable.h>
#include <util/text_color.h>
#include <util/hdf.h>
#include <util/mutex.h>

namespace HPHP {

class StringBuffer;

namespace Eval {
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

  typedef std::vector<std::string> LiveLists[DebuggerClient::AutoCompleteCount];
  typedef boost::shared_ptr<LiveLists> LiveListsPtr;
  static LiveListsPtr CreateNewLiveLists() {
    return LiveListsPtr(new LiveLists[DebuggerClient::AutoCompleteCount]());
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
                              int *nameLen = NULL);
  static String FormatTitle(const char *title);

public:
  DebuggerClient(std::string name = ""); // name only for api usage
  ~DebuggerClient();
  void reset();

  /**
   * Thread functions.
   */
  void start(const DebuggerClientOptions &options);
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

  bool code(CStrRef source, int lineFocus = 0, int line1 = 0, int line2 = 0,
            int charFocus0 = 0, int lineFocus1 = 0, int charFocus1 = 0);
  void shortCode(BreakPointInfoPtr bp);
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
  void setCommand(const std::string &cmd) { m_command = cmd;}
  bool arg(int index, const char *s);
  int argCount() { return m_args.size();}
  std::string argValue(int index);
  // The entire line after that argument, un-escaped.
  std::string lineRest(int index);
  StringVec *args() { return &m_args;}

  /**
   * Send the commmand to DebuggerProxy and expect same type of command back.
   */
  template<typename T> boost::shared_ptr<T> xend(DebuggerCommand *cmd) {
    return boost::static_pointer_cast<T>(xend(cmd));
  }
  void send(DebuggerCommand *cmd);
  DebuggerCommandPtr recv(int expected);

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
  InstPointInfoPtrVec *getInstPoints() { return &m_instPoints;}
  void setInstPoints(InstPointInfoPtrVec &ips) { m_instPoints = ips;}
  void setMatchedBreakPoints(BreakPointInfoPtrVec breakpoints);
  void setCurrentLocation(int64 threadId, BreakPointInfoPtr breakpoint);
  BreakPointInfoPtrVec *getMatchedBreakPoints() { return &m_matched;}
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
  void setLiveLists(LiveListsPtr liveLists) { m_acLiveLists = liveLists;}

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
  bool isTakingCommand() const { return m_inputState == TakingCommand; }
  void setTakingInterrupt() { m_inputState = TakingInterrupt; }
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

private:
  enum InputState {
    TakingCommand,
    TakingCode,
    TakingInterrupt
  };
  enum RunState {
    NotYet,
    Running,
    Stopped
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
  std::string m_printFunction;
  std::set<std::string> m_tutorialVisited;

  DECLARE_DBG_SETTING
  DECLARE_DBG_CLIENT_SETTING

  std::string m_logFile;
  FILE* m_logFileHandler;

  DebuggerClientOptions m_options;
  AsyncFunc<DebuggerClient> m_mainThread;
  bool m_stopped;
  bool m_quitting;

  InputState m_inputState;
  RunState m_runState;
  int m_signum;
  int m_sigTime;

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

  DMachineInfoPtrVec m_machines; // all connected ones
  DMachineInfoPtr m_machine;     // current
  std::string m_rpcHost;         // current RPC host

  DSandboxInfoPtrVec m_sandboxes;
  DThreadInfoPtrVec m_threads;
  int64 m_threadId;
  std::map<int64, int> m_threadIdMap; // maps threadId to index

  BreakPointInfoPtrVec m_breakpoints;
  BreakPointInfoPtr m_breakpoint;
  BreakPointInfoPtrVec m_matched;
  InstPointInfoPtrVec m_instPoints;

  // list command's current location, which may be different from m_breakpoint
  std::string m_listFile;
  int m_listLine;
  int m_listLineFocus;

  WatchPtrVec m_watches;

  Array m_stacktrace;
  int m_frame;

  ClientState m_clientState;
  std::string m_sourceRoot;
  std::string m_outputBuf;

  // helpers
  void runImpl();
  std::string getPrompt();
  void addToken(std::string &token, int idx);
  void parseCommand(const char *line);
  void shiftCommand();
  bool parse(const char *line);
  bool match(const char *cmd);
  int  checkEvalEnd();
  bool processTakeCode();
  bool processEval();
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
  void switchMachine(DMachineInfoPtr machine);
  SmartPtr<Socket> connectLocal();
  bool connectRemote(const std::string &host, int port);

  DebuggerCommandPtr xend(DebuggerCommand *cmd);

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
  FILE* m_usageLogFP;
  std::string m_usageLogHeader;
  void initUsageLogging();
  void finiUsageLogging();
  void usageLog(const std::string& cmd, const std::string& line);
  void usageLogInit() { usageLog("init", ""); }
  void usageLogSignal() { usageLog("signal", ""); }
  void usageLogDone(const std::string& cmdType) { usageLog("done", cmdType); }
  void usageLogInterrupt(DebuggerCommandPtr cmd);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CLIENT_H__
