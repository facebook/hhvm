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

#include "hphp/runtime/vm/jit/translator.h"

#include <cinttypes>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "folly/Conv.h"
#include "folly/MapUtil.h"
#include "folly/Optional.h"

#include "hphp/util/map-walker.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-instrs.h"
#include "hphp/runtime/vm/jit/type.h"

#define KindOfUnknown DontUseKindOfUnknownInThisFile
#define KindOfInvalid DontUseKindOfInvalidInThisFile

TRACE_SET_MOD(trans);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

Lease Translator::s_writeLease;

int locPhysicalOffset(Location l, const Func* f) {
  f = f ? f : liveFunc();
  assert_not_implemented(l.space == Location::Stack ||
                         l.space == Location::Local ||
                         l.space == Location::Iter);
  int localsToSkip = l.space == Location::Iter ? f->numLocals() : 0;
  int iterInflator = l.space == Location::Iter ? kNumIterCells : 1;
  return -((l.offset + 1) * iterInflator + localsToSkip);
}

bool Translator::liveFrameIsPseudoMain() {
  ActRec* ar = (ActRec*)vmfp();
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

static uint32_t m_w = 1;    /* must not be zero */
static uint32_t m_z = 1;    /* must not be zero */

static uint32_t get_random()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

static const int kTooPolyRet = 6;

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* ctx, const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd) {
  if (mInd == 0) {
    auto const baseIndex = mii.valCount();
    baseClass = ni.inputs[baseIndex]->rtt < Type::Obj
      ? ni.inputs[baseIndex]->rtt.getClass()
      : nullptr;
  }
  if (!baseClass) return PropInfo();

  auto keyType = ni.inputs[iInd]->rtt;
  if (!keyType.isConst(Type::Str)) return PropInfo();
  auto const name = keyType.strVal();

  bool accessible;
  // If we are not in repo-authoriative mode, we need to check that
  // baseClass cannot change in between requests
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return PropInfo();
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return PropInfo();
      }
    }
  }
  // Lookup the index of the property based on ctx and baseClass
  Slot idx = baseClass->getDeclPropIndex(ctx, name, accessible);
  // If we couldn't find a property that is accessible in the current
  // context, bail out
  if (idx == kInvalidSlot || !accessible) {
    return PropInfo();
  }
  // If it's a declared property we're good to go: even if a subclass
  // redefines an accessible property with the same name it's guaranteed
  // to be at the same offset
  return PropInfo(
    baseClass->declPropOffset(idx),
    baseClass->declPropRepoAuthType(idx)
  );
}

PropInfo getFinalPropertyOffset(const NormalizedInstruction& ni,
                                Class* ctx, const MInstrInfo& mii) {
  unsigned mInd = ni.immVecM.size() - 1;
  unsigned iInd = mii.valCount() + 1 + mInd;

  const Class* cls = nullptr;
  return getPropertyOffset(ni, ctx, cls, mii, mInd, iInd);
}

static folly::Optional<DataType>
predictionForRepoAuthType(RepoAuthType repoTy) {
  using T = RepoAuthType::Tag;
  switch (repoTy.tag()) {
  case T::OptBool:  return KindOfBoolean;
  case T::OptInt:   return KindOfInt64;
  case T::OptDbl:   return KindOfDouble;
  case T::OptRes:   return KindOfResource;

  case T::OptSArr:
  case T::OptArr:
    return KindOfArray;

  case T::OptStr:
  case T::OptSStr:
    return KindOfString;

  case T::OptSubObj:
  case T::OptExactObj:
  case T::OptObj:
    return KindOfObject;

  case T::Bool:
  case T::Uninit:
  case T::InitNull:
  case T::Int:
  case T::Dbl:
  case T::Res:
  case T::Str:
  case T::Arr:
  case T::Obj:
  case T::Null:
  case T::SStr:
  case T::SArr:
  case T::SubObj:
  case T::ExactObj:
  case T::Cell:
  case T::Ref:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::InitGen:
  case T::Gen:
    return folly::none;
  }
  not_reached();
}

static std::pair<DataType,double>
predictMVec(const NormalizedInstruction* ni) {
  auto info = getFinalPropertyOffset(*ni,
                                     ni->func()->cls(),
                                     getMInstrInfo(ni->mInstrOp()));
  if (info.offset != -1) {
    auto const predTy = predictionForRepoAuthType(info.repoAuthType);
    if (predTy) {
      FTRACE(1, "prediction for CGetM prop: {}, hphpc\n",
        static_cast<int>(*predTy));
      return std::make_pair(*predTy, 1.0);
    }
    // If the RepoAuthType converts to an exact data type, there's no
    // point in having a prediction because we know its type with 100%
    // accuracy.  Disable it in that case here.
    if (convertToDataType(info.repoAuthType)) {
      return std::make_pair(KindOfAny, 0.0);
    }
  }

  auto& immVec = ni->immVec;
  StringData* name;
  MemberCode mc;
  if (immVec.decodeLastMember(ni->m_unit, name, mc)) {
    auto pred = predictType(TypeProfileKey(mc, name));
    TRACE(1, "prediction for CGetM %s named %s: %d, %f\n",
          mc == MET ? "elt" : "prop",
          name->data(),
          pred.first,
          pred.second);
    return pred;
  }

  return std::make_pair(KindOfAny, 0.0);
}

/*
 * predictOutputs --
 *
 *   Provide a best guess for the output type of this instruction.
 */
static DataType
predictOutputs(const NormalizedInstruction* ni) {
  if (!RuntimeOption::EvalJitTypePrediction) return KindOfAny;

  if (RuntimeOption::EvalJitStressTypePredPercent &&
      RuntimeOption::EvalJitStressTypePredPercent > int(get_random() % 100)) {
    int dt;
    while (true) {
      dt = get_random() % (KindOfRef + 1);
      switch (dt) {
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfString:
        case KindOfArray:
        case KindOfObject:
        case KindOfResource:
          break;
        // KindOfRef and KindOfUninit can't happen for lots of predicted
        // types.
        case KindOfRef:
        case KindOfUninit:
        default:
          continue;
      }
      break;
    }
    return DataType(dt);
  }

  if (ni->op() == OpCns ||
      ni->op() == OpCnsE ||
      ni->op() == OpCnsU) {
    StringData* sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
    auto const tv = Unit::lookupCns(sd);
    if (tv) return tv->m_type;
  }

  if (ni->op() == OpMod) {
    // x % 0 returns boolean false, so we don't know for certain, but it's
    // probably an int.
    return KindOfInt64;
  }

  if (ni->op() == OpPow) {
    // int ** int => int, unless result > 2 ** 52, then it's a double
    // anything ** double => double
    // double ** anything => double
    // anything ** anything => int
    auto lhs = ni->inputs[0]->rtt;
    auto rhs = ni->inputs[1]->rtt;

    if (lhs <= Type::Int && rhs <= Type::Int) {
      // Best guess, since overflowing isn't common
      return KindOfInt64;
    }

    if (lhs <= Type::Dbl || rhs <= Type::Dbl) {
      return KindOfDouble;
    }

    return KindOfInt64;
  }

  if (ni->op() == OpSqrt) {
    // sqrt returns a double, unless you pass something nasty to it.
    return KindOfDouble;
  }

  if (ni->op() == OpDiv) {
    // Integers can produce integers if there's no residue, but $i / $j in
    // general produces a double. $i / 0 produces boolean false, so we have
    // actually check the result.
    return KindOfDouble;
  }

  if (ni->op() == OpAbs) {
    if (ni->inputs[0]->rtt <= Type::Dbl) {
      return KindOfDouble;
    }

    // some types can't be converted to integers and will return false here
    if (ni->inputs[0]->rtt <= Type::Arr) {
      return KindOfBoolean;
    }

    // If the type is not numeric we need to convert it to a numeric type,
    // a string can be converted to an Int64 or a Double but most other types
    // will end up being integral.
    return KindOfInt64;
  }

  if (ni->op() == OpClsCnsD) {
    const NamedEntityPair& cne =
      ni->unit()->lookupNamedEntityPairId(ni->imm[1].u_SA);
    StringData* cnsName = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
    Class* cls = cne.second->getCachedClass();
    if (cls) {
      DataType dt = cls->clsCnsType(cnsName);
      if (dt != KindOfUninit) {
        TRACE(1, "clscnsd: %s:%s prediction type %d\n",
              cne.first->data(), cnsName->data(), dt);
        return dt;
      }
    }
  }

  if (ni->op() == OpSetM) {
    /*
     * SetM pushes null for certain rare combinations of input types, a string
     * if the base was a string, or (most commonly) its first stack input. We
     * mark the output as predicted here and do a very rough approximation of
     * what really happens; most of the time the prediction will be a noop
     * since MInstrTranslator side exits in all uncommon cases.
     */

    auto inType = ni->inputs[0]->rtt;
    auto const inDt = inType.isKnownDataType() ? inType.toDataType()
                                               : KindOfAny;
    // If the base is a string, the output is probably a string. Unless the
    // member code is MW, then we're either going to fatal or promote the
    // string to an array.
    Type baseType;
    switch (ni->immVec.locationCode()) {
      case LGL: case LGC:
      case LNL: case LNC:
      case LSL: case LSC:
        baseType = Type::Gen;
        break;

      default:
        baseType = ni->inputs[1]->rtt;
    }
    if (baseType <= Type::Str && ni->immVecM.size() == 1) {
      return ni->immVecM[0] == MW ? inDt : KindOfString;
    }

    // Otherwise, it's probably the input type.
    return inDt;
  }

  auto const op = ni->op();
  static const double kAccept = 1.0;
  std::pair<DataType, double> pred = std::make_pair(KindOfAny, 0.0);
  if (op == OpCGetS) {
    auto nameType = ni->inputs[1]->rtt;
    if (nameType.isConst(Type::Str)) {
      auto propName = nameType.strVal();
      pred = predictType(TypeProfileKey(TypeProfileKey::StaticPropName,
                                        propName));
      TRACE(1, "prediction for static fields named %s: %d, %f\n",
            propName->data(),
            pred.first,
            pred.second);
    }
  } else if (op == OpCGetM) {
    pred = predictMVec(ni);
  }
  if (pred.second < kAccept) {
    const StringData* const invName
      = ni->op() == Op::FCallD
        ? ni->m_unit->lookupLitstrId(ni->imm[2].u_SA)
        : nullptr;
    if (invName) {
      pred = predictType(TypeProfileKey(TypeProfileKey::MethodName, invName));
      FTRACE(1, "prediction for methods named {}: {}, {:.2}\n",
             invName->data(),
             pred.first,
             pred.second);
    }
  }
  if (pred.second >= kAccept) {
    FTRACE(1, "accepting prediction of type {}\n", pred.first);
    assert(pred.first != KindOfUninit);
    return pred.first;
  }
  return KindOfAny;
}

