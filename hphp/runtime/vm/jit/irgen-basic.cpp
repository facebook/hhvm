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

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

const StaticString s_reified_type_must_be_ts(
  "Reified type must be a type structure");
const StaticString s_new_instance_of_not_string(
  "You cannot create a new instance of this type as it is not a string");

} // namespace

void emitClsRefGetC(IRGS& env, uint32_t slot) {
  auto const name = topC(env);
  if (!name->type().subtypeOfAny(TObj, TStr)) {
    interpOne(env, *env.currentNormalizedInstruction);
    return;
  }
  popC(env);
  if (name->isA(TObj)) {
    putClsRef(env, slot, gen(env, LdObjClass, name));
    decRef(env, name);
    return;
  }
  if (!name->hasConstVal()) gen(env, RaiseStrToClassNotice, name);
  auto const cls = ldCls(env, name);
  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const isreified = gen(env, IsReifiedName, name);
      gen(env, JmpZero, taken, isreified);
    },
    [&] {
      auto ts = gen(env, LdReifiedGeneric, name);
      putClsRef(env, slot, cls, ts);
    },
    [&] { putClsRef(env, slot, cls); }
  );
  decRef(env, name);
}

void emitClsRefGetTS(IRGS& env, uint32_t slot) {
  auto const required_ts_type = RuntimeOption::EvalHackArrDVArrs ? TDict : TArr;
  auto const ts = topC(env);
  if (!ts->isA(required_ts_type)) {
    if (ts->type().maybe(required_ts_type)) {
      PUNT(ClsRefGetTS-UnguardedTS);
    } else {
      gen(env, RaiseError, cns(env, s_reified_type_must_be_ts.get()));
    }
  }
  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const val = gen(
        env,
        RuntimeOption::EvalHackArrDVArrs ? AKExistsDict : AKExistsArr,
        ts,
        cns(env, s_generic_types.get())
      );
      gen(env, JmpZero, taken, val);
    },
    [&] {
      // Has reified generics
      hint(env, Block::Hint::Unlikely);
      interpOne(env, *env.currentNormalizedInstruction);
    },
    // taken
    [&] {
      // Does not have reified generics
      auto const key = cns(env, s_classname.get());
      auto const clsName = profiledArrayAccess(env, ts, key,
        [&] (SSATmp* base, SSATmp* key, uint32_t pos) {
          return gen(env, RuntimeOption::EvalHackArrDVArrs ? DictGetK
                                                           : MixedArrayGetK,
                     IndexData { pos }, base, key);
        },
        [&] (SSATmp* key) {
          if (RuntimeOption::EvalHackArrDVArrs) {
            return gen(env, DictGet, ts, key);
          }
          return gen(env, ArrayGet, MOpModeData { MOpMode::Warn }, ts, key);
        }
      );
      if (!clsName->isA(TStr)) {
        if (!ts->type().maybe(TStr)) PUNT(ClsRefGetTS-ClassnameNotStr);
        gen(env, RaiseError, cns(env, s_new_instance_of_not_string.get()));
      }
      auto const cls = ldCls(env, clsName);
      popDecRef(env);
      putClsRef(env, slot, cls);
    }
  );
}

void emitCGetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const loc = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeBoxAndCountnessInit
  );
  pushIncRef(env, loc);
}

void emitCGetQuietL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  pushIncRef(
    env,
    [&] {
      auto const loc = ldLocInner(
        env,
        id,
        ldrefExit,
        ldPMExit,
        DataTypeBoxAndCountnessInit
      );

      if (loc->type() <= TUninit) {
        return cns(env, TInitNull);
      }

      if (loc->type().maybe(TUninit)) {
        return cond(
          env,
          [&] (Block* taken) {
            gen(env, CheckInit, taken, loc);
          },
          [&] { // Next: local is InitCell.
            return loc;
          },
          [&] { // Taken: local is Uninit
            return cns(env, TInitNull);
          }
        );
      }

      return loc;
    }()
  );
}

void emitCUGetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  pushIncRef(env, ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeGeneric));
}

void emitPushL(IRGS& env, int32_t id) {
  assertTypeLocal(env, id, TInitCell);  // bytecode invariant
  auto* locVal = ldLoc(env, id, makeExit(env), DataTypeGeneric);
  push(env, locVal);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
}

void emitCGetL2(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const oldTop = pop(env, DataTypeGeneric);
  auto const val = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeBoxAndCountnessInit
  );
  pushIncRef(env, val);
  push(env, oldTop);
}

