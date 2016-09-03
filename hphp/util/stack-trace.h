/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <folly/Range.h>

#include "hphp/util/compatibility.h"
#include "hphp/util/portability.h"

#ifndef _MSC_VER
#include <dlfcn.h>
#endif

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

/*
 * An unsymbolized stack frame.
 */
struct StackFrame {
  explicit StackFrame(void* addr) : addr{addr} {}

  void* addr{nullptr};
  int lineno{0};
  int offset{0};
};

/*
 * A StackFrame that has been symbolized with a filename and function name.
 */
struct StackFrameExtra : StackFrame {
  explicit StackFrameExtra(void* addr) : StackFrame(addr) {}

  std::string toString() const;

  std::string filename;
  std::string funcname;
};

////////////////////////////////////////////////////////////////////////////////

/*
 * Taking a stacktrace at current execution location.  Do not use directly, use
 * StackTrace or StackTraceNoHeap instead.
 */
struct StackTraceBase {
  static constexpr unsigned kMaxFrame = 175;

  static bool Enabled;
  static const char* const* FunctionBlacklist;
  static unsigned FunctionBlacklistCount;

 protected:
  StackTraceBase();
};

////////////////////////////////////////////////////////////////////////////////

struct StackTrace : StackTraceBase {
  struct PerfMap {
    void rebuild();
    bool translate(StackFrameExtra*) const;

    struct Range {
      uintptr_t base;
      uintptr_t past;
      struct Cmp {
        bool operator() (const Range& a, const Range& b) const {
          return a.past <= b.base;
        }
      };
    };
    bool m_built = false;
    std::map<Range,std::string,Range::Cmp> m_map;
  };

  //////////////////////////////////////////////////////////////////////////////

  /*
   * Construct the curent stacktrace if `trace' is true, else an empty
   * stacktrace.
   */
  explicit StackTrace(bool trace = true);

  /*
   * Construct a stacktrace from a list of instruction pointers.
   */
  StackTrace(void* const* ips, size_t count);

  /*
   * Constructing from hexEncode() results.
   */
  explicit StackTrace(folly::StringPiece);

  /*
   * Translate a frame pointer to file name and line number pair.
   */
  static std::shared_ptr<StackFrameExtra> Translate(void* addr,
                                                    PerfMap* pm = nullptr);

  /*
   * Encode the stacktrace addresses in a string.
   *
   * Skip minLevel frames, and end after the maxLevel'th at the most.
   */
  std::string hexEncode(int minLevel = 0, int maxLevel = 999) const;

  /*
   * Generate a string representation of the stack trace.
   */
  const std::string& toString(int skip = 0, int limit = -1) const;

  /*
   * Get translated frames.
   */
  void get(std::vector<std::shared_ptr<StackFrameExtra>>&) const;

  //////////////////////////////////////////////////////////////////////////////

private:
  std::vector<void*> m_frames;

  // Cached backtrace string.
  mutable std::string m_trace;
  mutable bool m_trace_usable{false};
};

////////////////////////////////////////////////////////////////////////////////

/*
 * Computing a stack trace without using the heap.  Ideally this should be safe
 * to use within a signal handler.
 */
struct StackTraceNoHeap : StackTraceBase {
  /*
   * Construct the curent stacktrace if `trace' is true, else an empty
   * stacktrace.
   */
  explicit StackTraceNoHeap(bool trace = true);

  /*
   * Log stacktrace into the given file.
   */
  void log(const char* errorType, int fd, const char* buildId,
           int debuggerCount) const;

  /*
   * Add extra information to log together with a crash stacktrace log.
   */
  static void AddExtraLogging(const char* name, const std::string& value);
  static void ClearAllExtraLogging();

  struct ExtraLoggingClearer {
    ExtraLoggingClearer() {}
    ~ExtraLoggingClearer() {
      StackTraceNoHeap::ClearAllExtraLogging();
    }
  };

private:
  void printStackTrace(int fd) const;

private:
  void* m_frames[kMaxFrame];
  unsigned m_frame_count;
};

////////////////////////////////////////////////////////////////////////////////
}

#endif
