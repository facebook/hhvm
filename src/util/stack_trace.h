/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__

#include <string>
#include <vector>
#include "base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Taking a stacktrace at current execution location.
 */
class StackTrace {
public:
  DECLARE_BOOST_TYPES(Frame);
  class Frame {
  public:
    Frame(void *_bt) : bt(_bt), lineno(0), offset(0) {}

    void *bt;
    std::string filename;
    std::string funcname;
    int lineno;
    int offset;

    std::string toString() const;
  };

public:
  static std::string ReportEmail;

  /**
   * Attaches the handler to a bunch of normally fatal signals (SEGV, BUS,
   * ABRT, etc).a
   */
  static void InstallReportOnErrors();

  /**
   * Asking for a backtrace-on-signal will log the trace to stderr and
   * then terminate the program if that signal is caught. Use this to attach to
   * a specific signal.
   */
  static void InstallReportOnSignal(int signal);

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static FramePtr Translate(void *bt);

  /**
   * Run addr2line to translate a function pointer into function name and line
   * number.
   */
  static bool Addr2line(const char *filename, const char *address,
                        FramePtr &frame);

  /**
   * Demangle a function name.
   */
  static std::string Demangle(const char *mangled);

public:
  /**
   * Constructor, and this will save current stack trace.
   */
  StackTrace();

  /**
   * Copy constructor. This will copy over stack trace without saving current
   * stack's.
   */
  StackTrace(const StackTrace &bt);

  /**
   * Constructing from hexEncode() results.
   */
  StackTrace(const std::string &hexEncoded);

  /**
   * Generate an output of the written stack trace.
   */
  const std::string &toString() const;

  /**
   * Get frames in raw pointers or translated frames.
   */
  void get(std::vector<void*> &bt) const { bt = m_bt_pointers;}
  void get(FramePtrVec &frames) const;
  std::string hexEncode(int minLevel = 0, int maxLevel = 999) const;

  /**
   * Log stacktrace into a file under /tmp. If "out" is not null,
   * also store translated stack trace into the variable.
   * Returns the name of the generated file.
   */
  std::string log(const char *errorType, std::string *out = NULL) const;

private:
  std::vector<void*> m_bt_pointers;
  mutable std::string m_bt;

  /**
   * Record bt pointers.
   */
  void create();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __STACKTRACE_H__
