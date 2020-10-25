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
#include "hphp/runtime/vm/jit/irgen-bespoke.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/logging-array.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

void stMBase(IRGS& env, SSATmp* base) {
  if (base->isA(TPtrToCell)) base = gen(env, ConvPtrToLval, base);
  assert_flog(base->isA(TLvalToCell), "Unexpected mbase: {}", *base->inst());
  gen(env, StMBase, base);
}

SSATmp* extractBase(IRGS& env) {
  auto const& mbase = env.irb->fs().mbase();
  if (mbase.value) return mbase.value;
  auto const mbaseLval = gen(env, LdMBase, TLvalToCell);
  return gen(env, LdMem, mbase.type, mbaseLval);
}

SSATmp* classConvertPuntOnRaise(IRGS& env, SSATmp* key) {
  if (key->isA(TCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      PUNT(BespokeClsConvert);
    }
    return gen(env, LdClsName, key);
  }
  if (key->isA(TLazyCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      PUNT(BespokeClsConvert);
    }
    return gen(env, LdLazyClsName, key);
  }
  return key;
}

SSATmp* memberKey(IRGS& env, MemberKey mk) {
  auto const res = [&] () -> SSATmp* {
    switch (mk.mcode) {
      case MW:
        return nullptr;
      case MEL: case MPL:
        return ldLocWarn(env, mk.local, nullptr, DataTypeSpecific);
      case MEC: case MPC:
        return topC(env, BCSPRelOffset{int32_t(mk.iva)});
      case MEI:
        return cns(env, mk.int64);
      case MET: case MPT: case MQT:
        return cns(env, mk.litstr);
    }
    not_reached();
  }();
  if (!res) return nullptr;

  if (!res->type().isKnownDataType()) PUNT(MInstr-KeyNotKnown);
  return classConvertPuntOnRaise(env, res);
}

SSATmp* emitSetNewElem(IRGS& env, SSATmp* origValue) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);
  auto const value = [&] {
    if (!(baseType <= TKeyset)) return origValue;

    if (!origValue->type().isKnownDataType()) {
      PUNT(Bespoke-SetNewElem-Keyset);
    }
    return classConvertPuntOnRaise(env, origValue);
  }();

  if (baseType <= TKeyset && !value->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, value);
    return value;
  }

  auto const baseLoc = gen(env, LdMBase, TLvalToCell);
  if (!canUpdateCanonicalBase(baseLoc)) {
    gen(env, SetNewElem, baseLoc, value);
    return value;
  }

  auto const layout = baseType.arrSpec().bespokeLayout();
  auto const newArr = layout->emitAppend(env, base, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

SSATmp* emitSetElem(IRGS& env, SSATmp* key, SSATmp* value) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);
  auto const isVec = baseType.subtypeOfAny(TVec, TVArr);
  auto const isDict = baseType.subtypeOfAny(TDict, TDArr);
  if ((isVec && !key->isA(TInt)) ||
      (isDict && !key->isA(TInt | TStr))) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  } else if (baseType <= TKeyset) {
    gen(env, ThrowInvalidOperation,
        cns(env, s_InvalidKeysetOperationMsg.get()));
    return cns(env, TBottom);
  }

  auto const baseLoc = gen(env, LdMBase, TLvalToCell);
  if (!canUpdateCanonicalBase(baseLoc)) {
    gen(env, SetElem, baseLoc, key, value);
    return value;
  }

  auto const layout = baseType.arrSpec().bespokeLayout();
  auto const newArr = layout->emitSet(env, base, key, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

void emitBespokeSetM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto const value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);
  auto const result = [&] () -> SSATmp* {
    if (mcodeIsProp(mk.mcode)) PUNT(BespokeSetMProp);
    if (mk.mcode == MW) {
      return emitSetNewElem(env, value);
    }

    assertx(mcodeIsElem(mk.mcode));
    auto const key = memberKey(env, mk);
    return emitSetElem(env, key, value);
  }();
  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

