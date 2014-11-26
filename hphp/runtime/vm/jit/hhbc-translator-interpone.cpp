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

#include <cstdlib>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

Type arithOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return Type::Cell;
  }

  auto both = t1 | t2;
  if (both.maybe(Type::Dbl)) return Type::Dbl;
  if (both.maybe(Type::Arr)) return Type::Arr;
  if (both.maybe(Type::Str)) return Type::Cell;
  return Type::Int;
}

Type arithOpOverResult(Type t1, Type t2) {
  if (t1 <= Type::Int && t2 <= Type::Int) {
    return Type::Int | Type::Dbl;
  }
  return arithOpResult(t1, t2);
}

Type bitOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return Type::Cell;
  }

  auto both = t1 | t2;
  if (both <= Type::Str) return Type::Str;
  return Type::Int;
}

Type setOpResult(Type locType, Type valType, SetOpOp op) {
  switch (op) {
  case SetOpOp::PlusEqual:
  case SetOpOp::MinusEqual:
  case SetOpOp::MulEqual:    return arithOpResult(locType.unbox(), valType);
  case SetOpOp::PlusEqualO:
  case SetOpOp::MinusEqualO:
  case SetOpOp::MulEqualO:   return arithOpOverResult(locType.unbox(), valType);
  case SetOpOp::ConcatEqual: return Type::Str;
  case SetOpOp::PowEqual:
  case SetOpOp::DivEqual:
  case SetOpOp::ModEqual:    return Type::UncountedInit;
  case SetOpOp::AndEqual:
  case SetOpOp::OrEqual:
  case SetOpOp::XorEqual:    return bitOpResult(locType.unbox(), valType);
  case SetOpOp::SlEqual:
  case SetOpOp::SrEqual:     return Type::Int;
  }
  not_reached();
}

uint32_t localInputId(const NormalizedInstruction& inst) {
  switch (inst.op()) {
    case OpSetWithRefLM:
    case OpFPassL:
      return inst.imm[1].u_LA;

    default:
      return inst.imm[0].u_LA;
  }
}

//////////////////////////////////////////////////////////////////////

}

folly::Optional<Type> HhbcTranslator::interpOutputType(
    const NormalizedInstruction& inst,
    folly::Optional<Type>& checkTypeType) const {
  using namespace jit::InstrFlags;
  auto localType = [&]{
    auto locId = localInputId(inst);
    static_assert(std::is_unsigned<typeof(locId)>::value,
                  "locId should be unsigned");
    assert(locId < curFunc()->numLocals());
    return m_irb->localType(locId, DataTypeSpecific);
  };

  auto boxed = [&] (Type t) -> Type {
    if (t.equals(Type::Gen)) return Type::BoxedInitCell;
    assert(t.isBoxed() || t.notBoxed());
    checkTypeType = t.isBoxed() ? t : boxType(t); // inner type is predicted
    return Type::BoxedInitCell;
  };

  if (inst.outputPredicted) return Type::Gen;

  auto outFlag = getInstrInfo(inst.op()).type;
  if (outFlag == OutFInputL) {
    outFlag = inst.preppedByRef ? OutVInputL : OutCInputL;
  } else if (outFlag == OutFInputR) {
    outFlag = inst.preppedByRef ? OutVInput : OutCInput;
  }

  switch (outFlag) {
    case OutNull:        return Type::InitNull;
    case OutNullUninit:  return Type::Uninit;
    case OutString:      return Type::Str;
    case OutStringImm:   return Type::StaticStr;
    case OutDouble:      return Type::Dbl;
    case OutIsTypeL:
    case OutBoolean:
    case OutPredBool:
    case OutBooleanImm:  return Type::Bool;
    case OutInt64:       return Type::Int;
    case OutArray:       return Type::Arr;
    case OutArrayImm:    return Type::Arr; // Should be StaticArr: t2124292
    case OutObject:
    case OutThisObject:  return Type::Obj;
    case OutResource:    return Type::Res;

    case OutFDesc:       return folly::none;
    case OutUnknown:     return Type::Gen;

    case OutPred:
      checkTypeType = inst.outPred;
      // Returning inst.outPred from this function would turn the CheckStk
      // after the InterpOne into a nop.
      return Type::Gen;

    case OutCns:         return Type::Cell;
    case OutVUnknown:    return Type::BoxedInitCell;

    case OutSameAsInput: return topType(0);
    case OutVInput:      return boxed(topType(0));
    case OutVInputL:     return boxed(localType());
    case OutFInputL:
    case OutFInputR:     not_reached();

    case OutArith:       return arithOpResult(topType(0), topType(1));
    case OutArithO:      return arithOpOverResult(topType(0), topType(1));
    case OutBitOp:
      return bitOpResult(topType(0),
                         inst.op() == HPHP::OpBitNot ? Type::Bottom
                                                     : topType(1));
    case OutSetOp:      return setOpResult(localType(), topType(0),
                                           SetOpOp(inst.imm[1].u_OA));
    case OutIncDec: {
      auto ty = localType().unbox();
      return ty <= Type::Dbl ? ty : Type::Cell;
    }
    case OutStrlen:
      return topType(0) <= Type::Str ? Type::Int : Type::UncountedInit;
    case OutClassRef:   return Type::Cls;
    case OutFPushCufSafe: return folly::none;

    case OutNone:       return folly::none;

    case OutCInput: {
      auto ttype = topType(0);
      if (ttype.notBoxed()) return ttype;
      // All instructions that are OutCInput or OutCInputL cannot push uninit or
      // a ref, so only specific inner types need to be checked.
      if (ttype.unbox().strictSubtypeOf(Type::InitCell)) {
        checkTypeType = ttype.unbox();
      }
      return Type::Cell;
    }

    case OutCInputL: {
      auto ltype = localType();
      if (ltype.notBoxed()) return ltype;
      if (ltype.unbox().strictSubtypeOf(Type::InitCell)) {
        checkTypeType = ltype.unbox();
      }
      return Type::Cell;
    }
  }
  not_reached();
}

