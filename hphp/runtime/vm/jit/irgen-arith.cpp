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
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * TODO(#5710382): expand these macros.  They don't actually help.
 */

#define BINARY_ARITH          \
  AOP(Add, AddInt, AddDbl)    \
  AOP(Sub, SubInt, SubDbl)    \
  AOP(Mul, MulInt, MulDbl)    \
  AOP(AddO, AddIntO, AddDbl)  \
  AOP(SubO, SubIntO, SubDbl)  \
  AOP(MulO, MulIntO, MulDbl)  \

#define BINARY_BITOP  \
  BOP(BitAnd, AndInt) \
  BOP(BitOr,  OrInt)  \
  BOP(BitXor, XorInt) \

bool areBinaryArithTypesSupported(Op op, Type t1, Type t2) {
  auto checkArith = [](Type ty) {
    return ty.subtypeOfAny(Type::Int, Type::Bool, Type::Dbl);
  };
  auto checkBitOp = [](Type ty) {
    return ty.subtypeOfAny(Type::Int, Type::Bool);
  };

  switch (op) {
  #define AOP(OP, OPI, OPD) \
    case Op::OP: return checkArith(t1) && checkArith(t2);
  BINARY_ARITH
  #undef AOP
  #define BOP(OP, OPI) \
    case Op::OP: return checkBitOp(t1) && checkBitOp(t2);
  BINARY_BITOP
  #undef BOP
  default: not_reached();
  }
}

Opcode intArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPI;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

Opcode dblArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPD;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

Opcode bitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return OPI;
    BINARY_BITOP
    #undef BOP
    default: not_reached();
  }
}

bool isBitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return true;
    BINARY_BITOP
    #undef BOP
    default: return false;
  }
}

SSATmp* promoteBool(HTS& env, SSATmp* src) {
  // booleans in arithmetic and bitwise operations get cast to ints
  return src->type() <= Type::Bool ? gen(env, ConvBoolToInt, src) : src;
}

Opcode promoteBinaryDoubles(HTS& env, Op op, SSATmp*& src1, SSATmp*& src2) {
  auto const type1 = src1->type();
  auto const type2 = src2->type();
  auto opc = intArithOp(op);
  if (type1 <= Type::Dbl) {
    opc = dblArithOp(op);
    if (type2 <= Type::Int) {
      src2 = gen(env, ConvIntToDbl, src2);
    }
  } else if (type2 <= Type::Dbl) {
    opc = dblArithOp(op);
    src1 = gen(env, ConvIntToDbl, src1);
  }
  return opc;
}

void binaryBitOp(HTS& env, Op op) {
  auto const type2 = topC(env, 0)->type();
  auto const type1 = topC(env, 1)->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    PUNT(BunaryBitOp-Unsupported);
    return;
  }

  auto const src2 = promoteBool(env, popC(env));
  auto const src1 = promoteBool(env, popC(env));
  push(env, gen(env, bitOp(op), src1, src2));
}

void binaryArith(HTS& env, Op op) {
  auto const type2 = topC(env, 0)->type();
  auto const type1 = topC(env, 1)->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    // either an int or a dbl, but can't tell
    PUNT(BinaryArith-Unsupported);
    return;
  }

  auto spillValues = peekSpillValues(env);
  auto src2 = promoteBool(env, popC(env));
  auto src1 = promoteBool(env, popC(env));
  auto const opc = promoteBinaryDoubles(env, op, src1, src2);

  if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
    assert(src1->isA(Type::Int) && src2->isA(Type::Int));

    auto const exit = makeExitImpl(
      env,
      bcOff(env),
      ExitFlag::Interp,
      spillValues,
      CustomExit{}
    );

    push(env, gen(env, opc, exit, src1, src2));
  } else {
    push(env, gen(env, opc, src1, src2));
  }
}

