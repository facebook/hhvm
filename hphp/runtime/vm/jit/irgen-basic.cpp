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

void implAGet(HTS& env, SSATmp* classSrc) {
  if (classSrc->type() <= Type::Str) {
    push(env, ldCls(env, classSrc));
    return;
  }
  push(env, gen(env, LdObjClass, classSrc));
}

void checkThis(HTS& env, SSATmp* ctx) {
  env.irb->ifThen(
    [&] (Block* taken) {
      gen(env, CheckCtxThis, taken, ctx);
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      auto const err = cns(env, makeStaticString(Strings::FATAL_NULL_THIS));
      gen(env, RaiseError, err);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

void emitAGetC(HTS& env) {
  auto const name = topC(env);
  if (name->type().subtypeOfAny(Type::Obj, Type::Str)) {
    popC(env);
    implAGet(env, name);
    gen(env, DecRef, name);
  } else {
    interpOne(env, Type::Cls, 1);
  }
}

void emitAGetL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (src->type().subtypeOfAny(Type::Obj, Type::Str)) {
    implAGet(env, src);
  } else {
    PUNT(AGetL);
  }
}

void emitCGetL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  // Mimic hhbc guard relaxation for now.
  auto cat = curSrcKey(env).op() == OpFPassL ? DataTypeSpecific
                                             : DataTypeCountnessInit;
  pushIncRef(env, ldLocInnerWarn(env, id, ldrefExit, ldPMExit, cat));
}

void emitPushL(HTS& env, int32_t id) {
  assertTypeLocal(env, id, Type::InitCell);  // bytecode invariant
  auto* locVal = ldLoc(env, id, makeExit(env), DataTypeGeneric);
  push(env, locVal);
  stLocRaw(env, id, fp(env), cns(env, Type::Uninit));
}

void emitCGetL2(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const oldTop = pop(env, Type::StackElem);
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

void emitVGetL(HTS& env, int32_t id) {
  auto value = ldLoc(env, id, makeExit(env), DataTypeCountnessInit);
  auto const t = value->type();
  always_assert(t.isBoxed() || t.notBoxed());

  if (t.notBoxed()) {
    if (value->isA(Type::Uninit)) {
      value = cns(env, Type::InitNull);
    }
    value = gen(env, Box, value);
    stLocRaw(env, id, fp(env), value);
  }
  pushIncRef(env, value);
}

void emitUnsetL(HTS& env, int32_t id) {
  auto const prev = ldLoc(env, id, makeExit(env), DataTypeCountness);
  stLocRaw(env, id, fp(env), cns(env, Type::Uninit));
  gen(env, DecRef, prev);
}

void emitBindL(HTS& env, int32_t id) {
  if (curFunc(env)->isPseudoMain()) {
    interpOne(env, Type::BoxedCell, 1);
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

void emitSetL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);

  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(env, DataTypeGeneric);
  pushStLoc(env, id, ldrefExit, ldPMExit, src);
}

void emitInitThisLoc(HTS& env, int32_t id) {
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

void emitPrint(HTS& env) {
  auto const type = topC(env)->type();
  if (!type.subtypeOfAny(Type::Int, Type::Bool, Type::Null, Type::Str)) {
    interpOne(env, Type::Int, 1);
    return;
  }

  auto const cell = popC(env);

  Opcode op;
  if (type <= Type::Str) {
    op = PrintStr;
  } else if (type <= Type::Int) {
    op = PrintInt;
  } else if (type <= Type::Bool) {
    op = PrintBool;
  } else {
    assert(type <= Type::Null);
    op = Nop;
  }
  // the print helpers decref their arg, so don't decref pop'ed value
  if (op != Nop) {
    gen(env, op, cell);
  }
  push(env, cns(env, 1));
}

void emitUnbox(HTS& env) {
  auto const exit = makeExit(env);
  auto const srcBox = popV(env);
  auto const unboxed = unbox(env, srcBox, exit);
  pushIncRef(env, unboxed);
  gen(env, DecRef, srcBox);
}

void emitThis(HTS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  checkThis(env, ctx);
  auto const this_ = gen(env, CastCtxThis, ctx);
  pushIncRef(env, this_);
}

void emitCheckThis(HTS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  checkThis(env, ctx);
}

void emitBareThis(HTS& env, BareThisOp subop) {
  if (!curClass(env)) {
    interpOne(env, Type::InitNull, 0); // will raise notice and push null
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

void emitClone(HTS& env) {
  if (!topC(env)->isA(Type::Obj)) PUNT(Clone-NonObj);
  auto const obj        = popC(env);
  push(env, gen(env, Clone, obj));
  gen(env, DecRef, obj);
}

void emitLateBoundCls(HTS& env) {
  auto const clss = curClass(env);
  if (!clss) {
    // no static context class, so this will raise an error
    interpOne(env, Type::Cls, 0);
    return;
  }
  auto const ctx = ldCtx(env);
  push(env, gen(env, LdClsCtx, ctx));
}

void emitSelf(HTS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr) {
    interpOne(env, Type::Cls, 0);
  } else {
    push(env, cns(env, clss));
  }
}

void emitParent(HTS& env) {
  auto const clss = curClass(env);
  if (clss == nullptr || clss->parent() == nullptr) {
    interpOne(env, Type::Cls, 0);
  } else {
    push(env, cns(env, clss->parent()));
  }
}

void emitNameA(HTS& env) {
  push(env, gen(env, LdClsName, popA(env)));
}

//////////////////////////////////////////////////////////////////////

void emitCastArray(HTS& env) {
  auto const src = popC(env);
  push(
    env,
    [&] {
      if (src->isA(Type::Arr))  return src;
      if (src->isA(Type::Null)) return cns(env, staticEmptyArray());
      if (src->isA(Type::Bool)) return gen(env, ConvBoolToArr, src);
      if (src->isA(Type::Dbl))  return gen(env, ConvDblToArr, src);
      if (src->isA(Type::Int))  return gen(env, ConvIntToArr, src);
      if (src->isA(Type::Str))  return gen(env, ConvStrToArr, src);
      if (src->isA(Type::Obj))  return gen(env, ConvObjToArr, src);
      return gen(env, ConvCellToArr, src);
    }()
  );
}

void emitCastBool(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToBool, src));
  gen(env, DecRef, src);
}

void emitCastDouble(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToDbl, src));
  gen(env, DecRef, src);
}

