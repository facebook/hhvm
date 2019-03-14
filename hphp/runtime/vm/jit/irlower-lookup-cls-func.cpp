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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
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

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

template<class TargetCache>
void implLdMeta(IRLS& env, const IRInstruction* inst) {
  auto const is_func = std::is_same<TargetCache,FuncCache>::value;

  auto const ch = TargetCache::alloc();
  rds::recordRds(ch, sizeof(TargetCache), is_func ? "FuncCache" : "ClassCache",
                 inst->marker().func()->fullName()->slice());

  auto args = argGroup(env, inst).imm(ch).ssa(0 /* name */);
  if (is_func) {
    args
      .addr(srcLoc(env, inst, 1).reg(),
            cellsToBytes(inst->extra<LdFunc>()->offset.offset))
      .ssa(2);
  }
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(TargetCache::lookup),
    callDest(env, inst),
    SyncOptions::Sync,
    args
  );
}

const Func* loadUnknownFuncHelper(const StringData* name,
                                  void (*raiser)(const StringData*,
                                                 const Class*)) {
  VMRegAnchor _;
  auto const func = Unit::loadFunc(name);
  if (UNLIKELY(!func)) raiser(name, nullptr);
  return func;
}

}

void cgDefCls(IRLS& env, const IRInstruction* inst) {
  auto unit = inst->marker().func()->unit();
  auto args = argGroup(env, inst)
    .immPtr(unit->lookupPreClassId(inst->src(0)->intVal())).
    imm(true);

  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(&Unit::defClass),
    callDest(env, inst),
    SyncOptions::Sync,
    args
  );
}

void cgLdCls(IRLS& env, const IRInstruction* inst) {
  implLdMeta<ClassCache>(env, inst);
}

void cgLdFunc(IRLS& env, const IRInstruction* inst) {
  implLdMeta<FuncCache>(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

const Class* lookupKnownClass(rds::Handle cache_handle,
                              const StringData* name) {
  assertx(rds::isNormalHandle(cache_handle));
  // The caller should already have checked.
  assertx(!rds::isHandleInit(cache_handle));

  AutoloadHandler::s_instance->autoloadClass(
    StrNR(const_cast<StringData*>(name))
  );

  // Autoloader should have inited it as a side-effect.
  if (UNLIKELY(!rds::isHandleInit(cache_handle, rds::NormalTag{}))) {
    raise_error(Strings::UNKNOWN_CLASS, name->data());
  }
  return rds::handleToRef<LowPtr<Class>, rds::Mode::Normal>(cache_handle).get();
}

const Func* loadUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_call_to_undefined);
}

const Func* lookupUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_resolve_undefined);
}

const Func* lookupFallbackFunc(const StringData* name,
                               const StringData* fallback) {
  VMRegAnchor _;

  // Try to load the first function.
  auto func = Unit::loadFunc(name);
  if (LIKELY(!func)) {
    // Then try to load the fallback function.
    raise_undefined_function_fallback_notice(name, fallback);
    func = Unit::loadFunc(fallback);
    if (UNLIKELY(!func)) {
      raise_error("Call to undefined function %s()",
                  stripInOutSuffix(name)->data());
    }
  }
  return func;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<class T> rds::Handle handleFrom(const NamedEntity* ne);

template<>
rds::Handle handleFrom<Func>(const NamedEntity* ne) {
  return ne->getFuncHandle();
}
template<>
rds::Handle handleFrom<Class>(const NamedEntity* ne) {
  return ne->getClassHandle();
}

template<class T, class SlowPath>
void implLdCached(IRLS& env, const IRInstruction* inst,
                  const StringData* name, SlowPath fill_cache) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = handleFrom<T>(NamedEntity::get(name));
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(env), CC_NE, sf, dst,
      [&] (Vout& v) { return fill_cache(v, ch); },
      [&] (Vout& v) {
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<T>));
        return ptr;
      }
    );
  } else {
    auto const pptr = rds::handleToPtr<LowPtr<T>, rds::Mode::Persistent>(ch);
    auto const ptr = v.makeReg();
    emitLdLowPtr(v, *v.cns(pptr), ptr, sizeof(LowPtr<T>));

    auto const sf = v.makeReg();
    v << testq{ptr, ptr, sf};
    unlikelyCond(v, vcold(env), CC_Z, sf, dst,
                 [&](Vout& v) { return fill_cache(v, ch); },
                 [&](Vout& /*v*/) { return ptr; });
  }
}