// Implementation function that only handles integer or double inc/dec.
SSATmp* implIncDec(HTS& env, bool pre, bool inc, bool over, SSATmp* src) {
  assert(src->type() <= Type::Int || src->type() <= Type::Dbl);

  Opcode op;

  if (src->type() <= Type::Dbl) {
    op = inc ? AddDbl : SubDbl;
  } else if (!over) {
    op = inc ? AddInt : SubInt;
  } else {
    op = inc ? AddIntO : SubIntO;
  }

  auto const one = src->type() <= Type::Int ? cns(env, 1) : cns(env, 1.0);
  SSATmp* res = nullptr;

  if (op == AddIntO || op == SubIntO) {
    auto spillValues = peekSpillValues(env);
    auto const exit = makeExitImpl(
      env,
      bcOff(env),
      ExitFlag::Interp,
      spillValues,
      CustomExit{}
    );
    res = gen(env, op, exit, src, one);
  } else {
    res = gen(env, op, src, one);
  }

  // No incref necessary on push since result is an int.
  push(env, pre ? res : src);
  return res;
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
  assert(!t0.equals(Type::Gen) && !t1.equals(Type::Gen));
  auto const badObjConvs = Type::Int | Type::Dbl | Type::Str;
  return (t0.maybe(Type::Obj) && t1.maybe(badObjConvs)) ||
         (t0.maybe(badObjConvs) && t1.maybe(Type::Obj)) ||
         (t0.maybe(Type::Obj) && t1.maybe(Type::Obj)) ||
         (t0.maybe(Type::Arr) && t1.maybe(Type::Arr));
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

void implCmp(HTS& env, Opcode opc) {
  // The following if-block is historical behavior from ir-translator: this
  // should be re-evaluated.
  if (opc == Lt || opc == Lte || opc == Gt || opc == Gte) {
    auto leftType = topC(env, 1)->type();
    auto rightType = topC(env, 0)->type();
    if (!leftType.isKnownDataType() || !rightType.isKnownDataType()) {
      PUNT(LtGtOp-UnknownInput);
    }
    auto const ok =
      leftType.subtypeOfAny(Type::Null, Type::Bool, Type::Int, Type::Dbl) &&
      rightType.subtypeOfAny(Type::Null, Type::Bool, Type::Int, Type::Dbl);
    if (!ok) {
      PUNT(LtGtOp-NotOk);
    }
  }

  Block* catchBlock = nullptr;
  auto const opc2 = matchReentrantCmp(opc);
  // if the comparison operator could re-enter, convert it to the re-entrant
  // form and add the required catch block.
  // TODO #3446092 un-overload these opcodes.
  if (cmpOpTypesMayReenter(topC(env, 0)->type(),
                           topC(env, 1)->type()) &&
      opc2 != opc) {
    catchBlock = makeCatch(env);
    opc = opc2;
  }
  // src2 opc src1
  auto const src1 = popC(env);
  auto const src2 = popC(env);
  push(env, gen(env, opc, catchBlock, src2, src1));
  gen(env, DecRef, src2);
  gen(env, DecRef, src1);
}

void implAdd(HTS& env, Op op) {
  if (topC(env, 0)->type() <= Type::Arr && topC(env, 1)->type() <= Type::Arr) {
    auto const catchBlock = makeCatch(env);
    auto const tr = popC(env);
    auto const tl = popC(env);
    // The ArrayAdd helper decrefs its args, so don't decref pop'ed values.
    push(env, gen(env, ArrayAdd, catchBlock, tl, tr));
    return;
  }
  binaryArith(env, op);
}

//////////////////////////////////////////////////////////////////////

}

void emitConcat(HTS& env) {
  auto const catchBlock = makeCatch(env);
  auto const tr         = popC(env);
  auto const tl         = popC(env);
  // ConcatCellCell consumes only first ref, not second.
  push(env, gen(env, ConcatCellCell, catchBlock, tl, tr));
  // So we need to consume second ref ourselves.
  gen(env, DecRef, tr);
}

void emitConcatN(HTS& env, int32_t n) {
  if (n == 2) return emitConcat(env);

  auto const catchBlock = makeCatch(env);

  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const t3 = popC(env);

  if (!(t1->type() <= Type::Str) ||
      !(t2->type() <= Type::Str) ||
      !(t3->type() <= Type::Str)) {
    PUNT(ConcatN);
  }

  if (n == 3) {
    push(env, gen(env, ConcatStr3, catchBlock, t3, t2, t1));
    gen(env, DecRef, t2);
    gen(env, DecRef, t1);
    return;
  }

  always_assert(n == 4);
  auto const t4 = popC(env);
  if (!(t4->type() <= Type::Str)) PUNT(ConcatN);

  push(env, gen(env, ConcatStr4, catchBlock, t4, t3, t2, t1));
  gen(env, DecRef, t3);
  gen(env, DecRef, t2);
  gen(env, DecRef, t1);
}

