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

#ifndef incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_
#define incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_

#include <runtime/eval/debugger/debugger_thrift_buffer.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

enum InterruptType {
  SessionStarted,
  SessionEnded,
  RequestStarted,
  RequestEnded,
  PSPEnded,
  HardBreakPoint, // From f_hphpd_break().
  BreakPointReached,
  ExceptionThrown,
};

// Represents a site in the code, at the source level.
// Forms an InterruptSite by looking at the current thread's current PC and
// grabbing source data out of the corresponding Unit.
class InterruptSite {
public:
  InterruptSite(bool hardBreakPoint, CVarRef e);

  const char *getFile() const { return m_file.data(); }
  const char *getClass() const { return m_class ? m_class : ""; }
  const char *getFunction() const { return m_function ? m_function : ""; }
  // Placeholder for future namespace support.
  const char *getNamespace() const { return nullptr; }
  int getFileLen() const;

  int32_t getLine0() const { return m_line0; }
  int32_t getChar0() const { return m_char0; }
  int32_t getLine1() const { return m_line1; }
  int32_t getChar1() const { return m_char1; }
  CVarRef getException() { return m_exception; }

  std::string &url() const { return m_url; }
  std::string desc() const;

  const VM::SourceLoc *getSourceLoc() const { return &m_sourceLoc; }
  const VM::OffsetRangeVec& getCurOffsetRange() const {
    return m_offsetRangeVec;
  }
  const VM::Unit* getUnit() const { return m_unit; }

  bool valid() const { return m_valid; }
  bool funcEntry() const { return m_funcEntry; }

private:
  Variant m_exception;

  // cached
  mutable const char *m_class;
  mutable const char *m_function;
  mutable String m_file;
  mutable std::string m_url;

  int32_t m_line0;
  int32_t m_char0;
  int32_t m_line1;
  int32_t m_char1;

  VM::SourceLoc m_sourceLoc;
  VM::OffsetRangeVec m_offsetRangeVec;
  VM::Unit* m_unit;
  bool m_valid;
  bool m_funcEntry;
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DFunctionInfo);
DECLARE_BOOST_TYPES(BreakPointInfo);
class BreakPointInfo {
public:
  enum State {
    Always   = -1,
    Once     = 1,
    Disabled = 0,
  };

  static const char *ErrorClassName;

  static const char *GetInterruptName(InterruptType interrupt);
  static bool MatchFile(const char *haystack, int haystack_len,
                        const std::string &needle);
  static bool MatchFile(const std::string& file, const std::string& fullPath,
                        const std::string& relPath);

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
  void setState(State state) { m_state = state; }

  bool valid();
  bool same(BreakPointInfoPtr bpi);
  bool match(InterruptType interrupt, InterruptSite &site);

  int index() const { return m_index;}
  std::string state(bool padding) const;
  std::string desc() const;
  std::string site() const;
  std::string regex(const std::string &name) const;

  void sendImpl(DebuggerThriftBuffer &thrift);
  void recvImpl(DebuggerThriftBuffer &thrift);

  static void SendImpl(const BreakPointInfoPtrVec &bps,
                       DebuggerThriftBuffer &thrift);
  static void RecvImpl(BreakPointInfoPtrVec &bps,
                       DebuggerThriftBuffer &thrift);

  bool breakable(int stackDepth) const;
  void unsetBreakable(int stackDepth);
  void setBreakable(int stackDepth);
  void changeBreakPointDepth(int stackDepth);

  int16_t m_index; // client side index number

  int8_t m_state;
  bool m_valid;
  int8_t m_interrupt;

  // file::line1-line2
  std::string m_file;
  int32_t m_line1;
  int32_t m_line2;
  int32_t m_char1;
  int32_t m_char2;

  // class::func()
  DFunctionInfoPtrVec m_funcs;

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
  std::list<int> breakDepthStack;

  static bool Match(const char *haystack, int haystack_len,
                    const std::string &needle, bool regex, bool exact);
  static bool MatchClass(const char *fcls, const std::string &bcls,
                         bool regex, const char *func);

  void createIndex();
  std::string descBreakPointReached() const;
  std::string descExceptionThrown() const;

  void parseExceptionThrown(const std::string &exp);
  void parseBreakPointReached(const std::string &exp, const std::string &file);
  bool parseLines(const std::string &token);

  bool checkException(CVarRef e);
  bool checkUrl(std::string &url);
  bool checkLines(int line);
  bool checkStack(InterruptSite &site);
  bool checkClause();
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_BREAK_POINT_H_
