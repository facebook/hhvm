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

#ifndef incl_HPHP_BACKTRACE_INL_H_
#error "backtrace-inl.h should only be included by backtrace.h"
#endif

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"

#include <folly/small_vector.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct c_WaitableWaitHandle;

///////////////////////////////////////////////////////////////////////////////

namespace backtrace_detail {

struct BTContext {
  BTContext();

  BTContext(const BTContext&) = delete;
  BTContext(BTContext&&) = delete;
  BTContext& operator=(const BTContext&) = delete;
  BTContext& operator=(BTContext&&) = delete;

  const ActRec* clone(const BTContext& src, const ActRec* fp);

  bool hasInlFrames{false};

  // fakeAR is used to generate pseudo-frames representing inlined functions
  // whose frames have been elided. The array operates like a ring buffer as
  // createBacktrace needs to inspect the current and previous frame pointer,
  // thus we introduce an m_sfp cycle between these frames.
  ActRec fakeAR[2];
  IStack inlineStack;

  // stashedAR stores a pointer to the frame that should be returned after the
  // inlined stack has been traversed and stashedPC stores the corresponding PC.
  ActRec* stashedAR{nullptr};
  Offset stashedPC{kInvalidOffset};
};

ActRec* getPrevActRec(
  BTContext& ctx,
  const ActRec* fp,
  Offset* prevPc,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
);

ActRec* initBTContextAt(BTContext& ctx, jit::CTCA ip,
                        ActRec* fp, Offset* prevPc);

}

template<class L>
void walkStack(L func, bool skipTop) {
  using namespace backtrace_detail;

  VMRegAnchor _;
  folly::small_vector<c_WaitableWaitHandle*, 64> visitedWHs;
  auto fp = vmfp();

  // If there are no VM frames, we're done.
  if (!fp || !rds::header()) return;

  BTContext ctx;

  // Handle the subsequent VM frames.
  auto prevPc = pcOff();
  auto const addr = vmJitReturnAddr();
  if (auto const inl = initBTContextAt(ctx, addr, fp, &prevPc)) {
    fp = inl;
  }

  if (skipTop) fp = getPrevActRec(ctx, fp, &prevPc, visitedWHs);

  for (; fp != nullptr; fp = getPrevActRec(ctx, fp, &prevPc, visitedWHs)) {
    if (ArrayData::call_helper(func, fp, prevPc)) return;
  }
}

namespace backtrace_detail {

template<typename F, typename Pred>
from_ret_t<F> fromLeafImpl(F f, Pred pred,
                           from_ret_t<F> def,
                           bool skipTop) {
  auto ret = def;
  walkStack([&] (const ActRec* fp, Offset off) {
    if (!pred(fp)) return false;
    ret = f(fp, off);
    return true;
  }, skipTop);
  return ret;
}

inline bool true_pred(const ActRec* fp) { return true; }

}

template<typename F>
backtrace_detail::from_ret_t<F> fromLeaf(
  F f, backtrace_detail::from_ret_t<F> def
) {
  return backtrace_detail::fromLeafImpl(
    f, backtrace_detail::true_pred, def, false
  );
}

template<typename F>
backtrace_detail::from_ret_t<F> fromCaller(
  F f, backtrace_detail::from_ret_t<F> def
) {
  return backtrace_detail::fromLeafImpl(
    f, backtrace_detail::true_pred, def, true
  );
}

template<typename F, typename Pred>
backtrace_detail::from_ret_t<F> fromLeaf(
  F f, Pred pred, backtrace_detail::from_ret_t<F> def
) {
  return backtrace_detail::fromLeafImpl(f, pred, def, false);
}

template<typename F, typename Pred>
backtrace_detail::from_ret_t<F> fromCaller(
  F f, Pred pred, backtrace_detail::from_ret_t<F> def
) {
  return backtrace_detail::fromLeafImpl(f, pred, def, true);
}

///////////////////////////////////////////////////////////////////////////////

}
