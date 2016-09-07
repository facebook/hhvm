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

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/stats.h"

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
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-specialization.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

#include <folly/Format.h>
#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

template<class Then>
void ifNonPersistent(Vout& v, Type ty, Vloc loc, Then then) {
  if (!ty.maybe(TPersistent)) {
    then(v); // non-persistent check below will always succeed
    return;
  }

  auto const sf = v.makeReg();
  v << cmplim{0, loc.reg()[FAST_REFCOUNT_OFFSET], sf};
  static_assert(UncountedValue < 0 && StaticValue < 0, "");
  ifThen(v, CC_GE, sf, then);
}

template<class Then>
void ifRefCountedType(Vout& v, Vout& vtaken, Type ty, Vloc loc, Then then) {
  if (!ty.maybe(TCounted)) return;
  if (ty <= TStkElem && ty.isKnownDataType()) {
    if (isRefcountedType(ty.toDataType())) then(v);
    return;
  }
  auto const sf = v.makeReg();
  auto cond = CC_NLE;
  if (ty <= TCtx) {
    v << testqi{ActRec::kHasClassBit, loc.reg(0), sf};
    cond = CC_E;
  } else {
    assert(ty <= TStkElem);
    emitCmpTVType(v, sf, KindOfRefCountThreshold, loc.reg(1));
  }
  unlikelyIfThen(v, vtaken, cond, sf, then);
}

template<class Then>
void ifRefCountedNonPersistent(Vout& v, Type ty, Vloc loc, Then then) {
  ifRefCountedType(v, v, ty, loc, [&] (Vout& v) {
    ifNonPersistent(v, ty, loc, then);
  });
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace {

struct IncRefProfile {
  std::string toString() const {
    return folly::sformat("tryinc: {:4}", tryinc);
  }

  static void reduce(IncRefProfile& a, const IncRefProfile& b) {
    a.tryinc += b.tryinc;
  }

  /*
   * The number of times this IncRef made it at least as far as the static
   * check (meaning it was given a refcounted DataType).
   */
  uint16_t tryinc;
};

}

void cgIncRef(IRLS& env, const IRInstruction* inst) {
  // This is redundant with a check in ifRefCountedNonPersistent, but we check
  // anyway to avoid emitting profiling code in this case.
  auto const ty = inst->src(0)->type();
  if (!ty.maybe(TCounted)) return;

  auto const loc = srcLoc(env, inst, 0);
  auto& v = vmain(env);

  if (ty.maybe(TCctx)) {
    always_assert(ty <= TCtx && ty.maybe(TObj));
    auto const sf = v.makeReg();
    v << testqi{ActRec::kHasClassBit, loc.reg(), sf};
    ifThen(v, CC_Z, sf, [&] (Vout& v) { emitIncRef(v, loc.reg()); });
    return;
  }

  folly::Optional<rds::Handle> profHandle;
  auto vtaken = &v;

  // We profile generic IncRefs to see which ones are unlikely to see
  // refcounted values.
  if (RuntimeOption::EvalHHIROutlineGenericIncDecRef &&
      !ty.isKnownDataType()) {
    auto const profileKey = makeStaticString(
      folly::to<std::string>("IncRefProfile-", ty.toString())
    );
    auto const profile = TargetProfile<IncRefProfile> {
      env.unit.context(), inst->marker(), profileKey
    };

    if (profile.profiling()) {
      profHandle = profile.handle();
    } else if (profile.optimizing()) {
      auto const data = profile.data(IncRefProfile::reduce);
      if (data.tryinc == 0) {
        FTRACE(3, "irlower-inc-dec: Emitting cold IncRef for {}, {}\n",
               data, *inst);
        vtaken = &vcold(env);
      }
    }
  }

  ifRefCountedType(v, *vtaken, ty, loc, [&] (Vout& v) {
    if (profHandle) {
      v << incwm{rvmtl()[*profHandle + offsetof(IncRefProfile, tryinc)],
                 v.makeReg()};
    }
    ifNonPersistent(v, ty, loc, [&] (Vout& v) {
      emitIncRef(v, loc.reg());
    });
  });
}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Profile for the type and release-frequency of a DecRef instruction.
 */
struct DecRefProfile {
  float destroyRate() const {
    return hits ? float(destroy) / hits : 0.0;
  }

  std::string toString() const {
    return folly::sformat("hits: {:4} trydec: {:4}, destroy: {:4} ({:.2%}%)",
                          hits, trydec, destroy, destroyRate());
  }

  static void reduce(DecRefProfile& a, const DecRefProfile& b) {
    a.hits    += b.hits;
    a.trydec  += b.trydec;
    a.destroy += b.destroy;
  }

  /*
   * The number of times this DecRef was executed.
   */
  uint16_t hits;
  /*
   * The number of times this DecRef made it at least as far as the static
   * check (meaning it was given a refcounted DataType).
   */
  uint16_t trydec;
  /*
   * The number of times this DecRef went to zero and called destroy().
   */
  uint16_t destroy;
};

using OptDecRefProfile = folly::Optional<TargetProfile<DecRefProfile>>;

/*
 * Depending on the current translation kind, do nothing, profile, or collect
 * profiling data for the current DecRef instruction.
 *
 * Returns the frequency that the DecRef results in destruction.
 */
float decRefDestroyRate(Vout& v, IRLS& env, const IRInstruction* inst,
                        OptDecRefProfile& profile) {
  auto const kind = env.unit.context().kind;

  // Without profiling data, we assume destroy is unlikely.
  if (kind != TransKind::Profile &&
      kind != TransKind::Optimize) return 0.0;

  auto const profileKey = makeStaticString(folly::to<std::string>(
    "DecRefProfile-",
    opcodeName(inst->op()), '-',
    inst->extra<DecRef>()->locId
  ));
  profile.emplace(env.unit.context(), inst->marker(), profileKey);

  if (profile->profiling()) {
    v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, hits)],
               v.makeReg()};
  } else if (profile->optimizing()) {
    auto const data = profile->data(DecRefProfile::reduce);

    if (data.destroyRate() == 0.0) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_0);
    } else if (data.destroyRate() == 1.0) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_100);
    } else {
      // Those are the only interesting cases where we could be doing better.
      FTRACE(5, "irlower-inc-dec: DecRefProfile<{}, {}>: {}\n",
             inst->marker().show(), profileKey->data(), data);
    }
    return data.destroyRate();
  }

  return 0.0;
}

