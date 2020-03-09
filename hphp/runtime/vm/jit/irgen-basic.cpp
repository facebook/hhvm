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

#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"
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

void emitClassGetC(IRGS& env) {
  auto const name = topC(env);
  if (!name->type().subtypeOfAny(TObj, TCls, TStr)) {
    interpOne(env);
    return;
  }
  popC(env);

  if (name->isA(TCls)) {
    push(env, name);
    return;
  }

  if (name->isA(TObj)) {
    push(env, gen(env, LdObjClass, name));
    decRef(env, name);
    return;
  }

  if (!name->hasConstVal()) gen(env, RaiseStrToClassNotice, name);

  auto const cls = ldCls(env, name);
  decRef(env, name);
  push(env, cls);
}

void emitClassGetTS(IRGS& env) {
  auto const required_ts_type = RuntimeOption::EvalHackArrDVArrs ? TDict : TArr;
  auto const ts = topC(env);
  if (!ts->isA(required_ts_type)) {
    if (ts->type().maybe(required_ts_type)) {
      PUNT(ClassGetTS-UnguardedTS);
    } else {
      gen(env, RaiseError, cns(env, s_reified_type_must_be_ts.get()));
    }
  }

  auto const val = gen(
    env,
    RuntimeOption::EvalHackArrDVArrs ? AKExistsDict : AKExistsArr,
    ts,
    cns(env, s_generic_types.get())
  );
  // Side-exit for now if it has reified generics
  gen(env, JmpNZero, makeExitSlow(env), val);

  auto const clsName = profiledArrayAccess(
    env, ts, cns(env, s_classname.get()),
    [&] (SSATmp* base, SSATmp* key, uint32_t pos) {
      return gen(
        env,
        RuntimeOption::EvalHackArrDVArrs
          ? DictGetK
          : MixedArrayGetK,
        IndexData { pos }, base, key
      );
    },
    [&] (SSATmp* key, SizeHintData) {
      if (RuntimeOption::EvalHackArrDVArrs) return gen(env, DictGet, ts, key);
      return gen(env, ArrayGet, MOpModeData { MOpMode::Warn }, ts, key);
    }
  );

  SSATmp* name;
  ifThen(
    env,
    [&] (Block* taken) {
      name = gen(env, CheckType, TStr, taken, clsName);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseError, cns(env, s_new_instance_of_not_string.get()));
    }
  );

  auto const cls = ldCls(env, name);
  popDecRef(env);
  push(env, cls);
  push(env, cns(env, TInitNull));
}

void emitCGetL(IRGS& env, NamedLocal loc) {
  auto const ldPMExit = makePseudoMainExit(env);
  auto const value = ldLocWarn(
    env,
    loc,
    ldPMExit,
    DataTypeCountnessInit
  );
  pushIncRef(env, value);
}

