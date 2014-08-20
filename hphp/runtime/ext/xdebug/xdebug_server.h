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
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

#ifdef ERROR
#undef ERROR
#endif

#define DBGP_VERSION "1.0"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

class XDebugServer {
////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

public:
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

///////////////////////////////////////////////////////////////////////////////
// Dbgp

private:

  // Called in construction. Helper for initializing the dbgp protocol with the
  // client.
  void initDbgp();

  // Called on destruction. Helper for shutting down the dbgp protocol
  void deinitDbgp();

  // Helpers adding various attributes to the passed node
  void addXmnls(xdebug_xml_node& node);
  void addCmdAndTrans(xdebug_xml_node& node);
  void addStatus(xdebug_xml_node& node);

  // Sends the passed xml messaeg to the client
  void sendMessage(xdebug_xml_node& xml);

  // Last transaction id sent by the client
  char* m_lastTransactionId = nullptr;

/////////////////////////////////////////////////////////////////////////////
// Commands

public:
  enum class Command {
    // TODO(#4489053) Implement this
    NONE
  };

  // Blocks waiting for commands from the client
  const char* getCommandStr(Command command) {
    // TODO(#4489053) Implement this
    return nullptr;
  }

private:
  // Blocks waiting for commands from the client
  void doCommandLoop();

  // Reads the input from the client until a null character is received.
  void readInput();

  Command m_lastCommand = Command::NONE;
  char* m_buffer = nullptr;
  size_t m_bufferSize = 0;

////////////////////////////////////////////////////////////////////////////////
// Logging

private:
  // Logs the string defined by the passed format string to the logfile, if the
  // logfile exists.
  inline void log(const char* format, ...) {
    if (m_logFile == nullptr) {
      return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(m_logFile, format, args);
    va_end(args);
  }

  // Flushes the logfile if it exists
  inline void logFlush() {
    if (m_logFile != nullptr) {
      fflush(m_logFile);
    }
  }

  FILE* m_logFile = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Server Status

private:
  enum class Status {
    STARTING,
    STOPPING,
    STOPPED,
    RUNNING,
    BREAK,
    DETACHED
  };

  // Reason for the current state
  enum class Reason {
    OK,
    ERROR,
    ABORTED,
    EXCEPTION
  };

  // Sets the status of the webserver
  void setStatus(Status status, Reason reason) {
    m_status = status;
    m_reason = reason;
  }

  // Corresponding string for status/reason
  const char* getStatusString(Status status);
  const char* getReasonString(Reason reason);

  // Set on dbgp init
  Status m_status = Status::DETACHED;
  Reason m_reason = Reason::OK;

////////////////////////////////////////////////////////////////////////////////
// Misc Data Members

private:
  Mode m_mode; // Set by constructor
  int m_socket = -1;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_SERVER_H_
