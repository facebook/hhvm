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

#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

bool areBinaryArithTypesSupported(Op op, Type t1, Type t2) {
  auto checkArith = [](Type ty) {
    return ty.subtypeOfAny(TInt, TBool, TDbl);
  };
  auto checkBitOp = [](Type ty) {
    return ty.subtypeOfAny(TInt, TBool);
  };

  switch (op) {
  case Op::Add:
  case Op::Sub:
  case Op::Mul:
  case Op::AddO:
  case Op::SubO:
  case Op::MulO:
    return checkArith(t1) && checkArith(t2);
  case Op::BitAnd:
  case Op::BitOr:
  case Op::BitXor:
    return checkBitOp(t1) && checkBitOp(t2);
  default:
    break;
  }
  always_assert(0);
}

Opcode intArithOp(Op op) {
  switch (op) {
  case Op::Add:  return AddInt;
  case Op::Sub:  return SubInt;
  case Op::Mul:  return MulInt;
  case Op::AddO: return AddIntO;
  case Op::SubO: return SubIntO;
  case Op::MulO: return MulIntO;
  default:
    break;
  }
  always_assert(0);
}

Opcode dblArithOp(Op op) {
  switch (op) {
  case Op::Add:  return AddDbl;
  case Op::Sub:  return SubDbl;
  case Op::Mul:  return MulDbl;
  case Op::AddO: return AddDbl;
  case Op::SubO: return SubDbl;
  case Op::MulO: return MulDbl;
  default:
    break;
  }
  always_assert(0);
}

Opcode bitOp(Op op) {
  switch (op) {
  case Op::BitAnd: return AndInt;
  case Op::BitOr:  return OrInt;
  case Op::BitXor: return XorInt;
  default:
    break;
  }
  always_assert(0);
}

bool isBitOp(Op op) {
  switch (op) {
  case Op::BitAnd:
  case Op::BitOr:
  case Op::BitXor:
    return true;
  default:
    return false;
  }
}

SSATmp* promoteBool(IRGS& env, SSATmp* src) {
  // booleans in arithmetic and bitwise operations get cast to ints
  return src->type() <= TBool ? gen(env, ConvBoolToInt, src) : src;
}

Opcode promoteBinaryDoubles(IRGS& env, Op op, SSATmp*& src1, SSATmp*& src2) {
  auto const type1 = src1->type();
  auto const type2 = src2->type();
  auto opc = intArithOp(op);
  if (type1 <= TDbl) {
    opc = dblArithOp(op);
    if (type2 <= TInt) {
      src2 = gen(env, ConvIntToDbl, src2);
    }
  } else if (type2 <= TDbl) {
    opc = dblArithOp(op);
    src1 = gen(env, ConvIntToDbl, src1);
  }
  return opc;
}

void binaryBitOp(IRGS& env, Op op) {
  auto const type2 = topC(env, BCSPOffset{0})->type();
  auto const type1 = topC(env, BCSPOffset{1})->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    PUNT(BunaryBitOp-Unsupported);
    return;
  }

  auto const src2 = promoteBool(env, popC(env));
  auto const src1 = promoteBool(env, popC(env));
  push(env, gen(env, bitOp(op), src1, src2));
}

void binaryArith(IRGS& env, Op op) {
  auto const type2 = topC(env, BCSPOffset{0})->type();
  auto const type1 = topC(env, BCSPOffset{1})->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    // either an int or a dbl, but can't tell
    PUNT(BinaryArith-Unsupported);
    return;
  }

  auto const exitSlow = makeExitSlow(env);
  auto src2 = promoteBool(env, popC(env));
  auto src1 = promoteBool(env, popC(env));
  auto const opc = promoteBinaryDoubles(env, op, src1, src2);

  if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
    assertx(src1->isA(TInt) && src2->isA(TInt));
    push(env, gen(env, opc, exitSlow, src1, src2));
  } else {
    push(env, gen(env, opc, src1, src2));
  }
}

/*
 * True if comparison may throw or reenter.
 *
 * 1. Objects compared with strings may involve calling a user-defined
 * __toString function.
 *
 * 2. Objects compared with ints or doubles raises a notice when the object is
 * converted to a number.
 *
 * 3. Array comparisons can throw if recursion is detected.
 */
