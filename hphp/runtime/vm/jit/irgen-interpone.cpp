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

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/location.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

Type arithOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) return TCell;
  const auto num = TDbl | TInt;
  if (!(t1 <= num) || !(t2 <= num)) return TBottom;
  auto both = t1 | t2;
  if (both.maybe(TDbl)) return TDbl;
  return TInt;
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
  case SetOpOp::MulEqual:    return arithOpResult(locType, valType);
  case SetOpOp::ConcatEqual: return TStr;
  case SetOpOp::PowEqual:
  case SetOpOp::DivEqual:
  case SetOpOp::ModEqual:    return TUncountedInit;
  case SetOpOp::AndEqual:
  case SetOpOp::OrEqual:
  case SetOpOp::XorEqual:    return bitOpResult(locType, valType);
  case SetOpOp::SlEqual:
  case SetOpOp::SrEqual:     return TInt;
  }
  not_reached();
}

uint32_t localInputId(SrcKey sk) {
  auto const idx = localImmIdx(sk.op());
  auto const argu = getImm(sk.pc(), idx);
  switch (immType(sk.op(), idx)) {
    case ArgType::LA:
      return argu.u_LA;
    case ArgType::NLA:
      return argu.u_NLA.id;
    case ArgType::ILA:
      return argu.u_ILA;
    default:
      always_assert(false);
  }
}

Optional<Type> interpOutputType(IRGS& env,
                                       Optional<Type>& checkTypeType) {
  using namespace jit::InstrFlags;
  auto const sk = curSrcKey(env);
  auto localType = [&]{
    auto locId = localInputId(sk);
    static_assert(std::is_unsigned<decltype(locId)>::value,
                  "locId should be unsigned");
    assertx(locId < curFunc(env)->numLocals());
    return env.irb->local(locId, DataTypeSpecific).type;
  };

  switch (getInstrInfo(sk.op()).type) {
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
    case OutVec:         return TVec;
    case OutVecImm:      return TVec;
    case OutDict:        return TDict;
    case OutDictImm:     return TDict;
    case OutKeyset:      return TKeyset;
    case OutKeysetImm:   return TKeyset;
    case OutObject:
    case OutThisObject:  return TObj;
    case OutResource:    return TRes;

    case OutFDesc:       return std::nullopt;
    case OutCns:         return TCell;

    case OutSameAsInput1: return topType(env, BCSPRelOffset{0});
    case OutSameAsInput2: return topType(env, BCSPRelOffset{1});
    case OutModifiedInput2: return topType(env, BCSPRelOffset{1}).modified();
    case OutModifiedInput3: return topType(env, BCSPRelOffset{2}).modified();

    case OutArith:
      return arithOpResult(topType(env, BCSPRelOffset{0}),
                           topType(env, BCSPRelOffset{1}));

    case OutUnknown:     return TCell;

    case OutBitOp:
      return bitOpResult(topType(env, BCSPRelOffset{0}),
                         sk.op() == HPHP::OpBitNot ?
                            TBottom : topType(env, BCSPRelOffset{1}));
    case OutSetOp:      return setOpResult(localType(),
                          topType(env, BCSPRelOffset{0}),
                          SetOpOp(getImm(sk.pc(), 1).u_OA));
    case OutIncDec: {
      auto ty = localType();
      return ty <= TDbl ? ty : TCell;
    }
    case OutNone:       return std::nullopt;

    case OutCInput: {
      return topType(env, BCSPRelOffset{0});
    }

    case OutCInputL: {
      auto ltype = localType();
      if (ltype <= TCell) return ltype;
      if (ltype < TInitCell) {
        checkTypeType = ltype;
      }
      return TCell;
    }
    case OutFunc: return TFunc;
    case OutFuncLike: return TFuncLike;
    case OutClass: return TCls;
    case OutClsMeth: return TClsMeth;
    case OutClsMethLike: return TClsMethLike;
    case OutLazyClass: return TLazyCls;
    case OutEnumClassLabel: return TEnumClassLabel;
  }
  not_reached();
}

