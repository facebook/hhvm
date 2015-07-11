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
#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include <cstdlib>

#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-guards.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

Type arithOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return TCell;
  }

  auto both = t1 | t2;
  if (both.maybe(TDbl)) return TDbl;
  if (both.maybe(TArr)) return TArr;
  if (both.maybe(TStr)) return TCell;
  return TInt;
}

Type arithOpOverResult(Type t1, Type t2) {
  if (t1 <= TInt && t2 <= TInt) {
    return TInt | TDbl;
  }
  return arithOpResult(t1, t2);
}

Type bitOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return TCell;
  }

  auto both = t1 | t2;
  if (both <= TStr) return TStr;
  return TInt;
}

Type setOpResult(Type locType, Type valType, SetOpOp op) {
  switch (op) {
  case SetOpOp::PlusEqual:
  case SetOpOp::MinusEqual:
  case SetOpOp::MulEqual:    return arithOpResult(locType.unbox(), valType);
  case SetOpOp::PlusEqualO:
  case SetOpOp::MinusEqualO:
  case SetOpOp::MulEqualO:   return arithOpOverResult(locType.unbox(), valType);
  case SetOpOp::ConcatEqual: return TStr;
  case SetOpOp::PowEqual:
  case SetOpOp::DivEqual:
  case SetOpOp::ModEqual:    return TUncountedInit;
  case SetOpOp::AndEqual:
  case SetOpOp::OrEqual:
  case SetOpOp::XorEqual:    return bitOpResult(locType.unbox(), valType);
  case SetOpOp::SlEqual:
  case SetOpOp::SrEqual:     return TInt;
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

folly::Optional<Type> interpOutputType(IRGS& env,
                                       const NormalizedInstruction& inst,
                                       folly::Optional<Type>& checkTypeType) {
  using namespace jit::InstrFlags;
  auto localType = [&]{
    auto locId = localInputId(inst);
    static_assert(std::is_unsigned<decltype(locId)>::value,
                  "locId should be unsigned");
    assertx(locId < curFunc(env)->numLocals());
    return env.irb->localType(locId, DataTypeSpecific);
  };

  auto boxed = [&] (Type t) -> Type {
    if (t == TGen) return TBoxedInitCell;
    assertx(t <= TCell || t <= TBoxedCell);
    checkTypeType = t <= TBoxedCell ? t : boxType(t);
    return TBoxedInitCell;
  };

  auto outFlag = getInstrInfo(inst.op()).type;
  if (outFlag == OutFInputL) {
    outFlag = inst.preppedByRef ? OutVInputL : OutCInputL;
  } else if (outFlag == OutFInputR) {
    outFlag = inst.preppedByRef ? OutVInput : OutCInput;
  }

  switch (outFlag) {
    case OutNull:        return TInitNull;
    case OutNullUninit:  return TUninit;
    case OutString:      return TStr;
    case OutStringImm:   return TStaticStr;
    case OutDouble:      return TDbl;
    case OutIsTypeL:
    case OutBoolean:
    case OutPredBool:
    case OutBooleanImm:  return TBool;
    case OutInt64:       return TInt;
    case OutArray:       return TArr;
    case OutArrayImm:    return TArr; // Should be StaticArr: t2124292
    case OutObject:
    case OutThisObject:  return TObj;
    case OutResource:    return TRes;

    case OutFDesc:       return folly::none;
    case OutUnknown:     return TGen;

    case OutCns:         return TCell;
    case OutVUnknown:    return TBoxedInitCell;

    case OutSameAsInput: return topType(env, BCSPOffset{0});
    case OutVInput:      return boxed(topType(env, BCSPOffset{0}));
    case OutVInputL:     return boxed(localType());
    case OutFInputL:
    case OutFInputR:     not_reached();

    case OutArith:       return arithOpResult(topType(env, BCSPOffset{0}),
                                              topType(env, BCSPOffset{1}));
    case OutArithO:      return arithOpOverResult(topType(env, BCSPOffset{0}),
                                                  topType(env, BCSPOffset{1}));
    case OutBitOp:
      return bitOpResult(topType(env, BCSPOffset{0}),
                         inst.op() == HPHP::OpBitNot ?
                            TBottom : topType(env, BCSPOffset{1}));
    case OutSetOp:      return setOpResult(localType(),
                          topType(env, BCSPOffset{0}),
                          SetOpOp(inst.imm[1].u_OA));
    case OutIncDec: {
      auto ty = localType().unbox();
      return ty <= TDbl ? ty : TCell;
    }
    case OutStrlen:
      return topType(env, BCSPOffset{0}) <= TStr ?
        TInt : TUncountedInit;
    case OutClassRef:   return TCls;
    case OutFPushCufSafe: return folly::none;

    case OutNone:       return folly::none;

    case OutCInput: {
      auto ttype = topType(env, BCSPOffset{0});
      if (ttype <= TCell) return ttype;
      // All instructions that are OutCInput or OutCInputL cannot push uninit or
      // a ref, so only specific inner types need to be checked.
      if (ttype.unbox() < TInitCell) {
        checkTypeType = ttype.unbox();
      }
      return TCell;
    }

    case OutCInputL: {
      auto ltype = localType();
      if (ltype <= TCell) return ltype;
      if (ltype.unbox() < TInitCell) {
        checkTypeType = ltype.unbox();
      }
      return TCell;
    }
  }
  not_reached();
}

jit::vector<InterpOneData::LocalType>
interpOutputLocals(IRGS& env,
                   const NormalizedInstruction& inst,
                   bool& smashesAllLocals,
                   folly::Optional<Type> pushedType) {
  using namespace jit::InstrFlags;
  if (!(getInstrInfo(inst.op()).out & Local)) return {};

  jit::vector<InterpOneData::LocalType> locals;
  auto setLocType = [&](uint32_t id, Type t) {
    // Relax the type to something guardable.  For InterpOne we don't bother to
    // keep track of specialized types or inner-ref types.  (And note that for
    // psuedomains we may in fact have to guard on the local type after this.)
    locals.emplace_back(id, relaxToGuardable(t));
  };
  auto setImmLocType = [&](uint32_t id, Type t) {
    setLocType(inst.imm[id].u_LA, t);
  };
  auto const func = curFunc(env);

  auto handleBoxiness = [&] (Type testTy, Type useTy) {
    return testTy <= TBoxedCell ? TBoxedInitCell :
           testTy.maybe(TBoxedCell) ? TGen :
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

    case OpFPassM:
      switch (inst.immVec.locationCode()) {
      case LL:
      case LNL:
      case LNC:
        // FPassM may or may not affect local types, depending on whether it is
        // passing to a parameter that is by reference.  If we're InterpOne'ing
        // it, just assume it might be by reference and smash all the locals.
        smashesAllLocals = true;
        break;
      default:
        break;
      }
      break;

    case OpSetOpL:
    case OpIncDecL: {
      assertx(pushedType.hasValue());
      auto locType = env.irb->localType(localInputId(inst), DataTypeSpecific);
      assertx(locType < TGen || curFunc(env)->isPseudoMain());

      auto stackType = pushedType.value();
      setImmLocType(0, handleBoxiness(locType, stackType));
      break;
    }

    case OpStaticLocInit:
      setImmLocType(0, TBoxedInitCell);
      break;

    case OpInitThisLoc:
      setImmLocType(0, TCell);
      break;

    case OpSetL: {
      auto locType = env.irb->localType(localInputId(inst), DataTypeSpecific);
      auto stackType = topType(env, BCSPOffset{0});
      // SetL preserves reffiness of a local.
      setImmLocType(0, handleBoxiness(locType, stackType));
      break;
    }
    case OpVGetL:
    case OpBindL: {
      assertx(pushedType.hasValue());
      assertx(*pushedType <= TBoxedCell);
      setImmLocType(0, pushedType.value());
      break;
    }

    case OpUnsetL:
    case OpPushL:
      setImmLocType(0, TUninit);
      break;

    case OpSetM:
    case OpSetOpM:
    case OpBindM:
    case OpVGetM:
    case OpSetWithRefLM:
    case OpSetWithRefRM:
    case OpUnsetM:
    case OpIncDecM:
      switch (inst.immVec.locationCode()) {
        case LL: {
          auto const& mii = getMInstrInfo(inst.mInstrOp());
          auto const& base = inst.inputs[mii.valCount()];
          assertx(base.space == Location::Local);

          // MInstrEffects expects to be used in the context of a normally
          // translated instruction, not an interpOne. The two important
          // differences are that the base is normally a PtrTo* and we need to
          // supply an IR opcode representing the operation. SetWithRefElem is
          // used instead of SetElem because SetElem makes a few assumptions
          // about side exits that interpOne won't do.
          auto const baseType = env.irb->localType(
            base.offset, DataTypeSpecific
          ).ptr(Ptr::Frame);
          auto const isUnset = inst.op() == OpUnsetM;
          auto const isProp = mcodeIsProp(inst.immVecM[0]);

          if (isUnset && isProp) break;

          // NullSafe (Q) props don't change the types of locals.
          if (inst.immVecM[0] == MQT) break;

          auto op = isProp ? SetProp : isUnset ? UnsetElem : SetWithRefElem;
          MInstrEffects effects(op, baseType);
          if (effects.baseValChanged) {
            auto const ty = effects.baseType.deref();
            assertx((ty <= TCell ||
                    ty <= TBoxedCell) ||
                    curFunc(env)->isPseudoMain());
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
      setImmLocType(3, TCell);
      /* fallthrough */
    case OpMIterInit:
    case OpMIterNext:
      setImmLocType(2, TBoxedInitCell);
      break;

    case OpIterInitK:
    case OpWIterInitK:
    case OpIterNextK:
    case OpWIterNextK:
      setImmLocType(3, TCell);
      /* fallthrough */
    case OpIterInit:
    case OpWIterInit:
    case OpIterNext:
    case OpWIterNext:
      setImmLocType(2, TGen);
      break;

    case OpVerifyParamType: {
      auto paramId = inst.imm[0].u_LA;
      auto const& tc = func->params()[paramId].typeConstraint;
      auto locType = env.irb->localType(localInputId(inst), DataTypeSpecific);
      if (tc.isArray() && !tc.isSoft() && !func->mustBeRef(paramId) &&
          (locType <= TObj || locType.maybe(TBoxedCell))) {
        setImmLocType(0, handleBoxiness(locType, TCell));
      }
      break;
    }

    case OpSilence:
      if (static_cast<SilenceOp>(inst.imm[0].u_OA) == SilenceOp::Start) {
        setImmLocType(inst.imm[0].u_LA, TInt);
      }
      break;

    default:
      not_reached();
  }

  return locals;
}

//////////////////////////////////////////////////////////////////////

}

void interpOne(IRGS& env, const NormalizedInstruction& inst) {
  folly::Optional<Type> checkTypeType;
  auto stackType = interpOutputType(env, inst, checkTypeType);
  auto popped = getStackPopped(inst.pc());
  auto pushed = getStackPushed(inst.pc());
  FTRACE(1, "emitting InterpOne for {}, result = {}, popped {}, pushed {}\n",
         inst.toString(),
         stackType.hasValue() ? stackType->toString() : "<none>",
         popped, pushed);

  InterpOneData idata { offsetFromIRSP(env, BCSPOffset{0}) };
  auto locals = interpOutputLocals(env, inst, idata.smashesAllLocals,
    stackType);
  idata.nChangedLocals = locals.size();
  idata.changedLocals = locals.data();

  interpOne(env, stackType, popped, pushed, idata);
  if (checkTypeType) {
    auto const out = getInstrInfo(inst.op()).out;
    auto const checkIdx = BCSPOffset{(out & InstrFlags::StackIns2) ? 2
                        : (out & InstrFlags::StackIns1) ? 1
                        : 0};
    checkTypeStack(env, checkIdx, *checkTypeType, inst.nextSk().offset(),
                   true /* outerOnly */);
  }
}

void interpOne(IRGS& env, int popped) {
  InterpOneData idata { offsetFromIRSP(env, BCSPOffset{0}) };
  interpOne(env, folly::none, popped, 0, idata);
}

void interpOne(IRGS& env, Type outType, int popped) {
  InterpOneData idata { offsetFromIRSP(env, BCSPOffset{0}) };
  interpOne(env, outType, popped, 1, idata);
}

void interpOne(IRGS& env,
               folly::Optional<Type> outType,
               int popped,
               int pushed,
               InterpOneData& idata) {
  auto const unit = curUnit(env);
  spillStack(env);
  env.irb->exceptionStackBoundary();
  auto const op = unit->getOpcode(bcOff(env));

  auto& iInfo = getInstrInfo(op);
  if (iInfo.type == jit::InstrFlags::OutFDesc) {
    env.fpiStack.push(FPIInfo { sp(env), env.irb->spOffset(), nullptr });
  } else if (isFCallStar(op) && !env.fpiStack.empty()) {
    env.fpiStack.pop();
  }

  idata.bcOff = bcOff(env);
  idata.cellsPopped = popped;
  idata.cellsPushed = pushed;
  idata.opcode = op;

  gen(
    env,
    opcodeChangesPC(idata.opcode) ? InterpOneCF : InterpOne,
    outType,
    idata,
    sp(env),
    fp(env)
  );
  assertx(env.irb->stackDeficit() == 0);
}

//////////////////////////////////////////////////////////////////////

/*
 * Instructions that unconditionally are implemented with InterpOne are
 * translated here.
 */

#define INTERP interpOne(env, *env.currentNormalizedInstruction);

void emitFPushObjMethod(IRGS& env, int32_t, ObjMethodOp) { INTERP }

void emitLowInvalid(IRGS& env)                { std::abort(); }
void emitCGetL3(IRGS& env, int32_t)           { INTERP }
void emitBox(IRGS& env)                       { INTERP }
void emitBoxR(IRGS& env)                      { INTERP }
void emitAddElemV(IRGS& env)                  { INTERP }
void emitAddNewElemV(IRGS& env)               { INTERP }
void emitClsCns(IRGS& env, const StringData*) { INTERP }
void emitExit(IRGS& env)                      { INTERP }
void emitFatal(IRGS& env, FatalOp)            { INTERP }
void emitUnwind(IRGS& env)                    { INTERP }
void emitThrow(IRGS& env)                     { INTERP }
void emitCGetN(IRGS& env)                     { INTERP }
void emitVGetN(IRGS& env)                     { INTERP }
void emitIssetN(IRGS& env)                    { INTERP }
void emitEmptyN(IRGS& env)                    { INTERP }
void emitSetN(IRGS& env)                      { INTERP }
void emitSetOpN(IRGS& env, SetOpOp)           { INTERP }
void emitSetOpG(IRGS& env, SetOpOp)           { INTERP }
void emitSetOpS(IRGS& env, SetOpOp)           { INTERP }
void emitIncDecN(IRGS& env, IncDecOp)         { INTERP }
void emitIncDecG(IRGS& env, IncDecOp)         { INTERP }
void emitIncDecS(IRGS& env, IncDecOp)         { INTERP }
void emitBindN(IRGS& env)                     { INTERP }
void emitUnsetN(IRGS& env)                    { INTERP }
void emitUnsetG(IRGS& env)                    { INTERP }
void emitFPassN(IRGS& env, int32_t)           { INTERP }
void emitFCallUnpack(IRGS& env, int32_t)      { INTERP }
void emitCufSafeArray(IRGS& env)              { INTERP }
void emitCufSafeReturn(IRGS& env)             { INTERP }
void emitIncl(IRGS& env)                      { INTERP }
void emitInclOnce(IRGS& env)                  { INTERP }
void emitReq(IRGS& env)                       { INTERP }
void emitReqDoc(IRGS& env)                    { INTERP }
void emitReqOnce(IRGS& env)                   { INTERP }
void emitEval(IRGS& env)                      { INTERP }
void emitDefTypeAlias(IRGS& env, int32_t)     { INTERP }
void emitDefCns(IRGS& env, const StringData*) { INTERP }
void emitDefCls(IRGS& env, int32_t)           { INTERP }
void emitDefFunc(IRGS& env, int32_t)          { INTERP }
void emitCatch(IRGS& env)                     { INTERP }
void emitHighInvalid(IRGS& env)               { std::abort(); }

//////////////////////////////////////////////////////////////////////

}}}
