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
  for (uint32_t i = 0; i < inst->getNumSrcs(); i++) {
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
      src2->getValInt() != 1) {
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

  Opcode opc = inst->getOpcode();
  switch (opc) {
  case OpAdd:       return simplifyAdd(src1, src2);
  case OpSub:       return simplifySub(src1, src2);
  case OpMul:       return simplifyMul(src1, src2);
  case OpAnd:       return simplifyAnd(src1, src2);
  case OpOr:        return simplifyOr(src1, src2);
  case OpXor:       return simplifyXor(src1, src2);

  case OpGt:
  case OpGte:
  case OpLt:
  case OpLte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
    return simplifyCmp(opc, src1, src2);

  case Concat:        return simplifyConcat(src1, src2);
  case Mov:           return simplifyMov(src1);
  case LdClsPropAddr: return simplifyLdClsPropAddr(inst);
  case ConvBoolToArr: return simplifyConvToArr(inst);
  case ConvDblToArr:  return simplifyConvToArr(inst);
  case ConvIntToArr:  return simplifyConvToArr(inst);
  case ConvStrToArr:  return simplifyConvToArr(inst);
  case ConvToBool:    return simplifyConvToBool(inst);
  case ConvArrToDbl:  return simplifyConvArrToDbl(inst);
  case ConvBoolToDbl: return simplifyConvBoolToDbl(inst);
  case ConvIntToDbl:  return simplifyConvIntToDbl(inst);
  case ConvStrToDbl:  return simplifyConvStrToDbl(inst);
  case ConvToInt:     return simplifyConvToInt(inst);
  case ConvToObj:     return simplifyConvToObj(inst);
  case ConvToStr:     return simplifyConvToStr(inst);
  case Unbox:         return simplifyUnbox(inst);
  case UnboxPtr:      return simplifyUnboxPtr(inst);
  case IsType:
  case IsNType:       return simplifyIsType(inst);
  case CheckInit:
  case CheckInitMem:  return simplifyCheckInit(inst);

  case JmpZero:
  case JmpNZero:
    return simplifyCondJmp(inst);

  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpSame:
  case JmpNSame:
    return simplifyQueryJmp(inst);

  case JmpIsType:
  case JmpIsNType:
    return simplifyJmpIsType(inst);

  case PrintStr:
  case PrintInt:
  case PrintBool:    return simplifyPrint(inst);
  case DecRef:
  case DecRefNZOrBranch:
  case DecRefNZ:     return simplifyDecRef(inst);
  case IncRef:       return simplifyIncRef(inst);
  case GuardType:    return simplifyGuardType(inst);

  case LdCls:        return simplifyLdCls(inst);

  case LdCtx:        return simplifyLdCtx(inst);
  case LdClsCtx:     return simplifyLdClsCtx(inst);
  case GetCtxFwdCall:return simplifyGetCtxFwdCall(inst);

  case SpillStack:   return simplifySpillStack(inst);
  case Call:         return simplifyCall(inst);

  case Jmp_:
  case JmpInstanceOf:
  case JmpNInstanceOf:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
    return nullptr;

  case LdObjClass:
  case LdCachedClass:
  case DecRefLoc:
  case DecRefStack:
  case GuardLoc:
  case GuardStk:
  case LdThis:
  case LdLoc:
  case LdMem:
  case LdRef:
  case LdStack:
  case LdPropAddr:
  case LdClsCns:
  case LdObjMethod:
  case RetVal:
  case FreeActRec:
  case LdClsMethodCache:
  case LdClsMethodFCache:
  case LdClsMethod:
  case ExitTrace:
  case ExitSlow:
  case ExitGuardFailure:
  case StMem:
  case StMemNT:
  case StLoc:
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

/*
 * Looks for whether the value in tmp was defined by a load, and if
 * so, changes that load into a load that guards on the given
 * type. Returns true if it succeeds.
 */
static bool hoistGuardToLoad(SSATmp* tmp, Type type) {
  IRInstruction* inst = tmp->getInstruction();
  switch (inst->getOpcode()) {
    case Mov:
    case IncRef:
    {
      // if inst is an incref or move, then chase down its src
      if (hoistGuardToLoad(inst->getSrc(0), type)) {
        // guard was successfully attached to a load instruction
        // refine the type of this mov/incref
        // Note: We can also further simplify incref's here if type is not
        // ref-counted
        tmp->setType(type);
        inst->setTypeParam(type);
        return true;
      }
      break;
    }
    case LdLoc:
    case LdStack:
    case LdMem:
    case LdProp:
    case LdRef:
    case LdClsCns:
    {
      if (!inst->getTaken()) {
        // Not a control flow instruction, so can't give it check semantics
        break;
      }
      Type instType = tmp->getType();
      if (instType == Type::Gen ||
          (instType == Type::Cell && !type.isBoxed())) {
        tmp->setType(type);
        inst->setTypeParam(type);
        return true;
      }
      break;
    }
    default:
      break;
  }
  return false;
}

SSATmp* Simplifier::simplifySpillStack(IRInstruction* inst) {
  SSATmp* sp = inst->getSrc(0);
  auto const spDeficit = inst->getSrc(1)->getValInt();
  auto       spillVals = inst->getSrcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const spillCells = spillValueCells(inst);
  int64_t adjustment = spDeficit - spillCells;
  for (uint32_t i = 0, cellOff = 0; i < numSpillSrcs; i++) {
    const int64_t offset = cellOff + adjustment;
    if (spillVals[i]->getType() == Type::ActRec) {
      cellOff += kNumActRecCells;
      i += kSpillStackActRecExtraArgs;
      continue;
    }
    auto* srcInst = spillVals[i]->getInstruction();
    // If our value came from a LdStack on the same sp and offset,
    // we don't need to spill it.
    if (srcInst->getOpcode() == LdStack && srcInst->getSrc(0) == sp &&
        srcInst->getSrc(1)->getValInt() == offset) {
      spillVals[i] = m_tb->genDefNone();
    }
    cellOff++;
  }

  // Note: although the instruction might have been modified above, we still
  // need to return nullptr so that it gets cloned later if it's stack-allocated
  return nullptr;
}

SSATmp* Simplifier::simplifyCall(IRInstruction* inst) {
  auto spillVals  = inst->getSrcs().subpiece(3);
  IRInstruction* spillStack = m_tb->getSp()->getInstruction();
  if (spillStack->getOpcode() != SpillStack) {
    return nullptr;
  }

  SSATmp* sp = spillStack->getSrc(0);
  int baseOffset = spillStack->getSrc(1)->getValInt() -
                   spillValueCells(spillStack);
  auto const numSpillSrcs = spillVals.size();
  for (int32_t i = 0; i < numSpillSrcs; i++) {
    const int64_t offset = -(i + 1) + baseOffset;
    assert(spillVals[i]->getType() != Type::ActRec);
    IRInstruction* srcInst = spillVals[i]->getInstruction();
    // If our value came from a LdStack on the same sp and offset,
    // we don't need to spill it.
    if (srcInst->getOpcode() == LdStack && srcInst->getSrc(0) == sp &&
        srcInst->getSrc(1)->getValInt() == offset) {
      spillVals[i] = m_tb->genDefNone();
    }
  }

  // Note: although the instruction might have been modified above, we still
  // need to return nullptr so that it gets cloned later if it's stack-allocated
  return nullptr;
}

SSATmp* Simplifier::simplifyLdCtx(IRInstruction* inst) {
  const Func* func = inst->getSrc(1)->getValFunc();
  if (func->isStatic()) {
    // ActRec->m_cls of a static function is always a valid class pointer with
    // the bottom bit set
    return m_tb->gen(LdCctx, inst->getSrc(0));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdClsCtx(IRInstruction* inst) {
  SSATmp*  ctx = inst->getSrc(0);
  Type ctxType = ctx->getType();
  if (ctxType.equals(Type::Obj)) {
    // this pointer... load its class ptr
    return m_tb->gen(LdObjClass, ctx);
  }
  if (ctxType.equals(Type::Cctx)) {
    return m_tb->gen(LdClsCctx, ctx);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyGetCtxFwdCall(IRInstruction* inst) {
  SSATmp*  srcCtx = inst->getSrc(0);
  if (srcCtx->isA(Type::Cctx)) {
    return srcCtx;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdCls(IRInstruction* inst) {
  SSATmp* clsName = inst->getSrc(0);
  if (clsName->isConst()) {
    const Class* cls = Unit::lookupClass(clsName->getValStr());
    if (cls) {
      if (RuntimeOption::RepoAuthoritative && (cls->attrs() & AttrUnique)) {
        // the class is unique
        return m_tb->genDefConst(cls);
      }
      const Class* ctx = inst->getSrc(1)->getValClass();
      if (ctx && ctx->classof(cls)) {
        // the class of the current function being compiled is the
        // same as or derived from cls, so cls must be defined and
        // cannot change the next time we execute this same code
        return m_tb->genDefConst(cls);
      }
    }
    return m_tb->gen(LdClsCached, clsName);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyGuardType(IRInstruction* inst) {
  Type type    = inst->getTypeParam();
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->getType();
  if (srcType.subtypeOf(type)) {
    /*
     * the type of the src is the same or more refined than type, so the
     * guard is unnecessary.
     */
    return src;
  }
  if (type.strictSubtypeOf(srcType)) {
    if (hoistGuardToLoad(src, type)) {
      return src;
    }
  } else {
    if (type.equals(Type::Str) && srcType.maybe(Type::Str)) {
      // If we're guarding against Str and srcType has StaticStr or CountedStr
      // in it, refine the output type. This can happen when we have a
      // KindOfString guard from Translator but internally we know a more
      // specific subtype of Str.
      FTRACE(1, "Guarding {} to {}\n", srcType.toString(), type.toString());
      inst->setTypeParam(type & srcType);
    } else {
      /*
       * incompatible types!  We should just generate a jump here and
       * return null.
       *
       * For now, this case should currently be impossible, but it may
       * come up later due to other optimizations.  The assert is so
       * we'll remember this spot ...
       */
      not_implemented();
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyQueryJmp(IRInstruction* inst) {
  SSATmp* src1 = inst->getSrc(0);
  SSATmp* src2 = inst->getSrc(1);
  Opcode opc = inst->getOpcode();
  // reuse the logic in simplifyCmp.
  SSATmp* newCmp = simplifyCmp(queryJmpToQueryOp(opc), src1, src2);
  if (!newCmp) return nullptr;

  SSATmp* newQueryJmp = makeInstruction(
    [=] (IRInstruction* condJmp) -> SSATmp* {
      SSATmp* newCondJmp = simplifyCondJmp(condJmp);
      if (newCondJmp) return newCondJmp;
      if (condJmp->getOpcode() == Nop) {
        // simplifyCondJmp folded the branch into a nop
        inst->convertToNop();
      }
      // Couldn't fold condJmp or combine it with newCmp
      return nullptr;
    },
    JmpNZero,
    inst->getTaken(),
    newCmp);
  if (!newQueryJmp) return nullptr;
  return newQueryJmp;
}

SSATmp* Simplifier::simplifyMov(SSATmp* src) {
  return src;
}

SSATmp* Simplifier::simplifyNot(SSATmp* src) {
  IRInstruction* inst = src->getInstruction();
  Opcode op = inst->getOpcode();

  // TODO: Add more algebraic simplification rules for NOT
  switch (op) {
    case ConvToBool:
      return simplifyNot(inst->getSrc(0));
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
      // XXX: this could technically be losing a ConvToBool, except
      // that we kinda know "not" instructions (Xor with 1) are always
      // going to be followed by ConvToBool.
      //
      // TODO(#2058865): This would make more sense with a real Not
      // instruction and allowing boolean output types for query ops.
      return m_tb->genCmp(negateQueryOp(op),
                          inst->getSrc(0),
                          inst->getSrc(1));
    case InstanceOf:
    case NInstanceOf:
    case InstanceOfBitmask:
    case NInstanceOfBitmask:
      // TODO: combine this with the above check and use isQueryOp or
      // add an isNegatable.
      return m_tb->gen(negateQueryOp(op),
                       inst->getNumSrcs(),
                       inst->getSrcs().begin());
    // TODO !(X | non_zero) --> 0
    default: (void)op;
  }
  return nullptr;
}

#define SIMPLIFY_CONST(OP) do {                                               \
  /* don't canonicalize to the right, OP might not be commutative */          \
  if (src1->isConst() && src2->isConst()) {                                   \
    if (src1->getType().isNull()) {                                     \
      /* Null op Null */                                                      \
      if (src2->getType().isNull()) {                                    \
        return genDefInt(0 OP 0);                                             \
      }                                                                       \
      /* Null op ConstInt */                                                  \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(0 OP src2->getValInt());                      \
      }                                                                       \
      /* Null op ConstBool */                                                 \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(0 OP src2->getValBool());                     \
      }                                                                       \
      /* Null op StaticStr */                                                 \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getValStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(0 OP str->toInt64());                              \
        }                                                                     \
        return genDefInt(0 OP 0);                                             \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::Int) {                                       \
      /* ConstInt op Null */                                                  \
      if (src2->getType().isNull()) {                                   \
        return genDefInt(src1->getValInt() OP 0);                      \
      }                                                                       \
      /* ConstInt op ConstInt */                                              \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(src1->getValInt() OP                          \
                         src2->getValInt());                           \
      }                                                                       \
      /* ConstInt op ConstBool */                                             \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(src1->getValInt() OP                          \
                         int(src2->getValBool()));                     \
      }                                                                       \
      /* ConstInt op StaticStr */                                             \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getValStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(src1->getValInt() OP str->toInt64());       \
        }                                                                     \
        return genDefInt(src1->getValInt() OP 0);                      \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::Bool) {                                      \
      /* ConstBool op Null */                                                 \
      if (src2->getType().isNull()) {                                   \
        return genDefInt(src1->getValBool() OP 0);                     \
      }                                                                       \
      /* ConstBool op ConstInt */                                             \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(int(src1->getValBool()) OP                    \
                         src2->getValInt());                           \
      }                                                                       \
      /* ConstBool op ConstBool */                                            \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(src1->getValBool() OP                         \
                         src2->getValBool());                          \
      }                                                                       \
      /* ConstBool op StaticStr */                                            \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str = src2->getValStr();                     \
        if (str->isInteger()) {                                               \
          return genDefInt(int(src1->getValBool()) OP str->toInt64()); \
        }                                                                     \
        return genDefInt(int(src1->getValBool()) OP 0);                \
      }                                                                       \
    }                                                                         \
    if (src1->getType() == Type::StaticStr) {                                 \
      const StringData* str = src1->getValStr();                       \
      int64_t strInt = 0;                                                       \
      if (str->isInteger()) {                                                 \
        strInt = str->toInt64();                                              \
      }                                                                       \
      /* StaticStr op Null */                                                 \
      if (src2->getType().isNull()) {                                    \
        return genDefInt(strInt OP 0);                                        \
      }                                                                       \
      /* StaticStr op ConstInt */                                             \
      if (src2->getType() == Type::Int) {                                     \
        return genDefInt(strInt OP src2->getValInt());                 \
      }                                                                       \
      /* StaticStr op ConstBool */                                            \
      if (src2->getType() == Type::Bool) {                                    \
        return genDefInt(strInt OP int(src2->getValBool()));           \
      }                                                                       \
      /* StaticStr op StaticStr */                                            \
      if (src2->getType() == Type::StaticStr) {                               \
        const StringData* str2 = src2->getValStr();                    \
        if (str2->isInteger()) {                                              \
          return genDefInt(strInt OP str2->toInt64());                        \
        }                                                                     \
        return genDefInt(strInt OP 0);                                        \
      }                                                                       \
    }                                                                         \
  }                                                                           \
} while (0)

#define SIMPLIFY_COMMUTATIVE(OP, NAME) do {                                 \
  SIMPLIFY_CONST(OP);                                                       \
  if (src1->isConst() && !src2->isConst()) {                                \
    return m_tb->gen##NAME(src2, src1);                                     \
  }                                                                         \
  if (src1->isA(Type::Int) && src2->isA(Type::Int)) {                       \
    IRInstruction* inst1 = src1->getInstruction();                          \
    IRInstruction* inst2 = src2->getInstruction();                          \
    if (inst1->getOpcode() == Op##NAME && inst1->getSrc(1)->isConst()) {    \
      /* (X + C1) + C2 --> X + C3 */                                        \
      if (src2->isConst()) {                                                \
        int64_t right = inst1->getSrc(1)->getValInt();                      \
        right OP##= src2->getValInt();                                      \
        return m_tb->gen##NAME(inst1->getSrc(0), genDefInt(right));         \
      }                                                                     \
      /* (X + C1) + (Y + C2) --> X + Y + C3 */                              \
      if (inst2->getOpcode() == Op##NAME && inst2->getSrc(1)->isConst()) {  \
        int64_t right = inst1->getSrc(1)->getValInt();                      \
        right OP##= inst2->getSrc(1)->getValInt();                          \
        SSATmp* left = m_tb->gen##NAME(inst1->getSrc(0), inst2->getSrc(0)); \
        return m_tb->gen##NAME(left, genDefInt(right));                     \
      }                                                                     \
    }                                                                       \
  }                                                                         \
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
  if (src2->isConst() && src2->getType() == Type::Int) {
    int64_t src2Val = src2->getValInt();
    // X + 0 --> X
    if (src2Val == 0) {
      if (src1->getType() == Type::Bool) {
        return m_tb->genConvToInt(src1);
      }
      return src1;
    }
    // X + -C --> X - C
    if (src2Val < 0) {
      return m_tb->genSub(src1, genDefInt(-src2Val));
    }
  }
  // X + (0 - Y) --> X - Y
  IRInstruction* inst2 = src2->getInstruction();
  Opcode op2 = inst2->getOpcode();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->getType() == Type::Int) {
      if (src->getValInt() == 0) {
        return m_tb->genSub(src1, inst2->getSrc(1));
      }
    }
  }
  return nullptr;
}
SSATmp* Simplifier::simplifySub(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_CONST(-);
  // X - X --> 0
  if (src1 == src2) {
    return genDefInt(0);
  }
  if (src2->isConst() && src2->getType() == Type::Int) {
    int64_t src2Val = src2->getValInt();
    // X - 0 --> X
    if (src2Val == 0) {
      if (src1->getType() == Type::Bool) {
        return m_tb->genConvToInt(src1);
      }
      return src1;
    }
    // X - -C --> X + C
    if (src2Val < 0 && src2Val > std::numeric_limits<int64_t>::min()) {
      return m_tb->genAdd(src1, genDefInt(-src2Val));
    }
  }
  // X - (0 - Y) --> X + Y
  IRInstruction* inst2 = src2->getInstruction();
  Opcode op2 = inst2->getOpcode();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->getType() == Type::Int) {
      if (src->getValInt() == 0) {
        return m_tb->genAdd(src1, inst2->getSrc(1));
      }
    }
  }
  // TODO patterns in the form of:
  // (X - C1) + (X - C2)
  // (X - C1) + C2
  // (X - C1) + (X + C2)
  return nullptr;
}
SSATmp* Simplifier::simplifyMul(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(*, Mul);
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X * (-1) --> -X
    if (src2->getValInt() == -1) {
      return m_tb->genSub(genDefInt(0), src1);
    }
    // X * 0 --> 0
    if (src2->getValInt() == 0) {
      return genDefInt(0);
    }
    // X * 1 --> X
    if (src2->getValInt() == 1) {
      if (src1->getType() == Type::Bool) {
        return m_tb->genConvToInt(src1);
      }
      return src1;
    }
    // X * 2 --> X + X
    if (src2->getValInt() == 2) {
      return m_tb->genAdd(src1, src1);
    }
    // TODO once IR has shifts
    // X * 2^C --> X << C
    // X * (2^C + 1) --> ((X << C) + X)
    // X * (2^C - 1) --> ((X << C) - X)
  }
  return nullptr;
}
SSATmp* Simplifier::simplifyAnd(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(&, |, And, Or);
  // X & X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X & 0 --> 0
    if (src2->getValInt() == 0) {
      return genDefInt(0);
    }
    // X & (~0) --> X
    if (src2->getValInt() == ~0L) {
      return src1;
    }
  }
  return nullptr;
}
SSATmp* Simplifier::simplifyOr(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(|, &, Or, And);
  // X | X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst() && src2->getType() == Type::Int) {
    // X | 0 --> X
    if (src2->getValInt() == 0) {
      return src1;
    }
    // X | (~0) --> ~0
    if (src2->getValInt() == ~0L) {
      return genDefInt(~0L);
    }
  }
  return nullptr;
}
SSATmp* Simplifier::simplifyXor(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(^, Xor);
  // X ^ X --> 0
  if (src1 == src2)
    return genDefInt(0);
  // X ^ 0 --> X
  if (src2->isConst() && src2->getType() == Type::Int) {
    if (src2->getValInt() == 0) {
      return src1;
    }
  }
  if (isNotInst(src1, src2)) {
    return simplifyNot(src1);
  }
  return nullptr;
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
  if (!src1->getType().notBoxed() || src1->getType() == Type::Cell ||
      !src2->getType().notBoxed() || src2->getType() == Type::Cell) {
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // OpSame and OpNSame have some special rules
  // ---------------------------------------------------------------------

  if (opName == OpSame || opName == OpNSame) {
    // OpSame and OpNSame do not perform type juggling
    if (src1->getType() != src2->getType()) {
      if (!(src1->getType().isString() && src2->getType().isString())) {
        return genDefBool(opName == OpNSame);
      }
    }

    // src1 and src2 are same type, treating Str and StaticStr as the same

    // OpSame and OpNSame have special rules for string and object
    // Other types may simplify to OpEq and OpNeq, respectively
    if (src1->getType().isString() && src2->getType().isString()) {
      if (src1->isConst() && src2->isConst()) {
        auto str1 = src1->getValStr();
        auto str2 = src2->getValStr();
        bool same = str1->same(str2);
        return genDefBool(cmpOp(opName, same, 1));
      } else {
        return nullptr;
      }
    }
    if (src1->getType() == Type::Obj && src2->getType() == Type::Obj) {
      return nullptr;
    }
    // for arrays, don't simplify Same to Eq
    if (src1->getType() == Type::Arr && src2->getType() == Type::Arr) {
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
  if (src1->getType().isNull() && src2->getType().isNull()) {
    return genDefBool(cmpOp(opName, 0, 0));
  }
  // const cmp const
  // TODO this list is incomplete - feel free to add more
  // TODO: can simplify const arrays when sizes are different or both 0
  if (src1->isConst() && src2->isConst()) {
    // StaticStr cmp StaticStr
    if (src1->getType() == Type::StaticStr &&
        src2->getType() == Type::StaticStr) {
      int cmp = src1->getValStr()->compare(src2->getValStr());
      return genDefBool(cmpOp(opName, cmp, 0));
    }
    // ConstInt cmp ConstInt
    if (src1->getType() == Type::Int && src2->getType() == Type::Int) {
      return genDefBool(
        cmpOp(opName, src1->getValInt(), src2->getValInt()));
    }
    // ConstBool cmp ConstBool
    if (src1->getType() == Type::Bool && src2->getType() == Type::Bool) {
      return genDefBool(
        cmpOp(opName, src1->getValBool(), src2->getValBool()));
    }
  }

  // ---------------------------------------------------------------------
  // Constant bool comparisons can be strength-reduced
  // NOTE: Comparisons with bools get juggled to bool.
  // ---------------------------------------------------------------------

  // Perform constant-bool optimizations
  if (src2->getType() == Type::Bool && src2->isConst()) {
    bool b = src2->getValBool();

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
      (src1->getType().isString() && src2->getType().isString())) {
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
  if (src1->getType().isNull()) {
    return m_tb->genCmp(commuteQueryOp(opName), src2, src1);
  }

  // case 1: null cmp string. Convert null to ""
  if (src1->getType().isString() && src2->getType().isNull()) {
    return m_tb->genCmp(opName, src1,
                        m_tb->genDefConst(StringData::GetStaticString("")));
  }

  // case 2a: null cmp anything. Convert null to false
  if (src2->getType().isNull()) {
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
        return m_tb->genCmp(opName, genDefBool(src1->getValInt()), src2);
      } else if (src1->getType().isString()) {
        auto str = src1->getValStr();
        return m_tb->genCmp(opName, genDefBool(str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->getType() == Type::Int && src2->isConst()) {
      // Based on the const bool optimization (above) opName should be OpEq
      always_assert(opName == OpEq);

      if (src2->getValBool()) {
        return m_tb->genCmp(OpNeq, src1, m_tb->genDefConst<int64_t>(0));
      } else {
        return m_tb->genCmp(OpEq, src1, m_tb->genDefConst<int64_t>(0));
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
  if (src2->getType().isString()) {
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
  if (src1->getType().isString() && src1->isConst() &&
      src2->getType() == Type::Int) {
    auto str = src1->getValStr();
    int64_t si; double sd;
    auto st = str->isNumericWithVal(si, sd, true /* allow errors */);
    if (st == KindOfDouble) {
      return m_tb->genCmp(opName, m_tb->genDefConst<double>(sd), src2);
    }
    if (st == KindOfNull) {
      si = 0;
    }
    return m_tb->genCmp(opName, m_tb->genDefConst<int64_t>(si), src2);
  }

  // case 5: array cmp array. No juggling to do
  // same-type simplification is performed above

  // case 6: array cmp anything. Array is greater
  if (src1->isArray()) {
    return genDefBool(cmpOp(opName, 1, 0));
  }
  if (src2->isArray()) {
    return genDefBool(cmpOp(opName, 0, 1));
  }

  // case 7: object cmp anything. Object is greater
  // ---------------------------------------------------------------------
  // Unfortunately, we are unsure of whether Type::Obj is an object or a
  // resource, so this code cannot be applied.
  // ---------------------------------------------------------------------
  return nullptr;
}

SSATmp* Simplifier::simplifyJmpIsType(IRInstruction* inst) {
  SSATmp* res = simplifyIsType(inst);
  if (res == nullptr) return nullptr;
  assert(res->isConst());
  if (res->getValBool()) {
    // Taken jump
    return m_tb->gen(Jmp_, inst->getTaken());
  } else {
    // Not taken jump; turn jump into a nop
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyIsType(IRInstruction* inst) {
  bool trueSense =
    inst->getOpcode() == IsType || inst->getOpcode() == JmpIsType;
  auto    type = inst->getTypeParam();
  auto    src  = inst->getSrc(0);
  auto srcType = src->getType();

  // The comparisons below won't work for these cases covered by this
  // assert, and we currently don't generate these types.
  assert(type.isKnownUnboxedDataType() && type != Type::StaticStr);
  if (type != Type::Obj) {
    if (srcType.subtypeOf(type) || (type.isString() && srcType.isString())) {
      return genDefBool(trueSense);
    }
    if (srcType != Type::Cell) {
      return genDefBool(!trueSense);
    }
  }
  if (srcType != Type::Obj) {
    // Note: for IsObject*, we need to emit a call to ObjectData::isResource
    // (or equivalent), so we can't fold away the case where we know we are
    // checking an object.
    return genDefBool(!trueSense);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConcat(SSATmp* src1, SSATmp* src2) {
  if (src1->isConst() && src1->getType() == Type::StaticStr &&
      src2->isConst() && src2->getType() == Type::StaticStr) {
    StringData* str1 = const_cast<StringData *>(src1->getValStr());
    StringData* str2 = const_cast<StringData *>(src2->getValStr());
    StringData* merge = StringData::GetStaticString(concat_ss(str1, str2));
    return m_tb->genDefConst(merge);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToArr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    Array arr = Array::Create(src->getValVariant());
    return m_tb->genDefConst(ArrayData::GetScalarArray(arr.get()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToBool(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->getType();
  if (srcType == Type::Bool) {
    return src;
  }
  if (srcType.isNull()) {
    return genDefBool(false);
  }
  if (srcType == Type::Obj) {
    return genDefBool(true);
  }
  if (src->isConst()) {
    if (srcType == Type::Int) {
      return genDefBool(bool(src->getValInt()));
    }
    if (srcType == Type::StaticStr) {
      // only the strings "", and "0" convert to false, all other strings
      // are converted to true
      const StringData* str = src->getValStr();
      return genDefBool(!str->empty() && !str->isZero());
    }
    if (srcType.isArray()) {
      if (src->getValArr()->empty()) {
        return genDefBool(false);
      }
      return genDefBool(true);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return genDefDbl(0.0);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    return genDefDbl(double(src->getValBool()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    return genDefDbl(double(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    const StringData *str = src->getValStr();
    if (str->isNumeric()) {
      return genDefDbl(str->toDouble());
    }
    return genDefDbl(0.0);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToInt(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->getType();
  if (srcType == Type::Int) {
    return src;
  }
  if (srcType.isNull()) {
    return genDefInt(0);
  }
  if (src->isConst()) {
    if (src->getType() == Type::Bool) {
      return genDefInt(int(src->getValBool()));
    }
    if (srcType == Type::StaticStr) {
      const StringData *str = src->getValStr();
      if (str->isInteger()) {
        return genDefInt(str->toInt64());
      }
      return genDefInt(0);
    }
    if (srcType.isArray()) {
      if (src->getValArr()->empty()) {
        return genDefInt(0);
      }
      return genDefInt(1);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToObj(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->getType();
  if (srcType == Type::Obj) {
    return src;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToStr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->getType();
  if (srcType.isString()) {
    return src;
  }
  // arrays always get converted to the string "Array"
  if (srcType.isArray()) {
    return m_tb->genDefConst(StringData::GetStaticString("Array"));
  }
  if (srcType.isNull()) {
    return m_tb->genDefConst(StringData::GetStaticString(""));
  }
  if (src->isConst()) {
    if (srcType == Type::Bool) {
      if (src->getValBool()) {
        return m_tb->genDefConst(StringData::GetStaticString("1"));
      }
      return m_tb->genDefConst(StringData::GetStaticString(""));
    }
    if (srcType == Type::Int) {
      std::stringstream ss;
      ss << src->getValInt();
      return m_tb->genDefConst(StringData::GetStaticString(ss.str()));
    }
    if (srcType == Type::Dbl) {
      // TODO constant dbl to string
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdClsPropAddr(IRInstruction* inst) {
  SSATmp* propName  = inst->getSrc(1);
  if (!propName->isConst()) return nullptr;

  SSATmp* cls   = inst->getSrc(0);
  const StringData* clsNameString  = cls->isConst()
                                     ? cls->getValClass()->preClass()->name()
                                     : nullptr;
  if (!clsNameString) {
    // see if you can get the class name from a LdCls
    IRInstruction* clsInst = cls->getInstruction();
    if (clsInst->getOpcode() == LdCls || clsInst->getOpcode() == LdClsCached) {
      SSATmp* clsName = clsInst->getSrc(0);
      assert(clsName->isA(Type::Str));
      if (clsName->isConst()) {
        clsNameString = clsName->getValStr();
      }
    }
  }
  if (!clsNameString) return nullptr;

  // We known both the class name and the property name statically so
  // we can use the caching version of LdClsPropAddr.  To avoid doing
  // accessibility checks, we only do this if the context class is the
  // same as the actual class the property is on.
  auto const ctxCls = inst->getSrc(2)->getValClass();
  if (!ctxCls || !clsNameString->isame(ctxCls->preClass()->name())) {
    return nullptr;
  }

  return m_tb->gen(LdClsPropAddrCached,
                   inst->getTaken(),
                   cls,
                   propName,
                   m_tb->genDefConst(clsNameString),
                   inst->getSrc(2));
}

SSATmp* Simplifier::simplifyUnbox(IRInstruction* inst) {
  auto* src = inst->getSrc(0);
  auto type = outputType(inst);

  Type srcType = src->getType();
  if (srcType.notBoxed()) {
    assert(type.equals(srcType));
    return src;
  }
  if (srcType.isBoxed()) {
    srcType = srcType.innerType();
    assert(type.equals(srcType));
    return m_tb->genLdRef(src, type, inst->getTaken()->getTrace());
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyUnboxPtr(IRInstruction* inst) {
  if (inst->getSrc(0)->isA(Type::PtrToCell)) {
    // Nothing to unbox
    return inst->getSrc(0);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyCheckInit(IRInstruction* inst) {
  Type srcType = inst->getSrc(0)->getType();
  srcType = inst->getOpcode() == CheckInitMem ? srcType.deref() : srcType;
  assert(srcType.notPtr());
  assert(inst->getTaken());
  if (srcType.isInit()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyPrint(IRInstruction* inst) {
  if (inst->getSrc(0)->getType().isNull()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRef(IRInstruction* inst) {
  if (!isRefCounted(inst->getSrc(0))) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyIncRef(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (!isRefCounted(src)) {
    return src;
  }
  return nullptr;
}

SSATmp* Simplifier::genDefInt(int64_t val) {
  return m_tb->genDefConst<int64_t>(val);
}

SSATmp* Simplifier::genDefDbl(double val) {
 return m_tb->genDefConst<double>(val);
}

SSATmp* Simplifier::genDefBool(bool val) {
  return m_tb->genDefConst<bool>(val);
}

SSATmp* Simplifier::simplifyCondJmp(IRInstruction* inst) {
  SSATmp* const src            = inst->getSrc(0);
  IRInstruction* const srcInst = src->getInstruction();
  const Opcode srcOpcode       = srcInst->getOpcode();

  // After other simplifications below (isConvIntOrPtrToBool), we can
  // end up with a non-Bool input.  Nothing more to do in this case.
  if (src->getType() != Type::Bool) {
    return nullptr;
  }

  // Constant propagate.
  if (src->isConst()) {
    bool val = src->getValBool();
    if (inst->getOpcode() == JmpZero) {
      val = !val;
    }
    if (val) {
      return m_tb->gen(Jmp_, inst->getTaken());
    }
    inst->convertToNop();
    return nullptr;
  }

  // Pull negations into the jump.
  if (isNotInst(src)) {
    return m_tb->gen(inst->getOpcode() == JmpZero ? JmpNZero : JmpZero,
                     inst->getTaken(),
                     srcInst->getSrc(0));
  }

  /*
   * Try to combine the src inst with the Jmp.  We can't do any
   * combinations of the src instruction with the jump if the src's
   * are refcounted, since we may have dec refs between the src
   * instruction and the jump.
   */
  for (auto& src : srcInst->getSrcs()) {
    if (isRefCounted(src)) return nullptr;
  }

  // If the source is conversion of an int or pointer to boolean, we
  // can test the int/ptr value directly.
  if (isConvIntOrPtrToBool(srcInst)) {
    return m_tb->gen(inst->getOpcode(),
                     inst->getTaken(),
                     srcInst->getSrc(0));
  }

  // Fuse jumps with query operators.
  if (isQueryOp(srcOpcode) && !disableBranchFusion(srcOpcode)) {
    SrcRange ssas = srcInst->getSrcs();
    return m_tb->gen(queryToJmpOp(
                       inst->getOpcode() == JmpZero
                         ? negateQueryOp(srcOpcode)
                         : srcOpcode),
                     srcInst->getTypeParam(), // if it had a type param
                     inst->getTaken(),
                     ssas.size(),
                     ssas.begin());
  }

  return nullptr;
}

}}} // namespace HPHP::VM::JIT
