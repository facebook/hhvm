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
#include "hphp/runtime/base/bespoke/layout-selection.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"
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

StaticString s_ColFromArray("ColFromArray");

// Simple code-gen helpers that do a single bespoke op, possibly with a few
// additional ops around them to produce better types. All of the mutating
// helpers here consume a ref on the input and produce one on the output.

template <typename Finish>
SSATmp* emitProfiledGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken, Finish finish,
    bool profiled = true) {
  auto const result = [&]{
    if (arr->isA(TVec)) {
      gen(env, CheckVecBounds, taken, arr, key);
      auto const data = BespokeGetData { BespokeGetData::KeyState::Present };
      auto const val = gen(env, BespokeGet, data, arr, key);
      return profiled ? profiledType(env, val, [&] { finish(val); })
                      : val;
    } else {
      auto const data = BespokeGetData { BespokeGetData::KeyState::Unknown };
      auto const val = gen(env, BespokeGet, data, arr, key);
      auto const pval = [&] {
        if (!profiled) return val;

        // We prefer to test the profiledType first, as this will rule out
        // TUninit when we have profiledType information. In this case, the
        // latter test will be optimized out.
        return profiledType(env, val, [&] {
          gen(env, CheckType, TInitCell, taken, val);
          finish(val);
        });
      }();

      auto const ival = gen(env, CheckType, TInitCell, taken, pval);
      // TODO(mcolavita): We need this assertion because we can lose const-val
      // information when we union the output type with TUninit.
      auto const resultType =
        arrLikeElemType(arr->type(), key->type(), curClass(env));
      return gen(env, AssertType, resultType.first, ival);
    }
  }();
  return result;
}

template <typename T>
Block* makeCatchBlock(IRGS& env, T genBody) {
  auto block = defBlock(env, Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), block);
  gen(env, BeginCatch);

  genBody();

  return block;
}

template <typename Finish>
SSATmp* emitProfiledGetThrow(
    IRGS& env, SSATmp* arr, SSATmp* key, Finish finish, bool profiled = true) {
  if (arr->isA(TVec)) {
    return cond(
      env,
      [&](Block* taken) {
        gen(env, CheckVecBounds, taken, arr, key);
      },
      [&] {
        auto const data = BespokeGetData { BespokeGetData::KeyState::Present };
        auto const val = gen(env, BespokeGet, data, arr, key);
        return profiled ? profiledType(env, val, [&] { finish(val); })
                        : val;
      },
      [&] {
        gen(env, ThrowOutOfBounds, arr, key);
        return cns(env, TBottom);
      }
    );
  } else {
    auto const catchBlock = makeCatchBlock(env, [&] {
      gen(env, ThrowOutOfBounds, arr, key);
    });
    auto const val = gen(env, BespokeGetThrow, catchBlock, arr, key);
    if (!profiled) return val;
    return profiledType(env, val, [&] { finish(val); });
  }
}

SSATmp* emitGet(IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) {
  return emitProfiledGet(env, arr, key, taken, [&](SSATmp*) {}, false);
}

SSATmp* emitElem(IRGS& env, SSATmp* arr, SSATmp* key, bool throwOnMissing) {
  return gen(env, BespokeElem, arr, key, cns(env, throwOnMissing));
}

SSATmp* emitSet(IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) {
  return gen(env, BespokeSet, arr, key, val);
}

SSATmp* emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) {
  return gen(env, BespokeAppend, arr, val);
}

