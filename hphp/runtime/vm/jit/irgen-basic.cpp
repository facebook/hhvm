/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-guards.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void implAGet(IRGS& env, SSATmp* classSrc) {
  if (classSrc->type() <= TStr) {
    push(env, ldCls(env, classSrc));
    return;
  }
  push(env, gen(env, LdObjClass, classSrc));
}

void checkThis(IRGS& env, SSATmp* ctx) {
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckCtxThis, taken, ctx);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const err = cns(env, makeStaticString(Strings::FATAL_NULL_THIS));
      gen(env, RaiseError, err);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

void emitAGetC(IRGS& env) {
  auto const name = topC(env);
  if (name->type().subtypeOfAny(TObj, TStr)) {
    popC(env);
    implAGet(env, name);
    gen(env, DecRef, name);
  } else {
    interpOne(env, TCls, 1);
  }
}

void emitAGetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (src->type().subtypeOfAny(TObj, TStr)) {
    implAGet(env, src);
  } else {
    PUNT(AGetL);
  }
}

void emitCGetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const loc = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeCountnessInit
  );
  pushIncRef(env, loc);
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
  auto const oldTop = pop(env);
  auto const val = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeCountnessInit
  );
  pushIncRef(env, val);
  push(env, oldTop);
}

void emitVGetL(IRGS& env, int32_t id) {
  auto value = ldLoc(env, id, makeExit(env), DataTypeCountnessInit);
  auto const t = value->type();

  if (t <= TCell) {
    if (value->isA(TUninit)) {
      value = cns(env, TInitNull);
    }
    value = gen(env, Box, value);
    stLocRaw(env, id, fp(env), value);
  } else if (t.maybe(TCell)) {
    value = cond(env,
                 [&](Block* taken) {
                   return gen(env, CheckType, TBoxedCell, taken, value);
                 },
                 [&](SSATmp* box) { // Next: value is Boxed
                   return gen(env, AssertType, TBoxedCell, box);
                 },
                 [&] { // Taken: value is not Boxed
                   auto const tmpType = t - TBoxedCell;
                   assertx(tmpType <= TCell);
                   auto const tmp = gen(env, AssertType, tmpType, value);
                   return gen(env, Box, tmp);
                 });
  }
  pushIncRef(env, value);
}

void emitUnsetL(IRGS& env, int32_t id) {
  auto const prev = ldLoc(env, id, makeExit(env), DataTypeCountness);
  stLocRaw(env, id, fp(env), cns(env, TUninit));
  gen(env, DecRef, prev);
}

void emitBindL(IRGS& env, int32_t id) {
  if (curFunc(env)->isPseudoMain()) {
    interpOne(env, TBoxedInitCell, 1);
    return;
  }

  auto const ldPMExit = makePseudoMainExit(env);
  auto const newValue = popV(env);
  // Note that the IncRef must happen first, for correctness in a
  // pseudo-main: the destructor could decref the value again after
  // we've stored it into the local.
  pushIncRef(env, newValue);
  auto const oldValue = ldLoc(env, id, ldPMExit, DataTypeSpecific);
  stLocRaw(env, id, fp(env), newValue);
  gen(env, DecRef, oldValue);
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
  if (!curClass(env)) {
    // Do nothing if this is null
    return;
  }
  auto const ldrefExit = makeExit(env);
  auto const oldLoc    = ldLoc(env, id, ldrefExit, DataTypeCountness);
  auto const ctx       = gen(env, LdCtx, fp(env));
  gen(env, CheckCtxThis, makeExitSlow(env), ctx);
  auto const this_     = gen(env, CastCtxThis, ctx);
  gen(env, IncRef, this_);
  stLocRaw(env, id, fp(env), this_);
  gen(env, DecRef, oldLoc);
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
  gen(env, DecRef, srcBox);
}

void emitThis(IRGS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  checkThis(env, ctx);
  auto const this_ = gen(env, CastCtxThis, ctx);
  pushIncRef(env, this_);
}

void emitCheckThis(IRGS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  checkThis(env, ctx);
}

void emitBareThis(IRGS& env, BareThisOp subop) {
  if (!curClass(env)) {
    interpOne(env, TInitNull, 0); // will raise notice and push null
    return;
  }
  auto const ctx = gen(env, LdCtx, fp(env));
  if (subop == BareThisOp::NeverNull) {
    env.irb->setThisAvailable();
  } else {
    gen(env, CheckCtxThis, makeExitSlow(env), ctx);
  }
  pushIncRef(env, gen(env, CastCtxThis, ctx));
}

