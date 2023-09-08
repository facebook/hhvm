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

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/unit-util.h"

namespace HPHP::jit::irgen {

void emitClassGetC(IRGS& env) {
  auto const name = topC(env);
  if (!name->type().subtypeOfAny(TObj, TCls, TStr, TLazyCls)) {
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

  if (name->isA(TStr) && RO::EvalRaiseStrToClsConversionWarning) {
    gen(env, RaiseStrToClassNotice, name);
  }

  auto const cls = ldCls(env, name);
  decRef(env, name);
  if (name->isA(TStr)) emitModuleBoundaryCheck(env, cls, false);
  push(env, cls);
}

void emitClassGetTS(IRGS& env) {
  auto const ts = topC(env);
  if (!ts->isA(TDict)) { if (ts->type().maybe(TDict)) { PUNT(ClassGetTS-UnguardedTS); } else {
      gen(env, RaiseError, cns(env, s_reified_type_must_be_ts.get()));
    }
  }

  auto const val = gen(
    env,
    AKExistsDict,
    ts,
    cns(env, s_generic_types.get())
  );
  // Side-exit for now if it has reified generics
  gen(env, JmpNZero, makeExitSlow(env), val);

  int locId = 0;
  auto const finish = [&] (SSATmp* clsName) {
    auto const name = cond(
      env,
      [&] (Block* taken) {
        return gen(env, CheckType, TStr, taken, clsName);
      },
      [&] (SSATmp* name) { return name; },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(env, RaiseError, cns(env, s_new_instance_of_not_string.get()));
        return cns(env, TBottom);
      }
    );
    auto const cls = ldCls(env, name);
    popDecRef(env, static_cast<DecRefProfileId>(locId++));
    push(env, cls);
    push(env, cns(env, TInitNull));
  };

  auto const clsName = profiledArrayAccess(
    env, ts, cns(env, s_classname.get()), MOpMode::Warn,
    [&] (SSATmp* base, SSATmp* key, SSATmp* pos) {
      return gen(env, DictGetK, base, key, pos);
    },
    [&] (SSATmp* key) {
      gen(env, ThrowArrayKeyException, ts, key);
      return cns(env, TBottom);
    },
    [&] (SSATmp* key, SizeHintData) {
      return gen(env, DictGet, ts, key);
    },
    finish
  );
  finish(clsName);
}

void emitCGetL(IRGS& env, NamedLocal loc) {
  auto const value = ldLocWarn(env, loc, DataTypeCountnessInit);
  pushIncRef(env, value);
}

void emitCGetQuietL(IRGS& env, int32_t id) {
  pushIncRef(
    env,
    [&] {
      auto const loc = ldLoc(env, id, DataTypeCountnessInit);

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
  pushIncRef(env, ldLoc(env, id, DataTypeGeneric));
}

void emitPushL(IRGS& env, int32_t id) {
  assertTypeLocal(env, id, TInitCell);  // bytecode invariant
  auto* locVal = ldLoc(env, id, DataTypeGeneric);
  push(env, locVal);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
}

void emitCGetL2(IRGS& env, NamedLocal loc) {
  auto const oldTop = pop(env, DataTypeGeneric);
  auto const val = ldLocWarn(env, loc, DataTypeCountnessInit);
  pushIncRef(env, val);
  push(env, oldTop);
}

void emitUnsetL(IRGS& env, int32_t id) {
  auto const prev = ldLoc(env, id, DataTypeGeneric);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
  decRef(env, prev);
}

void emitSetL(IRGS& env, int32_t id) {
  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(env, DataTypeGeneric);
  pushStLoc(env, id, src);
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

void emitSelfCls(IRGS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr) {
    interpOne(env);
  } else {
    push(env, cns(env, clss));
  }
}

void emitParentCls(IRGS& env) {
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

void emitLazyClassFromClass(IRGS& env) {
  auto const cls = popC(env);
  if (!cls->isA(TCls)) PUNT(LazyClassFromClass-NotClass);
  push(env, gen(env, LdLazyCls, cls));
}

const StaticString
  s_name_of_not_enum_class_label("Attempting to get name of non enum class label");

void emitEnumClassLabelName(IRGS& env) {
  auto const tv = popC(env);
  if (tv->isA(TEnumClassLabel)) {
    push(env, gen(env, LdEnumClassLabelName, tv));
    return;
  }
  gen(env, RaiseError, cns(env, s_name_of_not_enum_class_label.get()));
}

//////////////////////////////////////////////////////////////////////

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
      if (src->isA(TVec))     return src;
      if (src->isA(TArrLike)) return gen(env, ConvArrLikeToVec, src);
      if (src->isA(TClsMeth)) return raise("ClsMeth");
      if (src->isA(TObj))     return gen(env, ConvObjToVec, src);
      if (src->isA(TNull))    return raise("Null");
      if (src->isA(TBool))    return raise("Bool");
      if (src->isA(TInt))     return raise("Int");
      if (src->isA(TDbl))     return raise("Double");
      if (src->isA(TStr))     return raise("String");
      if (src->isA(TFunc))    return raise("Func");
      if (src->isA(TRes))     return raise("Resource");
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
      if (src->isA(TArrLike)) return gen(env, ConvArrLikeToDict, src);
      if (src->isA(TClsMeth)) return raise("ClsMeth");
      if (src->isA(TObj))     return gen(env, ConvObjToDict, src);
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
      if (src->isA(TArrLike)) return gen(env, ConvArrLikeToKeyset, src);
      if (src->isA(TClsMeth)) return raise("ClsMeth");
      if (src->isA(TObj))     return gen(env, ConvObjToKeyset, src);
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
  push(env, gen(env, ConvTVToStr, ConvNoticeData{}, src));
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

void emitPopC(IRGS& env)   { popDecRef(env, DecRefProfileId::Default, DataTypeGeneric); }
void emitPopU(IRGS& env)   { popU(env); }
void emitPopU2(IRGS& env) {
  auto const src = popC(env, DataTypeGeneric);
  popU(env);
  push(env, src);
}

void emitPopL(IRGS& env, int32_t id) {
  auto const src = popC(env, DataTypeGeneric);
  stLocMove(env, id, src);
}

void emitDir(IRGS& env) {
  auto const unit = curUnit(env);
  auto const handle = unit->perRequestFilepathHandle();
  if (handle != rds::kUninitHandle) {
    assertx(!RuntimeOption::RepoAuthoritative);
    assertx(RuntimeOption::EvalReuseUnitsByHash);
    auto const filepath =
      gen(env, LdUnitPerRequestFilepath, RDSHandleData { handle });
    push(env, gen(env, DirFromFilepath, filepath));
    return;
  }
  auto const p = [&] {
    if (auto const of = curFunc(env)->originalFilename()) {
      return of;
    }
    return unit->origFilepath();
  }();
  push(env, cns(env, makeStaticString(FileUtil::dirname(StrNR{p}))));
}

void emitFile(IRGS& env) {
  auto const unit = curUnit(env);
  auto const handle = unit->perRequestFilepathHandle();
  if (handle != rds::kUninitHandle) {
    assertx(!RuntimeOption::RepoAuthoritative);
    assertx(RuntimeOption::EvalReuseUnitsByHash);
    assertx(!curFunc(env)->originalFilename());
    push(env, gen(env, LdUnitPerRequestFilepath, RDSHandleData { handle }));
    return;
  }
  if (auto const of = curFunc(env)->originalFilename()) {
    push(env, cns(env, of));
    return;
  }
  push(env, cns(env, unit->origFilepath()));
}

void emitMethod(IRGS& env) { push(env, cns(env, curFunc(env)->fullName())); }
void emitDup(IRGS& env)    { pushIncRef(env, topC(env)); }

void emitFuncCred(IRGS& env) {
  push(env, gen(env, FuncCred, cns(env, curFunc(env))));
}
//////////////////////////////////////////////////////////////////////

void emitVec(IRGS& env, const ArrayData* x) {
  assertx(x->isVecType());
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

void emitLazyClass(IRGS& env, const StringData* name) {
  push(env, cns(env, LazyClassData::create(name)));
}

void emitEnumClassLabel(IRGS& env, const StringData* name) {
  push(env, cns(env, make_tv<KindOfEnumClassLabel>(name)));
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
void emitCGetCUNop(IRGS& env) {
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  auto const knownType = env.irb->stack(offset, DataTypeSpecific).type;
  assertTypeStack(env, BCSPRelOffset{0}, knownType & TInitCell);
}
void emitUGetCUNop(IRGS& env) {
  assertTypeStack(env, BCSPRelOffset{0}, TUninit);
}
void emitBreakTraceHint(IRGS&)     {}

//////////////////////////////////////////////////////////////////////

void emitResolveClass(IRGS& env, const StringData* name) {
  if (auto const cls = lookupUniqueClass(env, name)) {
    emitModuleBoundaryCheckKnown(env, cls);
    push(env, cns(env, cls));
    return;
  }
  interpOne(env);
}

}
