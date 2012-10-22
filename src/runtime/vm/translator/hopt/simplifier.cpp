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

#include "simplifier.h"
#include "tracebuilder.h"
#include <runtime/vm/runtime.h>
#include <sstream>

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

SSATmp* IRInstruction::simplify(Simplifier* simplifier) {
  if (isControlFlowInstruction()) {
    return simplifier->simplifyInst((Opcode)m_op, (Type::Tag)m_type,
                                    m_srcs[0], m_srcs[1], m_label);
  } else {
    return simplifier->simplifyInst((Opcode)m_op, (Type::Tag)m_type,
                                    m_srcs[0], m_srcs[1]);
  }
}

SSATmp* ExtendedInstruction::simplify(Simplifier* simplifier) {
  return simplifier->simplifyInst((Opcode)m_op, (Type::Tag)m_type,
                                  m_srcs[0], m_srcs[1],
                                  m_numSrcs - NUM_FIXED_SRCS,
                                  m_extendedSrcs);
}

SSATmp* TypeInstruction::simplify(Simplifier* simplifier) {
  return simplifier->simplifyInst((Opcode)m_op, (Type::Tag)m_type,
                                  m_srcs[0], m_srcType);
}

SSATmp* LabelInstruction::simplify(Simplifier* simplifier) {
  return NULL;
}

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

