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
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

void cgLdFunc(IRLS& env, const IRInstruction* inst) {
  auto const ch = FuncCache::alloc();
  rds::recordRds(ch, sizeof(FuncCache), "FuncCache",
                 inst->marker().func()->fullName()->data());

  // Raises an error if the function is not found.
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(FuncCache::lookup),
    callDest(dstLoc(env, inst, 0).reg()),
    SyncOptions::Sync,
    argGroup(env, inst).imm(ch).ssa(0 /* methodName */)
  );
}

///////////////////////////////////////////////////////////////////////////////

namespace {

const Func* lookupUnknownFunc(const StringData* name) {
  VMRegAnchor _;
  auto const func = Unit::loadFunc(name);
  if (UNLIKELY(!func)) {
    raise_error("Call to undefined function %s()", name->data());
  }
  return func;
}

const Func* lookupFallbackFunc(const StringData* name,
                               const StringData* fallback) {
  VMRegAnchor _;

  // Try to load the first function.
  auto func = Unit::loadFunc(name);
  if (LIKELY(!func)) {
    // Then try to load the fallback function.
    func = Unit::loadFunc(fallback);
    if (UNLIKELY(!func)) {
      raise_error("Call to undefined function %s()", name->data());
    }
  }
  return func;
}

template<class Extra, class SlowPath>
void implLdFuncCached(IRLS& env, const IRInstruction* inst,
                      const Extra& extra, SlowPath fill_cache) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(env), CC_NE, sf, dst,
      [&] (Vout& v) { return fill_cache(v); },
      [&] (Vout& v) {
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));
        return ptr;
      }
    );
  } else {
    auto const ptr = v.makeReg();
    emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));

    auto const sf = v.makeReg();
    v << testq{ptr, ptr, sf};
    unlikelyCond(
      v, vcold(env), CC_Z, sf, dst,
      [&] (Vout& v) { return fill_cache(v); },
      [&] (Vout& v) { return ptr; }
    );
  }
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLdFuncCached(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdFuncCached>();

  implLdFuncCached(env, inst, extra, [&] (Vout& v) {
    auto const ptr = v.makeReg();
    auto const args = argGroup(env, inst).immPtr(extra->name);
    cgCallHelper(v, env, CallSpec::direct(lookupUnknownFunc),
                 callDest(ptr), SyncOptions::Sync, args);
    return ptr;
  });
}

void cgLdFuncCachedU(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdFuncCachedU>();

  implLdFuncCached(env, inst, extra, [&] (Vout& v) {
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

void cgLdFuncCachedSafe(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdFuncCachedSafe>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const ch = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain(env);

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    fwdJcc(v, env, CC_NE, sf, inst->taken());
  }
  emitLdLowPtr(v, rvmtl()[ch], dst, sizeof(LowPtr<Func>));
}

///////////////////////////////////////////////////////////////////////////////

}}}
