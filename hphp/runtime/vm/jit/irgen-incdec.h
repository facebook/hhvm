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

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

////////////////////////////////////////////////////////////////////////////////

/*
 * Performs an IncDec operation on an SSATmp.  Returns nullptr if we don't
 * support irgen for the given source's type.
 */
inline SSATmp* incDec(IRGS& env, IncDecOp op, SSATmp* src) {
  // Old behavior handles non int/double types.  New behavior warns/fatals for
  // non int/double types.
  if (RuntimeOption::EvalWarnOnIncDecInvalidType == 0) {
    if (src->isA(TNull)) {
      if (!isInc(op)) return src;
      handleConvNoticeLevel(
        env,
        ConvNoticeData {
          flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForIncDec),
          s_ConvNoticeReasonIncDec.get()
        },
        "null",
        "int");
      return cns(env, 1);
    }

    if (src->type().subtypeOfAny(TBool, TArrLike, TObj, TRes)) {
      return src;
    }
  }
  if (!src->type().subtypeOfAny(TInt, TDbl)) {
    return nullptr;
  }

  Opcode opc;
  if (src->isA(TDbl)) {
    opc = isInc(op) ? AddDbl : SubDbl;
  } else if (!isIncDecO(op)) {
    opc = isInc(op) ? AddInt : SubInt;
  } else {
    opc = isInc(op) ? AddIntO : SubIntO;
  }

  auto const one = src->isA(TInt) ? cns(env, 1) : cns(env, 1.0);
  auto const result = opc == AddIntO || opc == SubIntO
    ? gen(env, opc, makeExitSlow(env), src, one)
    : gen(env, opc, src, one);

  return result;
}

////////////////////////////////////////////////////////////////////////////////

}}}
