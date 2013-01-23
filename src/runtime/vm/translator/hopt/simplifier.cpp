/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/vm/translator/hopt/simplifier.h"

#include <sstream>
#include <type_traits>

#include "runtime/vm/translator/hopt/tracebuilder.h"
#include "runtime/vm/runtime.h"

namespace HPHP {
namespace VM {
namespace JIT {

TRACE_SET_MOD(hhir);

void Simplifier::copyProp(IRInstruction* inst) {
  for (uint32 i = 0; i < inst->getNumSrcs(); i++) {
    IRInstruction* srcInst = inst->getSrc(i)->getInstruction();
    if (srcInst->getOpcode() == Mov) {
      inst->setSrc(i, srcInst->getSrc(0));
    } else if (srcInst->getOpcode() == IncRef &&
               !isRefCounted(srcInst->getSrc(0))) {
      srcInst->setOpcode(Mov);
      inst->setSrc(i, srcInst->getSrc(0));
    }
  }
}

static void unimplementedSimplify(Opcode opc) {
  // Do not assert(false), it is fine to not simplify as the default
  TRACE(3, "HHIR Simplifier: unimplemented support for opcode %s\n",
        opcodeName(opc));
  return;
}

static bool isNotInst(SSATmp *src1, SSATmp *src2) {
  // right operand should be 1
  if (!src2->isConst() || src2->getType() != Type::Int ||
      src2->getConstValAsInt() != 1) {
    return false;
  }
  // left operand should be a boolean
  if (src1->getType() != Type::Bool) {
    return false;
  }
  return true;
}

static bool isNotInst(SSATmp *tmp) {
  IRInstruction* inst = tmp->getInstruction();
  if (inst->getOpcode() != OpXor) {
    return false;
  }
  return isNotInst(inst->getSrc(0), inst->getSrc(1));
}

SSATmp* Simplifier::simplify(IRInstruction* inst) {
  SSATmp* src1 = inst->getSrc(0);
  SSATmp* src2 = inst->getSrc(1);

  switch (inst->getOpcode()) {
  case OpAdd:       return simplifyAdd(src1, src2);
  case OpSub:       return simplifySub(src1, src2);
  case OpMul:       return simplifyMul(src1, src2);
  case OpAnd:       return simplifyAnd(src1, src2);
  case OpOr:        return simplifyOr(src1, src2);
  case OpXor:       return simplifyXor(src1, src2);
  case OpGt:        return simplifyGt(src1, src2);
  case OpGte:       return simplifyGte(src1, src2);
  case OpLt:        return simplifyLt(src1, src2);
  case OpLte:       return simplifyLte(src1, src2);
  case OpEq:        return simplifyEq(src1, src2);
  case OpNeq:       return simplifyNeq(src1, src2);
  case OpSame:      return simplifySame(src1, src2);
  case OpNSame:     return simplifyNSame(src1, src2);
  case Concat:      return simplifyConcat(src1, src2);
  case InstanceOfD: return simplifyInstanceOfD(src1, src2, false);
  case NInstanceOfD:return simplifyInstanceOfD(src1, src2, true);
  case Mov:         return simplifyMov(src1);

  case LdClsPropAddr:
    return simplifyLdClsPropAddr(src1, src2, inst->getSrc(2));
  case Conv:
    return simplifyConv(inst);
  case Unbox:
    return simplifyUnbox(inst);
  case UnboxPtr:
    return simplifyUnboxPtr(inst);
  case IsType:
    return simplifyIsType(inst);

  case CheckInit:
    return simplifyCheckInit(inst);

  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpSame:
  case JmpNSame:
  case JmpZero:
  case JmpNZero:
  case Jmp_:
  case JmpInstanceOfD:
  case JmpNInstanceOfD:
  case JmpIsSet:
  case JmpIsType:
  case JmpIsNSet:
  case JmpIsNType:
    return nullptr;

  case IsSet:
  case IsNSet:
  case LdObjClass:
  case LdCachedClass:
  case DecRefLoc:
  case DecRefStack:
  case DecRef:
  case DecRefNZ:
  case GuardType:
  case GuardLoc:
  case GuardStk:
  case LdThis:
  case LdLoc:
  case LdMem:
  case LdRef:
  case LdStack:
  case LdPropAddr:
  case LdClsCns:
  case LdCls:
  case LdObjMethod:
  case RetVal:
  case FreeActRec:
  case LdClsMethodCache:
  case LdClsMethod:
  case Call:
  case SpillStack:
  case ExitTrace:
  case ExitSlow:
  case ExitGuardFailure:
  case StMem:
  case StMemNT:
  case StLoc:
  case IncRef:
  case DefFP:
  case DefSP:
  case LdFunc:
  case LdFixedFunc:
  case Box:
  case DefLabel:
  case Marker:
    return nullptr;

  default:
    unimplementedSimplify(inst->getOpcode());
    return nullptr;
  }
}

SSATmp* Simplifier::simplifyMov(SSATmp* src) {
  return src;
}

SSATmp* Simplifier::simplifyNot(SSATmp* src) {
  // const XORs are handled in simplifyXor()
  assert(!src->isConst());
  assert(src->getType() == Type::Bool);
  IRInstruction* inst = src->getInstruction()->getSrc(0)->getInstruction();
  Opcode op = inst->getOpcode();
  // TODO: Add more algebraic simplification rules for NOT
  switch (op) {
    case OpXor: {
      // !!X --> bool(X)
      if (isNotInst(inst->getSrc(0))) {
        return m_tb->genConvToBool(inst->getSrc(0));
      }
      break;
    }
    // !(X cmp Y) --> X opposite_cmp Y
    case OpLt:
    case OpLte:
    case OpGt:
    case OpGte:
    case OpEq:
    case OpNeq:
    case OpSame:
    case OpNSame:
      return m_tb->genCmp(negateQueryOp(op), inst->getSrc(0), inst->getSrc(1));
    // TODO !(X | non_zero) --> 0
    default: (void)op;
  }
  return NULL;
}

#define SIMPLIFY_CONST(OP) do {                                               \
  /* don't canonicalize to the right, OP might not be commutative */          \
  if (src1->isConst() && src2->isConst()) {                                   \
    if (Type::isNull(src1->getType())) {                                      \
      /* Null op Null */                                                      \
      if (Type::isNull(src2->getType())) {                                    \
        return genDefInt(0 OP 0);                                             \
      }                                                                       \
      /* Null op ConstInt */                                                  \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(0 OP src2->getConstValAsInt());                      \
      }                                                                       \
      /* Null op ConstBool */                                                 \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(0 OP src2->getConstValAsBool());                     \
      }                                                                       \
      /* Null op StaticStr */                                                 \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getConstValAsStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(0 OP str->toInt64());                              \
        }                                                                     \
        return genDefInt(0 OP 0);                                             \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::Int) {                                       \
      /* ConstInt op Null */                                                  \
      if (Type::isNull(src2->getType())) {                                    \
        return genDefInt(src1->getConstValAsInt() OP 0);                      \
      }                                                                       \
      /* ConstInt op ConstInt */                                              \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(src1->getConstValAsInt() OP                          \
                         src2->getConstValAsInt());                           \
      }                                                                       \
      /* ConstInt op ConstBool */                                             \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(src1->getConstValAsInt() OP                          \
                         int(src2->getConstValAsBool()));                     \
      }                                                                       \
      /* ConstInt op StaticStr */                                             \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getConstValAsStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(src1->getConstValAsInt() OP str->toInt64());       \
        }                                                                     \
        return genDefInt(src1->getConstValAsInt() OP 0);                      \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::Bool) {                                      \
      /* ConstBool op Null */                                                 \
      if (Type::isNull(src2->getType())) {                                    \
        return genDefInt(src1->getConstValAsBool() OP 0);                     \
      }                                                                       \
      /* ConstBool op ConstInt */                                             \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(int(src1->getConstValAsBool()) OP                    \
                         src2->getConstValAsInt());                           \
      }                                                                       \
      /* ConstBool op ConstBool */                                            \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(src1->getConstValAsBool() OP                         \
                         src2->getConstValAsBool());                          \
      }                                                                       \
      /* ConstBool op StaticStr */                                            \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getConstValAsStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(int(src1->getConstValAsBool()) OP str->toInt64()); \
        }                                                                     \
        return genDefInt(int(src1->getConstValAsBool()) OP 0);                \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::StaticStr) {                                 \
      const StringData* str = src1->getConstValAsStr();                       \
      int64 strInt = 0;                                                       \
      if (str->isInteger()) {                                                 \
        strInt = str->toInt64();                                              \
      }                                                                       \
      /* StaticStr op Null */                                                 \
      if (Type::isNull(src2->getType())) {                                    \
        return genDefInt(strInt OP 0);                                        \
      }                                                                       \
      /* StaticStr op ConstInt */                                             \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(strInt OP src2->getConstValAsInt());                 \
      }                                                                       \
      /* StaticStr op ConstBool */                                            \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(strInt OP int(src2->getConstValAsBool()));           \
      }                                                                       \
      /* StaticStr op StaticStr */                                            \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str2 = src2->getConstValAsStr();                    \
        if (str2->isInteger()) {                                              \
          return genDefInt(strInt OP str2->toInt64());                        \
        }                                                                     \
        return genDefInt(strInt OP 0);                                        \
      }                                                                       \
    }                                                                         \
  }                                                                           \
} while (0)

