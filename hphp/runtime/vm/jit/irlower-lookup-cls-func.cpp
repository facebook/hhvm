
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

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"

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
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

#include <type_traits>

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

void implLdClass(IRLS& env, const IRInstruction* inst, Vout& v, Vreg dst,
                 const LdClsFallback fallback) {
  auto const ch = ClassCache::alloc();
  rds::recordRds(ch, sizeof(ClassCache), "ClassCache",
                 inst->marker().func()->fullName()->slice());

  auto args = argGroup(env, inst)
                .imm(ch)
                .ssa(0 /* name */)
                .imm(static_cast<uint8_t>(fallback));
  cgCallHelper(
    v,
    env,
    CallSpec::direct(ClassCache::lookup),
    callDest(dst),
    SyncOptions::Sync,
    args
  );
}

void implLdFunc(IRLS& env, const IRInstruction* inst, Vout& v, Vreg dst) {
  auto const ch = FuncCache::alloc();
  rds::recordRds(ch, sizeof(FuncCache), "FuncCache",
                 inst->marker().func()->fullName()->slice());

  auto args = argGroup(env, inst).imm(ch).ssa(0 /* name */);
  cgCallHelper(
    v,
    env,
    CallSpec::direct(FuncCache::lookup),
    callDest(dst),
    SyncOptions::Sync,
    args
  );
}

const Func* loadUnknownFuncHelper(const StringData* name,
                                  void (*raiser)(const StringData*,
                                                 const Class*)) {
  VMRegAnchor _;
  CoeffectsAutoGuard _2;
  auto const func = Func::load(name);
  if (UNLIKELY(!func)) raiser(name, nullptr);
  return func;
}

const Class* lookupCls(const StringData* sd) {
  return Class::lookup(sd);
}

void implLdOrLookupCls(IRLS& env, const IRInstruction* inst, bool lookup,
                       const LdClsFallback loadFallback) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto const fallback = [&](Vout& v, Vreg out) {
    if (!lookup) return implLdClass(env, inst, v, out, loadFallback);
    auto const args = argGroup(env, inst).ssa(0);
    cgCallHelper(v, env, CallSpec::direct(lookupCls),
                 callDest(out), SyncOptions::None, args);
  };

  if (!RO::RepoAuthoritative) return fallback(vmain(env), dst);

  auto& v = vmain(env);
  auto& vc = vcold(env);
  auto done = v.makeBlock();
  auto then = vc.makeBlock();

  auto const sf1 = v.makeReg();
  auto const sf2 = v.makeReg();
  auto const cls1 = v.makeReg();
  auto const cls2 = v.makeReg();

  auto const mask = static_cast<int8_t>(StringData::kIsSymbolMask);
  v << testbim{mask, src[StringData::isSymbolOffset()], sf1};
  fwdJcc(v, env, CC_E, sf1, then);
  if (use_lowptr) {
    auto const low = v.makeReg();
    v << loadl{src[StringData::cachedClassOffset()], low};
    v << testl{low, low, sf2};
    fwdJcc(v, env, CC_E, sf2, then);
    v << movzlq{low, cls1};
  } else {
    v << load{src[StringData::cachedClassOffset()], cls1};
    v << testq{cls1, cls1, sf2};
    fwdJcc(v, env, CC_E, sf2, then);
  }
  v << phijmp{done, v.makeTuple({cls1})};

  vc = then;
  fallback(vc, cls2);
  vc << phijmp{done, vc.makeTuple({cls2})};

  v = done;
  v << phidef{v.makeTuple({dst})};
}

}

void cgLdCls(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdClsFallbackData>();
  implLdOrLookupCls(env, inst, false, extra->fallback);
}

void cgLookupClsRDS(IRLS& env, const IRInstruction* inst) {
  implLdOrLookupCls(env, inst, true, LdClsFallback::Fatal /* unused */);
}

void cgLdFunc(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  implLdFunc(env, inst, vmain(env), dst);
}

///////////////////////////////////////////////////////////////////////////////

const Class* autoloadKnownPersistentType(rds::Handle h,
                                         const StringData* name,
                                         const LdClsFallback fallback) {
  assertx(rds::isPersistentHandle(h));
  AutoloadHandler::s_instance->autoloadType(
    StrNR(const_cast<StringData*>(name))
  );
  auto const ptr =
    rds::handleToRef<LowPtr<Class>, rds::Mode::Persistent>(h).get();
  // Autoloader should have inited it as a side-effect.
  if (UNLIKELY(!ptr)) {
    ClassCache::loadFail(name, fallback);
  }
  return ptr;
}

