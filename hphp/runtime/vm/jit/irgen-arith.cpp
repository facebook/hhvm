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

#include "hphp/runtime/vm/jit/irgen-arith.h"

#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/base/tv-conv-notice.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

ConvNoticeData getConvNoticeDataForBitOp() {
   return ConvNoticeData {
      flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForBitOp),
      s_ConvNoticeReasonBitOp.get()
   };
}

ConvNoticeData getConvNoticeDataForMath() {
   return ConvNoticeData {
      flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForMath),
      s_ConvNoticeReasonMath.get(),
      false, // don't trigger notices from int <-> double
   };
}

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

// booleans in arithmetic and bitwise operations get cast to ints
SSATmp* promoteBool(IRGS& env, SSATmp* src, bool isBitOp) {
  if (!(src->type() <= TBool)) return src;
  handleConvNoticeLevel(env,
    isBitOp ? getConvNoticeDataForBitOp() : getConvNoticeDataForMath(),
    "bool",
    "int");
  return gen(env, ConvBoolToInt, src);
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

namespace {

void binaryBitOp(IRGS& env, Op op) {
  auto const type2 = topC(env, BCSPRelOffset{0})->type();
  auto const type1 = topC(env, BCSPRelOffset{1})->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    PUNT(BinaryBitOp-Unsupported);
    return;
  }

  auto const src2 = promoteBool(env, popC(env), true);
  auto const src1 = promoteBool(env, popC(env), true);
  push(env, gen(env, bitOp(op), src1, src2));
}

void binaryArith(IRGS& env, Op op) {
  auto const type2 = topC(env, BCSPRelOffset{0})->type();
  auto const type1 = topC(env, BCSPRelOffset{1})->type();
  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    // either an int or a dbl, but can't tell
    PUNT(BinaryArith-Unsupported);
  }

  auto const exitSlow = makeExitSlow(env);
  auto src2 = promoteBool(env, popC(env), false);
  auto src1 = promoteBool(env, popC(env), false);
  auto const opc = promoteBinaryDoubles(env, op, src1, src2);

  if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
    assertx(src1->isA(TInt) && src2->isA(TInt));
    push(env, gen(env, opc, exitSlow, src1, src2));
  } else {
    push(env, gen(env, opc, src1, src2));
  }
}

// Helpers for comparison generation:

Opcode toBoolCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtBool;
    case Op::Gte:   return GteBool;
    case Op::Lt:    return LtBool;
    case Op::Lte:   return LteBool;
    case Op::Eq:
    case Op::Same:  return EqBool;
    case Op::Neq:
    case Op::NSame: return NeqBool;
    case Op::Cmp:   return CmpBool;
    default: always_assert(false);
  }
}

Opcode toIntCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtInt;
    case Op::Gte:   return GteInt;
    case Op::Lt:    return LtInt;
    case Op::Lte:   return LteInt;
    case Op::Eq:
    case Op::Same:  return EqInt;
    case Op::Neq:
    case Op::NSame: return NeqInt;
    case Op::Cmp:   return CmpInt;
    default: always_assert(false);
  }
}

Opcode toDblCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtDbl;
    case Op::Gte:   return GteDbl;
    case Op::Lt:    return LtDbl;
    case Op::Lte:   return LteDbl;
    case Op::Eq:
    case Op::Same:  return EqDbl;
    case Op::Neq:
    case Op::NSame: return NeqDbl;
    case Op::Cmp:   return CmpDbl;
    default: always_assert(false);
  }
}

Opcode toStrCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtStr;
    case Op::Gte:   return GteStr;
    case Op::Lt:    return LtStr;
    case Op::Lte:   return LteStr;
    case Op::Eq:    return EqStr;
    case Op::Same:  return SameStr;
    case Op::Neq:   return NeqStr;
    case Op::NSame: return NSameStr;
    case Op::Cmp:   return CmpStr;
    default: always_assert(false);
  }
}

Opcode toStrIntCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtStrInt;
    case Op::Gte:   return GteStrInt;
    case Op::Lt:    return LtStrInt;
    case Op::Lte:   return LteStrInt;
    case Op::Eq:
    case Op::Same:  return EqStrInt;
    case Op::Neq:
    case Op::NSame: return NeqStrInt;
    case Op::Cmp:   return CmpStrInt;
    default: always_assert(false);
  }
}

Opcode toObjCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtObj;
    case Op::Gte:   return GteObj;
    case Op::Lt:    return LtObj;
    case Op::Lte:   return LteObj;
    case Op::Eq:    return EqObj;
    case Op::Same:  return SameObj;
    case Op::Neq:   return NeqObj;
    case Op::NSame: return NSameObj;
    case Op::Cmp:   return CmpObj;
    default: always_assert(false);
  }
}

Opcode toArrLikeCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtArrLike;
    case Op::Gte:   return GteArrLike;
    case Op::Lt:    return LtArrLike;
    case Op::Lte:   return LteArrLike;
    case Op::Eq:    return EqArrLike;
    case Op::Same:  return SameArrLike;
    case Op::Neq:   return NeqArrLike;
    case Op::NSame: return NSameArrLike;
    case Op::Cmp:   return CmpArrLike;
    default: always_assert(false);
  }
}

Opcode toArrLikeCmpOpcodeNoRelational(Op op) {
  switch (op) {
    case Op::Eq:    return EqArrLike;
    case Op::Same:  return SameArrLike;
    case Op::Neq:   return NeqArrLike;
    case Op::NSame: return NSameArrLike;
    default: always_assert(false);
  }
}

Opcode toResCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtRes;
    case Op::Gte:   return GteRes;
    case Op::Lt:    return LtRes;
    case Op::Lte:   return LteRes;
    case Op::Eq:
    case Op::Same:  return EqRes;
    case Op::Neq:
    case Op::NSame: return NeqRes;
    case Op::Cmp:   return CmpRes;
    default: always_assert(false);
  }
}

