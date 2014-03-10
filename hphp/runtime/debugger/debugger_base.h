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

#ifndef incl_HPHP_EVAL_DEBUGGER_BASE_H_
#define incl_HPHP_EVAL_DEBUGGER_BASE_H_

#include <memory>
#include <vector>
#include <string>

#include "hphp/runtime/debugger/break_point.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/util/hdf.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
// startup options for debugger client

struct DebuggerClientOptions {
  std::string host;
  int port;
  std::string extension;
  std::vector<std::string> cmds;
  std::string sandbox;
  std::string user;
  std::string configFName;
  std::string fileName;

  DebuggerClientOptions() : port(-1) {}
};

///////////////////////////////////////////////////////////////////////////////
// exceptions

// Client-side exceptions
class DebuggerClientException : public Exception {};

// Exception used to force the debugger client to exit the command prompt loop
// implemented in DebuggerClient::console(). Commands throw this when they do
// something that causes the server to start running code again, so we pop out
// of the command prompt loop and go back to waiting for interrupt messages from
// the server.
class DebuggerConsoleExitException : public DebuggerClientException {};

// Exception thrown when the client detects an error in the communication
// protocol with the server, but believes the connection is still alive.
// I.e., bad command type back, missing fields, etc.
class DebuggerProtocolException : public DebuggerClientException {};

// Exception thrown when the client loses its connection to the server.
class DebuggerServerLostException : public DebuggerClientException {};

// Both client- and server-side exceptions
class DebuggerException : public Exception {
  EXCEPTION_COMMON_IMPL(DebuggerException);
};

// Exception thrown in two cases:
//
// Client-side: thrown in cases where the client should completely exit. This
// will pop us out of both the command prompt loop and the message loop for the
// server, and cause the client to quit.
//
// Server-side: thrown when the server detects that the client is exiting. This
// causes a thread which is currently interrupted to terminate it's request with
// this exception. The message attempts to reflect that a request which was
// being debugged has been terminated.
class DebuggerClientExitException  : public DebuggerException {
  virtual const char *what() const throw() {
    return "Debugger client has just quit, request (if any) terminated.";
  }
  EXCEPTION_COMMON_IMPL(DebuggerClientExitException);
};

class DebuggerRestartException     : public DebuggerException {
public:
  explicit DebuggerRestartException(
    std::shared_ptr<std::vector<std::string>> args) : m_args(args) {}
  ~DebuggerRestartException() throw() {}

  virtual const char *what() const throw() {
    return "Debugger restarting program or aborting web request.";
  }
  EXCEPTION_COMMON_IMPL(DebuggerRestartException);

  std::shared_ptr<std::vector<std::string>> m_args;
};

///////////////////////////////////////////////////////////////////////////////
// utility functions

enum CodeColor {
  CodeColorNone,
  CodeColorKeyword,
  CodeColorComment,
  CodeColorString,
  CodeColorVariable,
  CodeColorHtml,
  CodeColorTag,
  CodeColorDeclaration,
  CodeColorConstant,
  CodeColorLineNo
};

/**
 * "line", starting line number, or 0 for no line number display.
 * "lineFocus", the line to highlight, with gray background.
 * highlight_code() doesn't need <?php and will treat source entirely PHP.
 */
String highlight_php(const String& source, int line = 0, int lineFocus0 = 0,
                     int charFocus0 = 0, int lineFocus1 = 0,
                     int charFocus1 = 0);
String highlight_code(const String& source, int line = 0, int lineFocus0 = 0,
                      int charFocus0 = 0, int lineFocus1 = 0,
                      int charFocus1 = 0);

extern const char *PHP_KEYWORDS[];

///////////////////////////////////////////////////////////////////////////////

struct DSandboxInfo;
struct DMachineInfo;

using DSandboxInfoPtr = std::shared_ptr<DSandboxInfo>;

class DMachineInfo {
public:
  DMachineInfo()
      : m_port(0), m_interrupting(false), m_sandboxAttached(false),
        m_initialized(false), m_rpcPort(0) {}

  std::string m_name;
  int m_port;
  DebuggerThriftBuffer m_thrift;

  bool m_interrupting; // True if the machine is paused at an interrupt
  bool m_sandboxAttached;
  DSandboxInfoPtr m_sandbox;
  bool m_initialized; // True if the initial connection protocol is complete
  std::string m_rpcHost;
  int m_rpcPort;
};

///////////////////////////////////////////////////////////////////////////////

class DSandboxInfo {
public:
  DSandboxInfo() {}
  explicit DSandboxInfo(const std::string &id) { set(id);}

  std::string m_user;
  std::string m_name;
  std::string m_path;

  const std::string &id() const;
  const std::string desc() const;
  static DSandboxInfo CreateDummyInfo(uint64_t unique);

  bool valid() const { return !m_user.empty(); }
  void set(const std::string &id);
  void update(const DSandboxInfo &src);

  void sendImpl(ThriftBuffer &thrift);
  void recvImpl(ThriftBuffer &thrift);

private:
  mutable std::string m_cached_id;
};

///////////////////////////////////////////////////////////////////////////////

class DThreadInfo {
public:
  int64_t m_id;
  std::string m_desc;
  std::string m_type;
  std::string m_url;

  int m_index; // used by DebuggerClient

  void sendImpl(ThriftBuffer &thrift);
  void recvImpl(ThriftBuffer &thrift);
};

using DThreadInfoPtr = std::shared_ptr<DThreadInfo>;

///////////////////////////////////////////////////////////////////////////////

class BreakPointInfo;
class DFunctionInfo {
public:
  std::string m_namespace;
  std::string m_class;
  std::string m_function;

  std::string getName() const;
  std::string site(std::string &preposition) const;
  std::string desc(const BreakPointInfo *bpi) const;

  void sendImpl(ThriftBuffer &thrift);
  void recvImpl(ThriftBuffer &thrift);
};

///////////////////////////////////////////////////////////////////////////////

class Macro {
public:
  std::string m_name;
  std::vector<std::string> m_cmds;

  unsigned int m_index; // currently playing position

  std::string desc(const char *indent);
  void load(Hdf node);
  void save(std::ostream &stream, int key);
};

///////////////////////////////////////////////////////////////////////////////
// Simple base class which can be overridden to provide implementation-specific
// usage logging for the debugger from both client- and server-side.

class DebuggerUsageLogger {
public:
  virtual ~DebuggerUsageLogger() {}
  virtual void init() {}
  virtual void log(const std::string &mode, const std::string &sandboxId,
                   const std::string &cmd, const std::string &data) {}
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_BASE_H_