SSATmp* emitIsset(IRGS& env, SSATmp* key) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  if (baseType.subtypeOfAny(TVec, TVArr) && !key->isA(TInt)) {
    return cns(env, false);
  }

  return cond(
    env,
    [&](Block* taken) {
      auto const layout = baseType.arrSpec().bespokeLayout();
      return layout->emitGet(env, base, key, taken);
    },
    [&](SSATmp* val) { return gen(env, IsNType, TNull, val); },
    [&] { return cns(env, false); }
  );
}

SSATmp* emitGetElem(IRGS& env, SSATmp* key, bool quiet) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  if (baseType.subtypeOfAny(TVec, TVArr) && !key->isA(TInt)) {
    if (quiet) return cns(env, TInitNull);

    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  return cond(
    env,
    [&](Block* taken) {
      auto const layout = baseType.arrSpec().bespokeLayout();
      return layout->emitGet(env, base, key, taken);
    },
    [&](SSATmp* val) {
      // TODO(mcolavita): type profile information
      gen(env, IncRef, val);
      return val;
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      if (quiet) return cns(env, TInitNull);

      gen(env, ThrowOutOfBounds, base, key);
      return cns(env, TBottom);
    }
  );
}

void emitBespokeQueryM(IRGS& env, uint32_t nDiscard, QueryMOp query,
                       MemberKey mk) {
  if (mk.mcode == MW) PUNT(BespokeQueryMNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeQueryMProp);
  auto const key = memberKey(env, mk);
  auto const result = [&] {
    switch (query) {
      case QueryMOp::InOut:
      case QueryMOp::CGet:
        return emitGetElem(env, key, false);
      case QueryMOp::CGetQuiet:
        return emitGetElem(env, key, true);
      case QueryMOp::Isset:
        return emitIsset(env, key);
    }
    not_reached();
  }();
  mFinalImpl(env, nDiscard, result);
}

void emitBespokeIdx(IRGS& env) {
  auto const def = topC(env, BCSPRelOffset{0});
  auto const base = topC(env, BCSPRelOffset{2});
  auto const origKey = topC(env, BCSPRelOffset{1}, DataTypeGeneric);
  if (!origKey->type().isKnownDataType()) PUNT(Bespoke-Idx-KeyNotKnown);
  auto const key = classConvertPuntOnRaise(env, origKey);

  auto const finish = [&](SSATmp* elem) {
    discard(env, 3);
    pushIncRef(env, elem);
    decRef(env, def);
    decRef(env, key);
    decRef(env, base);
  };

  auto const baseType = base->type();
  auto const isVec = baseType.subtypeOfAny(TVec, TVArr);
  if (key->isA(TNull) || (isVec && key->isA(TNull | TStr))) {
    finish(def);
    return;
  }

  if (!key->isA(TInt) && !key->isA(TStr)) {
    finish(def);
    updateMarker(env);
    env.irb->exceptionStackBoundary();
    gen(env, ThrowInvalidArrayKey, base, key);
    return;
  }

  cond(
    env,
    [&](Block* taken) {
      auto const layout = baseType.arrSpec().bespokeLayout();
      return layout->emitGet(env, base, key, taken);
    },
    [&](SSATmp* val) {
      // TODO(mcolavita): type profile information
      finish(val);
      return nullptr;
    },
    [&] {
      finish(def);
      return nullptr;
    }
  );
}

