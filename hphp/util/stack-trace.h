/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_STACKTRACE_H_
#define incl_HPHP_STACKTRACE_H_

#include <dlfcn.h>
#include "hphp/util/base.h"

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
    explicit Frame(void *_bt) : bt(_bt), lineno(0), offset(0) {}

    void *bt;
    int lineno;
    int offset;

    std::string toString() const;
  };

public:
  static bool Enabled;

protected:
  StackTraceBase();

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static bool Translate(void *bt, Frame * f, Dl_info&, void*,
                        void *bfds=nullptr, unsigned bfd_size=0) ;

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
    explicit Frame(void *_bt) : StackTraceBase::Frame(_bt) {}
    std::string filename;
    std::string funcname;
    std::string toString() const;
  };

public:

  explicit StackTrace(bool trace = true) ;

  /**
   * Translate a frame pointer to file name and line number pair.
   */
  static FramePtr Translate(void *bt);

  /**
   * Translate the frame pointer of a PHP function using the perf map.
   */
  static void TranslateFromPerfMap(void* bt, Frame* f);

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
  explicit StackTrace(const std::string &hexEncoded);
  explicit StackTrace(const char *hexEncoded);

  /**
   * Generate an output of the written stack trace.
   */
  const std::string &toString(int skip = 0, int limit = -1) const;

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
  explicit StackTraceNoHeap(bool trace = true);

  /**
   * Log stacktrace into the given file.
   */
  void log(const char *errorType, const char * fname, const char *pid,
           const char *buildId, int debuggerCount) const;

  /**
   * Add extra information to log together with a crash stacktrace log.
   */
  static void AddExtraLogging(const char *name, const std::string &value);
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

#endif // incl_HPHP_STACKTRACE_H_
