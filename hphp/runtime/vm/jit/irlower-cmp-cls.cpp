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

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/instance-bits.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

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

void cgInstanceOf(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const rhs = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const call_classof = [&] (Vreg dst) {
    cgCallHelper(v, env, CallSpec::method(&Class::classof),
                 {DestType::Byte, dst}, SyncOptions::None,
                 argGroup(env, inst).ssa(0).ssa(1));
    return dst;
  };

  if (inst->src(1)->isA(TCls)) {
    call_classof(dst);
    return;
  }

  auto const sf = v.makeReg();
  v << testq{rhs, rhs, sf};
  cond(v, CC_NZ, sf, dst,
    [&] (Vout& v) { return call_classof(v.makeReg()); },
    [&] (Vout& v) { return v.cns(false); } // rhs is nullptr
  );
}

IMPL_OPCODE_CALL(InstanceOfIface)

void cgInstanceOfIfaceVtable(IRLS& env, const IRInstruction* inst) {
  auto const iface = inst->extra<InstanceOfIfaceVtable>()->cls;
  auto const slot = iface->preClass()->ifaceVtableSlot();

  auto const dst = dstLoc(env, inst, 0).reg();
  auto const rcls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  emitCmpVecLen(v, sf, static_cast<int32_t>(slot),
                rcls[Class::vtableVecLenOff()]);
  cond(
    v, CC_A, sf, dst,
    [&] (Vout& v) {
      auto const vtableVec = v.makeReg();
      emitLdLowPtr(v, rcls[Class::vtableVecOff()], vtableVec,
                   sizeof(LowPtr<Class::VtableVecSlot>));

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

  // Check whether `lhs' is a subclass of `rhsCls', given that its classvec is
  // at least as long as rhsCls's.
  auto check_clsvec = [&] (Vout& v, Vreg d) {
    // If it's a subclass, rhsCls must be at the appropriate index.
    auto const vecOffset = Class::classVecOff() +
                           sizeof(LowPtr<Class>) * (rhsCls->classVecLen() - 1);
    auto const sf = v.makeReg();
    emitCmpLowPtr(v, sf, rhsCls, lhs[vecOffset]);
    v << setcc{CC_E, sf, d};
    return d;
  };

  // Check whether `lhs' points to a subclass of rhsCls, set `d' with the
  // boolean result, and return `d'.
  auto check_subclass = [&](Vreg d) {
    if (rhsCls->classVecLen() == 1) {
      // Every class has at least one entry in its class vec, so there's no
      // need to check the length.
      return check_clsvec(v, d);
    }
    assertx(rhsCls->classVecLen() > 1);

    // Check the length of the class vectors.  If the candidate's is at least
    // as long as the potential base (`rhsCls'), it might be a subclass.
    auto const sf = v.makeReg();
    emitCmpVecLen(v, sf, static_cast<int32_t>(rhsCls->classVecLen()),
                  lhs[Class::classVecLenOff()]);
    return cond(
      v, CC_NB, sf, d,
      [&] (Vout& v) { return check_clsvec(v, v.makeReg()); },
      [&] (Vout& v) { return v.cns(false); }
    );
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

IMPL_OPCODE_CALL(ProfileInstanceCheck)

IMPL_OPCODE_CALL(InterfaceSupportsArr)
IMPL_OPCODE_CALL(InterfaceSupportsVec)
IMPL_OPCODE_CALL(InterfaceSupportsDict)
IMPL_OPCODE_CALL(InterfaceSupportsKeyset)
IMPL_OPCODE_CALL(InterfaceSupportsStr)
IMPL_OPCODE_CALL(InterfaceSupportsInt)
IMPL_OPCODE_CALL(InterfaceSupportsDbl)

///////////////////////////////////////////////////////////////////////////////

}}}