void emitSetOpL(HTS& env, int32_t id, SetOpOp subop) {
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

  // Needs to modify locals after doing effectful operations like
  // ConcatCellCell, so we can't guard on their types.
  if (curFunc(env)->isPseudoMain()) PUNT(SetOpL-PseudoMain);

  // Null guard block for globals because we always punt on pseudomains
  auto const ldPMExit = nullptr;

  /*
   * Handle array addition first because we don't want to bother with
   * boxed locals.
   */
  bool const isAdd = (*subOpc == Op::Add || *subOpc == Op::AddO);
  if (isAdd && (env.irb->localType(id, DataTypeSpecific) <= Type::Arr) &&
      topC(env)->isA(Type::Arr)) {
    /*
     * ArrayAdd decrefs its sources and returns a new array with
     * refcount == 1. That covers the local, so incref once more for
     * the stack.
     */
    auto const catchBlock = makeCatch(env);
    auto const loc    = ldLoc(env, id, ldPMExit, DataTypeSpecific);
    auto const val    = popC(env);
    auto const result = gen(env, ArrayAdd, catchBlock, loc, val);
    stLocRaw(env, id, fp(env), result);
    pushIncRef(env, result);
    return;
  }

  auto const ldrefExit = makeExit(env);
  auto loc = ldLocInnerWarn(env, id, ldrefExit, ldPMExit, DataTypeGeneric);

  if (*subOpc == Op::Concat) {
    /*
     * The concat helpers incref their results, which will be consumed by
     * the stloc. We need an extra incref for the push onto the stack.
     */
    auto const catchBlock = makeCatch(env);
    auto const val    = popC(env);
    env.irb->constrainValue(loc, DataTypeSpecific);
    auto const result = gen(env, ConcatCellCell, catchBlock, loc, val);

    /*
     * Null exit block for 'ldrefExit' because we won't actually need to reload
     * the inner cell since we are doing a stLocNRC.  (Note that the inner cell
     * may have changed type if we re-entered during ConcatCellCell.)
     *
     * We can't put a non-null block here either, because it may need to
     * side-exit and we've already made observable progress executing this
     * instruction.  If we ever change ConcatCellCell not to decref its sources
     * we'll need to address this (or punt on a boxed source).
     */
    pushIncRef(env, stLocNRC(env, id, nullptr, ldPMExit, result));

    // ConcatCellCell does not DecRef its second argument,
    // so we need to do it here
    gen(env, DecRef, val);
    return;
  }

  if (areBinaryArithTypesSupported(*subOpc, loc->type(), topC(env)->type())) {
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

    SSATmp* result = nullptr;
    if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
      auto spillValues = peekSpillValues(env);
      spillValues.push_back(val);
      auto const exit = makeExitImpl(
        env,
        bcOff(env),
        ExitFlag::Interp,
        spillValues,
        CustomExit{}
      );
      result = gen(env, opc, exit, loc, val);
    } else {
      result = gen(env, opc, loc, val);
    }
    pushStLoc(env, id, ldrefExit, ldPMExit, result);
    return;
  }

  PUNT(SetOpL);
}

void emitIncDecL(HTS& env, int32_t id, IncDecOp subop) {
  auto const pre = isPre(subop);
  auto const inc = isInc(subop);
  auto const over = isIncDecO(subop);

  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInnerWarn(
    env,
    id,
    ldrefExit,
    ldPMExit,
    DataTypeSpecific
  );

  if (src->type() <= Type::Bool) {
    push(env, src);
    return;
  }

  if (src->type().subtypeOfAny(Type::Arr, Type::Obj)) {
    pushIncRef(env, src);
    return;
  }

  if (src->type() <= Type::Null) {
    push(env, inc && pre ? cns(env, 1) : src);
    if (inc) {
      stLoc(env, id, ldrefExit, ldPMExit, cns(env, 1));
    }
    return;
  }

  if (!src->type().subtypeOfAny(Type::Int, Type::Dbl)) {
    PUNT(IncDecL);
  }

  auto const res = implIncDec(env, pre, inc, over, src);
  stLoc(env, id, ldrefExit, ldPMExit, res);
}