void emitVGetL(IRGS& env, int32_t id) {
  auto const value = ldLoc(env, id, makeExit(env), DataTypeBoxAndCountnessInit);
  auto const boxed = boxHelper(
    env,
    gen(env, AssertType, TCell | TBoxedInitCell, value),
    [&] (SSATmp* v) {
      stLocRaw(env, id, fp(env), v);
    });

  pushIncRef(env, boxed);
}

void emitBox(IRGS& env) {
  push(env, gen(env, Box, pop(env, DataTypeGeneric)));
}

void emitUnsetL(IRGS& env, int32_t id) {
  auto const prev = ldLoc(env, id, makeExit(env), DataTypeBoxAndCountness);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
  decRef(env, prev);
}

void emitSetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);

  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(env, DataTypeGeneric);
  pushStLoc(env, id, ldrefExit, ldPMExit, src);
}

void emitInitThisLoc(IRGS& env, int32_t id) {
  if (!hasThis(env)) {
    // Do nothing if this is null
    return;
  }
  auto const ldrefExit = makeExit(env);
  auto const ctx       = ldCtx(env);
  auto const oldLoc = ldLoc(env, id, ldrefExit, DataTypeBoxAndCountness);
  auto const this_  = castCtxThis(env, ctx);
  gen(env, IncRef, this_);
  stLocRaw(env, id, fp(env), this_);
  decRef(env, oldLoc);
}

void emitPrint(IRGS& env) {
  auto const type = topC(env)->type();
  if (!type.subtypeOfAny(TInt, TBool, TNull, TStr)) {
    interpOne(env, TInt, 1);
    return;
  }

  auto const cell = popC(env);

  Opcode op;
  if (type <= TStr) {
    op = PrintStr;
  } else if (type <= TInt) {
    op = PrintInt;
  } else if (type <= TBool) {
    op = PrintBool;
  } else {
    assertx(type <= TNull);
    op = Nop;
  }
  // the print helpers decref their arg, so don't decref pop'ed value
  if (op != Nop) {
    gen(env, op, cell);
  }
  push(env, cns(env, 1));
}

void emitUnbox(IRGS& env) {
  auto const exit = makeExit(env);
  auto const srcBox = popV(env);
  auto const unboxed = unbox(env, srcBox, exit);
  pushIncRef(env, unboxed);
  decRef(env, srcBox);
}

void emitThis(IRGS& env) {
  auto const this_ = checkAndLoadThis(env);
  pushIncRef(env, this_);
}

void emitCheckThis(IRGS& env) {
  checkAndLoadThis(env);
}

void emitBareThis(IRGS& env, BareThisOp subop) {
  auto const ctx = ldCtx(env);
  if (!hasThis(env)) {
    if (subop == BareThisOp::NoNotice) {
      push(env, cns(env, TInitNull));
      return;
    }
    assertx(subop != BareThisOp::NeverNull);
    interpOne(env, TInitNull, 0); // will raise notice and push null
    return;
  }

  pushIncRef(env, castCtxThis(env, ctx));
}

void emitFuncNumArgs(IRGS& env) {
  if (curFunc(env)->isPseudoMain()) PUNT(FuncNumArgs-PseudoMain);
  push(env, gen(env, LdARNumParams, fp(env)));
}

void emitClone(IRGS& env) {
  if (!topC(env)->isA(TObj)) PUNT(Clone-NonObj);
  auto const obj        = popC(env);
  push(env, gen(env, Clone, obj));
  decRef(env, obj);
}

void emitLateBoundCls(IRGS& env, uint32_t slot) {
  auto const clss = curClass(env);
  if (!clss) {
    // no static context class, so this will raise an error
    interpOne(env, *env.currentNormalizedInstruction);
    return;
  }
  auto const ctx = ldCtx(env);
  putClsRef(env, slot, gen(env, LdClsCtx, ctx));
}

void emitSelf(IRGS& env, uint32_t slot) {
  auto const clss = curClass(env);
  if (clss == nullptr) {
    interpOne(env, *env.currentNormalizedInstruction);
  } else {
    putClsRef(env, slot, cns(env, clss));
  }
}

void emitParent(IRGS& env, uint32_t slot) {
  auto const clss = curClass(env);
  if (clss == nullptr || clss->parent() == nullptr) {
    interpOne(env, *env.currentNormalizedInstruction);
  } else {
    putClsRef(env, slot, cns(env, clss->parent()));
  }
}

void emitClsRefName(IRGS& env, uint32_t slot) {
  auto const cls = takeClsRefCls(env, slot);
  push(env, gen(env, LdClsName, cls));
}