bool cmpOpTypesMayReenter(Type t0, Type t1) {
  assertx(t0 != TGen && t1 != TGen);
  auto const badObjConvs = TInt | TDbl | TStr;
  return (t0.maybe(TObj) && t1.maybe(badObjConvs)) ||
         (t0.maybe(badObjConvs) && t1.maybe(TObj)) ||
         (t0.maybe(TObj) && t1.maybe(TObj)) ||
         (t0.maybe(TArr) && t1.maybe(TArr));
}

Opcode matchReentrantCmp(Opcode opc) {
  switch (opc) {
  case Gt:  return GtX;
  case Gte: return GteX;
  case Lt:  return LtX;
  case Lte: return LteX;
  case Eq:  return EqX;
  case Neq: return NeqX;
  default:  return opc;
  }
}

void implCmp(IRGS& env, Opcode opc) {
  // The following if-block is historical behavior from ir-translator: this
  // should be re-evaluated.
  if (opc == Lt || opc == Lte || opc == Gt || opc == Gte) {
    auto leftType = topC(env, BCSPOffset{1})->type();
    auto rightType = topC(env, BCSPOffset{0})->type();
    if (!leftType.isKnownDataType() || !rightType.isKnownDataType()) {
      PUNT(LtGtOp-UnknownInput);
    }
    auto const ok =
      leftType.subtypeOfAny(TNull, TBool, TInt, TDbl, TStr) &&
      rightType.subtypeOfAny(TNull, TBool, TInt, TDbl, TStr);
    if (!ok) {
      PUNT(LtGtOp-NotOk);
    }
  }

  auto const opc2 = matchReentrantCmp(opc);
  // if the comparison operator could re-enter, convert it to the re-entrant
  // form and add the required catch block.
  // TODO #3446092 un-overload these opcodes.
  if (cmpOpTypesMayReenter(topC(env, BCSPOffset{0})->type(),
                           topC(env, BCSPOffset{1})->type()) &&
      opc2 != opc) {
    opc = opc2;
  }
  // src2 opc src1
  auto const src1 = popC(env);
  auto const src2 = popC(env);
  push(env, gen(env, opc, src2, src1));
  gen(env, DecRef, src2);
  gen(env, DecRef, src1);
}

void implAdd(IRGS& env, Op op) {
  if (topC(env, BCSPOffset{0})->type() <= TArr &&
      topC(env, BCSPOffset{1})->type() <= TArr) {
    auto const tr = popC(env);
    auto const tl = popC(env);
    // The ArrayAdd helper decrefs its args, so don't decref pop'ed values.
    push(env, gen(env, ArrayAdd, tl, tr));
    return;
  }
  binaryArith(env, op);
}

template<class PreDecRef>
void implConcat(IRGS& env, SSATmp* c1, SSATmp* c2, PreDecRef preDecRef) {
  auto const t1 = c1->type();
  auto const t2 = c2->type();

  /*
   * We have some special translations for common combinations that avoid extra
   * conversion calls.
   */
  auto const str = [&] () -> SSATmp* {
    if (t2 <= TInt && t1 <= TStr) return gen(env, ConcatIntStr, c2, c1);
    if (t2 <= TStr && t1 <= TInt) return gen(env, ConcatStrInt, c2, c1);
    return nullptr;
  }();

  if (str) {
    preDecRef(str);
    // Note that the ConcatFoo opcode consumed the reference on its first
    // argument, so we only need to decref the second one.
    gen(env, DecRef, c1);
    return;
  }

  /*
   * Generic translation: convert both to strings, and then concatenate them.
   *
   * NB: the order we convert to strings is observable (because of __toString
   * methods), and the order we run DecRefs of the input cells is also
   * observable.
   *
   * We don't want to convert to strings if either was already a string.  Note
   * that for the c2 string, failing to do this could change big-O program
   * behavior if refcount opts were off, since we'd COW strings that we
   * shouldn't (a ConvCellToStr of a Str will simplify into an IncRef).
   */
  auto const s2 = t2 <= TStr ? c2 : gen(env, ConvCellToStr, c2);
  auto const s1 = t1 <= TStr ? c1 : gen(env, ConvCellToStr, c1);
  auto const r  = gen(env, ConcatStrStr, s2, s1);  // consumes s2 reference
  preDecRef(r);
  gen(env, DecRef, s1);
  if (s2 != c2) gen(env, DecRef, c2);
  if (s1 != c1) gen(env, DecRef, c1);
}

//////////////////////////////////////////////////////////////////////

}

