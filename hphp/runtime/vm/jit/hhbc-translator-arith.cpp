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
#include "hphp/runtime/vm/jit/hhbc-translator.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/base/strings.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitConcat() {
  auto const catchBlock = makeCatch();
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // Concat consumes only first ref, never second
  push(gen(ConcatCellCell, catchBlock, tl, tr));
  // so we need to consume second ref ourselves
  gen(DecRef, tr);
}

void HhbcTranslator::emitConcatN(int32_t n) {
  if (n == 2) return emitConcat();

  auto const catchBlock = makeCatch();

  SSATmp* t1 = popC();
  SSATmp* t2 = popC();
  SSATmp* t3 = popC();

  if (!t1->isA(Type::Str) ||
      !t2->isA(Type::Str) ||
      !t3->isA(Type::Str)) {
    PUNT(ConcatN);
  }

  if (n == 3) {
    push(gen(ConcatStr3, catchBlock, t3, t2, t1));
    gen(DecRef, t2);
    gen(DecRef, t1);

  } else if (n == 4) {
    SSATmp* t4 = popC();
    if (!t4->isA(Type::Str)) PUNT(ConcatN);

    push(gen(ConcatStr4, catchBlock, t4, t3, t2, t1));
    gen(DecRef, t3);
    gen(DecRef, t2);
    gen(DecRef, t1);

  } else {
    not_reached();
  }
}

// only handles integer or double inc/dec
SSATmp* HhbcTranslator::implIncDec(bool pre,
                                   bool inc,
                                   bool over,
                                   SSATmp* src) {
  assert(src->isA(Type::Int) || src->isA(Type::Dbl));

  Opcode op;

  if (src->isA(Type::Dbl)) {
    op = inc ? AddDbl : SubDbl;
  } else if (!over) {
    op = inc ? AddInt : SubInt;
  } else {
    op = inc ? AddIntO : SubIntO;
  }

  SSATmp* one = src->isA(Type::Int) ? cns(1) : cns(1.0);
  SSATmp* res = nullptr;

  if (op == AddIntO || op == SubIntO) {
    auto spills = peekSpillValues();
    auto const exit = makeExitImpl(
      bcOff(),
      ExitFlag::Interp,
      spills,
      CustomExit{}
    );
    res = gen(op, exit, src, one);
  } else {
    res = gen(op, src, one);
  }

  // no incref necessary on push since result is an int
  push(pre ? res : src);
  return res;
}

void HhbcTranslator::emitIncDecL(int32_t id, IncDecOp subop) {
  auto const pre = isPre(subop);
  auto const inc = isInc(subop);
  auto const over = isIncDecO(subop);

  auto const ldrefExit = makeExit();
  auto const ldPMExit = makePseudoMainExit();
  auto const src = ldLocInnerWarn(
    id,
    ldrefExit,
    ldPMExit,
    DataTypeSpecific
  );

  if (src->isA(Type::Bool)) {
    push(src);
    return;
  }

  if (src->type().subtypeOfAny(Type::Arr, Type::Obj)) {
    pushIncRef(src);
    return;
  }

  if (src->isA(Type::Null)) {
    push(inc && pre ? cns(1) : src);
    if (inc) {
      stLoc(id, ldrefExit, ldPMExit, cns(1));
    }
    return;
  }

  if (!src->type().subtypeOfAny(Type::Int, Type::Dbl)) {
    PUNT(IncDecL);
  }

  auto const res = implIncDec(pre, inc, over, src);
  stLoc(id, ldrefExit, ldPMExit, res);
}

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

static bool areBinaryArithTypesSupported(Op op, Type t1, Type t2) {
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

static Opcode intArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPI;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

static Opcode dblArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPD;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

static Opcode bitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return OPI;
    BINARY_BITOP
    #undef BOP
    default: not_reached();
  }
}

static bool isBitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return true;
    BINARY_BITOP
    #undef BOP
    default: return false;
  }
}

SSATmp* HhbcTranslator::promoteBool(SSATmp* src) {
  // booleans in arithmetic and bitwise operations get cast to ints
  return src->isA(Type::Bool) ? gen(ConvBoolToInt, src) : src;
}