jit::vector<InterpOneData::LocalType>
HhbcTranslator::interpOutputLocals(const NormalizedInstruction& inst,
                                   bool& smashesAllLocals,
                                   folly::Optional<Type> pushedType) {
  using namespace jit::InstrFlags;
  if (!(getInstrInfo(inst.op()).out & Local)) return {};

  jit::vector<InterpOneData::LocalType> locals;
  auto setLocType = [&](uint32_t id, Type t) {
    // Relax the type to something guardable.  For InterpOne we don't bother to
    // keep track of specialized types or inner-ref types.  (And note that for
    // psuedomains we may in fact have to guard on the local type after this.)
    locals.emplace_back(id, t.relaxToGuardable());
  };
  auto setImmLocType = [&](uint32_t id, Type t) {
    setLocType(inst.imm[id].u_LA, t);
  };
  auto* func = curFunc();

  auto handleBoxiness = [&] (Type testTy, Type useTy) {
    return testTy.isBoxed() ? Type::BoxedInitCell :
           testTy.maybeBoxed() ? Type::Gen :
           useTy;
  };

  switch (inst.op()) {
    case OpSetN:
    case OpSetOpN:
    case OpIncDecN:
    case OpBindN:
    case OpVGetN:
    case OpUnsetN:
      smashesAllLocals = true;
      break;

    case OpSetOpL:
    case OpIncDecL: {
      assert(pushedType.hasValue());
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      assert(locType < Type::Gen || inPseudoMain());

      auto stackType = inst.outputPredicted ? inst.outPred : pushedType.value();
      setImmLocType(0, handleBoxiness(locType, stackType));
      break;
    }

    case OpStaticLocInit:
      setImmLocType(0, Type::BoxedInitCell);
      break;

    case OpInitThisLoc:
      setImmLocType(0, Type::Cell);
      break;

    case OpSetL: {
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      auto stackType = topType(0);
      // SetL preserves reffiness of a local.
      setImmLocType(0, handleBoxiness(locType, stackType));
      break;
    }
    case OpVGetL:
    case OpBindL: {
      assert(pushedType.hasValue());
      assert(pushedType->isBoxed());
      setImmLocType(0, pushedType.value());
      break;
    }

    case OpUnsetL:
    case OpPushL:
      setImmLocType(0, Type::Uninit);
      break;

    case OpSetM:
    case OpSetOpM:
    case OpBindM:
    case OpVGetM:
    case OpSetWithRefLM:
    case OpSetWithRefRM:
    case OpUnsetM:
    case OpFPassM:
    case OpIncDecM:
      switch (inst.immVec.locationCode()) {
        case LL: {
          auto const& mii = getMInstrInfo(inst.mInstrOp());
          auto const& base = inst.inputs[mii.valCount()]->location;
          assert(base.space == Location::Local);

          // MInstrEffects expects to be used in the context of a normally
          // translated instruction, not an interpOne. The two important
          // differences are that the base is normally a PtrTo* and we need to
          // supply an IR opcode representing the operation. SetWithRefElem is
          // used instead of SetElem because SetElem makes a few assumptions
          // about side exits that interpOne won't do.
          auto const baseType = m_irb->localType(
            base.offset, DataTypeSpecific
          ).ptr(Ptr::Frame);
          auto const isUnset = inst.op() == OpUnsetM;
          auto const isProp = mcodeIsProp(inst.immVecM[0]);

          if (isUnset && isProp) break;
          auto op = isProp ? SetProp : isUnset ? UnsetElem : SetWithRefElem;
          MInstrEffects effects(op, baseType);
          if (effects.baseValChanged) {
            auto const ty = effects.baseType.deref();
            assert((ty.isBoxed() || ty.notBoxed()) || inPseudoMain());
            setLocType(base.offset, handleBoxiness(ty, ty));
          }
          break;
        }

        case LNL:
        case LNC:
          smashesAllLocals = true;
          break;

        default:
          break;
      }
      break;

    case OpMIterInitK:
    case OpMIterNextK:
      setImmLocType(3, Type::Cell);
    case OpMIterInit:
    case OpMIterNext:
      setImmLocType(2, Type::BoxedInitCell);
      break;

    case OpIterInitK:
    case OpWIterInitK:
    case OpIterNextK:
    case OpWIterNextK:
      setImmLocType(3, Type::Cell);
    case OpIterInit:
    case OpWIterInit:
    case OpIterNext:
    case OpWIterNext:
      setImmLocType(2, Type::Gen);
      break;

    case OpVerifyParamType: {
      auto paramId = inst.imm[0].u_LA;
      auto const& tc = func->params()[paramId].typeConstraint;
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      if (tc.isArray() && !tc.isSoft() && !func->mustBeRef(paramId) &&
          (locType <= Type::Obj || locType.maybeBoxed())) {
        setImmLocType(0, handleBoxiness(locType, Type::Cell));
      }
      break;
    }

    case OpSilence:
      if (static_cast<SilenceOp>(inst.imm[0].u_OA) == SilenceOp::Start) {
        setImmLocType(inst.imm[0].u_LA, Type::Int);
      }
      break;

    default:
      not_reached();
  }

  return locals;
}

