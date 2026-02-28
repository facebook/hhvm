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

BTFrame getPrevActRec(
  BTFrame frm,
  req::fast_set<c_WaitableWaitHandle*>& visitedWHs
);

/*
 * Take a `frm` cursor and perform miscellaneous preprocessing on it.
 *
 * Preprocessing includes:
 *  - Setting up virtual inline stack frames.
 *  - Setting up virtual FCallBuiltin inline frames.
 *  - Coalescing kInvalidOffset `pc` values to `pcOff()`.
 */
BTFrame initBTFrameAt(jit::CTCA ip, BTFrame frm);

}

template<class L>
void walkStackFrom(
    L func, BTFrame initFrm, jit::CTCA ip, bool skipTop,
    req::fast_set<c_WaitableWaitHandle*>& visitedWHs) {
  auto frm = backtrace_detail::initBTFrameAt(ip, initFrm);

  if (skipTop) frm = backtrace_detail::getPrevActRec(frm, visitedWHs);

  for (; frm; frm = backtrace_detail::getPrevActRec(frm, visitedWHs)) {
    if (ArrayData::call_helper(func, frm)) return;
  }
}

template<class L>
void walkStack(L func, c_WaitableWaitHandle* wh, bool skipTop) {
  using namespace backtrace_detail;

  VMRegAnchor _;

  req::fast_set<c_WaitableWaitHandle*> visitedWHs;

  auto frm = wh != nullptr
    ? getARFromWH(wh, visitedWHs)
    : BTFrame::regular(vmfp(), kInvalidOffset);

  // If there are no VM frames, we're done.
  if (!frm || !rds::header()) return;

  walkStackFrom(func, frm, vmJitReturnAddr(), skipTop, visitedWHs);
}

template<class L>
void walkStack(L func, bool skipTop) {
  walkStack(func, nullptr, skipTop);
}

namespace backtrace_detail {

template<typename F, typename Pred>
from_ret_t<F> fromLeafImpl(F f, Pred pred,
                           from_ret_t<F> def,
                           bool skipTop) {
  auto ret = def;
  walkStack([&] (const BTFrame& frm) {
    if (!pred(frm)) return false;
    ret = f(frm);
    return true;
  }, nullptr, skipTop);
  return ret;
}

inline bool true_pred(const BTFrame&) { return true; }

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