template<class T>
void implLdCachedSafe(IRLS& env, const IRInstruction* inst,
                      const StringData* name) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = handleFrom<T>(NamedEntity::get(name));
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    fwdJcc(v, env, CC_NE, sf, inst->taken());
    emitLdLowPtr(v, rvmtl()[ch], dst, sizeof(LowPtr<T>));
  } else {
    assertx(rds::isPersistentHandle(ch));
    auto const pptr = rds::handleToPtr<LowPtr<T>, rds::Mode::Persistent>(ch);
    emitLdLowPtr(v, *v.cns(pptr), dst, sizeof(LowPtr<T>));
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

  implLdCached<Class>(env, inst, name, [&] (Vout& v, rds::Handle ch) {
    auto const ptr = v.makeReg();
    auto const args = argGroup(env, inst).imm(ch).ssa(0);
    cgCallHelper(v, env, CallSpec::direct(lookupKnownClass),
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

void cgLdFuncCachedU(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdFuncCachedU>();

  implLdCached<Func>(env, inst, extra->name, [&] (Vout& v, rds::Handle) {
    // If we get here, things are going to be slow anyway, so shunt all the
    // autoloading logic to lookupFallbackFunc().
    auto const ptr = v.makeReg();
    auto const args = argGroup(env, inst)
      .immPtr(extra->name)
      .immPtr(extra->fallback);

    cgCallHelper(v, env, CallSpec::direct(lookupFallbackFunc),
                 callDest(ptr), SyncOptions::Sync, args);
    return ptr;
  });
}

void cgLdClsCachedSafe(IRLS& env, const IRInstruction* inst) {
  auto const name = inst->src(0)->strVal();
  implLdCachedSafe<Class>(env, inst, name);
}

IMPL_OPCODE_CALL(LookupClsRDS)

///////////////////////////////////////////////////////////////////////////////

void loadFuncContextImpl(ArrayData* arr, ActRec* preLiveAR, ActRec* fp) {
  ObjectData* inst = nullptr;
  Class* cls = nullptr;
  StringData* invName = nullptr;
  ArrayData* reifiedGenerics = nullptr;
  bool dynamic = false;

  auto func = vm_decode_function(
    VarNR(arr).operator const Variant&(),
    fp,
    false, // forward
    inst,
    cls,
    invName,
    dynamic,
    reifiedGenerics,
    DecodeFlags::NoWarn
  );
  assertx(dynamic);
  if (UNLIKELY(func == nullptr)) {
    raise_error("Invalid callable (array)");
  }

  preLiveAR->m_func = func;
  if (inst) {
    inst->incRefCount();
    preLiveAR->setThis(inst);
  } else if (cls) {
    preLiveAR->setClass(cls);
  } else {
    preLiveAR->trashThis();
  }
  if (UNLIKELY(invName != nullptr)) {
    preLiveAR->setMagicDispatch(invName);
  }
  if (func->hasReifiedGenerics()) {
    preLiveAR->setReifiedGenerics(reifiedGenerics);
  }
}

void loadArrayFunctionContext(ArrayData* arr, ActRec* preLiveAR, ActRec* fp) {
  try {
    loadFuncContextImpl(arr, preLiveAR, fp);
  } catch (...) {
    *arPreliveOverwriteCells(preLiveAR) = make_array_like_tv(arr);
    throw;
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgLdArrFuncCtx(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst)
    .ssa(0)
    .addr(srcLoc(env, inst, 1).reg(),
          cellsToBytes(inst->extra<LdArrFuncCtx>()->offset.offset))
    .ssa(2);

  cgCallHelper(vmain(env), env, CallSpec::direct(loadArrayFunctionContext),
               callDest(env, inst), SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(OODeclExists)

///////////////////////////////////////////////////////////////////////////////

}}}
