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
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

// The following wrappers around the BespkeLayout's virtual emit helpers are
// reponsible for providing layout-generic type information about the result.

SSATmp* typedEmitGet(BespokeLayout layout, IRGS& env,
                     SSATmp* arr, SSATmp* key, Block* taken) {
  auto const result = layout.emitGet(env, arr, key, taken);
  auto const elem = arrLikeElemType(arr->type(), key->type(), curClass(env));
  // TODO(kshaunak): We should also pull in TypeProfile information here.
  return gen(env, AssertType, elem.first, result);
}

SSATmp* typedEmitSet(BespokeLayout layout, IRGS& env,
                     SSATmp* arr, SSATmp* key, SSATmp* val) {
  auto const result = layout.emitSet(env, arr, key, val);
  return gen(env, AssertType, TCounted, result);
}

SSATmp* typedEmitAppend(BespokeLayout layout, IRGS& env,
                        SSATmp* arr, SSATmp* val) {
  auto const result = layout.emitAppend(env, arr, val);
  return gen(env, AssertType, TCounted, result);
}

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
        return ldLocWarn(env, mk.local, DataTypeSpecific);
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

SSATmp* emitSetNewElem(BespokeLayout layout, IRGS& env, SSATmp* origValue) {
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

  auto const newArr = typedEmitAppend(layout, env, base, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

SSATmp* emitSetElem(BespokeLayout layout, IRGS& env,
                    SSATmp* key, SSATmp* value) {
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

  auto const newArr = typedEmitSet(layout, env, base, key, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

void emitBespokeSetM(BespokeLayout layout, IRGS& env,
                     uint32_t nDiscard, MemberKey mk) {
  auto const value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);
  auto const result = [&] () -> SSATmp* {
    if (mcodeIsProp(mk.mcode)) PUNT(BespokeSetMProp);
    if (mk.mcode == MW) {
      return emitSetNewElem(layout, env, value);
    }

    assertx(mcodeIsElem(mk.mcode));
    auto const key = memberKey(env, mk);
    return emitSetElem(layout, env, key, value);
  }();
  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

SSATmp* emitIsset(BespokeLayout layout, IRGS& env, SSATmp* key) {
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
    [&](Block* taken) { return typedEmitGet(layout, env, base, key, taken); },
    [&](SSATmp* val) { return gen(env, IsNType, TInitNull, val); },
    [&] { return cns(env, false); }
  );
}

SSATmp* emitGetElem(BespokeLayout layout, IRGS& env, SSATmp* key, bool quiet) {
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
      return typedEmitGet(layout, env, base, key, taken);
    },
    [&](SSATmp* val) {
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

void emitBespokeQueryM(BespokeLayout layout, IRGS& env,
                       uint32_t nDiscard, QueryMOp query, MemberKey mk) {
  if (mk.mcode == MW) PUNT(BespokeQueryMNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeQueryMProp);
  auto const key = memberKey(env, mk);
  auto const result = [&] {
    switch (query) {
      case QueryMOp::InOut:
      case QueryMOp::CGet:
        return emitGetElem(layout, env, key, false);
      case QueryMOp::CGetQuiet:
        return emitGetElem(layout, env, key, true);
      case QueryMOp::Isset:
        return emitIsset(layout, env, key);
    }
    not_reached();
  }();
  mFinalImpl(env, nDiscard, result);
}

void emitBespokeIdx(BespokeLayout layout, IRGS& env) {
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
      return typedEmitGet(layout, env, base, key, taken);
    },
    [&](SSATmp* val) {
      finish(val);
      return nullptr;
    },
    [&] {
      finish(def);
      return nullptr;
    }
  );
}

void emitBespokeAKExists(BespokeLayout layout, IRGS& env) {
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

  ifThenElse(
    env,
    [&](Block* taken) {
      return typedEmitGet(layout, env, base, key, taken);
    },
    [&] { finish(true); },
    [&] { finish(false); }
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

SSATmp* bespokeElemImpl(BespokeLayout layout, IRGS& env,
                        MOpMode mode, Type baseType, SSATmp* key) {
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
    return layout.emitElem(env, baseLval, key, shouldThrow);
  } else {
    return cond(
      env,
      [&](Block* taken) {
        return typedEmitGet(layout, env, base, key, taken);
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

void emitBespokeDim(BespokeLayout layout, IRGS& env,
                    MOpMode mode, MemberKey mk) {
  auto const key = memberKey(env, mk);
  if (mk.mcode == MW) PUNT(BespokeDimNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeDimProp);
  assertx(mcodeIsElem(mk.mcode));

  auto const baseType = env.irb->fs().mbase().type;
  auto const val = bespokeElemImpl(layout, env, mode, baseType, key);

  stMBase(env, val);
}

void emitBespokeAddElemC(BespokeLayout layout, IRGS& env) {
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
  auto const newArr = typedEmitSet(layout, env, arr, key, value);
  push(env, newArr);
  decRef(env, key);
}

void emitBespokeAddNewElemC(BespokeLayout layout, IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{1})->type();
  if (!arrType.subtypeOfAny(TKeyset, TVec, TVArr)) {
    PUNT(AddNewElemC-Bespoke-WrongType);
  }

  auto const value = popC(env, DataTypeGeneric);
  auto const arr = popC(env);
  auto const newArr = typedEmitAppend(layout, env, arr, value);
  push(env, newArr);
}

void emitBespokeColFromArray(BespokeLayout layout, IRGS& env,
                             CollectionType type) {
  assertx(type != CollectionType::Pair);
  auto const arr = popC(env);
  auto const arrType = arr->type();
  if (!arrType.subtypeOfAny(TVec, TDict)) PUNT(Bespoke-BadColType);
  if (arrType <= TVec &&
      !(type == CollectionType::Vector || type == CollectionType::ImmVector)) {
    PUNT(Bespoke-ColTypeMismatch);
  }
  if (arrType <= TDict &&
      (type == CollectionType::Vector || type == CollectionType::ImmVector)) {
    PUNT(Bespoke-ColTypeMismatch);
  }
  auto const vanilla = layout.emitEscalateToVanilla(env, arr, "ColFromArray");
  auto const col = gen(env, NewColFromArray, NewColData { type }, vanilla);
  decRef(env, arr);
  push(env, col);
}

void emitBespokeClassGetTS(BespokeLayout layout, IRGS& env) {
  auto const reqType = RO::EvalHackArrDVArrs ? TDict : TDArr;
  auto const arr = topC(env);
  auto const arrType = arr->type();
  if (!(arrType <= reqType)) {
    if (arrType.maybe(reqType)) {
      PUNT(Bespoke-ClassGetTS-UnguardedTS);
    } else {
      gen(env, RaiseError, cns(env, s_reified_type_must_be_ts.get()));
      return;
    }
  }

  auto const generics = cns(env, s_generic_types.get());
  ifElse(
    env,
    [&](Block* taken) { typedEmitGet(layout, env, arr, generics, taken); },
    [&] { gen(env, Jmp, makeExitSlow(env)); }
  );

  auto const classKey = cns(env, s_classname.get());
  auto const classVal = cond(
    env,
    [&](Block* taken) {
      return typedEmitGet(layout, env, arr, classKey, taken);
    },
    [&] (SSATmp* val) { return val; },
    [&] {
      gen(env, ThrowArrayKeyException, arr, classKey);
      return cns(env, TBottom);
    }
  );

  auto const className = cond(
    env,
    [&] (Block* taken) {
      return gen(env, CheckType, TStr, taken, classVal);
    },
    [&] (SSATmp* val) { return val; },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseError, cns(env, s_new_instance_of_not_string.get()));
      return cns(env, TBottom);
    }
  );

  auto const cls = ldCls(env, className);
  popDecRef(env);
  push(env, cls);
  push(env, cns(env, TInitNull));
}

void emitBespokeShapesIdx(BespokeLayout layout, IRGS& env, uint32_t numArgs) {
  if (numArgs != 2 && numArgs != 3) PUNT(Bespoke-ShapesIdx-BadArgs);

  auto const def = [&] {
    if (numArgs < 3) return cns(env, TInitNull);

    auto const defVal = popC(env);
    auto const defType = defVal->type();
    if (!(defType <= TUninit) && defType.maybe(TUninit)) {
      PUNT(Bespoke-ShapesIdx-BadDefault);
    }
    return defType <= TUninit ? cns(env, TInitNull) : defVal;
  }();

  auto const key = popC(env);
  if (!key->type().subtypeOfAny(TInt, TStr)) {
    PUNT(Bespoke-ShapesIdx-BadKey);
  }

  auto const arr = popC(env);
  auto const arrType = arr->type();
  if (!(arrType <= (RuntimeOption::EvalHackArrDVArrs ? TDict : TDArr))) {
    if (arrType <= TNull) {
      decRef(env, key);
      push(env, def);
      return;
    } else {
      PUNT(Bespoke-ShapesIdx-BadVal);
    }
  }

  auto const res = cond(
    env,
    [&] (Block* taken) {
      return typedEmitGet(layout, env, arr, key, taken);
    },
    [&] (SSATmp* val) {
      gen(env, IncRef, val);
      decRef(env, def);
      return val;
    },
    [&] {
      return def;
    }
  );

  decRef(env, key);
  decRef(env, arr);
  push(env, res);
}

template <bool isFirst, bool isKey>
void emitBespokeFirstLast(BespokeLayout layout, IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) PUNT(Bespoke-FirstLast-BadArgs);
  auto const arr = popC(env);
  auto const size = gen(env, Count, arr);
  auto const elem = arrLikeFirstLastType(
    arr->type(), isFirst, isKey, curClass(env));
  auto const type = elem.first;
  auto const maybeEmpty = !elem.second;

  auto const res = cond(
    env,
    [&](Block* taken) {
      if (maybeEmpty) gen(env, JmpZero, taken, size);
    },
    [&] {
      if (isKey && arr->isA(TVArr|TVec)) {
        return isFirst ? cns(env, 0) : gen(env, SubInt, size, cns(env, 1));
      }

      auto const pos = isFirst ? layout.emitIterFirstPos(env, arr)
                               : layout.emitIterLastPos(env, arr);
      auto const elm = layout.emitIterElm(env, arr, pos);
      auto const val = isKey ? layout.emitIterGetKey(env, arr, elm)
                             : layout.emitIterGetVal(env, arr, elm);
      auto const result = gen(env, AssertType, type, val);

      gen(env, IncRef, result);
      return result;
    },
    [&] { return cns(env, maybeEmpty ? TInitNull : TBottom); }
  );
  push(env, res);
  decRef(env, arr);
}

using BespokeOptEmitFn = void (*)(BespokeLayout, IRGS&, uint32_t);
const hphp_fast_string_imap<BespokeOptEmitFn> s_bespoke_builtin_impls{
  {"HH\\Shapes::idx", emitBespokeShapesIdx},
  {"HH\\Lib\\_Private\\Native\\first", emitBespokeFirstLast<true, false>},
  {"HH\\Lib\\_Private\\Native\\last", emitBespokeFirstLast<false, false>},
  {"HH\\Lib\\_Private\\Native\\first_key", emitBespokeFirstLast<true, true>},
  {"HH\\Lib\\_Private\\Native\\last_key", emitBespokeFirstLast<false, true>},
};

void emitBespokeFCallBuiltin(
    BespokeLayout layout, IRGS& env, uint32_t numArgs,
    uint32_t numNonDefault, uint32_t numOut, const StringData* funcName) {
  auto const it = s_bespoke_builtin_impls.find(funcName->data());
  assertx(it != s_bespoke_builtin_impls.end());
  assertx(it->second);
  it->second(layout, env, numArgs);
}

void translateDispatchBespoke(BespokeLayout layout, IRGS& env,
                              const NormalizedInstruction& ni) {
  auto const DEBUG_ONLY sk = ni.source;
  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: perform bespoke translation\n",
             sk.offset(), opcodeToName(sk.op()));
  switch (ni.op()) {
    case Op::QueryM:
      emitBespokeQueryM(layout, env, ni.imm[0].u_IVA,
                        (QueryMOp) ni.imm[1].u_OA, ni.imm[2].u_KA);
      return;
    case Op::SetM:
      emitBespokeSetM(layout, env, ni.imm[0].u_IVA, ni.imm[1].u_KA);
      return;
    case Op::Idx:
    case Op::ArrayIdx:
      emitBespokeIdx(layout, env);
      return;
    case Op::AKExists:
      emitBespokeAKExists(layout, env);
      return;
    case Op::Dim:
      emitBespokeDim(layout, env, (MOpMode) ni.imm[0].u_OA, ni.imm[1].u_KA);
      return;
    case Op::AddElemC:
      emitBespokeAddElemC(layout, env);
      return;
    case Op::AddNewElemC:
      emitBespokeAddNewElemC(layout, env);
      return;
    case Op::ColFromArray:
      emitBespokeColFromArray(layout, env, (CollectionType) ni.imm[0].u_OA);
      return;
    case Op::ClassGetTS:
      emitBespokeClassGetTS(layout, env);
      return;
    case Op::FCallBuiltin:
      emitBespokeFCallBuiltin(
        layout, env, ni.imm[0].u_IVA, ni.imm[1].u_IVA, ni.imm[2].u_IVA,
        ni.unit()->lookupLitstrId(ni.imm[3].u_SA));
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
std::optional<BespokeLayout>
guardToLayout(IRGS& env, SrcKey sk, Location loc, Type type) {
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
  return layout;
}

void emitLogArrayReach(IRGS& env, Location loc, SrcKey sk) {
  // We won't have a tracelet ID to use during tracelet-selection time.
  // In profiling tracelets, we should have exactly one tracelet ID to log.
  if (env.formingRegion || env.context.kind != TransKind::Profile) return;
  assertx(env.context.transIDs.size() == 1);

  auto const transID = *env.context.transIDs.begin();
  auto const profile = bespoke::getSinkProfile(transID, sk);
  if (!profile) return;

  auto const arr = loadLocation(env, loc);
  gen(env, LogArrayReach, SinkProfileData(profile), arr);
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
        translateDispatchBespoke(topLayout, env, ni);
      } catch (const FailedIRGen& exn) {
        FTRACE_MOD(Trace::region, 1,
          "bespoke irgen for {} failed with {} while vanilla irgen succeeded\n",
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
        SCOPE_ASSERT_DETAIL("lost type info") {
          return folly::sformat(
            "Local {}: expected type: {}, inferred type: {}",
            i, vanillaLocalTypes[i], env.irb->fs().local(i).type);
        };
        assertx(env.irb->fs().local(i).type <= vanillaLocalTypes[i]);
      }
      for (int32_t i = 0; i < vanillaStackTypes.size(); i ++) {
        auto const offset = offsetFromIRSP(env, BCSPRelOffset{-i});
        SCOPE_ASSERT_DETAIL("lost type info") {
          return folly::sformat(
            "Stack {}: expected type: {}, inferred type: {}",
            i, vanillaStackTypes[i], env.irb->fs().stack(offset).type);
        };
        assertx(env.irb->fs().stack(offset).type <= vanillaStackTypes[i]);
      }
    }
  );
}

bool canProfilePropsInline(const Class* cls) {
  if (!cls) return false;
  if (!cls->pinitVec().empty()) return false;
  if (cls->hasReifiedGenerics() || cls->hasReifiedParent()) return false;

  auto num_array_props = 0;
  auto const limit = RO::EvalHHIRInliningMaxInitObjProps;
  for (auto slot = 0; slot < cls->numDeclProperties(); slot++) {
    if (cls->declProperties()[slot].attrs & AttrIsConst) return false;
    auto const index = cls->propSlotToIndex(slot);
    auto const tv = cls->declPropInit()[index].val.tv();
    if (!tvIsArrayLike(tv)) continue;
    if ((++num_array_props) > limit) return false;
  }
  return true;
}

void emitProfileArrLikeProps(IRGS& env) {
  auto const obj = topC(env);
  auto const cls = obj->type().clsSpec().exactCls();

  if (cls && cls->needsInitThrowable()) return;

  if (!canProfilePropsInline(cls)) {
    gen(env, ProfileArrLikeProps, obj);
    return;
  }

  for (auto slot = 0; slot < cls->numDeclProperties(); slot++) {
    auto const index = cls->propSlotToIndex(slot);
    auto const tv = cls->declPropInit()[index].val.tv();
    if (!tvIsArrayLike(tv)) continue;
    auto const profile = bespoke::getLoggingProfile(cls, slot);
    if (!profile) continue;

    auto const arr = gen(
      env,
      NewLoggingArray,
      LoggingProfileData(profile),
      cns(env, tv.val().parr)
    );
    auto const data = IndexData(index);
    auto const addr = gen(env, LdPropAddr, data, TLvalToPropCell, obj);
    gen(env, StMem, addr, arr);
  }
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
      translateDispatchBespoke(*bespoke, env, ni);
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
  if (op == Op::NewObjD || op == Op::NewObjRD) {
    emitProfileArrLikeProps(env);
  } else if (isArrLikeConstructorOp(op) || isArrLikeCastOp(op)) {
    auto const profile = bespoke::getLoggingProfile(sk);
    if (!profile) return;
    auto const data = LoggingProfileData(profile);
    push(env, gen(env, NewLoggingArray, data, popC(env)));
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
