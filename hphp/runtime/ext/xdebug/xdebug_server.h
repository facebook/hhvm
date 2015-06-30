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

#ifndef incl_HPHP_XDEBUG_SERVER_H_
#define incl_HPHP_XDEBUG_SERVER_H_

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/ext/xdebug/status.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"
#include "hphp/util/async-func.h"

#ifdef ERROR
#undef ERROR
#endif

#define DBGP_VERSION "1.0"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

struct ThreadInfo;
struct XDebugCommand;
struct XDebugBreakpoint;

struct XDebugServer {
////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

  enum class Mode {
    REQ, // Server created during request init
    JIT // Server created on demand
  };

  // An XDebugServer is only valid if the constructor succeeds. An exception is
  // thrown otherwise. The constructor is responsible for establishing a valid
  // dbgp connection with the client
  explicit XDebugServer(Mode mode);
  ~XDebugServer();

private:
  // Closes the logfile
  void closeLog();

  // Looks up the given hostname and stores the results in "in". Returns true on
  // success, false on failure
  bool lookupHostname(const char* hostname, struct in_addr& in);

  // Initializes and connects to the client, defined by the given hostname:port
  // Returns the socket fd, or -1 on connection error, or -2 on timeout error
  int createSocket(const char* hostname, int port);

  // Destroys the current socket
  void destroySocket();

////////////////////////////////////////////////////////////////////////////////
// Statics

public:
  // Request specific initialization
  static void onRequestInit();

  // Returns true if the xdebug server is needed by this thread. If remote_mode
  // is "jit" then this always returns false as whether or not the server is
  // needed is decided at runtime.
  static bool isNeeded();

  // Returns true if the xdebug server is attached to the current thread
  static inline bool isAttached() {
    return XDEBUG_GLOBAL(Server) != nullptr;
  }

  // Attempts to attach the xdebug server to the current thread. Assumes it
  // is not already attached. Raises a warning on failure. The actual error will
  // be written to the remote debugging log
  static void attach(Mode mode);

  // Assumes an xdebug server is attached to the thread and attempts to detach
  // it.
  static void detach();

///////////////////////////////////////////////////////////////////////////////
// Dbgp

public:
  // Called in construction. Helper for initializing the dbgp protocol with the
  // client. Returns true on success. False on failure.
  bool initDbgp();

  // Called on destruction. Helper for shutting down the dbgp protocol
  void deinitDbgp();

  // adds the status of the server to the given node
  void addStatus(xdebug_xml_node& node);

  // Adds the passed command to the given node
  void addCommand(xdebug_xml_node& node, const XDebugCommand& cmd);

  // Send the given stream of information to the client
  void sendStream(const char* name, const char* bytes, int len);

  // Sends the passed xml message to the client
  void sendMessage(xdebug_xml_node& xml);

  // Adds the xdebug xmlns to the node
  void addXmlns(xdebug_xml_node& node);

  // Add the error with the passed error code to the given node
  void addError(xdebug_xml_node& node, XDebugError code);

/////////////////////////////////////////////////////////////////////////////
// Commands

public:
  // Performs a breakpoint, sending the passed info to the client. This method
  // blocks until the breakpoint has finished. True is returned on success,
  // false on failure (for instance, if the client disconnects)
  bool breakpoint(const Variant& filename,
                  const Variant& exception,
                  const Variant& message,
                  int line);

  // Grabs the appropriate breakpoint info from the passed breakpoint and calls
  // breakpoint(filename, exception, message, line)
  bool breakpoint(const XDebugBreakpoint& bp, const Variant& message);

  // Remote debugging options. The defaults are taken from xdebug.
  int m_maxChildren = 32;    // max number of array or object children
  int m_maxData = 1024;      // max variable data (string length)
  int m_maxDepth = 1;        // max depth of arrays/objects
  bool m_showHidden = false; // whether hidden members are shown

private:
  // Blocks waiting for commands from the client. Returns false if there was
  // an error. True otherwise.
  bool doCommandLoop();

  // Reads the input from the client until a null character is received. Returns
  // true on success. false on failure.
  bool readInput();

  // Parses the input from the buffer and returns an instance of the
  // corresponding command. Throws an XDebugError on failure.
  XDebugCommand* parseCommand();

  // Valid states of the input parsing state machine
  enum class ParseState {
    NORMAL,
    QUOTED,
    OPT_FOLLOWS,
    SEP_FOLLOWS,
    VALUE_FOLLOWS_FIRST_CHAR,
    VALUE_FOLLOWS,
    SKIP_CHAR
  };

  // Grab the command and an array of arguments from the given input string.
  // This was taken and translated from php5 xdebug in order match parsing
  // behavior.  Throws an XDebugError on failure.
  void parseInput(folly::StringPiece in, String& cmd, Array& args);

  /*
   * Runs in a separate thread.
   *
   * Polls the XDebugServer's socket for commands when the debugged thread is
   * busy running.  If a break command is parsed out, then the polling thread
   * interrupts the running thread, and blocks.  If a different command is
   * parsed out, then it is dropped.
   */
  void pollSocketLoop();

public:
  /*
   * Simultaneously gets the result of 'm_break', and clears it.
   */
  XDebugCommand* getAndClearBreak() {
    return m_break.exchange(nullptr);
  }

private:

  const XDebugCommand* m_lastCommand = nullptr;
  char* m_buffer = nullptr;
  char* m_bufferCur = nullptr;
  size_t m_bufferAvail = 0;
  size_t m_bufferSize = 0;

  AsyncFunc<XDebugServer> m_pollingThread;

  ThreadInfo* m_requestThread{nullptr};

  /*
   * Mutex that protects m_{pause,stop}PollingThread.
   */
  std::mutex m_pollingMtx;

  /*
   * The "break" command that is read out by the polling thread is stored here.
   */
  std::atomic<XDebugCommand*> m_break{nullptr};

  /* Whether to pause or stop the polling thread. */
  std::atomic<bool> m_pausePollingThread{false};
  std::atomic<bool> m_stopPollingThread{false};

////////////////////////////////////////////////////////////////////////////////
// Logging

 public:
  // Logs the string defined by the passed format string to the logfile, if the
  // logfile exists.
  void log(const char* format, ...) {
    if (m_logFile == nullptr) {
      return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(m_logFile, format, args);
    va_end(args);
  }

  // Flushes the logfile if it exists
  void logFlush() {
    if (m_logFile != nullptr) {
      fflush(m_logFile);
    }
  }

private:
  FILE* m_logFile = nullptr;

public:
  // Sets the status of the webserver
  void setStatus(XDebugStatus status, XDebugReason reason) {
    m_status = status;
    m_reason = reason;
  }

  // Store the status and its reason in the passed arguments
  void getStatus(XDebugStatus& status, XDebugReason& reason) {
    status = m_status;
    reason = m_reason;
  }

private:
  /* Set on dbgp init. */
  XDebugStatus m_status{XDebugStatus::Detached};
  XDebugReason m_reason{XDebugReason::Ok};

////////////////////////////////////////////////////////////////////////////////
// Misc Data Members

private:
  Mode m_mode; // Set by constructor
  int m_socket = -1;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
