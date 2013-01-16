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

#ifndef __HPHP_EVAL_DEBUGGER_BREAK_POINT_H__
#define __HPHP_EVAL_DEBUGGER_BREAK_POINT_H__

#include <runtime/eval/debugger/debugger_thrift_buffer.h>
#include <runtime/base/frame_injection.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

enum InterruptType {
  SessionStarted,
  SessionEnded,
  RequestStarted,
  RequestEnded,
  PSPEnded,
  HardBreakPoint,
  BreakPointReached,
  ExceptionThrown,
};

class InterruptSite {
public:
  InterruptSite(CVarRef e = null_variant, const char *cls = NULL,
                const char *function = NULL, StringData *file = NULL,
                int line0 = 0, int char0 = 0, int line1 = 0, int char1 = 0)
    : m_exception(e), m_class(cls), m_function(function), m_file(file),
      m_line0(line0), m_char0(char0), m_line1(line1), m_char1(char1),
      m_jumping(false) { }

  virtual const char *getFile() const = 0;
  virtual const char *getClass() const = 0;
  virtual const char *getFunction() const = 0;
  virtual const char *getNamespace() const { return NULL; }
  int getFileLen() const;

  int32 getLine0() const { return m_line0;}
  int32 getChar0() const { return m_char0;}
  int32 getLine1() const { return m_line1;}
  int32 getChar1() const { return m_char1;}
  CVarRef getException() { return m_exception;}

  std::string &url() const { return m_url;}
  std::string desc() const;

  bool isJumping() const { return m_jumping;}
  void setJumping() { m_jumping = true;}

protected:
  Variant m_exception;

  // cached
  mutable const char *m_class;
  mutable const char *m_function;
  mutable String m_file;
  mutable std::string m_url;

  int32 m_line0;
  int32 m_char0;
  int32 m_line1;
  int32 m_char1;

  // jump instruction
  bool m_jumping;
};

class InterruptSiteFI : public InterruptSite {
public:
  InterruptSiteFI(FrameInjection *frame, CVarRef e = null_variant,
                  int char0 = 0, int line1 = 0, int char1 = 0)
      : InterruptSite(e) {
    m_frame = frame;
    assert(m_frame);
    m_line0 = m_frame->getLine();
    m_char0 = char0;
    m_line1 = line1;
    m_char1 = char1;
  }

  FrameInjection *getFrame() const { return m_frame;}

  virtual const char *getFile() const;
  virtual const char *getClass() const;
  virtual const char *getFunction() const;

private:
  FrameInjection *m_frame;
};

class InterruptSiteVM : public InterruptSite {
public:
  InterruptSiteVM(bool hardBreakPoint = false, CVarRef e = null_variant);
  virtual const char *getFile() const;
  virtual const char *getClass() const;
  virtual const char *getFunction() const;
  const VM::SourceLoc *getSourceLoc() const {
    return &m_sourceLoc;
  }
  const VM::OffsetRangeVec& getCurOffsetRange() const {
    return m_offsetRangeVec;
  }
  const VM::Unit* getUnit() const {
    return m_unit;
  }
  bool valid() const { return m_valid; }
  bool funcEntry() const { return m_funcEntry; }
private:
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

  int16 m_index; // client side index number

  int8 m_state;
  bool m_valid;
  int8 m_interrupt;

  // file::line1-line2
  std::string m_file;
  int32 m_line1;
  int32 m_line2;
  int32 m_char1;
  int32 m_char2;

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
  bool checkFrame(FrameInjection *frame);
  bool checkClause();
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_BREAK_POINT_H__
