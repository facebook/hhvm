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
  InterruptSite(FrameInjection *frame, CVarRef e = null_variant,
                int char0 = 0, int line1 = 0, int char1 = 0)
      : m_frame(frame), m_exception(e), m_function(NULL), m_file_strlen(-1),
        m_jumping(false), m_char0(char0), m_line1(line1), m_char1(char1) {
    ASSERT(m_frame);
  }

  FrameInjection *getFrame() const { return m_frame;}
  const char *getFile() const;
  int32 getLine0() const { return m_frame->getLine();}
  int32 getChar0() const { return m_char0;}
  int32 getLine1() const { return m_line1;}
  int32 getChar1() const { return m_char1;}
  CVarRef getException() { return m_exception;}
  const char *getNamespace() const { return NULL;}
  const char *getClass() const { return m_frame->getClassName();}
  const char *getFunction() const;
  int getFileLen() const;

  std::string &url() const { return m_url;}
  std::string desc() const;

  bool isJumping() const { return m_jumping;}
  void setJumping() { m_jumping = true;}

private:
  FrameInjection *m_frame;
  Variant m_exception;

  // cached
  mutable String m_file;
  mutable const char *m_function;
  mutable int m_file_strlen;
  mutable std::string m_url;

  // jump instruction
  bool m_jumping;

  // additional source location only available from hphpi
  int32 m_char0;
  int32 m_line1;
  int32 m_char1;
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