jit::vector<InterpOneData::LocalType>
interpOutputLocals(IRGS& env,
                   bool& smashesAllLocals,
                   Optional<Type> pushedType) {
  using namespace jit::InstrFlags;
  auto const sk = curSrcKey(env);
  auto const& info = getInstrInfo(sk.op());
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
    assertx(id < kMaxHhbcImms);
    assertx(id == localImmIdx(sk.op()));
    setLocType(localInputId(sk), t);
  };

  auto const mDefine = static_cast<unsigned char>(MOpMode::Define);

  switch (sk.op()) {
    case OpSetOpL:
    case OpIncDecL: {
      assertx(pushedType.has_value());

      auto stackType = pushedType.value();
      setImmLocType(0, stackType);
      break;
    }

    case OpSetL:
    case OpPopL: {
      auto stackType = topType(env, BCSPRelOffset{0});
      setImmLocType(0, stackType);
      break;
    }

    case OpUnsetL:
    case OpPushL:
      setImmLocType(0, TUninit);
      break;

    // New minstrs are handled extremely conservatively.
    case OpQueryM:
    case OpMemoGet:
    case OpMemoGetEager:
    case OpMemoSet:
    case OpMemoSetEager:
      break;
    case OpDim:
      if (getImm(sk.pc(), 0).u_OA & mDefine) smashesAllLocals = true;
      break;
    case OpSetM:
    case OpIncDecM:
    case OpSetOpM:
    case OpSetRangeM:
    case OpUnsetM:
      smashesAllLocals = true;
      break;

    case OpIterInit:
    case OpLIterInit:
    case OpIterNext:
    case OpLIterNext: {
      auto const ita = getImm(sk.pc(),  0).u_ITA;
      setLocType(ita.valId, TCell);
      if (ita.hasKey()) setLocType(ita.keyId, TCell);
      break;
    }

    case OpSilence:
      if (static_cast<SilenceOp>(getImm(sk.pc(), 1).u_OA) == SilenceOp::Start) {
        setImmLocType(0, TInt);
      }
      break;

    default:
      always_assert_flog(
        false, "Unknown local-modifying op {}", opcodeToName(sk.op())
      );
  }

  return locals;
}

//////////////////////////////////////////////////////////////////////

}

void interpOne(IRGS& env) {
  Optional<Type> checkTypeType;
  auto const sk = curSrcKey(env);
  auto stackType = interpOutputType(env, checkTypeType);
  auto popped = getStackPopped(sk.pc());
  auto pushed = getStackPushed(sk.pc());
  FTRACE(1, "emitting InterpOne for {}, result = {}, popped {}, pushed {}\n",
         instrToString(sk.pc(), sk.func()),
         stackType.has_value() ? stackType->toString() : "<none>",
         popped, pushed);

  InterpOneData idata { spOffBCFromIRSP(env) };
  auto locals = interpOutputLocals(env, idata.smashesAllLocals, stackType);
  idata.nChangedLocals = locals.size();
  idata.changedLocals = locals.data();

  interpOne(env, stackType, popped, pushed, idata);
  if (env.irb->inUnreachableState()) return;
  if (checkTypeType) {
    auto const out = getInstrInfo(sk.op()).out;
    auto const checkIdx = BCSPRelOffset{
      (out & InstrFlags::StackIns1) ? 1 : 0
    }.to<SBInvOffset>(env.irb->fs().bcSPOff());

    auto const loc = Location::Stack { checkIdx };
    checkType(env, loc, *checkTypeType, makeExit(env, nextSrcKey(env)));
  }
}

void interpOne(IRGS& env, int popped) {
  InterpOneData idata { spOffBCFromIRSP(env) };
  interpOne(env, std::nullopt, popped, 0, idata);
}

void interpOne(IRGS& env, Type outType, int popped) {
  InterpOneData idata { spOffBCFromIRSP(env) };
  interpOne(env, outType, popped, 1, idata);
}

void interpOne(IRGS& env,
               Optional<Type> outType,
               int popped,
               int pushed,
               InterpOneData& idata) {
  auto const func = curFunc(env);
  auto const op = func->getOp(bcOff(env));

  idata.bcOff = bcOff(env);
  idata.cellsPopped = popped;
  idata.cellsPushed = pushed;
  idata.opcode = op;

  spillInlinedFrames(env);

  auto const cf = opcodeChangesPC(idata.opcode);
  gen(
    env,
    cf ? InterpOneCF : InterpOne,
    outType,
    idata,
    sp(env),
    fp(env)
  );

  if (!cf && isInlining(env)) {
    // Can't continue due to spilled frames.
    assertx(!nextSrcKey(env).funcEntry());
    auto const rbjData = ReqBindJmpData {
      nextSrcKey(env),
      spOffBCFromStackBase(env),
      spOffBCFromIRSP(env),
      false /* popFrame */
    };
    gen(env, ReqBindJmp, rbjData, sp(env), fp(env));
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Instructions that unconditionally are implemented with InterpOne are
 * translated here.
 */

void emitExit(IRGS& env)                      { interpOne(env); }
void emitFatal(IRGS& env, FatalOp)            { interpOne(env); }
void emitSetOpG(IRGS& env, SetOpOp)           { interpOne(env); }
void emitIncDecG(IRGS& env, IncDecOp)         { interpOne(env); }
void emitUnsetG(IRGS& env)                    { interpOne(env); }
void emitIncl(IRGS& env)                      { interpOne(env); }
void emitInclOnce(IRGS& env)                  { interpOne(env); }
void emitReq(IRGS& env)                       { interpOne(env); }
void emitReqDoc(IRGS& env)                    { interpOne(env); }
void emitReqOnce(IRGS& env)                   { interpOne(env); }
void emitEval(IRGS& env)                      { interpOne(env); }
void emitChainFaults(IRGS& env)               { interpOne(env); }
void emitContGetReturn(IRGS& env)             { interpOne(env); }
//////////////////////////////////////////////////////////////////////

}