// Emit a commuted version of the specified operation. The given callable is
// invoked with the adjusted (if any) opcode to use. The callable should
// generate the proper instruction(s) to perform the comparison operation which
// matches the given opcode. The result from those instructions may be used as
// inputs to additional instructions. Regardless, the return value is the result
// of the commuted comparison.
template <typename F>
SSATmp* emitCommutedOp(IRGS& env, Op op, F f) {
  switch (op) {
    case Op::Gt:    return f(Op::Lt);
    case Op::Gte:   return f(Op::Lte);
    case Op::Lt:    return f(Op::Gt);
    case Op::Lte:   return f(Op::Gte);
    case Op::Eq:    return f(Op::Eq);
    case Op::Same:  return f(Op::Same);
    case Op::Neq:   return f(Op::Neq);
    case Op::NSame: return f(Op::NSame);
    case Op::Cmp:
      return gen(env, SubInt, cns(env, 0), f(Op::Cmp));
    default: always_assert(false);
  }
}

// Emit a check for whether the given object is a collection.
template <typename F>
SSATmp* emitCollectionCheck(IRGS& env, Op op, SSATmp* src, F f) {
  assertx(src->type() <= TObj);

  return cond(
    env,
    [&] (Block* taken) {
      auto const isCol = gen(env, IsCol, src);
      gen(env, JmpNZero, taken, isCol);
    },
    // Not a collection, just emit the code given by the callable.
    [&] { return f(); },
    [&] {
      // It's a collection, so either ThrowInvalidOperation, or return a
      // constant depending on the type of comparison being done.
      hint(env, Block::Hint::Unlikely);
      switch (op) {
        case Op::Gt:
        case Op::Gte:
        case Op::Lt:
        case Op::Lte:
        case Op::Cmp:
          gen(
            env,
            ThrowInvalidOperation,
            cns(env, s_cmpWithCollection.get())
          );
          // Dead-code, but needed to satisfy cond().
          return cns(env, false);
        case Op::Eq: return cns(env, false);
        case Op::Neq: return cns(env, true);
        default: always_assert(false);
      }
    }
  );
}

// Emit a comparison against an object and string. This needs special handling
// because the behavior varies depending on whether the object has a toString()
// method or not.
SSATmp* emitObjStrCmp(IRGS& env, Op op, SSATmp* obj, SSATmp* str) {
  assertx(obj->type() <= TObj);
  assertx(str->type() <= TStr);

  return cond(
    env,
    [&] (Block* taken) {
      auto const hasToString = gen(env, HasToString, obj);
      gen(env, JmpNZero, taken, hasToString);
    },
    [&] {
      // If there's no toString() method, than the object is always greater than
      // the string.
      switch (op) {
        case Op::Neq:
        case Op::Gt:
        case Op::Gte: return cns(env, true);
        case Op::Eq:
        case Op::Lt:
        case Op::Lte: return cns(env, false);
        case Op::Cmp: return cns(env, 1);
        default: always_assert(false);
      }
    },
    [&] {
      // If there is a toString() method, use it (via ConvObjToStr) to turn the
      // object into a string, and then do a string comparison.
      auto const converted = gen(env, ConvObjToStr, obj);
      auto const result = gen(env, toStrCmpOpcode(op), converted, str);
      decRef(env, converted);
      return result;
    }
  );
}

// Emit a boolean comparison against two constants. Will be simplified to a
// constant later on.
SSATmp* emitConstCmp(IRGS& env, Op op, bool left, bool right) {
  return gen(env, toBoolCmpOpcode(op), cns(env, left), cns(env, right));
}

SSATmp* emitHackArrBoolCmp(IRGS& env, Op op, SSATmp* arr, SSATmp* right) {
  auto const arrTy = arr->type();
  assertx(arrTy.subtypeOfAny(TVec, TDict, TKeyset));
  assertx(right->type() <= TBool);

  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp: {
      auto const m = arrTy <= TVec  ? s_cmpWithVec :
                     arrTy <= TDict ? s_cmpWithDict :
                                      s_cmpWithKeyset;
      gen(env, ThrowInvalidOperation, cns(env, m.get()));
      return cns(env, op == Op::Cmp ? 0 : false);
    }
    case Op::Eq:
    case Op::Neq:
      return gen(env, toBoolCmpOpcode(op), gen(env, ConvTVToBool, arr), right);
    default: always_assert(false);
  }
}

// Relational comparisons of vecs with non-vecs isn't allowed and will always
// throw. Equality comparisons always act as if they are not equal.
SSATmp* emitMixedVecCmp(IRGS& env, Op op) {
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithVec.get())
      );
      return cns(env, false);
    case Op::Same:
    case Op::Eq: return cns(env, false);
    case Op::NSame:
    case Op::Neq: return cns(env, true);
    default: always_assert(false);
  }
}

// Relational comparisons of dicts with non-dicts isn't allowed and will always
// throw. Equality comparisons always act as if they are not equal.
SSATmp* emitMixedDictCmp(IRGS& env, Op op) {
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithDict.get())
      );
      return cns(env, false);
    case Op::Same:
    case Op::Eq: return cns(env, false);
    case Op::NSame:
    case Op::Neq: return cns(env, true);
    default: always_assert(false);
  }
}

// Relational comparisons of keysets with non-keysets isn't allowed and will
// always throw. Equality comparisons always act as if they are not equal.
SSATmp* emitMixedKeysetCmp(IRGS& env, Op op) {
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithKeyset.get())
      );
      return cns(env, false);
    case Op::Same:
    case Op::Eq: return cns(env, false);
    case Op::NSame:
    case Op::Neq: return cns(env, true);
    default: always_assert(false);
  }
}

SSATmp* emitMixedFuncCmp(IRGS& env, Op op) {
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithFunc.get())
      );
      return cns(env, false);
    case Op::Same:
    case Op::Eq: return cns(env, false);
    case Op::NSame:
    case Op::Neq: return cns(env, true);
    default: always_assert(false);
  }
}

SSATmp* emitMixedClsMethCmp(IRGS& env, Op op) {
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(env, ThrowInvalidOperation, cns(env, s_cmpWithClsMeth.get()));
      return cns(env, false);
    case Op::Same:
    case Op::Eq:  return cns(env, false);
    case Op::NSame:
    case Op::Neq: return cns(env, true);
    default: always_assert(false);
  }
}