#define SIMPLIFY_COMMUTATIVE(OP, NAME) do {                               \
  SIMPLIFY_CONST(OP);                                                     \
  if (src1->isConst() && !src2->isConst()) {                              \
    return m_tb->gen##NAME(src2, src1);                                   \
  }                                                                       \
  IRInstruction* inst1 = src1->getInstruction();                          \
  IRInstruction* inst2 = src2->getInstruction();                          \
  if (inst1->getOpcode() == Op##NAME && inst1->getSrc(1)->isConst()) {    \
    /* (X + C1) + C2 --> X + C3 */                                        \
    if (src2->isConst()) {                                                \
      int64 right = inst1->getSrc(1)->getConstValAsInt();                 \
      right OP##= src2->getConstValAsInt();                               \
      return m_tb->gen##NAME(inst1->getSrc(0), genDefInt(right));         \
    }                                                                     \
    /* (X + C1) + (Y + C2) --> X + Y + C3 */                              \
    if (inst2->getOpcode() == Op##NAME && inst2->getSrc(1)->isConst()) {  \
      int64 right = inst1->getSrc(1)->getConstValAsInt();                 \
      right OP##= inst2->getSrc(1)->getConstValAsInt();                   \
      SSATmp* left = m_tb->gen##NAME(inst1->getSrc(0), inst2->getSrc(0)); \
      return m_tb->gen##NAME(left, genDefInt(right));                     \
    }                                                                     \
  }                                                                       \
} while (0)

