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

#include "hphp/util/async-func.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

auto constexpr DBGP_VERSION = "1.0";

struct ThreadInfo;
struct XDebugBreakpoint;
struct XDebugCommand;
struct xdebug_xml_node;

////////////////////////////////////////////////////////////////////////////////

struct XDebugServer {

  enum class Mode {
    /* Server created during request init. */
    Req,
    /* Server created on demand. */
    Jit,
  };

  /* State to put the polling thread into. */
  enum class PollingState : uint8_t {
    Run,

    /* Wait while the request thread parses and runs commands. */
    Pause,

    /* Stop running, exit the thread. */
    Stop,
  };

  /*
   * An XDebugServer is only valid if the constructor succeeds.  An exception is
   * thrown otherwise.  The constructor is responsible for establishing a valid
   * dbgp connection with the client.
   */
  explicit XDebugServer(Mode mode);
  ~XDebugServer();

  //////////////////////////////////////////////////////////////////////////////

  static void onRequestInit();

  /*
   * Returns true if the xdebug server is needed by this thread.  If remote_mode
   * is "jit" then this always returns false as whether or not the server is
   * needed is decided at runtime.
   */
  static bool isNeeded();

  /*
   * Returns true if the xdebug server is attached to the current thread.
   */
  static bool isAttached() {
    return XDEBUG_GLOBAL(Server) != nullptr;
  }

  /*
   * Attempts to attach the xdebug server to the current thread.  Assumes it is
   * not already attached.  Raises a warning on failure.  The actual error will
   * be written to the remote debugging log.
   */
  static void attach(Mode mode);

  /*
   * Assumes an xdebug server is attached to the thread and attempts to detach
   * it.
   */
  static void detach();

  //////////////////////////////////////////////////////////////////////////////

  /* Adds the status of the server to the given node. */
  void addStatus(xdebug_xml_node& node);

  /* Adds the passed command to the given node. */
  void addCommand(xdebug_xml_node& node, const XDebugCommand& cmd);

  /* Send the given stream of information to the client. */
  void sendStream(const char* name, const char* bytes, int len);

  /* Sends the passed xml message to the client. */
  void sendMessage(xdebug_xml_node& xml);

  /* Sends response to last parsed command. */
  void sendResponse(xdebug_xml_node& xml);

  /* Send xml response message for input error to the client. */
  void sendErrorMessage(
    const std::shared_ptr<XDebugCommand>& cmd,
    const XDebugExn& error
  );

  /* Adds the xdebug xmlns to the node. */
  void addXmlns(xdebug_xml_node& node);

  /* Add the specified error to the given node. */
  void addError(xdebug_xml_node& node, const XDebugExn& ex);

  //////////////////////////////////////////////////////////////////////////////

  /* Sets the status of the webserver. */
  void setStatus(XDebugStatus status, XDebugReason reason) {
    m_status = status;
    m_reason = reason;
  }

  /* Store the status and its reason in the passed arguments. */
  void getStatus(XDebugStatus& status, XDebugReason& reason) {
    status = m_status;
    reason = m_reason;
  }

  //////////////////////////////////////////////////////////////////////////////

  /*
   * Performs a breakpoint, sending the passed info to the client.  This method
   * blocks until the breakpoint has finished.  True is returned on success,
   * false on failure (for instance, if the client disconnects).
   */
  bool breakpoint(const XDebugBreakpoint& bp,
                  const Variant& exnName,
                  const Variant& message);
  bool breakpoint(const Variant& filename,
                  const Variant& exception,
                  const Variant& message,
                  int line);

  void processAsyncCommandQueue();
  bool processCommand(const std::shared_ptr<XDebugCommand>& cmd);

  void addNewCommand(const std::shared_ptr<XDebugCommand>& cmd) {
    std::lock_guard<std::mutex> guard(m_asyncCommandQueueMtx);
    m_asyncCommandQueue.emplace_back(cmd);
  }


  //////////////////////////////////////////////////////////////////////////////

  /*
   * Logs the string defined by the passed format string to the logfile, if the
   * logfile exists.
   */
  void log(const char* format, ...) {
    if (m_logFile == nullptr) {
      return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(m_logFile, format, args);
    va_end(args);
  }

  /* Flushes the logfile if it exists. */
  void logFlush() {
    if (m_logFile != nullptr) {
      fflush(m_logFile);
    }
  }

 private:

  /*
   * Helpers for sending the initialization and shutdown messages to the
   * debugging client.  initDbgp() will return true on success.
   */
  bool initDbgp();
  void deinitDbgp();

  /*
   * Blocks waiting for commands from the client. Returns false if there was an
   * error.
   */
  bool doCommandLoop();

  /*
   * Add the last parsed command if it is available.
   */
  void addLastCommandIfAvailable(xdebug_xml_node& node);

  /*
   * Reads the input from the client until a null character is received.
   * Returns true on success.
   */
  bool readInput();

  /*
   * Parses the input from the buffer and returns an instance of the
   * corresponding command.  Throws an XDebugError on failure.
   */
  std::shared_ptr<XDebugCommand> parseCommand();

  /*
   * Grab the command and an array of arguments from the given input string.
   * This was taken and translated from php5 xdebug in order match parsing
   * behavior.  Throws an XDebugError on failure.
   */
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

  void closeLog();

  /*
   * Looks up the given hostname and stores the results in "in".  Returns true
   * on success.
   */
  bool lookupHostname(const char* hostname, struct in_addr& in);

  /*
   * Initializes and connects to the client, defined by the given hostname:port.
   * Returns the socket fd, -1 on connection error, or -2 on timeout error.
   */
  int createSocket(const char* hostname, int port);

  void destroySocket();

  //////////////////////////////////////////////////////////////////////////////

 public:

  /*
   * Options for controlling value serialization.
   */

  /* Max number of array or object children. */
  int m_maxChildren{32};

  /* Max variable data (string length). */
  int m_maxData{1024};

  /* Max depth of arrays/objects. */
  int m_maxDepth{1};

  /* Whether hidden members are shown. */
  bool m_showHidden{false};

  /* Whether IDE supports notification or not. */
  bool m_supportsNotify{false};

 private:
  std::vector<std::shared_ptr<XDebugCommand>> m_lastCommands;
  char* m_buffer{nullptr};
  char* m_bufferCur{nullptr};
  size_t m_bufferAvail{0};
  size_t m_bufferSize{0};

  AsyncFunc<XDebugServer> m_pollingThread;

  ThreadInfo* m_requestThread{nullptr};

  /* Mutex that protects the socket. */
  std::recursive_mutex m_pollingMtx;

  /* Mutex that protects m_asyncCommandQueue. */
  std::mutex m_asyncCommandQueueMtx;

  /*
   * The async commands queue that gets read by the polling thread but
   * processed by request thread.
   */
  std::vector<std::shared_ptr<XDebugCommand>> m_asyncCommandQueue;

  /* The request thread sets this to tell the polling thread what to do. */
  std::atomic<PollingState> m_pollingState{PollingState::Run};

  FILE* m_logFile{nullptr};

  XDebugStatus m_status{XDebugStatus::Detached};
  XDebugReason m_reason{XDebugReason::Ok};

  Mode m_mode;
  int m_socket{-1};
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
