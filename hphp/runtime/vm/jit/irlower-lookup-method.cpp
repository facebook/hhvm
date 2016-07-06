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
#include "hphp/runtime/vm/func.h"

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

  // Allocate the request-local one-way method cache for this lookup.
  auto const handle = rds::alloc<Entry, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    rds::recordRds(handle, sizeof(TypedValue), "MethodCache",
                   inst->marker().func()->fullName()->toCppString());
  }

  auto const mc_handler = extra->fatal ? handlePrimeCacheInit<true>
                                       : handlePrimeCacheInit<false>;

  auto const fast_path = v.makeBlock();
  auto const slow_path = v.makeBlock();
  auto const done = v.makeBlock();

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
  v << jcc{CC_NE, sf, {fast_path, slow_path}};

  // Inline cache hit; store the value in the AR.
  v = fast_path;
  auto funcptr = v.makeReg();
  v << shrqi{32, func_class, funcptr, v.makeReg()};
  v << store{funcptr, fp[cellsToBytes(extra->offset.offset) + AROFF(m_func)]};
  v << jmp{done};

  // Initialize the inline cache, or do a lookup in the out-of-line cache if
  // we've finished initialization and have smashed this call.
  v = slow_path;

  auto const args = argGroup(env, inst)
    .imm(safe_cast<int32_t>(handle))
    .addr(fp, cellsToBytes(extra->offset.offset))
    .immPtr(extra->method)
    .ssa(0 /* cls */)
    .immPtr(inst->marker().func()->cls())
    .reg(func_class);

  cgCallHelper(v, env, CallSpec::smashable(mc_handler),
               kVoidDest, SyncOptions::Sync, args);
  v << jmp{done};
  v = done;
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

namespace {

template<class FromThisFn>
void implGetCtxFwdCall(IRLS& env, const IRInstruction* inst,
                       FromThisFn ctx_from_this) {
  auto const dstCtx = dstLoc(env, inst, 0).reg();
  auto const srcCtx = srcLoc(env, inst, 0).reg();
  auto const ty = inst->src(0)->type();

  auto& v = vmain(env);

  if (ty <= TCctx) {
    v << copy{srcCtx, dstCtx};
  } else if (ty <= TObj) {
    ctx_from_this(v, srcCtx, dstCtx);
  } else {
    // If we don't know whether we have a $this, we need to check dynamically.
    auto const sf = v.makeReg();
    v << testqi{1, srcCtx, sf};
    cond(v, CC_Z, sf, dstCtx,
      [&] (Vout& v) { return ctx_from_this(v, srcCtx, v.makeReg()); },
      [&] (Vout& v) { return srcCtx; }
    );
  }
}

}

void cgGetCtxFwdCall(IRLS& env, const IRInstruction* inst) {
  implGetCtxFwdCall(env, inst, [&] (Vout& v, Vreg rthis, Vreg dst) {
    auto const callee = inst->src(1)->funcVal();

    if (callee->isStatic()) {
      // Load (this->m_cls | 0x1) into `dst'.
      auto const cls = v.makeReg();
      emitLdObjClass(v, rthis, cls);
      v << orqi{1, cls, dst, v.makeReg()};
    } else {
      // Just incref $this.
      emitIncRef(v, rthis);
      v << copy{rthis, dst};
    }
    return dst;
  });
}

void cgGetCtxFwdCallDyn(IRLS& env, const IRInstruction* inst) {
  implGetCtxFwdCall(env, inst, [&] (Vout& v, Vreg rthis, Vreg dst) {
    auto const extra = inst->extra<ClsMethodData>();
    auto const ch = StaticMethodFCache::alloc(
      extra->clsName,
      extra->methodName,
      ctxName(inst->marker())
    );

    // The StaticMethodFCache here is guaranteed to already be initialized in
    // RDS by the pre-conditions of this instruction.
    auto const sf = v.makeReg();
    v << cmplim{1, rvmtl()[ch + offsetof(StaticMethodFCache, m_static)], sf};

    return cond(v, CC_E, sf, dst,
      [&] (Vout& v) {
        // Load (this->m_cls | 0x1) into `dst'.
        auto cls = v.makeReg();
        auto tmp = v.makeReg();
        emitLdObjClass(v, rthis, cls);
        v << orqi{1, cls, tmp, v.makeReg()};
        return tmp;
      },
      [&] (Vout& v) {
        // Just incref $this.
        emitIncRef(v, rthis);
        return rthis;
      }
    );
  });
}

///////////////////////////////////////////////////////////////////////////////

}}}