#define SIMPLIFY_DISTRIBUTIVE(OUTOP, INOP, OUTNAME, INNAME) do {              \
  /* assumes that OUTOP is commutative, don't use with subtract! */           \
  SIMPLIFY_COMMUTATIVE(OUTOP, OUTNAME);                                       \
  IRInstruction* inst1 = src1->getInstruction();                              \
  IRInstruction* inst2 = src2->getInstruction();                              \
  Opcode op1 = inst1->getOpcode();                                            \
  Opcode op2 = inst2->getOpcode();                                            \
  /* all combinations of X * Y + X * Z --> X * (Y + Z) */                     \
  if (op1 == Op##INNAME && op2 == Op##INNAME) {                               \
    if (inst1->getSrc(0) == inst2->getSrc(0)) {                               \
      SSATmp* fold = m_tb->gen##OUTNAME(inst1->getSrc(1), inst2->getSrc(1));  \
      return m_tb->gen##INNAME(inst1->getSrc(0), fold);                       \
    }                                                                         \
    if (inst1->getSrc(0) == inst2->getSrc(1)) {                               \
      SSATmp* fold = m_tb->gen##OUTNAME(inst1->getSrc(1), inst2->getSrc(0));  \
      return m_tb->gen##INNAME(inst1->getSrc(0), fold);                       \
    }                                                                         \
    if (inst1->getSrc(1) == inst2->getSrc(0)) {                               \
      SSATmp* fold = m_tb->gen##OUTNAME(inst1->getSrc(0), inst2->getSrc(1));  \
      return m_tb->gen##INNAME(inst1->getSrc(1), fold);                       \
    }                                                                         \
    if (inst1->getSrc(1) == inst2->getSrc(1)) {                               \
      SSATmp* fold = m_tb->gen##OUTNAME(inst1->getSrc(0), inst2->getSrc(0));  \
      return m_tb->gen##INNAME(inst1->getSrc(1), fold);                       \
    }                                                                         \
  }                                                                           \
} while (0)

