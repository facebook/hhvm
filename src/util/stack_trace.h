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

#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__

#include <dlfcn.h>
#include "base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Taking a stacktrace at current execution location.
 */
class StackTraceBase {
public:
  DECLARE_BOOST_TYPES(Frame);
  class Frame {
  public:
    Frame(void *_bt) : bt(_bt), lineno(0), offset(0) {}

    void *bt;
    int lineno;
    int offset;

    std::string toString() const;
  };

public:
  static bool Enabled;
  static std::string ReportDirectory;
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

protected:
  StackTraceBase();

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static bool Translate(void *bt, Frame * f, Dl_info&, void*,
                        void *bfds=NULL, unsigned bfd_size=0) ;

  /**
   * Run addr2line to translate a function pointer into function name and line
   * number.
   */

  static bool Addr2line(const char *filename, const char *address,
                        Frame* frame, void *addr2line_data,
                        void* bfds, unsigned bfds_size);


  static const unsigned int MAXFRAME = 175;

};

class StackTrace : public StackTraceBase {
public:
  DECLARE_BOOST_TYPES(Frame);
  class Frame : public StackTraceBase::Frame {
  public:
    Frame(void *_bt) : StackTraceBase::Frame(_bt) {}
    std::string filename;
    std::string funcname;
    std::string toString() const;
  };

public:

  StackTrace(bool trace = true) ;

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static FramePtr Translate(void *bt);

  /**
   * Demangle a function name.
   */
  static std::string Demangle(const char *mangled);

  /**
   * Copy constructor. This will copy over stack trace without saving current
   * stack's.
   */
  StackTrace(const StackTrace &bt);

  /**
   * Constructing from hexEncode() results.
   */
  StackTrace(const std::string &hexEncoded);

  StackTrace(const char *hexEncoded);

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

private:
  std::vector<void*> m_bt_pointers;
  mutable std::string m_bt;

  /**
   * Record bt pointers.
   */
  void create();

  /**
   * Init by hex string
   */
  void initFromHex(const char *);
};

// Do not use heap here, so this will still work if called during a heap error
class StackTraceNoHeap : public StackTraceBase {
public:
  /**
   * Constructor, and this will save current stack trace if trace is true.
   * It can be false for an empty stacktrace.
   */
  StackTraceNoHeap(bool trace = true) ;

  /**
   * Log stacktrace into a file under /tmp. If "out" is not null,
   * also store translated stack trace into the variable.
   * Returns the name of the generated file.
   */
  void log(const char *errorType, const char * fname, const char *pid) const;

  /**
   * Add extra information to log together with a crash stacktrace log.
   */
  static void AddExtraLogging(const char *name, const char *value);
  static void ClearAllExtraLogging();

  class ExtraLoggingClearer {
  public:
    ExtraLoggingClearer() {}
    ~ExtraLoggingClearer() { StackTraceNoHeap::ClearAllExtraLogging();}
  };

private:
  /**
   * Generate an output of the written stack trace.
   */
  void printStackTrace(int fd) const;

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static bool Translate(int fd, void *bt, int frame_num, void *,
                        unsigned int bfds_size) ;

  /**
   * Demangle a function name.
   */
  static void Demangle(int fd, const char *mangled);

  void *m_btpointers[MAXFRAME];
  unsigned int m_btpointers_cnt;

  /**
   * Record bt pointers.
   */
  void create();
};
///////////////////////////////////////////////////////////////////////////////
}

#endif // __STACKTRACE_H__
