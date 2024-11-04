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

void implIterArr(
  IRLS& env,
  const IRInstruction* inst,
  CallSpec target,
  CallDest dstInfo
) {
  assertx(inst->is(IterInitArr, IterNextArr));
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const iterId = inst->extra<IterData>()->args.iterId;
  auto const args = argGroup(env, inst)
    .addr(fp, iterOffset(inst->marker(), iterId))
    .ssa(0 /* arr */);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, dstInfo, SyncOptions::None, args);
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

}

void cgLdIterPos(IRLS& env, const IRInstruction* inst) {
  static_assert(Iter::posSize() == 8, "");
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<LdIterPos>());
  vmain(env) << load{iter + Iter::posOffset(), dst};
}

void cgLdIterEnd(IRLS& env, const IRInstruction* inst) {
  static_assert(Iter::endSize() == 8, "");
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<LdIterEnd>());
  vmain(env) << load{iter + Iter::endOffset(), dst};
}

void cgStIterPos(IRLS& env, const IRInstruction* inst) {
  static_assert(Iter::posSize() == 8, "");
  auto const src  = srcLoc(env, inst, 1).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<StIterPos>());
  vmain(env) << store{src, iter + Iter::posOffset()};
}

void cgStIterEnd(IRLS& env, const IRInstruction* inst) {
  static_assert(Iter::endSize() == 8, "");
  auto const src  = srcLoc(env, inst, 1).reg();
  auto const iter = iteratorPtr(env, inst, inst->extra<StIterEnd>());
  vmain(env) << store{src, iter + Iter::endOffset()};
}

void cgKillIter(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  v << killeffects{};
  if (!debug) return;

  int32_t trash;
  memset(&trash, kIterTrashFill, sizeof(trash));
  auto const iter = iteratorPtr(env, inst, inst->extra<KillIter>());
  for (auto i = 0; i < sizeof(Iter); i += sizeof(trash)) {
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

void cgIterGetKeyArr(IRLS& env, const IRInstruction* inst) {
  auto const flags = inst->extra<IterData>()->args.flags;
  auto const target = CallSpec::direct(iter_select(iter_get_key_array, flags));
  auto const dstInfo = callDestTV(env, inst);
  auto const args = argGroup(env, inst).ssa(0 /* arr */).ssa(1 /* pos */);
  cgCallHelper(vmain(env), env, target, dstInfo, SyncOptions::None, args);
}

void cgIterGetValArr(IRLS& env, const IRInstruction* inst) {
  auto const flags = inst->extra<IterData>()->args.flags;
  auto const target =
    CallSpec::direct(iter_select(iter_get_value_array, flags));
  auto const dstInfo = callDestTV(env, inst);
  auto const args = argGroup(env, inst).ssa(0 /* arr */).ssa(1 /* pos */);
  cgCallHelper(vmain(env), env, target, dstInfo, SyncOptions::None, args);
}

void cgIterInitArr(IRLS& env, const IRInstruction* inst) {
  auto const flags = inst->extra<IterData>()->args.flags;
  auto const target = CallSpec::direct(iter_select(iter_init_array, flags));
  implIterArr(env, inst, target, callDest(env, inst));
}

void cgIterNextArr(IRLS& env, const IRInstruction* inst) {
  auto const flags = inst->extra<IterData>()->args.flags;
  auto const target = CallSpec::direct(iter_select(iter_next_array, flags));
  implIterArr(env, inst, target, callDest(env, inst));
}

IMPL_OPCODE_CALL(IterExtractBase)
IMPL_OPCODE_CALL(IterInitObj)
IMPL_OPCODE_CALL(IterNextObj)

///////////////////////////////////////////////////////////////////////////////

}