SSATmp* Simplifier::simplifyAdd(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(+, *, Add, Mul);
  // X + 0 --> X
  if (src2->isConst() && src2->getType() == Type::Int) {
    if (src2->getConstValAsInt() == 0) {
      return src1;
    }
  }
  // X + (0 - Y) --> X - Y
  IRInstruction* inst2 = src2->getInstruction();
  Opcode op2 = inst2->getOpcode();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->getType() == Type::Int) {
      if (src->getConstValAsInt() == 0) {
        return m_tb->genSub(src1, inst2->getSrc(1));
      }
    }
  }
  return NULL;
}
SSATmp* Simplifier::simplifySub(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_CONST(-);
  // X - X --> 0
  if (src1 == src2) {
    return genDefInt(0);
  }
  // X - 0 --> X
  if (src2->isConst() && src2->getType() == Type::Int) {
    if (src2->getConstValAsInt() == 0) {
      return src1;
    }
  }
  // X - (0 - Y) --> X + Y
  IRInstruction* inst2 = src2->getInstruction();
  Opcode op2 = inst2->getOpcode();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->getType() == Type::Int) {
      if (src->getConstValAsInt() == 0) {
        return m_tb->genAdd(src1, inst2->getSrc(1));
      }
    }
  }
  // TODO patterns in the form of:
  // (X - C1) + (X - C2)
  // (X - C1) + C2
  // (X - C1) + (X + C2)
  return NULL;
}
SSATmp* Simplifier::simplifyMul(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(*, Mul);
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X * (-1) --> -X
    if (src2->getConstValAsInt() == -1) {
      return m_tb->genSub(genDefInt(0), src1);
    }
    // X * 0 --> 0
    if (src2->getConstValAsInt() == 0) {
      return genDefInt(0);
    }
    // X * 1 --> X
    if (src2->getConstValAsInt() == 1) {
      return src1;
    }
    // X * 2 --> X + X
    if (src2->getConstValAsInt() == 2) {
      return m_tb->genAdd(src1, src1);
    }
    // TODO once IR has shifts
    // X * 2^C --> X << C
    // X * (2^C + 1) --> ((X << C) + X)
    // X * (2^C - 1) --> ((X << C) - X)
  }
  return NULL;
}
SSATmp* Simplifier::simplifyAnd(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(&, |, And, Or);
  // X & X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X & 0 --> 0
    if (src2->getConstValAsInt() == 0) {
      return genDefInt(0);
    }
    // X & (~0) --> X
    if (src2->getConstValAsInt() == ~0L) {
      return src1;
    }
  }
  return NULL;
}
SSATmp* Simplifier::simplifyOr(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(|, &, Or, And);
  // X | X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X | 0 --> X
    if (src2->getConstValAsInt() == 0) {
      return src1;
    }
    // X | (~0) --> ~0
    if (src2->getConstValAsInt() == ~0L) {
      return genDefInt(~0L);
    }
  }
  return NULL;
}
SSATmp* Simplifier::simplifyXor(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(^, Xor);
  // X ^ X --> 0
  if (src1 == src2)
    return genDefInt(0);
  // X ^ 0 --> X
  if (src2->isConst() && src2->getType() == Type::Int) {
    if (src2->getConstValAsInt() == 0) {
      return src1;
    }
  }
  if (isNotInst(src1, src2)) {
    return simplifyNot(src1);
  }
  return NULL;
}

static SSATmp* chaseIncRefs(SSATmp* tmp) {
  while (tmp->getInstruction()->getOpcode() == IncRef) {
    tmp = tmp->getInstruction()->getSrc(0);
  }
  return tmp;
}