namespace {

void raiseClsMethCompareWarningHelper(IRGS& env, Op op) {
  assertx(RO::EvalIsCompatibleClsMethType);
  if (!RuntimeOption::EvalRaiseClsMethComparisonWarning) return;
  switch (op) {
    case Op::Gt:
    case Op::Gte:
    case Op::Lt:
    case Op::Lte:
    case Op::Cmp:
      gen(
        env,
        RaiseNotice,
        cns(env, makeStaticString(Strings::CLSMETH_COMPAT_NON_CLSMETH_REL_CMP))
      );
      return;
    case Op::Same:
    case Op::Eq:
    case Op::NSame:
    case Op::Neq: return;
    default: always_assert(false);
  }
}

}

SSATmp* implNullCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TNull);
  auto const rightTy = right->type();

  // Left operand is null.

  if (rightTy <= TStr) {
    // Null converts to the empty string when being compared against a string.
    return gen(
      env, toStrCmpOpcode(op), cns(env, staticEmptyString()), right);
  } else if (rightTy <= TObj) {
    // When compared to an object, null is treated as false, and the object as
    // true. We cannot use ConvObjToBool here because that has special behavior
    // in certain cases, which we do not want here. Also note that no collection
    // check is done.
    return emitConstCmp(env, op, false, true);
  } else if (rightTy.subtypeOfAny(TVec, TDict, TKeyset)) {
    // Treat null as false when comparing to an array.
    return
      emitCommutedOp(
        env,
        op,
        [&](Op op){
          return emitHackArrBoolCmp(env, op, right, cns(env, false));
        }
      );
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // Otherwise, convert both sides to booleans (with null becoming false).
    return gen(
      env,
      toBoolCmpOpcode(op),
      cns(env, false),
      gen(env, ConvTVToBool, right));
  }
}

SSATmp* implBoolCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TBool);
  auto const rightTy = right->type();

  if (rightTy.subtypeOfAny(TVec, TDict, TKeyset)) {
    return
      emitCommutedOp(
        env,
        op,
        [&](Op op){ return emitHackArrBoolCmp(env, op, right, left); }
      );
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // Convert whatever is on the right to a boolean and compare. The conversion
    // may be a no-op if the right operand is already a bool.
    return
      gen(env, toBoolCmpOpcode(op), left, gen(env, ConvTVToBool, right));
  }
}

SSATmp* implIntCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TInt);
  auto const rightTy = right->type();

  // Left operand is int.

  if (rightTy <= TDbl) {
    // If compared against a double, promote to a double.
    return gen(env, toDblCmpOpcode(op), gen(env, ConvIntToDbl, left), right);
  } else if (rightTy <= TStr) {
    // If compared against a string, commute the expression and do a specialized
    // string-int comparison.
    return
      emitCommutedOp(
        env,
        op,
        [&](Op op2){ return gen(env, toStrIntCmpOpcode(op2), right, left); }
      );
  } else if (rightTy.subtypeOfAny(TNull, TBool)) {
    // If compared against null or bool, convert both sides to bools.
    return gen(
      env,
      toBoolCmpOpcode(op),
      gen(env, ConvIntToBool, left),
      gen(env, ConvTVToBool, right));
  } else if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TObj) {
    // If compared against an object, emit a collection check before performing
    // the comparison.
    return emitCollectionCheck(
      env,
      op,
      right,
      [&]{
        return gen(
          env,
          toIntCmpOpcode(op),
          left,
          gen(env, ConvObjToInt, ConvNoticeData{}, right)
        );
      }
    );
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // For everything else, convert to an int. The conversion may be a no-op if
    // the right operand is already an int.
    return gen(
      env,
      toIntCmpOpcode(op),
      left,
      gen(env, ConvTVToInt, ConvNoticeData{}, right));
  }
}

SSATmp* implDblCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TDbl);
  auto const rightTy = right->type();

  // Left operand is a double.

  if (rightTy.subtypeOfAny(TNull, TBool)) {
    // If compared against null or bool, convert both sides to bools.
    return gen(
      env,
      toBoolCmpOpcode(op),
      gen(env, ConvDblToBool, left),
      gen(env, ConvTVToBool, right));
  } else if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TObj) {
    // If compared against an object, emit a collection check before performing
    // the comparison.
    return emitCollectionCheck(
      env,
      op,
      right,
      [&]{
        return gen(
          env,
          toDblCmpOpcode(op),
          left,
          gen(env, ConvObjToDbl, right)
        );
      }
    );
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // For everything else, convert to a double. The conversion may be a no-op
    // if the right operand is already a double.
    return gen(env, toDblCmpOpcode(op), left, gen(env, ConvTVToDbl, right));
  }
}

const StaticString
  s_funcToStringWarning(Strings::FUNC_TO_STRING),
  s_clsToStringWarning(Strings::CLASS_TO_STRING);

namespace {

SSATmp* equalClsMeth(IRGS& env, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TClsMeth);
  assertx(right->type() <= TClsMeth);
  return cond(
      env,
      [&](Block* taken) {
        auto const leftCls = gen(env, LdClsFromClsMeth, left);
        auto const rightCls = gen(env, LdClsFromClsMeth, right);
        assertx(leftCls->type() <= TCls);
        assertx(rightCls->type() <= TCls);
        gen(env, JmpZero, taken, gen(env, EqCls, leftCls, rightCls));
      },
      [&] {
        auto const leftFunc = gen(env, LdFuncFromClsMeth, left);
        auto const rightFunc = gen(env, LdFuncFromClsMeth, right);
        assertx(leftFunc->type() <= TFunc);
        assertx(rightFunc->type() <= TFunc);
        return gen(env, EqFunc, leftFunc, rightFunc);
       },
      [&] {
        return cns(env, false);
       }
    );
}

void raiseClsMethToVecWarningHelper(IRGS& env) {
  assertx(RO::EvalIsCompatibleClsMethType);
  if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
    gen(
      env,
      RaiseNotice,
      cns(env, makeStaticString("Implicit clsmeth to vec conversion")));
  }
}

}

SSATmp* implVecCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TVec);
  auto const rightTy = right->type();

  // Left operand is a vec.
  if (rightTy.subtypeOfAny(TNull, TBool)) {
    return emitHackArrBoolCmp(env, op, left, gen(env, ConvTVToBool, right));
  } else if (rightTy <= TVec) {
    return gen(env, toArrLikeCmpOpcode(op), left, right);
  } else if (rightTy <= TClsMeth && RO::EvalIsCompatibleClsMethType) {
    raiseClsMethToVecWarningHelper(env);
    auto const arr = convertClsMethToVec(env, right);
    const auto ret = gen(env, toArrLikeCmpOpcode(op), left, arr);
    decRef(env, arr);
    return ret;
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    return emitMixedVecCmp(env, op);
  }
}

