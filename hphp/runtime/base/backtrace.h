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

#pragma once

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/types.h"

#include "hphp/util/low-ptr.h"

#include <folly/small_vector.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Array;
struct BTFrame;
struct Class;
struct Func;
struct StructuredLogEntry;
struct VMParserFrame;
struct c_ResumableWaitHandle;
struct c_WaitableWaitHandle;

///////////////////////////////////////////////////////////////////////////////

struct CompactFrame final {
  CompactFrame(const Func* f = nullptr, int32_t ppc = 0, bool ht = false)
    : func(f)
    , prevPc(ppc) {}

  uint64_t hash() const {
    auto const f = reinterpret_cast<uintptr_t>(func.get());
    return hash_int64_pair(prevPc, f >> 4);
  }

  const LowPtr<const Func> func;
  int32_t prevPc;
};

struct CompactTraceData {
  Array extract() const;
  void insert(const BTFrame& frm);
  const auto& frames() const { return m_frames; }
  auto size() const { return m_frames.size(); }

  using Ptr = std::shared_ptr<CompactTraceData>;
  static Ptr Create() {
    return std::make_shared<CompactTraceData>();
  }

 private:
  folly::small_vector<CompactFrame, 16> m_frames;
};

struct CompactTrace : SweepableResourceData {
  Array extract() const;

  CompactTraceData* get() const {
    if (!m_backtrace) m_backtrace = CompactTraceData::Create();
    return m_backtrace.get();
  }

  void insert(const BTFrame& frm) {
    get()->insert(frm);
  }

  DECLARE_RESOURCE_ALLOCATION(CompactTrace)
  TYPE_SCAN_IGNORE_ALL;

  uint32_t size() const {
    if (!m_backtrace) return 0;
    return m_backtrace->size();
  }

  const auto& frames() const {
    return get()->frames();
  }

 private:
  mutable CompactTraceData::Ptr m_backtrace;
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

