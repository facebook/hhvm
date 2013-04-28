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

#include <runtime/base/type_conversions.h>
#include "runtime/vm/translator/hopt/tracebuilder.h"
#include "runtime/vm/runtime.h"

namespace HPHP {
namespace VM {
namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

StackValueInfo getStackValue(SSATmp* sp, uint32_t index) {
  assert(sp->isA(Type::StkPtr));
  IRInstruction* inst = sp->inst();

  switch (inst->op()) {
  case DefSP:
    return {};

  case ReDefGeneratorSP: {
    auto srcInst = inst->getSrc(0)->inst();
    assert(srcInst->op() == StashGeneratorSP);
    return getStackValue(srcInst->getSrc(0), index);
  }
  case ReDefSP:
    return getStackValue(inst->getSrc(1), index);

  case ExceptionBarrier:
    return getStackValue(inst->getSrc(0), index);

  case AssertStk:
    // fallthrough
  case CastStk:
    // fallthrough
  case GuardStk:
    // We don't have a value, but we may know the type due to guarding
    // on it.
    if (inst->getExtra<StackOffset>()->offset == index) {
      return StackValueInfo { inst->getTypeParam() };
    }
    return getStackValue(inst->getSrc(0), index);

  case CallArray: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { nullptr };
    }
    auto info =
      getStackValue(inst->getSrc(0),
                    // Pushes a return value, pops an ActRec and args Array
                    index -
                      (1 /* pushed */ - kNumActRecCells + 1 /* popped */));
    info.spansCall = true;
    return info;
  }

  case Call: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { nullptr };
    }
    auto info =
      getStackValue(inst->getSrc(0),
                    index -
                     (1 /* pushed */ - kNumActRecCells /* popped */));
    info.spansCall = true;
    return info;
  }

  case SpillStack: {
    int64_t numPushed    = 0;
    int32_t numSpillSrcs = inst->getNumSrcs() - 2;

    for (int i = 0; i < numSpillSrcs; ++i) {
      SSATmp* tmp = inst->getSrc(i + 2);
      if (index == numPushed) {
        if (tmp->inst()->op() == IncRef) {
          tmp = tmp->inst()->getSrc(0);
        }
        if (!tmp->type().equals(Type::None)) {
          return StackValueInfo { tmp };
        }
      }
      ++numPushed;
    }

    // This is not one of the values pushed onto the stack by this
    // spillstack instruction, so continue searching.
    SSATmp* prevSp = inst->getSrc(0);
    int64_t numPopped = inst->getSrc(1)->getValInt();
    return getStackValue(prevSp,
                         // pop values pushed by spillstack
                         index - (numPushed - numPopped));
  }

  case InterpOne: {
    SSATmp* prevSp = inst->getSrc(1);
    int64_t spAdjustment = inst->getSrc(3)->getValInt(); // # popped - # pushed
    Type resultType = inst->getTypeParam();
    if (index == 0 && !resultType.equals(Type::None)) {
      return StackValueInfo { resultType };
    }
    return getStackValue(prevSp, index + spAdjustment);
  }

  case SpillFrame:
    return getStackValue(inst->getSrc(0),
                         // pushes an ActRec
                         index - kNumActRecCells);

  case NewObj:
    if (index == kNumActRecCells) {
      // newly allocated object, which we unfortunately don't have any
      // kind of handle to :-(
      return StackValueInfo { Type::Obj };
    }

    return getStackValue(sp->inst()->getSrc(2),
                         // NewObj pushes an object and an ActRec
                         index - (1 + kNumActRecCells));

  default:
    {
      // Assume it's a vector instruction.  This will assert in
      // vectorBaseIdx if not.
      auto const base = inst->getSrc(vectorBaseIdx(inst));
      assert(base->inst()->op() == LdStackAddr);
      if (base->inst()->getExtra<LdStackAddr>()->offset == index) {
        VectorEffects ve(inst);
        assert(ve.baseTypeChanged || ve.baseValChanged);
        return StackValueInfo { ve.baseType.derefIfPtr() };
      }
      return getStackValue(base->inst()->getSrc(0), index);
    }
  }

  // Should not get here!
  not_reached();
}

//////////////////////////////////////////////////////////////////////

static void copyPropSrc(IRInstruction* inst, int index) {
  auto tmp     = inst->getSrc(index);
  auto srcInst = tmp->inst();

  switch (srcInst->op()) {
  case Mov:
    inst->setSrc(index, srcInst->getSrc(0));
    break;

  case IncRef:
    if (!isRefCounted(srcInst->getSrc(0))) {
      srcInst->setOpcode(Mov);
      inst->setSrc(index, srcInst->getSrc(0));
    }
    break;

  default:
    return;
  }
}

void copyProp(IRInstruction* inst) {
  for (uint32_t i = 0; i < inst->getNumSrcs(); i++) {
    copyPropSrc(inst, i);
  }
}

//////////////////////////////////////////////////////////////////////

template<class... Args> SSATmp* Simplifier::cns(Args&&... cns) {
  return m_tb->cns(std::forward<Args>(cns)...);
}

template<class... Args> SSATmp* Simplifier::gen(Args&&... args) {
  return m_tb->gen(std::forward<Args>(args)...);
}