SSATmp* implDictCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TDict);
  auto const rightTy = right->type();

  // Left operand is a dict.
  if (rightTy.subtypeOfAny(TNull, TBool)) {
    return emitHackArrBoolCmp(env, op, left, gen(env, ConvTVToBool, right));
  } else if (rightTy <= TDict) {
    // Dicts can't use relational comparisons.
    if (op == Op::Eq || op == Op::Neq ||
        op == Op::Same || op == Op::NSame) {
      return gen(env, toArrLikeCmpOpcodeNoRelational(op), left, right);
    } else {
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithDict.get())
      );
      return cns(env, false);
    }
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    return emitMixedDictCmp(env, op);
  }
}

SSATmp* implKeysetCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TKeyset);
  auto const rightTy = right->type();

  // Left operand is a keyset.
  if (rightTy.subtypeOfAny(TNull, TBool)) {
    return emitHackArrBoolCmp(env, op, left, gen(env, ConvTVToBool, right));
  } else if (rightTy <= TKeyset) {
    // Keysets can't use relational comparisons.
    if (op == Op::Eq || op == Op::Neq ||
        op == Op::Same || op == Op::NSame) {
      return gen(env, toArrLikeCmpOpcodeNoRelational(op), left, right);
    } else {
      gen(
        env,
        ThrowInvalidOperation,
        cns(env, s_cmpWithKeyset.get())
      );
      return cns(env, false);
    }
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    return emitMixedKeysetCmp(env, op);
  }
}

SSATmp* implStrCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TStr);

  if (right->isA(TCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    right = gen(env, LdClsName, right);
  }
  if (right->isA(TLazyCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    right = gen(env, LdLazyClsName, right);
  }

  auto const rightTy = right->type();

  // Left operand is a string.

  if (rightTy <= TStr) {
    // No conversion needed.
    return gen(env, toStrCmpOpcode(op), left, right);
  } else if (rightTy <= TNull) {
    // Comparisons against null are turned into comparisons with the empty
    // string.
    return gen(env, toStrCmpOpcode(op), left, cns(env, staticEmptyString()));
  } else if (rightTy <= TBool) {
    // If compared against a bool, convert the string to a bool.
    return gen(env, toBoolCmpOpcode(op), gen(env, ConvStrToBool, left), right);
  } else if (rightTy <= TInt) {
    // If compared against an integer, do no conversion and use the specialized
    // string-int comparison.
    return gen(env, toStrIntCmpOpcode(op), left, right);
  } else if (rightTy <= TDbl) {
    // If compared against a double, convert the string to a double.
    return gen(env, toDblCmpOpcode(op), gen(env, ConvStrToDbl, left), right);
  } else if (rightTy <= TRes) {
    // Bizarrely, comparison against a resource is done by converting both the
    // string and the resource to a double and comparing the two.
    return gen(
      env,
      toDblCmpOpcode(op),
      gen(env, ConvStrToDbl, left),
      gen(env, ConvResToDbl, right));
  } else if (rightTy <= TObj) {
    // If compared against an object, first do a collection check on the object,
    // and then emit an object-string comparison (swapping the order of the
    // operands).
    return
      emitCollectionCheck(
        env,
        op,
        right,
        [&]{
          return emitCommutedOp(
            env,
            op,
            [&](Op op) { return emitObjStrCmp(env, op, right, left); }
          );
        }
      );
  } else if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // Strings are less than anything else (usually arrays).
    return emitConstCmp(env, op, false, true);
  }
}

SSATmp* implObjCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TObj);
  auto const rightTy = right->type();

  // Left operand is an object.

  if (rightTy <= TObj) {
    // No conversion needed.
    return gen(env, toObjCmpOpcode(op), left, right);
  } else if (rightTy <= TBool) {
    // If compared against a bool, convert to a bool.
    return gen(env, toBoolCmpOpcode(op), gen(env, ConvObjToBool, left), right);
  } else if (rightTy <= TInt) {
    // If compared against an integer, emit a collection check before doing the
    // comparison.
    return emitCollectionCheck(
      env,
      op,
      left,
      [&]{
        return gen(
          env,
          toIntCmpOpcode(op),
          gen(env, ConvObjToInt, ConvNoticeData{}, left),
          right
        );
      }
    );
  } else if (rightTy <= TDbl) {
    // If compared against a double, emit a collection check before performing
    // the comparison.
    return emitCollectionCheck(
      env,
      op,
      left,
      [&]{
        return gen(
          env,
          toDblCmpOpcode(op),
          gen(env, ConvObjToDbl, left),
          right
        );
      }
    );
  } else if (rightTy <= TStr) {
    // If compared against a string, first do a collection check, and then emit
    // an object-string comparison.
    return
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{ return emitObjStrCmp(env, op, left, right); }
      );
  } else if (rightTy <= TCls) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    return
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{ return emitObjStrCmp(env, op, left, gen(env, LdClsName, right)); }
      );
  } else if (rightTy <= TLazyCls) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    return
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{
          return emitObjStrCmp(env, op, left, gen(env, LdLazyClsName, right));
        }
      );
  } else if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // For anything else, the object is always greater.
    return emitConstCmp(env, op, true, false);
  }
}

SSATmp* implResCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TRes);
  auto const rightTy = right->type();

  // Left operand is a resource.

  if (rightTy <= TRes) {
    // No conversion needed.
    return gen(env, toResCmpOpcode(op), left, right);
  } else if (rightTy <= TNull) {
    // Resources are always greater than nulls.
    return emitConstCmp(env, op, true, false);
  } else if (rightTy <= TBool) {
    // If compared against a boolean, convert to a boolean.
    return gen(env, toBoolCmpOpcode(op), cns(env, true), right);
  } else if (rightTy <= TInt) {
    // If compared against an integer, convert to an integer.
    return gen(env, toIntCmpOpcode(op), gen(env, ConvResToInt, left), right);
  } else if (rightTy <= TDbl) {
    // If compared against a double, convert to a double.
    return gen(env, toDblCmpOpcode(op), gen(env, ConvResToDbl, left), right);
  } else if (rightTy <= TStr) {
    // Bizaarly, comparison against a string is done by converting both the
    // string and the resource to a double and comparing the two.
    return gen(
      env,
      toDblCmpOpcode(op),
      gen(env, ConvResToDbl, left),
      gen(env, ConvStrToDbl, right));
  } else if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TFunc) {
    return emitMixedFuncCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    // Resources are always less than anything else.
    return emitConstCmp(env, op, false, true);
  }
}