Opcode HhbcTranslator::promoteBinaryDoubles(Op op,
                                            SSATmp*& src1,
                                            SSATmp*& src2) {
  auto type1 = src1->type();
  auto type2 = src2->type();

  Opcode opc = intArithOp(op);
  if (type1 <= Type::Dbl) {
    opc = dblArithOp(op);
    if (type2 <= Type::Int) {
      src2 = gen(ConvIntToDbl, src2);
    }
  } else if (type2 <= Type::Dbl) {
    opc = dblArithOp(op);
    src1 = gen(ConvIntToDbl, src1);
  }
  return opc;
}

void HhbcTranslator::emitSetOpL(uint32_t id, SetOpOp subop) {
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
  if (inPseudoMain()) PUNT(SetOpL-PseudoMain);

  // Null guard block for globals because we always punt on pseudomains
  auto const ldPMExit = nullptr;

  /*
   * Handle array addition first because we don't want to bother with
   * boxed locals.
   */
  bool isAdd = (*subOpc == Op::Add || *subOpc == Op::AddO);
  if (isAdd && (m_irb->localType(id, DataTypeSpecific) <= Type::Arr) &&
      topC()->isA(Type::Arr)) {
    /*
     * ArrayAdd decrefs its sources and returns a new array with
     * refcount == 1. That covers the local, so incref once more for
     * the stack.
     */
    auto const catchBlock = makeCatch();
    auto const loc    = ldLoc(id, ldPMExit, DataTypeSpecific);
    auto const val    = popC();
    auto const result = gen(ArrayAdd, catchBlock, loc, val);
    genStLocal(id, m_irb->fp(), result);
    pushIncRef(result);
    return;
  }

  auto const ldrefExit = makeExit();
  auto loc = ldLocInnerWarn(id, ldrefExit, ldPMExit, DataTypeGeneric);

  if (*subOpc == Op::Concat) {
    /*
     * The concat helpers incref their results, which will be consumed by
     * the stloc. We need an extra incref for the push onto the stack.
     */
    auto const catchBlock = makeCatch();
    auto const val    = popC();
    m_irb->constrainValue(loc, DataTypeSpecific);
    auto const result = gen(ConcatCellCell, catchBlock, loc, val);

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
    pushIncRef(stLocNRC(id, nullptr, ldPMExit, result));

    // ConcatCellCell does not DecRef its second argument,
    // so we need to do it here
    gen(DecRef, val);
    return;
  }

  if (areBinaryArithTypesSupported(*subOpc, loc->type(), topC()->type())) {
    auto val = popC();
    m_irb->constrainValue(loc, DataTypeSpecific);
    loc = promoteBool(loc);
    val = promoteBool(val);
    Opcode opc;
    if (isBitOp(*subOpc)) {
      opc = bitOp(*subOpc);
    } else {
      opc = promoteBinaryDoubles(*subOpc, loc, val);
    }

    SSATmp* result = nullptr;
    if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
      auto spillValues = peekSpillValues();
      spillValues.push_back(val);
      auto const exit = makeExitImpl(
        bcOff(),
        ExitFlag::Interp,
        spillValues,
        CustomExit{}
      );
      result = gen(opc, exit, loc, val);
    } else {
      result = gen(opc, loc, val);
    }
    pushStLoc(id, ldrefExit, ldPMExit, result);
    return;
  }

  PUNT(SetOpL);
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
static bool cmpOpTypesMayReenter(Type t0, Type t1) {
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

void HhbcTranslator::implCmp(Opcode opc) {
  // The following if-block is historical behavior from ir-translator: this
  // should be re-evaluated.
  if (opc == Lt || opc == Lte || opc == Gt || opc == Gte) {
    auto leftType = topC(1)->type();
    auto rightType = topC(0)->type();
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
  if (cmpOpTypesMayReenter(topC(0)->type(), topC(1)->type()) && opc2 != opc) {
    catchBlock = makeCatch();
    opc = opc2;
  }
  // src2 opc src1
  auto const src1 = popC();
  auto const src2 = popC();
  push(gen(opc, catchBlock, src2, src1));
  gen(DecRef, src2);
  gen(DecRef, src1);
}


void HhbcTranslator::emitBinaryBitOp(Op op) {
  Type type2 = topC(0)->type();
  Type type1 = topC(1)->type();

  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    PUNT(BunaryBitOp-Unsupported);
    return;
  }

  SSATmp* src2 = promoteBool(popC());
  SSATmp* src1 = promoteBool(popC());
  push(gen(bitOp(op), src1, src2));
}

void HhbcTranslator::emitBinaryArith(Op op) {
  Type type2 = topC(0)->type();
  Type type1 = topC(1)->type();

  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    // either an int or a dbl, but can't tell
    PUNT(BinaryArith-Unsupported);
    return;
  }

  auto spillValues = peekSpillValues();
  SSATmp* src2 = promoteBool(popC());
  SSATmp* src1 = promoteBool(popC());
  Opcode opc = promoteBinaryDoubles(op, src1, src2);

  if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
    assert(src1->isA(Type::Int) && src2->isA(Type::Int));

    auto const exit = makeExitImpl(
      bcOff(),
      ExitFlag::Interp,
      spillValues,
      CustomExit{}
    );

    push(gen(opc, exit, src1, src2));
  } else {
    push(gen(opc, src1, src2));
  }
}

