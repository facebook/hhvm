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

template<class T>
constexpr const char* errorString();
template<>
constexpr const char* errorString<Class>() {
  return Strings::UNKNOWN_CLASS;
}
template<>
constexpr const char* errorString<RecordDesc>() {
  return Strings::UNKNOWN_RECORD;
}

template<class T>
const T* lookupKnownType(rds::Handle cache_handle,
                         const StringData* name) {
  assertx(rds::isNormalHandle(cache_handle));
  // The caller should already have checked.
  assertx(!rds::isHandleInit(cache_handle));

  AutoloadHandler::s_instance->autoloadType<T>(
    StrNR(const_cast<StringData*>(name))
  );

  // Autoloader should have inited it as a side-effect.
  if (UNLIKELY(!rds::isHandleInit(cache_handle, rds::NormalTag{}))) {
    raise_error(errorString<T>(), name->data());
  }
  return rds::handleToRef<LowPtr<T>, rds::Mode::Normal>(cache_handle).get();
}

const Func* loadUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_call_to_undefined);
}

const Func* lookupUnknownFunc(const StringData* name) {
  return loadUnknownFuncHelper(name, raise_resolve_undefined);
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
template<>
rds::Handle handleFrom<RecordDesc>(const NamedEntity* ne) {
  return ne->getRecordDescHandle();
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
    cgCallHelper(v, env, CallSpec::direct(lookupKnownType<Class>),
                 callDest(ptr), SyncOptions::Sync, args);
    return ptr;
  });
}

void cgLdRecDescCached(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdRecDescCached>();

  implLdCached<RecordDesc>(env, inst, extra->recName,
                           [&] (Vout& v, rds::Handle ch) {
    auto const ptr = v.makeReg();
    auto const args = argGroup(env, inst).imm(ch).immPtr(extra->recName);
    cgCallHelper(v, env, CallSpec::direct(lookupKnownType<RecordDesc>),
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
  implLdCachedSafe<Class>(env, inst, name);
}

void cgLdRecDescCachedSafe(IRLS& env, const IRInstruction* inst) {
  auto const name = inst->extra<LdRecDescCachedSafe>()->recName;
  implLdCachedSafe<RecordDesc>(env, inst, name);
}

IMPL_OPCODE_CALL(LookupClsRDS)

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(OODeclExists)

///////////////////////////////////////////////////////////////////////////////

}}}
