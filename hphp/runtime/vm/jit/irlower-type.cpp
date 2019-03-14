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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/ref-data.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type-profile.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgCheckType(IRLS& env, const IRInstruction* inst) {
  // Note: If you add new supported type checks, you should update
  // negativeCheckType() to indicate whether it is precise or not.
  auto const src = inst->src(0);
  auto const dst = inst->dst();
  auto const srcData = srcLoc(env, inst, 0).reg(0);
  auto const srcType = srcLoc(env, inst, 0).reg(1);

  auto& v = vmain(env);

  auto const doJcc = [&] (ConditionCode cc, Vreg sf) {
    fwdJcc(v, env, ccNegate(cc), sf, inst->taken());
  };

  auto const doMov = [&] {
    auto const dstData = dstLoc(env, inst, 0).reg(0);
    auto const dstType = dstLoc(env, inst, 0).reg(1);

    if (dst->isA(TBool) && !src->isA(TBool)) {
      v << movtqb{srcData, dstData};
    } else {
      v << copy{srcData, dstData};
    }
    if (dstType == InvalidReg) return;
    if (srcType != InvalidReg) {
      v << copy{srcType, dstType};
    } else {
      v << ldimmb{static_cast<data_type_t>(src->type().toDataType()), dstType};
    }
  };

  auto const typeParam = inst->typeParam();

  if (src->isA(typeParam)) {
    // src is the target type or better.  Just define our dst.
    doMov();
    return;
  }
  if (!src->type().maybe(typeParam)) {
    // src is definitely not the target type.  Always jump.
    v << jmp{label(env, inst->taken())};
    return;
  }

  if (srcType != InvalidReg) {
    emitTypeTest(v, env, typeParam, srcType, srcData, v.makeReg(), doJcc);
    doMov();
    return;
  }

  if (src->type() <= TBoxedCell && typeParam <= TBoxedCell) {
    // We should never have specific known Boxed types; those should only be
    // used for hints and predictions.
    always_assert(!(typeParam < TBoxedInitCell));
    doMov();
    return;
  }

  /*
   * See if we're just checking the array kind or object class of a value with
   * a mostly-known type.
   *
   * Important: We don't support typeParam being something like
   * StaticArr=kPackedKind unless the src->type() also already knows its
   * staticness.  We do allow things like CheckType<Arr=Packed> t1:StaticArr,
   * though.  This is why we have to check that the unspecialized type is at
   * least as big as the src->type().
   */
  if (typeParam.isSpecialized() &&
      typeParam.unspecialize() >= src->type()) {
    detail::emitSpecializedTypeTest(v, env, typeParam, srcData,
                                    v.makeReg(), doJcc);
    doMov();
    return;
  }

  /*
   * Since not all of our unions carry a type register, there are some
   * situations with strings and arrays that are neither constantly-foldable
   * nor in the emitTypeTest() code path.
   *
   * We currently actually check their persistent bit here, which will let
   * both static and uncounted strings through.  Also note that
   * CheckType<Uncounted> t1:{Null|Str} doesn't get this treatment currently---
   * the emitTypeTest() path above will only check the type register.
   */
  if (!typeParam.isSpecialized() &&
      typeParam <= TUncounted &&
      src->type().subtypeOfAny(TStr, TArrLike) &&
      src->type().maybe(typeParam)) {
    assertx(src->type().maybe(TPersistent));

    auto const sf = emitCmpRefCount(v, 0, srcData);
    doJcc(CC_L, sf);
    doMov();
    return;
  }

  always_assert_flog(
    false,
    "Bad src: {} and dst: {} types in '{}'",
    src->type(), typeParam, *inst
  );
}

void cgCheckTypeMem(IRLS& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const srcLoc = tmpLoc(env, src);
  emitTypeCheck(vmain(env), env, inst->typeParam(),
                memTVTypePtr(src, srcLoc), memTVValPtr(src, srcLoc),
                inst->taken());
}

void cgCheckLoc(IRLS& env, const IRInstruction* inst) {
  auto const baseOff = localOffset(inst->extra<CheckLoc>()->locId);
  auto const base = srcLoc(env, inst, 0).reg()[baseOff];

  emitTypeCheck(vmain(env), env, inst->typeParam(),
                base + TVOFF(m_type), base + TVOFF(m_data), inst->taken());
}

void cgCheckStk(IRLS& env, const IRInstruction* inst) {
  auto const baseOff = cellsToBytes(inst->extra<CheckStk>()->offset.offset);
  auto const base = srcLoc(env, inst, 0).reg()[baseOff];

  emitTypeCheck(vmain(env), env, inst->typeParam(),
                base + TVOFF(m_type), base + TVOFF(m_data), inst->taken());
}

void cgCheckRefInner(IRLS& env, const IRInstruction* inst) {
  if (inst->typeParam() >= TInitCell) return;
  auto const base = srcLoc(env, inst, 0).reg()[RefData::cellOffset()];

  emitTypeCheck(vmain(env), env, inst->typeParam(),
                base + TVOFF(m_type), base + TVOFF(m_data), inst->taken());
}

