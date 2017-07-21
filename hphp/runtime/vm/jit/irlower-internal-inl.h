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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/object-data.h"

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-specialization.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

inline Vout& vmain(IRLS& env) { assert(env.vmain); return *env.vmain; }
inline Vout& vcold(IRLS& env) { assert(env.vcold); return *env.vcold; }

inline Vlabel label(IRLS& env, Block* b) { return env.labels[b]; }

inline Vloc srcLoc(IRLS& env, const IRInstruction* inst, unsigned i) {
  return env.locs[inst->src(i)];
}

inline Vloc dstLoc(IRLS& env, const IRInstruction* inst, unsigned i) {
  return env.locs[inst->dst(i)];
}

inline ArgGroup argGroup(IRLS& env, const IRInstruction* inst) {
  return ArgGroup(inst, env.locs);
}

inline CallDest callDest(Vreg reg0) {
  return { DestType::SSA, reg0 };
}

inline CallDest callDest(Vreg reg0, Vreg reg1) {
  return { DestType::TV, reg0, reg1 };
}

inline CallDest callDest(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;

  auto const loc = dstLoc(env, inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;
  assertx(loc.numAllocated() == 1);

  return {
    inst->dst(0)->isA(TBool) ? DestType::Byte : DestType::SSA,
    loc.reg(0)
  };
}

inline CallDest callDestTV(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;

  auto const loc = dstLoc(env, inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;

  if (loc.isFullSIMD()) {
    assertx(loc.numAllocated() == 1);
    return { DestType::SIMD, loc.reg(0) };
  }
  if (loc.numAllocated() == 2) {
    return { DestType::TV, loc.reg(0), loc.reg(1) };
  }
  assertx(loc.numAllocated() == 1);

  // Sometimes we statically know the type and only need the value.
  return { DestType::TV, loc.reg(0), InvalidReg };
}

inline CallDest callDestDbl(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;
  auto const loc = dstLoc(env, inst, 0);
  return { DestType::Dbl, loc.reg(0) };
}

///////////////////////////////////////////////////////////////////////////////

inline void fwdJcc(Vout& v, IRLS& env, ConditionCode cc,
                   Vreg sf, Block* target) {
  auto const next = v.makeBlock();
  v << jcc{cc, sf, {next, label(env, target)}};
  v = next;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * Materialize `data' into a Vreg and return it.
 */
inline Vreg materialize(Vout& v, Vptr data) {
  auto const t = v.makeReg();
  v << load{data, t};
  return t;
}
inline Vreg materialize(Vout&, Vreg data) { return data; }

/*
 * Test whether the value given by `dataSrc' has the same type specialization
 * as `type' does.
 *
 * Assumes that the DataType corresponding to `dataSrc' already matches `type'.
 */
template <class Loc, class JmpFn>
void emitSpecializedTypeTest(Vout& v, IRLS& /*env*/, Type type, Loc dataSrc,
                             Vreg sf, JmpFn doJcc) {
  if (type < TRes) {
    // No cls field in Resource.
    always_assert(false && "unexpected guard on specialized Resource");
  }

  if (type < TObj || type < TCls) {
    // Emit the specific class test.
    assertx(type.clsSpec());
    assertx(type.clsSpec().exact() ||
            type.clsSpec().cls()->attrs() & AttrNoOverride);

    auto const data = materialize(v, dataSrc);
    if (type < TObj) {
      emitCmpLowPtr(v, sf, type.clsSpec().cls(),
                    data[ObjectData::getVMClassOffset()]);
    } else {
      v << cmpq{v.cns(type.clsSpec().cls()), data, sf};
    }
    doJcc(CC_E, sf);
  } else {
    assertx(type < TArr && type.arrSpec() && type.arrSpec().kind());
    assertx(type.arrSpec().type() == nullptr);

    auto const arrSpec = type.arrSpec();
    auto const data = materialize(v, dataSrc);

    static_assert(sizeof(HeaderKind) == 1, "");
    v << cmpbim{*arrSpec.kind(), data[HeaderKindOffset], sf};
    doJcc(CC_E, sf);
  }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

template<class Loc, class JmpFn>
void emitTypeTest(Vout& v, IRLS& env, Type type,
                  Loc typeSrc, Loc dataSrc, Vreg sf, JmpFn doJcc) {
  // Note: If you add new supported type tests, you should update
  // negativeCheckType() to indicate whether it is precise or not.
  always_assert(!type.hasConstVal());
  always_assert_flog(
    !type.subtypeOfAny(TCls, TCountedStr, TPersistentArrLike),
    "Unsupported type in emitTypeTest(): {}", type
  );

  // Nothing to check.
  if (type == TGen) return;

  auto const cc = [&] {

    auto const mask_cmp = [&] (int mask, int bits, ConditionCode cc) {
      auto const masked = emitMaskTVType(v, mask, typeSrc);
      emitCmpTVType(v, sf, bits, masked);
      return cc;
    };

    auto const cmp = [&] (DataType kind, ConditionCode cc) {
      emitCmpTVType(v, sf, kind, typeSrc);
      return cc;
    };

    auto const test = [&] (int bits, ConditionCode cc) {
      emitTestTVType(v, sf, bits, typeSrc);
      return cc;
    };

    if (type <= TPersistentStr) return cmp(KindOfPersistentString, CC_E);
    if (type <= TStr)           return test(KindOfStringBit, CC_NZ);
    if (type <= TArr)           return test(KindOfArrayBit, CC_NZ);
    if (type <= TVec)           return mask_cmp(kDataTypeEquivalentMask,
                                                KindOfHackArrayVecType,
                                                CC_E);
    if (type <= TDict)          return mask_cmp(kDataTypeEquivalentMask,
                                                KindOfHackArrayDictType,
                                                CC_E);
    if (type <= TKeyset)        return mask_cmp(kDataTypeEquivalentMask,
                                                KindOfHackArrayKeysetType,
                                                CC_E);
    if (type <= TArrLike)       return test(KindOfArrayLikeMask, CC_NZ);

    // These are intentionally == and not <=.
    if (type == TNull)          return cmp(KindOfNull, CC_LE);
    if (type == TUncountedInit) return test(KindOfUncountedInitBit, CC_NZ);
    if (type == TUncounted)     return cmp(KindOfRefCountThreshold, CC_LE);
    if (type == TCell)          return cmp(KindOfRef, CC_NE);

    always_assert(type.isKnownDataType());
    always_assert(!(type < TBoxedInitCell));

    auto const dt = type.toDataType();
    return cmp(dt, CC_E);
  }();

  doJcc(cc, sf);

  if (type.isSpecialized()) {
    auto const sf2 = v.makeReg();
    detail::emitSpecializedTypeTest(v, env, type, dataSrc, sf2, doJcc);
  }
}

template<class Loc>
void emitTypeCheck(Vout& v, IRLS& env, Type type,
                   Loc typeSrc, Loc dataSrc, Block* taken) {
  emitTypeTest(v, env, type, typeSrc, dataSrc, v.makeReg(),
    [&] (ConditionCode cc, Vreg sf) {
      fwdJcc(v, env, ccNegate(cc), sf, taken);
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
