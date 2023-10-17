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

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/instance-bits.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"
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
#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgEqCls(IRLS& env, const IRInstruction* inst) {
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const src1 = srcLoc(env, inst, 0).reg();
  auto const src2 = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  emitCmpLowPtr<Class>(v, sf, src2, src1);
  v << setcc{CC_E, sf, dst};
}

void cgEqLazyCls(IRLS& env, const IRInstruction* inst) {
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto const src1 = srcLoc(env, inst, 0).reg();
  auto const src2 = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpq{src2, src1, sf};
  v << setcc{CC_E, sf, dst};
}

// Check whether `lhs' is a subclass of `rhs', given that its classvec is
// at least as long as rhs's.
template <typename Cls, typename Len>
Vreg check_clsvec(Vout& v, Vreg d, Vreg lhs, Cls rhs, Len rhsVecLen) {
  // If it's a subclass, rhs must be at the appropriate index.
  auto const vecOffset = rhsVecLen * static_cast<int>(sizeof(LowPtr<Class>)) +
    (Class::classVecOff() - sizeof(LowPtr<Class>));
  auto const sf = v.makeReg();
  emitCmpLowPtr<Class>(v, sf, rhs, lhs[vecOffset]);
  v << setcc{CC_E, sf, d};
  return d;
}

// Check whether `lhs' is a subclass of `rhs', given that sf is the result
// of comparing their classVecLens
template <typename Cls, typename Len>
Vreg check_subcls(Vout& v, Vreg sf, Vreg d, Vreg lhs, Cls rhs, Len rhsVecLen) {
  return cond(v, CC_NB, sf, d,
       [&] (Vout& v) {
         return check_clsvec(v, v.makeReg(), lhs, rhs, rhsVecLen);
       },
       [&] (Vout& v) { return v.cns(false); }
      );
}

void emitClassofNonIFace(Vout& v, Vreg lhs, Vreg rhs, Vreg dst) {
  // This essentially inlines Class::classofNonIFace
  auto const rhsTmp = v.makeReg();
  auto const rhsLen = v.makeReg();
  auto const sf = v.makeReg();
  static_assert(sizeof(Class::classVecLen_t) == 2);
  v << loadw{rhs[Class::classVecLenOff()], rhsTmp};
  v << movzwq{rhsTmp, rhsLen};
  v << cmpwm{rhsTmp, lhs[Class::classVecLenOff()], sf};
  check_subcls(v, sf, dst, lhs, rhs, rhsLen);
}

void cgInstanceOf(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const rhs = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const call_classof = [&] (Vreg dest) {
    cgCallHelper(v, env,
                 CallSpec::method(&Class::classof), {DestType::Byte, dest},
                 SyncOptions::None, argGroup(env, inst).ssa(0).ssa(1));
    return dest;
  };

  if (!inst->src(1)->isA(TCls)) {
    auto const sf = v.makeReg();
    v << testq{rhs, rhs, sf};
    cond(v, CC_NZ, sf, dst,
         [&] (Vout& v) { return call_classof(v.makeReg()); },
         [&] (Vout& v) { return v.cns(false); } // rhs is nullptr
        );
    return;
  }

  auto const spec = inst->src(1)->type().clsSpec();
  if (!spec.cls() || (spec.cls()->attrs() & AttrInterface)) {
    call_classof(dst);
    return;
  }

  auto const lhs = srcLoc(env, inst, 0).reg();
  emitClassofNonIFace(v, lhs, rhs, dst);
}

IMPL_OPCODE_CALL(InstanceOfIface)

void cgInstanceOfIfaceVtable(IRLS& env, const IRInstruction* inst) {
  auto const iface = inst->extra<InstanceOfIfaceVtable>()->cls;
  auto const slot = iface->preClass()->ifaceVtableSlot();

  auto const dst = dstLoc(env, inst, 0).reg();
  auto const rcls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  static_assert(sizeof(Class::vtableVecLen_t) == 2);
  v << cmpwim{static_cast<int32_t>(slot), rcls[Class::vtableVecLenOff()], sf};

  cond(
    v, CC_A, sf, dst,
    [&] (Vout& v) {
      auto const vtableVec = v.makeReg();
      v << load{rcls[Class::vtableVecOff()], vtableVec};
      auto const ifaceOff = slot * sizeof(Class::VtableVecSlot) +
                            offsetof(Class::VtableVecSlot, iface);
      auto const sf = v.makeReg();
      emitCmpLowPtr<Class>(v, sf, iface, vtableVec[ifaceOff]);

      auto tmp = v.makeReg();
      v << setcc{CC_E, sf, tmp};
      return tmp;
    },
    [&] (Vout& v) { return v.cns(false); }
  );
}

/*
 * Check instanceof using the superclass vector on the end of the Class entry.
 */
