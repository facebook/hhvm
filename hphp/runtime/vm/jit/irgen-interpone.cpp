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

#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include <cstdlib>

#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
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
  if (both.maybe(TVec)) return TVec;
  if (both.maybe(TDict)) return TDict;
  if (both.maybe(TKeyset)) return TKeyset;
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
  return inst.imm[localImmIdx(inst.op())].u_LA;
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
    return env.irb->local(locId, DataTypeSpecific).type;
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
    case OutArrayImm:    return TArr; // Should be StaticArr/Vec/Dict: t2124292
    case OutVec:         return TVec;
    case OutVecImm:      return TVec;
    case OutDict:        return TDict;
    case OutDictImm:     return TDict;
    case OutKeyset:      return TKeyset;
    case OutKeysetImm:   return TKeyset;
    case OutObject:
    case OutThisObject:  return TObj;
    case OutResource:    return TRes;

    case OutFDesc:       return folly::none;
    case OutCns:         return TCell;
    case OutVUnknown:    return TBoxedInitCell;

    case OutSameAsInput1: return topType(env, BCSPRelOffset{0});
    case OutModifiedInput3: return topType(env, BCSPRelOffset{2}).modified();
    case OutVInput:      return boxed(topType(env, BCSPRelOffset{0}));
    case OutVInputL:     return boxed(localType());
    case OutFInputL:
    case OutFInputR:     not_reached();

    case OutArith:
      return arithOpResult(topType(env, BCSPRelOffset{0}),
                           topType(env, BCSPRelOffset{1}));
    case OutArithO:
      return arithOpOverResult(topType(env, BCSPRelOffset{0}),
                               topType(env, BCSPRelOffset{1}));
    case OutUnknown: {
      if (isFPassStar(inst.op())) {
        return inst.preppedByRef ? TBoxedInitCell : TCell;
      }
      return TGen;
    }
    case OutBitOp:
      return bitOpResult(topType(env, BCSPRelOffset{0}),
                         inst.op() == HPHP::OpBitNot ?
                            TBottom : topType(env, BCSPRelOffset{1}));
    case OutSetOp:      return setOpResult(localType(),
                          topType(env, BCSPRelOffset{0}),
                          SetOpOp(inst.imm[1].u_OA));
    case OutIncDec: {
      auto ty = localType().unbox();
      return ty <= TDbl ? ty : TCell;
    }
    case OutFPushCufSafe: return folly::none;

    case OutNone:       return folly::none;

    case OutCInput: {
      auto ttype = topType(env, BCSPRelOffset{0});
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
  auto const& info = getInstrInfo(inst.op());
  // Anything with Local in its output or a member base input can modify a
  // local.
  if (!(info.out & Local) && !(info.in & MBase)) return {};

  jit::vector<InterpOneData::LocalType> locals;
  auto setLocType = [&](uint32_t id, Type t) {
    // Relax the type to something guardable.  For InterpOne we don't bother to
    // keep track of specialized types or inner-ref types.  (And note that for
    // psuedomains we may in fact have to guard on the local type after this.)
    locals.emplace_back(id, relaxToGuardable(t));
  };
  auto setImmLocType = [&](uint32_t id, Type t) {
    assert(id < 4);
    setLocType(inst.imm[id].u_LA, t);
  };
  auto handleBoxiness = [&] (Type testTy, Type useTy) {
    return testTy <= TBoxedCell ? TBoxedInitCell :
           testTy.maybe(TBoxedCell) ? TGen :
           useTy;
  };

  auto const mDefine = static_cast<unsigned char>(MOpMode::Define);

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
      assertx(pushedType.hasValue());
      auto locType = env.irb->local(localInputId(inst), DataTypeSpecific).type;
      assertx(locType < TGen || curFunc(env)->isPseudoMain());

      auto stackType = pushedType.value();
      setImmLocType(0, handleBoxiness(locType, stackType));
      break;
    }

    case OpStaticLocInit:
    case OpStaticLocDef:
      setImmLocType(0, TBoxedInitCell);
      break;

    case OpStaticLocCheck:
      setImmLocType(0, TGen);
      break;

    case OpInitThisLoc:
      setImmLocType(0, TCell);
      break;

    case OpSetL:
    case OpPopL: {
      auto locType = env.irb->local(localInputId(inst), DataTypeSpecific).type;
      auto stackType = topType(env, BCSPRelOffset{0});
      // [Set,Pop]L preserves reffiness of a local.
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

    // New minstrs are handled extremely conservatively.
    case OpQueryM:
    case OpMemoGet:
      break;
    case OpDim:
      if (inst.imm[0].u_OA & mDefine) smashesAllLocals = true;
      break;
    case OpFPassDim:
    case OpFPassM:
    case OpVGetM:
    case OpSetM:
    case OpIncDecM:
    case OpSetOpM:
    case OpBindM:
    case OpUnsetM:
    case OpSetWithRefLML:
    case OpSetWithRefRML:
    case OpMemoSet:
      smashesAllLocals = true;
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
      auto locType = env.irb->local(localInputId(inst), DataTypeSpecific).type;
      setImmLocType(0, handleBoxiness(locType, TCell));
      break;
    }

    case OpSilence:
      if (static_cast<SilenceOp>(inst.imm[1].u_OA) == SilenceOp::Start) {
        setImmLocType(0, TInt);
      }
      break;

    default:
      always_assert_flog(
        false, "Unknown local-modifying op {}", opcodeToName(inst.op())
      );
  }

  return locals;
}

