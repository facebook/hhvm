/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/debugger/debugger_thrift_buffer.h"
#include "hphp/util/hdf.h"

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////
// startup options for debugger client

struct DebuggerClientOptions {
  std::string host;
  int port{-1};
  std::string extension;
  std::vector<std::string> cmds;
  std::string sandbox;
  std::string user;
  std::string configFName;
  std::string fileName;
};

///////////////////////////////////////////////////////////////////////////////
// exceptions

// Client-side exceptions
struct DebuggerClientException : Exception {};

// Exception used to force the debugger client to exit the command prompt loop
// implemented in DebuggerClient::console(). Commands throw this when they do
// something that causes the server to start running code again, so we pop out
// of the command prompt loop and go back to waiting for interrupt messages from
// the server.
struct DebuggerConsoleExitException : DebuggerClientException {};

// Exception thrown when the client detects an error in the communication
// protocol with the server, but believes the connection is still alive.
// I.e., bad command type back, missing fields, etc.
struct DebuggerProtocolException : DebuggerClientException {};

// Exception thrown when the client loses its connection to the server.
struct DebuggerServerLostException : DebuggerClientException {};

// Both client- and server-side exceptions
struct DebuggerException : Exception {
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
struct DebuggerClientExitException : DebuggerException {
  const char* what() const noexcept override {
    return "Debugger client has just quit, request (if any) terminated.";
  }
  EXCEPTION_COMMON_IMPL(DebuggerClientExitException);
};

// Exception thrown when a DebuggerClientExitException occurs specifically
// due to a failure to set hphpd as the active debugger for the HHVM instance.
struct DebuggerClientAttachFailureException : DebuggerClientExitException {
  const char* what() const noexcept override {
    return "Debugger client was unable to attach to the request thread. "
      "Another debugger is already attached.";
  }
};

struct DebuggerRestartException : DebuggerException {
  explicit DebuggerRestartException(
    std::shared_ptr<std::vector<std::string>>& args): m_args(args) {}

  const char* what() const noexcept override {
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
 * highlight_code() doesn't need <?hh and will treat source entirely PHP.
 */
String highlight_php(const String& source, int line = 0, int lineFocus0 = 0,
                     int charFocus0 = 0, int lineFocus1 = 0,
                     int charFocus1 = 0);
String highlight_code(const String& source, int line = 0, int lineFocus0 = 0,
                      int charFocus0 = 0, int lineFocus1 = 0,
                      int charFocus1 = 0);

extern const char* PHP_KEYWORDS[];

///////////////////////////////////////////////////////////////////////////////

struct BreakPointInfo;
struct DSandboxInfo;
struct DMachineInfo;

using DSandboxInfoPtr = std::shared_ptr<DSandboxInfo>;

struct DMachineInfo {
  std::string m_name;
  int m_port{0};
  DebuggerThriftBuffer m_thrift;

  DSandboxInfoPtr m_sandbox;
  bool m_interrupting{false}; // If the machine is paused at an interrupt
  bool m_sandboxAttached{false};
  bool m_initialized{false}; // If the initial connection protocol is complete
};

///////////////////////////////////////////////////////////////////////////////

struct DSandboxInfo {
  DSandboxInfo() {}
  explicit DSandboxInfo(const std::string &id) { set(id); }

  std::string m_user;
  std::string m_name;
  std::string m_path;

  const std::string& id() const;
  const std::string desc() const;
  static DSandboxInfo CreateDummyInfo(uint64_t unique);

  bool valid() const { return !m_user.empty(); }
  void set(const std::string& id);
  void update(const DSandboxInfo& src);

  void sendImpl(ThriftBuffer& thrift);
  void recvImpl(ThriftBuffer& thrift);

private:
  mutable std::string m_cached_id;
};

///////////////////////////////////////////////////////////////////////////////

struct DThreadInfo {
  int64_t m_id;
  std::string m_desc;
  std::string m_type;
  std::string m_url;

  int m_index; // used by DebuggerClient

  void sendImpl(ThriftBuffer& thrift);
  void recvImpl(ThriftBuffer& thrift);
};

using DThreadInfoPtr = std::shared_ptr<DThreadInfo>;

///////////////////////////////////////////////////////////////////////////////

struct DFunctionInfo {
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

struct Macro {
  std::string m_name;
  std::vector<std::string> m_cmds;

  unsigned m_index; // currently playing position

  std::string desc(const char *indent);
  void load(const IniSetting::Map& ini, Hdf node);
  void save(std::ostream &stream, int key);
};

///////////////////////////////////////////////////////////////////////////////
// Simple base class which can be overridden to provide implementation-specific
// usage logging for the debugger from both client- and server-side.

struct DebuggerUsageLogger {
  virtual ~DebuggerUsageLogger() {}
  virtual void init() {}
  virtual void clearClientInfo() {}
  virtual void setClientInfo(const std::string& /*username*/, uid_t /*uid*/,
                             pid_t /*clientPid*/) {}
  virtual void
  log(const std::string& /*clientId*/, const std::string& /*mode*/,
      const std::string& /*sandboxId*/, const std::string& /*cmd*/,
      const std::string& /*data*/) {}
};

///////////////////////////////////////////////////////////////////////////////
}

