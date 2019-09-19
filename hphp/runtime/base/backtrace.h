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

#ifndef incl_HPHP_BACKTRACE_H_
#define incl_HPHP_BACKTRACE_H_

#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/types.h"

#include "hphp/util/low-ptr.h"

#include <folly/Optional.h>
#include <folly/small_vector.h>

#include <cstdint>
#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Array;
struct Class;
struct Func;
struct Resource;
struct StructuredLogEntry;
struct VMParserFrame;
struct c_ResumableWaitHandle;
struct c_WaitableWaitHandle;

///////////////////////////////////////////////////////////////////////////////

struct CompactTrace : SweepableResourceData {
  Array extract() const;
  void insert(const ActRec* fp, int32_t prevPc) { m_key.insert(fp, prevPc); }

  DECLARE_RESOURCE_ALLOCATION(CompactTrace)

  struct Frame final {
    Frame() = default;
    Frame(const Func* f, int32_t ppc, bool ht)
      : func(f)
      , prevPc(ppc)
      , hasThis(ht)
    {}

    LowPtr<const Func> func;
    union {
      struct {
        int32_t prevPc : 31;
        bool hasThis   : 1;
      };
      uint32_t prevPcAndHasThis;
    };
  };

  struct Key final {
    Array extract() const;
    void insert(const ActRec* fp, int32_t prevPc);

    TYPE_SCAN_IGNORE_ALL;

    uint64_t m_hash{0x9e3779b9};
    folly::small_vector<Frame, 16> m_frames;
  };

  const folly::small_vector<Frame, 16>& frames() const {
    return m_key.m_frames;
  }

private:
  Key m_key;
};

struct BacktraceArgs {

  /**
   * creates the backtrace using the settings from BacktraceArgs
   * example: createBacktrace(BacktraceArgs().skipTop().setLimit(numOfFrames));
   */
  friend Array createBacktrace(const BacktraceArgs& backtraceArgs);

  /**
   * Skip the top frame of the stack. Used to jump over internal frames.
   */
  BacktraceArgs& skipTop(bool skipTop = true) {
    m_skipTop = skipTop;
    return *this;
  }

  /**
   * Skip the top inlined frames of the stack. Used to properly attribute cost
   * measured by event hooks. If both skipTop and skipInlined are set then
   * the first frame and all immediately following inlined frames are skipped.
   */
  BacktraceArgs& skipInlined(bool skipInlined = true) {
    m_skipInlined = skipInlined;
    return *this;
  }

  /**
   * Include the current frame on top of the stack. If both skipTop and withSelf
   * are set then first frame is skipped and the second is added.
   */
  BacktraceArgs& withSelf(bool withSelf = true) {
    m_withSelf = withSelf;
    return *this;
  }

  /**
   * Return the object associated with each frame.
   */
  BacktraceArgs& withThis(bool withThis = true) {
    m_withThis = withThis;
    return *this;
  }

  /**
   * Return the metadata associated with each frame having 86metadata local.
   */
  BacktraceArgs& withMetadata(bool withMetadata = true) {
    m_withMetadata = withMetadata;
    return *this;
  }

  /**
   * Do not return function arguments for frames on the stack.
   */
  BacktraceArgs& ignoreArgs(bool ignoreArgs = true) {
    m_withArgNames = m_withArgNames && !ignoreArgs;
    m_withArgValues = m_withArgValues && !ignoreArgs;
    return *this;
  }

  /**
   * Print out the first x frames up to some limit x.
   */
  BacktraceArgs& setLimit(int limit) {
    m_limit = limit;
    return *this;
  }

  /**
   * The parser frame when a file has a compile time error and a backtrace
   * is needed.
   */
  BacktraceArgs& setParserFrame(VMParserFrame* parserFrame) {
    m_parserFrame = parserFrame;
    return *this;
  }

  /**
   * Include the pseudo main in the backtrace.
   */
  BacktraceArgs& withPseudoMain(bool withPseudoMain = true) {
    m_withPseudoMain = withPseudoMain;
    return *this;
  }

  /**
   * Include argument values in backtrace.
   */
  BacktraceArgs& withArgValues(bool withValues = true) {
    m_withArgValues = withValues;
    return *this;
  }