void emitBespokeAKExists(IRGS& env) {
  auto const base = popC(env);
  auto const origKey = popC(env);
  if (!origKey->type().isKnownDataType()) PUNT(Bespoke-AKExists-KeyNotKnown);
  auto const key = classConvertPuntOnRaise(env, origKey);

  auto const finish = [&](bool res) {
    push(env, cns(env, res));
    decRef(env, base);
    decRef(env, key);
  };

  auto const throwBadKey = [&] {
    finish(false);
    updateMarker(env);
    env.irb->exceptionStackBoundary();
    gen(env, ThrowInvalidArrayKey, base, key);
  };

  auto const baseType = base->type();
  auto const isVec = baseType.subtypeOfAny(TVec, TVArr);
  if (isVec && key->isA(TStr)) {
    finish(false);
    return;
  } else if (!key->type().subtypeOfAny(TInt, TStr)) {
    throwBadKey();
    return;
  }

  cond(
    env,
    [&](Block* taken) {
      auto const layout = baseType.arrSpec().bespokeLayout();
      return layout->emitGet(env, base, key, taken);
    },
    [&](SSATmp* val) {
      finish(true);
      return nullptr;
    },
    [&] {
      finish(false);
      return nullptr;
    }
  );
}

SSATmp* tvTempBasePtr(IRGS& env) {
  return gen(env, LdMIStateAddr, cns(env, offsetof(MInstrState, tvTempBase)));
}

SSATmp* baseValueToLval(IRGS& env, SSATmp* base) {
  auto const temp = tvTempBasePtr(env);
  gen(env, StMem, temp, base);
  return gen(env, ConvPtrToLval, temp);
}

SSATmp* bespokeElemImpl(IRGS& env, MOpMode mode, Type baseType, SSATmp* key) {
  auto const base = extractBase(env);
  auto const baseLval = gen(env, LdMBase, TLvalToCell);
  auto const needsLval = mode == MOpMode::Unset || mode == MOpMode::Define;
  auto const shouldThrow = mode == MOpMode::Warn || mode == MOpMode::InOut ||
                           mode == MOpMode::Define;

  auto const invalid_key = [&] {
    gen(env, ThrowInvalidArrayKey, extractBase(env), key);
    return cns(env, TBottom);
  };

  if (baseType.subtypeOfAny(TVec, TVArr) && key->isA(TStr)) {
    return shouldThrow ? invalid_key() : ptrToInitNull(env);
  }
  if (!key->isA(TInt | TStr)) return invalid_key();
  if (baseType <= TKeyset && needsLval) {
    gen(env, ThrowInvalidOperation,
        cns(env, s_InvalidKeysetOperationMsg.get()));
    return cns(env, TBottom);
  }

  if (needsLval) {
    auto const layout = baseType.arrSpec().bespokeLayout();
    return layout->emitElem(env, baseLval, key, shouldThrow);
  } else {
    return cond(
      env,
      [&](Block* taken) {
        auto const layout = baseType.arrSpec().bespokeLayout();
        return layout->emitGet(env, base, key, taken);
      },
      [&](SSATmp* val) {
        return baseValueToLval(env, val);
      },
      [&] {
        if (shouldThrow) gen(env, ThrowOutOfBounds, base, key);
        return ptrToInitNull(env);
      }
    );
  }
}

void emitBespokeDim(IRGS& env, MOpMode mode, MemberKey mk) {
  auto const key = memberKey(env, mk);
  if (mk.mcode == MW) PUNT(BespokeDimNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeDimProp);
  assertx(mcodeIsElem(mk.mcode));

  auto const baseType = env.irb->fs().mbase().type;
  auto const val = bespokeElemImpl(env, mode, baseType, key);

  stMBase(env, val);
}

void emitBespokeAddElemC(IRGS& env) {
  auto const keyType = topC(env, BCSPRelOffset{1})->type();
  auto const arrType = topC(env, BCSPRelOffset{2})->type();
  if (!arrType.subtypeOfAny(TDict, TDArr)) {
    PUNT(AddElemC-Bespoke-WrongType);
  } else if (!keyType.subtypeOfAny(TInt, TStr, TCls, TLazyCls)) {
    interpOne(env, arrType.unspecialize(), 3);
    return;
  }

  auto const value = popC(env, DataTypeGeneric);
  auto const key = classConvertPuntOnRaise(env, popC(env));
  auto const arr = popC(env);
  auto const layout = arrType.arrSpec().bespokeLayout();
  auto const newArr = layout->emitSet(env, arr, key, value);
  push(env, newArr);
  decRef(env, key);
}