SSATmp* implFunCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  auto const rightTy = right->type();

  if (rightTy <= TFunc) {
    if (op == Op::Eq || op == Op::Same) {
      return gen(env, EqFunc, left, right);
    }
    if (op == Op::Neq || op == Op::NSame) {
      return gen(env, XorBool, gen(env, EqFunc, left, right), cns(env, true));
    }
    PUNT(Func-cmp);
  }

  if (rightTy <= TVec) {
    return emitMixedVecCmp(env, op);
  } else if (rightTy <= TDict) {
    return emitMixedDictCmp(env, op);
  } else if (rightTy <= TKeyset) {
    return emitMixedKeysetCmp(env, op);
  } else if (rightTy <= TClsMeth) {
    return emitMixedClsMethCmp(env, op);
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else {
    return emitMixedFuncCmp(env, op);
  }
}

SSATmp* implLazyClsCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  auto const rightTy = right->type();

  if (rightTy <= TLazyCls) {
    if (op == Op::Eq || op == Op::Same) {
      return gen(env, EqLazyCls, left, right);
    }
    if (op == Op::Neq || op == Op::NSame) {
      return gen(env, XorBool, gen(env, EqLazyCls, left, right), cns(env, true));
    }
  }

  if (rightTy <= TStr) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    return implStrCmp(env, op, gen(env, LdLazyClsName, left), right);
  }

  if (rightTy <= TCls) {
    if (op == Op::Eq || op == Op::Neq || op == Op::Same || op == Op::NSame) {
      auto const lstr = gen(env, LdLazyClsName, left);
      auto const rstr = gen(env, LdClsName, right);
      return implStrCmp(env, op, lstr, rstr);
    }
  }

  PUNT(LazyCls-cmp);
}

SSATmp* implClsCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  auto const rightTy = right->type();

  if (rightTy <= TCls) {
    if (op == Op::Eq || op == Op::Same) {
      return gen(env, EqCls, left, right);
    }
    if (op == Op::Neq || op == Op::NSame) {
      return gen(env, XorBool, gen(env, EqCls, left, right), cns(env, true));
    }
  }

  if (rightTy <= TStr) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    auto const str = gen(env, LdClsName, left);
    return implStrCmp(env, op, str, right);
  }

  if (rightTy <= TLazyCls) {
    if (op == Op::Eq || op == Op::Neq || op == Op::Same || op == Op::NSame) {
      auto const lstr = gen(env, LdClsName, left);
      auto const rstr = gen(env, LdLazyClsName, right);
      return implStrCmp(env, op, lstr, rstr);
    }
  }

  PUNT(Cls-cmp);
}

SSATmp* implClsMethCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  auto const rightTy = right->type();
  if (rightTy <= TClsMeth) {
    if (op == Op::Eq || op == Op::Same) {
      return equalClsMeth(env, left, right);
    }
    if (op == Op::Neq || op == Op::NSame) {
      return gen(env, XorBool, equalClsMeth(env, left, right), cns(env, true));
    }
    PUNT(ClsMeth-ClsMeth-cmp);
  }

  // Left (TClsMeth) is compatible with vec
  if (rightTy <= TVec && RO::EvalIsCompatibleClsMethType) {
    raiseClsMethToVecWarningHelper(env);
    auto const arr = convertClsMethToVec(env, left);
    const auto ret = implVecCmp(env, op, arr, right);
    decRef(env, arr);
    return ret;
  } else if (rightTy <= TRFunc) {
    PUNT(RFunc-cmp);
  } else if (rightTy <= TRClsMeth) {
    PUNT(RClsMeth-cmp);
  } else if (rightTy <= TRecord) {
    PUNT(Record-cmp);
  }

  return emitMixedClsMethCmp(env, op);
}

SSATmp* implRecordCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  PUNT(Record-cmp);
}

SSATmp* implRFuncCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  PUNT(RFunc-cmp);
}

SSATmp* implRClsMethCmp(IRGS& env, Op, SSATmp*, SSATmp*) {
  PUNT(RClsMeth-cmp);
}

namespace {

bool isEqualityOp(Op op) { return op == Op::Eq || op == Op::Neq; }

void maybeRaiseBadComparisonViolation(
  IRGS& env, Op op, SSATmp* comp_result, SSATmp* lhs, SSATmp* rhs) {
  const bool eqOp = isEqualityOp(op);
  const auto level = flagToConvNoticeLevel(
    eqOp
      ? RuntimeOption::EvalNoticeOnCoerceForEq
      : RuntimeOption::EvalNoticeOnCoerceForCmp);
  // we should never be getting here if conv level is change
  assertx(level != ConvNoticeLevel::Change);
  if (level == ConvNoticeLevel::None) return;
  if (!eqOp) {
    gen(env, RaiseBadComparisonViolation, BadComparisonData{false}, lhs, rhs);
  } else {
    // only trigger the notice if we're eq and the result is true or
    // neq and the result is false since otherwise a coercion is
    // impossible
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, op == Op::Eq ? JmpNZero : JmpZero, taken, comp_result);
      },
      [&] {
        gen(env, RaiseBadComparisonViolation, BadComparisonData{true}, lhs, rhs);
      }
    );
  }
}
}

/*
 * Responsible for converting the bytecode comparisons (which are type-agnostic)
 * to IR comparisons (which are typed). This generally involves inserting the
 * right kind of type conversions to satisfy PHP semantics. For a few special
 * cases, (object-string and string-int), we have special IR opcodes because the
 * required semantics cannot be easily represented via type conversions.
 */