void emitXor(HTS& env) {
  auto const btr = popC(env);
  auto const btl = popC(env);
  auto const tr = gen(env, ConvCellToBool, btr);
  auto const tl = gen(env, ConvCellToBool, btl);
  push(env, gen(env, XorBool, tl, tr));
  gen(env, DecRef, btl);
  gen(env, DecRef, btr);
}

void emitShl(HTS& env) {
  auto const catch1 = makeCatch(env);
  auto const catch2 = makeCatch(env);
  auto const shiftAmount = popC(env);
  auto const lhs         = popC(env);

  auto const lhsInt         = gen(env, ConvCellToInt, catch1, lhs);
  auto const shiftAmountInt = gen(env, ConvCellToInt, catch2, shiftAmount);

  push(env, gen(env, Shl, lhsInt, shiftAmountInt));
  gen(env, DecRef, lhs);
  gen(env, DecRef, shiftAmount);
}

void emitShr(HTS& env) {
  auto const catch1 = makeCatch(env);
  auto const catch2 = makeCatch(env);
  auto const shiftAmount = popC(env);
  auto const lhs         = popC(env);

  auto const lhsInt         = gen(env, ConvCellToInt, catch1, lhs);
  auto const shiftAmountInt = gen(env, ConvCellToInt, catch2, shiftAmount);

  push(env, gen(env, Shr, lhsInt, shiftAmountInt));
  gen(env, DecRef, lhs);
  gen(env, DecRef, shiftAmount);
}

void emitPow(HTS& env) {
  interpOne(env, Type::UncountedInit, 2);
}

void emitBitNot(HTS& env) {
  auto const srcType = topC(env)->type();
  if (srcType <= Type::Int) {
    auto const src = popC(env);
    push(env, gen(env, XorInt, src, cns(env, -1)));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = gen(env, ConvDblToInt, popC(env));
    push(env, gen(env, XorInt, src, cns(env, -1)));
    return;
  }

  auto const resultType = srcType <= Type::Str ? Type::Str
                        : srcType.needsReg() ? Type::Cell
                        : Type::Int;
  interpOne(env, resultType, 1);
}


void emitNot(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, XorBool, gen(env, ConvCellToBool, src), cns(env, true)));
  gen(env, DecRef, src);
}