SSATmp* emitEscalateToVanilla(
    IRGS& env, SSATmp* arr, const StaticString& reason) {
  auto const layout = arr->type().arrSpec().layout();
  if (layout.vanilla()) return arr;
  if (layout.bespoke()) {
    auto const str = cns(env, reason.get());
    auto const result = gen(env, BespokeEscalateToVanilla, arr, str);
    decRef(env, arr);
    return result;
  }
  return cond(
    env,
    [&](Block* taken) {
      return gen(env, CheckType, TVanillaArrLike, taken, arr);
    },
    [&](SSATmp* vanilla) { return vanilla; },
    [&]{
      auto const str = cns(env, reason.get());
      auto const type = TArrLike.narrowToLayout(ArrayLayout::Bespoke());
      auto const bespoke = gen(env, AssertType, type, arr);
      return gen(env, BespokeEscalateToVanilla, bespoke, str);
    }
  );
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
        return ldLoc(env, mk.local.id, DataTypeSpecific);
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

  if (res->type().maybe(TUninit)) PUNT(MInstr-Uninit-Key);
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

  auto const newArr = emitAppend(env, base, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

SSATmp* emitSetElem(IRGS& env, SSATmp* key, SSATmp* value) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);
  if (((baseType <= TVec) && !key->isA(TInt)) ||
      ((baseType <= TDict) && !key->isA(TInt | TStr))) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  } else if (baseType <= TKeyset) {
    auto const message = cns(env, s_InvalidKeysetOperationMsg.get());
    gen(env, ThrowInvalidOperation, message);
    return cns(env, TBottom);
  }

  auto const baseLoc = gen(env, LdMBase, TLvalToCell);
  if (!canUpdateCanonicalBase(baseLoc)) {
    gen(env, SetElem, baseLoc, key, value);
    return value;
  }

  auto const newArr = emitSet(env, base, key, value);

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
  if ((baseType <= TVec) && !key->isA(TInt)) {
    return cns(env, false);
  }

  return cond(
    env,
    [&](Block* taken) { return emitGet(env, base, key, taken); },
    [&](SSATmp* val) { return gen(env, IsNType, TInitNull, val); },
    [&] { return cns(env, false); }
  );
}

SSATmp* emitGetElem(IRGS& env, SSATmp* key, bool quiet, uint32_t nDiscard) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  if ((baseType <= TVec) && !key->isA(TInt)) {
    if (quiet) return cns(env, TInitNull);

    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  auto const finish = [&](SSATmp* val) {
    gen(env, IncRef, val);
    mFinalImpl(env, nDiscard, val);
  };

  if (quiet) {
    return cond(
      env,
      [&](Block* taken) {
        return emitProfiledGet(env, base, key, taken, finish);
      },
      [&](SSATmp* val) {
        gen(env, IncRef, val);
        return val;
      },
      [&] {
        return cns(env, TInitNull);
      }
    );
  } else {
    auto const val = emitProfiledGetThrow(env, base, key, finish);
    gen(env, IncRef, val);
    return val;
  }
}