void HhbcTranslator::emitNot() {
  SSATmp* src = popC();
  push(gen(XorBool, gen(ConvCellToBool, src), cns(true)));
  gen(DecRef, src);
}


void HhbcTranslator::emitSub()  { emitBinaryArith(Op::Sub); }
void HhbcTranslator::emitMul()  { emitBinaryArith(Op::Mul); }
void HhbcTranslator::emitSubO() { emitBinaryArith(Op::SubO); }
void HhbcTranslator::emitMulO() { emitBinaryArith(Op::MulO); }

void HhbcTranslator::addImpl(Op op) {
  if (topC(0)->type() <= Type::Arr && topC(1)->type() <= Type::Arr) {
    auto const catchBlock = makeCatch();
    auto const tr = popC();
    auto const tl = popC();
    // The ArrayAdd helper decrefs its args, so don't decref pop'ed values.
    push(gen(ArrayAdd, catchBlock, tl, tr));
    return;
  }
  emitBinaryArith(op);
}

void HhbcTranslator::emitAdd()  { addImpl(Op::Add); }
void HhbcTranslator::emitAddO() { addImpl(Op::AddO); }

#define BOP(OP, OPI) \
  void HhbcTranslator::emit ## OP() { emitBinaryBitOp(Op::OP); }
BINARY_BITOP
#undef BOP

void HhbcTranslator::emitDiv() {
  auto divisorType  = topC(0)->type();
  auto dividendType = topC(1)->type();

  auto isNumeric = [&] (Type type) {
    return type.subtypeOfAny(Type::Int, Type::Dbl, Type::Bool);
  };

  // not going to bother with string division etc.
  if (!isNumeric(divisorType) || !isNumeric(dividendType)) {
    emitInterpOne(Type::UncountedInit, 2);
    return;
  }

  auto divisor  = topC(0);
  auto dividend = topC(1);

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
        auto catchBlock = makeCatch();
        popC();
        popC();
        gen(RaiseWarning, catchBlock,
            cns(makeStaticString(Strings::DIVISION_BY_ZERO)));
        push(cns(false));
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
        popC();
        popC();
        if (dividendVal == LLONG_MIN || dividendVal % divisorVal) {
          push(cns((double)dividendVal / divisorVal));
        } else {
          push(cns(dividendVal / divisorVal));
        }
        return;
      }
      /* fall through */
    }
    emitInterpOne(Type::UncountedInit, 2);
    return;
  }

  auto make_double = [&] (SSATmp* src) {
    if (src->isA(Type::Int)) {
      return gen(ConvIntToDbl, src);
    } else if (src->isA(Type::Bool)) {
      return gen(ConvBoolToDbl, src);
    }
    assert(src->isA(Type::Dbl));
    return src;
  };

  divisor  = make_double(popC());
  dividend = make_double(popC());

  // on division by zero we spill false and exit with a warning
  auto exitSpillValues = peekSpillValues();
  exitSpillValues.push_back(cns(false));

  auto const exit = makeExitWarn(nextBcOff(), exitSpillValues,
                                 makeStaticString(Strings::DIVISION_BY_ZERO));

  assert(divisor->isA(Type::Dbl) && dividend->isA(Type::Dbl));
  push(gen(DivDbl, exit, dividend, divisor));
}