void emitCGetQuietL(IRGS& env, int32_t id) {
  auto const ldPMExit = makePseudoMainExit(env);
  pushIncRef(
    env,
    [&] {
      auto const loc = ldLoc(
        env,
        id,
        ldPMExit,
        DataTypeCountnessInit
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
            return gen(env, AssertType, TInitCell, loc);
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
  auto const ldPMExit = makePseudoMainExit(env);
  pushIncRef(env, ldLoc(env, id, ldPMExit, DataTypeGeneric));
}

void emitPushL(IRGS& env, int32_t id) {
  assertTypeLocal(env, id, TInitCell);  // bytecode invariant
  auto* locVal = ldLoc(env, id, makeExit(env), DataTypeGeneric);
  push(env, locVal);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
}

void emitCGetL2(IRGS& env, NamedLocal loc) {
  auto const ldPMExit = makePseudoMainExit(env);
  auto const oldTop = pop(env, DataTypeGeneric);
  auto const val = ldLocWarn(
    env,
    loc,
    ldPMExit,
    DataTypeCountnessInit
  );
  pushIncRef(env, val);
  push(env, oldTop);
}

void emitUnsetL(IRGS& env, int32_t id) {
  auto const prev = ldLoc(env, id, makeExit(env), DataTypeCountness);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
  decRef(env, prev);
}

void emitSetL(IRGS& env, int32_t id) {
  auto const ldPMExit = makePseudoMainExit(env);

  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(env, DataTypeGeneric);
  pushStLoc(env, id, ldPMExit, src);
}

void emitInitThisLoc(IRGS& env, int32_t id) {
  if (!hasThis(env)) {
    // Do nothing if this is null
    return;
  }
  auto const ldExit = makeExit(env);
  auto const oldLoc = ldLoc(env, id, ldExit, DataTypeCountness);
  auto const this_  = ldThis(env);
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

void emitThis(IRGS& env) {
  auto const this_ = checkAndLoadThis(env);
  pushIncRef(env, this_);
}

void emitCheckThis(IRGS& env) {
  checkAndLoadThis(env);
}

void emitBareThis(IRGS& env, BareThisOp subop) {
  if (!hasThis(env)) {
    if (subop == BareThisOp::NoNotice) {
      push(env, cns(env, TInitNull));
      return;
    }
    assertx(subop != BareThisOp::NeverNull);
    interpOne(env, TInitNull, 0); // will raise notice and push null
    return;
  }

  pushIncRef(env, ldThis(env));
}

void emitClone(IRGS& env) {
  if (!topC(env)->isA(TObj)) PUNT(Clone-NonObj);
  auto const obj        = popC(env);
  push(env, gen(env, Clone, obj));
  decRef(env, obj);
}

void emitLateBoundCls(IRGS& env) {
  auto const clss = curClass(env);
  if (!clss) {
    // no static context class, so this will raise an error
    interpOne(env);
    return;
  }
  push(env, ldCtxCls(env));
}

void emitSelf(IRGS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr) {
    interpOne(env);
  } else {
    push(env, cns(env, clss));
  }
}

void emitParent(IRGS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr || clss->parent() == nullptr) {
    interpOne(env);
  } else {
    push(env, cns(env, clss->parent()));
  }
}

void emitClassName(IRGS& env) {
  auto const cls = popC(env);
  if (!cls->isA(TCls)) PUNT(ClassName-NotClass);
  push(env, gen(env, LdClsName, cls));
}

//////////////////////////////////////////////////////////////////////

void emitCastArray(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvTVToArr, src));
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
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToVArr, src);
      if (src->isA(TClsMeth)) return gen(env, ConvClsMethToVArr, src);
      if (src->isA(TObj))    return gen(env, ConvObjToVArr, src);
      if (src->isA(TRecord)) PUNT(CastVArrayRecord); // TODO: T53309767
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TFunc))   return raise("Func");
      if (src->isA(TRes))    return raise("Resource");
      PUNT(CastVArrayUnknown);
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
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToDArr, src);
      if (src->isA(TClsMeth)) return gen(env, ConvClsMethToDArr, src);
      if (src->isA(TObj))    return gen(env, ConvObjToDArr, src);
      if (src->isA(TRecord)) PUNT(CastDArrayRecord); // TODO: T53309767
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TFunc))   return raise("Func");
      if (src->isA(TRes))    return raise("Resource");
      PUNT(CastDArrayUnknown);
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
      if (src->isA(TKeyset)) return gen(env, ConvKeysetToVec, src);
      if (src->isA(TClsMeth)) return gen(env, ConvClsMethToVec, src);
      if (src->isA(TObj))    return gen(env, ConvObjToVec, src);
      if (src->isA(TRecord)) PUNT(CastVecRecord); // TODO: T53309767
      if (src->isA(TNull))   return raise("Null");
      if (src->isA(TBool))   return raise("Bool");
      if (src->isA(TInt))    return raise("Int");
      if (src->isA(TDbl))    return raise("Double");
      if (src->isA(TStr))    return raise("String");
      if (src->isA(TFunc))   return raise("Func");
      if (src->isA(TRes))    return raise("Resource");
      PUNT(CastVecUnknown);
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
      if (src->isA(TArr))     return gen(env, ConvArrToDict, src);
      if (src->isA(TVec))     return gen(env, ConvVecToDict, src);
      if (src->isA(TKeyset))  return gen(env, ConvKeysetToDict, src);
      if (src->isA(TClsMeth)) return gen(env, ConvClsMethToDict, src);
      if (src->isA(TObj))     return gen(env, ConvObjToDict, src);
      if (src->isA(TRecord))  PUNT(CastDictRecord); // TODO: T53309767
      if (src->isA(TNull))    return raise("Null");
      if (src->isA(TBool))    return raise("Bool");
      if (src->isA(TInt))     return raise("Int");
      if (src->isA(TDbl))     return raise("Double");
      if (src->isA(TStr))     return raise("String");
      if (src->isA(TFunc))    return raise("Func");
      if (src->isA(TRes))     return raise("Resource");
      PUNT(CastDictUnknown);
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
      if (src->isA(TClsMeth)) return gen(env, ConvClsMethToKeyset, src);
      if (src->isA(TObj))     return gen(env, ConvObjToKeyset, src);
      if (src->isA(TRecord))  PUNT(CastKeysetRecord); // TODO: T53309767
      if (src->isA(TNull))    return raise("Null");
      if (src->isA(TBool))    return raise("Bool");
      if (src->isA(TInt))     return raise("Int");
      if (src->isA(TDbl))     return raise("Double");
      if (src->isA(TStr))     return raise("String");
      if (src->isA(TFunc))    return raise("Func");
      if (src->isA(TRes))     return raise("Resource");
      PUNT(CastKeysetUnknown);
    }()
  );
}