const StaticString s_wait_handle("WaitHandle");

/*
 * NB: this opcode structure is sparse; it cannot just be indexed by
 * opcode.
 */
using namespace InstrFlags;
static const struct {
  Op op;
  InstrInfo info;
} instrInfoSparse [] = {

  // Op             Inputs            Outputs       OutputTypes    Stack delta
  // --             ------            -------       -----------    -----------

  /*** 1. Basic instructions ***/

  { OpPopA,        {Stack1,           None,         OutNone,          -1 }},
  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone,          -1 }},
  { OpPopV,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpPopR,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpDup,         {Stack1,           StackTop2,    OutSameAsInput,    1 }},
  { OpBox,         {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnbox,       {Stack1,           Stack1,       OutCInput,         0 }},
  { OpBoxR,        {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput,         0 }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull,           1 }},
  { OpNullUninit,  {None,             Stack1,       OutNullUninit,     1 }},
  { OpTrue,        {None,             Stack1,       OutBooleanImm,     1 }},
  { OpFalse,       {None,             Stack1,       OutBooleanImm,     1 }},
  { OpInt,         {None,             Stack1,       OutInt64,          1 }},
  { OpDouble,      {None,             Stack1,       OutDouble,         1 }},
  { OpString,      {None,             Stack1,       OutStringImm,      1 }},
  { OpArray,       {None,             Stack1,       OutArrayImm,       1 }},
  { OpNewArray,    {None,             Stack1,       OutArray,          1 }},
  { OpNewMixedArray,  {None,          Stack1,       OutArray,          1 }},
  { OpNewVArray,   {None,             Stack1,       OutArray,          1 }},
  { OpNewMIArray,  {None,             Stack1,       OutArray,          1 }},
  { OpNewMSArray,  {None,             Stack1,       OutArray,          1 }},
  { OpNewLikeArrayL,  {None,          Stack1,       OutArray,          1 }},
  { OpNewPackedArray, {StackN,        Stack1,       OutArray,          0 }},
  { OpNewStructArray, {StackN,        Stack1,       OutArray,          0 }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpNewCol,      {None,             Stack1,       OutObject,         1 }},
  { OpColAddElemC, {StackTop3,        Stack1,       OutObject,        -2 }},
  { OpColAddNewElemC, {StackTop2,     Stack1,       OutObject,        -1 }},
  { OpCns,         {None,             Stack1,       OutCns,            1 }},
  { OpCnsE,        {None,             Stack1,       OutCns,            1 }},
  { OpCnsU,        {None,             Stack1,       OutCns,            1 }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpClsCnsD,     {None,             Stack1,       OutPred,           1 }},
  { OpFile,        {None,             Stack1,       OutString,         1 }},
  { OpDir,         {None,             Stack1,       OutString,         1 }},
  { OpNameA,       {Stack1,           Stack1,       OutString,         0 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  { OpConcatN,     {StackN,           Stack1,       OutString,         0 }},
  /* Arithmetic ops */
  { OpAbs,         {Stack1,           Stack1,       OutPred,           0 }},
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Arithmetic ops that overflow ints to floats */
  { OpAddO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpSubO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpMulO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpPow,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpSqrt,        {Stack1,           Stack1,       OutPred,           0 }},
  /* Logical ops */
  { OpXor,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNot,         {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpSame,        {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNSame,       {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEq,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNeq,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  /* Bitwise ops */
  { OpBitAnd,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitOr,       {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitXor,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitNot,      {Stack1,           Stack1,       OutBitOp,          0 }},
  { OpShl,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  { OpShr,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  /* Cast instructions */
  { OpCastBool,    {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpCastInt,     {Stack1,           Stack1,       OutInt64,          0 }},
  { OpCastDouble,  {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCastString,  {Stack1,           Stack1,       OutString,         0 }},
  { OpCastArray,   {Stack1,           Stack1,       OutArray,          0 }},
  { OpCastObject,  {Stack1,           Stack1,       OutObject,         0 }},
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutPredBool,       0 }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64,          0 }},
  { OpClone,       {Stack1,           Stack1,       OutObject,         0 }},
  { OpExit,        {Stack1,           Stack1,       OutNull,           0 }},
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpNS,       {None,             None,         OutNone,           0 }},
  { OpJmpZ,        {Stack1,           None,         OutNone,          -1 }},
  { OpJmpNZ,       {Stack1,           None,         OutNone,          -1 }},
  { OpSwitch,      {Stack1,           None,         OutNone,          -1 }},
  { OpSSwitch,     {Stack1,           None,         OutNone,          -1 }},
  /*
   * RetC and RetV are special. Their manipulation of the runtime stack are
   * outside the boundaries of the tracelet abstraction; since they always end
   * a basic block, they behave more like "glue" between BBs than the
   * instructions in the body of a BB.
   *
   * RetC and RetV consume a value from the stack, and this value's type needs
   * to be known at compile-time.
   */
  { OpRetC,        {AllLocals,        None,         OutNone,           0 }},
  { OpRetV,        {AllLocals,        None,         OutNone,           0 }},
  { OpThrow,       {Stack1,           None,         OutNone,          -1 }},
  { OpUnwind,      {None,             None,         OutNone,           0 }},

  /*** 5. Get instructions ***/

  { OpCGetL,       {Local,            Stack1,       OutCInputL,        1 }},
  { OpCGetL2,      {Stack1|Local,     StackIns1,    OutCInputL,        1 }},
  { OpCGetL3,      {StackTop2|Local,  StackIns2,    OutCInputL,        1 }},
  { OpPushL,       {Local,            Stack1|Local, OutCInputL,        1 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutPred,           1 }},
  { OpVGetL,       {Local,            Stack1|Local, OutVInputL,        1 }},
  { OpVGetN,       {Stack1,           Stack1|Local, OutVUnknown,       0 }},
  // TODO: In pseudo-main, the VGetG instruction invalidates what we know
  // about the types of the locals because it could cause any one of the
  // local variables to become "boxed". We need to add logic to tracelet
  // analysis to deal with this properly.
  { OpVGetG,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  { OpVGetS,       {StackTop2,        Stack1,       OutVUnknown,      -1 }},
  { OpVGetM,       {MVector,          Stack1|Local, OutVUnknown,       1 }},
  { OpAGetC,       {Stack1,           Stack1,       OutClassRef,       0 }},
  { OpAGetL,       {Local,            Stack1,       OutClassRef,       1 }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpIssetN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpEmptyL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpEmptyN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEmptyM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpIsTypeC,     {Stack1|
                    DontGuardStack1,  Stack1,       OutBoolean,        0 }},
  { OpIsTypeL,     {Local,            Stack1,       OutIsTypeL,        1 }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput,    0 }},
  { OpSetN,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetG,        {StackTop2,        Stack1,       OutSameAsInput,   -1 }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutPred,           0 }},
  { OpSetWithRefLM,{MVector|Local ,   Local,        OutNone,           0 }},
  { OpSetWithRefRM,{MVector|Stack1,   Local,        OutNone,          -1 }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp,          0 }},
  { OpSetOpN,      {StackTop2,        Stack1|Local, OutUnknown,       -1 }},
  { OpSetOpG,      {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpSetOpS,      {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpSetOpM,      {MVector|Stack1,   Stack1|Local, OutUnknown,        0 }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec,         1 }},
  { OpIncDecN,     {Stack1,           Stack1|Local, OutUnknown,        0 }},
  { OpIncDecG,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpIncDecS,     {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpIncDecM,     {MVector,          Stack1|Local, OutUnknown,        1 }},
  { OpBindL,       {Stack1|Local|
                    IgnoreInnerType,  Stack1|Local, OutSameAsInput,    0 }},
  { OpBindN,       {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpBindG,       {StackTop2,        Stack1,       OutSameAsInput,   -1 }},
  { OpBindS,       {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpBindM,       {MVector|Stack1,   Stack1|Local, OutSameAsInput,    0 }},
  { OpUnsetL,      {Local,            Local,        OutNone,           0 }},
  { OpUnsetN,      {Stack1,           Local,        OutNone,          -1 }},
  { OpUnsetG,      {Stack1,           None,         OutNone,          -1 }},
  { OpUnsetM,      {MVector,          Local,        OutNone,           0 }},

  /*** 8. Call instructions ***/

  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushFuncD,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushFuncU,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushObjMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushObjMethodD,
                   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushClsMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodF,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushCtor,   {Stack1,           Stack1|FStack,OutObject,
                                                         kNumActRecCells }},
  { OpFPushCtorD,  {None,             Stack1|FStack,OutObject,
                                                     kNumActRecCells + 1 }},
  { OpFPushCufIter,{None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushCuf,    {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufF,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufSafe,{StackTop2|DontGuardAny,
                                      StackTop2|FStack, OutFPushCufSafe,
                                                         kNumActRecCells }},
  { OpFPassCW,     {FuncdRef,         None,         OutSameAsInput,    0 }},
  { OpFPassCE,     {FuncdRef,         None,         OutSameAsInput,    0 }},
  { OpFPassV,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassL,      {Local|FuncdRef,   Stack1,       OutFInputL,        1 }},
  { OpFPassN,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassS,      {StackTop2|FuncdRef,
                                      Stack1,       OutUnknown,       -1 }},
  { OpFPassM,      {MVector|FuncdRef, Stack1|Local, OutUnknown,        1 }},
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallD,      {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallUnpack, {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallArray,  {FStack,           Stack1,       OutPred,
                                                   -(int)kNumActRecCells }},
  // TODO: output type is known
  { OpFCallBuiltin,{BStackN,          Stack1,       OutPred,          0 }},
  { OpCufSafeArray,{StackTop3|DontGuardAny,
                                      Stack1,       OutArray,         -2 }},
  { OpCufSafeReturn,{StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown,       -2 }},
  { OpDecodeCufIter,{Stack1,          None,         OutNone,          -1 }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInit,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpWIterInit,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterInitK,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInitK,  {Stack1,           Local,        OutUnknown,       -1 }},
  { OpWIterInitK,  {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterNext,    {None,             Local,        OutUnknown,        0 }},
  { OpMIterNext,   {None,             Local,        OutUnknown,        0 }},
  { OpWIterNext,   {None,             Local,        OutUnknown,        0 }},
  { OpIterNextK,   {None,             Local,        OutUnknown,        0 }},
  { OpMIterNextK,  {None,             Local,        OutUnknown,        0 }},
  { OpWIterNextK,  {None,             Local,        OutUnknown,        0 }},
  { OpIterFree,    {None,             None,         OutNone,           0 }},
  { OpMIterFree,   {None,             None,         OutNone,           0 }},
  { OpCIterFree,   {None,             None,         OutNone,           0 }},
  { OpIterBreak,   {None,             None,         OutNone,           0 }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpDefFunc,     {None,             None,         OutNone,           0 }},
  { OpDefTypeAlias,{None,             None,         OutNone,           0 }},
  { OpDefCls,      {None,             None,         OutNone,           0 }},
  { OpDefCns,      {Stack1,           Stack1,       OutBoolean,        0 }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject,     1 }},
  { OpBareThis,    {None,             Stack1,       OutUnknown,        1 }},
  { OpCheckThis,   {This,             None,         OutNone,           0 }},
  { OpInitThisLoc,
                   {None,             Local,        OutUnknown,        0 }},
  { OpStaticLoc,
                   {None,             Stack1,       OutBoolean,        1 }},
  { OpStaticLocInit,
                   {Stack1,           Local,        OutVUnknown,      -1 }},
  { OpCatch,       {None,             Stack1,       OutObject,         1 }},
  { OpVerifyParamType,
                   {Local,            Local,        OutUnknown,        0 }},
  { OpVerifyRetTypeV,
                   {Stack1,           Stack1,       OutSameAsInput,    0 }},
  { OpVerifyRetTypeC,
                   {Stack1,           Stack1,       OutSameAsInput,    0 }},
  { OpOODeclExists,
                   {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpSelf,        {None,             Stack1,       OutClassRef,       1 }},
  { OpParent,      {None,             Stack1,       OutClassRef,       1 }},
  { OpLateBoundCls,{None,             Stack1,       OutClassRef,       1 }},
  { OpNativeImpl,  {None,             None,         OutNone,           0 }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject,         1 }},
  { OpStrlen,      {Stack1,           Stack1,       OutStrlen,         0 }},
  { OpIncStat,     {None,             None,         OutNone,           0 }},
  { OpIdx,         {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpFloor,       {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCeil,        {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCheckProp,   {None,             Stack1,       OutBoolean,        1 }},
  { OpInitProp,    {Stack1,           None,         OutNone,          -1 }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone,           0 }},
  { OpAssertRATL,  {None,             None,         OutNone,           0 }},
  { OpAssertRATStk,{None,             None,         OutNone,           0 }},
  { OpBreakTraceHint,{None,           None,         OutNone,           0 }},

  /*** 14. Generator instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutNull,           1 }},
  { OpContEnter,   {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpContRaise,   {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpYield,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpYieldK,      {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpContCheck,   {None,             None,         OutNone,           0 }},
  { OpContValid,   {None,             Stack1,       OutBoolean,        1 }},
  { OpContKey,     {None,             Stack1,       OutUnknown,        1 }},
  { OpContCurrent, {None,             Stack1,       OutUnknown,        1 }},

  /*** 15. Async functions instructions ***/

  { OpAwait,       {Stack1,           Stack1,       OutUnknown,        0 }},
};

static hphp_hash_map<Op, InstrInfo> instrInfo;
static bool instrInfoInited;
static void initInstrInfo() {
  if (!instrInfoInited) {
    for (size_t i = 0; i < sizeof(instrInfoSparse) / sizeof(instrInfoSparse[0]);
         i++) {
      instrInfo[instrInfoSparse[i].op] = instrInfoSparse[i].info;
    }
    if (!RuntimeOption::EvalCheckReturnTypeHints) {
      for (size_t j = 0; j < 2; ++j) {
        auto& ii = instrInfo[j == 0 ? OpVerifyRetTypeC : OpVerifyRetTypeV];
        ii.in = ii.out = None;
        ii.type = OutNone;
      }
    }
    instrInfoInited = true;
  }
}

const InstrInfo& getInstrInfo(Op op) {
  assert(instrInfoInited);
  return instrInfo[op];
}

static int numHiddenStackInputs(const NormalizedInstruction& ni) {
  assert(ni.immVec.isValid());
  return ni.immVec.numStackValues();
}

namespace {
int64_t countOperands(uint64_t mask) {
  const uint64_t ignore = FuncdRef | Local | Iter | AllLocals |
    DontGuardStack1 | IgnoreInnerType | DontGuardAny | This;
  mask &= ~ignore;

  static const uint64_t counts[][2] = {
    {Stack3,       1},
    {Stack2,       1},
    {Stack1,       1},
    {StackIns1,    2},
    {StackIns2,    3},
    {FStack,       kNumActRecCells},
  };

  int64_t count = 0;
  for (auto const& pair : counts) {
    if (mask & pair[0]) {
      count += pair[1];
      mask &= ~pair[0];
    }
  }
  assert(mask == 0);
  return count;
}
}

int64_t getStackPopped(PC pc) {
  auto const op = *reinterpret_cast<const Op*>(pc);
  switch (op) {
    case Op::FCall:        return getImm((Op*)pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallD:       return getImm((Op*)pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallArray:   return kNumActRecCells + 1;

    case Op::NewPackedArray:
    case Op::ConcatN:
    case Op::FCallBuiltin:
    case Op::CreateCl:     return getImm((Op*)pc, 0).u_IVA;

    case Op::NewStructArray: return getImmVector((Op*)pc).size();

    default:             break;
  }

  uint64_t mask = getInstrInfo(op).in;
  int64_t count = 0;

  // All instructions with these properties are handled above
  assert((mask & (StackN | BStackN)) == 0);

  if (mask & MVector) {
    count += getImmVector((Op*)pc).numStackValues();
    mask &= ~MVector;
  }

  return count + countOperands(mask);
}

int64_t getStackPushed(PC pc) {
  return countOperands(getInstrInfo(*reinterpret_cast<const Op*>(pc)).out);
}

int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  auto op = ni.op();
  switch (op) {
    case Op::FCall:
    case Op::FCallD:
      {
        int numArgs = ni.imm[0].u_IVA;
        return 1 - numArgs - kNumActRecCells;
      }

    case Op::NewPackedArray:
    case Op::ConcatN:
    case Op::FCallBuiltin:
    case Op::CreateCl:
      return 1 - ni.imm[0].u_IVA;

    case Op::NewStructArray:
      return 1 - ni.immVec.numStackValues();

    default:
      break;
  }
  const InstrInfo& info = instrInfo[op];
  if (info.in & MVector) {
    hiddenStackInputs = numHiddenStackInputs(ni);
    SKTRACE(2, ni.source, "Has %d hidden stack inputs\n", hiddenStackInputs);
  }
  int delta = instrInfo[op].numPushed - hiddenStackInputs;
  return delta;
}

// Task #3449943: This returns true even if there's meta-data telling
// that the value was inferred.
bool outputIsPredicted(NormalizedInstruction& inst) {
  auto const& iInfo = getInstrInfo(inst.op());
  auto doPrediction =
    (iInfo.type == OutPred || iInfo.type == OutCns) && !inst.endsRegion;
  if (doPrediction) {
    // All OutPred ops except for SetM have a single stack output for now.
    assert(iInfo.out == Stack1 || inst.op() == OpSetM);
    auto dt = predictOutputs(&inst);
    if (dt != KindOfAny) {
      inst.outPred = Type(dt, dt == KindOfRef ? KindOfAny : KindOfNone);
      inst.outputPredicted = true;
    } else {
      doPrediction = false;
    }
  }

  return doPrediction;
}

bool isAlwaysNop(Op op) {
  switch (op) {
  case Op::BoxRNop:
  case Op::DefClsNop:
  case Op::FPassC:
  case Op::FPassVNop:
  case Op::Nop:
  case Op::UnboxRNop:
    return true;
  case Op::VerifyRetTypeC:
  case Op::VerifyRetTypeV:
    return !RuntimeOption::EvalCheckReturnTypeHints;
  default:
    return false;
  }
}

static void addMVectorInputs(NormalizedInstruction& ni,
                             int& currentStackOffset,
                             std::vector<InputInfo>& inputs) {
  assert(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  currentStackOffset -= ni.immVec.numStackValues();
  int localStackOffset = currentStackOffset;

  auto push_stack = [&] {
    ++stackCount;
    inputs.emplace_back(Location(Location::Stack, localStackOffset++));
  };
  auto push_local = [&] (int imm) {
    ++localCount;
    inputs.emplace_back(Location(Location::Local, imm));
  };

  /*
   * Note that we have to push as we go so that the arguments come in
   * the order expected for the M-vector.
   */

  /*
   * Also note: if we eventually have immediates that are not local
   * ids (i.e. string ids), this analysis step is going to have to be
   * a bit wiser.
   */
  auto opPtr = (const Op*)ni.source.pc();
  auto const location = getMLocation(opPtr);
  auto const lcode = location.lcode;

  const bool trailingClassRef = lcode == LSL || lcode == LSC;

  switch (numLocationCodeStackVals(lcode)) {
  case 0: {
    if (lcode == LH) {
      inputs.emplace_back(Location(Location::This));
    } else {
      assert(lcode == LL || lcode == LGL || lcode == LNL);
      if (location.hasImm()) {
        push_local(location.imm);
      }
    }
  } break;
  case 1:
    if (lcode == LSL) {
      // We'll get the trailing stack value after pushing all the
      // member vector elements.
      assert(location.hasImm());
      push_local(location.imm);
    } else {
      push_stack();
    }
    break;
  case 2:
    push_stack();
    if (!trailingClassRef) {
      // This one is actually at the back.
      push_stack();
    }
    break;
  default: not_reached();
  }

  // Now push all the members in the correct order.
  for (auto const& member : getMVector(opPtr)) {
    auto const mcode = member.mcode;
    ni.immVecM.push_back(mcode);

    if (mcode == MW) {
      // No stack and no locals.
      continue;
    } else if (member.hasImm()) {
      int64_t imm = member.imm;
      if (memberCodeImmIsLoc(mcode)) {
        push_local(imm);
      } else if (memberCodeImmIsString(mcode)) {
        inputs.emplace_back(Location(Location::Litstr, imm));
      } else {
        assert(memberCodeImmIsInt(mcode));
        inputs.emplace_back(Location(Location::Litint, imm));
      }
    } else {
      push_stack();
    }
    inputs.back().dontGuardInner = true;
  }

  if (trailingClassRef) {
    push_stack();
  }

  assert(stackCount == ni.immVec.numStackValues());

  SKTRACE(2, ni.source, "M-vector using %d hidden stack "
                        "inputs, %d locals\n", stackCount, localCount);
}

/*
 * getInputsImpl --
 *   Returns locations for this instruction's inputs.
 *
 * Throws:
 *   TranslationFailedExc:
 *     Unimplemented functionality, probably an opcode.
 *
 *   UnknownInputExc:
 *     Consumed a datum whose type or value could not be constrained at
 *     translation time, because the tracelet has already modified it.
 *     Truncate the tracelet at the preceding instruction, which must
 *     exists because *something* modified something in it.
 */
static
void getInputsImpl(SrcKey startSk,
                   NormalizedInstruction* ni,
                   int& currentStackOffset,
                   InputInfoVec& inputs,
                   const LocalTypeFn& localType) {
#ifdef USE_TRACE
  const SrcKey& sk = ni->source;
#endif
  if (isAlwaysNop(ni->op())) return;

  assert(inputs.empty());
  always_assert_flog(
    instrInfo.count(ni->op()),
    "Invalid opcode in getInputsImpl: {}\n",
    opcodeToName(ni->op())
  );
  const InstrInfo& info = instrInfo[ni->op()];
  Operands input = info.in;
  if (input & FuncdRef) {
    inputs.needsRefCheck = true;
  }
  if (input & Iter) {
    inputs.emplace_back(Location(Location::Iter, ni->imm[0].u_IVA));
  }
  if (input & FStack) {
    currentStackOffset -= ni->imm[0].u_IVA; // arguments consumed
    currentStackOffset -= kNumActRecCells; // ActRec is torn down as well
  }
  if (input & IgnoreInnerType) ni->ignoreInnerType = true;
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", currentStackOffset - 1);
    inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      }
    }
  }
  if (input & StackN) {
    int numArgs = (ni->op() == Op::NewPackedArray ||
                   ni->op() == Op::ConcatN)
      ? ni->imm[0].u_IVA
      : ni->immVec.numStackValues();

    SKTRACE(1, sk, "getInputs: stackN %d %d\n",
            currentStackOffset - 1, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (input & BStackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    }
  }
  if (input & MVector) {
    addMVectorInputs(*ni, currentStackOffset, inputs);
  }
  if (input & Local) {
    // (Almost) all instructions that take a Local have its index at
    // their first immediate.
    int loc;
    auto insertAt = inputs.end();
    switch (ni->op()) {
      case OpSetWithRefLM:
        insertAt = inputs.begin();
        // fallthrough
      case OpFPassL:
        loc = ni->imm[1].u_IVA;
        break;

      default:
        loc = ni->imm[0].u_IVA;
        break;
    }
    SKTRACE(1, sk, "getInputs: local %d\n", loc);
    inputs.emplace(insertAt, Location(Location::Local, loc));
  }

  auto wantInlineReturn = [&] {
    const int localCount = ni->func()->numLocals();
    // Inline return causes us to guard this tracelet more precisely. If
    // we're already chaining to get here, just do a generic return in the
    // hopes of avoiding further specialization. The localCount constraint
    // is an unfortunate consequence of the current generic machinery not
    // working for 0 locals.
    if (mcg->numTranslations(startSk) >= kTooPolyRet && localCount > 0) {
      return false;
    }
    int numRefCounted = 0;
    for (int i = 0; i < localCount; ++i) {
      if (localType(i).maybeCounted()) {
        numRefCounted++;
      }
    }
    return numRefCounted <= RuntimeOption::EvalHHIRInliningMaxReturnDecRefs;
  };

  if ((input & AllLocals) && wantInlineReturn()) {
    ni->inlineReturn = true;
    ni->ignoreInnerType = true;
    int n = ni->func()->numLocals();
    for (int i = 0; i < n; ++i) {
      inputs.emplace_back(Location(Location::Local, i));
    }
  }

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", currentStackOffset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if (inputs.size() &&
      ((input & DontGuardAny) || dontGuardAnyInputs(ni->op()))) {
    for (int i = inputs.size(); i--; ) {
      inputs[i].dontGuard = true;
    }
  }
  if (input & This) {
    inputs.emplace_back(Location(Location::This));
  }
}

void getInputs(SrcKey startSk, NormalizedInstruction& inst,
               InputInfoVec& infos, const LocalTypeFn& localType) {
  // MCGenerator expected top of stack to be index -1, with indexes growing
  // down from there. hhir defines top of stack to be index 0, with indexes
  // growing up from there. To compensate we start with a stack offset of 1 and
  // negate the index of any stack input after the call to getInputs.
  int stackOff = 1;
  getInputsImpl(startSk, &inst, stackOff, infos, localType);
  for (auto& info : infos) {
    if (info.loc.isStack()) info.loc.offset = -info.loc.offset;
  }
}

bool dontGuardAnyInputs(Op op) {
  switch (op) {
#define CASE(iNm) case Op ## iNm:
#define NOOP(...)
  INSTRS
  PSEUDOINSTR_DISPATCH(NOOP)
    return false;

  default:
    return true;
  }
#undef NOOP
#undef CASE
}

bool outputDependsOnInput(const Op op) {
  switch (instrInfo[op].type) {
    case OutNull:
    case OutNullUninit:
    case OutString:
    case OutStringImm:
    case OutDouble:
    case OutBoolean:
    case OutBooleanImm:
    case OutPredBool:
    case OutInt64:
    case OutArray:
    case OutArrayImm:
    case OutObject:
    case OutResource:
    case OutThisObject:
    case OutUnknown:
    case OutVUnknown:
    case OutClassRef:
    case OutPred:
    case OutCns:
    case OutStrlen:
    case OutNone:
      return false;

    // NB: this sounds like it should have the output depend on the
    // input, but it behaves the same as OutBoolean unless we are
    // inlining, and in that case we don't relaxDeps.
    case OutIsTypeL:
      return false;

    case OutFDesc:
    case OutSameAsInput:
    case OutCInput:
    case OutVInput:
    case OutCInputL:
    case OutVInputL:
    case OutFInputL:
    case OutFInputR:
    case OutArith:
    case OutArithO:
    case OutBitOp:
    case OutSetOp:
    case OutIncDec:
    case OutFPushCufSafe:
      return true;
  }
  not_reached();
}

const StaticString s_http_response_header("http_response_header");
const StaticString s_php_errormsg("php_errormsg");
const StaticString s_extract("extract");
const StaticString s_extractNative("__SystemLib\\extract");
const StaticString s_parse_str("parse_str");
const StaticString s_parse_strNative("__SystemLib\\parse_str");

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  auto locals = caller->localNames();
  for (int i = 0; i < caller->numNamedLocals(); ++i) {
    if (locals[i]->same(s_http_response_header.get()) ||
        locals[i]->same(s_php_errormsg.get())) {
      return true;
    }
  }

  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);
    return str->isame(s_extract.get()) ||
           str->isame(s_extractNative.get()) ||
           str->isame(s_parse_str.get()) ||
           str->isame(s_parse_strNative.get());
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assert(fpi);
  Op* fpushPc = (Op*)unit->at(fpi->m_fpushOff);
  auto const op = *fpushPc;

  if (op == OpFPushFunc) {
    // If the call has any arguments, the FPushFunc will be in a different
    // tracelet -- the tracelet will break on every FPass* because the reffiness
    // of the callee isn't knowable. So we have to say the call destroys locals,
    // to be conservative. If there aren't any arguments, then it can't destroy
    // locals -- even if the call is to extract(), there's no argument, so it
    // won't do anything.
    auto const numArgs = inst.imm[0].u_IVA;
    return (numArgs != 0);
  }
  if (op == OpFPushFuncD) return checkTaintId(getImm(fpushPc, 1).u_SA);
  if (op == OpFPushFuncU) {
    return checkTaintId(getImm(fpushPc, 1).u_SA) ||
           checkTaintId(getImm(fpushPc, 2).u_SA);
  }

  return false;
}

bool instrBreaksProfileBB(const NormalizedInstruction* inst) {
  if (instrIsNonCallControlFlow(inst->op()) ||
      inst->outputPredicted ||
      inst->op() == OpAwait || // may branch to scheduler and suspend execution
      inst->op() == OpClsCnsD) { // side exits if misses in the RDS
    return true;
  }
  // In profiling mode, don't trace through a control flow merge point
  if (inst->func()->anyBlockEndsAt(inst->offset())) {
    return true;
  }
  return false;
}

Translator::Translator()
  : uniqueStubs{}
  , m_createdTime(HPHP::Timer::GetCurrentTimeMicros())
  , m_mode(TransKind::Invalid)
  , m_profData(nullptr)
  , m_useAHot(RuntimeOption::RepoAuthoritative &&
              RuntimeOption::EvalJitAHotSize > 0)
{
  initInstrInfo();
  if (RuntimeOption::EvalJitPGO) {
    m_profData.reset(new ProfData());
  }
}

bool
Translator::isSrcKeyInBL(const SrcKey& sk) {
  auto unit = sk.unit();
  if (unit->isInterpretOnly()) return true;
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLSrcKey.find(sk) != m_dbgBLSrcKey.end()) {
    return true;
  }

  // Loop until the end of the basic block inclusively. This is useful for
  // function exit breakpoints, which are implemented by blacklisting the RetC
  // opcodes.
  PC pc = nullptr;
  do {
    pc = (pc == nullptr) ?
      unit->at(sk.offset()) : pc + instrLen((Op*) pc);
    if (m_dbgBLPC.checkPC(pc)) {
      m_dbgBLSrcKey.insert(sk);
      return true;
    }
  } while (!opcodeBreaksBB(*reinterpret_cast<const Op*>(pc)));
  return false;
}

void
Translator::clearDbgBL() {
  Lock l(m_dbgBlacklistLock);
  m_dbgBLSrcKey.clear();
  m_dbgBLPC.clear();
}

bool
Translator::addDbgBLPC(PC pc) {
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLPC.checkPC(pc)) {
    // already there
    return false;
  }
  m_dbgBLPC.addPC(pc);
  return true;
}

const char* Translator::ResultName(TranslateResult r) {
  static const char* const names[] = {
    "Failure",
    "Retry",
    "Success",
  };
  return names[r];
}

bool instrMustInterp(const NormalizedInstruction& inst) {
  if (RuntimeOption::EvalJitAlwaysInterpOne) return true;

  switch (inst.op()) {
    // Generate a case for each instruction we support at least partially.
# define CASE(name) case Op::name:
  INSTRS
# undef CASE
# define NOTHING(...) // PSEUDOINSTR_DISPATCH has the cases in it
  PSEUDOINSTR_DISPATCH(NOTHING)
# undef NOTHING
      return false;

    default:
      return true;
  }
}

void Translator::traceStart(TransContext context) {
  assert(!m_irTrans);

  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         " HHIR during translation ",
         color(ANSI_COLOR_END));

  m_irTrans.reset(new IRTranslator(context));
}

void Translator::traceEnd() {
  assert(!m_irTrans->hhbcTrans().isInlining());
  m_irTrans->hhbcTrans().end();
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         "",
         color(ANSI_COLOR_END));
}

void Translator::traceFree() {
  FTRACE(1, "HHIR free: arena size: {}\n",
         m_irTrans->hhbcTrans().unit().arena().size());
  m_irTrans.reset();
}

/*
 * Create two maps for all blocks in the region:
 *   - a map from RegionDesc::BlockId -> IR Block* for all region blocks
 *   - a map from RegionDesc::BlockId -> RegionDesc::Block
 */
void Translator::createBlockMaps(const RegionDesc&        region,
                                 BlockIdToIRBlockMap&     blockIdToIRBlock,
                                 BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  IRBuilder& irb = ht.irBuilder();
  blockIdToIRBlock.clear();
  blockIdToRegionBlock.clear();
  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    RegionDesc::Block* rBlock = blocks[i].get();
    auto id = rBlock->id();
    DEBUG_ONLY Offset bcOff = rBlock->start().offset();
    assert(IMPLIES(i == 0, bcOff == irb.unit().bcOff()));
    Block* iBlock = i == 0 ? irb.unit().entry()
                           : irb.unit().defBlock();
    blockIdToIRBlock[id]     = iBlock;
    blockIdToRegionBlock[id] = rBlock;
    FTRACE(1,
           "createBlockMaps: RegionBlock {} => IRBlock {} (BC offset = {})\n",
           id, iBlock->id(), bcOff);
  }
}

/*
 * Set IRBuilder's Block associated to blockId's block according to
 * the mapping in blockIdToIRBlock.
 */
void Translator::setIRBlock(RegionDesc::BlockId            blockId,
                            const BlockIdToIRBlockMap&     blockIdToIRBlock,
                            const BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  IRBuilder& irb = m_irTrans->hhbcTrans().irBuilder();
  auto rit = blockIdToRegionBlock.find(blockId);
  assert(rit != blockIdToRegionBlock.end());
  RegionDesc::Block* rBlock = rit->second;

  Offset bcOffset = rBlock->start().offset();

  auto iit = blockIdToIRBlock.find(blockId);
  assert(iit != blockIdToIRBlock.end());

  assert(!irb.hasBlock(bcOffset));
  FTRACE(3, "  setIRBlock: blockId {}, offset {} => IR Block {}\n",
         blockId, bcOffset, iit->second->id());
  irb.setBlock(bcOffset, iit->second);
}

/*
 * Set IRBuilder's Blocks for srcBlockId's successors' offsets within
 * the region.
 */
void Translator::setSuccIRBlocks(
  const RegionDesc&              region,
  RegionDesc::BlockId            srcBlockId,
  const BlockIdToIRBlockMap&     blockIdToIRBlock,
  const BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  FTRACE(3, "setSuccIRBlocks: srcBlockId = {}\n", srcBlockId);
  IRBuilder& irb = m_irTrans->hhbcTrans().irBuilder();
  irb.resetOffsetMapping();
  for (auto dstBlockId : region.succs(srcBlockId)) {
    setIRBlock(dstBlockId, blockIdToIRBlock, blockIdToRegionBlock);
  }
}

/*
 * Compute the set of bytecode offsets that may follow the execution
 * of srcBlockId in the region.
 */
static void findSuccOffsets(const RegionDesc&              region,
                            RegionDesc::BlockId            srcBlockId,
                            const BlockIdToRegionBlockMap& blockIdToRegionBlock,
                            OffsetSet&                     set) {
  set.clear();
  for (auto dstBlockId : region.succs(srcBlockId)) {
    auto rit = blockIdToRegionBlock.find(dstBlockId);
    assert(rit != blockIdToRegionBlock.end());
    RegionDesc::Block* rDstBlock = rit->second;
    Offset bcOffset = rDstBlock->start().offset();
    set.insert(bcOffset);
  }
}

/*
 * Returns whether offset is a control-flow merge within region.
 */
static bool isMergePoint(Offset offset, const RegionDesc& region) {
  for (auto const block : region.blocks()) {
    auto const bid = block->id();
    if (block->start().offset() == offset) {
      auto inCount = region.preds(bid).size();
      // NB: The initial block has an invisible "entry arc".
      if (block == region.entry()) ++inCount;
      if (inCount >= 2) return true;
    }
  }
  return false;
}

static bool blockHasUnprocessedPred(
  const RegionDesc&             region,
  RegionDesc::BlockId           blockId,
  const RegionDesc::BlockIdSet& processedBlocks)
{
  for (auto predId : region.preds(blockId)) {
    if (processedBlocks.count(predId) == 0) {
      return true;
    }
  }
  return false;
}

/*
 * Returns whether any instruction following inst (whether by fallthrough or
 * branch target) is a merge in region.
 */
static bool nextIsMerge(const NormalizedInstruction& inst,
                        const RegionDesc& region) {
  Offset fallthruOffset = inst.offset() + instrLen((Op*)(inst.pc()));
  if (instrHasConditionalBranch(inst.op())) {
    auto offsetPtr = instrJumpOffset((Op*)inst.pc());
    Offset takenOffset = inst.offset() + *offsetPtr;
    return fallthruOffset == takenOffset
      || isMergePoint(takenOffset, region)
      || isMergePoint(fallthruOffset, region);
  }
  if (isUnconditionalJmp(inst.op())) {
    auto offsetPtr = instrJumpOffset((Op*)inst.pc());
    Offset takenOffset = inst.offset() + *offsetPtr;
    return isMergePoint(takenOffset, region);
  }
  return isMergePoint(fallthruOffset, region);
}

Translator::TranslateResult
Translator::translateRegion(const RegionDesc& region,
                            bool bcControlFlow,
                            RegionBlacklist& toInterp,
                            TransFlags trflags) {
  const Timer translateRegionTimer(Timer::translateRegion);
  FTRACE(1, "translateRegion starting with:\n{}\n", show(region));

  m_region = &region;
  SCOPE_EXIT { m_region = nullptr; };

  std::string errorMsg;
  always_assert_log(check(region, errorMsg),
                    [&] { return errorMsg + "\n" + show(region); });

  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  IRBuilder& irb = ht.irBuilder();
  auto const startSk = region.start();

  BlockIdToIRBlockMap     blockIdToIRBlock;
  BlockIdToRegionBlockMap blockIdToRegionBlock;

  if (bcControlFlow) {
    ht.setGenMode(IRGenMode::CFG);
    createBlockMaps(region, blockIdToIRBlock, blockIdToRegionBlock);
  }

  RegionDesc::BlockIdSet processedBlocks;

  Timer irGenTimer(Timer::translateRegion_irGeneration);
  auto& blocks = region.blocks();
  for (auto b = 0; b < blocks.size(); b++) {
    auto const& block  = blocks[b];
    auto const blockId = block->id();
    auto sk            = block->start();
    auto typePreds     = makeMapWalker(block->typePreds());
    auto byRefs        = makeMapWalker(block->paramByRefs());
    auto refPreds      = makeMapWalker(block->reffinessPreds());
    auto knownFuncs    = makeMapWalker(block->knownFuncs());
    auto skipTrans     = false;

    const Func* topFunc = nullptr;
    TransID profTransId = getTransId(blockId);
    ht.setProfTransID(profTransId);

    OffsetSet succOffsets;
    if (ht.genMode() == IRGenMode::CFG) {
      Block* irBlock = blockIdToIRBlock[blockId];
      if (blockHasUnprocessedPred(region, blockId, processedBlocks)) {
        always_assert(RuntimeOption::EvalJitLoops ||
                      RuntimeOption::EvalJitPGORegionSelector == "wholecfg");
        irb.clearBlockState(irBlock);
      }
      BCMarker marker(sk, block->initialSpOffset(), profTransId);
      ht.irBuilder().startBlock(irBlock, marker);
      findSuccOffsets(region, blockId, blockIdToRegionBlock, succOffsets);
      setSuccIRBlocks(region, blockId, blockIdToIRBlock, blockIdToRegionBlock);
    }

    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      ht.setBcOff(sk.offset(), false);

      // Emit prediction guards. If this is the first instruction in the region
      // the guards will go to a retranslate request. Otherwise, they'll go to
      // a side exit.
      bool isFirstRegionInstr = (block == region.entry() && i == 0);
      if (isFirstRegionInstr) ht.emitRB(Trace::RBTypeTraceletGuards, sk);

      // Emit type guards.
      while (typePreds.hasNext(sk)) {
        auto const& pred = typePreds.next();
        auto type = pred.type;
        auto loc  = pred.location;
        if (type <= Type::Cls) {
          // Do not generate guards for class; instead assert the type.
          assert(loc.tag() == RegionDesc::Location::Tag::Stack);
          ht.assertType(loc, type);
        } else if (isFirstRegionInstr) {
          bool checkOuterTypeOnly = m_mode != TransKind::Profile;
          ht.guardTypeLocation(loc, type, checkOuterTypeOnly);
        } else {
          ht.checkType(loc, type, sk.offset());
        }
      }

      // Emit reffiness guards. For now, we only support reffiness guards at
      // the beginning of the region.
      while (refPreds.hasNext(sk)) {
        assert(sk == startSk);
        auto const& pred = refPreds.next();
        ht.guardRefs(pred.arSpOffset, pred.mask, pred.vals);
      }

      // Finish emitting guards, and emit profiling counters.
      if (isFirstRegionInstr) {
        ht.endGuards();
        if (RuntimeOption::EvalJitTransCounters) {
          ht.emitIncTransCounter();
        }

        if (m_mode == TransKind::Profile) {
          if (block->func()->isEntry(block->start().offset())) {
            ht.emitCheckCold(m_profData->curTransID());
          } else {
            ht.emitIncProfCounter(m_profData->curTransID());
          }
        }
        ht.emitRB(Trace::RBTypeTraceletBody, sk);
      }

      // Update the current funcd, if we have a new one.
      if (knownFuncs.hasNext(sk)) {
        topFunc = knownFuncs.next();
      }

      // Create and initialize the instruction.
      NormalizedInstruction inst(sk, block->unit());
      inst.funcd = topFunc;
      if (i == block->length() - 1) {
        inst.endsRegion = region.isExit(blockId);
        inst.nextIsMerge = nextIsMerge(inst, region);
        if (instrIsNonCallControlFlow(inst.op()) &&
            b < blocks.size() - 1) {
          inst.nextOffset = blocks[b+1]->start().offset();
        }
      }

      // We can get a more precise output type for interpOne if we know all of
      // its inputs, so we still populate the rest of the instruction even if
      // this is true.
      inst.interp = toInterp.count(ProfSrcKey{profTransId, sk});

      InputInfoVec inputInfos;
      getInputs(startSk, inst, inputInfos, [&] (int i) {
        return irb.localType(i, DataTypeGeneric);
      });

      // Populate the NormalizedInstruction's input vector, using types from
      // HhbcTranslator.
      std::vector<DynLocation> dynLocs;
      dynLocs.reserve(inputInfos.size());
      auto newDynLoc = [&] (const InputInfo& ii) {
        dynLocs.emplace_back(ii.loc, ht.typeFromLocation(ii.loc));
        FTRACE(2, "typeFromLocation: {} -> {}\n",
               ii.loc.pretty(), dynLocs.back().rtt);
        return &dynLocs.back();
      };
      FTRACE(2, "populating inputs for {}\n", inst.toString());
      for (auto const& ii : inputInfos) {
        inst.inputs.push_back(newDynLoc(ii));
      }
      if (inputInfos.needsRefCheck) {
        assert(byRefs.hasNext(sk));
        inst.preppedByRef = byRefs.next();
      }

      /*
       * Check for a type prediction. Put it in the
       * NormalizedInstruction so the emit* method can use it if
       * needed.  In PGO mode, we don't really need the values coming
       * from the interpreter type profiler.  TransKind::Profile
       * translations end whenever there's a side-exit, and type
       * predictions incur side-exits.  And when we stitch multiple
       * TransKind::Profile translations together to form a larger
       * region (in TransKind::Optimize mode), the guard for the top
       * of the stack essentially does the role of type prediction.
       * And, if the value is also inferred, then the guard is
       * omitted.
       */
      auto const doPrediction = mode() == TransKind::Live &&
                                  outputIsPredicted(inst);

      // If this block ends with an inlined FCall, we don't emit anything for
      // the FCall and instead set up HhbcTranslator for inlining. Blocks from
      // the callee will be next in the region.
      if (i == block->length() - 1 &&
          (inst.op() == Op::FCall || inst.op() == Op::FCallD) &&
          block->inlinedCallee()) {
        auto const* callee = block->inlinedCallee();
        FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
               "and stack:\n{}\n",
               block->func()->fullName()->data(),
               callee->fullName()->data(),
               inst.imm[0].u_IVA,
               ht.showStack());
        auto returnSk = inst.nextSk();
        auto returnFuncOff = returnSk.offset() - block->func()->base();
        ht.beginInlining(inst.imm[0].u_IVA, callee, returnFuncOff,
                         doPrediction ? inst.outPred : Type::Gen);
        // "Fallthrough" into the callee's first block
        ht.endBlock(blocks[b + 1]->start().offset(), inst.nextIsMerge);
        continue;
      }

      // Singleton inlining optimization.
      if (RuntimeOption::EvalHHIRInlineSingletons) {
        bool didInlineSingleton = [&] {
          if (!RuntimeOption::RepoAuthoritative) return false;

          // I don't really want to inline my arm, thanks.
          if (arch() != Arch::X64) return false;

          // Don't inline if we're retranslating due to a side-exit from an
          // inlined call.
          if (trflags.noinlineSingleton && startSk == inst.source) return false;

          // Bail early if this isn't a push.
          if (inst.op() != Op::FPushFuncD &&
              inst.op() != Op::FPushClsMethodD) {
            return false;
          }

          // ...and also if this is the end of the block.
          if (i == block->length() - 1) return false;

          auto nextSK = inst.nextSk();

          // If the normal machinery is already inlining this function, don't
          // do anything here.
          if (i == block->length() - 2 &&
              (nextSK.op() == Op::FCall || nextSK.op() == Op::FCallD) &&
              block->inlinedCallee()) {
            return false;
          }

          // This is safe to do even if singleton inlining fails; we just won't
          // change topFunc in the next pass since hasNext() will return false.
          if (knownFuncs.hasNext(nextSK)) {
            topFunc = knownFuncs.next();

            // Detect a singleton pattern and inline it if found.
            return m_irTrans->tryTranslateSingletonInline(inst, topFunc);
          }

          return false;
        }();

        // Skip the translation of this instruction (the FPush) -and- the next
        // instruction (the FCall) if we succeeded at singleton inlining.  We
        // still want the fallthrough and prediction logic, though.
        if (didInlineSingleton) {
          skipTrans = true;
          continue;
        }
      }

      // Emit IR for the body of the instruction.
      try {
        if (!skipTrans) m_irTrans->translateInstr(inst);
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk{profTransId, sk};
        always_assert_log(
          !toInterp.count(psk),
          [&] {
            std::ostringstream oss;
            oss << folly::format("IR generation failed with {}\n", exn.what());
            print(oss, m_irTrans->hhbcTrans().unit());
            return oss.str();
          });
        toInterp.insert(psk);
        return Retry;
      }

      skipTrans = false;

      // Insert a fallthrough jump
      if (ht.genMode() == IRGenMode::CFG &&
          i == block->length() - 1 && block != blocks.back()) {
        if (instrAllowsFallThru(inst.op())) {
          auto nextOffset = inst.nextOffset != kInvalidOffset
            ? inst.nextOffset
            : inst.offset() + instrLen((Op*)(inst.pc()));
          // prepareForSideExit is done later in Trace mode, but it
          // needs to happen here or else we generate the SpillStack
          // after the fallthrough jump, which is just weird.
          if (b < blocks.size() - 1 && region.isSideExitingBlock(blockId)) {
            ht.prepareForSideExit();
          }
          ht.endBlock(nextOffset, inst.nextIsMerge);
        } else if (isRet(inst.op()) || inst.op() == OpNativeImpl) {
          // "Fallthrough" from inlined return to the next block
          ht.endBlock(blocks[b + 1]->start().offset(), inst.nextIsMerge);
        }
        if (region.isExit(blockId)) {
          ht.end();
        }
      }

      // Check the prediction. If the predicted type is less specific than what
      // is currently on the eval stack, checkType won't emit any code.
      if (doPrediction && ht.topType(0, DataTypeGeneric).maybe(inst.outPred)) {
        ht.checkTypeStack(0, inst.outPred,
                          sk.advanced(block->unit()).offset());
      }
    }

    if (ht.genMode() == IRGenMode::Trace) {
      if (b < blocks.size() - 1 && region.isSideExitingBlock(blockId)) {
        ht.prepareForSideExit();
      }
    }

    processedBlocks.insert(blockId);

    assert(!typePreds.hasNext());
    assert(!byRefs.hasNext());
    assert(!refPreds.hasNext());
    assert(!knownFuncs.hasNext());
  }

  traceEnd();
  irGenTimer.end();

  try {
    mcg->traceCodeGen();
    if (m_mode == TransKind::Profile) {
      profData()->setProfiling(startSk.func()->getFuncId());
    }
  } catch (const FailedCodeGen& exn) {
    SrcKey sk{exn.vmFunc, exn.bcOff, exn.resumed};
    ProfSrcKey psk{exn.profTransId, sk};
    always_assert_log(
      !toInterp.count(psk),
      [&] {
        std::ostringstream oss;
        oss << folly::format("code generation failed with {}\n", exn.what());
        print(oss, m_irTrans->hhbcTrans().unit());
        return oss.str();
      });
    toInterp.insert(psk);
    return Retry;
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      assert(m_useAHot);
      m_useAHot = false;
      // We can't return Retry here because the code block selection
      // will still say hot.
      return Translator::Failure;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
  return Success;
}

uint64_t* Translator::getTransCounterAddr() {
  if (!isTransDBEnabled()) return nullptr;

  TransID id = m_translations.size();

  // allocate a new chunk of counters if necessary
  if (id >= m_transCounters.size() * transCountersPerChunk) {
    uint32_t   size = sizeof(uint64_t) * transCountersPerChunk;
    auto *chunk = (uint64_t*)malloc(size);
    bzero(chunk, size);
    m_transCounters.push_back(chunk);
  }
  assert(id / transCountersPerChunk < m_transCounters.size());
  return &(m_transCounters[id / transCountersPerChunk]
           [id % transCountersPerChunk]);
}

void Translator::addTranslation(const TransRec& transRec) {
  if (Trace::moduleEnabledRelease(Trace::trans, 1)) {
    // Log the translation's size, creation time, SrcKey, and size
    Trace::traceRelease("New translation: %" PRId64 " %s %u %u %d\n",
                        HPHP::Timer::GetCurrentTimeMicros() - m_createdTime,
                        folly::format("{}:{}:{}",
                          transRec.src.unit()->filepath()->data(),
                          transRec.src.getFuncId(),
                          transRec.src.offset()).str().c_str(),
                        transRec.aLen,
                        transRec.acoldLen,
                        transRec.kind);
  }

  if (!isTransDBEnabled()) return;
  uint32_t id = getCurrentTransID();
  m_translations.emplace_back(transRec);
  m_translations[id].id = id;

  if (transRec.aLen > 0) {
    m_transDB[transRec.aStart] = id;
  }
  if (transRec.acoldLen > 0) {
    m_transDB[transRec.acoldStart] = id;
  }
}

uint64_t Translator::getTransCounter(TransID transId) const {
  if (!isTransDBEnabled()) return -1ul;
  assert(transId < m_translations.size());

  uint64_t counter;

  if (transId / transCountersPerChunk >= m_transCounters.size()) {
    counter = 0;
  } else {
    counter =  m_transCounters[transId / transCountersPerChunk]
                              [transId % transCountersPerChunk];
  }
  return counter;
}

const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup,
                                  Class* ctx) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  if (cls->attrs() & AttrInterface) return nullptr;
  bool privateOnly = false;
  if (!RuntimeOption::RepoAuthoritative ||
      !(cls->preClass()->attrs() & AttrUnique)) {
    if (!ctx || !ctx->classof(cls)) {
      return nullptr;
    }
    if (!staticLookup) privateOnly = true;
  }

  const Func* func;
  LookupResult res = staticLookup ?
    g_context->lookupClsMethod(func, cls, name, nullptr, ctx, false) :
    g_context->lookupObjMethod(func, cls, name, ctx, false);

  if (res == LookupResult::MethodNotFound) return nullptr;

  assert(res == LookupResult::MethodFoundWithThis ||
         res == LookupResult::MethodFoundNoThis ||
         (staticLookup ?
          res == LookupResult::MagicCallStaticFound :
          res == LookupResult::MagicCallFound));

  magicCall =
    res == LookupResult::MagicCallStaticFound ||
    res == LookupResult::MagicCallFound;

  if ((privateOnly && (!(func->attrs() & AttrPrivate) || magicCall)) ||
      func->isAbstract() ||
      func->attrs() & AttrInterceptable) {
    return nullptr;
  }

  if (staticLookup) {
    if (magicCall) {
      /*
       *  i) We cant tell if a magic call would go to __call or __callStatic
       *       - Could deal with this by checking for the existence of __call
       *
       * ii) hphp semantics is that in the case of an object call, we look
       *     for __call in the scope of the object (this is incompatible
       *     with zend) which means we would have to know that there is no
       *     __call higher up in the tree
       *       - Could deal with this by checking for AttrNoOverride on the
       *         class
       */
      func = nullptr;
    }
  } else if (!(func->attrs() & AttrPrivate)) {
    if (magicCall || func->attrs() & AttrStatic) {
      if (!(cls->preClass()->attrs() & AttrNoOverride)) {
        func = nullptr;
      }
    } else if (!(func->attrs() & AttrNoOverride && !func->hasStaticLocals()) &&
               !(cls->preClass()->attrs() & AttrNoOverride)) {
      // Even if a func has AttrNoOverride, if it has static locals it
      // is cloned into subclasses (to give them different copies of
      // the static locals), so we need to skip this.
      func = nullptr;
    }
  }
  return func;
}

///////////////////////////////////////////////////////////////////////////////
}

void invalidatePath(const std::string& path) {
  TRACE(1, "invalidatePath: abspath %s\n", path.c_str());
  assert(path.size() >= 1 && path[0] == '/');
  Treadmill::enqueue([path] {
    /*
     * inotify saw this path change. Now poke the unit loader; it will
     * notice the underlying php file has changed.
     *
     * We don't actually need to *do* anything with the Unit* from
     * this lookup; since the path has changed, the file we'll get out is
     * going to be some new file, not the old file that needs invalidation.
     */
    String spath(path);
    lookupUnit(spath.get(), "", nullptr /* initial_opt */);
  });
}

///////////////////////////////////////////////////////////////////////////////
}
