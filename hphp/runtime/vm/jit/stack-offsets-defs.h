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
#ifndef incl_HPHP_JIT_STACK_OFFSETS_DEFS_H_
#define incl_HPHP_JIT_STACK_OFFSETS_DEFS_H_

#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

/*
 * Convert stack offsets that are relative to the current HHBC instruction
 * (positive is higher on the stack) to offsets that are relative to the
 * currently defined StkPtr.
 */
inline IRSPOffset toIRSPOffset(
  BCSPOffset offsetFromInstr,
  FPInvOffset curSPTop,
  FPInvOffset spOffset
) {
  auto const invSPOff = curSPTop - offsetFromInstr.offset;
  auto const ret = -static_cast<int32_t>(invSPOff - spOffset);
  return IRSPOffset{ret};
}

inline BCSPOffset toBCSPOffset(FPInvOffset offsetFromFP, FPInvOffset curSPTop) {
  return BCSPOffset{curSPTop - offsetFromFP};
}

} } // HPHP::jit

#endif