//////////////////////////////////////////////////////////////////////

void emitCastArray(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToArr, src));
}

void emitCastVArray(IRGS& env) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);

  auto const src = popC(env);

  auto const raise = [&](const char* type) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(
        env,
        makeStaticString(folly::sformat("{} to varray conversion", type))
      )
    );
    return cns(env, TBottom);
  };

  push(
    env,
    [&] {
      if (src->isA(TArr)) {
        return cond(
          env,
          [&](Block* taken) { return gen(env, CheckVArray, taken, src); },
          [&](SSATmp* varr) { return varr; },
          [&]{ return gen(env, ConvArrToVArr, src); }
        );
      }
      if (src->isA(TVec))    return gen(env, ConvVecToVArr, src);
      if (src->isA(TDict))   return gen(env, ConvDictToVArr, src);
      if (src->isA(TShape))  return gen(env, ConvShapeToVArr, src);
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToVArr, src);
      if (src->isA(TObj))    return gen(env, ConvObjToVArr, src);
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TRes))    return raise("Resource");
      // Unexpected types may only be seen in unreachable code.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }()
  );
}

void emitCastDArray(IRGS& env) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);

  auto const src = popC(env);

  auto const raise = [&](const char* type) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(
        env,
        makeStaticString(folly::sformat("{} to darray conversion", type))
      )
    );
    return cns(env, TBottom);
  };

  push(
    env,
    [&] {
      if (src->isA(TArr)) {
        return cond(
          env,
          [&](Block* taken) { return gen(env, CheckDArray, taken, src); },
          [&](SSATmp* darr) { return darr; },
          [&]{ return gen(env, ConvArrToDArr, src); }
        );
      }
      if (src->isA(TVec))    return gen(env, ConvVecToDArr, src);
      if (src->isA(TDict))   return gen(env, ConvDictToDArr, src);
      if (src->isA(TShape))  return gen(env, ConvShapeToDArr, src);
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToDArr, src);
      if (src->isA(TObj))    return gen(env, ConvObjToDArr, src);
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TRes))    return raise("Resource");
      // Unexpected types may only be seen in unreachable code.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }()
  );
}

void emitCastVec(IRGS& env) {
  auto const src = popC(env);

  auto const raise = [&](const char* type) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(
        env,
        makeStaticString(folly::sformat("{} to vec conversion", type))
      )
    );
    return cns(env, TBottom);
  };

  push(
    env,
    [&] {
      if (src->isA(TVec))    return src;
      if (src->isA(TArr))    return gen(env, ConvArrToVec, src);
      if (src->isA(TDict))   return gen(env, ConvDictToVec, src);
      if (src->isA(TShape))  return gen(env, ConvShapeToVec, src);
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToVec, src);
      if (src->isA(TObj))    return gen(env, ConvObjToVec, src);
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TRes))    return raise("Resource");
      // Unexpected types may only be seen in unreachable code.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }()
  );
}

void emitCastDict(IRGS& env) {
  auto const src = popC(env);

  auto const raise = [&](const char* type) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(
        env,
        makeStaticString(folly::sformat("{} to dict conversion", type))
      )
    );
    return cns(env, TBottom);
  };

  push(
    env,
    [&] {
      if (src->isA(TDict))    return src;
      if (src->isA(TShape))   return gen(env, ConvShapeToDict, src);
      if (src->isA(TArr))     return gen(env, ConvArrToDict, src);
      if (src->isA(TVec))     return gen(env, ConvVecToDict, src);
      if (src->isA(TKeyset))  return gen(env, ConvKeysetToDict, src);
      if (src->isA(TObj))     return gen(env, ConvObjToDict, src);
      if (src->isA(TNull))    return raise("Null");
      if (src->isA(TBool))    return raise("Bool");
      if (src->isA(TInt))     return raise("Int");
      if (src->isA(TDbl))     return raise("Double");
      if (src->isA(TStr))     return raise("String");
      if (src->isA(TRes))     return raise("Resource");
      // Unexpected types may only be seen in unreachable code.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }()
  );
}