//////////////////////////////////////////////////////////////////////

SSATmp* Simplifier::simplify(IRInstruction* inst) {
  SSATmp* src1 = inst->getSrc(0);
  SSATmp* src2 = inst->getSrc(1);

  Opcode opc = inst->op();
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
  case OpNot:         return simplifyNot(src1);
  case LdClsPropAddr: return simplifyLdClsPropAddr(inst);
  case ConvBoolToArr: return simplifyConvToArr(inst);
  case ConvDblToArr:  return simplifyConvToArr(inst);
  case ConvIntToArr:  return simplifyConvToArr(inst);
  case ConvStrToArr:  return simplifyConvToArr(inst);
  case ConvArrToBool: return simplifyConvArrToBool(inst);
  case ConvDblToBool: return simplifyConvDblToBool(inst);
  case ConvIntToBool: return simplifyConvIntToBool(inst);
  case ConvStrToBool: return simplifyConvStrToBool(inst);
  case ConvArrToDbl:  return simplifyConvArrToDbl(inst);
  case ConvBoolToDbl: return simplifyConvBoolToDbl(inst);
  case ConvIntToDbl:  return simplifyConvIntToDbl(inst);
  case ConvStrToDbl:  return simplifyConvStrToDbl(inst);
  case ConvArrToInt:  return simplifyConvArrToInt(inst);
  case ConvBoolToInt: return simplifyConvBoolToInt(inst);
  case ConvDblToInt:  return simplifyConvDblToInt(inst);
  case ConvStrToInt:  return simplifyConvStrToInt(inst);
  case ConvBoolToStr: return simplifyConvBoolToStr(inst);
  case ConvDblToStr:  return simplifyConvDblToStr(inst);
  case ConvIntToStr:  return simplifyConvIntToStr(inst);
  case ConvCellToBool:return simplifyConvCellToBool(inst);
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
  case LdThis:       return simplifyLdThis(inst);
  case LdCtx:        return simplifyLdCtx(inst);
  case LdClsCtx:     return simplifyLdClsCtx(inst);
  case GetCtxFwdCall:return simplifyGetCtxFwdCall(inst);

  case SpillStack:   return simplifySpillStack(inst);
  case Call:         return simplifyCall(inst);
  case CastStk:      return simplifyCastStk(inst);
  case AssertStk:    return simplifyAssertStk(inst);
  case LdStack:      return simplifyLdStack(inst);
  case LdStackAddr:  return simplifyLdStackAddr(inst);
  case DecRefStack:  return simplifyDecRefStack(inst);
  case DecRefLoc:    return simplifyDecRefLoc(inst);
  case LdLoc:        return simplifyLdLoc(inst);
  case StRef:        return simplifyStRef(inst);

  case ExitOnVarEnv: return simplifyExitOnVarEnv(inst);

  default:
    return nullptr;
  }
}

/*
 * Looks for whether the value in tmp was defined by a load, and if
 * so, changes that load into a load that guards on the given
 * type. Returns true if it succeeds.
 */
