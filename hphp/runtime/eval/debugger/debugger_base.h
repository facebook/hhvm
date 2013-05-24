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

#ifndef incl_HPHP_EVAL_DEBUGGER_BASE_H_
#define incl_HPHP_EVAL_DEBUGGER_BASE_H_

#include "hphp/runtime/eval/debugger/break_point.h"
#include "hphp/runtime/base/util/string_buffer.h"
#include "hphp/runtime/base/util/exceptions.h"
#include "hphp/util/hdf.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
// startup options for debugger client

struct DebuggerClientOptions {
  std::string host;
  int port;
  std::string extension;
  StringVec cmds;
  std::string sandbox;
  std::string user;
  bool apiMode;
  std::string configFName;

  DebuggerClientOptions() : port(-1), apiMode(false) {}
};

///////////////////////////////////////////////////////////////////////////////
// exceptions

// client side exception
class DebuggerClientException      : public Exception {};
class DebuggerConsoleExitException : public DebuggerClientException {};
class DebuggerProtocolException    : public DebuggerClientException {};
class DebuggerServerLostException  : public DebuggerClientException {};

// both client and server side exception
class DebuggerException            : public Exception {
  EXCEPTION_COMMON_IMPL(DebuggerException);
};
class DebuggerClientExitException  : public DebuggerException {
  virtual const char *what() const throw() {
    return "Debugger client has just quit.";
  }
  EXCEPTION_COMMON_IMPL(DebuggerClientExitException);
};
class DebuggerRestartException     : public DebuggerException {
public:
  explicit DebuggerRestartException(StringVecPtr args) : m_args(args) {}
  ~DebuggerRestartException() throw() {}

  virtual const char *what() const throw() {
    return "Debugger restarting program or aborting web request.";
  }
  EXCEPTION_COMMON_IMPL(DebuggerRestartException);

  StringVecPtr m_args;
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
String highlight_php(CStrRef source, int line = 0, int lineFocus0 = 0,
                     int charFocus0 = 0, int lineFocus1 = 0,
                     int charFocus1 = 0);
String highlight_code(CStrRef source, int line = 0, int lineFocus0 = 0,
                      int charFocus0 = 0, int lineFocus1 = 0,
                      int charFocus1 = 0);

extern const char *PHP_KEYWORDS[];

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DMachineInfo);
class DMachineInfo {
public:
  DMachineInfo()
      : m_port(0), m_interrupting(false), m_sandboxAttached(false),
        m_initialized(false), m_rpcPort(0) {}

  std::string m_name;
  int m_port;
  DebuggerThriftBuffer m_thrift;

  bool m_interrupting;
  bool m_sandboxAttached;
  bool m_initialized;
  std::string m_rpcHost;
  int m_rpcPort;
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DSandboxInfo);
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

DECLARE_BOOST_TYPES(DThreadInfo);
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

///////////////////////////////////////////////////////////////////////////////

class BreakPointInfo;
DECLARE_BOOST_TYPES(DFunctionInfo);
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

DECLARE_BOOST_TYPES(Macro);
class Macro {
public:
  std::string m_name;
  StringVec m_cmds;

  unsigned int m_index; // currently playing position

  std::string desc(const char *indent);
  void load(Hdf node);
  void save(Hdf node);
};

///////////////////////////////////////////////////////////////////////////////
// Simple base class which can be overridden to provide implementation-specific
// usage logging for the debugger from both client- and server-side.

class DebuggerUsageLogger {
public:
  virtual ~DebuggerUsageLogger() {}
  virtual void init() {}
  virtual void log(const std::string &mode, const std::string &cmd,
                   const std::string &data) {}
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_BASE_H_
