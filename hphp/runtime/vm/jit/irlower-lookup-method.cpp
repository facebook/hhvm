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
#include "hphp/runtime/vm/interp-helpers.h"
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

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

/*
 * The `mcprep' instruction here creates a smashable move, which serves as
 * the inline cache, or "prime cache" for the method lookup.
 *
 * On our first time through this codepath in the TC, we "prime" this cache
 * (which holds across /all/ requests) by smashing the mov immediate to hold
 * a Func* in the upper 32 bits, and a Class* in the lower 32 bits.  This is
 * not always possible (see MethodCache::handleStaticCall() for details), in
 * which case we smash an immediate with some low bits set, so that we always
 * miss on the inline cache when comparing against our live Class*.
 *
 * The inline cache is set up so that we always miss initially, and take the
 * slow path to initialize it. After initialization, the slow path uses the
 * out-of-line method cache (allocated above). The inline cache is set only
 * during the first call(s) (usually once), but the one-way request-local
 * method cache is updated on each miss.
 */
void cgLdSmashable(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << mcprep{dst};
}

void cgCheckSmashableClass(IRLS& env, const IRInstruction* inst) {
  auto const smashable = srcLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const tmp = v.makeReg();
  auto const smashableCls = v.makeReg();
  v << movtql{smashable, tmp};
  v << movzlq{tmp, smashableCls};

  auto const sf = v.makeReg();
  v << cmpq{smashableCls, cls, sf};
  v << jcc{CC_NE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgLdSmashableFunc(IRLS& env, const IRInstruction* inst) {
  auto const smashable = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  v << shrqi{32, smashable, dst, v.makeReg()};
}

void cgLdObjMethodD(IRLS& env, const IRInstruction* inst) {
  assertx(inst->taken() && inst->taken()->isCatch()); // must have catch block

  auto const target = CallSpec::direct(MethodCache::handleDynamicCall);
  auto const args = argGroup(env, inst)
    .ssa(0 /* cls */)
    .ssa(1 /* methodName */)
    .immPtr(inst->extra<OptClassAndFuncData>()->cls)
    .immPtr(inst->extra<OptClassAndFuncData>()->func); // callerFunc

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgLdObjMethodS(IRLS& env, const IRInstruction* inst) {
  assertx(inst->taken() && inst->taken()->isCatch()); // must have catch block
  using namespace MethodCache;

  // Allocate the request-local one-way method cache for this lookup.
  auto const handle =
    rds::alloc<Entry, rds::Mode::Normal, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    rds::recordRds(handle, sizeof(TypedValue), "MethodCache",
                   inst->marker().func()->fullName()->slice());
  }

  auto const target = CallSpec::direct(MethodCache::handleStaticCall);
  auto const args = argGroup(env, inst)
    .ssa(0 /* cls */)
    .immPtr(inst->extra<FuncNameCtxData>()->name)
    .immPtr(inst->extra<FuncNameCtxData>()->context)
    .immPtr(inst->extra<FuncNameCtxData>()->func) // callerFunc
    .imm(safe_cast<int32_t>(handle))
    .ssa(1 /* smashable */);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(LdClsCtor)
IMPL_OPCODE_CALL(LookupClsMethod)

void cgProfileMethod(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileCallTargetData>();

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .ssa(0)
    .ssa(1)
    .immPtr(inst->marker().func());

  cgCallHelper(vmain(env), env, CallSpec::method(&MethProfile::reportMeth),
               kVoidDest, SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

StaticString s_anonymous(":anonymous:");

const StringData* ctxName(const Class* ctx) {
  return ctx ? ctx->name() : s_anonymous.get();
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLookupClsMethodCache(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const ch = StaticMethodCache::alloc(
    extra->clsName,
    extra->methodName,
    ctxName(extra->context)
  );

  if (false) { // typecheck
    const UNUSED Func* f = StaticMethodCache::lookup(
      ch,
      extra->namedType,
      extra->clsName,
      extra->methodName,
      extra->context,
      extra->callerFunc
    );
  }

  auto const args = argGroup(env, inst)
    .imm(ch)
    .immPtr(extra->namedType)
    .immPtr(extra->clsName)
    .immPtr(extra->methodName)
    .immPtr(extra->context)
    .immPtr(extra->callerFunc);

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
    ctxName(extra->context)
  );

  auto const sf = checkRDSHandleInitialized(v, ch);
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  markRDSAccess(v, ch);
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
    ctxName(extra->context)
  );
  assertx(rds::isNormalHandle(ch));

  markRDSAccess(v, ch);

  // The StaticMethodCache here is guaranteed to already be initialized in RDS
  // by the pre-conditions of this instruction.
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodCache, m_cls)],
               dst, sizeof(LowPtr<const Class>));
}

void cgLookupClsMethodFCache(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ClsMethodData>();
  auto const dst = dstLoc(env, inst, 0).reg(0);
  auto const cls = inst->src(0)->clsVal();
  auto& v = vmain(env);

  auto const ch = StaticMethodFCache::alloc(
    cls->name(),
    extra->methodName,
    ctxName(extra->context)
  );
  assertx(rds::isNormalHandle(ch));

  const Func* (*lookup)(rds::Handle, const Class*,
                        const StringData*, const Class*, const Func*) =
    StaticMethodFCache::lookup;

  auto const args = argGroup(env, inst)
   .imm(ch)
   .immPtr(cls)
   .immPtr(extra->methodName)
   .immPtr(extra->context)
   .immPtr(extra->callerFunc);

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
    ctxName(extra->context)
  );
  assertx(rds::isNormalHandle(ch));

  auto const sf = checkRDSHandleInitialized(v, ch);
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  markRDSAccess(v, ch);
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodFCache, m_func)],
               dst, sizeof(LowPtr<const Func>));
}

///////////////////////////////////////////////////////////////////////////////

}
