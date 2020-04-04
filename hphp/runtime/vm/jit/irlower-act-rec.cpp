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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"
#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgDefFuncEntryFP(IRLS& env, const IRInstruction* inst) {
  auto const func = inst->extra<DefFuncEntryFP>()->func;
  auto const prevFP = srcLoc(env, inst, 0).reg();
  auto const newFP = srcLoc(env, inst, 1).reg();
  auto const callFlags = srcLoc(env, inst, 2).reg();
  auto const numArgs = srcLoc(env, inst, 3).reg();
  auto const ctx = srcLoc(env, inst, 4).reg();
  auto& v = vmain(env);

  v << store{prevFP, newFP + AROFF(m_sfp)};
  v << unstublogue{};
  v << phplogue{newFP};
  emitImmStoreq(v, uintptr_t(func), newFP + AROFF(m_func));

  int32_t constexpr flagsDelta =
    CallFlags::Flags::CallOffsetStart - ActRec::CallOffsetStart;
  assertx(ActRec::LocalsDecRefd == 0);
  assertx(ActRec::AsyncEagerRet == 1);
  assertx(ActRec::CallOffsetStart == 2);
  assertx(CallFlags::Flags::ReservedZero == flagsDelta + 0);
  assertx(CallFlags::Flags::AsyncEagerReturn == flagsDelta + 1);
  assertx(CallFlags::Flags::CallOffsetStart == flagsDelta + 2);
  auto const callFlagsLow32 = v.makeReg();
  auto const callOffAndFlags = v.makeReg();
  v << movtql{callFlags, callFlagsLow32};
  v << shrli{flagsDelta, callFlagsLow32, callOffAndFlags, v.makeReg()};
  v << storel{callOffAndFlags, newFP + AROFF(m_callOffAndFlags)};

  auto const numArgs32 = v.makeReg();
  v << movtql{numArgs, numArgs32};
  v << storel{numArgs32, newFP + AROFF(m_numArgs)};

  if (func->cls()) {
    v << store{ctx, newFP + AROFF(m_thisUnsafe)};
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedThisSlot, newFP + AROFF(m_thisUnsafe));
  }

  if (func->attrs() & AttrMayUseVV) {
    v << storeqi{0, newFP + AROFF(m_varEnv)};
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, newFP + AROFF(m_varEnv));
  }

  v << copy{newFP, dstLoc(env, inst, 0).reg()};
}

void cgIsFunReifiedGenericsMatched(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const callFlags = srcLoc(env, inst, 0).reg();
  auto const func = inst->extra<FuncData>()->func;
  assertx(func->hasReifiedGenerics());
  auto& v = vmain(env);

  auto const info = func->getReifiedGenericsInfo();
  if (info.m_hasSoftGenerics || info.m_typeParamInfo.size() > 15) {
    v << copy{v.cns(0), dst};
    return;
  }

  // Extract generics bitmap from call flags.
  auto const genericsBitmap = v.makeReg();
  auto const genericsBitmap64 = v.makeReg();
  v << shrqi{32, callFlags, genericsBitmap64, v.makeReg()};
  v << movtqw{genericsBitmap64, genericsBitmap};

  // Higher order 16 bits contain the tag in a compact tagged pointer
  // Tag contains ((1 << number-of-parameters) | bitmap)
  auto const bitmapImmed = static_cast<int16_t>(info.m_bitmap);
  auto const topBit = 1u << info.m_typeParamInfo.size();

  auto const sf = v.makeReg();
  if (bitmapImmed == topBit - 1) {
    v << cmpwi{static_cast<int16_t>(bitmapImmed | topBit), genericsBitmap, sf};
  } else {
    auto const anded = v.makeReg();
    v << andwi{static_cast<int16_t>(bitmapImmed | -topBit),
               genericsBitmap, anded, v.makeReg()};
    v << cmpwi{static_cast<int16_t>(bitmapImmed | topBit), anded, sf};
  }
  v << setcc{CC_Z, sf, dst};
}

void cgLdARFlags(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{fp[AROFF(m_callOffAndFlags)], dst};
}

void cgLdARNumParams(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{fp[AROFF(m_numArgs)], dst};
}


void cgLdFrameThis(IRLS& env, const IRInstruction* inst) {
  assertx(!inst->func() || inst->ctx() || inst->func()->isClosureBody());
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << load{fp[AROFF(m_thisUnsafe)], dst};
}

void cgLdFrameCls(IRLS& env, const IRInstruction* inst) {
  return cgLdFrameThis(env, inst);
}

void cgDbgCheckLocalsDecRefd(IRLS& env, const IRInstruction* inst) {
  if (!debug) return;
  auto& v = vmain(env);
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const callOffAndFlags = v.makeReg();
  auto const check = v.makeReg();
  auto const sf = v.makeReg();
  v << loadb{fp[AROFF(m_callOffAndFlags)], callOffAndFlags};
  v << andbi{1 << ActRec::LocalsDecRefd, callOffAndFlags, check, v.makeReg()};
  v << testb{check, check, sf};
  ifThen(v, CC_NZ, sf, [&](Vout& v) {
    v << trap{TRAP_REASON};
  });
}

///////////////////////////////////////////////////////////////////////////////

}}}