void implCmp(IRGS& env, Op op) {
  auto const right = popC(env);
  auto const left = popC(env);
  auto const leftTy = left->type();
  auto const rightTy = right->type();

  if (!leftTy.isKnownDataType() || !rightTy.isKnownDataType()) {
    // Can't do much if we don't even know the types.
    PUNT(cmpUnknownDataType);
  }

  auto equiv = [&] {
    return
      equivDataTypes(leftTy.toDataType(), rightTy.toDataType()) ||
      (isClassType(leftTy.toDataType()) &&
       isStringType(rightTy.toDataType()))||
      (isClassType(leftTy.toDataType()) &&
       isLazyClassType(rightTy.toDataType()))||
      (isLazyClassType(leftTy.toDataType()) &&
       isClassType(rightTy.toDataType()))||
      (isLazyClassType(leftTy.toDataType()) &&
       isStringType(rightTy.toDataType()))||
      (isStringType(leftTy.toDataType()) &&
        isClassType(rightTy.toDataType())) ||
      (isStringType(leftTy.toDataType()) &&
        isLazyClassType(rightTy.toDataType())) ||
      (isClsMethType(leftTy.toDataType()) &&
        isArrayLikeType(rightTy.toDataType()) &&
        RO::EvalIsCompatibleClsMethType) ||
      (isArrayLikeType(leftTy.toDataType()) &&
        isClsMethType(rightTy.toDataType()) &&
        RO::EvalIsCompatibleClsMethType);
  };

  // If it's a same-ish comparison and the types don't match (taking into
  // account Str and StaticStr), lower to a bool comparison of
  // constants. Otherwise, switch on the type of the left operand to emit the
  // right kind of comparison.
  if (((op == Op::Same || op == Op::NSame) ||
        (isEqualityOp(op) && useStrictEquality())) &&
       !equiv()) {
    push(env, emitConstCmp(env, op, false, true));
  } else {
    SSATmp* res = [&]() {
      if (leftTy <= TNull) return implNullCmp(env, op, left, right);
      else if (leftTy <= TBool) return implBoolCmp(env, op, left, right);
      else if (leftTy <= TInt) return implIntCmp(env, op, left, right);
      else if (leftTy <= TDbl) return implDblCmp(env, op, left, right);
      else if (leftTy <= TVec) return implVecCmp(env, op, left, right);
      else if (leftTy <= TDict) return implDictCmp(env, op, left, right);
      else if (leftTy <= TKeyset) return implKeysetCmp(env, op, left, right);
      else if (leftTy <= TStr) return implStrCmp(env, op, left, right);
      else if (leftTy <= TObj) return implObjCmp(env, op, left, right);
      else if (leftTy <= TRes) return implResCmp(env, op, left, right);
      else if (leftTy <= TFunc) return implFunCmp(env, op, left, right);
      else if (leftTy <= TCls) return implClsCmp(env, op, left, right);
      else if (leftTy <= TLazyCls) return implLazyClsCmp(env, op, left, right);
      else if (leftTy <= TClsMeth) return implClsMethCmp(env, op, left, right);
      else if (leftTy <= TRecord) return implRecordCmp(env, op, left, right);
      else if (leftTy <= TRFunc) return implRFuncCmp(env, op, left, right);
      else if (leftTy <= TRClsMeth) return implRClsMethCmp(env, op, left, right);
      else always_assert(false);
      not_reached();
    }();

    if (op != Op::Same && op != Op::NSame && !equiv()
      && (isEqualityOp(op) ||
          (!(leftTy <= TInt && rightTy <= TDbl) &&
          !(leftTy <= TDbl && rightTy <= TInt)))) {
       maybeRaiseBadComparisonViolation(env, op, res, left, right);
    }

    push(env, res);
  }

  decRef(env, left);
  decRef(env, right);
}

void implAdd(IRGS& env, Op op) {
  binaryArith(env, op);
}

template<class PreDecRef>
void implConcat(IRGS& env, SSATmp* c1, SSATmp* c2, PreDecRef preDecRef) {
  auto cast =
    [&] (SSATmp* s) {
      if (s->isA(TStr)) return s;
      const ConvNoticeLevel notice_level =
        flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForStrConcat);
      auto const ret = gen(
        env,
        ConvTVToStr,
        ConvNoticeData{notice_level, s_ConvNoticeReasonConcat.get()},
        s);
      decRef(env, s);
      return ret;
    };

  /*
   * We have some special translations for common combinations that avoid extra
   * conversion calls.
   */
  auto const str =
    [&] () -> SSATmp* {
      if (c1->isA(TInt)) return gen(env, ConcatStrInt, cast(c2), c1);
      if (c2->isA(TInt)) return gen(env, ConcatIntStr, c2, c1 = cast(c1));
      return nullptr;
    }();

  if (str) {
    preDecRef(str);
    // Note that the ConcatFoo opcode consumed the reference on its first
    // argument, so we only need to decref the second one.
    decRef(env, c1);
    return;
  }

  /*
   * Generic translation: convert both to strings, and then concatenate them.
   *
   * NB: the order we convert to strings is observable (because of __toString
   * methods).
   *
   * We don't want to convert to strings if either was already a string.  Note
   * that for the c2 string, failing to do this could change big-O program
   * behavior if refcount opts were off, since we'd COW strings that we
   * shouldn't (a ConvTVToStr of a Str will simplify into an IncRef).
   */
  auto const s2 = cast(c2);
  auto const s1 = cast(c1);
  auto const r  = gen(env, ConcatStrStr, s2, s1);  // consumes s2 reference
  preDecRef(r);
  decRef(env, s1);
}

//////////////////////////////////////////////////////////////////////

}

void emitConcat(IRGS& env) {
  auto const c1 = popC(env);
  auto const c2 = popC(env);
  implConcat(env, c1, c2, [&] (SSATmp* r) { push(env, r); });
}