void emitDiv(HTS& env) {
  auto const divisorType  = topC(env, 0)->type();
  auto const dividendType = topC(env, 1)->type();

  auto isNumeric = [&] (Type type) {
    return type.subtypeOfAny(Type::Int, Type::Dbl, Type::Bool);
  };

  // not going to bother with string division etc.
  if (!isNumeric(divisorType) || !isNumeric(dividendType)) {
    interpOne(env, Type::UncountedInit, 2);
    return;
  }

  auto divisor  = topC(env, 0);
  auto dividend = topC(env, 1);

  // we can't codegen this but we may be able to special case it away
  if (!divisor->isA(Type::Dbl) && !dividend->isA(Type::Dbl)) {
    // TODO(#2570625): support integer-integer division, move this to simlifier:
    if (divisor->isConst()) {
      int64_t divisorVal;
      if (divisor->isA(Type::Int)) {
        divisorVal = divisor->intVal();
      } else {
        assert(divisor->isA(Type::Bool));
        divisorVal = divisor->boolVal();
      }

      if (divisorVal == 0) {
        auto catchBlock = makeCatch(env);
        popC(env);
        popC(env);
        gen(env, RaiseWarning, catchBlock,
            cns(env, makeStaticString(Strings::DIVISION_BY_ZERO)));
        push(env, cns(env, false));
        return;
      }

      if (dividend->isConst()) {
        int64_t dividendVal;
        if (dividend->isA(Type::Int)) {
          dividendVal = dividend->intVal();
        } else {
          assert(dividend->isA(Type::Bool));
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
    interpOne(env, Type::UncountedInit, 2);
    return;
  }

  auto make_double = [&] (SSATmp* src) {
    if (src->isA(Type::Int)) {
      return gen(env, ConvIntToDbl, src);
    } else if (src->isA(Type::Bool)) {
      return gen(env, ConvBoolToDbl, src);
    }
    assert(src->isA(Type::Dbl));
    return src;
  };

  divisor  = make_double(popC(env));
  dividend = make_double(popC(env));

  // on division by zero we spill false and exit with a warning
  auto exitSpillValues = peekSpillValues(env);
  exitSpillValues.push_back(cns(env, false));

  auto const exit = makeExitWarn(
    env,
    nextBcOff(env),
    exitSpillValues,
    makeStaticString(Strings::DIVISION_BY_ZERO)
  );

  assert(divisor->isA(Type::Dbl) && dividend->isA(Type::Dbl));
  push(env, gen(env, DivDbl, exit, dividend, divisor));
}

void emitMod(HTS& env) {
  auto const catchBlock1 = makeCatch(env);
  auto const catchBlock2 = makeCatch(env);
  auto const btr = popC(env);
  auto const btl = popC(env);
  auto const tr = gen(env, ConvCellToInt, catchBlock1, btr);
  auto const tl = gen(env, ConvCellToInt, catchBlock2, btl);

  // We only want to decref btr and btl if the ConvCellToInt operation gave us
  // a new value back.
  if (tr != btr) gen(env, DecRef, btr);
  if (tl != btl) gen(env, DecRef, btl);
  // Exit path spills an additional false
  auto exitSpillValues = peekSpillValues(env);
  exitSpillValues.push_back(cns(env, false));

  // Generate an exit for the rare case that r is zero.  Interpreting
  // will raise a notice and produce the boolean false.  Punch out
  // here and resume after the Mod instruction; this should be rare.
  auto const exit = makeExitWarn(
    env,
    nextBcOff(env),
    exitSpillValues,
    makeStaticString(Strings::DIVISION_BY_ZERO)
  );
  gen(env, JmpZero, exit, tr);

  // We unfortunately need to special-case r = -1 here. In two's
  // complement, trying to divide INT_MIN by -1 will cause an integer
  // overflow.
  if (tr->isConst()) {
    // This whole block only exists so irb->cond doesn't get mad when one of
    // the branches gets optimized out due to constant folding.
    if (tr->intVal() == -1LL) {
      push(env, cns(env, 0));
    } else if (tr->intVal() == 0) {
      // mod by zero is undefined. don't emit opmod for it because
      // this could cause issues in simplifier/codegen
      // this should never get reached anyway, we just need to dump
      // something on the stack
      push(env, cns(env, false));
    } else {
      push(env, gen(env, Mod, tl, tr));
    }
    return;
  }

  // check for -1 (dynamic version)
  auto const res = env.irb->cond(
    0,
    [&] (Block* taken) {
      auto const negone = gen(env, Eq, tr, cns(env, -1));
      gen(env, JmpNZero, taken, negone);
    },
    [&] {
      return gen(env, Mod, tl, tr);
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      return cns(env, 0);
    }
  );
  push(env, res);
}

//////////////////////////////////////////////////////////////////////

void emitBitAnd(HTS& env) { binaryBitOp(env, Op::BitAnd); }
void emitBitOr(HTS& env)  { binaryBitOp(env, Op::BitOr); }
void emitBitXor(HTS& env) { binaryBitOp(env, Op::BitXor); }

void emitSub(HTS& env)    { binaryArith(env, Op::Sub); }
void emitMul(HTS& env)    { binaryArith(env, Op::Mul); }
void emitSubO(HTS& env)   { binaryArith(env, Op::SubO); }
void emitMulO(HTS& env)   { binaryArith(env, Op::MulO); }

void emitGt(HTS& env)     { implCmp(env, Gt);    }
void emitGte(HTS& env)    { implCmp(env, Gte);   }
void emitLt(HTS& env)     { implCmp(env, Lt);    }
void emitLte(HTS& env)    { implCmp(env, Lte);   }
void emitEq(HTS& env)     { implCmp(env, Eq);    }
void emitNeq(HTS& env)    { implCmp(env, Neq);   }
void emitSame(HTS& env)   { implCmp(env, Same);  }
void emitNSame(HTS& env)  { implCmp(env, NSame); }

void emitAdd(HTS& env)    { implAdd(env, Op::Add); }
void emitAddO(HTS& env)   { implAdd(env, Op::AddO); }

//////////////////////////////////////////////////////////////////////

}}}
