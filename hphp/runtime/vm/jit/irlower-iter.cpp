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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/iter.h"

#include <folly/container/Array.h>

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

int iterOffset(const BCMarker& marker, uint32_t id) {
  auto const func = marker.func();
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

void implIterInit(IRLS& env, const IRInstruction* inst) {
  auto const isInitK = inst->is(IterInitArrK, IterInitObjK);
  auto const extra = &inst->extra<IterData>()->args;

  auto const src = inst->src(0);
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  auto const valOff = localOffset(extra->valId);

  auto& v = vmain(env);

  if (src->isA(TArrLike)) {
    auto args = argGroup(env, inst)
      .addr(fp, iterOff)
      .ssa(0 /* src */)
      .addr(fp, valOff);
    if (isInitK) {
      args.addr(fp, localOffset(extra->keyId));
    }

    auto const op = [&]{
      auto const flag = has_flag(extra->flags, IterArgs::Flags::BaseConst);
      return flag ? IterTypeOp::LocalBaseConst : IterTypeOp::LocalBaseMutable;
    }();
    auto const target = isInitK
      ? CallSpec::direct(new_iter_array_key_helper(op))
      : CallSpec::direct(new_iter_array_helper(op));
    cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
    return;
  }

  always_assert(src->type() <= TObj);

  auto args = argGroup(env, inst)
    .ssa(0 /* src */)
    .addr(fp, valOff);
  if (isInitK) {
    args.addr(fp, localOffset(extra->keyId));
  } else {
    args.imm(0);
  }

  auto const target = CallSpec::direct(new_iter_object);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void implIterNext(IRLS& env, const IRInstruction* inst, CallSpec target) {
  always_assert(inst->is(IterNextArr, IterNextArrK,
                         IterNextObj, IterNextObjK));
  auto const isArr = inst->is(IterNextArr, IterNextArrK);
  auto const isKey = inst->is(IterNextArrK, IterNextObjK);
  auto const extra = &inst->extra<IterData>()->args;

  auto const sync = isArr ? SyncOptions::None : SyncOptions::Sync;
  auto const args = [&] {
    auto const fp = srcLoc(env, inst, 1).reg();
    auto ret = argGroup(env, inst);
    if (isArr) ret.addr(fp, iterOffset(inst->marker(), extra->iterId));
    ret.addr(fp, localOffset(extra->valId));
    if (isKey) ret.addr(fp, localOffset(extra->keyId));
    ret.ssa(0);
    return ret;
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), sync, args);
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename T>
Vptr iteratorPtr(IRLS& env, const IRInstruction* inst, const T* extra) {
  assertx(inst->src(0)->isA(TFramePtr));
  auto const fp = srcLoc(env, inst, 0).reg();
  return fp[iterOffset(inst->marker(), extra->iterId)];
}

int32_t iteratorType(const IterTypeData& data) {
  auto const nextHelperIndex = [&]{
    if (data.type.bespoke) {
      if (!data.baseConst) return IterNextIndex::Array;
      if (data.layout.is_struct()) return IterNextIndex::StructDict;
      return IterNextIndex::Array;
    }
    switch (data.baseType) {
      case KindOfVec: {
        auto is_ptr_iter = data.baseConst
                        && !data.outputKey
                        && VanillaVec::stores_unaligned_typed_values;
        return is_ptr_iter
          ? IterNextIndex::VanillaVecPointer
          : IterNextIndex::VanillaVec;
      }
      case KindOfDict: {
        return data.baseConst
          ? IterNextIndex::ArrayMixedPointer
          : IterNextIndex::ArrayMixed;
      }
      default:
        always_assert(false);
    }
  }();

  auto const type = IterImpl::packTypeFields(
      nextHelperIndex, data.type, data.layout.toUint16());
  return safe_cast<int32_t>(type);
}

}

void cgCheckIter(IRLS& env, const IRInstruction* inst) {
  static_assert(sizeof(IterSpecialization) == 1, "");
  auto const iter = iteratorPtr(env, inst, inst->extra<CheckIter>());
  auto const data = inst->extra<CheckIter>();
  auto const type = data->type;
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  // For bespoke specialized iterators, we have to check both the type byte
  // and the ArrayLayout. For vanilla iterators, we can check the type alone.
  if (type.bespoke) {
    auto const full_type = iteratorType(*data);
    v << cmplim{full_type, iter + IterImpl::typeOffset(), sf};
  } else {
    v << cmpbim{type.as_byte, iter + IterImpl::specializationOffset(), sf};
  }
  v << jcc{CC_NE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgLdIterPos(IRLS& env, const IRInstruction* inst) {
  static_assert(IterImpl::posSize() == 8, "");
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<LdIterPos>());
  vmain(env) << load{iter + IterImpl::posOffset(), dst};
}

void cgLdIterEnd(IRLS& env, const IRInstruction* inst) {
  static_assert(IterImpl::endSize() == 8, "");
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<LdIterEnd>());
  vmain(env) << load{iter + IterImpl::endOffset(), dst};
}

void cgStIterType(IRLS& env, const IRInstruction* inst) {
  static_assert(IterImpl::typeSize() == 4, "");
  auto const type = iteratorType(*inst->extra<StIterType>());
  auto const iter = iteratorPtr(env, inst, inst->extra<StIterType>());
  vmain(env) << storeli{type, iter + IterImpl::typeOffset()};
}

void cgStIterPos(IRLS& env, const IRInstruction* inst) {
  static_assert(IterImpl::posSize() == 8, "");
  auto const src  = srcLoc(env, inst, 1).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<StIterPos>());
  vmain(env) << store{src, iter + IterImpl::posOffset()};
}

void cgStIterEnd(IRLS& env, const IRInstruction* inst) {
  static_assert(IterImpl::endSize() == 8, "");
  auto const src  = srcLoc(env, inst, 1).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<StIterEnd>());
  vmain(env) << store{src, iter + IterImpl::endOffset()};
}

void cgKillIter(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  v << killeffects{};
  if (!debug) return;

  int32_t trash;
  memset(&trash, kIterTrashFill, sizeof(trash));
  auto const iter = iteratorPtr(env, inst, inst->extra<KillIter>());
  for (auto i = 0; i < sizeof(IterImpl); i += sizeof(trash)) {
    v << storeli{trash, iter + i};
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgProfileIterInit(IRLS& env, const IRInstruction* inst) {
  assertx(inst->src(0)->type().subtypeOfAny(TVec, TDict, TKeyset));
  auto const extra = inst->extra<RDSHandleData>();
  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .ssa(0);
  cgCallHelper(vmain(env), env, CallSpec::method(&ArrayIterProfile::update),
               kVoidDest, SyncOptions::None, args);
}

void cgIterInitArr(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterInitArrK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterInitObj(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterInitObjK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterNextArr(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst, CallSpec::direct(iter_array_next_ind));
}

void cgIterNextArrK(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst, CallSpec::direct(iter_array_next_key_ind));
}

void cgIterNextObj(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst, CallSpec::direct(iter_object_next_ind));
}

void cgIterNextObjK(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst, CallSpec::direct(iter_object_next_key_ind));
}

IMPL_OPCODE_CALL(IterExtractBase)

///////////////////////////////////////////////////////////////////////////////

}