static bool hoistGuardToLoad(SSATmp* tmp, Type type) {
  IRInstruction* inst = tmp->inst();
  switch (inst->op()) {
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
      Type instType = tmp->type();
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
  auto const sp           = inst->getSrc(0);
  auto const spDeficit    = inst->getSrc(1)->getValInt();
  auto       spillVals    = inst->getSrcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const spillCells   = spillValueCells(inst);
  int64_t    adjustment   = spDeficit - spillCells;

  // If there's nothing to spill, and no stack adjustment, we don't
  // need the instruction; the old stack is still accurate.
  if (!numSpillSrcs && spDeficit == 0) return sp;

  // If our value came from a LdStack on the same sp and offset,
  // we don't need to spill it.
  for (uint32_t i = 0, cellOff = 0; i < numSpillSrcs; i++) {
    const int64_t offset = cellOff + adjustment;
    auto* srcInst = spillVals[i]->inst();
    if (srcInst->op() == LdStack && srcInst->getSrc(0) == sp &&
        srcInst->getExtra<LdStack>()->offset == offset) {
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
  auto const spillStack = inst->getSrc(0)->inst();
  if (spillStack->op() != SpillStack) {
    return nullptr;
  }

  SSATmp* sp = spillStack->getSrc(0);
  int baseOffset = spillStack->getSrc(1)->getValInt() -
                   spillValueCells(spillStack);
  auto const numSpillSrcs = spillVals.size();
  for (int32_t i = 0; i < numSpillSrcs; i++) {
    const int64_t offset = -(i + 1) + baseOffset;
    assert(spillVals[i]->type() != Type::ActRec);
    IRInstruction* srcInst = spillVals[i]->inst();
    // If our value came from a LdStack on the same sp and offset,
    // we don't need to spill it.
    if (srcInst->op() == LdStack && srcInst->getSrc(0) == sp &&
        srcInst->getExtra<LdStack>()->offset == offset) {
      spillVals[i] = m_tb->genDefNone();
    }
  }

  // Note: although the instruction might have been modified above, we still
  // need to return nullptr so that it gets cloned later if it's stack-allocated
  return nullptr;
}

// We never inline functions that could have a VarEnv, so an
// ExitOnVarEnv that has a frame based on DefInlineFP can be removed.
SSATmp* Simplifier::simplifyExitOnVarEnv(IRInstruction* inst) {
  auto const frameInst = inst->getSrc(0)->inst();
  if (frameInst->op() == DefInlineFP) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdCtx(IRInstruction* inst) {
  const Func* func = inst->getSrc(1)->getValFunc();
  if (func->isStatic()) {
    // ActRec->m_cls of a static function is always a valid class pointer with
    // the bottom bit set
    return gen(LdCctx, inst->getSrc(0));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdClsCtx(IRInstruction* inst) {
  SSATmp*  ctx = inst->getSrc(0);
  Type ctxType = ctx->type();
  if (ctxType.equals(Type::Obj)) {
    // this pointer... load its class ptr
    return gen(LdObjClass, ctx);
  }
  if (ctxType.equals(Type::Cctx)) {
    return gen(LdClsCctx, ctx);
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
      if (Transl::TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
        // the class is always defined
        return cns(cls);
      }
      const Class* ctx = inst->getSrc(1)->getValClass();
      if (ctx && ctx->classof(cls)) {
        // the class of the current function being compiled is the
        // same as or derived from cls, so cls must be defined and
        // cannot change the next time we execute this same code
        return cns(cls);
      }
    }
    return gen(LdClsCached, clsName);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyGuardType(IRInstruction* inst) {
  Type type    = inst->getTypeParam();
  SSATmp* src  = inst->getSrc(0);
  Type srcType = src->type();
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
  Opcode opc = inst->op();
  // reuse the logic in simplifyCmp.
  SSATmp* newCmp = simplifyCmp(queryJmpToQueryOp(opc), src1, src2);
  if (!newCmp) return nullptr;

  SSATmp* newQueryJmp = makeInstruction(
    [=] (IRInstruction* condJmp) -> SSATmp* {
      SSATmp* newCondJmp = simplifyCondJmp(condJmp);
      if (newCondJmp) return newCondJmp;
      if (condJmp->op() == Nop) {
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
  if (src->isConst()) {
    return cns(!src->getValBool());
  }

  IRInstruction* inst = src->inst();
  Opcode op = inst->op();

  switch (op) {
  // !!X --> X
  case OpNot:
    return inst->getSrc(0);

  // !(X cmp Y) --> X opposite_cmp Y
  case OpLt:
  case OpLte:
  case OpGt:
  case OpGte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
    // Not for Dbl:  (x < NaN) != !(x >= NaN)
    if (!inst->getSrc(0)->isA(Type::Dbl) &&
        !inst->getSrc(1)->isA(Type::Dbl)) {
      return gen(negateQueryOp(op), inst->getSrc(0), inst->getSrc(1));
    }
    break;

  case InstanceOfBitmask:
  case NInstanceOfBitmask:
    // TODO: combine this with the above check and use isQueryOp or
    // add an isNegatable.
    return gen(
      negateQueryOp(op),
      std::make_pair(inst->getNumSrcs(), inst->getSrcs().begin())
    );
    return nullptr;
  // TODO !(X | non_zero) --> 0
  default: (void)op;
  }
  return nullptr;
}

#define SIMPLIFY_CONST(OP) do {                                         \
  /* don't canonicalize to the right, OP might not be commutative */    \
  if (src1->isConst() && src2->isConst()) {                             \
    if (src1->type().isNull()) {                                        \
      /* Null op Null */                                                \
      if (src2->type().isNull()) {                                      \
        return cns(int64_t(0 OP 0));                                    \
      }                                                                 \
      /* Null op ConstInt */                                            \
      if (src2->type() == Type::Int) {                                  \
        return cns(int64_t(0 OP src2->getValInt()));                    \
      }                                                                 \
      /* Null op ConstBool */                                           \
      if (src2->type() == Type::Bool) {                                 \
        return cns(int64_t(0 OP src2->getValBool()));                   \
      }                                                                 \
      /* Null op StaticStr */                                           \
      if (src2->type() == Type::StaticStr) {                            \
        const StringData* str = src2->getValStr();                      \
        if (str->isInteger()) {                                         \
          return cns(int64_t(0 OP str->toInt64()));                     \
        }                                                               \
        return cns(int64_t(0 OP 0));                                    \
      }                                                                 \
    }                                                                   \
    if (src1->type() == Type::Int) {                                    \
      /* ConstInt op Null */                                            \
      if (src2->type().isNull()) {                                      \
        return cns(int64_t(src1->getValInt()) OP 0);                    \
      }                                                                 \
      /* ConstInt op ConstInt */                                        \
      if (src2->type() == Type::Int) {                                  \
        return cns(int64_t(src1->getValInt() OP                         \
                           src2->getValInt()));                         \
      }                                                                 \
      /* ConstInt op ConstBool */                                       \
      if (src2->type() == Type::Bool) {                                 \
        return cns(int64_t(src1->getValInt() OP                         \
                         int(src2->getValBool())));                     \
      }                                                                 \
      /* ConstInt op StaticStr */                                       \
      if (src2->type() == Type::StaticStr) {                            \
        const StringData* str = src2->getValStr();                      \
        if (str->isInteger()) {                                         \
          return cns(int64_t(src1->getValInt() OP str->toInt64()));     \
        }                                                               \
        return cns(int64_t(src1->getValInt() OP 0));                    \
      }                                                                 \
    }                                                                   \
    if (src1->type() == Type::Bool) {                                   \
      /* ConstBool op Null */                                           \
      if (src2->type().isNull()) {                                      \
        return cns(int64_t(src1->getValBool() OP 0));                   \
      }                                                                 \
      /* ConstBool op ConstInt */                                       \
      if (src2->type() == Type::Int) {                                  \
        return cns(int64_t(int(src1->getValBool()) OP                   \
                         src2->getValInt()));                           \
      }                                                                 \
      /* ConstBool op ConstBool */                                      \
      if (src2->type() == Type::Bool) {                                 \
        return cns(int64_t(src1->getValBool() OP                        \
                         src2->getValBool()));                          \
      }                                                                 \
      /* ConstBool op StaticStr */                                      \
      if (src2->type() == Type::StaticStr) {                            \
        const StringData* str = src2->getValStr();                      \
        if (str->isInteger()) {                                         \
          return cns(int64_t(int(src1->getValBool()) OP str->toInt64())); \
        }                                                               \
        return cns(int64_t(int(src1->getValBool()) OP 0));              \
      }                                                                 \
    }                                                                   \
    if (src1->type() == Type::StaticStr) {                              \
      const StringData* str = src1->getValStr();                        \
      int64_t strInt = 0;                                               \
      if (str->isInteger()) {                                           \
        strInt = str->toInt64();                                        \
      }                                                                 \
      /* StaticStr op Null */                                           \
      if (src2->type().isNull()) {                                      \
        return cns(int64_t(strInt OP 0));                               \
      }                                                                 \
      /* StaticStr op ConstInt */                                       \
      if (src2->type() == Type::Int) {                                  \
        return cns(int64_t(strInt OP src2->getValInt()));               \
      }                                                                 \
      /* StaticStr op ConstBool */                                      \
      if (src2->type() == Type::Bool) {                                 \
        return cns(int64_t(strInt OP int(src2->getValBool())));         \
      }                                                                 \
      /* StaticStr op StaticStr */                                      \
      if (src2->type() == Type::StaticStr) {                            \
        const StringData* str2 = src2->getValStr();                     \
        if (str2->isInteger()) {                                        \
          return cns(int64_t(strInt OP str2->toInt64()));               \
        }                                                               \
        return cns(int64_t(strInt OP 0));                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
} while (0)

#define SIMPLIFY_COMMUTATIVE(OP, NAME) do {                             \
  SIMPLIFY_CONST(OP);                                                   \
  if (src1->isConst() && !src2->isConst()) {                            \
    return gen(Op##NAME, src2, src1);                                   \
  }                                                                     \
  if (src1->isA(Type::Int) && src2->isA(Type::Int)) {                   \
    IRInstruction* inst1 = src1->inst();                                \
    IRInstruction* inst2 = src2->inst();                                \
    if (inst1->op() == Op##NAME && inst1->getSrc(1)->isConst()) {       \
      /* (X + C1) + C2 --> X + C3 */                                    \
      if (src2->isConst()) {                                            \
        int64_t right = inst1->getSrc(1)->getValInt();                  \
        right OP##= src2->getValInt();                                  \
        return gen(Op##NAME, inst1->getSrc(0), cns(right));             \
      }                                                                 \
      /* (X + C1) + (Y + C2) --> X + Y + C3 */                          \
      if (inst2->op() == Op##NAME && inst2->getSrc(1)->isConst()) {     \
        int64_t right = inst1->getSrc(1)->getValInt();                  \
        right OP##= inst2->getSrc(1)->getValInt();                      \
        SSATmp* left = gen(Op##NAME, inst1->getSrc(0), inst2->getSrc(0)); \
        return gen(Op##NAME, left, cns(right));                         \
      }                                                                 \
    }                                                                   \
  }                                                                     \
} while (0)

#define SIMPLIFY_DISTRIBUTIVE(OUTOP, INOP, OUTNAME, INNAME) do {        \
  /* assumes that OUTOP is commutative, don't use with subtract! */     \
  SIMPLIFY_COMMUTATIVE(OUTOP, OUTNAME);                                 \
  IRInstruction* inst1 = src1->inst();                                  \
  IRInstruction* inst2 = src2->inst();                                  \
  Opcode op1 = inst1->op();                                             \
  Opcode op2 = inst2->op();                                             \
  /* all combinations of X * Y + X * Z --> X * (Y + Z) */               \
  if (op1 == Op##INNAME && op2 == Op##INNAME) {                         \
    if (inst1->getSrc(0) == inst2->getSrc(0)) {                         \
      SSATmp* fold = gen(Op##OUTNAME, inst1->getSrc(1), inst2->getSrc(1)); \
      return gen(Op##INNAME, inst1->getSrc(0), fold);                   \
    }                                                                   \
    if (inst1->getSrc(0) == inst2->getSrc(1)) {                         \
      SSATmp* fold = gen(Op##OUTNAME, inst1->getSrc(1), inst2->getSrc(0)); \
      return gen(Op##INNAME, inst1->getSrc(0), fold);                   \
    }                                                                   \
    if (inst1->getSrc(1) == inst2->getSrc(0)) {                         \
      SSATmp* fold = gen(Op##OUTNAME, inst1->getSrc(0), inst2->getSrc(1)); \
      return gen(Op##INNAME, inst1->getSrc(1), fold);                   \
    }                                                                   \
    if (inst1->getSrc(1) == inst2->getSrc(1)) {                         \
      SSATmp* fold = gen(Op##OUTNAME, inst1->getSrc(0), inst2->getSrc(0)); \
      return gen(Op##INNAME, inst1->getSrc(1), fold);                   \
    }                                                                   \
  }                                                                     \
} while (0)

SSATmp* Simplifier::simplifyAdd(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(+, *, Add, Mul);
  if (src2->isConst() && src2->type() == Type::Int) {
    int64_t src2Val = src2->getValInt();
    // X + 0 --> X
    if (src2Val == 0) {
      if (src1->type() == Type::Bool) {
        return gen(ConvBoolToInt, src1);
      }
      return src1;
    }
    // X + -C --> X - C
    if (src2Val < 0) {
      return gen(OpSub, src1, cns(-src2Val));
    }
  }
  // X + (0 - Y) --> X - Y
  IRInstruction* inst2 = src2->inst();
  Opcode op2 = inst2->op();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->type() == Type::Int) {
      if (src->getValInt() == 0) {
        return gen(OpSub, src1, inst2->getSrc(1));
      }
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifySub(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_CONST(-);
  // X - X --> 0
  if (src1 == src2) {
    return cns(0);
  }
  if (src2->isConst() && src2->type() == Type::Int) {
    int64_t src2Val = src2->getValInt();
    // X - 0 --> X
    if (src2Val == 0) {
      if (src1->type() == Type::Bool) {
        return gen(ConvBoolToInt, src1);
      }
      return src1;
    }
    // X - -C --> X + C
    if (src2Val < 0 && src2Val > std::numeric_limits<int64_t>::min()) {
      return gen(OpAdd, src1, cns(-src2Val));
    }
  }
  // X - (0 - Y) --> X + Y
  IRInstruction* inst2 = src2->inst();
  Opcode op2 = inst2->op();
  if (op2 == OpSub) {
    SSATmp* src = inst2->getSrc(0);
    if (src->isConst() && src->type() == Type::Int) {
      if (src->getValInt() == 0) {
        return gen(OpAdd, src1, inst2->getSrc(1));
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
  if (src2->isConst() && src2->type() == Type::Int) {
    // X * (-1) --> -X
    if (src2->getValInt() == -1) {
      return gen(OpSub, cns(0), src1);
    }
    // X * 0 --> 0
    if (src2->getValInt() == 0) {
      return cns(0);
    }
    // X * 1 --> X
    if (src2->getValInt() == 1) {
      if (src1->type() == Type::Bool) {
        return gen(ConvBoolToInt, src1);
      }
      return src1;
    }
    // X * 2 --> X + X
    if (src2->getValInt() == 2) {
      return gen(OpAdd, src1, src1);
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
  if (src2->isConst() && src2->type() == Type::Int) {
    // X & 0 --> 0
    if (src2->getValInt() == 0) {
      return cns(0);
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
  if (src2->isConst() && src2->type() == Type::Int) {
    // X | 0 --> X
    if (src2->getValInt() == 0) {
      return src1;
    }
    // X | (~0) --> ~0
    if (src2->getValInt() == ~uint64_t(0)) {
      return cns(~uint64_t(0));
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyXor(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(^, Xor);
  // X ^ X --> 0
  if (src1 == src2)
    return cns(0);
  // X ^ 0 --> X
  if (src2->isConst() && src2->type() == Type::Int) {
    if (src2->getValInt() == 0) {
      return src1;
    }
  }
  // Bool(X) ^ 1    --> Int(!X)
  // Bool(X) ^ true --> Int(!X)
  if (src1->isA(Type::Bool) && src2->isConst() &&
      ((src2->isA(Type::Int) && src2->getValInt() == 1) ||
       (src2->isA(Type::Bool) && src2->getValBool() == true))) {
    return gen(ConvBoolToInt, gen(OpNot, src1));
  }
  return nullptr;
}

static SSATmp* chaseIncRefs(SSATmp* tmp) {
  while (tmp->inst()->op() == IncRef) {
    tmp = tmp->inst()->getSrc(0);
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
      src1->type() != Type::Dbl) {
    // (val1 == val1) does not simplify to true when val1 is a NaN
    return cns(bool(cmpOp(opName, 0, 0)));
  }

  // need both types to be unboxed and known to simplify
  if (!src1->type().notBoxed() || src1->type() == Type::Cell ||
      !src2->type().notBoxed() || src2->type() == Type::Cell) {
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // OpSame and OpNSame have some special rules
  // ---------------------------------------------------------------------

  if (opName == OpSame || opName == OpNSame) {
    // OpSame and OpNSame do not perform type juggling
    if (src1->type() != src2->type()) {
      if (!(src1->type().isString() && src2->type().isString())) {
        return cns(opName == OpNSame);
      }
    }

    // src1 and src2 are same type, treating Str and StaticStr as the same

    // OpSame and OpNSame have special rules for string and object
    // Other types may simplify to OpEq and OpNeq, respectively
    if (src1->type().isString() && src2->type().isString()) {
      if (src1->isConst() && src2->isConst()) {
        auto str1 = src1->getValStr();
        auto str2 = src2->getValStr();
        bool same = str1->same(str2);
        return cns(bool(cmpOp(opName, same, 1)));
      } else {
        return nullptr;
      }
    }
    if (src1->type() == Type::Obj && src2->type() == Type::Obj) {
      return nullptr;
    }
    // for arrays, don't simplify Same to Eq
    if (src1->type() == Type::Arr && src2->type() == Type::Arr) {
      return nullptr;
    }
    // Type is neither a string nor an object - simplify to OpEq/OpNeq
    if (opName == OpSame) {
      return gen(OpEq, src1, src2);
    }
    return gen(OpNeq, src1, src2);
  }

  // ---------------------------------------------------------------------
  // We may now perform constant-constant optimizations
  // ---------------------------------------------------------------------

  // Null cmp Null
  if (src1->type().isNull() && src2->type().isNull()) {
    return cns(bool(cmpOp(opName, 0, 0)));
  }
  // const cmp const
  // TODO this list is incomplete - feel free to add more
  // TODO: can simplify const arrays when sizes are different or both 0
  if (src1->isConst() && src2->isConst()) {
    // StaticStr cmp StaticStr
    if (src1->type() == Type::StaticStr &&
        src2->type() == Type::StaticStr) {
      int cmp = src1->getValStr()->compare(src2->getValStr());
      return cns(bool(cmpOp(opName, cmp, 0)));
    }
    // ConstInt cmp ConstInt
    if (src1->type() == Type::Int && src2->type() == Type::Int) {
      return cns(bool(
        cmpOp(opName, src1->getValInt(), src2->getValInt())));
    }
    // ConstBool cmp ConstBool
    if (src1->type() == Type::Bool && src2->type() == Type::Bool) {
      return cns(bool(
        cmpOp(opName, src1->getValBool(), src2->getValBool())));
    }
  }

  // ---------------------------------------------------------------------
  // Constant bool comparisons can be strength-reduced
  // NOTE: Comparisons with bools get juggled to bool.
  // ---------------------------------------------------------------------

  // Perform constant-bool optimizations
  if (src2->type() == Type::Bool && src2->isConst()) {
    bool b = src2->getValBool();

    // The result of the comparison might be independent of the truth
    // value of the LHS. If so, then simplify.
    // E.g. `some-int > true`. some-int may juggle to false or true
    //  (0 or 1), but `0 > true` and `1 > true` are both false, so we can
    //  simplify to false immediately.
    if (cmpOp(opName, false, b) == cmpOp(opName, true, b)) {
      return cns(bool(cmpOp(opName, false, b)));
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
        return gen(OpEq, src1, cns(false));
      } else {
        return gen(OpEq, src1, cns(true));
      }
    }
  }

  // ---------------------------------------------------------------------
  // For same-type cmps, canonicalize any constants to the right
  // Then stop - there are no more simplifications left
  // ---------------------------------------------------------------------

  if (src1->type() == src2->type() ||
      (src1->type().isString() && src2->type().isString())) {
    if (src1->isConst() && !src2->isConst()) {
      return gen(commuteQueryOp(opName), src2, src1);
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // Perform type juggling and type canonicalization for different types
  // see http://www.php.net/manual/en/language.operators.comparison.php
  // ---------------------------------------------------------------------

  // nulls get canonicalized to the right
  if (src1->type().isNull()) {
    return gen(commuteQueryOp(opName), src2, src1);
  }

  // case 1: null cmp string. Convert null to ""
  if (src1->type().isString() && src2->type().isNull()) {
    return gen(opName, src1, cns(StringData::GetStaticString("")));
  }

  // case 2a: null cmp anything. Convert null to false
  if (src2->type().isNull()) {
    return gen(opName, src1, cns(false));
  }

  // bools get canonicalized to the right
  if (src1->type() == Type::Bool) {
    return gen(commuteQueryOp(opName), src2, src1);
  }

  // case 2b: bool cmp anything. Convert anything to bool
  if (src2->type() == Type::Bool) {
    if (src1->isConst()) {
      if (src1->type() == Type::Int) {
        return gen(opName, cns(bool(src1->getValInt())), src2);
      } else if (src1->type().isString()) {
        auto str = src1->getValStr();
        return gen(opName, cns(str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->type() == Type::Int && src2->isConst()) {
      // Based on the const bool optimization (above) opName should be OpEq
      always_assert(opName == OpEq);

      if (src2->getValBool()) {
        return gen(OpNeq, src1, cns(0));
      } else {
        return gen(OpEq, src1, cns(0));
      }
    }

    // Nothing fancy to do - perform juggling as normal.
    return gen(opName, gen(ConvCellToBool, src1), src2);
  }

  // From here on, we must be careful of how Type::Obj gets dealt with,
  // since Type::Obj can refer to an object or to a resource.

  // case 3: object cmp object. No juggling to do
  // same-type simplification is performed above

  // strings get canonicalized to the left
  if (src2->type().isString()) {
    return gen(commuteQueryOp(opName), src2, src1);
  }

  // ints get canonicalized to the right
  if (src1->type() == Type::Int) {
    return gen(commuteQueryOp(opName), src2, src1);
  }

  // case 4: number/string/resource cmp. Convert to number (int OR double)
  // NOTE: The following if-test only checks for some of the SRON-SRON
  //  cases (specifically, string-int). Other cases (like string-string)
  //  are dealt with earlier, while other cases (like number-resource)
  //  are not caught at all (and end up exiting this macro at the bottom).
  if (src1->type().isString() && src1->isConst() &&
      src2->type() == Type::Int) {
    auto str = src1->getValStr();
    int64_t si; double sd;
    auto st = str->isNumericWithVal(si, sd, true /* allow errors */);
    if (st == KindOfDouble) {
      return gen(opName, cns(sd), src2);
    }
    if (st == KindOfNull) {
      si = 0;
    }
    return gen(opName, cns(si), src2);
  }

  // case 5: array cmp array. No juggling to do
  // same-type simplification is performed above

  // case 6: array cmp anything. Array is greater
  if (src1->isArray()) {
    return cns(bool(cmpOp(opName, 1, 0)));
  }
  if (src2->isArray()) {
    return cns(bool(cmpOp(opName, 0, 1)));
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
    return gen(Jmp_, inst->getTaken());
  } else {
    // Not taken jump; turn jump into a nop
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyIsType(IRInstruction* inst) {
  bool trueSense =
    inst->op() == IsType || inst->op() == JmpIsType;
  auto    type = inst->getTypeParam();
  auto    src  = inst->getSrc(0);
  auto srcType = src->type();

  // The comparisons below won't work for these cases covered by this
  // assert, and we currently don't generate these types.
  assert(type.isKnownUnboxedDataType() && type != Type::StaticStr);
  if (type != Type::Obj) {
    if (srcType.subtypeOf(type) || (type.isString() && srcType.isString())) {
      return cns(trueSense);
    }
    if (srcType != Type::Cell) {
      return cns(!trueSense);
    }
  }
  if (srcType != Type::Obj) {
    // Note: for IsObject*, we need to emit a call to ObjectData::isResource
    // (or equivalent), so we can't fold away the case where we know we are
    // checking an object.
    return cns(!trueSense);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConcat(SSATmp* src1, SSATmp* src2) {
  if (src1->isConst() && src1->type() == Type::StaticStr &&
      src2->isConst() && src2->type() == Type::StaticStr) {
    StringData* str1 = const_cast<StringData *>(src1->getValStr());
    StringData* str2 = const_cast<StringData *>(src2->getValStr());
    StringData* merge = StringData::GetStaticString(concat_ss(str1, str2));
    return cns(merge);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvToArr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    Array arr = Array::Create(src->getValVariant());
    return cns(ArrayData::GetScalarArray(arr.get()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToBool(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(false);
    }
    return cns(true);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToBool(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(bool(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToBool(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(bool(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToBool(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    // only the strings "", and "0" convert to false, all other strings
    // are converted to true
    const StringData* str = src->getValStr();
    return cns(!str->empty() && !str->isZero());
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(0.0);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    return cns(double(src->getValBool()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    return cns(double(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  if (src->isConst()) {
    const StringData *str = src->getValStr();
    int64_t lval;
    double dval;
    DataType ret = str->isNumericWithVal(lval, dval, 1);
    if (ret == KindOfInt64) {
      dval = (double)lval;
    } else if (ret != KindOfDouble) {
      dval = 0.0;
    }
    return cns(dval);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToInt(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(0);
    }
    return cns(1);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToInt(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(int(src->getValBool()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToInt(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(toInt64(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToInt(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    const StringData *str = src->getValStr();
    int64_t lval;
    double dval;
    DataType ret = str->isNumericWithVal(lval, dval, 1);
    if (ret == KindOfDouble) {
      lval = (int64_t)dval;
    } else if (ret != KindOfInt64) {
      lval = 0;
    }
    return cns(lval);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToStr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    if (src->getValBool()) {
      return cns(StringData::GetStaticString("1"));
    }
    return cns(StringData::GetStaticString(""));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToStr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(
      StringData::convert_double_helper(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToStr(IRInstruction* inst) {
  SSATmp* src  = inst->getSrc(0);
  if (src->isConst()) {
    return cns(
      StringData::convert_integer_helper(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvCellToBool(IRInstruction* inst) {
  auto const src     = inst->getSrc(0);
  auto const srcType = src->type();

  if (srcType.isBool())   return src;
  if (srcType.isNull())   return cns(false);
  if (srcType.isArray())  return gen(ConvArrToBool, src);
  if (srcType.isDbl())    return gen(ConvDblToBool, src);
  if (srcType.isInt())    return gen(ConvIntToBool, src);
  if (srcType.isString()) return gen(ConvStrToBool, src);
  if (srcType.isObj())    return cns(true);

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
    IRInstruction* clsInst = cls->inst();
    if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
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

  return gen(LdClsPropAddrCached,
                   inst->getTaken(),
                   cls,
                   propName,
                   cns(clsNameString),
                   inst->getSrc(2));
}

/*
 * If we're in an inlined frame, use the this that we put in the
 * inlined ActRec.  (This could chase more intervening SpillStack
 * instructions to find the SpillFrame, but for now we don't inline
 * calls that will have that.)
 */
SSATmp* Simplifier::simplifyLdThis(IRInstruction* inst) {
  auto fpInst = inst->getSrc(0)->inst();
  if (fpInst->op() == DefInlineFP) {
    auto spInst = fpInst->getSrc(0)->inst();
    if (spInst->op() == SpillFrame &&
        spInst->getSrc(3)->isA(Type::Obj)) {
      return spInst->getSrc(3);
    }
    return nullptr;
  }

  return nullptr;
}

SSATmp* Simplifier::simplifyUnbox(IRInstruction* inst) {
  auto* src = inst->getSrc(0);
  auto type = outputType(inst);

  Type srcType = src->type();
  if (srcType.notBoxed()) {
    assert(type.equals(srcType));
    return src;
  }
  if (srcType.isBoxed()) {
    srcType = srcType.innerType();
    assert(type.equals(srcType));
    return gen(LdRef, type, inst->getTaken()->getTrace(), src);
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
  Type srcType = inst->getSrc(0)->type();
  srcType = inst->op() == CheckInitMem ? srcType.deref() : srcType;
  assert(srcType.notPtr());
  assert(inst->getTaken());
  if (srcType.isInit()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyPrint(IRInstruction* inst) {
  if (inst->getSrc(0)->type().isNull()) {
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

SSATmp* Simplifier::simplifyCondJmp(IRInstruction* inst) {
  SSATmp* const src            = inst->getSrc(0);
  IRInstruction* const srcInst = src->inst();
  const Opcode srcOpcode       = srcInst->op();

  // After other simplifications below (isConvIntOrPtrToBool), we can
  // end up with a non-Bool input.  Nothing more to do in this case.
  if (src->type() != Type::Bool) {
    return nullptr;
  }

  // Constant propagate.
  if (src->isConst()) {
    bool val = src->getValBool();
    if (inst->op() == JmpZero) {
      val = !val;
    }
    if (val) {
      return gen(Jmp_, inst->getTaken());
    }
    inst->convertToNop();
    return nullptr;
  }

  // Pull negations into the jump.
  if (src->inst()->op() == OpNot) {
    return gen(inst->op() == JmpZero ? JmpNZero : JmpZero,
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
    return gen(inst->op(), inst->getTaken(), srcInst->getSrc(0));
  }

  // Fuse jumps with query operators.
  if (isQueryOp(srcOpcode)) {
    SrcRange ssas = srcInst->getSrcs();
    return gen(
      queryToJmpOp(
        inst->op() == JmpZero
          ? negateQueryOp(srcOpcode)
          : srcOpcode),
        srcInst->getTypeParam(), // if it had a type param
        inst->getTaken(),
        std::make_pair(ssas.size(), ssas.begin())
    );
  }

  return nullptr;
}

SSATmp* Simplifier::simplifyCastStk(IRInstruction* inst) {
  auto const info = getStackValue(inst->getSrc(0),
                                  inst->getExtra<CastStk>()->offset);
  if (info.knownType.subtypeOf(inst->getTypeParam())) {
    // No need to cast---the type was as good or better.
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyAssertStk(IRInstruction* inst) {
  auto const info = getStackValue(inst->getSrc(0),
                                  inst->getExtra<AssertStk>()->offset);

  // AssertStk indicated that we knew the type from static analysis,
  // so this assert just double checks.
  if (info.value) assert(info.value->isA(inst->getTypeParam()));

  if (info.knownType.subtypeOf(inst->getTypeParam())) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdStack(IRInstruction* inst) {
  auto const info = getStackValue(inst->getSrc(0),
                                  inst->getExtra<LdStack>()->offset);

  // We don't want to extend live ranges of tmps across calls, so we
  // don't get the value if spansCall is true; however, we can use
  // any type information known.
  if (info.value && (!info.spansCall ||
                      info.value->inst()->op() == DefConst)) {
    return info.value;
  }
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->getTypeParam(), info.knownType)
    );
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRefLoc(IRInstruction* inst) {
  if (inst->getTypeParam().notCounted()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdLoc(IRInstruction* inst) {
  if (inst->getTypeParam().isNull()) {
    return cns(inst->getTypeParam());
  }
  return nullptr;
}

// Replace StRef with StRefNT when we know we aren't going to change
// its m_type field.
SSATmp* Simplifier::simplifyStRef(IRInstruction* inst) {
  auto const oldUnbox = inst->getSrc(0)->type().unbox();
  auto const newType = inst->getSrc(1)->type();
  if (oldUnbox.isKnownDataType() &&
      oldUnbox.equals(newType) && !oldUnbox.isString()) {
    inst->setOpcode(StRefNT);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdStackAddr(IRInstruction* inst) {
  auto const info = getStackValue(inst->getSrc(0),
                                  inst->getExtra<StackOffset>()->offset);
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->getTypeParam(), info.knownType.ptr())
    );
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRefStack(IRInstruction* inst) {
  auto const info = getStackValue(inst->getSrc(0),
                                  inst->getExtra<StackOffset>()->offset);
  if (info.value && !info.spansCall) {
    return gen(DecRef, info.knownType, info.value);
  }
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->getTypeParam(), info.knownType)
    );
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}}}