void cgExtendsClass(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ExtendsClassData>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const lhs = srcLoc(env, inst, 0).reg();
  auto const rhsCls = extra->cls;
  auto& v = vmain(env);

  assertx(rhsCls != nullptr);

  // Check whether `lhs' points to a subclass of rhsCls, set `d' with the
  // boolean result, and return `d'.
  auto check_subclass = [&](Vreg d) {
    if (rhsCls->classVecLen() == 1) {
      // Every class has at least one entry in its class vec, so there's no
      // need to check the length.
      return check_clsvec(v, d, lhs, rhsCls, 1);
    }
    assertx(rhsCls->classVecLen() > 1);

    // Check the length of the class vectors.  If the candidate's is at least
    // as long as the potential base (`rhsCls'), it might be a subclass.
    auto const sf = v.makeReg();
    static_assert(sizeof(Class::classVecLen_t) == 2);
    v << cmpwim{static_cast<int32_t>(rhsCls->classVecLen()),
                lhs[Class::classVecLenOff()], sf};
    return check_subcls(v, sf, d, lhs, rhsCls, rhsCls->classVecLen());
  };

  if (rhsCls->attrs() & AttrAbstract ||
      (extra->strictLikely && !(rhsCls->attrs() & AttrNoOverride))) {
    // If the test must be extended, or the hint says it's probably not an
    // exact match, don't check for the same class.
    check_subclass(dst);
    return;
  }

  // Test if it is the exact same class.
  // TODO(#2044801): We should be doing this control flow at the IR level.
  auto const sf = v.makeReg();
  emitCmpLowPtr<Class>(v, sf, v.cns(rhsCls), lhs);

  if (rhsCls->attrs() & AttrNoOverride) {
    // If the test class cannot be extended, we only need to do the same-class
    // check, never the subclass check.
    v << setcc{CC_E, sf, dst};
    return;
  }

  cond(
    v, CC_E, sf, dst,
    [&] (Vout& v) { return v.cns(true); },
    [&] (Vout& v) { return check_subclass(v.makeReg()); }
  );
}

/*
 * Check instanceof using instance bitmasks.
 *
 * Note that it's not necessary to check whether the test class is defined; if
 * it doesn't exist than the candidate can't be an instance of it and will fail
 * this check.
 */
static void implInstanceOfBitmask(IRLS& env, const IRInstruction* inst,
                                ConditionCode cc) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const lhs = srcLoc(env, inst, 0).reg();
  auto const rhsName = inst->src(1)->strVal();
  auto& v = vmain(env);

  int offset;
  uint8_t mask;
  if (!InstanceBits::getMask(rhsName, offset, mask)) {
    always_assert(!"cgInstanceOfBitmask had no bitmask");
  }

  auto const sf = v.makeReg();
  v << testbim{int8_t(mask), lhs[offset], sf};
  v << setcc{cc, sf, dst};
}

void cgInstanceOfBitmask(IRLS& env, const IRInstruction* inst) {
  implInstanceOfBitmask(env, inst, CC_NZ);
}
void cgNInstanceOfBitmask(IRLS& env, const IRInstruction* inst) {
  implInstanceOfBitmask(env, inst, CC_Z);
}

///////////////////////////////////////////////////////////////////////////////

void cgIsTypeStructCached(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const cellTy = inst->src(1)->type();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const branch = label(env, inst->taken());

  auto const isObj = inst->src(1)->isA(TObj);

  if (!isObj && cellTy.maybe(TObj)) {
    // PUNT
    v << jmp{label(env, inst->taken())};
    return;
  }

  auto const pairSize = safe_cast<int32_t>(sizeof(TSClassCache::Pair));
  auto const hash = emitHashInt64(env, inst, arr);
  auto const offset = v.makeReg();

  static_assert(folly::isPowTwo(sizeof(TSClassCache::Pair)),
                "invalid pair size");

  v << andqi{
    ((int32_t)TSClassCache::kNumLines - 1) * pairSize,
    hash,
    offset,
    v.makeReg()
  };

  auto const ch = rds::bindTSCache(inst->func());
  markRDSAccess(v, ch.handle());
  auto const sf = v.makeReg();
  v << cmpqm{arr, offset[rvmtl() + ch.handle()], sf};
  ifThen(v, CC_NE, sf, branch);

  // Now that we know it is cached, we can return false
  if (!isObj) {
    // Definitely not object
    v << copy{v.cns(false), dst};
    return;
  }

  markRDSAccess(v, ch.handle());
  auto const rhs = v.makeReg();
  emitLdLowPtr(v, offset[rvmtl() + ch.handle() + sizeof(ArrayData*)],
               rhs, sizeof(LowPtr<const Class>));
  auto const lhs = [&] {
    assertx(isObj);
    if (auto const exact = cellTy.clsSpec().exactCls()) return v.cns(exact);
    auto const lhs = v.makeReg();
    emitLdObjClass(vmain(env), srcLoc(env, inst, 1).reg(), lhs);
    return lhs;
  }();
  emitClassofNonIFace(v, lhs, rhs, dst);
}

void cgProfileIsTypeStruct(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  auto args = argGroup(env, inst)
              .ssa(0)
              .addr(rvmtl(), safe_cast<int32_t>(extra->handle));

  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(profileIsTypeStructHelper),
    kVoidDest,
    SyncOptions::None,
    args
  );
}

void cgProfileCoeffectFunParam(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  auto args = argGroup(env, inst)
              .typedValue(0)
              .addr(rvmtl(), safe_cast<int32_t>(extra->handle));

  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(profileCoeffectFunParamHelper),
    kVoidDest,
    SyncOptions::None,
    args
  );
}

void cgLdCoeffectFunParamNaive(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdCoeffectFunParamNaive>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(CoeffectRule::getFunParam),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).typedValue(0).imm(extra->paramId)
  );
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(ProfileInstanceCheck)

IMPL_OPCODE_CALL(InterfaceSupportsArrLike)
IMPL_OPCODE_CALL(InterfaceSupportsStr)
IMPL_OPCODE_CALL(InterfaceSupportsInt)
IMPL_OPCODE_CALL(InterfaceSupportsDbl)

IMPL_OPCODE_CALL(IsTypeStruct)
IMPL_OPCODE_CALL(IsTypeStructShallow)

///////////////////////////////////////////////////////////////////////////////

}
