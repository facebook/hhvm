/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

const StaticString s_emptyString("");

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

Opcode toArrCmpOpcode(Op op) {
  switch (op) {
    case Op::Gt:    return GtArr;
    case Op::Gte:   return GteArr;
    case Op::Lt:    return LtArr;
    case Op::Lte:   return LteArr;
    case Op::Eq:    return EqArr;
    case Op::Same:  return SameArr;
    case Op::Neq:   return NeqArr;
    case Op::NSame: return NSameArr;
    case Op::Cmp:   return CmpArr;
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

void implNullCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TNull);
  auto const rightTy = right->type();

  // Left operand is null.

  if (rightTy <= TStr) {
    // Null converts to the empty string when being compared against a string.
    push(env,
         gen(env,
             toStrCmpOpcode(op),
             cns(env, s_emptyString.get()),
             right));
  } else if (rightTy <= TObj) {
    // When compared to an object, null is treated as false, and the object as
    // true. We cannot use ConvObjToBool here because that has special behavior
    // in certain cases, which we do not want here. Also note that no collection
    // check is done.
    push(env, emitConstCmp(env, op, false, true));
  } else {
    // Otherwise, convert both sides to booleans (with null becoming false).
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             cns(env, false),
             gen(env, ConvCellToBool, right)));
  }
}

void implBoolCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TBool);
  // Convert whatever is on the right to a boolean and compare. The conversion
  // may be a no-op if the right operand is already a bool.
  push(env,
       gen(env,
           toBoolCmpOpcode(op),
           left,
           gen(env, ConvCellToBool, right)));
}

void implIntCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TInt);
  auto const rightTy = right->type();

  // Left operand is int.

  if (rightTy <= TDbl) {
    // If compared against a double, promote to a double.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             gen(env, ConvIntToDbl, left),
             right));
  } else if (rightTy <= TStr) {
    // If compared against a string, commute the expression and do a specialized
    // string-int comparison.
    push(
      env,
      emitCommutedOp(
        env,
        op,
        [&](Op op){ return gen(env, toStrIntCmpOpcode(op), right, left); }
      )
    );
  } else if (rightTy.subtypeOfAny(TNull, TBool)) {
    // If compared against null or bool, convert both sides to bools.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             gen(env, ConvIntToBool, left),
             gen(env, ConvCellToBool, right)));
  } else if (rightTy <= TArr) {
    // All ints are implicity less than arrays.
    push(env, emitConstCmp(env, op, false, true));
  } else if (rightTy <= TObj) {
    // If compared against an object, emit a collection check before performing
    // the comparison.
    push(
      env,
      emitCollectionCheck(
        env,
        op,
        right,
        [&]{
          return gen(
            env,
            toIntCmpOpcode(op),
            left,
            gen(env, ConvObjToInt, right)
          );
        }
      )
    );
  } else {
    // For everything else, convert to an int. The conversion may be a no-op if
    // the right operand is already an int.
    push(env,
         gen(env,
             toIntCmpOpcode(op),
             left,
             gen(env, ConvCellToInt, right)));
  }
}

void implDblCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TDbl);
  auto const rightTy = right->type();

  // Left operand is a double.

  if (rightTy.subtypeOfAny(TNull, TBool)) {
    // If compared against null or bool, convert both sides to bools.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             gen(env, ConvDblToBool, left),
             gen(env, ConvCellToBool, right)));
  } else if (rightTy <= TArr) {
    // All doubles are implicitly less than arrays.
    push(env, emitConstCmp(env, op, false, true));
  } else if (rightTy <= TObj) {
    // If compared against an object, emit a collection check before performing
    // the comparison.
    push(
      env,
      emitCollectionCheck(
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
      )
    );
  } else {
    // For everything else, convert to a double. The conversion may be a no-op
    // if the right operand is already a double.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             left,
             gen(env, ConvCellToDbl, right)));
  }
}

void implArrCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TArr);
  auto const rightTy = right->type();

  // Left operand is an array.

  if (rightTy <= TArr) {
    // No conversion needed.
    push(env, gen(env, toArrCmpOpcode(op), left, right));
  } else if (rightTy.subtypeOfAny(TNull, TBool)) {
    // If compared against null or bool, convert both sides to bools.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             gen(env, ConvArrToBool, left),
             gen(env, ConvCellToBool, right)));
  } else if (rightTy <= TObj) {
    // Objects are always greater than arrays. Emit a collection check first.
    push(
      env,
      emitCollectionCheck(
        env,
        op,
        right,
        [&]{ return emitConstCmp(env, op, false, true); }
      )
    );
  } else {
    // Array is always greater than everything else.
    push(env, emitConstCmp(env, op, true, false));
  }
}

void implStrCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TStr);
  auto const rightTy = right->type();

  // Left operand is a string.

  if (rightTy <= TStr) {
    // No conversion needed.
    push(env, gen(env, toStrCmpOpcode(op), left, right));
  } else if (rightTy <= TNull) {
    // Comparisons against null are turned into comparisons with the empty
    // string.
    push(env,
         gen(env,
             toStrCmpOpcode(op),
             left,
             cns(env, s_emptyString.get())));
  } else if (rightTy <= TBool) {
    // If compared against a bool, convert the string to a bool.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             gen(env, ConvStrToBool, left),
             right));
  } else if (rightTy <= TInt) {
    // If compared against an integer, do no conversion and use the specialized
    // string-int comparison.
    push(env,
         gen(env,
             toStrIntCmpOpcode(op),
             left,
             right));
  } else if (rightTy <= TDbl) {
    // If compared against a double, convert the string to a double.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             gen(env, ConvStrToDbl, left),
             right));
  } else if (rightTy <= TRes) {
    // Bizarrely, comparison against a resource is done by converting both the
    // string and the resource to a double and comparing the two.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             gen(env, ConvStrToDbl, left),
             gen(env, ConvResToDbl, right)));
  } else if (rightTy <= TObj) {
    // If compared against an object, first do a collection check on the object,
    // and then emit an object-string comparison (swapping the order of the
    // operands).
    push(
      env,
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
      )
    );
  } else {
    // Strings are less than anything else (usually arrays).
    push(env, emitConstCmp(env, op, false, true));
  }
}

void implObjCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TObj);
  auto const rightTy = right->type();

  // Left operand is an object.

  if (rightTy <= TObj) {
    // No conversion needed.
    push(env, gen(env, toObjCmpOpcode(op), left, right));
  } else if (rightTy <= TBool) {
    // If compared against a bool, convert to a bool.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             gen(env, ConvObjToBool, left),
             right));
  } else if (rightTy <= TInt) {
    // If compared against an integer, emit a collection check before doing the
    // comparison.
    push(
      env,
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{
          return gen(
            env,
            toIntCmpOpcode(op),
            gen(env, ConvObjToInt, left),
            right
          );
        }
      )
    );
  } else if (rightTy <= TDbl) {
    // If compared against a double, emit a collection check before performing
    // the comparison.
    push(
      env,
      emitCollectionCheck(
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
      )
    );
  } else if (rightTy <= TStr) {
    // If compared against a string, first do a collection check, and then emit
    // an object-string comparison.
    push(
      env,
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{ return emitObjStrCmp(env, op, left, right); }
      )
    );
  } else if (rightTy <= TArr) {
    // Object is always greater than array, but we need a collection check
    // first.
    push(
      env,
      emitCollectionCheck(
        env,
        op,
        left,
        [&]{ return emitConstCmp(env, op, true, false); }
      )
    );
  } else {
    // For anything else, the object is always greater.
    push(env, emitConstCmp(env, op, true, false));
  }
}