void emitBespokeAddNewElemC(IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{1})->type();
  if (!arrType.subtypeOfAny(TKeyset, TVec, TVArr)) {
    PUNT(AddNewElemC-Bespoke-WrongType);
  }

  auto const value = popC(env, DataTypeGeneric);
  auto const arr = popC(env);
  auto const layout = arrType.arrSpec().bespokeLayout();
  auto const newArr = layout->emitAppend(env, arr, value);
  push(env, newArr);
  decRef(env, value);
}

void translateDispatchBespoke(IRGS& env,
                              const NormalizedInstruction& ni) {
  auto const DEBUG_ONLY sk = ni.source;
  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: perform bespoke translation\n",
             sk.offset(), opcodeToName(sk.op()));
  switch (ni.op()) {
    case Op::QueryM:
      emitBespokeQueryM(env, ni.imm[0].u_IVA, (QueryMOp) ni.imm[1].u_OA,
                        ni.imm[2].u_KA);
      return;
    case Op::SetM:
      emitBespokeSetM(env, ni.imm[0].u_IVA, ni.imm[1].u_KA);
      return;
    case Op::Idx:
    case Op::ArrayIdx:
      emitBespokeIdx(env);
      return;
    case Op::AKExists:
      emitBespokeAKExists(env);
      return;
    case Op::Dim:
      emitBespokeDim(env, (MOpMode) ni.imm[0].u_OA, ni.imm[1].u_KA);
      return;
    case Op::AddElemC:
      emitBespokeAddElemC(env);
      return;
    case Op::AddNewElemC:
      emitBespokeAddNewElemC(env);
      return;
    case Op::FCallBuiltin:
    case Op::ClassGetTS:
    case Op::ColFromArray:
      interpOne(env);
      return;
    case Op::IterInit:
    case Op::LIterInit:
    case Op::LIterNext:
      always_assert(false);
    default:
      not_reached();
  }
}

folly::Optional<Location> getVanillaLocationForBuiltin(const IRGS& env,
                                                        SrcKey sk) {
  auto const soff = env.irb->fs().bcSPOff();

  assertx(sk.op() == Op::FCallBuiltin);
  auto const func =
    Func::lookupBuiltin(sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA));
  if (!func) return folly::none;
  auto const param = getBuiltinVanillaParam(func->fullName()->data());
  if (param < 0) return folly::none;

  if (getImm(sk.pc(), 0).u_IVA != func->numParams()) return folly::none;
  if (getImm(sk.pc(), 2).u_IVA != func->numInOutParams()) return folly::none;
  return {Location::Stack{soff - func->numParams() + 1 + param}};
}

folly::Optional<Location> getVanillaLocation(const IRGS& env, SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  if (op == Op::FCallBuiltin) {
    return getVanillaLocationForBuiltin(env, sk);
  } else if (isMemberDimOp(op) || (op == Op::QueryM || op == Op::SetM)) {
    return {Location::MBase{}};
  }

  switch (op) {
    // Array accesses constrain the base.
    case Op::Idx:
    case Op::ArrayIdx:
    case Op::AddElemC:
      return {Location::Stack{soff - 2}};
    case Op::AddNewElemC:
      return {Location::Stack{soff - 1}};
    case Op::AKExists:
    case Op::ClassGetTS:
    case Op::ColFromArray:
    case Op::IterInit:
      return {Location::Stack{soff}};

    // Local iterators constrain the local base.
    case Op::LIterInit:
    case Op::LIterNext: {
      auto const local = getImm(sk.pc(), localImmIdx(op)).u_LA;
      return {Location::Local{safe_cast<uint32_t>(local)}};
    }

    default:
      return folly::none;
  }
  always_assert(false);
}

