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
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"

/*
 * Because of a circular dependence with ExecutionContext, these
 * translation-related helpers cannot live in translator.h.
 */
namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline ActRec* liveFrame() { return vmfp(); }
inline const Func* liveFunc() { return liveFrame()->func(); }
inline const Unit* liveUnit() { return liveFunc()->unit(); }
inline Class* liveClass() { return liveFunc()->cls(); }
inline ResumeMode liveResumeMode() { return resumeModeFromActRec(liveFrame()); }
inline bool liveHasThis() { return liveClass() && liveFrame()->hasThis(); }
inline SrcKey liveSK() {
  return { liveFunc(), vmpc(), liveResumeMode() };
}
inline jit::SBInvOffset liveSpOff() {
  auto const stackBase = Stack::anyFrameStackBase(liveFrame());
  return jit::SBInvOffset{safe_cast<int32_t>(stackBase - vmsp())};
}

///////////////////////////////////////////////////////////////////////////////

namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline int cellsToBytes(int nCells) {
  return safe_cast<int32_t>(nCells * ssize_t(sizeof(TypedValue)));
}

inline int localOffset(int locId) {
  return -cellsToBytes(locId + 1);
}

///////////////////////////////////////////////////////////////////////////////

}}