void emitConcat(IRGS& env) {
  auto const c1 = popC(env);
  auto const c2 = popC(env);
  implConcat(env, c1, c2, [&] (SSATmp* r) { push(env, r); });
}

void emitConcatN(IRGS& env, int32_t n) {
  if (n == 2) return emitConcat(env);

  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const t3 = popC(env);

  if (!(t1->type() <= TStr) ||
      !(t2->type() <= TStr) ||
      !(t3->type() <= TStr)) {
    PUNT(ConcatN);
  }

  if (n == 3) {
    push(env, gen(env, ConcatStr3, t3, t2, t1));
    gen(env, DecRef, t2);
    gen(env, DecRef, t1);
    return;
  }

  always_assert(n == 4);
  auto const t4 = popC(env);
  if (!(t4->type() <= TStr)) PUNT(ConcatN);

  push(env, gen(env, ConcatStr4, t4, t3, t2, t1));
  gen(env, DecRef, t3);
  gen(env, DecRef, t2);
  gen(env, DecRef, t1);
}

void emitSetOpL(IRGS& env, int32_t id, SetOpOp subop) {
  auto const subOpc = [&]() -> folly::Optional<Op> {
    switch (subop) {
    case SetOpOp::PlusEqual:   return Op::Add;
    case SetOpOp::MinusEqual:  return Op::Sub;
    case SetOpOp::MulEqual:    return Op::Mul;
    case SetOpOp::PlusEqualO:  return Op::AddO;
    case SetOpOp::MinusEqualO: return Op::SubO;
    case SetOpOp::MulEqualO:   return Op::MulO;
    case SetOpOp::DivEqual:    return folly::none;
    case SetOpOp::ConcatEqual: return Op::Concat;
    case SetOpOp::ModEqual:    return folly::none;
    case SetOpOp::PowEqual:    return folly::none;
    case SetOpOp::AndEqual:    return Op::BitAnd;
    case SetOpOp::OrEqual:     return Op::BitOr;
    case SetOpOp::XorEqual:    return Op::BitXor;
    case SetOpOp::SlEqual:     return folly::none;
    case SetOpOp::SrEqual:     return folly::none;
    }
    not_reached();
  }();
  if (!subOpc) PUNT(SetOpL-Unsupported);

  // Needs to modify locals after doing effectful operations like converting
  // things to strings, so we can't guard on their types.
  if (curFunc(env)->isPseudoMain()) PUNT(SetOpL-PseudoMain);

  // Null guard block for globals because we always punt on pseudomains
  auto const ldPMExit = nullptr;

  /*
   * Handle array addition first because we don't want to bother with
   * boxed locals.
   */
  bool const isAdd = (*subOpc == Op::Add || *subOpc == Op::AddO);
  if (isAdd && (env.irb->localType(id, DataTypeSpecific) <= TArr) &&
      topC(env)->isA(TArr)) {
    /*
     * ArrayAdd decrefs its sources and returns a new array with
     * refcount == 1. That covers the local, so incref once more for
     * the stack.
     */
    auto const loc    = ldLoc(env, id, ldPMExit, DataTypeSpecific);
    auto const val    = popC(env);
    auto const result = gen(env, ArrayAdd, loc, val);
    stLocRaw(env, id, fp(env), result);
    pushIncRef(env, result);
    return;
  }

  auto const ldrefExit = makeExit(env);
  auto loc = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeGeneric);

  if (*subOpc == Op::Concat) {
    /*
     * The concat helpers incref their results, which will be consumed by
     * the stloc. We need an extra incref for the push onto the stack.
     */
    auto const val    = popC(env);
    env.irb->constrainValue(loc, DataTypeSpecific);
    implConcat(env, val, loc, [&] (SSATmp* result) {
      /*
       * Null exit block for 'ldrefExit' because we won't actually need to
       * reload the inner cell since we are doing a stLocNRC.  (Note that the
       * inner cell may have changed type if we re-entered during Concat.)
       *
       * We can't put a non-null block here either, because it may need to
       * side-exit and we've already made observable progress executing this
       * instruction.  If we ever change ConcatStrFoo not to decref its sources
       * we'll need to address this (or punt on a boxed source).
       */
      pushIncRef(env, stLocNRC(env, id, nullptr, ldPMExit, result));
    });
    return;
  }

  if (!areBinaryArithTypesSupported(*subOpc, loc->type(), topC(env)->type())) {
    PUNT(SetOpL);
  }

  auto const exitSlow = makeExitSlow(env);
  auto val = popC(env);
  env.irb->constrainValue(loc, DataTypeSpecific);
  loc = promoteBool(env, loc);
  val = promoteBool(env, val);
  Opcode opc;
  if (isBitOp(*subOpc)) {
    opc = bitOp(*subOpc);
  } else {
    opc = promoteBinaryDoubles(env, *subOpc, loc, val);
  }

  auto const result = opc == AddIntO || opc == SubIntO || opc == MulIntO
    ? gen(env, opc, exitSlow, loc, val)
    : gen(env, opc, loc, val);
  pushStLoc(env, id, ldrefExit, ldPMExit, result);
}

