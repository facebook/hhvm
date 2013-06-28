/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/simplifier.h"

#include <sstream>
#include <type_traits>

#include "hphp/runtime/base/memory/smart_containers.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/vm/jit/tracebuilder.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

StackValueInfo getStackValue(SSATmp* sp, uint32_t index) {
  assert(sp->isA(Type::StkPtr));
  IRInstruction* inst = sp->inst();

  switch (inst->op()) {
  case DefSP:
    return StackValueInfo();

  case ReDefGeneratorSP:
  case StashGeneratorSP:
    return getStackValue(inst->src(0), index);

  case ReDefSP:
    return getStackValue(inst->src(1), index);

  case ExceptionBarrier:
    return getStackValue(inst->src(0), index);

  case SideExitGuardStk:
    always_assert(0 && "simplifier is not tested for running after jumpopts");

  case AssertStk:
    // fallthrough
  case CastStk:
    // fallthrough
  case CoerceStk:
    // fallthrough
  case CheckStk:
    // fallthrough
  case GuardStk:
    // We don't have a value, but we may know the type due to guarding
    // on it.
    if (inst->extra<StackOffset>()->offset == index) {
      return StackValueInfo { inst->typeParam() };
    }
    return getStackValue(inst->src(0), index);

  case AssertStkVal:
    if (inst->extra<StackOffset>()->offset == index) {
      return StackValueInfo { inst->src(1) };
    }
    return getStackValue(inst->src(0), index);

  case CallArray: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { nullptr };
    }
    auto info =
      getStackValue(inst->src(0),
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
      getStackValue(inst->src(0),
                    index -
                     (1 /* pushed */ - kNumActRecCells /* popped */));
    info.spansCall = true;
    return info;
  }

  case SpillStack: {
    int64_t numPushed    = 0;
    int32_t numSpillSrcs = inst->numSrcs() - 2;

    for (int i = 0; i < numSpillSrcs; ++i) {
      SSATmp* tmp = inst->src(i + 2);
      if (index == numPushed) {
        if (tmp->inst()->op() == IncRef) {
          tmp = tmp->inst()->src(0);
        }
        if (!tmp->type().equals(Type::None)) {
          return StackValueInfo { tmp };
        }
      }
      ++numPushed;
    }

    // This is not one of the values pushed onto the stack by this
    // spillstack instruction, so continue searching.
    SSATmp* prevSp = inst->src(0);
    int64_t numPopped = inst->src(1)->getValInt();
    return getStackValue(prevSp,
                         // pop values pushed by spillstack
                         index - (numPushed - numPopped));
  }

  case InterpOne: {
    SSATmp* prevSp = inst->src(1);
    auto const& extra = *inst->extra<InterpOne>();
    int64_t spAdjustment = extra.cellsPopped - extra.cellsPushed;
    Type resultType = inst->typeParam();
    if (index == 0 && !resultType.equals(Type::None)) {
      return StackValueInfo { resultType };
    }

    // If the index we're looking for is a cell pushed by the InterpOne (other
    // than top of stack), we know nothing about its type.
    if (index < extra.cellsPushed) return StackValueInfo{ nullptr };

    return getStackValue(prevSp, index + spAdjustment);
  }

  case SpillFrame:
  case CufIterSpillFrame:
    return getStackValue(inst->src(0),
                         // pushes an ActRec
                         index - kNumActRecCells);

  default:
    {
      // Assume it's a vector instruction.  This will assert in
      // vectorBaseIdx if not.
      auto const base = inst->src(vectorBaseIdx(inst));
      assert(base->inst()->op() == LdStackAddr);
      if (base->inst()->extra<LdStackAddr>()->offset == index) {
        VectorEffects ve(inst);
        assert(ve.baseTypeChanged || ve.baseValChanged);
        return StackValueInfo { ve.baseType.derefIfPtr() };
      }
      return getStackValue(base->inst()->src(0), index);
    }
  }

  // Should not get here!
  not_reached();
}