void emitConcatN(IRGS& env, uint32_t n) {
  if (n == 2) return emitConcat(env);

  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const t3 = popC(env);
  auto const t4 = n == 4 ? popC(env) : nullptr;

  const ConvNoticeLevel level =
    flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForStrConcat);
  const auto convData = ConvNoticeData{level, s_ConvNoticeReasonConcat.get()};
  auto const s4 = !t4 || t4->isA(TStr) ? t4 : gen(env, ConvTVToStr, convData, t4);
  auto const s3 = t3->isA(TStr) ? t3 : gen(env, ConvTVToStr, convData, t3);
  auto const s2 = t2->isA(TStr) ? t2 : gen(env, ConvTVToStr, convData, t2);
  auto const s1 = t1->isA(TStr) ? t1 : gen(env, ConvTVToStr, convData, t1);

  if (n == 3) {
    push(env, gen(env, ConcatStr3, s3, s2, s1));
  } else {
    always_assert(n == 4);

    push(env, gen(env, ConcatStr4, s4, s3, s2, s1));
    decRef(env, s3);
  }
  decRef(env, s2);
  decRef(env, s1);

  if (s1 != t1) decRef(env, t1);
  if (s2 != t2) decRef(env, t2);
  if (s3 != t3) decRef(env, t3);
  if (s4 != t4) decRef(env, t4);
}

void emitSetOpL(IRGS& env, int32_t id, SetOpOp subop) {
  auto const subOpc = [&]() -> Optional<Op> {
    switch (subop) {
    case SetOpOp::PlusEqual:   return Op::Add;
    case SetOpOp::MinusEqual:  return Op::Sub;
    case SetOpOp::MulEqual:    return Op::Mul;
    case SetOpOp::PlusEqualO:  return Op::AddO;
    case SetOpOp::MinusEqualO: return Op::SubO;
    case SetOpOp::MulEqualO:   return Op::MulO;
    case SetOpOp::DivEqual:    return std::nullopt;
    case SetOpOp::ConcatEqual: return Op::Concat;
    case SetOpOp::ModEqual:    return std::nullopt;
    case SetOpOp::PowEqual:    return std::nullopt;
    case SetOpOp::AndEqual:    return Op::BitAnd;
    case SetOpOp::OrEqual:     return Op::BitOr;
    case SetOpOp::XorEqual:    return Op::BitXor;
    case SetOpOp::SlEqual:     return std::nullopt;
    case SetOpOp::SrEqual:     return std::nullopt;
    }
    not_reached();
  }();
  if (!subOpc) PUNT(SetOpL-Unsupported);

  auto loc = ldLoc(env, id, DataTypeGeneric);

  if (*subOpc == Op::Concat) {
    /*
     * The concat helpers incref their results, which will be consumed by
     * the stloc. We need an extra incref for the push onto the stack.
     */
    auto const val    = popC(env);
    env.irb->constrainValue(loc, DataTypeSpecific);
    implConcat(env, val, loc, [&] (SSATmp* result) {
      pushIncRef(env, stLocNRC(env, id, result));
    });
    return;
  }

  if (!areBinaryArithTypesSupported(*subOpc, loc->type(), topC(env)->type())) {
    PUNT(SetOpL);
  }

  auto const exitSlow = makeExitSlow(env);
  auto val = popC(env);
  env.irb->constrainValue(loc, DataTypeSpecific);
  bool isBitOp_ = isBitOp(*subOpc);
  loc = promoteBool(env, loc, isBitOp_);
  val = promoteBool(env, val, isBitOp_);
  Opcode opc;
  if (isBitOp_) {
    opc = bitOp(*subOpc);
  } else {
    opc = promoteBinaryDoubles(env, *subOpc, loc, val);
  }

  auto const result = opc == AddIntO || opc == SubIntO || opc == MulIntO
    ? gen(env, opc, exitSlow, loc, val)
    : gen(env, opc, loc, val);
  pushStLoc(env, id, result);
}

void emitIncDecL(IRGS& env, NamedLocal loc, IncDecOp subop) {
  auto const src = ldLocWarn(env, loc, DataTypeSpecific);

  if (auto const result = incDec(env, subop, src)) {
    pushIncRef(env, isPre(subop) ? result : src);
    // Update marker to ensure newly-pushed value isn't clobbered by DecRef.
    updateMarker(env);
    stLoc(env, loc.id, result);
    return;
  }

  PUNT(IncDecL);
}

void implShift(IRGS& env, Opcode op) {
  auto const shiftAmount    = popC(env);
  auto const lhs            = popC(env);
  auto const lhsInt         = gen(env, ConvTVToInt, getConvNoticeDataForBitOp(), lhs);
  auto const shiftAmountInt = gen(env, ConvTVToInt, getConvNoticeDataForBitOp(), shiftAmount);

  // - PHP7 defines shifts of width >= 64 to return the value you get from a
  //   naive shift, i.e., either 0 or -1 depending on the shift and value. This
  //   is notably *not* the semantics of the x86 shift instructions, so we need
  //   to do some comparison logic here.
  // - PHP7 defines negative shifts to throw an ArithmeticError.
  // - PHP5 semantics for such operations are machine-dependent.
  push(env, gen(env, op, lhsInt, shiftAmountInt));
  decRef(env, lhs);
  decRef(env, shiftAmount);
}

void emitShl(IRGS& env) {
  implShift(env, Shl);
}

void emitShr(IRGS& env) {
  implShift(env, Shr);
}

void emitPow(IRGS& env) {
  // Special-case exponent of 2 or 3, i.e.
  // $x**2 becomes $x*$x,
  // $x**3 becomes ($x*$x)*$x
  auto exponent = topC(env);
  auto base = topC(env, BCSPRelOffset{1});
  if ((exponent->hasConstVal(2) || exponent->hasConstVal(3)) &&
      (base->isA(TDbl) || base->isA(TInt))) {
    auto const intVal = exponent->intVal();
    auto const isCube = intVal == 3;

    auto makeExitPow = [&] (SSATmp* src, bool computeSquare) {
      auto const exit = defBlock(env, Block::Hint::Unlikely);
      BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), exit);
      assertx(src->isA(TInt));
      src = gen(env, ConvIntToDbl, src);
      SSATmp* genPowResult;
      if (computeSquare) {
        genPowResult = gen(env, MulDbl, src, src);
        if (isCube) {
          genPowResult = gen(env, MulDbl, genPowResult, src);
        }
      } else {
        assertx(base->isA(TInt));
        auto const src1 = gen(env, ConvIntToDbl, base);
        genPowResult = gen(env, MulDbl, src, src1);
      }
      discard(env, 2);
      push(env, genPowResult);
      gen(env, Jmp, makeExit(env, nextSrcKey(env)));
      return exit;
    };

    SSATmp* genPowResult;
    if (base->isA(TInt)) {
      auto const exitPow = makeExitPow(base, true);
      genPowResult = gen(env, MulIntO, exitPow, base, base);
    } else {
      genPowResult = gen(env, MulDbl, base, base);
    }
    if (isCube) {
      if (genPowResult->isA(TInt)) {
        auto const exitPow = makeExitPow(genPowResult, false);
        genPowResult = gen(env, MulIntO, exitPow, genPowResult, base);
      } else {
        genPowResult = gen(env, MulDbl, genPowResult, base);
      }
    }
    discard(env, 2);
    push(env, genPowResult);
    return;
  }
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
    handleConvNoticeLevel(env, getConvNoticeDataForBitOp(), "double", "int");
    auto const src = gen(env, ConvDblToInt, popC(env));
    push(env, gen(env, XorInt, src, cns(env, -1)));
    return;
  }

  auto const resultType = srcType <= TStr || srcType <= TCls
                        ? TStr
                        : (srcType.needsReg() ? TCell : TInt);
  interpOne(env, resultType, 1);
}