const Class* lookupKnownType(rds::Handle cache_handle,
                             const StringData* name,
                             const LdClsFallback fallback) {
  assertx(rds::isNormalHandle(cache_handle));
  // The caller should already have checked.
  assertx(!rds::isHandleInit(cache_handle));

  AutoloadHandler::s_instance->autoloadType(
    StrNR(const_cast<StringData*>(name))
  );

  // Autoloader should have inited it as a side-effect.
  if (UNLIKELY(!rds::isHandleInit(cache_handle, rds::NormalTag{}))) {
    ClassCache::loadFail(name, fallback);
  }
  return rds::handleToRef<LowPtr<Class>, rds::Mode::Normal>(cache_handle).get();
}

const Func* loadUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_call_to_undefined);
}

const Func* lookupUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_resolve_func_undefined);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<class T> rds::Handle handleFrom(const StringData* name);

template<>
rds::Handle handleFrom<Func>(const StringData* name) {
  auto ne = NamedFunc::getOrCreate(name);
  return ne->getFuncHandle(name);
}

template<>
rds::Handle handleFrom<Class>(const StringData* name) {
  auto ne = NamedType::getOrCreate(name);
  return ne->getClassHandle(name);
}

template<class T, class SlowPath>
void implLdCached(IRLS& env, const IRInstruction* inst,
                  const StringData* name, SlowPath fill_cache) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = handleFrom<T>(name);
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(env), CC_NE, sf, dst,
      [&] (Vout& v) { return fill_cache(v, ch); },
      [&] (Vout& v) {
        markRDSAccess(v, ch);
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<T>));
        return ptr;
      }
    );
  } else {
    auto const pptr = rds::handleToPtr<LowPtr<T>, rds::Mode::Persistent>(ch);
    markRDSAccess(v, ch);
    auto const ptr = v.makeReg();
    emitLdLowPtr(v, *v.cns(pptr), ptr, sizeof(LowPtr<T>));

    auto const sf = v.makeReg();
    v << testq{ptr, ptr, sf};
    unlikelyCond(v, vcold(env), CC_Z, sf, dst,
                 [&](Vout& v) { return fill_cache(v, ch); },
                 [&](Vout& /*v*/) { return ptr; });
  }
}

template<Opcode opc>
void ldFuncCachedHelper(IRLS& env, const IRInstruction* inst,
                        const CallSpec& call) {
  auto const extra = inst->extra<opc>();

  implLdCached<Func>(env, inst, extra->name, [&] (Vout& v, rds::Handle) {
    auto const ptr = v.makeReg();
    auto const args = argGroup(env, inst).immPtr(extra->name);
    cgCallHelper(v, env, call, callDest(ptr), SyncOptions::Sync, args);
    return ptr;
  });
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLdClsCached(IRLS& env, const IRInstruction* inst) {
  auto const name = inst->src(0)->strVal();
  auto const extra = inst->extra<LdClsFallbackData>();

  implLdCached<Class>(env, inst, name, [&] (Vout& v, rds::Handle ch) {
    auto const ptr = v.makeReg();
    auto const target = rds::isPersistentHandle(ch)
                        ? autoloadKnownPersistentType
                        : lookupKnownType;
    auto const args = argGroup(env, inst)
                        .imm(ch)
                        .ssa(0)
                        .imm(static_cast<uint8_t>(extra->fallback));
    cgCallHelper(v, env, CallSpec::direct(target),
                 callDest(ptr), SyncOptions::Sync, args);
    return ptr;
  });
}

void cgLdFuncCached(IRLS& env, const IRInstruction* inst) {
  ldFuncCachedHelper<LdFuncCached>(
    env, inst, CallSpec::direct(loadUnknownFunc)
  );
}

void cgLookupFuncCached(IRLS& env, const IRInstruction* inst) {
  ldFuncCachedHelper<LookupFuncCached>(
    env, inst, CallSpec::direct(lookupUnknownFunc)
  );
}

void cgLdClsCachedSafe(IRLS& env, const IRInstruction* inst) {
  auto const name = inst->src(0)->strVal();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = handleFrom<Class>(name);
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    fwdJcc(v, env, CC_NE, sf, inst->taken());
    markRDSAccess(v, ch);
    emitLdLowPtr(v, rvmtl()[ch], dst, sizeof(LowPtr<Class>));
  } else {
    assertx(rds::isPersistentHandle(ch));
    auto const pptr =
      rds::handleToPtr<LowPtr<Class>, rds::Mode::Persistent>(ch);
    markRDSAccess(v, ch);
    emitLdLowPtr(v, *v.cns(pptr), dst, sizeof(LowPtr<Class>));
  }
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(OODeclExists)

///////////////////////////////////////////////////////////////////////////////

}
