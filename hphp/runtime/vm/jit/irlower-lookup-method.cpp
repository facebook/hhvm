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
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/method-lookup.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdObjMethod(IRLS& env, const IRInstruction* inst) {
  assertx(inst->taken() && inst->taken()->isCatch()); // must have catch block
  using namespace MethodCache;

  auto const cls = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<LdObjMethodData>();
  auto& v = vmain(env);
  auto& vc = vcold(env);

  // Allocate the request-local one-way method cache for this lookup.
  auto const handle = rds::alloc<Entry, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    rds::recordRds(handle, sizeof(TypedValue), "MethodCache",
                   inst->marker().func()->fullName()->toCppString());
  }

  auto const mc_handler = extra->fatal ? tc::ustubs().handlePrimeCacheInitFatal
                                       : tc::ustubs().handlePrimeCacheInit;

  /*
   * The `mcprep' instruction here creates a smashable move, which serves as
   * the inline cache, or "prime cache" for the method lookup.
   *
   * On our first time through this codepath in the TC, we "prime" this cache
   * (which holds across /all/ requests) by smashing the mov immediate to hold
   * a Func* in the upper 32 bits, and a Class* in the lower 32 bits.  This is
   * not always possible (see handlePrimeCacheInit() for details), in which
   * case we smash an immediate with some low bits set, so that we always miss
   * on the inline cache when comparing against our live Class*.
   *
   * The inline cache is set up so that we always miss initially, and take the
   * slow path to initialize it.  After initialization, we also smash the slow
   * path call to point instead to a lookup routine for the out-of-line method
   * cache (allocated above).  The inline cache is guaranteed to be set only
   * once, but the one-way request-local method cache is updated on each miss.
   */
  auto func_class = v.makeReg();
  v << mcprep{func_class};

  // Get the Class* part of the cache line.
  auto tmp = v.makeReg();
  auto classptr = v.makeReg();
  v << movtql{func_class, tmp};
  v << movzlq{tmp, classptr};

  // Check the inline cache.
  auto const sf = v.makeReg();
  v << cmpq{classptr, cls, sf};

  unlikelyIfThenElse(
    v, vc, CC_NE, sf,
    [&] (Vout& v) { // then block (unlikely)
      auto const args = argGroup(env, inst)
        .imm(safe_cast<int32_t>(handle))
        .addr(fp, cellsToBytes(extra->offset.offset))
        .immPtr(extra->method)
        .ssa(0 /* cls */)
        .immPtr(inst->marker().func()->cls())
        .reg(func_class);

      cgCallHelper(v, env, CallSpec::smashable(mc_handler),
                   kVoidDest, SyncOptions::Sync, args);
    },
    [&] (Vout& v) { // else block (likely)
      auto const funcptr = v.makeReg();
      v << shrqi{32, func_class, funcptr, v.makeReg()};
      v << store{funcptr,
                 fp[cellsToBytes(extra->offset.offset) + AROFF(m_func)]};
    });
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(LdClsCtor)

template<bool forward>
void lookupClsMethodHelper(Class* cls, StringData* meth,
                           ActRec* ar, ActRec* fp) {
  try {
    const Func* f;
    auto const ctx = fp->m_func->cls();
    auto const obj = ctx && fp->hasThis() ? fp->getThis() : nullptr;
    auto const res = lookupClsMethod(f, cls, meth, obj, ctx, true);

    ar->m_func = f;

    if (res == LookupResult::MethodFoundNoThis ||
        res == LookupResult::MagicCallStaticFound) {
      if (!f->isStaticInPrologue()) {
        raise_missing_this(f);
      }
      if (forward && ctx) {
        if (fp->hasThis()) {
          cls = fp->getThis()->getVMClass();
        } else {
          cls = fp->getClass();
        }
      }
      ar->setClass(cls);
    } else {
      assertx(obj);
      assertx(res == LookupResult::MethodFoundWithThis ||
              res == LookupResult::MagicCallFound);
      obj->incRefCount();
      ar->setThis(obj);
    }

    if (res == LookupResult::MagicCallFound ||
        res == LookupResult::MagicCallStaticFound) {
      ar->setMagicDispatch(meth);
      meth->incRefCount();
    }
  } catch (...) {
    *arPreliveOverwriteCells(ar) = make_tv<KindOfString>(meth);
    throw;
  }
}

void cgLookupClsMethod(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LookupClsMethod>();
  auto const sp = srcLoc(env, inst, 2).reg();

  auto const args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .addr(sp, cellsToBytes(extra->calleeAROffset.offset))
    .ssa(3);

  if (extra->forward) {
    cgCallHelper(vmain(env), env,
                 CallSpec::direct(lookupClsMethodHelper<true>),
                 callDest(env, inst), SyncOptions::Sync, args);
  } else {
    cgCallHelper(vmain(env), env,
                 CallSpec::direct(lookupClsMethodHelper<false>),
                 callDest(env, inst), SyncOptions::Sync, args);
  }
}