void HhbcTranslator::emitInterpOne(const NormalizedInstruction& inst) {
  folly::Optional<Type> checkTypeType;
  auto stackType = interpOutputType(inst, checkTypeType);
  auto popped = getStackPopped(inst.pc());
  auto pushed = getStackPushed(inst.pc());
  FTRACE(1, "emitting InterpOne for {}, result = {}, popped {}, pushed {}\n",
         inst.toString(),
         stackType.hasValue() ? stackType->toString() : "<none>",
         popped, pushed);

  InterpOneData idata;
  auto locals = interpOutputLocals(inst, idata.smashesAllLocals, stackType);
  idata.nChangedLocals = locals.size();
  idata.changedLocals = locals.data();

  emitInterpOne(stackType, popped, pushed, idata);
  if (checkTypeType) {
    auto const out = getInstrInfo(inst.op()).out;
    auto const checkIdx = (out & InstrFlags::StackIns2) ? 2
                        : (out & InstrFlags::StackIns1) ? 1
                        : 0;
    checkTypeStack(checkIdx, *checkTypeType, inst.nextSk().offset());
  }
}

void HhbcTranslator::emitInterpOne(int popped) {
  InterpOneData idata;
  emitInterpOne(folly::none, popped, 0, idata);
}

void HhbcTranslator::emitInterpOne(Type outType, int popped) {
  InterpOneData idata;
  emitInterpOne(outType, popped, 1, idata);
}