smart::vector<SSATmp*> collectStackValues(SSATmp* sp, uint32_t stackDepth) {
  smart::vector<SSATmp*> ret;
  ret.reserve(stackDepth);
  for (uint32_t i = 0; i < stackDepth; ++i) {
    auto const value = getStackValue(sp, i).value;
    if (value) {
      ret.push_back(value);
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

static void copyPropSrc(IRInstruction* inst, int index) {
  auto tmp     = inst->src(index);
  auto srcInst = tmp->inst();

  switch (srcInst->op()) {
  case Mov:
    inst->setSrc(index, srcInst->src(0));
    break;

  case IncRef:
    if (!isRefCounted(srcInst->src(0))) {
      srcInst->setOpcode(Mov);
      inst->setSrc(index, srcInst->src(0));
    }
    break;

  default:
    return;
  }
}

void copyProp(IRInstruction* inst) {
  for (uint32_t i = 0; i < inst->numSrcs(); i++) {
    copyPropSrc(inst, i);
  }
}

/*
 * Checks if property propName of class clsTmp, called from context class ctx,
 * can be accessed via the static property cache.
 * Right now, this returns true for two cases:
 *   (a) the property is accessed from within the class containing it
 *   (b) the property belongs to a persistent class and it's accessible from ctx
 */
bool canUseSPropCache(SSATmp* clsTmp,
                      const StringData* propName,
                      const Class* ctx) {
  if (propName == nullptr) return false;

  const StringData* clsName = findClassName(clsTmp);
  if (ctx) {
    const StringData* ctxName = ctx->preClass()->name();;
    if (clsName && ctxName && clsName->isame(ctxName)) return true;
  }

  if (!clsTmp->isConst()) return false;

  const Class* cls = clsTmp->getValClass();

  if (!Transl::TargetCache::classIsPersistent(cls)) return false;

  // If the class requires initialization, it might not have been
  // initialized yet.  getSProp() below will trigger initialization,
  // but that's only valid to do earlier if it doesn't require any
  // property initializer ([sp]init methods).
  if (cls->hasInitMethods()) return false;

  bool visible, accessible;
  cls->getSProp(const_cast<Class*>(ctx), propName, visible, accessible);

  return visible && accessible;
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
  SSATmp* src1 = inst->src(0);
  SSATmp* src2 = inst->src(1);

  Opcode opc = inst->op();
  switch (opc) {
  case OpAdd:       return simplifyAdd(src1, src2);
  case OpSub:       return simplifySub(src1, src2);
  case OpMul:       return simplifyMul(src1, src2);
  case OpBitAnd:    return simplifyBitAnd(src1, src2);
  case OpBitOr:     return simplifyBitOr(src1, src2);
  case OpBitXor:    return simplifyBitXor(src1, src2);
  case OpLogicXor:  return simplifyLogicXor(src1, src2);

  case OpGt:
  case OpGte:
  case OpLt:
  case OpLte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
    return simplifyCmp(opc, inst, src1, src2);

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
  case ConvCellToInt: return simplifyConvCellToInt(inst);
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
  case CheckType:
  case AssertType:   return simplifyCheckType(inst);
  case CheckStk:     return simplifyCheckStk(inst);
  case AssertNonNull:return simplifyAssertNonNull(inst);

  case LdCls:        return simplifyLdCls(inst);
  case LdThis:       return simplifyLdThis(inst);
  case LdCtx:        return simplifyLdCtx(inst);
  case LdClsCtx:     return simplifyLdClsCtx(inst);
  case GetCtxFwdCall:return simplifyGetCtxFwdCall(inst);

  case SpillStack:   return simplifySpillStack(inst);
  case Call:         return simplifyCall(inst);
  case CastStk:      return simplifyCastStk(inst);
  case CoerceStk:    return simplifyCoerceStk(inst);
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

SSATmp* Simplifier::simplifySpillStack(IRInstruction* inst) {
  auto const sp           = inst->src(0);
  auto const spDeficit    = inst->src(1)->getValInt();
  auto       spillVals    = inst->srcs().subpiece(2);
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
    if (srcInst->op() == LdStack && srcInst->src(0) == sp &&
        srcInst->extra<LdStack>()->offset == offset) {
      spillVals[i] = m_tb->genDefNone();
    }
    cellOff++;
  }

  // Note: although the instruction might have been modified above, we still
  // need to return nullptr so that it gets cloned later if it's stack-allocated
  return nullptr;
}

SSATmp* Simplifier::simplifyCall(IRInstruction* inst) {
  auto spillVals  = inst->srcs().subpiece(3);
  auto const spillStack = inst->src(0)->inst();
  if (spillStack->op() != SpillStack) {
    return nullptr;
  }

  SSATmp* sp = spillStack->src(0);
  int baseOffset = spillStack->src(1)->getValInt() -
                   spillValueCells(spillStack);
  auto const numSpillSrcs = spillVals.size();
  for (int32_t i = 0; i < numSpillSrcs; i++) {
    const int64_t offset = -(i + 1) + baseOffset;
    assert(spillVals[i]->type() != Type::ActRec);
    IRInstruction* srcInst = spillVals[i]->inst();
    // If our value came from a LdStack on the same sp and offset,
    // we don't need to spill it.
    if (srcInst->op() == LdStack && srcInst->src(0) == sp &&
        srcInst->extra<LdStack>()->offset == offset) {
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
  auto const frameInst = inst->src(0)->inst();
  if (frameInst->op() == DefInlineFP) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdCtx(IRInstruction* inst) {
  const Func* func = inst->src(1)->getValFunc();
  if (func->isStatic()) {
    // ActRec->m_cls of a static function is always a valid class pointer with
    // the bottom bit set
    return gen(LdCctx, inst->src(0));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdClsCtx(IRInstruction* inst) {
  SSATmp*  ctx = inst->src(0);
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
  SSATmp*  srcCtx = inst->src(0);
  if (srcCtx->isA(Type::Cctx)) {
    return srcCtx;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdCls(IRInstruction* inst) {
  SSATmp* clsName = inst->src(0);
  if (clsName->isConst()) {
    const Class* cls = Unit::lookupClass(clsName->getValStr());
    if (cls) {
      if (Transl::TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
        // the class is always defined
        return cns(cls);
      }
      const Class* ctx = inst->src(1)->getValClass();
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

SSATmp* Simplifier::simplifyCheckType(IRInstruction* inst) {
  Type type    = inst->typeParam();
  SSATmp* src  = inst->src(0);
  Type srcType = src->type();

  if (srcType.subtypeOf(type)) {
    /*
     * The type of the src is the same or more refined than type, so the
     * guard is unnecessary.
     */
    return src;
  }
  if (type.strictSubtypeOf(srcType)) {
    return nullptr;
  }

  if (type.equals(Type::Str) && srcType.maybe(Type::Str)) {
    /*
     * If we're guarding against Str and srcType has StaticStr or CountedStr
     * in it, refine the output type. This can happen when we have a
     * KindOfString guard from Translator but internally we know a more
     * specific subtype of Str.
     */
    FTRACE(1, "CheckType: refining {} to {}\n", srcType.toString(),
           type.toString());
    inst->setTypeParam(type & srcType);
    return nullptr;
  }

  /*
   * We got a predicted type that is wrong -- it's incompatible with
   * the tracked type.  So throw the prediction away, since it would
   * always fail.
   */
  FTRACE(1, "WARNING: CheckType: removed incorrect prediction that {} is {}\n",
         srcType.toString(), type.toString());
  return src;
}

SSATmp* Simplifier::simplifyCheckStk(IRInstruction* inst) {
  auto type = inst->typeParam();
  auto sp = inst->src(0);
  auto offset = inst->extra<CheckStk>()->offset;

  auto stkVal = getStackValue(sp, offset);
  if (stkVal.knownType.equals(Type::None)) return nullptr;

  if (stkVal.knownType.subtypeOf(type)) {
    return sp;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyQueryJmp(IRInstruction* inst) {
  SSATmp* src1 = inst->src(0);
  SSATmp* src2 = inst->src(1);
  Opcode opc = inst->op();
  // reuse the logic in simplifyCmp.
  SSATmp* newCmp = simplifyCmp(queryJmpToQueryOp(opc), nullptr, src1, src2);
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
    inst->taken(),
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
    return inst->src(0);

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
    if (!inst->src(0)->isA(Type::Dbl) &&
        !inst->src(1)->isA(Type::Dbl)) {
      return gen(negateQueryOp(op), inst->src(0), inst->src(1));
    }
    break;

  case InstanceOfBitmask:
  case NInstanceOfBitmask:
    // TODO: combine this with the above check and use isQueryOp or
    // add an isNegatable.
    return gen(
      negateQueryOp(op),
      std::make_pair(inst->numSrcs(), inst->srcs().begin())
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
    if (inst1->op() == Op##NAME && inst1->src(1)->isConst()) {          \
      /* (X + C1) + C2 --> X + C3 */                                    \
      if (src2->isConst()) {                                            \
        int64_t right = inst1->src(1)->getValInt();                     \
        right OP##= src2->getValInt();                                  \
        return gen(Op##NAME, inst1->src(0), cns(right));                \
      }                                                                 \
      /* (X + C1) + (Y + C2) --> X + Y + C3 */                          \
      if (inst2->op() == Op##NAME && inst2->src(1)->isConst()) {        \
        int64_t right = inst1->src(1)->getValInt();                     \
        right OP##= inst2->src(1)->getValInt();                         \
        SSATmp* left = gen(Op##NAME, inst1->src(0), inst2->src(0));     \
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
    if (inst1->src(0) == inst2->src(0)) {                               \
      SSATmp* fold = gen(Op##OUTNAME, inst1->src(1), inst2->src(1));    \
      return gen(Op##INNAME, inst1->src(0), fold);                      \
    }                                                                   \
    if (inst1->src(0) == inst2->src(1)) {                               \
      SSATmp* fold = gen(Op##OUTNAME, inst1->src(1), inst2->src(0));    \
      return gen(Op##INNAME, inst1->src(0), fold);                      \
    }                                                                   \
    if (inst1->src(1) == inst2->src(0)) {                               \
      SSATmp* fold = gen(Op##OUTNAME, inst1->src(0), inst2->src(1));    \
      return gen(Op##INNAME, inst1->src(1), fold);                      \
    }                                                                   \
    if (inst1->src(1) == inst2->src(1)) {                               \
      SSATmp* fold = gen(Op##OUTNAME, inst1->src(0), inst2->src(0));    \
      return gen(Op##INNAME, inst1->src(1), fold);                      \
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
    SSATmp* src = inst2->src(0);
    if (src->isConst() && src->type() == Type::Int) {
      if (src->getValInt() == 0) {
        return gen(OpSub, src1, inst2->src(1));
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
    SSATmp* src = inst2->src(0);
    if (src->isConst() && src->type() == Type::Int) {
      if (src->getValInt() == 0) {
        return gen(OpAdd, src1, inst2->src(1));
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

SSATmp* Simplifier::simplifyBitAnd(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(&, |, BitAnd, BitOr);
  // X & X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst()) {
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

SSATmp* Simplifier::simplifyBitOr(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_DISTRIBUTIVE(|, &, BitOr, BitAnd);
  // X | X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->isConst()) {
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

SSATmp* Simplifier::simplifyBitXor(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(^, BitXor);
  // X ^ X --> 0
  if (src1 == src2)
    return cns(0);
  // X ^ 0 --> X; X ^ -1 --> ~X
  if (src2->isConst()) {
    if (src2->getValInt() == 0) {
      return src1;
    }
    if (src2->getValInt() == -1) {
      return gen(OpBitNot, src1);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLogicXor(SSATmp* src1, SSATmp* src2) {
  SIMPLIFY_COMMUTATIVE(^, LogicXor);
  if (src1 == src2) {
    return cns(false);
  }

  // SIMPLIFY_COMMUTATIVE takes care of the both-sides-const case, and
  // canonicalizes a single const to the right
  if (src2->isConst()) {
    if (src2->getValBool()) {
      return gen(OpNot, src1);
    } else {
      return src1;
    }
  }
  return nullptr;
}

static SSATmp* chaseIncRefs(SSATmp* tmp) {
  while (tmp->inst()->op() == IncRef) {
    tmp = tmp->inst()->src(0);
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

SSATmp* Simplifier::simplifyCmp(Opcode opName, IRInstruction* inst,
                                SSATmp* src1, SSATmp* src2) {
  auto newInst = [inst, this](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };
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
      return newInst(OpEq, src1, src2);
    }
    return newInst(OpNeq, src1, src2);
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
        return newInst(OpEq, src1, cns(false));
      } else {
        return newInst(OpEq, src1, cns(true));
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
      return newInst(commuteQueryOp(opName), src2, src1);
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // Perform type juggling and type canonicalization for different types
  // see http://www.php.net/manual/en/language.operators.comparison.php
  // ---------------------------------------------------------------------

  // nulls get canonicalized to the right
  if (src1->type().isNull()) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 1: null cmp string. Convert null to ""
  if (src1->type().isString() && src2->type().isNull()) {
    return newInst(opName, src1, cns(StringData::GetStaticString("")));
  }

  // case 2a: null cmp anything. Convert null to false
  if (src2->type().isNull()) {
    return newInst(opName, src1, cns(false));
  }

  // bools get canonicalized to the right
  if (src1->type() == Type::Bool) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 2b: bool cmp anything. Convert anything to bool
  if (src2->type() == Type::Bool) {
    if (src1->isConst()) {
      if (src1->type() == Type::Int) {
        return newInst(opName, cns(bool(src1->getValInt())), src2);
      } else if (src1->type().isString()) {
        auto str = src1->getValStr();
        return newInst(opName, cns(str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->type() == Type::Int && src2->isConst()) {
      // Based on the const bool optimization (above) opName should be OpEq
      always_assert(opName == OpEq);

      if (src2->getValBool()) {
        return newInst(OpNeq, src1, cns(0));
      } else {
        return newInst(OpEq, src1, cns(0));
      }
    }

    // Nothing fancy to do - perform juggling as normal.
    return newInst(opName, gen(ConvCellToBool, src1), src2);
  }

  // From here on, we must be careful of how Type::Obj gets dealt with,
  // since Type::Obj can refer to an object or to a resource.

  // case 3: object cmp object. No juggling to do
  // same-type simplification is performed above

  // strings get canonicalized to the left
  if (src2->type().isString()) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // ints get canonicalized to the right
  if (src1->type() == Type::Int) {
    return newInst(commuteQueryOp(opName), src2, src1);
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
      return newInst(opName, cns(sd), src2);
    }
    if (st == KindOfNull) {
      si = 0;
    }
    return newInst(opName, cns(si), src2);
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
    return gen(Jmp_, inst->taken());
  } else {
    // Not taken jump; turn jump into a nop
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyIsType(IRInstruction* inst) {
  bool trueSense =
    inst->op() == IsType || inst->op() == JmpIsType;
  auto    type = inst->typeParam();
  auto    src  = inst->src(0);
  auto srcType = src->type();

  // The comparisons below won't work for these cases covered by this
  // assert, and we currently don't generate these types.
  assert(type.isKnownUnboxedDataType() && type != Type::StaticStr);

  // CountedStr and StaticStr are disjoint, but compatible for this purpose.
  if (type.isString() && srcType.isString()) {
    return cns(trueSense);
  }

  // The types are disjoint; the result must be false.
  if ((srcType & type).equals(Type::Bottom)) {
    return cns(!trueSense);
  }

  // The src type is a subtype of the tested type. You'd think the result would
  // always be true, but it's not for is_object.
  if (!type.subtypeOf(Type::Obj) && srcType.subtypeOf(type)) {
    return cns(trueSense);
  }

  // At this point, either the tested type is a subtype of the src type, or they
  // are non-disjoint but neither is a subtype of the other. (Or it's the weird
  // Obj case.) We can't simplify this away.
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
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    Array arr = Array::Create(src->getValVariant());
    return cns(ArrayData::GetScalarArray(arr.get()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToBool(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(false);
    }
    return cns(true);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToBool(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(bool(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToBool(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(bool(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToBool(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    // only the strings "", and "0" convert to false, all other strings
    // are converted to true
    const StringData* str = src->getValStr();
    return cns(!str->empty() && !str->isZero());
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvArrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(0.0);
    }
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  if (src->isConst()) {
    return cns(double(src->getValBool()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  if (src->isConst()) {
    return cns(double(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
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
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    if (src->getValArr()->empty()) {
      return cns(0);
    }
    return cns(1);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvBoolToInt(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(int(src->getValBool()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToInt(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(toInt64(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvStrToInt(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
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
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    if (src->getValBool()) {
      return cns(StringData::GetStaticString("1"));
    }
    return cns(StringData::GetStaticString(""));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvDblToStr(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(
      StringData::convert_double_helper(src->getValDbl()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvIntToStr(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  if (src->isConst()) {
    return cns(
      StringData::convert_integer_helper(src->getValInt()));
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyConvCellToBool(IRInstruction* inst) {
  auto const src     = inst->src(0);
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

SSATmp* Simplifier::simplifyConvCellToInt(IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType.isInt())    return src;
  if (srcType.isNull())   return cns(0);
  if (srcType.isArray())  return gen(ConvArrToInt, src);
  if (srcType.isBool())   return gen(ConvBoolToInt, src);
  if (srcType.isDbl())    return gen(ConvDblToInt, src);
  if (srcType.isString()) return gen(ConvStrToInt, src);
  if (srcType.isObj())    return gen(ConvObjToInt, inst->taken(), src);

  return nullptr;
}

SSATmp* Simplifier::simplifyLdClsPropAddr(IRInstruction* inst) {
  SSATmp* propName = inst->src(1);
  if (!propName->isConst()) return nullptr;

  SSATmp* cls = inst->src(0);
  auto ctxCls = inst->src(2)->getValClass();

  if (canUseSPropCache(cls, propName->getValStr(), ctxCls)) {

    const StringData* clsNameStr = findClassName(cls);

    return gen(LdClsPropAddrCached,
               inst->taken(),
               cls,
               propName,
               cns(clsNameStr),
               inst->src(2));
  }

  return nullptr;
}

/*
 * If we're in an inlined frame, use the this that we put in the
 * inlined ActRec.  (This could chase more intervening SpillStack
 * instructions to find the SpillFrame, but for now we don't inline
 * calls that will have that.)
 */
SSATmp* Simplifier::simplifyLdThis(IRInstruction* inst) {
  auto fpInst = inst->src(0)->inst();
  if (fpInst->op() == DefInlineFP) {
    auto spInst = fpInst->src(0)->inst();
    if (spInst->op() == SpillFrame &&
        spInst->src(3)->isA(Type::Obj)) {
      return spInst->src(3);
    }
    return nullptr;
  }

  return nullptr;
}

SSATmp* Simplifier::simplifyUnbox(IRInstruction* inst) {
  auto* src = inst->src(0);
  auto type = outputType(inst);

  Type srcType = src->type();
  if (srcType.notBoxed()) {
    assert(type.equals(srcType));
    return src;
  }
  if (srcType.isBoxed()) {
    srcType = srcType.innerType();
    assert(type.equals(srcType));
    return gen(LdRef, type, inst->taken()->trace(), src);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyUnboxPtr(IRInstruction* inst) {
  if (inst->src(0)->isA(Type::PtrToCell)) {
    // Nothing to unbox
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyCheckInit(IRInstruction* inst) {
  Type srcType = inst->src(0)->type();
  srcType = inst->op() == CheckInitMem ? srcType.deref() : srcType;
  assert(srcType.notPtr());
  assert(inst->taken());
  if (srcType.isInit()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyPrint(IRInstruction* inst) {
  if (inst->src(0)->type().isNull()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRef(IRInstruction* inst) {
  if (!isRefCounted(inst->src(0))) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyIncRef(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  if (!isRefCounted(src)) {
    return src;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyCondJmp(IRInstruction* inst) {
  SSATmp* const src            = inst->src(0);
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
      return gen(Jmp_, inst->taken());
    }
    inst->convertToNop();
    return nullptr;
  }

  // Pull negations into the jump.
  if (src->inst()->op() == OpNot) {
    return gen(inst->op() == JmpZero ? JmpNZero : JmpZero,
               inst->taken(),
               srcInst->src(0));
  }

  /*
   * Try to combine the src inst with the Jmp.  We can't do any
   * combinations of the src instruction with the jump if the src's
   * are refcounted, since we may have dec refs between the src
   * instruction and the jump.
   */
  for (auto& src : srcInst->srcs()) {
    if (isRefCounted(src)) return nullptr;
  }

  // If the source is conversion of an int or pointer to boolean, we
  // can test the int/ptr value directly.
  if (isConvIntOrPtrToBool(srcInst)) {
    return gen(inst->op(), inst->taken(), srcInst->src(0));
  }

  // Fuse jumps with query operators.
  if (isQueryOp(srcOpcode)) {
    SrcRange ssas = srcInst->srcs();
    return gen(
      queryToJmpOp(
        inst->op() == JmpZero
          ? negateQueryOp(srcOpcode)
          : srcOpcode),
      srcInst->typeParam(), // if it had a type param
      inst->taken(),
      std::make_pair(ssas.size(), ssas.begin())
    );
  }

  return nullptr;
}

SSATmp* Simplifier::simplifyCastStk(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<CastStk>()->offset);
  if (info.knownType.subtypeOf(inst->typeParam())) {
    // No need to cast---the type was as good or better.
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyCoerceStk(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<CoerceStk>()->offset);
  if (info.knownType.subtypeOf(inst->typeParam())) {
    // No need to cast---the type was as good or better.
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyAssertStk(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<AssertStk>()->offset);

  // AssertStk indicated that we knew the type from static analysis,
  // so this assert just double checks.
  if (info.value) assert(info.value->isA(inst->typeParam()));

  if (info.knownType.subtypeOf(inst->typeParam())) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdStack(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<LdStack>()->offset);

  // We don't want to extend live ranges of tmps across calls, so we
  // don't get the value if spansCall is true; however, we can use
  // any type information known.
  if (info.value && (!info.spansCall ||
                      info.value->inst()->op() == DefConst)) {
    return info.value;
  }
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->typeParam(), info.knownType)
    );
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRefLoc(IRInstruction* inst) {
  if (inst->typeParam().notCounted()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdLoc(IRInstruction* inst) {
  if (inst->typeParam().isNull()) {
    return cns(inst->typeParam());
  }
  return nullptr;
}

// Replace StRef with StRefNT when we know we aren't going to change
// its m_type field.
SSATmp* Simplifier::simplifyStRef(IRInstruction* inst) {
  auto const oldUnbox = inst->src(0)->type().unbox();
  auto const newType = inst->src(1)->type();
  if (oldUnbox.isKnownDataType() &&
      oldUnbox.equals(newType) && !oldUnbox.isString()) {
    inst->setOpcode(StRefNT);
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyLdStackAddr(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<StackOffset>()->offset);
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->typeParam(), info.knownType.ptr())
    );
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyDecRefStack(IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<StackOffset>()->offset);
  if (info.value && !info.spansCall) {
    inst->convertToNop();
    return gen(DecRef, info.value);
  }
  if (!info.knownType.equals(Type::None)) {
    inst->setTypeParam(
      Type::mostRefined(inst->typeParam(), info.knownType)
    );
  }
  if (inst->typeParam().notCounted()) {
    inst->convertToNop();
    return nullptr;
  }
  return nullptr;
}

SSATmp* Simplifier::simplifyAssertNonNull(IRInstruction* inst) {
  auto t = inst->typeParam();
  assert(t.maybe(Type::Nullptr));
  if (t.subtypeOf(Type::Nullptr)) {
    return inst->src(0);
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}}