void emitCastInt(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToInt, src));
  gen(env, DecRef, src);
}

void emitCastObject(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToObj, src));
}

void emitCastString(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, ConvCellToStr, src));
  gen(env, DecRef, src);
}

void emitIncStat(HTS& env, int32_t counter, int32_t value) {
  if (!Stats::enabled()) return;
  gen(env, IncStat, cns(env, counter), cns(env, value), cns(env, false));
}

//////////////////////////////////////////////////////////////////////

void emitPopA(HTS& env) { popA(env); }
void emitPopC(HTS& env) { popDecRef(env, Type::Cell, DataTypeGeneric); }
void emitPopV(HTS& env) { popDecRef(env, Type::BoxedCell, DataTypeGeneric); }
void emitPopR(HTS& env) { popDecRef(env, Type::Gen, DataTypeGeneric); }

void emitDir(HTS& env)  { push(env, cns(env, curUnit(env)->dirpath())); }
void emitFile(HTS& env) { push(env, cns(env, curUnit(env)->filepath())); }

void emitDup(HTS& env) { pushIncRef(env, topC(env)); }

//////////////////////////////////////////////////////////////////////

void emitArray(HTS& env, const ArrayData* x)   { push(env, cns(env, x)); }
void emitString(HTS& env, const StringData* s) { push(env, cns(env, s)); }
void emitInt(HTS& env, int64_t val)            { push(env, cns(env, val)); }
void emitDouble(HTS& env, double val)          { push(env, cns(env, val)); }
void emitTrue(HTS& env)                        { push(env, cns(env, true)); }
void emitFalse(HTS& env)                       { push(env, cns(env, false)); }

void emitNull(HTS& env)       { push(env, cns(env, Type::InitNull)); }
void emitNullUninit(HTS& env) { push(env, cns(env, Type::Uninit)); }

//////////////////////////////////////////////////////////////////////

void emitNop(HTS&)                {}
void emitBoxRNop(HTS&)            {}
void emitUnboxRNop(HTS&)          {}
void emitRGetCNop(HTS&)           {}
void emitFPassC(HTS&, int32_t)    {}
void emitFPassVNop(HTS&, int32_t) {}
void emitDefClsNop(HTS&, Id)      {}
void emitBreakTraceHint(HTS&)     {}

//////////////////////////////////////////////////////////////////////

}}}