  static VMParserFrame* setGlobalParserFrame(VMParserFrame* parserFrame) {
    auto oldParserFrame = g_context->m_parserFrame;
    g_context->m_parserFrame = parserFrame;
    return oldParserFrame;
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
      !m_skipInlined &&
      !m_withSelf &&
      (!RuntimeOption::EnableArgsInBacktraces || !m_withThis) &&
      !m_withMetadata &&
      (!RuntimeOption::EnableArgsInBacktraces || !m_withArgValues) &&
      !m_withArgNames &&
      !m_limit &&
      !m_parserFrame &&
      !g_context->m_parserFrame &&
      !m_fromWaitHandle;
  }

private:
  bool m_skipTop{false};
  bool m_skipInlined{false};
  bool m_withSelf{false};
  bool m_withThis{false};
  bool m_withMetadata{false};
  bool m_withArgValues{true};
  bool m_withArgNames{false};
  int m_limit{0};
  VMParserFrame* m_parserFrame{nullptr};
  c_WaitableWaitHandle* m_fromWaitHandle{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
/*
 * Inline stacktrace fragments.
 *
 * For your enjoyment, please peruse the following ASCII art diagram:
 *
 *    ActRec:  ________________
 *            |                |
 *            |  func: top     |
 *            |  callOffset()  |
 *            |________________|
 *                     |
 *                     | <---------+
 *    ActRec:  ________v_______    |
 *            |                |   | (relative to top->base())
 * vmfp() --> |  func: caller  |   |
 *            |  callOffset() -+---+
 *            |________________|
 *                     |
 *                     | <---------+
 *    IFrame:  ________v_______    |
 *            |                |   | (relative to caller->base())
 *            |  func: inl1    |   |
 *            |  callOff ------+---+
 *            |________________|
 *                     |
 *                     | <---------+
 *    IFrame:  ________v_______    |
 *            |                |   | (relative to inl1->base())
 *            |  func: inl2    |   |
 *            |  callOff ------+---+
 *            |________________|
 *                     |
 *                     | <---------------------------------+
 *                     v                                   |
 *            some native function                         |
 *                                                         |
 *    IStack corresponding to the TCA of the native call:  | (relative to
 *             ____________________                            |  inl2->base())
 *            |                    |                           |
 *            |  frame: inl2       |                           |
 *            |  pubFrame: caller  |                           |
 *            |  callOff ----------+---------------------------+
 *            |____________________|
 *
 *
 * In the presence of inline frame elision, there are no ActRecs corresponding
 * to the inlined callee frames.  Instead, we maintain IFrame structs which we
 * use to construct the backtrace.
 *
 * The value of vmpc() is undocumented, because sometimes it appears to point
 * to the call from `caller` to `inl1`, and sometimes it seems to point to the
 * call from `inl2` to the native function.
 */

using IFrameID = int32_t;

// Represents the frame defined by DefFP or EnterFrame.
constexpr IFrameID kRootIFrameID = std::numeric_limits<IFrameID>::max();

struct IFrame {
  const Func* func;       // callee (m_func)
  Offset callOff;         // caller offset (callOffset())
  int32_t sbToRootSbOff;  // offset between stack bases of root and this frame
  IFrameID parent;        // parent frame (m_sfp)
};

struct IStack {
  IFrameID frame;     // leaf frame in this stack
  IFrameID pubFrame;  // the last published frame to which vmfp() points to
  Offset callOff;

  template<class SerDe> void serde(SerDe& sd) {
    sd(frame)
      (callOff)
      ;
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Everything we need to represent a frame in the backtrace.
 */
struct BTFrame {
  // No frame. End of backtrace.
  static BTFrame none() {
    return BTFrame{};
  }

  // Regular frame pointed to by `fp`. This includes resumables.
  // FIXME: fp may be nullptr, in which case this is an equivalent of none()
  static BTFrame regular(ActRec* fp, Offset bcOff) {
    return BTFrame{fp, bcOff, kInvalidAfwhTailFrameIdx};
  }

  // Inlined frame that does not have a materialized ActRec. IFrame backed by
  // `frameID` contains the metadata necessary to reconstruct the frame.
  // The nearest published frame `fp` is represented by `pubFrameId`.
  static BTFrame iframe(ActRec* fp, Offset bcOff,
                        IFrameID frameId, IFrameID pubFrameId) {
    assertx(fp != nullptr);
    assertx(frameId != kRootIFrameID && frameId != pubFrameId);
    return BTFrame{fp, bcOff, frameId, pubFrameId};
  }

  static BTFrame afwhTailFrame(ActRec* fp, Offset bcOff,
                               int8_t afwhTailFrameIdx) {
    assertx(fp != nullptr);
    assertx(afwhTailFrameIdx >= 0);
    return BTFrame{fp, bcOff, afwhTailFrameIdx};
  }

  BTFrame withFP(ActRec* fp) const {
    return BTFrame{fp, m_bcOff, m_frameId, m_pubFrameId};
  }

  operator bool() const { return m_fp != nullptr; }
  Offset bcOff() const { return m_bcOff; }
  const Func* func() const;
  bool isInlined() const;
  bool isRegularResumed() const;
  bool localsAvailable() const;
  TypedValue* local(int n) const;
  ObjectData* getThis() const;

  // Internal helpers for getPrevActRec(). Do not use for accessing frame info,
  // instead use the helpers above.
  ActRec* fpInternal() const { return m_fp; }
  IFrameID frameIdInternal() const { return m_frameId; }
  IFrameID pubFrameIdInternal() const {
    assertx(m_frameId != kRootIFrameID);
    return m_pubFrameId;
  }
  int8_t afwhTailFrameIdxInternal() const {
    assertx(m_frameId == kRootIFrameID);
    return m_afwhTailFrameIdx;
  }

  static constexpr int8_t kInvalidAfwhTailFrameIdx = -1;

 private:
  BTFrame() = default;
  BTFrame(ActRec* fp, Offset bcOff, IFrameID frameId, IFrameID pubFrameId)
    : m_fp(fp), m_bcOff(bcOff), m_frameId(frameId), m_pubFrameId(pubFrameId) {}
  BTFrame(ActRec* fp, Offset bcOff, int8_t afwhTailFrameIdx)
    : m_fp(fp), m_bcOff(bcOff), m_frameId(kRootIFrameID)
    , m_afwhTailFrameIdx(afwhTailFrameIdx) {}

  ActRec* m_fp{nullptr};
  Offset m_bcOff{kInvalidOffset};
  IFrameID m_frameId{kRootIFrameID};
  union {
    // Used if m_frameId != kRootIFrameID
    IFrameID m_pubFrameId;
    // Used if m_frameId == kRootIFrameID, this is afwh tail frame if >= 0
    int8_t m_afwhTailFrameIdx;
  };
};

Array createBacktrace(const BacktraceArgs& backtraceArgs);
Array createCrashBacktrace(BTFrame frame, jit::CTCA addr);
void addBacktraceToStructLog(const Array& bt, StructuredLogEntry& cols);
int64_t createBacktraceHash(bool consider_metadata);
void fillCompactBacktrace(CompactTraceData* trace, bool skipTop);
req::ptr<CompactTrace> createCompactBacktrace(bool skipTop = false);
std::pair<const Func*, Offset> getCurrentFuncAndOffset();

/*
 * Walk up the dependency chain for `wh` and return the first frame found.
 */
BTFrame getARFromWH(
  c_WaitableWaitHandle* currentWaitHandle,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
);

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

template<class L>
void walkStackFrom(
    L func, BTFrame initFrm, jit::CTCA ip, bool skipTop,
    folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs);

namespace backtrace_detail {

template<typename F>
using from_ret_t = std::result_of_t<F(const BTFrame&)>;

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
 * Like the above functions, but for the first function whose frame satisfies
 * `pred(fp)`.
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