void cgProfileMethod(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileMethodData>();
  auto const sp = srcLoc(env, inst, 0).reg();

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .addr(sp, cellsToBytes(extra->bcSPOff.offset))
    .ssa(1);

  cgCallHelper(vmain(env), env, CallSpec::method(&MethProfile::reportMeth),
               kVoidDest, SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

const char* ctxName(const BCMarker& marker) {
  auto const ctx = marker.func()->cls();
  return ctx ? ctx->name()->data() : ":anonymous:";
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLookupClsMethodCache(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodCache::alloc(
    extra->clsName,
    extra->methodName,
    ctxName(inst->marker())
  );

  if (false) { // typecheck
    UNUSED TypedValue* fake_fp = nullptr;
    const UNUSED Func* f = StaticMethodCache::lookup(
      ch,
      extra->namedEntity,
      extra->clsName,
      extra->methodName,
      fake_fp
    );
  }

  auto const args = argGroup(env, inst)
    .imm(ch)
    .immPtr(extra->namedEntity)
    .immPtr(extra->clsName)
    .immPtr(extra->methodName)
    .reg(fp);

  // May raise an error if the class is undefined.
  cgCallHelper(v, env, CallSpec::direct(StaticMethodCache::lookup),
               callDest(dst), SyncOptions::Sync, args);
}

void cgLdClsMethodCacheFunc(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodCache::alloc(
    extra->clsName,
    extra->methodName,
    ctxName(inst->marker())
  );

  auto const sf = checkRDSHandleInitialized(v, ch);
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodCache, m_func)],
               dst, sizeof(LowPtr<const Func>));
}

void cgLdClsMethodCacheCls(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodCache::alloc(
    extra->clsName,
    extra->methodName,
    ctxName(inst->marker())
  );

  // The StaticMethodCache here is guaranteed to already be initialized in RDS
  // by the pre-conditions of this instruction.
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodCache, m_cls)],
               dst, sizeof(LowPtr<const Class>));
}

void cgLookupClsMethodFCache(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg(0);
  auto const cls = inst->src(0)->clsVal();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodFCache::alloc(
    cls->name(),
    extra->methodName,
    ctxName(inst->marker())
  );
  assertx(rds::isNormalHandle(ch));

  const Func* (*lookup)(rds::Handle, const Class*,
                        const StringData*, TypedValue*) =
    StaticMethodFCache::lookup;

  auto const args = argGroup(env, inst)
   .imm(ch)
   .immPtr(cls)
   .immPtr(extra->methodName)
   .reg(fp);

  cgCallHelper(v, env, CallSpec::direct(lookup),
               callDest(dst), SyncOptions::Sync, args);
}

void cgLdClsMethodFCacheFunc(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodFCache::alloc(
    extra->clsName,
    extra->methodName,
    ctxName(inst->marker())
  );

  auto const sf = checkRDSHandleInitialized(v, ch);
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodFCache, m_func)],
               dst, sizeof(LowPtr<const Func>));
}

///////////////////////////////////////////////////////////////////////////////

void cgCheckFuncStatic(IRLS& env, const IRInstruction* inst) {
  auto const funcPtrReg = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testlim{AttrStatic, funcPtrReg[Func::attrsOff()], sf};
  v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgFwdCtxStaticCall(IRLS& env, const IRInstruction* inst) {
  auto const dstCtx = dstLoc(env, inst, 0).reg();
  auto const srcCtx = srcLoc(env, inst, 0).reg();
  auto const ty = inst->src(0)->type();

  auto& v = vmain(env);

  auto ctx_from_this =  [] (Vout& v, Vreg rthis, Vreg dst) {
    // Load (this->m_cls | 0x1) into `dst'.
    auto const cls = emitLdObjClass(v, rthis, v.makeReg());
    v << orqi{ActRec::kHasClassBit, cls, dst, v.makeReg()};
    return dst;
  };

  if (ty <= TCctx) {
    v << copy{srcCtx, dstCtx};
  } else if (ty <= TObj) {
    ctx_from_this(v, srcCtx, dstCtx);
  } else {
    // If we don't know whether we have a $this, we need to check dynamically.
    auto const sf = v.makeReg();
    v << testqi{ActRec::kHasClassBit, srcCtx, sf};
    unlikelyCond(
      v, vcold(env), CC_NZ, sf, dstCtx, [&](Vout& /*v*/) { return srcCtx; },
      [&](Vout& v) { return ctx_from_this(v, srcCtx, v.makeReg()); });
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