void emitIncDecL(IRGS& env, int32_t id, IncDecOp subop) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeSpecific
  );

  if (auto const result = incDec(env, subop, src)) {
    pushIncRef(env, isPre(subop) ? result : src);
    stLoc(env, id, ldrefExit, ldPMExit, result);
    return;
  }

  PUNT(IncDecL);
}

void emitXor(IRGS& env) {
  auto const btr = popC(env);
  auto const btl = popC(env);
  auto const tr = gen(env, ConvCellToBool, btr);
  auto const tl = gen(env, ConvCellToBool, btl);
  push(env, gen(env, XorBool, tl, tr));
  gen(env, DecRef, btl);
  gen(env, DecRef, btr);
}

void emitShl(IRGS& env) {
  auto const shiftAmount    = popC(env);
  auto const lhs            = popC(env);
  auto const lhsInt         = gen(env, ConvCellToInt, lhs);
  auto const shiftAmountInt = gen(env, ConvCellToInt, shiftAmount);

  push(env, gen(env, Shl, lhsInt, shiftAmountInt));
  gen(env, DecRef, lhs);
  gen(env, DecRef, shiftAmount);
}

void emitShr(IRGS& env) {
  auto const shiftAmount    = popC(env);
  auto const lhs            = popC(env);
  auto const lhsInt         = gen(env, ConvCellToInt, lhs);
  auto const shiftAmountInt = gen(env, ConvCellToInt, shiftAmount);

  push(env, gen(env, Shr, lhsInt, shiftAmountInt));
  gen(env, DecRef, lhs);
  gen(env, DecRef, shiftAmount);
}

void emitPow(IRGS& env) {
  interpOne(env, TUncountedInit, 2);
}

void emitBitNot(IRGS& env) {
  auto const srcType = topC(env)->type();
  if (srcType <= TInt) {
    auto const src = popC(env);
    push(env, gen(env, XorInt, src, cns(env, -1)));
    return;
  }

  if (srcType <= TDbl) {
    auto const src = gen(env, ConvDblToInt, popC(env));
    push(env, gen(env, XorInt, src, cns(env, -1)));
    return;
  }

  auto const resultType = srcType <= TStr ? TStr
                        : srcType.needsReg() ? TCell
                        : TInt;
  interpOne(env, resultType, 1);
}


void emitNot(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, XorBool, gen(env, ConvCellToBool, src), cns(env, true)));
  gen(env, DecRef, src);
}