void cgCheckMBase(IRLS& env, const IRInstruction* inst) {
  cgCheckTypeMem(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void implIsType(IRLS& env, const IRInstruction* inst, bool negate) {
  auto const src = inst->src(0);
  auto const loc = srcLoc(env, inst, 0);
  auto& v = vmain(env);

  auto const doJcc = [&] (ConditionCode cc, Vreg sf) {
    auto const dst = dstLoc(env, inst, 0).reg();
    v << setcc{negate ? ccNegate(cc) : cc, sf, dst};
  };

  if (src->isA(TPtrToGen)) {
    auto const base = loc.reg();
    emitTypeTest(v, env, inst->typeParam(), base[TVOFF(m_type)],
                 base[TVOFF(m_data)], v.makeReg(), doJcc);
    return;
  }
  assertx(src->isA(TGen));

  auto const data = loc.reg(0);
  auto const type = loc.reg(1) != InvalidReg
    ? loc.reg(1)
    : v.cns(src->type().toDataType());

  emitTypeTest(v, env, inst->typeParam(), type, data, v.makeReg(), doJcc);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgIsType(IRLS& env, const IRInstruction* inst) {
  implIsType(env, inst, false);
}
void cgIsNType(IRLS& env, const IRInstruction* inst) {
  implIsType(env, inst, true);
}
void cgIsTypeMem(IRLS& env, const IRInstruction* inst) {
  implIsType(env, inst, false);
}
void cgIsNTypeMem(IRLS& env, const IRInstruction* inst) {
  implIsType(env, inst, true);
}

///////////////////////////////////////////////////////////////////////////////

void cgCheckVArray(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testbim{ArrayData::kVArray, src + ArrayData::offsetofDVArray(), sf};
  fwdJcc(v, env, CC_Z, sf, inst->taken());
  v << copy{src, dst};
}

void cgCheckDArray(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testbim{ArrayData::kDArray, src + ArrayData::offsetofDVArray(), sf};
  fwdJcc(v, env, CC_Z, sf, inst->taken());
  v << copy{src, dst};
}

void cgIsDVArray(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testbim{ArrayData::kDVArrayMask, src + ArrayData::offsetofDVArray(), sf};
  v << setcc{CC_NZ, sf, dst};
}

///////////////////////////////////////////////////////////////////////////////

void cgAssertType(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const& dtype = inst->dst()->type();
  if (dtype == TBottom) {
    v << trap{TRAP_REASON};
    v = v.makeBlock();
    return;
  }

  auto const dst = dstLoc(env, inst, 0);
  auto const src = srcLoc(env, inst, 0);
  copyTV(v, src, dst, dtype);
}

void cgAssertLoc(IRLS&, const IRInstruction*) {}
void cgAssertStk(IRLS&, const IRInstruction*) {}
void cgAssertMBase(IRLS&, const IRInstruction*) {}
void cgHintLocInner(IRLS&, const IRInstruction*) {}
void cgHintStkInner(IRLS&, const IRInstruction*) {}
void cgHintMBaseInner(IRLS&, const IRInstruction*) {}

void cgProfileType(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .typedValue(0);

  cgCallHelper(vmain(env), env, CallSpec::method(&TypeProfile::report),
               kVoidDest, SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void implRecordReifiedGenericsAndGetData(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<StackRangeData>();
  auto& v = vmain(env);

  auto const args = argGroup(env, inst)
    .imm(extra->size)
    .addr(sp, cellsToBytes(extra->offset.offset));

  auto func = inst->op() == RecordReifiedGenericsAndGetName
              ? CallSpec::direct(recordReifiedGenericsAndGetName)
              : CallSpec::direct(recordReifiedGenericsAndGetTSList);

  cgCallHelper(v, env, func, callDest(env, inst), SyncOptions::Sync, args);
}

} // namespace

void cgRecordReifiedGenericsAndGetName(IRLS& env, const IRInstruction* inst) {
  implRecordReifiedGenericsAndGetData(env, inst);
}

void cgRecordReifiedGenericsAndGetTSList(IRLS& env, const IRInstruction* inst) {
  implRecordReifiedGenericsAndGetData(env, inst);
}

void cgResolveTypeStruct(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<ResolveTypeStructData>();
  auto& v = vmain(env);

  auto const args = argGroup(env, inst)
    .imm(extra->size)      // num
    .addr(sp, cellsToBytes(extra->offset.offset)) // values
    .imm(extra->cls)       // declaring cls
    .ssa(1)                // called cls
    .imm(extra->suppress)  // suppress
    .imm(extra->isOrAsOp); // isOrAsOp

  cgCallHelper(v, env, CallSpec::direct(resolveTypeStructHelper),
               callDest(env, inst), SyncOptions::Sync, args);
}

IMPL_OPCODE_CALL(MangleReifiedName)

///////////////////////////////////////////////////////////////////////////////

}}}