void HhbcTranslator::emitMod() {
  auto catchBlock1 = makeCatch();
  auto catchBlock2 = makeCatch();
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = gen(ConvCellToInt, catchBlock1, btr);
  SSATmp* tl = gen(ConvCellToInt, catchBlock2, btl);

  // We only want to decref btr and btl if the ConvCellToInt operation gave us
  // a new value back.
  if (tr != btr) gen(DecRef, btr);
  if (tl != btl) gen(DecRef, btl);
  // Exit path spills an additional false
  auto exitSpillValues = peekSpillValues();
  exitSpillValues.push_back(cns(false));

  // Generate an exit for the rare case that r is zero.  Interpreting
  // will raise a notice and produce the boolean false.  Punch out
  // here and resume after the Mod instruction; this should be rare.
  auto const exit = makeExitWarn(nextBcOff(), exitSpillValues,
                                 makeStaticString(Strings::DIVISION_BY_ZERO));
  gen(JmpZero, exit, tr);

  // We unfortunately need to special-case r = -1 here. In two's
  // complement, trying to divide INT_MIN by -1 will cause an integer
  // overflow.
  if (tr->isConst()) {
    // This whole block only exists so m_irb->cond doesn't get mad when one
    // of the branches gets optimized out due to constant folding.
    if (tr->intVal() == -1LL) {
      push(cns(0));
    } else if (tr->intVal() == 0) {
      // mod by zero is undefined. don't emit opmod for it because
      // this could cause issues in simplifier/codegen
      // this should never get reached anyway, we just need to dump
      // something on the stack
      push(cns(false));
    } else {
      push(gen(Mod, tl, tr));
    }
    return;
  }

  // check for -1 (dynamic version)
  SSATmp *res = m_irb->cond(
    0,
    [&] (Block* taken) {
      SSATmp* negone = gen(Eq, tr, cns(-1));
      gen(JmpNZero, taken, negone);
    },
    [&] {
      return gen(Mod, tl, tr);
    },
    [&] {
      m_irb->hint(Block::Hint::Unlikely);
      return cns(0);
    });
  push(res);
}

void HhbcTranslator::emitPow() {
  emitInterpOne(Type::UncountedInit, 2);
}

void HhbcTranslator::emitBitNot() {
  auto const srcType = topC()->type();
  if (srcType <= Type::Int) {
    auto const src = popC();
    push(gen(XorInt, src, cns(-1)));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = gen(ConvDblToInt, popC());
    push(gen(XorInt, src, cns(-1)));
    return;
  }

  auto const resultType = srcType <= Type::Str ? Type::Str
                        : srcType.needsReg() ? Type::Cell
                        : Type::Int;
  emitInterpOne(resultType, 1);
}


void HhbcTranslator::emitXor() {
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = gen(ConvCellToBool, btr);
  SSATmp* tl = gen(ConvCellToBool, btl);
  push(gen(XorBool, tl, tr));
  gen(DecRef, btl);
  gen(DecRef, btr);
}

void HhbcTranslator::emitShl() {
  auto catch1 = makeCatch();
  auto catch2 = makeCatch();
  auto shiftAmount = popC();
  auto lhs         = popC();

  auto lhsInt         = gen(ConvCellToInt, catch1, lhs);
  auto shiftAmountInt = gen(ConvCellToInt, catch2, shiftAmount);

  push(gen(Shl, lhsInt, shiftAmountInt));
  gen(DecRef, lhs);
  gen(DecRef, shiftAmount);
}

void HhbcTranslator::emitShr() {
  auto catch1 = makeCatch();
  auto catch2 = makeCatch();
  auto shiftAmount = popC();
  auto lhs         = popC();

  auto lhsInt         = gen(ConvCellToInt, catch1, lhs);
  auto shiftAmountInt = gen(ConvCellToInt, catch2, shiftAmount);

  push(gen(Shr, lhsInt, shiftAmountInt));
  gen(DecRef, lhs);
  gen(DecRef, shiftAmount);
}

//////////////////////////////////////////////////////////////////////

}}