  /**
   * Include argument names in backtrace.
   */
  BacktraceArgs& withArgNames(bool withNames = true) {
    m_withArgNames = withNames;
    return *this;
  }

  /**
   * Backtrace from wait handle, instead of current frame.
   */
  BacktraceArgs& fromWaitHandle(c_WaitableWaitHandle* handle) {
    m_fromWaitHandle = handle;
    return *this;
  }

  bool isCompact() const {
    return
      RuntimeOption::EvalEnableCompactBacktrace &&
      !m_skipTop &&
      !m_skipInlined &&
      !m_withSelf &&
      !m_withThis &&
      !m_withMetadata &&
      !m_withPseudoMain &&
      (!RuntimeOption::EnableArgsInBacktraces || !m_withArgValues) &&
      !m_withArgNames &&
      !m_limit &&
      !m_parserFrame &&
      !m_fromWaitHandle;
  }

private:
  bool m_skipTop{false};
  bool m_skipInlined{false};
  bool m_withSelf{false};
  bool m_withThis{false};
  bool m_withMetadata{false};
  bool m_withPseudoMain{false};
  bool m_withArgValues{true};
  bool m_withArgNames{false};
  int m_limit{0};
  VMParserFrame* m_parserFrame{nullptr};
  c_WaitableWaitHandle* m_fromWaitHandle{nullptr};
};

///////////////////////////////////////////////////////////////////////////////

using IFrameID = int32_t;

struct IFrame {
  const Func* func; // callee (m_func)
  int32_t callOff;  // caller offset (m_callOff)
  IFrameID parent;  // parent frame (m_sfp)
};

struct IStack {
  IFrameID frame; // leaf frame in this stack
  uint32_t nframes;
  uint32_t callOff;

  template<class SerDe> void serde(SerDe& sd) {
    sd(frame)
      (nframes)
      (callOff)
      ;
  }
};

///////////////////////////////////////////////////////////////////////////////

Array createBacktrace(const BacktraceArgs& backtraceArgs);
void addBacktraceToStructLog(const Array& bt, StructuredLogEntry& cols);
int64_t createBacktraceHash(bool consider_metadata);
req::ptr<CompactTrace> createCompactBacktrace();

/*
 * Walk the logical VM stack, including inlined frames.
 *
 * `func` should have the signature:
 *
 *    bool func(ActRec* fp, Offset prevPC);
 *
 *  If `func` returns true, the walk terminates; otherwise, it continues.
 *
 *  Note that the `fp` argument to `func` is not valid outside `func`; it may
 *  refer to temporary stack-allocated memory.
 */
template<typename L>
void walkStack(L func, bool skipTop = false);

namespace backtrace_detail {

template<typename F>
using from_ret_t = std::result_of_t<F(const ActRec*, Offset)>;

}

/*
 * Call `f` with the frame pointer and PC of the current Hack function (i.e.,
 * the leafmost function which called into native code).
 *
 * If there is no such function, returns `def`; otherwise, forwards `f`'s
 * return.
 */
template<typename F>
backtrace_detail::from_ret_t<F> fromLeaf(
  F f,
  backtrace_detail::from_ret_t<F> def = backtrace_detail::from_ret_t<F>{}
);

/*
 * Like fromLeaf(), but for the leaf function's caller.
 */
template<typename F>
backtrace_detail::from_ret_t<F> fromCaller(
  F f,
  backtrace_detail::from_ret_t<F> def = backtrace_detail::from_ret_t<F>{}
);

/*
 * Like fromLeaf() and fromCaller(), but for the first function whose frame
 * satisfies `pred(fp)`.
 */
template<typename F, typename Pred>
backtrace_detail::from_ret_t<F> fromLeaf(
  F f, Pred pred,
  backtrace_detail::from_ret_t<F> def = backtrace_detail::from_ret_t<F>{}
);
template<typename F, typename Pred>
backtrace_detail::from_ret_t<F> fromCaller(
  F f, Pred pred,
  backtrace_detail::from_ret_t<F> def = backtrace_detail::from_ret_t<F>{}
);

///////////////////////////////////////////////////////////////////////////////

}

#define incl_HPHP_BACKTRACE_INL_H_
#include "hphp/runtime/base/backtrace-inl.h"
#undef incl_HPHP_BACKTRACE_INL_H_

#endif // incl_HPHP_BACKTRACE_H_
