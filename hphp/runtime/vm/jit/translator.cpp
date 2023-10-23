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

#include "hphp/runtime/vm/jit/translator.h"

#include <cinttypes>
#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/Conv.h>
#include <folly/MapUtil.h>

#include "hphp/util/arch.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bc-pattern.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-basic.h"
#include "hphp/runtime/vm/jit/irgen-bespoke.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"


TRACE_SET_MOD(trans);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * NB: this opcode structure is sparse; it cannot just be indexed by
 * opcode.
 */
using namespace InstrFlags;
static const struct {
  Op op;
  InstrInfo info;
} instrInfoSparse [] = {

  // Op             Inputs            Outputs       OutputTypes
  // --             ------            -------       -----------

  /*** 1. Basic instructions ***/

  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone         }},
  { OpPopU,        {Stack1|
                    DontGuardStack1,  None,         OutNone         }},
  { OpPopU2,       {StackTop2|
                    DontGuardAny,     Stack1,       OutSameAsInput1 }},
  { OpPopL,        {Stack1|Local,     Local,        OutNone         }},
  { OpDup,         {Stack1,           StackTop2,    OutSameAsInput1 }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull         }},
  { OpNullUninit,  {None,             Stack1,       OutNullUninit   }},
  { OpTrue,        {None,             Stack1,       OutBooleanImm   }},
  { OpFalse,       {None,             Stack1,       OutBooleanImm   }},
  { OpInt,         {None,             Stack1,       OutInt64        }},
  { OpDouble,      {None,             Stack1,       OutDouble       }},
  { OpString,      {None,             Stack1,       OutStringImm    }},
  { OpLazyClass,   {None,             Stack1,       OutLazyClass    }},
  { OpEnumClassLabel, {None,          Stack1,       OutEnumClassLabel }},
  { OpDict,        {None,             Stack1,       OutDictImm      }},
  { OpKeyset,      {None,             Stack1,       OutKeysetImm    }},
  { OpVec,         {None,             Stack1,       OutVecImm       }},
  { OpNewDictArray,   {None,          Stack1,       OutDict         }},
  { OpNewStructDict,  {StackN,        Stack1,       OutDict         }},
  { OpNewVec,         {StackN,        Stack1,       OutVec          }},
  { OpNewKeysetArray, {StackN,        Stack1,       OutKeyset       }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutModifiedInput3 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutModifiedInput2 }},
  { OpNewCol,      {None,             Stack1,       OutObject       }},
  { OpNewPair,     {StackTop2,        Stack1,       OutObject       }},
  { OpColFromArray,   {Stack1,        Stack1,       OutObject       }},
  { OpCnsE,        {None,             Stack1,       OutCns          }},
  { OpClsCns,      {Stack1|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpClsCnsD,     {None,             Stack1,       OutUnknown      }},
  { OpClsCnsL,     {Stack1|Local,     Stack1,       OutUnknown      }},
  { OpFile,        {None,             Stack1,       OutString       }},
  { OpDir,         {None,             Stack1,       OutString       }},
  { OpMethod,      {None,             Stack1,       OutString       }},
  { OpClassName,   {Stack1,           Stack1,       OutString       }},
  { OpLazyClassFromClass, {Stack1,    Stack1,       OutLazyClass    }},
  { OpEnumClassLabelName, {Stack1,    Stack1,       OutString       }},
  { OpFuncCred,    {None,             Stack1,       OutObject       }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString       }},
  { OpConcatN,     {StackN,           Stack1,       OutString       }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith        }},
  { OpSub,         {StackTop2,        Stack1,       OutArith        }},
  { OpMul,         {StackTop2,        Stack1,       OutArith        }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutUnknown      }},
  { OpMod,         {StackTop2,        Stack1,       OutUnknown      }},
  { OpPow,         {StackTop2,        Stack1,       OutUnknown      }},
  /* Logical ops */
  { OpNot,         {Stack1,           Stack1,       OutBoolean      }},
  { OpSame,        {StackTop2,        Stack1,       OutBoolean      }},
  { OpNSame,       {StackTop2,        Stack1,       OutBoolean      }},
  { OpEq,          {StackTop2,        Stack1,       OutBoolean      }},
  { OpNeq,         {StackTop2,        Stack1,       OutBoolean      }},
  { OpLt,          {StackTop2,        Stack1,       OutBoolean      }},
  { OpLte,         {StackTop2,        Stack1,       OutBoolean      }},
  { OpGt,          {StackTop2,        Stack1,       OutBoolean      }},
  { OpGte,         {StackTop2,        Stack1,       OutBoolean      }},
  { OpCmp,         {StackTop2,        Stack1,       OutInt64        }},
  /* Bitwise ops */
  { OpBitAnd,      {StackTop2,        Stack1,       OutBitOp        }},
  { OpBitOr,       {StackTop2,        Stack1,       OutBitOp        }},
  { OpBitXor,      {StackTop2,        Stack1,       OutBitOp        }},
  { OpBitNot,      {Stack1,           Stack1,       OutBitOp        }},
  { OpShl,         {StackTop2,        Stack1,       OutInt64        }},
  { OpShr,         {StackTop2,        Stack1,       OutInt64        }},
  /* Cast instructions */
  { OpCastBool,    {Stack1,           Stack1,       OutBoolean      }},
  { OpCastInt,     {Stack1,           Stack1,       OutInt64        }},
  { OpCastDouble,  {Stack1,           Stack1,       OutDouble       }},
  { OpCastString,  {Stack1,           Stack1,       OutString       }},
  { OpCastDict,    {Stack1,           Stack1,       OutDict         }},
  { OpCastKeyset,  {Stack1,           Stack1,       OutKeyset       }},
  { OpCastVec,     {Stack1,           Stack1,       OutVec          }},
  { OpDblAsBits,   {Stack1,           Stack1,       OutInt64        }},
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean      }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutPredBool     }},
  { OpIsLateBoundCls,{Stack1,         Stack1,       OutBoolean      }},
  { OpIsTypeStructC,{StackTop2,       Stack1,       OutBoolean      }},
  { OpThrowAsTypeStructException,
                   {StackTop2,        None,         OutNone         }},
  { OpCombineAndResolveTypeStruct,
                   {StackN,           Stack1,       OutDict         }},
  { OpSelect,      {StackTop3,        Stack1,       OutUnknown      }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64        }},
  { OpClone,       {Stack1,           Stack1,       OutObject       }},
  { OpExit,        {Stack1|DontGuardAny,
                                      Stack1,       OutNull         }},
  { OpFatal,       {Stack1|DontGuardAny,
                                      None,         OutNone         }},

  /*** 4. Control flow instructions ***/

  { OpEnter,       {None,             None,         OutNone         }},
  { OpJmp,         {None,             None,         OutNone         }},
  { OpJmpZ,        {Stack1,           None,         OutNone         }},
  { OpJmpNZ,       {Stack1,           None,         OutNone         }},
  { OpSwitch,      {Stack1,           None,         OutNone         }},
  { OpSSwitch,     {Stack1,           None,         OutNone         }},
  /*
   * RetC and RetM are special. Their manipulation of the runtime stack are
   * outside the boundaries of the tracelet abstraction; since they always end
   * a basic block, they behave more like "glue" between BBs than the
   * instructions in the body of a BB.
   *
   * RetC and RetM consume values from the stack, and these values' types needs
   * to be known at compile-time.
   */
  { OpRetC,        {None,             None,         OutNone         }},
  { OpRetM,        {None,             None,         OutNone         }},
  { OpRetCSuspended,
                   {None,             None,         OutNone         }},
  { OpThrow,       {Stack1,           None,         OutNone         }},

  /*** 5. Get instructions ***/

  { OpCGetL,       {Local,            Stack1,       OutCInputL      }},
  { OpCGetL2,      {Stack1|DontGuardStack1|
                    Local,            StackIns1,    OutCInputL      }},
  { OpCGetQuietL,  {Local,            Stack1,       OutCInputL      }},
  // In OpCUGetL we rely on OutCInputL returning TCell (which covers Uninit
  // values) instead of TInitCell.
  { OpCUGetL,      {Local,            Stack1,       OutCInputL      }},
  { OpPushL,       {Local,            Stack1|Local, OutCInputL      }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown      }},
  { OpCGetS,       {StackTop2,        Stack1,       OutUnknown      }},
  { OpClassGetC,   {Stack1,           Stack1,       OutClass        }},
  { OpClassGetTS,  {Stack1,           StackTop2,    OutUnknown      }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean      }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean      }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean      }},
  { OpIssetS,      {StackTop2,        Stack1,       OutBoolean      }},
  { OpIsUnsetL,    {Local,            Stack1,       OutBoolean      }},
  { OpIsTypeC,     {Stack1|
                    DontGuardStack1,  Stack1,       OutBoolean      }},
  { OpIsTypeL,     {Local,            Stack1,       OutIsTypeL      }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput1  }},
  { OpSetG,        {StackTop2,        Stack1,       OutSameAsInput1  }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput1  }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp        }},
  { OpSetOpG,      {StackTop2|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpSetOpS,      {StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec       }},
  { OpIncDecG,     {Stack1|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpIncDecS,     {StackTop2|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpUnsetL,      {Local,            Local,        OutNone         }},
  { OpUnsetG,      {Stack1|DontGuardAny,
                                      None,         OutNone         }},

  /*** 8. Call instructions ***/

  { OpNewObj,      {Stack1,           Stack1,       OutObject       }},
  { OpNewObjD,     {None,             Stack1,       OutObject       }},
  { OpNewObjS,     {None,             Stack1,       OutObject       }},
  { OpLockObj,     {Stack1,           Stack1,       OutSameAsInput1 }},

  /*
   * FCall* are special. Like the Ret* instructions, their manipulation of
   * the runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCallClsMethod,
                   {StackTop2,        StackN,       OutUnknown      }},
  { OpFCallClsMethodM,
                   {Stack1,           StackN,       OutUnknown      }},
  { OpFCallClsMethodD,
                   {None,             StackN,       OutUnknown      }},
  { OpFCallClsMethodS,
                   {Stack1,           StackN,       OutUnknown      }},
  { OpFCallClsMethodSD,
                   {None,             StackN,       OutUnknown      }},
  { OpFCallCtor,   {None,             StackN,       OutUnknown      }},
  { OpFCallFunc,   {Stack1,           StackN,       OutUnknown      }},
  { OpFCallFuncD,  {None,             StackN,       OutUnknown      }},
  { OpFCallObjMethod,
                   {Stack1|DontGuardAny,
                                      StackN,       OutUnknown      }},
  { OpFCallObjMethodD,
                   {None,             StackN,       OutUnknown      }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown      }},
  { OpLIterInit,   {Local,            Local,        OutUnknown      }},
  { OpIterNext,    {None,             Local,        OutUnknown      }},
  { OpLIterNext,   {Local,            Local,        OutUnknown      }},
  { OpIterFree,    {None,             None,         OutNone         }},
  { OpLIterFree,   {Local,            None,         OutNone         }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},
  { OpInclOnce,    {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},
  { OpReq,         {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},
  { OpReqOnce,     {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},
  { OpReqDoc,      {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},
  { OpEval,        {Stack1|DontGuardAny,
                                     Stack1,       OutUnknown      }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject   }},
  { OpBareThis,    {None,             Stack1,       OutUnknown      }},
  { OpCheckThis,   {This,             None,         OutNone         }},
  { OpChainFaults, {StackTop2|DontGuardAny,
                                      Stack1,       OutObject       }},
  { OpVerifyParamType,
                   {Stack1,           Stack1,       OutUnknown      }},
  { OpVerifyParamTypeTS,
                   {Local|Stack1,     None,         OutNone         }},
  { OpVerifyRetTypeC,
                   {Stack1,           Stack1,       OutUnknown      }},
  { OpVerifyRetTypeTS,
                   {StackTop2,        Stack1,       OutUnknown      }},
  { OpVerifyRetNonNullC,
                   {Stack1,           Stack1,       OutSameAsInput1  }},
  { OpVerifyOutType,
                   {Stack1,           Stack1,       OutUnknown      }},
  { OpOODeclExists,
                   {StackTop2,        Stack1,       OutBoolean      }},
  { OpSelfCls,     {None,             Stack1,       OutClass        }},
  { OpParentCls,   {None,             Stack1,       OutClass        }},
  { OpLateBoundCls,{None,             Stack1,       OutClass        }},
  { OpRecordReifiedGeneric,
                   {Stack1,           Stack1,       OutVec          }},
  { OpCheckClsReifiedGenericMismatch,
                   {Stack1,           None,         OutNone         }},
  { OpClassHasReifiedGenerics,
                   {Stack1,           Stack1,       OutBoolean      }},
  { OpGetClsRGProp,
                   {Stack1,           Stack1,       OutVec          }},
  { OpHasReifiedParent,
                   {Stack1,           Stack1,       OutBoolean      }},
  { OpCheckClsRGSoft,
                   {Stack1,           None,         OutNone         }},
  { OpNativeImpl,  {None,             None,         OutNone         }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject       }},
  { OpIdx,         {StackTop3,        Stack1,       OutUnknown      }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown      }},
  { OpArrayMarkLegacy,   {StackTop2,  Stack1,       OutUnknown      }},
  { OpArrayUnmarkLegacy, {StackTop2,  Stack1,       OutUnknown      }},
  { OpCheckProp,   {None,             Stack1,       OutBoolean      }},
  { OpInitProp,    {Stack1,           None,         OutNone         }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone         }},
  { OpThrowNonExhaustiveSwitch,
                   {None,             None,         OutNone         }},
  { OpRaiseClassStringConversionNotice,
                   {None,             None,         OutNone         }},
  { OpAssertRATL,  {None,             None,         OutNone         }},
  { OpAssertRATStk,{None,             None,         OutNone         }},
  { OpBreakTraceHint,{None,           None,         OutNone         }},
  { OpGetMemoKeyL, {Local,            Stack1,       OutUnknown      }},
  { OpResolveRFunc,{Stack1,           Stack1,       OutFuncLike     }},
  { OpResolveFunc, {None,             Stack1,       OutFunc         }},
  { OpResolveMethCaller,
                   {None,             Stack1,       OutFunc         }},
  { OpResolveClsMethod,
                   {Stack1,           Stack1,       OutClsMeth      }},
  { OpResolveClsMethodD,
                   {None,             Stack1,       OutClsMeth      }},
  { OpResolveClsMethodS,
                   {None,             Stack1,       OutClsMeth      }},
  { OpResolveRClsMethod,
                   {StackTop2,        Stack1,       OutClsMethLike  }},
  { OpResolveRClsMethodD,
                   {Stack1,           Stack1,       OutClsMethLike  }},
  { OpResolveRClsMethodS,
                   {Stack1,           Stack1,       OutClsMethLike  }},
  { OpResolveClass,{None,             Stack1,       OutClass      }},
  { OpSetImplicitContextByValue,
                   {Stack1,           Stack1,       OutUnknown      }},
  { OpVerifyImplicitContextState,
                   {None,             None,         OutNone         }},
  { OpCreateSpecialImplicitContext,
                   {StackTop2,        Stack1,       OutUnknown      }},

  /*** 14. Generator instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutNull         }},
  { OpContEnter,   {Stack1,           Stack1,       OutUnknown      }},
  { OpContRaise,   {Stack1,           Stack1,       OutUnknown      }},
  { OpYield,       {Stack1,           Stack1,       OutUnknown      }},
  { OpYieldK,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpContCheck,   {None,             None,         OutNone         }},
  { OpContValid,   {None,             Stack1,       OutBoolean      }},
  { OpContKey,     {None,             Stack1,       OutUnknown      }},
  { OpContCurrent, {None,             Stack1,       OutUnknown      }},
  { OpContGetReturn,
                   {None,             Stack1,       OutUnknown      }},

  /*** 15. Async functions instructions ***/

  { OpWHResult,    {Stack1,           Stack1,       OutUnknown      }},
  { OpAwait,       {Stack1,           Stack1,       OutUnknown      }},
  { OpAwaitAll,    {LocalRange,       Stack1,       OutNull         }},

  /*** 16. Member instructions ***/

  { OpBaseGC,      {StackI,           MBase,        OutNone         }},
  { OpBaseGL,      {Local,            MBase,        OutNone         }},
  { OpBaseSC,      {StackI|StackI2,   MBase,        OutNone         }},
  { OpBaseL,       {Local,            MBase,        OutNone         }},
  { OpBaseC,       {StackI,           MBase,        OutNone         }},
  { OpBaseH,       {None,             MBase,        OutNone         }},
  { OpDim,         {MBase|MKey,       MBase,        OutNone         }},
  { OpQueryM,      {BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpSetM,        {Stack1|DontGuardStack1|BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpSetRangeM,   {StackTop3|BStackN|MBase,
                                      None,         OutNone         }},
  { OpIncDecM,     {BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpSetOpM,      {Stack1|BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpUnsetM,      {BStackN|MBase|MKey,
                                      None,         OutNone         }},
  { OpMemoGet,     {LocalRange,       None,         OutUnknown      }},
  { OpMemoGetEager,{LocalRange,       None,         OutUnknown      }},
  { OpMemoSet,     {Stack1|LocalRange,
                                      Stack1,       OutSameAsInput1 }},
  { OpMemoSetEager, {Stack1|LocalRange,
                                      Stack1,       OutSameAsInput1 }},
  { OpNop,         {None|DontGuardAny,
                                      None,         OutNone         }},
  { OpCGetCUNop,   {Stack1|DontGuardAny,
                                      Stack1,       OutCInput       }},
  { OpUGetCUNop,   {Stack1|DontGuardAny,
                                      Stack1,       OutNullUninit   }},
};

namespace {
hphp_hash_map<Op, InstrInfo> instrInfo;
bool instrInfoInited;
}

void initInstrInfo() {
  if (!instrInfoInited) {
    instrInfo.reserve(sizeof(instrInfoSparse)/sizeof(*instrInfoSparse));
    for (auto& info : instrInfoSparse) {
      instrInfo[info.op] = info.info;
    }
    instrInfoInited = true;
  }
}

const InstrInfo& getInstrInfo(Op op) {
  assertx(instrInfoInited);
  return instrInfo[op];
}

namespace {
int64_t countOperands(uint64_t mask) {
  const uint64_t ignore = Local | Iter | DontGuardStack1 |
    DontGuardAny | This | MBase | StackI | StackI2 | MKey | LocalRange;
  mask &= ~ignore;

  static const uint64_t counts[][2] = {
    {Stack3,       1},
    {Stack2,       1},
    {Stack1,       1},
    {StackIns1,    2},
  };

  int64_t count = 0;
  for (auto const& pair : counts) {
    if (mask & pair[0]) {
      count += pair[1];
      mask &= ~pair[0];
    }
  }
  assertx(mask == 0);
  return count;
}
}

int64_t getStackPopped(PC pc) {
  auto const op = peek_op(pc);
  switch (op) {
    case Op::FCallClsMethod:
    case Op::FCallClsMethodM:
    case Op::FCallClsMethodD:
    case Op::FCallClsMethodS:
    case Op::FCallClsMethodSD:
    case Op::FCallCtor:
    case Op::FCallFunc:
    case Op::FCallFuncD:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD: {
      auto const fca = getImm(pc, 0).u_FCA;
      auto const nin = countOperands(getInstrInfo(op).in);
      return nin + fca.numInputs() + (kNumActRecCells - 1) + fca.numRets;
    }

    case Op::QueryM:
    case Op::IncDecM:
    case Op::UnsetM:
    case Op::NewVec:
    case Op::NewKeysetArray:
    case Op::ConcatN:
    case Op::CombineAndResolveTypeStruct:
    case Op::CreateCl:
      return getImm(pc, 0).u_IVA;

    case Op::SetM:
    case Op::SetOpM:
      return getImm(pc, 0).u_IVA + 1;

    case Op::SetRangeM:
      return getImm(pc, 0).u_IVA + 3;

    case Op::NewStructDict:
      return getImmVector(pc).size();

    default:
      break;
  }

  uint64_t mask = getInstrInfo(op).in;

  // All instructions with these properties are handled above
  assertx((mask & (StackN | BStackN)) == 0);

  return countOperands(mask);
}

int64_t getStackPushed(PC pc) {
  auto const op = peek_op(pc);
  switch (op) {
    case Op::FCallClsMethod:
    case Op::FCallClsMethodM:
    case Op::FCallClsMethodD:
    case Op::FCallClsMethodS:
    case Op::FCallClsMethodSD:
    case Op::FCallCtor:
    case Op::FCallFunc:
    case Op::FCallFuncD:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD:
      return getImm(pc, 0).u_FCA.numRets;
    default:
      break;
  }

  uint64_t mask = getInstrInfo(op).out;

  // All instructions with these properties are handled above
  assertx((mask & (StackN | BStackN)) == 0);

  return countOperands(mask);
}

bool isAlwaysNop(const NormalizedInstruction& ni) {
  switch (ni.op()) {
  case Op::Nop:
  case Op::CGetCUNop:
  case Op::UGetCUNop:
    return true;
  default:
    return false;
  }
}

#define NA
#define ONE(a) a(0)
#define TWO(a, b) a(0) b(1)
#define THREE(a, b, c) a(0) b(1) c(2)
#define FOUR(a, b, c, d) a(0) b(1) c(2) d(3)
#define FIVE(a, b, c, d, e) a(0) b(1) c(2) d(3) e(4)
#define SIX(a, b, c, d, e, f) a(0) b(1) c(2) d(3) e(4) f(5)
// Iterator bytecodes are handled specially here
#define LA(n) assertx(idx == 0xff); idx = n;
#define NLA(n) assertx(idx == 0xff); idx = n;
#define ILA(n) assertx(idx == 0xff); idx = n;
#define MA(n)
#define BLA(n)
#define SLA(n)
#define IVA(n)
#define I64A(n)
#define IA(n)
#define DA(n)
#define SA(n)
#define AA(n)
#define RATA(n)
#define BA(n)
#define OA(op) BA
#define VSA(n)
#define KA(n)
#define LAR(n)
#define ITA(n)
#define FCA(n)
#define O(name, imm, ...) case Op::name: imm break;

size_t localImmIdx(Op op) {
  switch (op) {
    case Op::LIterInit:
    case Op::LIterNext:
    case Op::LIterFree:
      return 1;
    default:
      break;
  }

  size_t idx = 0xff;
  switch (op) {
    OPCODES
  }
  assertx(idx != 0xff);
  return idx;
}

size_t memberKeyImmIdx(Op op) {
  size_t idx = 0xff;
  switch (op) {
#undef LA
#undef NLA
#undef ILA
#undef KA
#define LA(n)
#define NLA(n)
#define ILA(n)
#define KA(n) assertx(idx == 0xff); idx = n;
    OPCODES
  }
  assertx(idx != 0xff);
  return idx;
}

#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX
#undef LA
#undef NLA
#undef ILA
#undef MA
#undef BLA
#undef SLA
#undef IVA
#undef I64A
#undef IA
#undef DA
#undef SA
#undef AA
#undef RATA
#undef BA
#undef OA
#undef VSA
#undef KA
#undef LAR
#undef ITA
#undef FCA
#undef O

unsigned localRangeImmIdx(Op op) {
  switch (op) {
    case Op::AwaitAll:
    case Op::MemoSet:
    case Op::MemoSetEager:
      return 0;
    case Op::MemoGet:
      return 1;
    case Op::MemoGetEager:
      return 2;
    default:
      always_assert_flog("op {} doesn't have LocalRange!\n", opcodeToName(op));
      return -1;
  }
}

uint32_t getLocalOperand(const NormalizedInstruction& ni) {
  auto const idx = localImmIdx(ni.op());
  auto const argu = ni.imm[idx];
  switch (immType(ni.op(), idx)) {
    case ArgType::LA:
      return argu.u_LA;
    case ArgType::NLA:
      return argu.u_NLA.id;
    case ArgType::ILA:
      return argu.u_ILA;
    default:
      always_assert(false);
  }
  not_reached();
}

/*
 * Get location metadata for the inputs of `ni'.
 */
InputInfoVec getInputs(const NormalizedInstruction& ni, SBInvOffset bcSPOff) {
  InputInfoVec inputs;
  if (isAlwaysNop(ni)) return inputs;

  always_assert_flog(
    instrInfo.count(ni.op()),
    "Invalid opcode in getInputsImpl: {}\n",
    opcodeToName(ni.op())
  );
  UNUSED auto const sk = ni.source;

  auto const& info = instrInfo[ni.op()];
  auto const flags = info.in;
  auto stackOff = bcSPOff;

  if (flags & Stack1) {
    SKTRACE(1, sk, "getInputs: Stack1 %d\n", stackOff.offset);
    inputs.emplace_back(Location::Stack { stackOff-- });

    if (flags & DontGuardStack1) inputs.back().dontGuard = true;

    if (flags & Stack2) {
      SKTRACE(1, sk, "getInputs: Stack2 %d\n", stackOff.offset);
      inputs.emplace_back(Location::Stack { stackOff-- });

      if (flags & Stack3) {
        SKTRACE(1, sk, "getInputs: Stack3 %d\n", stackOff.offset);
        inputs.emplace_back(Location::Stack { stackOff-- });
      }
    }
  }
  if (flags & StackI) {
    inputs.emplace_back(Location::Stack {
      BCSPRelOffset{safe_cast<int32_t>(ni.imm[0].u_IVA)}.
        to<SBInvOffset>(bcSPOff)
    });
  }
  if (flags & StackI2) {
    inputs.emplace_back(Location::Stack {
      BCSPRelOffset{safe_cast<int32_t>(ni.imm[1].u_IVA)}.
        to<SBInvOffset>(bcSPOff)
    });
  }

  if (flags & StackN) {
    int numArgs = [&] () -> int {
      switch (ni.op()) {
      case Op::NewVec:
      case Op::NewKeysetArray:
      case Op::CombineAndResolveTypeStruct:
      case Op::ConcatN:
        return ni.imm[0].u_IVA;
      default:
        return ni.immVec.numStackValues();
      }
      not_reached();
    }();

    SKTRACE(1, sk, "getInputs: StackN %d %d\n", stackOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location::Stack { stackOff-- });
      inputs.back().dontGuard = true;
    }
  }
  if (flags & BStackN) {
    int numArgs = ni.imm[0].u_IVA;

    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", stackOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location::Stack { stackOff-- });
      inputs.back().dontGuard = true;
    }
  }
  if (isFCall(ni.op())) {
    auto const& fca = ni.imm[0].u_FCA;
    SKTRACE(1, sk, "getInputs: %s %d %d\n",
            opcodeToName(ni.op()), stackOff.offset, fca.numInputs());

    if (fca.hasGenerics()) inputs.emplace_back(Location::Stack { stackOff-- });
    if (fca.hasUnpack()) inputs.emplace_back(Location::Stack { stackOff-- });
    stackOff -= fca.numArgs + (kNumActRecCells - 1);

    switch (ni.op()) {
      case Op::FCallCtor:
      case Op::FCallObjMethod:
      case Op::FCallObjMethodD:
        inputs.emplace_back(Location::Stack { stackOff-- });
        break;
      default:
        stackOff--;
        break;
    }
  }

  if (flags & Local) {
    auto const loc = getLocalOperand(ni);
    SKTRACE(1, sk, "getInputs: local %d\n", loc);
    inputs.emplace_back(Location::Local{loc});
  }

  if (flags & LocalRange) {
    auto const& range = ni.imm[localRangeImmIdx(ni.op())].u_LAR;
    SKTRACE(1, sk, "getInputs: localRange %d+%d\n",
            range.first, range.count);
    for (int i = 0; i < range.count; ++i) {
      inputs.emplace_back(Location::Local { uint32_t(range.first + i) });
    }
  }

  if (flags & MKey) {
    auto mk = ni.imm[memberKeyImmIdx(ni.op())].u_KA;
    switch (mk.mcode) {
      case MEL:
      case MPL:
        inputs.emplace_back(Location::Local { uint32_t(mk.local.id) });
        break;
      case MEC:
      case MPC:
        inputs.emplace_back(Location::Stack {
          BCSPRelOffset{int32_t(mk.iva)}.to<SBInvOffset>(bcSPOff)
        });
        break;
      case MW:
      case MEI:
      case MET:
      case MPT:
      case MQT:
        // The inputs vector is only used for deciding when to break the
        // tracelet, which can never happen for these cases.
        break;
    }
  }

  if (flags & MBase) inputs.emplace_back(Location::MBase{});

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackOff.offset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if ((flags & DontGuardAny)) {
    for (auto& info : inputs) info.dontGuard = true;
  }
  return inputs;
}

bool dontGuardAnyInputs(const NormalizedInstruction& ni) {
  return getInstrInfo(ni.op()).in & DontGuardAny;
}

/*
 * Get the list of output locals written by the `ni' instruction.
 */
jit::fast_set<uint32_t> getLocalOutputs(const NormalizedInstruction& ni) {
  fast_set<uint32_t> locals;
  auto const op = ni.op();

  if (isIteratorOp(op)) {
    auto const ita = ni.imm[0].u_ITA;
    locals.insert(ita.valId);
    if (ita.hasKey()) locals.insert(ita.keyId);
  } else {
    auto const info = getInstrInfo(op);
    if (info.out & Local) {
      auto const id = getLocalOperand(ni);
      locals.insert(id);
    }
    if (info.out & LocalRange) {
      auto const& range = ni.imm[localRangeImmIdx(op)].u_LAR;
      for (unsigned i = 0; i < range.count; ++i) {
        locals.insert(range.first + i);
      }
    }
    if (info.out & MKey) {
      auto const mk = ni.imm[memberKeyImmIdx(op)].u_KA;
      if (mk.mcode == MEL || mk.mcode == MPL) {
        locals.insert(mk.local.id);
      }
    }
  }

  return locals;
}

bool instrBreaksProfileBB(const NormalizedInstruction& inst) {
  auto const op = inst.op();
  if (isFCall(op)) return true;

  if (instrIsNonCallControlFlow(op) ||
      op == OpAwait || // may branch to scheduler and suspend execution
      op == OpAwaitAll || // similar to Await
      op == OpClsCnsD || // side exits if misses in the RDS
      op == OpThrowNonExhaustiveSwitch || // control flow breaks bb
      op == OpVerifyParamTypeTS) { // avoids combinatorial explosion
                                   // with nullable types
    return true;
  }
  // In profiling mode, don't trace through a control flow merge point,
  // however, allow inlining of default parameter funclets
  assertx(profData());
  if (profData()->anyBlockEndsAt(inst.func(), inst.offset()) &&
      !inst.func()->isEntry(inst.nextSk().offset())) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

namespace {

#define IMM_BLA(n)     ni.immVec
#define IMM_SLA(n)     ni.immVec
#define IMM_VSA(n)     ni.immVec
#define IMM_IVA(n)     ni.imm[n].u_IVA
#define IMM_I64A(n)    ni.imm[n].u_I64A
#define IMM_LA(n)      ni.imm[n].u_LA
#define IMM_NLA(n)     ni.imm[n].u_NLA
#define IMM_ILA(n)     ni.imm[n].u_ILA
#define IMM_IA(n)      ni.imm[n].u_IA
#define IMM_DA(n)      ni.imm[n].u_DA
#define IMM_SA(n)      ni.unit()->lookupLitstrId(ni.imm[n].u_SA)
#define IMM_RATA(n)    ni.imm[n].u_RATA
#define IMM_AA(n)      ni.unit()->lookupArrayId(ni.imm[n].u_AA)
#define IMM_BA(n)      ni.imm[n].u_BA
#define IMM_OA_IMPL(n) ni.imm[n].u_OA
#define IMM_OA(subop)  (subop)IMM_OA_IMPL
#define IMM_KA(n)      ni.imm[n].u_KA
#define IMM_LAR(n)     ni.imm[n].u_LAR
#define IMM_ITA(n)     ni.imm[n].u_ITA
#define IMM_FCA(n)     ni.imm[n].u_FCA

#define ONE(x0)                , IMM_##x0(0)
#define TWO(x0,x1)             , IMM_##x0(0), IMM_##x1(1)
#define THREE(x0,x1,x2)        , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2)
#define FOUR(x0,x1,x2,x3)      , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3)
#define FIVE(x0,x1,x2,x3,x4)   , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3), IMM_##x4(4)
#define SIX(x0,x1,x2,x3,x4,x5) , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3), IMM_##x4(4), IMM_##x5(5)
#define NA                   /*  */

void translateDispatch(irgen::IRGS& irgs,
                       const NormalizedInstruction& ni) {
#define O(nm, imms, ...) case Op::nm: irgen::emit##nm(irgs imms); return;
  switch (ni.op()) { OPCODES }
#undef O
}

#undef SIX
#undef FIVE
#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA

//////////////////////////////////////////////////////////////////////

Type flavorToType(FlavorDesc f) {
  switch (f) {
    case CV:  return TInitCell;
    case CUV: return TCell;
    case UV:  return TUninit;
    case NOV: break;
  }
  always_assert(false);
}

}

void translateInstr(irgen::IRGS& irgs, const NormalizedInstruction& ni) {
  assertx(curSrcKey(irgs) == ni.source);

  if (ni.source.funcEntry()) {
    emitFuncEntry(irgs);
    return;
  }

  auto pc = ni.pc();
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    if (isFCall(ni.op()) && instrInputFlavor(pc, i) == UV) {
      // This is a hack to deal with the fact that this instruction is
      // actually popping an ActRec in the middle of its "pops." We could
      // assert on the Uninit values on the stack, but the call is going to
      // override them anyway so it's not worth guarding on them.
      break;
    }
    auto const type = flavorToType(instrInputFlavor(pc, i));
    irgen::assertTypeStack(irgs, BCSPRelOffset{i}, type);
  }

  FTRACE(1, "\nTranslating {}: {} with state:\n{}\n",
         ni.offset(), ni, show(irgs));

  irgen::ringbufferEntry(irgs, Trace::RBTypeBytecodeStart, ni.source, 2);
  irgen::implIncStat(irgs, Stats::Instr_TC);
  if (Stats::enableInstrCount()) {
    irgen::implIncStat(irgs, Stats::opToTranslStat(ni.op()));
  }

  if (isAlwaysNop(ni)) return;

  if (ni.interp || RuntimeOption::EvalJitAlwaysInterpOne) {
    irgen::interpOne(irgs);
    return;
  }

  if (ni.forceSurpriseCheck) surpriseCheck(irgs);

  translateDispatchBespoke(irgs, ni, [&](irgen::IRGS& env) {
    translateDispatch(env, ni);
  });

  FTRACE(3, "\nTranslated {}: {} with state:\n{}\n",
         ni.offset(), ni, show(irgs));
}

//////////////////////////////////////////////////////////////////////

}

void invalidatePath(const std::string& path) {
  TRACE(1, "invalidatePath: abspath %s\n", path.c_str());
  assertx(path.size() >= 1 && path[0] == '/');
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
    lookupUnit(spath.get(), "", nullptr /* initial_opt */, nullptr, false);
  });
}

///////////////////////////////////////////////////////////////////////////////
}