void emitCastBool(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvTVToBool, src));
  decRef(env, src);
}

void emitCastDouble(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvTVToDbl, src));
  decRef(env, src);
}

void emitCastInt(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvTVToInt, src));
  decRef(env, src);
}

void emitCastString(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvTVToStr, src));
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

void emitPopC(IRGS& env)   { popDecRef(env, DataTypeGeneric); }
void emitPopU(IRGS& env)   { popU(env); }
void emitPopU2(IRGS& env) {
  auto const src = popC(env, DataTypeGeneric);
  popU(env);
  push(env, src);
}

void emitPopL(IRGS& env, int32_t id) {
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = popC(env, DataTypeGeneric);
  stLocMove(env, id, ldPMExit, src);
}

void emitPopFrame(IRGS& env, uint32_t nout) {
  jit::vector<SSATmp*> v{nout, nullptr};
  for (auto i = nout; i > 0; --i) v[i - 1] = pop(env, DataTypeGeneric);
  for (uint32_t i = 0; i < 3; ++i) popU(env);
  for (auto tmp : v) push(env, tmp);
}

void emitDir(IRGS& env)    { push(env, cns(env, curUnit(env)->dirpath())); }
void emitFile(IRGS& env)   { push(env, cns(env, curUnit(env)->filepath())); }
void emitMethod(IRGS& env) { push(env, cns(env, curFunc(env)->fullName())); }
void emitDup(IRGS& env)    { pushIncRef(env, topC(env)); }

void emitFuncCred(IRGS& env) {
  push(env, gen(env, FuncCred, cns(env, curFunc(env))));
}
//////////////////////////////////////////////////////////////////////

void emitArray(IRGS& env, const ArrayData* x) {
  assertx(x->isPHPArrayType());
  assertx(!RuntimeOption::EvalHackArrDVArrs || x->isNotDVArray());
  push(env, cns(env, x));
}

void emitVec(IRGS& env, const ArrayData* x) {
  assertx(x->isVecArrayType());
  push(env, cns(env, x));
}

void emitDict(IRGS& env, const ArrayData* x) {
  assertx(x->isDictType());
  push(env, cns(env, x));
}

void emitKeyset(IRGS& env, const ArrayData* x) {
  assertx(x->isKeysetType());
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