void emitDiv(IRGS& env) {
  auto const divisorType  = topC(env, BCSPOffset{0})->type();
  auto const dividendType = topC(env, BCSPOffset{1})->type();

  auto isNumeric = [&] (Type type) {
    return type.subtypeOfAny(TInt, TDbl, TBool);
  };

  // not going to bother with string division etc.
  if (!isNumeric(divisorType) || !isNumeric(dividendType)) {
    interpOne(env, TUncountedInit, 2);
    return;
  }

  auto divisor  = topC(env, BCSPOffset{0});
  auto dividend = topC(env, BCSPOffset{1});

  // we can't codegen this but we may be able to special case it away
  if (!divisor->isA(TDbl) && !dividend->isA(TDbl)) {
    // TODO(#2570625): support integer-integer division, move this to
    // simplifier:
    if (divisor->hasConstVal()) {
      int64_t divisorVal;
      if (divisor->isA(TInt)) {
        divisorVal = divisor->intVal();
      } else {
        assertx(divisor->isA(TBool));
        divisorVal = divisor->boolVal();
      }

      if (divisorVal == 0) {
        popC(env);
        popC(env);
        gen(env, RaiseWarning,
            cns(env, makeStaticString(Strings::DIVISION_BY_ZERO)));
        push(env, cns(env, false));
        return;
      }

      if (dividend->hasConstVal()) {
        int64_t dividendVal;
        if (dividend->isA(TInt)) {
          dividendVal = dividend->intVal();
        } else {
          assertx(dividend->isA(TBool));
          dividendVal = dividend->boolVal();
        }
        popC(env);
        popC(env);
        if (dividendVal == LLONG_MIN || dividendVal % divisorVal) {
          push(env, cns(env, (double)dividendVal / divisorVal));
        } else {
          push(env, cns(env, dividendVal / divisorVal));
        }
        return;
      }
      /* fall through */
    }
    interpOne(env, TUncountedInit, 2);
    return;
  }

  auto make_double = [&] (SSATmp* src) {
    if (src->isA(TInt)) {
      return gen(env, ConvIntToDbl, src);
    } else if (src->isA(TBool)) {
      return gen(env, ConvBoolToDbl, src);
    }
    assertx(src->isA(TDbl));
    return src;
  };

  divisor  = make_double(popC(env));
  dividend = make_double(popC(env));

  SSATmp* divVal = nullptr;  // edge-defined value
  ifThen(
    env,
    [&] (Block* taken) {
      divVal = gen(env, DivDbl, taken, dividend, divisor);
    },
    [&] {
      // Make progress by raising a warning and pushing false before
      // side-exiting to the next instruction.
      hint(env, Block::Hint::Unlikely);
      auto const msg = cns(env, makeStaticString(Strings::DIVISION_BY_ZERO));
      gen(env, RaiseWarning, msg);
      push(env, cns(env, false));
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );

  push(env, divVal);
}

void emitMod(IRGS& env) {
  auto const btr = popC(env);
  auto const btl = popC(env);
  auto const tr = gen(env, ConvCellToInt, btr);
  auto const tl = gen(env, ConvCellToInt, btl);

  // Generate an exit for the rare case that r is zero.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, JmpZero, taken, tr);
    },
    [&] {
      // Make progress before side-exiting to the next instruction: raise a
      // warning and push false.
      hint(env, Block::Hint::Unlikely);
      auto const msg = cns(env, makeStaticString(Strings::DIVISION_BY_ZERO));
      gen(env, RaiseWarning, msg);
      gen(env, DecRef, btr);
      gen(env, DecRef, btl);
      push(env, cns(env, false));
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );

  // DecRefs on the main line must happen after the potentially-throwing exit
  // above: if we throw during the RaiseWarning, those values must still be on
  // the stack.
  gen(env, DecRef, btr);
  gen(env, DecRef, btl);

  // Check for -1.  The Mod IR instruction has undefined behavior for -1, but
  // php semantics are to return zero.
  auto const res = cond(
    env,
    [&] (Block* taken) {
      auto const negone = gen(env, Eq, tr, cns(env, -1));
      gen(env, JmpNZero, taken, negone);
    },
    [&] {
      return gen(env, Mod, tl, tr);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return cns(env, 0);
    }
  );
  push(env, res);
}

//////////////////////////////////////////////////////////////////////

void emitBitAnd(IRGS& env) { binaryBitOp(env, Op::BitAnd); }
void emitBitOr(IRGS& env)  { binaryBitOp(env, Op::BitOr); }
void emitBitXor(IRGS& env) { binaryBitOp(env, Op::BitXor); }

void emitSub(IRGS& env)    { binaryArith(env, Op::Sub); }
void emitMul(IRGS& env)    { binaryArith(env, Op::Mul); }
void emitSubO(IRGS& env)   { binaryArith(env, Op::SubO); }
void emitMulO(IRGS& env)   { binaryArith(env, Op::MulO); }

void emitGt(IRGS& env)     { implCmp(env, Gt);    }
void emitGte(IRGS& env)    { implCmp(env, Gte);   }
void emitLt(IRGS& env)     { implCmp(env, Lt);    }
void emitLte(IRGS& env)    { implCmp(env, Lte);   }
void emitEq(IRGS& env)     { implCmp(env, Eq);    }
void emitNeq(IRGS& env)    { implCmp(env, Neq);   }
void emitSame(IRGS& env)   { implCmp(env, Same);  }
void emitNSame(IRGS& env)  { implCmp(env, NSame); }

void emitAdd(IRGS& env)    { implAdd(env, Op::Add); }
void emitAddO(IRGS& env)   { implAdd(env, Op::AddO); }

//////////////////////////////////////////////////////////////////////

}}}