// Returns a location that we should do some layout-sensitive guards for.
// Unlike getVanillaLocation, this helper checks options, known types, etc.
folly::Optional<Location> getLocationToGuard(const IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return folly::none;

  // If this check fails, the bytecode is not layout-sensitive.
  auto const loc = getVanillaLocation(env, sk);
  if (!loc) return folly::none;

  // Even if the bytecode is layout-sensitive, it may be applied to e.g. an
  // object input, or our known types may be too general for us to guard.
  auto const gc = isIteratorOp(sk.op()) ? DataTypeIterBase : DataTypeSpecific;
  auto const type = env.irb->typeOf(*loc, gc);
  auto const needsGuard = type != TBottom && type <= TArrLike &&
                          typeFitsConstraint(type, gc);
  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: location {}: {} {} layout guard\n",
             sk.offset(), opcodeToName(sk.op()), show(*loc), type,
             needsGuard ? "needs" : "does not need");
  return needsGuard ? loc : folly::none;
}

// Strengthen the guards for a layout-sensitive location prior to irgen for the
// bytecode, returning false if standard vanilla translation should be used and
// true if bespoke translation should be used.
//
// Right now, we strengthen the guard based only on the type of the array-like
// at that location. In the future, we'll still do that for live translations,
// but we'll use profiling for optimized translations.
bool guardToLayout(IRGS& env, SrcKey sk, Location loc, Type type) {
  assertx(env.context.kind != TransKind::Profile);
  assertx(!env.irb->guardFailBlock());

  auto const layout = type.arrSpec().bespokeLayout();
  auto const target_type = layout ? TArrLike.narrowToBespokeLayout(*layout)
                                  : TVanillaArrLike;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: guard input {} to layout: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), target_type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setArrayLayoutSensitive();
  env.irb->constrainLocation(loc, gc);
  checkType(env, loc, target_type, bcOff(env));
  return !target_type.arrSpec().vanilla();
}

void emitLogArrayReach(IRGS& env, Location loc, SrcKey sk) {
  // We won't have a tracelet ID to use during tracelet-selection time.
  // In profiling tracelets, we should have exactly one tracelet ID to log.
  if (env.formingRegion || env.context.kind != TransKind::Profile) return;
  assertx(env.context.transIDs.size() == 1);

  auto const arr = loadLocation(env, loc);
  auto const transID = *env.context.transIDs.begin();
  gen(env, LogArrayReach, TransIDData(transID), arr);
}