jit::vector<InterpOneData::ClsRefSlot>
interpClsRefSlots(IRGS& /*env*/, const NormalizedInstruction& inst) {
  jit::vector<InterpOneData::ClsRefSlot> slots;

  auto const op = inst.op();
  auto const numImmeds = numImmediates(op);
  for (auto i = uint32_t{0}; i < numImmeds; ++i) {
    auto const type = immType(op, i);
    if (type == ArgType::CAR) {
      slots.emplace_back(inst.imm[i].u_CAR, false);
    } else if (type == ArgType::CAW) {
      slots.emplace_back(inst.imm[i].u_CAW, true);
    }
  }

  return slots;
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

  InterpOneData idata { spOffBCFromIRSP(env) };
  auto locals = interpOutputLocals(env, inst, idata.smashesAllLocals,
    stackType);
  idata.nChangedLocals = locals.size();
  idata.changedLocals = locals.data();

  auto slots = interpClsRefSlots(env, inst);
  idata.nChangedClsRefSlots = slots.size();
  idata.changedClsRefSlots = slots.data();

  interpOne(env, stackType, popped, pushed, idata);
  if (checkTypeType) {
    auto const out = getInstrInfo(inst.op()).out;
    auto const checkIdx = BCSPRelOffset{
      (out & InstrFlags::StackIns1) ? 1 : 0
    }.to<FPInvOffset>(env.irb->fs().bcSPOff());

    checkType(env, Location::Stack { checkIdx }, *checkTypeType,
              inst.nextSk().offset(), true /* outerOnly */);
  }
}

void interpOne(IRGS& env, int popped) {
  InterpOneData idata { spOffBCFromIRSP(env) };
  interpOne(env, folly::none, popped, 0, idata);
}

void interpOne(IRGS& env, Type outType, int popped) {
  InterpOneData idata { spOffBCFromIRSP(env) };
  interpOne(env, outType, popped, 1, idata);
}

void interpOne(IRGS& env,
               folly::Optional<Type> outType,
               int popped,
               int pushed,
               InterpOneData& idata) {
  auto const unit = curUnit(env);
  auto const op = unit->getOp(bcOff(env));

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
}

//////////////////////////////////////////////////////////////////////

/*
 * Instructions that unconditionally are implemented with InterpOne are
 * translated here.
 */

#define INTERP interpOne(env, *env.currentNormalizedInstruction);

void emitFPushObjMethod(IRGS& env, uint32_t, ObjMethodOp) { INTERP }

void emitAddElemV(IRGS& env)                  { INTERP }
void emitAddNewElemV(IRGS& env)               { INTERP }
void emitExit(IRGS& env)                      { INTERP }
void emitFatal(IRGS& env, FatalOp)            { INTERP }
void emitUnwind(IRGS& env)                    { INTERP }
void emitThrow(IRGS& env)                     { INTERP }
void emitCGetN(IRGS& env)                     { INTERP }
void emitCGetQuietN(IRGS& env)                { INTERP }
void emitVGetN(IRGS& env)                     { INTERP }
void emitIssetN(IRGS& env)                    { INTERP }
void emitEmptyN(IRGS& env)                    { INTERP }
void emitSetN(IRGS& env)                      { INTERP }
void emitSetOpN(IRGS& env, SetOpOp)           { INTERP }
void emitSetOpG(IRGS& env, SetOpOp)           { INTERP }
void emitSetOpS(IRGS& env, SetOpOp, uint32_t) { INTERP }
void emitIncDecN(IRGS& env, IncDecOp)         { INTERP }
void emitIncDecG(IRGS& env, IncDecOp)         { INTERP }
void emitBindN(IRGS& env)                     { INTERP }
void emitUnsetN(IRGS& env)                    { INTERP }
void emitUnsetG(IRGS& env)                    { INTERP }
void emitFPassN(IRGS& env, uint32_t)          { INTERP }
void emitCufSafeArray(IRGS& env)              { INTERP }
void emitCufSafeReturn(IRGS& env)             { INTERP }
void emitIncl(IRGS& env)                      { INTERP }
void emitInclOnce(IRGS& env)                  { INTERP }
void emitReq(IRGS& env)                       { INTERP }
void emitReqDoc(IRGS& env)                    { INTERP }
void emitReqOnce(IRGS& env)                   { INTERP }
void emitEval(IRGS& env)                      { INTERP }
void emitDefTypeAlias(IRGS& env, uint32_t)    { INTERP }
void emitDefCns(IRGS& env, const StringData*) { INTERP }
void emitDefCls(IRGS& env, uint32_t)          { INTERP }
void emitAliasCls(IRGS& env,
                  const StringData*,
                  const StringData*)          { INTERP }
void emitDefFunc(IRGS& env, uint32_t)         { INTERP }
void emitCatch(IRGS& env)                     { INTERP }
void emitContGetReturn(IRGS& env)             { INTERP }
void emitContAssignDelegate(IRGS& env, int32_t)
                                              { INTERP }
void emitContEnterDelegate(IRGS& env)         { INTERP }
void emitYieldFromDelegate(IRGS& env, int32_t, int32_t)
                                              { INTERP }
void emitContUnsetDelegate(IRGS& env, CudOp, int32_t)
                                              { INTERP }

//////////////////////////////////////////////////////////////////////

}}}