void emitClone(IRGS& env) {
  if (!topC(env)->isA(TObj)) PUNT(Clone-NonObj);
  auto const obj        = popC(env);
  push(env, gen(env, Clone, obj));
  gen(env, DecRef, obj);
}

void emitLateBoundCls(IRGS& env) {
  auto const clss = curClass(env);
  if (!clss) {
    // no static context class, so this will raise an error
    interpOne(env, TCls, 0);
    return;
  }
  auto const ctx = ldCtx(env);
  push(env, gen(env, LdClsCtx, ctx));
}

void emitSelf(IRGS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr) {
    interpOne(env, TCls, 0);
  } else {
    push(env, cns(env, clss));
  }
}

void emitParent(IRGS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr || clss->parent() == nullptr) {
    interpOne(env, TCls, 0);
  } else {
    push(env, cns(env, clss->parent()));
  }
}

void emitNameA(IRGS& env) {
  push(env, gen(env, LdClsName, popA(env)));
}

//////////////////////////////////////////////////////////////////////

void emitCastArray(IRGS& env) {
  auto const src = popC(env);
  push(
    env,
    [&] {
      if (src->isA(TArr))  return src;
      if (src->isA(TNull)) return cns(env, staticEmptyArray());
      if (src->isA(TBool)) return gen(env, ConvBoolToArr, src);
      if (src->isA(TDbl))  return gen(env, ConvDblToArr, src);
      if (src->isA(TInt))  return gen(env, ConvIntToArr, src);
      if (src->isA(TStr))  return gen(env, ConvStrToArr, src);
      if (src->isA(TObj))  return gen(env, ConvObjToArr, src);
      return gen(env, ConvCellToArr, src);
    }()
  );
}

void emitCastBool(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToBool, src));
  gen(env, DecRef, src);
}

void emitCastDouble(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToDbl, src));
  gen(env, DecRef, src);
}

void emitCastInt(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToInt, src));
  gen(env, DecRef, src);
}

void emitCastObject(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToObj, src));
}

void emitCastString(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToStr, src));
  gen(env, DecRef, src);
}

void emitIncStat(IRGS& env, int32_t counter, int32_t value) {
  if (!Stats::enabled()) return;
  gen(env, IncStat, cns(env, counter), cns(env, value), cns(env, false));
}

//////////////////////////////////////////////////////////////////////

void emitPopA(IRGS& env) { popA(env); }
void emitPopC(IRGS& env) { popDecRef(env, DataTypeGeneric); }
void emitPopV(IRGS& env) { popDecRef(env, DataTypeGeneric); }
void emitPopR(IRGS& env) { popDecRef(env, DataTypeGeneric); }

void emitDir(IRGS& env)  { push(env, cns(env, curUnit(env)->dirpath())); }
void emitFile(IRGS& env) { push(env, cns(env, curUnit(env)->filepath())); }

void emitDup(IRGS& env) { pushIncRef(env, topC(env)); }

//////////////////////////////////////////////////////////////////////

void emitArray(IRGS& env, const ArrayData* x)   { push(env, cns(env, x)); }
void emitString(IRGS& env, const StringData* s) { push(env, cns(env, s)); }
void emitInt(IRGS& env, int64_t val)            { push(env, cns(env, val)); }
void emitDouble(IRGS& env, double val)          { push(env, cns(env, val)); }
void emitTrue(IRGS& env)                        { push(env, cns(env, true)); }
void emitFalse(IRGS& env)                       { push(env, cns(env, false)); }

void emitNull(IRGS& env)       { push(env, cns(env, TInitNull)); }
void emitNullUninit(IRGS& env) { push(env, cns(env, TUninit)); }

//////////////////////////////////////////////////////////////////////

void emitNop(IRGS&)                {}
void emitBoxRNop(IRGS&)            {}
void emitUnboxRNop(IRGS&)          {}
void emitRGetCNop(IRGS&)           {}
void emitFPassC(IRGS&, int32_t)    {}
void emitFPassVNop(IRGS&, int32_t) {}
void emitDefClsNop(IRGS&, Id)      {}
void emitBreakTraceHint(IRGS&)     {}

//////////////////////////////////////////////////////////////////////

}}}