CallSpec getDtorCallSpec(DataType type) {
  switch (type) {
    case KindOfString:
      return CallSpec::method(&StringData::release);
    case KindOfArray:
      return CallSpec::method(&ArrayData::release);
    case KindOfVec:
      return CallSpec::direct(PackedArray::Release);
    case KindOfDict:
      return CallSpec::direct(MixedArray::Release);
    case KindOfKeyset:
      return CallSpec::direct(SetArray::Release);
    case KindOfObject:
      return CallSpec::method(
        RuntimeOption::EnableObjDestructCall
          ? &ObjectData::release
          : &ObjectData::releaseNoObjDestructCheck
      );
    case KindOfResource:
      return CallSpec::method(&ResourceHdr::release);
    case KindOfRef:
      return CallSpec::method(&RefData::release);
    DT_UNCOUNTED_CASE:
    case KindOfClass:
      break;
  }
  always_assert(false);
}

CallSpec makeDtorCall(Type ty, Vloc loc, ArgGroup& args) {
  static auto const TPackedArr = Type::Array(ArrayData::kPackedKind);
  static auto const TMixedArr = Type::Array(ArrayData::kMixedKind);
  static auto const TAPCArr = Type::Array(ArrayData::kApcKind);

  if (ty <= TPackedArr) return CallSpec::direct(PackedArray::Release);
  if (ty <= TMixedArr)  return CallSpec::direct(MixedArray::Release);
  if (ty <= TAPCArr)    return CallSpec::direct(APCLocalArray::Release);
  if (ty <= TArr)       return CallSpec::array(&g_array_funcs.release);

  if (ty <= TObj && ty.clsSpec().cls()) {
    auto const cls = ty.clsSpec().cls();

    // These conditions must match the ones which cause us to call
    // cls->instanceDtor() in ObjectData::release().
    if ((cls->attrs() & AttrNoOverride) &&
        !cls->getDtor() &&
        cls->instanceDtor()) {
      args.immPtr(cls);
      return CallSpec::direct(cls->instanceDtor().get());
    }
  }

  return ty.isKnownDataType() ? getDtorCallSpec(ty.toDataType())
                              : CallSpec::destruct(loc.reg(1));
}

/*
 * We've tried a variety of tweaks to this and found the current state of
 * things to be optimal, at least when measurements of the following factors
 * were made:
 *
 * - whether to load the count into a register
 *
 * - whether to use `if (!--count) release();' if we don't need a static check
 *
 * - whether to skip using the register and just emit --count if we know
 *   its not static, and can't hit zero.
 *
 * The current scheme generates `if (!--count) release();' for types that
 * cannot possibly be static.  For types that might be static, it generates a
 * compare of the m_count field against 1, followed by two conditional branches
 * on the same flags.  We make use of the invariant that count fields are never
 * zero, and use a code sequence that looks like this:
 *
 *    cmpl $1, $FAST_REFCOUNT_OFFSET(%base)
 *    je do_release  // call the destructor, usually in cold code
 *    jl skip_dec    // count < 1 implies it's static
 *    decl $FAST_REFCOUNT_OFFSET(%base)
 *  skip_dec:
 *    // ....
 */
