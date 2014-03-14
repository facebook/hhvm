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

#ifndef incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_
#define incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_

#include <memory>
#include <vector>
#include <list>
#include <utility>

#include "hphp/runtime/debugger/debugger_thrift_buffer.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

// The type of interrupt that is sent from the server to notify the debugger
// client about a notable event during execution.
enum InterruptType : int8_t {
  // The server is now ready to interact with the debugger
  SessionStarted,
  // The server has terminated the debug session.
  SessionEnded,
  // The server has received a web request
  RequestStarted,
  // The server has sent a response to the web request
  RequestEnded,
  // The server has finished all processing of a web request
  // also known as Post Send Processing has Ended.
  PSPEnded,
  // The server has executed f_hphpd_break()
  HardBreakPoint,
  // The server has reached a point where it has been told to stop and wait
  // for the debugger to tell it to resume execution. For example,
  // a user breakpoint has been reached, or a step command has completed.
  BreakPointReached,
  // The server is about throw an exception
  ExceptionThrown,

  // The server has reached the start of an exception handler.
  ExceptionHandler,
  // The above type of interrupt is not sent from the server to the debugger
  // but is used for flow control inside the server. We could consider exposing
  // this type of interrupt to clients, and thus allowing users to request the
  // server to break execution when an interrupt handler is reached, but the
  // value seems quite low at this time.
  // We have assertions that check that these interrupts stays server-side.
};

// Represents a site in the code, at the source level.
// Forms an InterruptSite by looking at the current thread's current PC and
// grabbing source data out of the corresponding Unit.
class InterruptSite {
public:
  InterruptSite(bool hardBreakPoint, const Variant& e);

  const InterruptSite *getCallingSite() const;
  const char *getFile() const { return m_file.data(); }
  const char *getClass() const { return m_class ? m_class : ""; }
  const char *getFunction() const { return m_function.data(); }
  // Placeholder for future namespace support.
  const char *getNamespace() const { return nullptr; }
  int getFileLen() const;

  int32_t getLine0() const { return m_line0; }
  int32_t getChar0() const { return m_char0; }
  int32_t getLine1() const { return m_line1; }
  int32_t getChar1() const { return m_char1; }

  // Optionally provided by VM, could be an exception object, a string, or null
  // depending on the context.
  const Variant& getError() { return m_error; }

  std::string &url() const { return m_url; }
  std::string desc() const;

  const SourceLoc *getSourceLoc() const { return &m_sourceLoc; }
  const Offset getCurOffset() const { return m_offset; }
  const Unit* getUnit() const { return m_unit; }

  bool valid() const { return m_valid; }
  bool funcEntry() const { return m_funcEntry; }

private:
  InterruptSite(ActRec* fp, Offset offset, const Variant& error);
  void Initialize(ActRec *fp);

  Variant m_error;
  ActRec *m_activationRecord;

  // cached
  mutable std::unique_ptr<const InterruptSite> m_callingSite;
  mutable const char *m_class;
  mutable std::string m_function;
  mutable String m_file;
  mutable std::string m_url;

  int32_t m_line0;
  int32_t m_char0;
  int32_t m_line1;
  int32_t m_char1;

  SourceLoc m_sourceLoc;
  Offset m_offset;
  Unit* m_unit;
  bool m_valid;
  bool m_funcEntry;
};

///////////////////////////////////////////////////////////////////////////////

struct BreakPointInfo;
struct DebuggerProxy;
struct DFunctionInfo;

using BreakPointInfoPtr = std::shared_ptr<BreakPointInfo>;
using DebuggerProxyPtr = std::shared_ptr<DebuggerProxy>;

class BreakPointInfo {
public:
  // The state of the break point
  enum State : int8_t {
    Always   = -1, // always break when reaching this break point
    Once     = 1, // break the first time, then disable
    Disabled = 0, // carry on with execution when reaching this break point
  };

  // Does the break point correspond to a known executable location?
  enum BindState : int8_t {
    KnownToBeValid, // Breakpoint refers to valid location or member
    KnownToBeInvalid, // Breakpoint cannot be bound (no such class or line)
    Unknown, // The file or class referenced by breakpoint is not loaded
  };