void emitBespokeQueryM(
    IRGS& env, uint32_t nDiscard, QueryMOp query, MemberKey mk) {
  if (mk.mcode == MW) PUNT(BespokeQueryMNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeQueryMProp);
  auto const key = memberKey(env, mk);
  auto const result = [&] {
    switch (query) {
      case QueryMOp::InOut:
      case QueryMOp::CGet:
        return emitGetElem(env, key, false, nDiscard);
      case QueryMOp::CGetQuiet:
        return emitGetElem(env, key, true, nDiscard);
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
  auto const origKey = topC(env, BCSPRelOffset{1});
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
  if (key->isA(TNull) || ((baseType <= TVec) && key->isA(TNull | TStr))) {
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

  auto const res = cond(
    env,
    [&](Block* taken) {
      return emitGet(env, base, key, taken);
    },
    [&](SSATmp* val) { return val; },
    [&] { return def; }
  );
  auto const pres = profiledType(env, res, [&] { finish(res); });
  finish(pres);
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
  if ((baseType <= TVec) && key->isA(TStr)) {
    finish(false);
    return;
  } else if (!key->type().subtypeOfAny(TInt, TStr)) {
    throwBadKey();
    return;
  }

  ifThenElse(
    env,
    [&](Block* taken) {
      return emitGet(env, base, key, taken);
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

template <typename Finish>
SSATmp* bespokeElemImpl(
    IRGS& env, MOpMode mode, Type baseType, SSATmp* key, Finish finish) {
  auto const base = extractBase(env);
  auto const baseLval = gen(env, LdMBase, TLvalToCell);
  auto const needsLval = mode == MOpMode::Unset || mode == MOpMode::Define;
  auto const shouldThrow = mode == MOpMode::Warn || mode == MOpMode::InOut ||
                           mode == MOpMode::Define;

  auto const invalid_key = [&] {
    gen(env, ThrowInvalidArrayKey, extractBase(env), key);
    return cns(env, TBottom);
  };

  if ((baseType <= TVec) && key->isA(TStr)) {
    return shouldThrow ? invalid_key() : ptrToInitNull(env);
  }
  if (!key->isA(TInt | TStr)) return invalid_key();
  if (baseType <= TKeyset && needsLval) {
    gen(env, ThrowInvalidOperation,
        cns(env, s_InvalidKeysetOperationMsg.get()));
    return cns(env, TBottom);
  }

  if (needsLval) {
    return emitElem(env, baseLval, key, shouldThrow);
  } else if (shouldThrow) {
    auto const val =  emitProfiledGetThrow(env, base, key, [&](SSATmp* val) {
      finish(baseValueToLval(env, val));
    });
    return baseValueToLval(env, val);
  } else {
    return cond(
      env,
      [&](Block* taken) {
        return emitProfiledGet(env, base, key, taken, [&](SSATmp* val) {
          finish(baseValueToLval(env, val));
        });
      },
      [&](SSATmp* val) { return baseValueToLval(env, val); },
      [&] { return ptrToInitNull(env); }
    );
  }
}

void emitBespokeDim(IRGS& env, MOpMode mode, MemberKey mk) {
  auto const key = memberKey(env, mk);
  if (mk.mcode == MW) PUNT(BespokeDimNewElem);
  if (mcodeIsProp(mk.mcode)) PUNT(BespokeDimProp);
  assertx(mcodeIsElem(mk.mcode));

  auto const baseType = env.irb->fs().mbase().type;
  auto const finish = [&](SSATmp* val) { stMBase(env, val); };
  auto const val = bespokeElemImpl(env, mode, baseType, key, finish);
  finish(val);
}

void emitBespokeAddElemC(IRGS& env) {
  auto const keyType = topC(env, BCSPRelOffset{1})->type();
  auto const arrType = topC(env, BCSPRelOffset{2})->type();
  if (!(arrType <= TDict)) {
    PUNT(AddElemC-Bespoke-WrongType);
  } else if (!keyType.subtypeOfAny(TInt, TStr, TCls, TLazyCls)) {
    interpOne(env, arrType.unspecialize(), 3);
    return;
  }

  auto const value = popC(env, DataTypeGeneric);
  auto const key = classConvertPuntOnRaise(env, popC(env));
  auto const arr = popC(env);
  auto const newArr = emitSet(env, arr, key, value);
  push(env, newArr);
  decRef(env, key);
}

void emitBespokeAddNewElemC(IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{1})->type();
  if (!arrType.subtypeOfAny(TKeyset, TVec)) {
    PUNT(AddNewElemC-Bespoke-WrongType);
  }

  auto const value = popC(env, DataTypeGeneric);
  auto const arr = popC(env);
  auto const newArr = emitAppend(env, arr, value);
  push(env, newArr);
}

void emitBespokeColFromArray(IRGS& env,
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
  auto const vanilla = emitEscalateToVanilla(env, arr, s_ColFromArray);
  auto const col = gen(env, NewColFromArray, NewColData { type }, vanilla);
  push(env, col);
}

void emitBespokeClassGetTS(IRGS& env) {
  auto const arr = topC(env);
  auto const arrType = arr->type();
  if (!(arrType <= TDict)) {
    if (arrType.maybe(TDict)) {
      PUNT(Bespoke-ClassGetTS-UnguardedTS);
    } else {
      gen(env, RaiseError, cns(env, s_reified_type_must_be_ts.get()));
      return;
    }
  }

  auto const generics = cns(env, s_generic_types.get());
  ifElse(
    env,
    [&](Block* taken) { emitGet(env, arr, generics, taken); },
    [&] { gen(env, Jmp, makeExitSlow(env)); }
  );

  auto const classKey = cns(env, s_classname.get());
  auto const classVal = cond(
    env,
    [&](Block* taken) {
      return emitGet(env, arr, classKey, taken);
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

void emitBespokeShapesAt(IRGS& env, int32_t numArgs) {
  assertx(numArgs == 2);
  auto const arr = topC(env, BCSPRelOffset{1});
  auto const key = topC(env, BCSPRelOffset{0});
  assertx(arr->isA(TDict));
  assertx(key->type().subtypeOfAny(TInt, TStr));

  auto const finish = [&](SSATmp* val) {
    discard(env, 2 + kNumActRecCells);
    pushIncRef(env, val);
    decRef(env, arr);
    decRef(env, key);
  };

  auto const exit = makeExitSlow(env);
  finish(emitProfiledGet(env, arr, key, exit, finish));
}

void emitBespokeShapesIdx(IRGS& env, int32_t numArgs) {
  assertx(numArgs == 2 || numArgs == 3);
  auto const arr = topC(env, BCSPRelOffset{numArgs - 1});
  auto const key = topC(env, BCSPRelOffset{numArgs - 2});
  auto const def = numArgs == 3 ? topC(env) : cns(env, TInitNull);
  assertx(arr->isA(TDict));
  assertx(key->type().subtypeOfAny(TInt, TStr));

  auto const finish = [&](SSATmp* val) {
    discard(env, numArgs + kNumActRecCells);
    pushIncRef(env, val);
    decRef(env, arr);
    decRef(env, key);
    decRef(env, def);
  };

  auto const val = cond(
    env,
    [&](Block* taken) { return emitGet(env, arr, key, taken); },
    [&](SSATmp* val) { return val; },
    [&] { return def; }
  );
  finish(profiledType(env, val, [&] { finish(val); }));
}

void emitBespokeShapesExists(IRGS& env, int32_t numArgs) {
  assertx(numArgs == 2);
  auto const arr = topC(env, BCSPRelOffset{1});
  auto const key = topC(env, BCSPRelOffset{0});
  assertx(arr->isA(TDict));
  assertx(key->type().subtypeOfAny(TInt, TStr));

  auto const val = cond(
    env,
    [&](Block* taken) { emitGet(env, arr, key, taken); },
    [&] { return cns(env, true); },
    [&] { return cns(env, false); }
  );

  discard(env, 2 + kNumActRecCells);
  push(env, val);
  decRef(env, arr);
  decRef(env, key);
}

///////////////////////////////////////////////////////////////////////////////

enum class LayoutSensitiveCall {
  ShapesAt,
  ShapesIdx,
  ShapesExists,
};

const StaticString
  s_HH_Shapes("HH\\Shapes"),
  s_at("at"),
  s_idx("idx"),
  s_keyExists("keyExists");

folly::Optional<LayoutSensitiveCall>
getLayoutSensitiveCall(const IRGS& env, SrcKey sk) {
  if (sk.op() != Op::FCallClsMethodD) return folly::none;

  auto const cls  = sk.unit()->lookupLitstrId(getImm(sk.pc(), 2).u_SA);
  auto const func = sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA);

  if (!cls->isame(s_HH_Shapes.get())) return folly::none;

  if (func->isame(s_at.get()))        return LayoutSensitiveCall::ShapesAt;
  if (func->isame(s_idx.get()))       return LayoutSensitiveCall::ShapesIdx;
  if (func->isame(s_keyExists.get())) return LayoutSensitiveCall::ShapesExists;

  return folly::none;
}

bool canSpecializeCall(const IRGS& env, SrcKey sk, LayoutSensitiveCall call) {
  auto const fca = getImm(sk.pc(), 0).u_FCA;
  FTRACE_MOD(Trace::hhir, 3, "Analyzing layout-sensitive call:\n");

  if (fca.numRets != 1) return false;
  if (fca.hasGenerics() || fca.hasUnpack()) return false;

  auto const hasInOut = [&]{
    if (!fca.enforceInOut()) return false;
    for (auto i = 0; i < fca.numArgs; i++) {
      if (fca.isInOut(i)) return true;
    }
    return false;
  }();

  if (hasInOut) return false;

  auto const numArgsOkay = [&]{
    if (fca.numRets != 1) return false;
    auto const is_shapes_idx = call == LayoutSensitiveCall::ShapesIdx;
    return fca.numArgs == 2 || (fca.numArgs == 3 && is_shapes_idx);
  }();

  if (!numArgsOkay) return false;

  auto const numArgs = safe_cast<int32_t>(fca.numArgs);
  auto const al = Location::Stack{env.irb->fs().bcSPOff() - numArgs + 1};
  auto const kl = Location::Stack{env.irb->fs().bcSPOff() - numArgs + 2};
  auto const arr = env.irb->typeOf(al, DataTypeSpecific);
  auto const key = env.irb->typeOf(kl, DataTypeSpecific);
  FTRACE_MOD(Trace::hhir, 3, "Guarded arguments of layout-sensitive call:\n"
             "  arr: {} @ {}\n  key: {} @ {}\n", arr, show(al), key, show(kl));

  auto const specialize = (arr <= TDict) && key.subtypeOfAny(TInt, TStr);
  FTRACE_MOD(Trace::hhir, 3, "Types {}pecializing call on layout.\n",
             specialize ? "match. S" : "do not match. Not s");
  return specialize;
}

///////////////////////////////////////////////////////////////////////////////

void translateDispatchBespoke(IRGS& env, const NormalizedInstruction& ni) {
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
    case Op::ColFromArray:
      emitBespokeColFromArray(env, (CollectionType) ni.imm[0].u_OA);
      return;
    case Op::ClassGetTS:
      emitBespokeClassGetTS(env);
      return;
    case Op::FCallClsMethodD: {
      using LSC = LayoutSensitiveCall;
      auto const call = getLayoutSensitiveCall(env, ni.source);
      auto const numArgs = safe_cast<int32_t>(ni.imm[0].u_FCA.numArgs);
      switch (*call) {
        case LSC::ShapesAt:     return emitBespokeShapesAt(env, numArgs);
        case LSC::ShapesIdx:    return emitBespokeShapesIdx(env, numArgs);
        case LSC::ShapesExists: return emitBespokeShapesExists(env, numArgs);
      }
    }
    default:
      SCOPE_ASSERT_DETAIL("Unhandled Bespoke op") { return ni.toString(); };
      always_assert(false);
  }
}

folly::Optional<Location> getVanillaLocation(const IRGS& env, SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  if (isMemberDimOp(op) || (op == Op::QueryM || op == Op::SetM)) {
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

    case Op::FCallClsMethodD: {
      auto const call = getLayoutSensitiveCall(env, sk);
      if (!call || !canSpecializeCall(env, sk, *call)) return folly::none;
      auto const numArgs = safe_cast<int32_t>(getImm(sk.pc(), 0).u_FCA.numArgs);
      return {Location::Stack{soff - numArgs + 1}};
    }

    default:
      return folly::none;
  }
  always_assert(false);
}

// Returns a location that we should do some layout-sensitive guards for.
// Unlike getVanillaLocation, this helper checks known types.
folly::Optional<Location> getLocationToGuard(const IRGS& env, SrcKey sk) {
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

// Decide on what layout to specialize code for. In live translations,
// we simply use the known layout of the array, which allows us to completely
// avoid guarding on layouts (and thus over-specializing these translations).
//
// In optimized translations, we use the layout chosen by layout selection,
// emitting a check if necessary to refine the array's type to that layout.
ArrayLayout guardToLayout(IRGS& env, SrcKey sk, Location loc, Type type) {
  auto const kind = env.context.kind;
  assertx(!env.irb->guardFailBlock());
  if (kind != TransKind::Optimize) return type.arrSpec().layout();

  auto const layout = bespoke::layoutForSink(env.profTransIDs, sk);
  auto const target = TArrLike.narrowToLayout(layout);
  if (!type.maybe(target)) {
    // If the predicted type is incompatible with the known type, avoid
    // generating an impossible CheckType followed by unreachable code.
    assertx(type.arrSpec().vanilla() || type.arrSpec().bespoke());
    return type.arrSpec().layout();
  }

  if (RO::EvalBespokeEscalationSampleRate) {
    ifThen(
      env,
      [&](Block* taken) {
        env.irb->setGuardFailBlock(taken);
        checkType(env, loc, target, bcOff(env));
        env.irb->resetGuardFailBlock();
      },
      [&]{
        auto const arr = loadLocation(env, loc);
        gen(env, LogGuardFailure, target, arr);
        gen(env, Jmp, makeExit(env, curSrcKey(env)));
      }
    );
  } else {
    checkType(env, loc, target, bcOff(env));
  }
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

      auto const layout = ArrayLayout::Bespoke();
      auto const type = TArrLike.narrowToLayout(layout);
      assertTypeLocation(env, loc, type);

      try {
        translateDispatchBespoke(env, ni);
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

LoggingProfileData makeLoggingProfileData(bespoke::LoggingProfile* profile) {
  auto const isStatic = profile->data && profile->data->staticLoggingArray;
  return LoggingProfileData(profile, isStatic);
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
    if (!arrayTypeCouldBeBespoke(tv.val().parr->toDataType())) continue;
    auto const profile = bespoke::getLoggingProfile(cls, slot);
    if (!profile) continue;

    auto const arr = gen(
      env,
      NewLoggingArray,
      makeLoggingProfileData(profile),
      cns(env, tv.val().parr)
    );
    auto const data = IndexData(index);
    auto const addr = gen(env, LdPropAddr, data, TLvalToPropCell, obj);
    gen(env, StMem, addr, arr);
  }
}

bool specializeStructSource(IRGS& env, SrcKey sk, ArrayLayout layout) {
  auto const op = sk.op();
  assertx(op == Op::NewDictArray || op == Op::NewStructDict);

  if (op == Op::NewDictArray) {
    auto const data = ArrayLayoutData { layout };
    auto const arr = gen(env, AllocBespokeStructDict, data);
    push(env, arr);
    return true;
  }

  auto const imms = getImmVector(sk.pc());
  auto const size = safe_cast<uint32_t>(imms.size());

  auto const data = [&]() -> NewBespokeStructData {
    auto const slayout = bespoke::StructLayout::As(layout.bespokeLayout());
    auto const slots = size ? new (env.unit.arena()) Slot[size] : nullptr;
    for (auto i = 0; i < size; i++) {
      auto const key = curUnit(env)->lookupLitstrId(imms.vec32()[i]);
      slots[i] = slayout->keySlot(key);
      assertx(slots[i] != kInvalidSlot);
    }
    return {layout, spOffBCFromIRSP(env), safe_cast<uint32_t>(size), slots};
  }();

  auto const arr = gen(env, NewBespokeStructDict, data, sp(env));
  discard(env, size);
  push(env, arr);
  return true;
}

bool specializeSource(IRGS& env, SrcKey sk) {
  auto const kind = env.context.kind;
  if (kind != TransKind::Live && kind != TransKind::Optimize) return false;
  auto const op = sk.op();
  if (!isArrLikeConstructorOp(op) && !isArrLikeCastOp(op)) return false;
  auto const profile = bespoke::getLoggingProfile(sk);
  auto const layout = profile ? profile->getLayout() : ArrayLayout::Top();
  if (!layout.bespoke()) return false;

  if (auto const bad = profile->getStaticBespokeArray()) {
    assertx(arrayTypeCouldBeBespoke(bad->toDataType()));
    assertx(isArrLikeConstructorOp(op));
    push(env, cns(env, bad));
    return true;
  }

  assertx(layout.is_struct());
  return specializeStructSource(env, sk, layout);
}

}

///////////////////////////////////////////////////////////////////////////////

void handleBespokeInputs(IRGS& env, const NormalizedInstruction& ni,
                         std::function<void(IRGS&)> emitVanilla) {
  if (!allowBespokeArrayLikes()) return emitVanilla(env);
  auto const sk = ni.source;
  if (specializeSource(env, sk)) return;
  auto const loc = getLocationToGuard(env, sk);
  if (!loc) return emitVanilla(env);

  auto const type = env.irb->typeOf(*loc, DataTypeGeneric);
  assertx(type <= TArrLike);
  if (type.isKnownDataType() && !arrayTypeCouldBeBespoke(type.toDataType())) {
    assertTypeLocation(env, *loc, TVanillaArrLike);
    return emitVanilla(env);
  }

  emitLogArrayReach(env, *loc, sk);

  if (isIteratorOp(sk.op())) {
    emitVanilla(env);
  } else if (isFCall(sk.op())) {
    guardToLayout(env, sk, *loc, type);
    translateDispatchBespoke(env, ni);
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
    auto const layout = guardToLayout(env, sk, *loc, type);
    if (layout.vanilla()) {
      emitVanilla(env);
    } else {
      translateDispatchBespoke(env, ni);
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
    auto const newArr = topC(env);
    assertx(newArr->type().isKnownDataType());
    if (!arrayTypeCouldBeBespoke(newArr->type().toDataType())) return;

    auto const profile = bespoke::getLoggingProfile(sk);
    if (!profile) return;
    auto const data = makeLoggingProfileData(profile);
    push(env, gen(env, NewLoggingArray, data, popC(env)));
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