void HhbcTranslator::emitInterpOne(folly::Optional<Type> outType, int popped,
                                   int pushed, InterpOneData& idata) {
  auto unit = curFunc()->unit();
  auto sp = spillStack();
  auto op = unit->getOpcode(bcOff());

  auto& iInfo = getInstrInfo(op);
  if (iInfo.type == jit::InstrFlags::OutFDesc) {
    m_fpiStack.emplace(sp, m_irb->spOffset());
  } else if (isFCallStar(op) && !m_fpiStack.empty()) {
    m_fpiStack.pop();
  }

  idata.bcOff = bcOff();
  idata.cellsPopped = popped;
  idata.cellsPushed = pushed;
  idata.opcode = op;

  auto const changesPC = opcodeChangesPC(idata.opcode);
  gen(changesPC ? InterpOneCF : InterpOne, outType,
      makeCatch(), idata, sp, m_irb->fp());
  assert(m_irb->stackDeficit() == 0);
}

//////////////////////////////////////////////////////////////////////

/*
 * Instructions that unconditionally are implemented with InterpOne are
 * translated here.
 */

#define INTERP emitInterpOne(*m_currentNormalizedInstruction);

void HhbcTranslator::emitFPushObjMethod(int32_t, ObjMethodOp) { INTERP }

void HhbcTranslator::emitLowInvalid()              { std::abort(); }
void HhbcTranslator::emitCGetL3(int32_t)           { INTERP }
void HhbcTranslator::emitBox()                     { INTERP }
void HhbcTranslator::emitBoxR()                    { INTERP }
void HhbcTranslator::emitAddElemV()                { INTERP }
void HhbcTranslator::emitAddNewElemV()             { INTERP }
void HhbcTranslator::emitClsCns(const StringData*) { INTERP }
void HhbcTranslator::emitExit()                    { INTERP }
void HhbcTranslator::emitFatal(FatalOp)            { INTERP }
void HhbcTranslator::emitUnwind()                  { INTERP }
void HhbcTranslator::emitThrow()                   { INTERP }
void HhbcTranslator::emitCGetN()                   { INTERP }
void HhbcTranslator::emitVGetN()                   { INTERP }
void HhbcTranslator::emitIssetN()                  { INTERP }
void HhbcTranslator::emitEmptyN()                  { INTERP }
void HhbcTranslator::emitSetN()                    { INTERP }
void HhbcTranslator::emitSetOpN(SetOpOp)           { INTERP }
void HhbcTranslator::emitSetOpG(SetOpOp)           { INTERP }
void HhbcTranslator::emitSetOpS(SetOpOp)           { INTERP }
void HhbcTranslator::emitIncDecN(IncDecOp)         { INTERP }
void HhbcTranslator::emitIncDecG(IncDecOp)         { INTERP }
void HhbcTranslator::emitIncDecS(IncDecOp)         { INTERP }
void HhbcTranslator::emitBindN()                   { INTERP }
void HhbcTranslator::emitUnsetN()                  { INTERP }
void HhbcTranslator::emitUnsetG()                  { INTERP }
void HhbcTranslator::emitFPassN(int32_t)           { INTERP }
void HhbcTranslator::emitFCallUnpack(int32_t)      { INTERP }
void HhbcTranslator::emitCufSafeArray()            { INTERP }
void HhbcTranslator::emitCufSafeReturn()           { INTERP }
void HhbcTranslator::emitIncl()                    { INTERP }
void HhbcTranslator::emitInclOnce()                { INTERP }
void HhbcTranslator::emitReq()                     { INTERP }
void HhbcTranslator::emitReqDoc()                  { INTERP }
void HhbcTranslator::emitReqOnce()                 { INTERP }
void HhbcTranslator::emitEval()                    { INTERP }
void HhbcTranslator::emitDefTypeAlias(int32_t)     { INTERP }
void HhbcTranslator::emitCatch()                   { INTERP }
void HhbcTranslator::emitHighInvalid()             { std::abort(); }

//////////////////////////////////////////////////////////////////////

}}