  static const char *ErrorClassName;

  static const char *GetInterruptName(InterruptType interrupt);
  static bool MatchFile(const char *haystack, int haystack_len,
                        const std::string &needle);
  static bool MatchFile(const std::string& file, const std::string& fullPath);

public:
  BreakPointInfo() : m_index(0) {} // for thrift
  BreakPointInfo(bool regex, State state, const std::string &file, int line);
  BreakPointInfo(bool regex, State state, InterruptType interrupt,
                 const std::string &url);
  BreakPointInfo(bool regex, State state, InterruptType interrupt,
                 const std::string &exp, const std::string &file);
  ~BreakPointInfo();

  void setClause(const std::string &clause, bool check);
  void toggle();
  void transferStack(BreakPointInfoPtr bpi);
  void setState(State state) { m_state = state; }

  bool valid();
  bool same(BreakPointInfoPtr bpi);
  bool match(DebuggerProxy &proxy, InterruptType interrupt,
      InterruptSite &site);
  bool cmatch(DebuggerProxy &proxy, InterruptType interrupt,
      InterruptSite &site);

  int index() const { return m_index;}
  std::string state(bool padding) const;
  std::string desc() const;

  std::string site() const;
  std::string regex(const std::string &name) const;

  void sendImpl(int version, DebuggerThriftBuffer &thrift);
  void recvImpl(int version, DebuggerThriftBuffer &thrift);

  static void SendImpl(int version,
                       const std::vector<BreakPointInfoPtr>& bps,
                       DebuggerThriftBuffer &thrift);
  static void RecvImpl(int version,
                       std::vector<BreakPointInfoPtr>& bps,
                       DebuggerThriftBuffer &thrift);

  bool breakable(int stackDepth, Offset offset) const;
  void unsetBreakable(int stackDepth, Offset offset);
  void setBreakable(int stackDepth);

  int16_t m_index; // client side index number

  State m_state; // Always, Once, Disabled
  BindState m_bindState; // KnownToBeValid, KnownToBeInvalid, Unknown
  bool m_valid; // false if syntactically invalid
  InterruptType m_interruptType; // Why this break point was reached

  // file::line1-line2
  std::string m_file;
  int32_t m_line1;
  int32_t m_line2;
  int32_t m_char1;
  int32_t m_char2;

  // class::func()
  std::vector<std::shared_ptr<DFunctionInfo>> m_funcs;

  std::string getNamespace() const;
  std::string getClass() const;
  std::string getFunction() const;
  std::string getFuncName() const;
  std::string getExceptionClass() const { return m_class; }

  // URL
  std::string m_url;

  // whether strings are regex
  bool m_regex;

  // "if", "&&" clause
  bool m_check;
  std::string m_clause;
  std::string m_php; // cached

  // server results
  std::string m_output;
  std::string m_exceptionClass;
  std::string m_exceptionObject;

private:
  // exception class
  std::string m_namespace;
  std::string m_class;
  // Records the stack depth and offset of first operation for each break point
  // that is currently disabled except at deeper stack levels.
  std::list<std::pair<int, Offset>> m_stack;

  static bool Match(const char *haystack, int haystack_len,
                    const std::string &needle, bool regex, bool exact);
  static bool MatchClass(const char *fcls, const std::string &bcls,
                         bool regex, const char *func);
  bool match(DebuggerProxy &proxy, InterruptType interrupt,
      InterruptSite &site, bool evalClause);

  void createIndex();
  std::string descBreakPointReached() const;
  std::string descExceptionThrown() const;

  void parseExceptionThrown(const std::string &exp);
  void parseBreakPointReached(const std::string &exp, const std::string &file);
  int32_t parseFileLocation(const std::string &str, int32_t offset);
  bool parseLines(const std::string &token);

  bool checkExceptionOrError(const Variant& e);
  bool checkUrl(std::string &url);
  bool checkLines(int line);
  bool checkStack(InterruptSite &site);
  bool checkClause(DebuggerProxy &proxy);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_