void emitCastKeyset(IRGS& env) {
  auto const src = popC(env);

  auto const raise = [&](const char* type) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(
        env,
        makeStaticString(folly::sformat("{} to keyset conversion", type))
      )
    );
    return cns(env, TBottom);
  };

  push(
    env,
    [&] {
      if (src->isA(TKeyset))  return src;
      if (src->isA(TArr))     return gen(env, ConvArrToKeyset, src);
      if (src->isA(TVec))     return gen(env, ConvVecToKeyset, src);
      if (src->isA(TDict))    return gen(env, ConvDictToKeyset, src);
      if (src->isA(TShape))   return gen(env, ConvShapeToKeyset, src);
      if (src->isA(TObj))     return gen(env, ConvObjToKeyset, src);
      if (src->isA(TNull))    return raise("Null");
      if (src->isA(TBool))    return raise("Bool");
      if (src->isA(TInt))     return raise("Int");
      if (src->isA(TDbl))     return raise("Double");
      if (src->isA(TStr))     return raise("String");
      if (src->isA(TRes))     return raise("Resource");
      // Unexpected types may only be seen in unreachable code.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }()
  );
}

void emitCastBool(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToBool, src));
  decRef(env, src);
}

void emitCastDouble(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToDbl, src));
  decRef(env, src);
}

void emitCastInt(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToInt, src));
  decRef(env, src);
}

void emitCastObject(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToObj, src));
}

void emitCastString(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToStr, src));
  decRef(env, src);
}

//////////////////////////////////////////////////////////////////////

void emitDblAsBits(IRGS& env) {
  auto const src = popC(env);
  if (!src->isA(TDbl)) {
    push(env, cns(env, 0));
    return;
  }
  push(env, gen(env, DblAsBits, src));
}

//////////////////////////////////////////////////////////////////////

void implIncStat(IRGS& env, uint32_t counter) {
  if (!Stats::enabled()) return;
  gen(env, IncStat, cns(env, counter));
}

//////////////////////////////////////////////////////////////////////

void emitDiscardClsRef(IRGS& env, uint32_t slot)   { killClsRef(env, slot); }

void emitPopC(IRGS& env)   { popDecRef(env, DataTypeGeneric); }
void emitPopV(IRGS& env)   { popDecRef(env, DataTypeGeneric); }
void emitPopU(IRGS& env)   { popU(env); }
void emitPopU2(IRGS& env) {
  auto const src = popC(env, DataTypeGeneric);
  popU(env);
  push(env, src);
}

void emitPopL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = popC(env, DataTypeGeneric);
  stLocMove(env, id, ldrefExit, ldPMExit, src);
}

void emitDir(IRGS& env)    { push(env, cns(env, curUnit(env)->dirpath())); }
void emitFile(IRGS& env)   { push(env, cns(env, curUnit(env)->filepath())); }
void emitMethod(IRGS& env) { push(env, cns(env, curFunc(env)->fullName())); }
void emitDup(IRGS& env)    { pushIncRef(env, topC(env)); }

//////////////////////////////////////////////////////////////////////

void emitArray(IRGS& env, const ArrayData* x) {
  assertx(x->isPHPArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || x->isNotDVArray());
  push(env, cns(env, x));
}

void emitVec(IRGS& env, const ArrayData* x) {
  assertx(x->isVecArray());
  push(env, cns(env, x));
}

void emitDict(IRGS& env, const ArrayData* x) {
  assertx(x->isDict());
  push(env, cns(env, x));
}

void emitKeyset(IRGS& env, const ArrayData* x) {
  assertx(x->isKeyset());
  push(env, cns(env, x));
}

void emitString(IRGS& env, const StringData* s) { push(env, cns(env, s)); }
void emitInt(IRGS& env, int64_t val)            { push(env, cns(env, val)); }
void emitDouble(IRGS& env, double val)          { push(env, cns(env, val)); }
void emitTrue(IRGS& env)                        { push(env, cns(env, true)); }
void emitFalse(IRGS& env)                       { push(env, cns(env, false)); }

void emitNull(IRGS& env)       { push(env, cns(env, TInitNull)); }
void emitNullUninit(IRGS& env) { push(env, cns(env, TUninit)); }

//////////////////////////////////////////////////////////////////////

void emitNop(IRGS&)                {}
void emitEntryNop(IRGS&)           {}
void emitCGetCUNop(IRGS& env) {
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  auto const knownType = env.irb->stack(offset, DataTypeSpecific).type;
  assertTypeStack(env, BCSPRelOffset{0}, knownType & TInitCell);
}
void emitUGetCUNop(IRGS& env) {
  assertTypeStack(env, BCSPRelOffset{0}, TUninit);
}
void emitDefClsNop(IRGS&, uint32_t){}
void emitBreakTraceHint(IRGS&)     {}

//////////////////////////////////////////////////////////////////////

}}}