template<class T, class U>
static typename std::common_type<T,U>::type cmpOp(Opcode opName, T a, U b) {
  switch (opName) {
  case OpGt:   return a > b;
  case OpGte:  return a >= b;
  case OpLt:   return a < b;
  case OpLte:  return a <= b;
  case OpSame:
  case OpEq:   return a == b;
  case OpNSame:
  case OpNeq:  return a != b;
  default:
    not_reached();
  }
}

SSATmp* Simplifier::simplifyCmp(Opcode opName, SSATmp* src1, SSATmp* src2) {
  // ---------------------------------------------------------------------
  // Perform some execution optimizations immediately
  // ---------------------------------------------------------------------

  // Identity optimization
  if ((src1 == src2 || chaseIncRefs(src1) == chaseIncRefs(src2)) &&
      src1->getType() != Type::Dbl) {
    // (val1 == val1) does not simplify to true when val1 is a NaN
    return genDefBool(cmpOp(opName, 0, 0));
  }

  // need both types to be unboxed and known to simplify
  if (!Type::isUnboxed(src1->getType()) || src1->getType() == Type::Cell ||
      !Type::isUnboxed(src2->getType()) || src2->getType() == Type::Cell) {
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // OpSame and OpNSame have some special rules
  // ---------------------------------------------------------------------

  if (opName == OpSame || opName == OpNSame) {
    // OpSame and OpNSame do not perform type juggling
    if (src1->getType() != src2->getType()) {
      if (!(Type::isString(src1->getType()) &&
            Type::isString(src2->getType()))) {
        return genDefBool(opName == OpNSame);
      }
    }

    // src1 and src2 are same type, treating Str and StaticStr as the same

    // OpSame and OpNSame have special rules for string and object
    // Other types may simplify to OpEq and OpNeq, respectively
    if (Type::isString(src1->getType()) && Type::isString(src2->getType())) {
      if (src1->isConst() && src2->isConst()) {
        auto str1 = src1->getConstValAsStr();
        auto str2 = src2->getConstValAsStr();
        bool same = str1->same(str2);
        return genDefBool(cmpOp(opName, same, 1));
      } else {
        return nullptr;
      }
    }
    if (src1->getType() == Type::Obj && src2->getType() == Type::Obj) {
      return nullptr;
    }
    // Type is neither a string nor an object - simplify to OpEq/OpNeq
    if (opName == OpSame) {
      return m_tb->genCmp(OpEq, src1, src2);
    }
    return m_tb->genCmp(OpNeq, src1, src2);
  }

  // ---------------------------------------------------------------------
  // We may now perform constant-constant optimizations
  // ---------------------------------------------------------------------

  // Null cmp Null
  if (Type::isNull(src1->getType()) && Type::isNull(src2->getType())) {
    return genDefBool(cmpOp(opName, 0, 0));
  }
  // const cmp const
  // TODO this list is incomplete - feel free to add more
  // TODO: can simplify const arrays when sizes are different or both 0
  if (src1->isConst() && src2->isConst()) {
    // StaticStr cmp StaticStr
    if (src1->getType() == Type::StaticStr &&
        src2->getType() == Type::StaticStr) {
      int cmp = src1->getConstValAsStr()->compare(src2->getConstValAsStr());
      return genDefBool(cmpOp(opName, cmp, 0));
    }
    // ConstInt cmp ConstInt
    if (src1->getType() == Type::Int && src2->getType() == Type::Int) {
      return genDefBool(
        cmpOp(opName, src1->getConstValAsInt(), src2->getConstValAsInt()));
    }
    // ConstBool cmp ConstBool
    if (src1->getType() == Type::Bool && src2->getType() == Type::Bool) {
      return genDefBool(
        cmpOp(opName, src1->getConstValAsBool(), src2->getConstValAsBool()));
    }
  }

  // ---------------------------------------------------------------------
  // Constant bool comparisons can be strength-reduced
  // NOTE: Comparisons with bools get juggled to bool.
  // ---------------------------------------------------------------------

  // Perform constant-bool optimizations
  if (src2->getType() == Type::Bool && src2->isConst()) {
    bool b = src2->getConstValAsBool();

    // The result of the comparison might be independent of the truth
    // value of the LHS. If so, then simplify.
    // E.g. `some-int > true`. some-int may juggle to false or true
    //  (0 or 1), but `0 > true` and `1 > true` are both false, so we can
    //  simplify to false immediately.
    if (cmpOp(opName, false, b) == cmpOp(opName, true, b)) {
      return genDefBool(cmpOp(opName, false, b));
    }

    // There are only two distinct booleans - false and true (0 and 1).
    // From above, we know that (0 OP b) != (1 OP b).
    // Hence exactly one of (0 OP b) and (1 OP b) is true.
    // Hence there is exactly one boolean value of src1 that results in the
    // overall expression being true (after type-juggling).
    // Hence we may check for equality with that boolean.
    // E.g. `some-int > false` is equivalent to `some-int == true`
    if (opName != OpEq) {
      if (cmpOp(opName, false, b)) {
        return m_tb->genCmp(OpEq, src1, genDefBool(false));
      } else {
        return m_tb->genCmp(OpEq, src1, genDefBool(true));
      }
    }
  }

  // ---------------------------------------------------------------------
  // For same-type cmps, canonicalize any constants to the right
  // Then stop - there are no more simplifications left
  // ---------------------------------------------------------------------

  if (src1->getType() == src2->getType() ||
      (Type::isString(src1->getType()) && Type::isString(src2->getType()))) {
    if (src1->isConst() && !src2->isConst()) {
      return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // Perform type juggling and type canonicalization for different types
  // see http://www.php.net/manual/en/language.operators.comparison.php
  // ---------------------------------------------------------------------

  // nulls get canonicalized to the right
  if (Type::isNull(src1->getType())) {
    return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
  }

  // case 1: null cmp string. Convert null to ""
  if (Type::isString(src1->getType()) && Type::isNull(src2->getType())) {
    return m_tb->genCmp(opName, src1,
                        m_tb->genDefConst(StringData::GetStaticString("")));
  }

  // case 2a: null cmp anything. Convert null to false
  if (Type::isNull(src2->getType())) {
    return m_tb->genCmp(opName, src1, genDefBool(false));
  }

  // bools get canonicalized to the right
  if (src1->getType() == Type::Bool) {
    return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
  }

  // case 2b: bool cmp anything. Convert anything to bool
  if (src2->getType() == Type::Bool) {
    if (src1->isConst()) {
      if (src1->getType() == Type::Int) {
        return m_tb->genCmp(opName, genDefBool(src1->getConstValAsInt()), src2);
      } else if (Type::isString(src1->getType())) {
        auto str = src1->getConstValAsStr();
        return m_tb->genCmp(opName, genDefBool(str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->getType() == Type::Int && src2->isConst()) {
      // Based on the const bool optimization (above) opName should be OpEq
      always_assert(opName == OpEq);

      if (src2->getConstValAsBool()) {
        return m_tb->genCmp(OpNeq, src1, m_tb->genDefConst<int64>(0));
      } else {
        return m_tb->genCmp(OpEq, src1, m_tb->genDefConst<int64>(0));
      }
    }

    // Nothing fancy to do - perform juggling as normal.
    return m_tb->genCmp(opName, m_tb->genConvToBool(src1), src2);
  }

  // From here on, we must be careful of how Type::Obj gets dealt with,
  // since Type::Obj can refer to an object or to a resource.

  // case 3: object cmp object. No juggling to do
  // same-type simplification is performed above

  // strings get canonicalized to the left
  if (Type::isString(src2->getType())) {
    return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
  }

  // ints get canonicalized to the right
  if (src1->getType() == Type::Int) {
    return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
  }

  // case 4: number/string/resource cmp. Convert to number (int OR double)
  // NOTE: The following if-test only checks for some of the SRON-SRON
  //  cases (specifically, string-int). Other cases (like string-string)
  //  are dealt with earlier, while other cases (like number-resource)
  //  are not caught at all (and end up exiting this macro at the bottom).
  if (Type::isString(src1->getType()) && src1->isConst() &&
      src2->getType() == Type::Int) {
    auto str = src1->getConstValAsStr();
    int64 si; double sd;
    auto st = str->isNumericWithVal(si, sd, true /* allow errors */);
    if (st == KindOfDouble) {
      return m_tb->genCmp(opName, m_tb->genDefConst<double>(sd), src2);
    }
    if (st == KindOfNull) {
      si = 0;
    }
    return m_tb->genCmp(opName, m_tb->genDefConst<int64>(si), src2);
  }

  // case 5: array cmp array. No juggling to do
  // same-type simplification is performed above

  // case 6: array cmp anything. Array is greater
  if (src1->getType() == Type::Arr) {
    return genDefBool(cmpOp(opName, 1, 0));
  }
  if (src2->getType() == Type::Arr) {
    return genDefBool(cmpOp(opName, 0, 1));
  }

  // case 7: object cmp anything. Object is greater
  // ---------------------------------------------------------------------
  // Unfortunately, we are unsure of whether Type::Obj is an object or a
  // resource, so this code cannot be applied.
  // ---------------------------------------------------------------------
  return nullptr;
}

#define DEF_SIMPLIFY_CMP(op)                                      \
  SSATmp* Simplifier::simplify##op(SSATmp* src1, SSATmp* src2) {  \
    return simplifyCmp(Op##op, src1, src2);                       \
  }

DEF_SIMPLIFY_CMP(Gt)
DEF_SIMPLIFY_CMP(Gte)
DEF_SIMPLIFY_CMP(Lt)
DEF_SIMPLIFY_CMP(Lte)
DEF_SIMPLIFY_CMP(Eq)
DEF_SIMPLIFY_CMP(Neq)
DEF_SIMPLIFY_CMP(Same)
DEF_SIMPLIFY_CMP(NSame)

#undef DEF_SIMPLIFY_CMP

SSATmp* Simplifier::simplifyIsType(IRInstruction* inst) {
  auto type = inst->getTypeParam();
  auto src  = inst->getSrc(0);

  assert(Type::isUnboxed(type));
  assert(type != Type::Cell);
  if (type != Type::Obj) {
    if (src->getType() == type) {
      return genDefBool(true);
    }
    if (src->getType() != Type::Cell) {
      return genDefBool(false);
    }
  }
  if (src->getType() != Type::Obj) {
    // Note: for IsObject*, we need to emit a call to ObjectData::isResource
    // (or equivalent), so we can't fold away the case where we know we are
    // checking an object.
    return genDefBool(false);
  }
  return NULL;
}

SSATmp* Simplifier::simplifyConcat(SSATmp* src1, SSATmp* src2) {
  if (src1->isConst() && src1->getType() == Type::StaticStr &&
      src2->isConst() && src2->getType() == Type::StaticStr) {
    StringData* str1 = const_cast<StringData *>(src1->getConstValAsStr());
    StringData* str2 = const_cast<StringData *>(src2->getConstValAsStr());
    StringData* merge = StringData::GetStaticString(concat_ss(str1, str2));
    return m_tb->genDefConst(merge);
  }
  return NULL;
}

SSATmp* Simplifier::simplifyConv(IRInstruction* inst) {
  SSATmp* src      = inst->getSrc(0);
  Type::Tag type   = src->getType();
  Type::Tag toType = inst->getTypeParam();
  if (toType == type) {
    return src;
  }
  if (toType == Type::Str) {
    if (Type::isString(type)) {
      return src;
    }
    // arrays always get converted to the string "Array"
    if (type == Type::Arr) {
      return m_tb->genDefConst(StringData::GetStaticString("Array"));
    }
    if (type == Type::Null) {
      return m_tb->genDefConst(StringData::GetStaticString(""));
    }
    if (src->isConst()) {
      if (type == Type::Bool) {
        if (src->getConstValAsBool()) {
          return m_tb->genDefConst(StringData::GetStaticString("1"));
        }
        return m_tb->genDefConst(StringData::GetStaticString(""));
      }
      if (type == Type::Int) {
        std::stringstream ss;
        ss << src->getConstValAsInt();
        return m_tb->genDefConst(StringData::GetStaticString(ss.str()));
      }
      if (type == Type::Dbl) {
        // TODO constant dbl to string
      }
    }
  }
  if (toType == Type::Bool) {
    if (Type::isNull(type)) {
      return genDefBool(false);
    }
    if (type == Type::Obj) {
      return genDefBool(true);
    }
    if (src->isConst()) {
      if (type == Type::Int) {
        return genDefBool(bool(src->getConstValAsInt()));
      }
      if (type == Type::StaticStr) {
        // only the strings "", and "0" convert to false, all other strings
        // are converted to true
        const StringData* str = src->getConstValAsStr();
        return genDefBool(!str->empty() && !str->isZero());
      }
      if (type == Type::Arr) {
        if (((ConstInstruction*)src->getInstruction())->isEmptyArray()) {
          return genDefBool(false);
        }
        if (src->getConstValAsArr()->empty()) {
          return genDefBool(false);
        }
        return genDefBool(true);
      }
    }
  }
  if (toType == Type::Int) {
    if (Type::isNull(type)) {
      return genDefInt(0);
    }
    if (src->isConst()) {
      if (src->getType() == Type::Bool) {
        return genDefInt(int(src->getConstValAsBool()));
      }
      if (type == Type::StaticStr) {
        const StringData *str = src->getConstValAsStr();
        if (str->isInteger()) {
          return genDefInt(str->toInt64());
        }
        return genDefInt(0);
      }
      if (type == Type::Arr) {
        if (((ConstInstruction*)src->getInstruction())->isEmptyArray()) {
          return genDefInt(0);
        }
        if (src->getConstValAsArr()->empty()) {
          return genDefInt(0);
        }
        return genDefInt(1);
      }
    }
  }
  return NULL;
}

SSATmp* Simplifier::simplifyInstanceOfD(SSATmp* src1,
                                        SSATmp* src2,
                                        bool negate) {
  if (src1->getType() != Type::Obj) {
    return genDefBool(false);
  }
  return NULL;
}

SSATmp* Simplifier::simplifyLdClsPropAddr(SSATmp* cls,
                                          SSATmp* clsName,
                                          SSATmp* propName) {
  if (clsName->getType() == Type::Null) {
    IRInstruction* clsInst = cls->getInstruction();
    if (clsInst->getOpcode() == LdCls) {
      SSATmp* clsName = clsInst->getSrc(0);
      assert(clsName->isConst() && clsName->getType() == Type::StaticStr);
      return genLdClsPropAddr(cls, clsName, propName);
    }
  }
  return NULL;
}

SSATmp* Simplifier::simplifyUnbox(IRInstruction* inst) {
  auto* src           = inst->getSrc(0);
  auto* typeFailLabel = inst->getLabel();
  auto type           = outputType(inst);

  assert(typeFailLabel);
  assert(Type::isUnboxed(type));

  Type::Tag srcType = src->getType();
  if (Type::isUnboxed(srcType)) {
    // TODO: generate a guardType if this assertion fails
    assert(!Type::isMoreRefined(type, srcType));
    return src;
  }
  if (Type::isBoxed(srcType)) {
    srcType = Type::getInnerType(srcType);
    if (Type::isMoreRefined(srcType, type)) {
      type = srcType;
    }
    return m_tb->genLdRef(src, type, typeFailLabel->getParent());
  }
  return NULL;
}

SSATmp* Simplifier::simplifyUnboxPtr(IRInstruction* inst) {
  if (inst->getSrc(0)->getType() == Type::PtrToCell) {
    return inst->getSrc(0);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyCheckInit(IRInstruction* inst) {
  if (inst->getLabel() != NULL) {
    Type::Tag type = inst->getSrc(0)->getType();
    if (Type::isInit(type)) {
      // Unnecessary CheckInit! Mark it unnecessary by deleting its label
      inst->setLabel(NULL);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::genDefInt(int64 val) {
  return m_tb->genDefConst<int64>(val);
}

SSATmp* Simplifier::genDefBool(bool val) {
  return m_tb->genDefConst<bool>(val);
}

SSATmp* Simplifier::genLdClsPropAddr(SSATmp* cls,
                                     SSATmp* clsName,
                                     SSATmp* prop) {
  return m_tb->genLdClsPropAddr(cls, clsName, prop);
}

}}} // namespace HPHP::VM::JIT