static
void unimplementedSimplify(Opcode opc) {
  // Do not ASSERT(false), it is fine to not simplify as the default
  TRACE(1, "HHIR Simplifier: unimplemented support for opcode %s\n",
        OpcodeStrings[opc]);
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

SSATmp* Simplifier::simplifyInst(Opcode opc,
                                 Type::Tag type,
                                 SSATmp* src1,
                                 SSATmp* src2) {
  switch (opc) {
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
    case Conv:        return simplifyConv(type,  src1);
    case InstanceOfD: return simplifyInstanceOfD(src1, src2, false);
    case NInstanceOfD:return simplifyInstanceOfD(src1, src2, true);
    case IsSet:       return simplifyIsSet(src1, false);
    case IsNSet:      return simplifyIsSet(src1, true);
    case Unbox:       return simplifyUnbox(type, src1);
    case Mov:         return simplifyMov(src1);
    case LdObjClass:  return simplifyLdObjClass(src1);
    case LdCachedClass: return simplifyLdCachedClass(src1);
    case LdCls:
    case LdObjMethod:
      return NULL;
    case RetVal:
    case FreeActRec:

  // stores
    case StMem:
    case StMemNT:
    case StLoc:
    case IncRef:
    case DefFP:
    case DefSP:
    case LdFunc:
    case LdFixedFunc:
    case LdVarEnv:
    case Box:
      return NULL;
    default:
      unimplementedSimplify(opc);
      return NULL;
  }
}

SSATmp* Simplifier::simplifyInst(Opcode opc,
                                 Type::Tag type,
                                 SSATmp* src1,
                                 SSATmp* src2,
                                 uint32 numExtendedSrcs,
                                 SSATmp** extendedSrcs) {
  switch(opc) {
    case LdClsPropAddr:
      return simplifyLdClsPropAddr(src1, src2, extendedSrcs[0]);
    case AllocActRec:
      if (numExtendedSrcs == 3) {
        return simplifyAllocActRec(src1,
                                   src2,
                                   extendedSrcs[0],
                                   extendedSrcs[1],
                                   extendedSrcs[2]);
      } else if (numExtendedSrcs == 2) {
        return simplifyAllocActRec(src1,
                                   src2,
                                   extendedSrcs[0],
                                   NULL,
                                   extendedSrcs[1]);
      }
      return NULL;
    case LdClsMethod: // simplify: fall-through and return NULL
    case Call:
    case SpillStack:
    case SpillStackAllocAR:
    case ExitTrace:
    case ExitSlow:
    case ExitGuardFailure:
      return NULL;
    default:
      unimplementedSimplify(opc);
      return NULL;
  }
}

SSATmp* Simplifier::simplifyInst(Opcode opc,
                                 Type::Tag type,
                                 SSATmp* src,
                                 Type::Tag srcType) {
  switch(opc) {
    case IsType: return simplifyIsType(srcType, src);
    default:
      unimplementedSimplify(opc);
      return NULL;
  }
}

SSATmp* Simplifier::simplifyInst(Opcode opc,
                                 Type::Tag type,
                                 SSATmp* src1,
                                 SSATmp* src2,
                                 LabelInstruction* label) {
  switch (opc) {
    case DecRefLoc:
    case DecRefStack:
    case DecRef:
    case DecRefNZ:
    case GuardType:
    case LdThis:
    case LdLoc:
    case LdMemNR:
    case LdRefNR:
    case LdStack:
    case LdPropAddr:
    case LdClsCns:
      return NULL;
    case Unbox:
         return simplifyUnbox(type, src1, label);
    case JmpGt:
    case JmpGte:
    case JmpLt:
    case JmpLte:
    case JmpEq:
    case JmpNeq:
    case JmpSame:
    case JmpNSame:
      return simplifyJcc(opc, type, src1, src2, label);
    case JmpZero:  return simplifyJmpZ(src1, label);
    case JmpNZero: return simplifyJmpNz(src1, label);
    case Jmp_:
      return NULL;
    case JmpInstanceOfD:
    case JmpNInstanceOfD:
    case JmpIsSet:
    case JmpIsType:
    case JmpIsNSet:
    case JmpIsNType:
      return NULL;
    default:
      unimplementedSimplify(opc);
      return NULL;
  }
}

SSATmp* Simplifier::simplifyInst(IRInstruction* inst) {
  return inst->simplify(this);
}

SSATmp* Simplifier::simplifyMov(SSATmp* src) {
  return src;
}
SSATmp* Simplifier::simplifyNot(SSATmp* src) {
  // const XORs are handled in simplifyXor()
  ASSERT(!src->isConst());
  ASSERT(src->getType() == Type::Bool);
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

SSATmp* chaseIncRefs(SSATmp* tmp) {
  while (tmp->getInstruction()->getOpcode() == IncRef) {
    tmp = tmp->getInstruction()->getSrc(0);
  }
  return tmp;
}

#define SIMPLIFY_CMP(OP, NAME, EXP) do {                                      \
  if (src1 == src2 || chaseIncRefs(src1) == chaseIncRefs(src2)) {             \
    /* want to use the same operator to compare two equal values */           \
    return genDefBool(0 OP 0);                                                \
  }                                                                           \
  /* need both types to be unboxed and known to simplify */                   \
  if (!Type::isUnboxed(src1->getType()) || src1->getType() == Type::Cell ||   \
      !Type::isUnboxed(src2->getType()) || src2->getType() == Type::Cell) {   \
    break;                                                                    \
  }                                                                           \
  /* Null cmp Null */                                                         \
  if (Type::isNull(src1->getType()) && Type::isNull(src2->getType())) {       \
    return genDefBool(0 OP 0);                                                \
  }                                                                           \
  if (src1->isConst() && src2->isConst()) {                                   \
    /* StaticStr cmp StaticStr */                                             \
    if (src1->getType() == Type::StaticStr &&                                 \
        src2->getType() == Type::StaticStr) {                                 \
      int cmp = src1->getConstValAsStr()->compare(src2->getConstValAsStr());  \
      return genDefBool(EXP(cmp));                                            \
    }                                                                         \
    /* ConstInt cmp ConstInt */                                               \
    if (src1->getType() == Type::Int && src2->getType() == Type::Int) {       \
      return genDefBool(src1->getConstValAsInt() OP src2->getConstValAsInt());\
    }                                                                         \
    /* ConstBool cmp ConstBool */                                             \
    if (src1->getType() == Type::Bool && src2->getType() == Type::Bool) {     \
      return genDefBool(src1->getConstValAsBool() OP                          \
                        src2->getConstValAsBool());                           \
    }                                                                         \
  }                                                                           \
  /* stop if we're OpSame/OpNSame, all the following folds are aimed at
   * operands of different types */                                           \
  if (NAME == OpSame || NAME == OpNSame) {                                    \
    if (src1->getType() != src2->getType()) {                                 \
      if (!(Type::isString(src1->getType()) &&                                \
            Type::isString(src2->getType()))) {                               \
        return genDefBool(NAME == OpNSame);                                   \
      }                                                                       \
    }                                                                         \
    /* strings and objects have special === rules */                          \
    /* other types may now simplify === to ==, since the types are equal */   \
    if (Type::isString(src1->getType()) ||                                    \
        src1->getType() == Type::Obj) {                                       \
      break;                                                                  \
    }                                                                         \
    if (NAME == OpSame) {                                                     \
      return m_tb->genCmp(OpEq, src1, src2);                                  \
    }                                                                         \
    return m_tb->genCmp(OpNeq, src1, src2);                                   \
  }                                                                           \
  if (src1->getType() == src2->getType() ||                                   \
      (Type::isString(src1->getType()) && Type::isString(src2->getType()))) { \
    /* Types are the same, no more simplifications left */                    \
    break;                                                                    \
  }                                                                           \
  /* nulls get canonicalized to the right */                                  \
  if (Type::isNull(src1->getType())) {                                        \
    return m_tb->genCmp(commuteQueryOp(NAME), src2, src1);                    \
  }                                                                           \
  if (Type::isNull(src2->getType())) {                                        \
    if (Type::isString(src1->getType())) {                                    \
      /* convert null to "", numerical or lexical comparison */               \
      return m_tb->genCmp(NAME, src1,                                         \
                          m_tb->genDefConst(StringData::GetStaticString("")));\
    }                                                                         \
    if (src1->getType() == Type::Int) {                                       \
      if (NAME == OpEq || NAME == OpNeq) {                                    \
        return m_tb->genCmp(NAME, src1, genDefInt(0));                        \
      }                                                                       \
      /* TODO: for all other comparison of int to null, we can optimize to use
         an unsigned integer comparison */                                    \
    }                                                                         \
    return m_tb->genCmp(NAME, src1, genDefBool(false));                       \
  }                                                                           \
  /* bools get canonicalized to the right */                                  \
  if (src1->getType() == Type::Bool) {                                        \
    return m_tb->genCmp(commuteQueryOp(NAME), src2, src1);                    \
  }                                                                           \
  if (src2->getType() == Type::Bool) {                                        \
    /* TODO: optimze comparison of ints to const bools */                     \
    return m_tb->genCmp(NAME, m_tb->genConvToBool(src1), src2);               \
  }                                                                           \
  /* strings get canonicalized to the left */                                 \
  if (Type::isString(src2->getType())) {                                      \
    return m_tb->genCmp(commuteQueryOp(NAME), src2, src1);                    \
  }                                                                           \
  if (src1->isConst() && src2->isConst()) {                                   \
    /* StaticStr cmp ConstBool */                                             \
    if (src1->getType() == Type::StaticStr && src2->getType() == Type::Bool) {\
      const StringData* str = src1->getConstValAsStr();                       \
      if (str->isInteger()) {                                                 \
        return genDefBool(bool(str->toInt64()) OP src2->getConstValAsBool()); \
      } else if (str->isNumeric()) {                                          \
        return genDefBool(bool(str->toDouble()) OP src2->getConstValAsBool());\
      }                                                                       \
      return genDefBool(!str->empty() OP src2->getConstValAsBool());          \
    }                                                                         \
    /* StaticStr cmp ConstInt */                                              \
    if (src1->getType() == Type::StaticStr && src2->getType() == Type::Int) { \
      const StringData* str = src1->getConstValAsStr();                       \
      if (str->isInteger()) {                                                 \
        return genDefBool(str->toInt64() OP src2->getConstValAsInt());        \
      } else if (str->isNumeric()) {                                          \
        return genDefBool(str->toDouble() OP                                  \
                          double(src2->getConstValAsInt()));                  \
      }                                                                       \
      return genDefBool(0 OP src2->getConstValAsInt());                       \
    }                                                                         \
    /* ConstInt cmp ConstBool */                                              \
    if (src1->getType() == Type::Int && src2->getType() == Type::Bool) {      \
      return genDefBool(bool(src1->getConstValAsInt()) OP                     \
                        src2->getConstValAsBool());                           \
    }                                                                         \
  }                                                                           \
  if (src1->getType() == Type::Arr && src2->getType() != Type::Arr) {         \
    /* array is always greater */                                             \
    return genDefBool(1 OP 0)                             ;                   \
  }                                                                           \
  if (src2->getType() == Type::Arr && src1->getType() != Type::Arr) {         \
    /* array is always greater */                                             \
    return genDefBool(0 OP 1)                             ;                   \
  }                                                                           \
} while (0)

SSATmp* Simplifier::simplifyGt(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) > 0)
  SIMPLIFY_CMP(>, OpGt, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyGte(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) >= 0)
  SIMPLIFY_CMP(>=, OpGte, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyLt(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) < 0)
  SIMPLIFY_CMP(<, OpLt, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyLte(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) <= 0)
  SIMPLIFY_CMP(<=, OpLte, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyEq(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) == 0)
  SIMPLIFY_CMP(==, OpEq, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyNeq(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) != 0)
  SIMPLIFY_CMP(!=, OpNeq, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifySame(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) == 0)
  SIMPLIFY_CMP(==, OpSame, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyNSame(SSATmp* src1, SSATmp* src2) {
  #define EXP(c) ((c) != 0)
  SIMPLIFY_CMP(!=, OpNSame, EXP);
  #undef EXP
  return NULL;
}
SSATmp* Simplifier::simplifyIsType(Type::Tag type, SSATmp* src) {
  ASSERT(Type::isUnboxed(type));
  ASSERT(type != Type::Cell);
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
SSATmp* Simplifier::simplifyConv(Type::Tag toType, SSATmp* src) {
  Type::Tag type = src->getType();
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
SSATmp* Simplifier::simplifyLdObjClass(SSATmp* obj) {
  return NULL;
}
SSATmp* Simplifier::simplifyLdCachedClass(SSATmp* obj) {
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

SSATmp* Simplifier::simplifyIsSet(SSATmp* src, bool negate) {
  return NULL;
}

SSATmp* Simplifier::simplifyLdClsPropAddr(SSATmp* cls,
                                          SSATmp* clsName,
                                          SSATmp* propName) {
  if (clsName->getType() == Type::Null) {
    IRInstruction* clsInst = cls->getInstruction();
    if (clsInst->getOpcode() == LdCls) {
      SSATmp* clsName = clsInst->getSrc(0);
      ASSERT(clsName->isConst() && clsName->getType() == Type::StaticStr);
      return genLdClsPropAddr(cls, clsName, propName);
    }
  }
  return NULL;
}

SSATmp* Simplifier::simplifyAllocActRec(SSATmp* src1, SSATmp* src2,
                                        SSATmp* src3, SSATmp* src4,
                                        SSATmp* src5) {
  return NULL;
}

// TODO: Remove this if unused
SSATmp* Simplifier::simplifyUnbox(Type::Tag type, SSATmp* src) {
  Type::Tag srcType = src->getType();
  if (Type::isUnboxed(srcType)) {
    return src;
  }
  if (Type::isBoxed(srcType)) {
    srcType = Type::getInnerType(srcType);
    if (Type::isMoreRefined(srcType, type)) {
      type = srcType;
    }
    return m_tb->genLdRef(src, type, NULL);
  }
  return NULL;
}
SSATmp* Simplifier::simplifyUnbox(Type::Tag type, SSATmp* src,
                                  LabelInstruction* typeFailLabel) {
  ASSERT(Type::isUnboxed(type));
  Type::Tag srcType = src->getType();
  if (Type::isUnboxed(srcType)) {
    // TODO: generate a guardType if this assertion fails
    ASSERT(!Type::isMoreRefined(type, srcType));
    return src;
  }
  if (Type::isBoxed(srcType)) {
    srcType = Type::getInnerType(srcType);
    if (Type::isMoreRefined(srcType, type)) {
      type = srcType;
    }
    return m_tb->genLdRef(src, type, typeFailLabel->getTrace());
  }
  return NULL;
}
SSATmp* Simplifier::simplifyJcc(Opcode opc, Type::Tag type,
                                SSATmp* src1, SSATmp* src2,
                                LabelInstruction* label) {
  return NULL;
}
SSATmp* Simplifier::simplifyJmpZ(SSATmp* src, LabelInstruction* label) {
  return NULL;
}
SSATmp* Simplifier::simplifyJmpNz(SSATmp* src, LabelInstruction* label) {
  return NULL;
}

Simplifier::Simplifier(TraceBuilder* t) : m_tb(t) {
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