void implResCmp(IRGS& env, Op op, SSATmp* left, SSATmp* right) {
  assert(left->type() <= TRes);
  auto const rightTy = right->type();

  // Left operand is a resource.

  if (rightTy <= TRes) {
    // No conversion needed.
    push(env, gen(env, toResCmpOpcode(op), left, right));
  } else if (rightTy <= TNull) {
    // Resources are always greater than nulls.
    push(env, emitConstCmp(env, op, true, false));
  } else if (rightTy <= TBool) {
    // If compared against a boolean, convert to a boolean.
    push(env,
         gen(env,
             toBoolCmpOpcode(op),
             cns(env, true),
             right));
  } else if (rightTy <= TInt) {
    // If compared against an integer, convert to an integer.
    push(env,
         gen(env,
             toIntCmpOpcode(op),
             gen(env, ConvResToInt, left),
             right));
  } else if (rightTy <= TDbl) {
    // If compared against a double, convert to a double.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             gen(env, ConvResToDbl, left),
             right));
  } else if (rightTy <= TStr) {
    // Bizaarly, comparison against a string is done by converting both the
    // string and the resource to a double and comparing the two.
    push(env,
         gen(env,
             toDblCmpOpcode(op),
             gen(env, ConvResToDbl, left),
             gen(env, ConvStrToDbl, right)));
  } else {
    // Resources are always less than anything else.
    push(env, emitConstCmp(env, op, false, true));
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

  // If it's a same-ish comparison and the types don't match (taking into
  // account Str and StaticStr), lower to a bool comparison of
  // constants. Otherwise, switch on the type of the left operand to emit the
  // right kind of comparison.
  if ((op == Op::Same || op == Op::NSame) &&
      !equivDataTypes(leftTy.toDataType(), rightTy.toDataType())) {
    push(env, emitConstCmp(env, op, false, true));
  } else if (leftTy <= TNull) implNullCmp(env, op, left, right);
  else if (leftTy <= TBool) implBoolCmp(env, op, left, right);
  else if (leftTy <= TInt) implIntCmp(env, op, left, right);
  else if (leftTy <= TDbl) implDblCmp(env, op, left, right);
  else if (leftTy <= TArr) implArrCmp(env, op, left, right);
  else if (leftTy <= TStr) implStrCmp(env, op, left, right);
  else if (leftTy <= TObj) implObjCmp(env, op, left, right);
  else if (leftTy <= TRes) implResCmp(env, op, left, right);
  else always_assert(false);

  decRef(env, left);
  decRef(env, right);
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
    decRef(env, c1);
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
  decRef(env, s1);
  if (s2 != c2) decRef(env, c2);
  if (s1 != c1) decRef(env, c1);
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
    decRef(env, t2);
    decRef(env, t1);
    return;
  }

  always_assert(n == 4);
  auto const t4 = popC(env);
  if (!(t4->type() <= TStr)) PUNT(ConcatN);

  push(env, gen(env, ConcatStr4, t4, t3, t2, t1));
  decRef(env, t3);
  decRef(env, t2);
  decRef(env, t1);
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
    // Update marker to ensure newly-pushed value isn't clobbered by DecRef.
    updateMarker(env);
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
  decRef(env, btl);
  decRef(env, btr);
}

