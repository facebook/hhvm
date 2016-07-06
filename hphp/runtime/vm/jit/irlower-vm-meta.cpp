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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/preclass.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

void cgLdClsName(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const preclass = v.makeReg();
  v << load{src[Class::preClassOff()], preclass};
  emitLdLowPtr(v, preclass[PreClass::nameOffset()],
               dst, sizeof(LowStringPtr));
}

void cgLdClsMethod(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  int32_t const mSlotVal = inst->src(1)->rawVal();

  // We could have a Cls or a Cctx.  The Cctx has the low bit set, so
  // we need to subtract one in that case.
  auto const methOff = int32_t(mSlotVal * sizeof(LowPtr<Func>)) -
                       (inst->src(0)->isA(TCctx) ? 1 : 0);
  auto& v = vmain(env);
  emitLdLowPtr(v, cls[methOff], dst, sizeof(LowPtr<Func>));
}

void cgLdIfaceMethod(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdIfaceMethod>();
  auto const func = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const vtable_vec = v.makeReg();
  auto const vtable = v.makeReg();

  emitLdLowPtr(v, cls[Class::vtableVecOff()],
               vtable_vec, sizeof(LowPtr<Class::VtableVecSlot>));
  auto const vtableOff = extra->vtableIdx * sizeof(Class::VtableVecSlot) +
                         offsetof(Class::VtableVecSlot, vtable);
  emitLdLowPtr(v, vtable_vec[vtableOff], vtable,
               sizeof(Class::VtableVecSlot::vtable));
  emitLdLowPtr(v, vtable[extra->methodIdx * sizeof(LowPtr<Func>)],
               func, sizeof(LowPtr<Func>));
}

void cgLdObjInvoke(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  emitLdLowPtr(v, cls[Class::invokeOff()], dst, sizeof(LowPtr<Func>));

  auto const sf = v.makeReg();
  v << testq{dst, dst, sf};
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgLdFuncVecLen(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // A Cctx is a Cls with the bottom bit set; subtract one from the offset to
  // handle that case.
  auto const off = Class::funcVecLenOff() - (inst->src(0)->isA(TCctx) ? 1 : 0);
  v << loadzlq{cls[off], dst};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdFuncNumParams(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const tmp = v.makeReg();
  // See Func::finishedEmittingParams and Func::numParams.
  v << loadzlq{func[Func::paramCountsOff()], tmp};
  v << shrqi{1, tmp, dst, v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

}}}
