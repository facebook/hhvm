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
  bool isInitK = inst->is(IterInitK, WIterInitK, LIterInitK);
  bool isWInit = inst->is(WIterInit, WIterInitK);
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
    } else if (isWInit) {
      args.imm(0);
    }

    auto const target = [&] {
      if (isWInit) {
        return CallSpec::direct(new_iter_array_key<true, false>);
      } else if (isLInit) {
        if (isInitK) return CallSpec::direct(new_iter_array_key<false, true>);
        return CallSpec::direct(new_iter_array<true>);
      } else if (isInitK) {
        return CallSpec::direct(new_iter_array_key<false, false>);
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

void implMIterInit(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<IterInitData>();

  auto const fp = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  auto const valOff = localOffset(extra->valId);

  auto& v = vmain(env);

  auto args = argGroup(env, inst)
    .addr(fp, iterOff)
    .ssa(0 /* src */);

  auto const innerType = inst->typeParam();
  assertx(innerType.isKnownDataType());

  if (innerType <= TArrLike) {
    args.addr(fp, valOff);
    if (inst->is(MIterInitK)) {
      args.addr(fp, localOffset(extra->keyId));
    } else {
      args.imm(0);
    }
    cgCallHelper(v, env, CallSpec::direct(new_miter_array_key),
                 callDest(env, inst), SyncOptions::Sync, args);
    return;
  }

  always_assert(innerType <= TObj);

  args.immPtr(inst->marker().func()->cls())
      .addr(fp, valOff);
  if (inst->is(MIterInitK)) {
    args.addr(fp, localOffset(extra->keyId));
  } else {
    args.imm(0);
  }

  // new_miter_object decrefs its src object if it propagates an exception out,
  // so we use SyncAdjustOne, which adjusts the stack pointer by 1 stack
  // element on an unwind, skipping over the src object.
  cgCallHelper(v, env, CallSpec::direct(new_miter_object),
               callDest(env, inst), SyncOptions::SyncAdjustOne, args);
}

void implIterNext(IRLS& env, const IRInstruction* inst) {
  // Nothing uses WIterNext so we intentionally don't support it here to avoid
  // a null check in the witer_next_key helper.  This will need to change if we
  // ever start using it in an hhas file.
  always_assert(!inst->is(WIterNext));

  bool isNextK = inst->is(IterNextK, WIterNextK);
  bool isWNext = inst->is(WIterNext, WIterNextK);

  auto const extra = inst->extra<IterData>();

  auto const args = [&] {
    auto const fp = srcLoc(env, inst, 0).reg();

    auto ret = argGroup(env, inst)
      .addr(fp, iterOffset(inst->marker(), extra->iterId))
      .addr(fp, localOffset(extra->valId));
    if (isNextK) ret.addr(fp, localOffset(extra->keyId));

    return ret;
  }();

  auto const target = isWNext ? CallSpec::direct(witer_next_key) :
                      isNextK ? CallSpec::direct(iter_next_key_ind) :
                                CallSpec::direct(iter_next_ind);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void implMIterNext(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<IterData>();

  auto const args = [&] {
    auto const fp = srcLoc(env, inst, 0).reg();

    auto ret = argGroup(env, inst)
      .addr(fp, iterOffset(inst->marker(), extra->iterId))
      .addr(fp, localOffset(extra->valId));
    if (inst->is(MIterNextK)) {
      ret.addr(fp, localOffset(extra->keyId));
    } else {
      ret.imm(0);
    }
    return ret;
  }();
  cgCallHelper(vmain(env), env, CallSpec::direct(miter_next_key),
               callDest(env, inst), SyncOptions::Sync, args);
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

void cgWIterInit(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgWIterInitK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgLIterInit(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgLIterInitK(IRLS& env, const IRInstruction* inst) {
  implIterInit(env, inst);
}

void cgMIterInit(IRLS& env, const IRInstruction* inst) {
  implMIterInit(env, inst);
}

void cgMIterInitK(IRLS& env, const IRInstruction* inst) {
  implMIterInit(env, inst);
}

void cgIterNext(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgIterNextK(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgWIterNext(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgWIterNextK(IRLS& env, const IRInstruction* inst) {
  implIterNext(env, inst);
}

void cgMIterNext(IRLS& env, const IRInstruction* inst) {
  implMIterNext(env, inst);
}

void cgMIterNextK(IRLS& env, const IRInstruction* inst) {
  implMIterNext(env, inst);
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

void cgMIterFree(IRLS& env, const IRInstruction* inst) {
  implIterFree(env, inst, CallSpec::method(&Iter::mfree));
}

///////////////////////////////////////////////////////////////////////////////

int64_t decodeCufIterHelper(Iter* it, TypedValue func, ActRec* ar) {
  ObjectData* obj = nullptr;
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool dynamic = false;

  if (LIKELY(ar->func()->isBuiltin())) {
    ar = g_context->getOuterVMFrame(ar);
  }
  auto const f = vm_decode_function(tvAsVariant(&func), ar, false,
                                    obj, cls, invName, dynamic,
                                    DecodeFlags::NoWarn);
  if (UNLIKELY(!f)) return false;

  auto& cit = it->cuf();
  assertx(dynamic == cit.dynamic());
  cit.setFunc(f);
  if (obj) {
    cit.setCtx(obj);
    obj->incRefCount();
  } else {
    cit.setCtx(cls);
  }
  cit.setName(invName);
  return true;
}

void cgDecodeCufIter(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<DecodeCufIter>();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);

  auto const args = argGroup(env, inst)
    .addr(fp, iterOff)
    .typedValue(0)
    .reg(fp);

  cgCallHelper(vmain(env), env, CallSpec::direct(decodeCufIterHelper),
               callDest(env, inst), SyncOptions::Sync, args);
}

void cgStCufIterFunc(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<StCufIterFunc>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const func    = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << store{func, fp[iterOff + CufIter::funcOff()]};
}

void cgStCufIterCtx(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<StCufIterCtx>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const ctx     = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << store{ctx, fp[iterOff + CufIter::ctxOff()]};
}

void cgStCufIterInvName(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<StCufIterInvName>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const name    = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << store{name, fp[iterOff + CufIter::nameOff()]};
}

void cgStCufIterDynamic(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<StCufIterDynamic>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const dynamic = srcLoc(env, inst, 1).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << storeb{dynamic, fp[iterOff + CufIter::dynamicOff()]};
}

void cgLdCufIterFunc(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<LdCufIterFunc>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const dst     = dstLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << load{fp[iterOff + CufIter::funcOff()], dst};
}

void cgLdCufIterCtx(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<LdCufIterCtx>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const dst     = dstLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << load{fp[iterOff + CufIter::ctxOff()], dst};
}

void cgLdCufIterInvName(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<LdCufIterInvName>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const dst     = dstLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << load{fp[iterOff + CufIter::nameOff()], dst};
}

void cgLdCufIterDynamic(IRLS& env, const IRInstruction* inst) {
  auto const extra   = inst->extra<LdCufIterDynamic>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const dst     = dstLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);
  vmain(env) << loadtqb{fp[iterOff + CufIter::dynamicOff()], dst};
}

void cgKillCufIter(IRLS& env, const IRInstruction* inst) {
  if (!RuntimeOption::EvalHHIRGenerateAsserts) return;

  auto& v            = vmain(env);
  auto const extra   = inst->extra<KillCufIter>();
  auto const fp      = srcLoc(env, inst, 0).reg();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);

  uint64_t trash;
  memset(&trash, kTrashCufIter, sizeof(trash));
  auto const trashCns = v.cns(trash);

  v << store{trashCns, fp[iterOff + CufIter::funcOff()]};
  v << store{trashCns, fp[iterOff + CufIter::ctxOff()]};
  v << store{trashCns, fp[iterOff + CufIter::nameOff()]};
  v << storeb{trashCns, fp[iterOff + CufIter::dynamicOff()]};
}

///////////////////////////////////////////////////////////////////////////////

}}}
