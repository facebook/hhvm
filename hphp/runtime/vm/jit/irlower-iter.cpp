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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
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

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

int iterOffset(const BCMarker& marker, uint32_t id) {
  auto const func = marker.func();
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

void implIterInit(IRLS& env, const IRInstruction* inst) {
  bool isInitK = inst->is(IterInitK, LIterInitK);
  bool isLInit = inst->is(LIterInit, LIterInitK);

  auto const extra = inst->extra<IterInitData>();

  auto const src = inst->src(0);
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  auto const valOff = localOffset(extra->valId);

  auto& v = vmain(env);

  auto args = argGroup(env, inst)
    .addr(fp, iterOff)
    .ssa(0 /* src */);

  if (src->isA(TArrLike)) {
    args.addr(fp, valOff);
    if (isInitK) {
      args.addr(fp, localOffset(extra->keyId));
    }

    auto const target = [&] {
      if (isLInit) {
        if (isInitK) return CallSpec::direct(new_iter_array_key<true>);
        return CallSpec::direct(new_iter_array<true>);
      } else if (isInitK) {
        return CallSpec::direct(new_iter_array_key<false>);
      } else {
        return CallSpec::direct(new_iter_array<false>);
      }
    }();

    cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
    return;
  }

  always_assert(src->type() <= TObj);
  always_assert(!isLInit);

  args.immPtr(inst->marker().func()->cls())
      .addr(fp, valOff);
  if (isInitK) {
    args.addr(fp, localOffset(extra->keyId));
  } else {
    args.imm(0);
  }

  // new_iter_object decrefs its src object if it propagates an exception
  // out, so we use SyncAdjustOne, which adjusts the stack pointer by 1 stack
  // element on an unwind, skipping over the src object.
  cgCallHelper(
    v, env, CallSpec::direct(new_iter_object),
    callDest(env, inst),
    extra->fromStack ? SyncOptions::SyncAdjustOne : SyncOptions::Sync,
    args
  );
}

void implIterNext(IRLS& env, const IRInstruction* inst) {
  bool isNextK = inst->is(IterNextK);

  auto const extra = inst->extra<IterData>();

  auto const args = [&] {
    auto const fp = srcLoc(env, inst, 0).reg();

    auto ret = argGroup(env, inst)
      .addr(fp, iterOffset(inst->marker(), extra->iterId))
      .addr(fp, localOffset(extra->valId));
    if (isNextK) ret.addr(fp, localOffset(extra->keyId));

    return ret;
  }();

  auto const target = isNextK ? CallSpec::direct(iter_next_key_ind) :
                                CallSpec::direct(iter_next_ind);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void implLIterNext(IRLS& env, const IRInstruction* inst) {
  always_assert(inst->is(LIterNext, LIterNextK));
  auto const isKey = inst->is(LIterNextK);

  auto const extra = inst->extra<IterData>();

  auto const args = [&] {
    auto const fp = srcLoc(env, inst, 1).reg();
    auto ret = argGroup(env, inst)
      .addr(fp, iterOffset(inst->marker(), extra->iterId))
      .addr(fp, localOffset(extra->valId));
    if (isKey) ret.addr(fp, localOffset(extra->keyId));
    ret.ssa(0);
    return ret;
  }();

  auto const target = isKey
    ? CallSpec::direct(liter_next_key_ind)
    : CallSpec::direct(liter_next_ind);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void implIterFree(IRLS& env, const IRInstruction* inst, CallSpec meth) {
  auto const extra = inst->extra<IterId>();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);

  cgCallHelper(vmain(env), env, meth, kVoidDest, SyncOptions::Sync,
               argGroup(env, inst).addr(fp, iterOff));
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void cgIterInit(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterInitK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgLIterInit(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgLIterInitK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgIterNext(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgIterNextK(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgLIterNext(IRLS& env, const IRInstruction* inst) {
  implLIterNext(env, inst);
}

void cgLIterNextK(IRLS& env, const IRInstruction* inst) {
  implLIterNext(env, inst);
}

void cgIterFree(IRLS& env, const IRInstruction* inst) {
  implIterFree(env, inst, CallSpec::method(&Iter::free));
}

///////////////////////////////////////////////////////////////////////////////

}}}