void implDecRef(Vout& v, IRLS& env,
                const IRInstruction* inst, Type ty,
                const OptDecRefProfile& profile, bool unlikelyDestroy) {
  auto const base = srcLoc(env, inst, 0).reg(0);

  auto const destroy = [&] (Vout& v) {
    emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Destroy
                                   : Stats::TC_DecRef_Likely_Destroy);
    if (profile && profile->profiling()) {
      v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, destroy)],
                 v.makeReg()};
    }
    auto args = argGroup(env, inst).reg(base);
    auto const dtor = makeDtorCall(ty, srcLoc(env, inst, 0), args);
    cgCallHelper(v, env, dtor, kVoidDest, SyncOptions::Sync, args);
  };

  emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Decl
                                 : Stats::TC_DecRef_Likely_Decl);
  if (profile && profile->profiling()) {
    v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, trydec)],
               v.makeReg()};
  }

  if (!ty.maybe(TPersistent)) {
    auto const sf = emitDecRef(v, base);
    ifThen(v, vcold(env), CC_E, sf, destroy, unlikelyDestroy);
    return;
  }

  emitDecRefWork(v, vcold(env), base, destroy, unlikelyDestroy);
}

void emitDecRefTypeStat(Vout& v, IRLS& env, const IRInstruction* inst) {
  if (!Trace::moduleEnabled(Trace::decreftype)) return;

  auto category = makeStaticString(inst->is(DecRef) ? "DecRef" : "DecRefNZ");
  auto key = makeStaticString(inst->src(0)->type().unspecialize().toString());

  auto const args = argGroup(env, inst)
    .immPtr(category)
    .immPtr(key)
    .imm(1);

  cgCallHelper(v, env, CallSpec::direct(Stats::incStatGrouped),
               kVoidDest, SyncOptions::None, args);
}

}

void cgDecRef(IRLS& env, const IRInstruction *inst) {
  // This is redundant with a check in ifRefCounted, but we check anyway to
  // avoid emitting profiling code in this case.
  auto const ty = inst->src(0)->type();
  if (!ty.maybe(TCounted)) return;

  auto& v = vmain(env);
  OptDecRefProfile profile;

  emitDecRefTypeStat(v, env, inst);

  auto const destroyRate = decRefDestroyRate(v, env, inst, profile);
  FTRACE(3, "irlower-inc-dec: destroyPercent {:.2%} for {}\n",
         destroyRate, *inst);

  auto impl = [&] (Vout& v, Type t) {
    implDecRef(
      v, env, inst, t, profile,
      destroyRate * 100 < RuntimeOption::EvalJitUnlikelyDecRefPercent);
  };

  if (ty.maybe(TCctx)) {
    always_assert(ty <= TCtx && ty.maybe(TObj));
    auto const loc = srcLoc(env, inst, 0);
    auto const sf = v.makeReg();
    v << testqi{ActRec::kHasClassBit, loc.reg(), sf};
    ifThen(v, CC_Z, sf, [&] (Vout& v) {
        impl(v, TObj);
      });
    return;
  }

  if (RuntimeOption::EvalHHIROutlineGenericIncDecRef &&
      profile &&
      profile->optimizing() &&
      !ty.isKnownDataType()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.trydec == 0) {
      // This DecRef never saw a refcounted type during profiling, so call the
      // stub in cold, keeping only the type check in main.
      FTRACE(3, "irlower-inc-dec: Emitting partially outlined DecRef "
                "for {}, {}\n", data, *inst);

      auto const data = srcLoc(env, inst, 0).reg(0);
      auto const type = srcLoc(env, inst, 0).reg(1);

      auto const sf = v.makeReg();
      emitCmpTVType(v, sf, KindOfRefCountThreshold, type);

      unlikelyIfThen(v, vcold(env), CC_NLE, sf, [&] (Vout& v) {
        auto const stub = tc::ustubs().decRefGeneric;
        v << copy2{data, type, rarg(0), rarg(1)};
        v << callfaststub{stub, makeFixup(inst->marker()), arg_regs(2)};
      });
      return;
    }
  }

  ifRefCountedType(v, v, ty, srcLoc(env, inst, 0), [&] (Vout& v) {
      impl(v, ty);
  });
}

void cgDecRefNZ(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const ty = inst->src(0)->type();

  if (ty.maybe(TCctx)) {
    always_assert(ty <= TCtx);
    if (ty.maybe(TObj)) {
      auto const loc = srcLoc(env, inst, 0);
      auto const sf = v.makeReg();
      v << testqi{ActRec::kHasClassBit, loc.reg(), sf};
      ifThen(v, CC_Z, sf, [&] (Vout& v) { emitDecRef(v, loc.reg()); });
    }
    return;
  }

  emitIncStat(v, Stats::TC_DecRef_NZ);
  emitDecRefTypeStat(v, env, inst);

  auto const src = srcLoc(env, inst, 0);

  ifRefCountedNonPersistent(v, ty, src, [&] (Vout& v) {
    emitDecRef(v, src.reg());
  });
}

///////////////////////////////////////////////////////////////////////////////

void cgDbgAssertRefCount(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0);
  auto& v = vmain(env);

  ifRefCountedType(v, v, inst->src(0)->type(), src, [&] (Vout& v) {
    emitAssertRefCount(v, src.reg());
  });
}

///////////////////////////////////////////////////////////////////////////////

}}}