// In a profiling tracelet, we don't want to guard on the array being vanilla,
// so we emit code to handle both vanilla and logging arrays.
void emitLoggingDiamond(
    IRGS& env, const NormalizedInstruction& ni, Location loc,
    std::function<void(IRGS&)> emitVanilla) {
  assertx(env.context.kind == TransKind::Profile);
  assertx(!env.irb->guardFailBlock());

  auto const dropArrSpec = [&](Type type) {
    return type <= TArrLike ? type.unspecialize() : type;
  };
  std::vector<Type> vanillaLocalTypes;
  std::vector<Type> vanillaStackTypes;
  ifThen(
    env,
    [&](Block* taken) {
      env.irb->setGuardFailBlock(taken);
      checkType(env, loc, TVanillaArrLike, bcOff(env));
      env.irb->resetGuardFailBlock();

      emitVanilla(env);

      // We have a vanilla and a logging side of the diamond. The logging side
      // may have lost type information because of using InterpOne.  We will
      // emit AssertTypes with the type information from the vanilla side after
      // the generated code on the logging side to regain this information.
      auto const locals = curFunc(env)->numLocals();
      auto const pushed = getStackPushed(ni.source.pc());
      vanillaLocalTypes.reserve(locals);
      vanillaStackTypes.reserve(pushed);
      for (uint32_t i = 0; i < locals; i ++) {
        auto const lType = env.irb->fs().local(i).type;
        vanillaLocalTypes.push_back(dropArrSpec(lType));
      }
      for (int32_t i = 0; i < pushed; i ++) {
        auto const idx = BCSPRelOffset{-i};
        auto const sType = env.irb->fs().stack(offsetFromIRSP(env, idx)).type;
        vanillaStackTypes.push_back(dropArrSpec(sType));
      }
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      auto const topLayout = BespokeLayout::TopLayout();
      auto const topType = TArrLike.narrowToBespokeLayout(topLayout);
      assertTypeLocation(env, loc, topType);

      try {
        translateDispatchBespoke(env, ni);
      } catch (const FailedIRGen& exn) {
        FTRACE_MOD(Trace::region, 1,
          "logging ir generation for {} failed with {} while vanilla succeeded\n",
           ni.toString(), exn.what());
        throw;
      }

      // For layout-sensitive bytecodes, opcodeChangesPC implies that
      // opcodeBreaksBB and we are at the end of the tracelet. Therefore we
      // don't need to worry about control flow after the InterpOneCF.
      auto const DEBUG_ONLY op = curFunc(env)->getOp(bcOff(env));
      auto const DEBUG_ONLY opChangePC = opcodeChangesPC(op);
      assertx(IMPLIES(opChangePC, opcodeBreaksBB(op, false)));

      for (uint32_t i = 0; i < vanillaLocalTypes.size(); i ++) {
        gen(env, AssertLoc, vanillaLocalTypes[i], LocalId(i), fp(env));
      }
      for (int32_t i = 0; i < vanillaStackTypes.size(); i ++) {
        auto const idx = BCSPRelOffset{-i};
        gen(env, AssertStk, vanillaStackTypes[i],
            IRSPRelOffsetData { offsetFromIRSP(env, idx) }, sp(env));
      }
    }
  );
}

}

///////////////////////////////////////////////////////////////////////////////

void handleBespokeInputs(IRGS& env, const NormalizedInstruction& ni,
                         std::function<void(IRGS&)> emitVanilla) {
  auto const sk = ni.source;
  auto const loc = getLocationToGuard(env, sk);
  if (!loc) return emitVanilla(env);

  auto const type = env.irb->typeOf(*loc, DataTypeGeneric);
  assertx(type <= TArrLike);

  emitLogArrayReach(env, *loc, sk);

  if (isIteratorOp(sk.op())) {
    emitVanilla(env);
  } else if (env.context.kind == TransKind::Profile) {
    // In a profiling tracelet, we'll emit a diamond that handles vanilla
    // array-likes on one side and bespoke array-likes on the other.
    if (type.arrSpec().vanilla()) {
      emitVanilla(env);
    } else {
      emitLoggingDiamond(env, ni, *loc, emitVanilla);
    }
  } else {
    // In an optimized or live translation, we guard to a specialized layout
    // and then emit either vanilla or bespoke code.
    auto const bespoke = guardToLayout(env, sk, *loc, type);
    if (bespoke) {
      translateDispatchBespoke(env, ni);
    } else {
      emitVanilla(env);
    }
  }
}

void handleVanillaOutputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return;
  if (env.context.kind != TransKind::Profile &&
      !shouldTestBespokeArrayLikes()) {
    return;
  }
  auto const op = sk.op();
  if (!isArrLikeConstructorOp(op) && !isArrLikeCastOp(op)) return;

  // These SrcKeys are always skipped in maybeMakeLoggingArray. If we make
  // this logic more complicated, we should expose a helper to share the code.
  if ((op == Op::Array || op == Op::Dict) &&
      sk.advanced().op() == Op::IsTypeStructC) {
    return;
  }

  auto const result = topC(env);
  auto const vanilla = result->type().arrSpec().vanilla();
  assertx(IMPLIES(isArrLikeConstructorOp(op), vanilla));
  if (!vanilla) return;

  auto const logging = gen(env, NewLoggingArray, result);
  discard(env);
  push(env, logging);
}

///////////////////////////////////////////////////////////////////////////////

}}}