void emitNot(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, XorBool, gen(env, ConvTVToBool, src), cns(env, true)));
  decRef(env, src);
}

const StaticString s_DIVISION_BY_ZERO(Strings::DIVISION_BY_ZERO);
void emitDiv(IRGS& env) {
  auto const divisorType  = topC(env, BCSPRelOffset{0})->type();
  auto const dividendType = topC(env, BCSPRelOffset{1})->type();

  auto isNumeric = [&] (Type type) {
    return type.subtypeOfAny(TInt, TDbl, TBool);
  };

  // not going to bother with string division etc.
  if (!isNumeric(divisorType) || !isNumeric(dividendType)) {
    interpOne(env, TUncountedInit, 2);
    return;
  }

  auto toDbl = [&] (SSATmp* x) {
    if (x->isA(TBool)) {
       // say int to match interp due to just using the float version of the int
      handleConvNoticeLevel(env, getConvNoticeDataForMath(), "bool", "int");
      return gen(env, ConvBoolToDbl, x);
    }
    return x->isA(TInt) ? gen(env, ConvIntToDbl, x) : x;
  };

  auto const divisor  = popC(env);
  auto const dividend = popC(env);

  ifThen(
    env,
    [&] (Block* taken) {
      auto const checkZero =
        divisor->isA(TInt) ? gen(env, EqInt,  divisor, cns(env, 0)) :
        divisor->isA(TDbl) ? gen(env, EqDbl,  divisor, cns(env, 0.0)) :
                             gen(env, EqBool, divisor, cns(env, false));
      gen(env, JmpNZero, taken, checkZero);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowDivisionByZeroException);
      }
  );

  if (divisor->isA(TDbl) || dividend->isA(TDbl)) {
    push(env, gen(env, DivDbl, toDbl(dividend), toDbl(divisor)));
    return;
  }

  if (divisor->isA(TInt) && dividend->isA(TInt)) {
    ifThen(
      env,
      [&] (Block* taken) {
        auto const badDividend = gen(env, EqInt, dividend, cns(env, LLONG_MIN));
        gen(env, JmpNZero, taken, badDividend);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        ifThen(
          env,
          [&] (Block* taken) {
            auto const badDivisor = gen(env, EqInt, divisor, cns(env, -1));
            gen(env, JmpNZero, taken, badDivisor);
          },
          [&] {
            hint(env, Block::Hint::Unlikely);

            // Avoid SIGFPE when dividing the miniumum respresentable integer
            // by -1.
            push(env, gen(env, DivDbl, toDbl(dividend), toDbl(divisor)));
            gen(env, Jmp, makeExit(env, nextSrcKey(env)));
          }
        );
      }
    );
  }

  auto const result = cond(
    env,
    [&] (Block* taken) {
      // We don't want to trigger notices for conversions from here because
      // they'd be duplicated in the result of the condition
      auto toInt = [&] (SSATmp* x) {
        return x->isA(TBool) ? gen(env, ConvBoolToInt, x) : x;
      };
      auto const mod = gen(env, Mod, toInt(dividend), toInt(divisor));
      gen(env, JmpNZero, taken, mod);
    },
    [&] {
      return gen(env,
                 DivInt,
                 promoteBool(env, dividend, false),
                 promoteBool(env, divisor, false));
    },
    [&] { return gen(env, DivDbl, toDbl(dividend), toDbl(divisor)); }
  );
  push(env, result);
}

void emitMod(IRGS& env) {
  auto const btr = popC(env);
  auto const btl = popC(env);
  auto const tl = gen(env, ConvTVToInt, getConvNoticeDataForMath(), btl);
  auto const tr = gen(env, ConvTVToInt, getConvNoticeDataForMath(), btr);

  // Generate an exit for the rare case that r is zero.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, JmpZero, taken, tr);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowDivisionByZeroException);
    }
  );

  // DecRefs on the main line must happen after the potentially-throwing exit
  // above: if we throw during the RaiseWarning, those values must still be on
  // the stack.
  decRef(env, btr);
  decRef(env, btl);

  // Check for -1.  The Mod IR instruction has undefined behavior for -1, but
  // php semantics are to return zero.
  auto const res = cond(
    env,
    [&] (Block* taken) {
      auto const negone = gen(env, EqInt, tr, cns(env, -1));
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

void emitGt(IRGS& env)     { implCmp(env, Op::Gt);    }
void emitGte(IRGS& env)    { implCmp(env, Op::Gte);   }
void emitLt(IRGS& env)     { implCmp(env, Op::Lt);    }
void emitLte(IRGS& env)    { implCmp(env, Op::Lte);   }
void emitEq(IRGS& env)     { implCmp(env, Op::Eq);    }
void emitNeq(IRGS& env)    { implCmp(env, Op::Neq);   }
void emitSame(IRGS& env)   { implCmp(env, Op::Same);  }
void emitNSame(IRGS& env)  { implCmp(env, Op::NSame); }
void emitCmp(IRGS& env)    { implCmp(env, Op::Cmp); }

void emitAdd(IRGS& env)    { implAdd(env, Op::Add); }
void emitAddO(IRGS& env)   { implAdd(env, Op::AddO); }

//////////////////////////////////////////////////////////////////////

}}}