void implShift(IRGS& env, Opcode op) {
  auto const shiftAmount    = popC(env);
  auto const lhs            = popC(env);
  auto const lhsInt         = gen(env, ConvCellToInt, lhs);
  auto const shiftAmountInt = gen(env, ConvCellToInt, shiftAmount);

  // - PHP7 defines shifts of width >= 64 to return the value you get from a
  //   naive shift, i.e., either 0 or -1 depending on the shift and value. This
  //   is notably *not* the semantics of the x86 shift instructions, so we need
  //   to do some comparison logic here.
  // - PHP7 defines negative shifts to throw an ArithmeticError.
  // - PHP5 semantics for such operations are machine-dependent.
  if (RuntimeOption::PHP7_IntSemantics &&
      !(shiftAmountInt->hasConstVal() &&
        shiftAmountInt->intVal() < 64 && shiftAmountInt->intVal() >= 0)) {
    push(env, cond(
      env,
      [&] (Block* taken) {
        auto const checkShift = gen(env, GteInt, shiftAmountInt, cns(env, 64));
        gen(env, JmpNZero, taken, checkShift);
      },
      [&] {
        return cond(
          env,
          [&] (Block* taken) {
            auto const checkShift =
              gen(env, LtInt, shiftAmountInt, cns(env, 0));
            gen(env, JmpNZero, taken, checkShift);
          },
          [&] {
            return gen(env, op, lhsInt, shiftAmountInt);
          },
          [&] {
            // Unlikely: shifting by a negative amount.
            hint(env, Block::Hint::Unlikely);

            gen(env, ThrowArithmeticError,
                cns(env, makeStaticString(Strings::NEGATIVE_SHIFT)));

            // Dead-code, but needed to satisfy cond().
            return cns(env, false);
          }
        );
      },
      [&] {
        // Unlikely: shifting by >= 64.
        hint(env, Block::Hint::Unlikely);

        if (op != Shr) {
          return cns(env, 0);
        }

        if (lhsInt->hasConstVal()) {
          int64_t lhsConst = lhsInt->intVal();
          if (lhsConst >= 0) {
            return cns(env, 0);
          } else {
            return cns(env, -1);
          }
        }

        return gen(env, op, lhsInt, cns(env, 63));
      }
    ));
  } else {
    push(env, gen(env, op, lhsInt, shiftAmountInt));
  }

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
  decRef(env, src);
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

  auto toDbl = [&] (SSATmp* x) {
    return
      x->isA(TInt)  ? gen(env, ConvIntToDbl, x) :
      x->isA(TBool) ? gen(env, ConvBoolToDbl, x) :
      x;
  };

  auto toInt = [&] (SSATmp* x) {
    return x->isA(TBool) ? gen(env, ConvBoolToInt, x) : x;
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
      auto const msg = cns(env, makeStaticString(Strings::DIVISION_BY_ZERO));
      gen(env, RaiseWarning, msg);

      // PHP5 results in false; we side exit since the type of the result
      // has now dramatically changed. PHP7 falls through to the IEEE
      // division semantics below (and doesn't side exit since the type is
      // still a double).
      if (!RuntimeOption::PHP7_IntSemantics) {
        push(env, cns(env, false));
        gen(env, Jmp, makeExit(env, nextBcOff(env)));
      } else if (!divisor->isA(TDbl) && !dividend->isA(TDbl)) {
        // We don't need to side exit here, but it's cleaner, and we assume
        // that division by zero is unikely
        push(env, gen(env, DivDbl, toDbl(dividend), toDbl(divisor)));
        gen(env, Jmp, makeExit(env, nextBcOff(env)));
      }
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
            gen(env, Jmp, makeExit(env, nextBcOff(env)));
          }
        );
      }
    );
  }

  auto const result = cond(
    env,
    [&] (Block* taken) {
      auto const mod = gen(env, Mod, toInt(dividend), toInt(divisor));
      gen(env, JmpNZero, taken, mod);
    },
    [&] { return gen(env, DivInt, toInt(dividend), toInt(divisor)); },
    [&] { return gen(env, DivDbl, toDbl(dividend), toDbl(divisor)); }
  );
  push(env, result);
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
      hint(env, Block::Hint::Unlikely);

      if (RuntimeOption::PHP7_IntSemantics) {
        auto const msg = cns(env, makeStaticString(Strings::MODULO_BY_ZERO));
        gen(env, ThrowDivisionByZeroError, msg);
      } else {
        // Make progress before side-exiting to the next instruction: raise a
        // warning and push false.
        auto const msg = cns(env, makeStaticString(Strings::DIVISION_BY_ZERO));
        gen(env, RaiseWarning, msg);
        decRef(env, btr);
        decRef(env, btl);
        push(env, cns(env, false));
        gen(env, Jmp, makeExit(env, nextBcOff(env)));
      }
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
