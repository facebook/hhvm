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
#include <stdarg.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/Conv.h>
#include <folly/MapUtil.h>

#include "hphp/util/arch.h"
#include "hphp/util/map-walker.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/repo-auth-type-codec.h"
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

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-basic.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
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

  { OpDiscardClsRef, {None,           None,         OutNone         }},
  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone         }},
  { OpPopV,        {Stack1|
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
  { OpArray,       {None,             Stack1,       OutArrayImm     }},
  { OpDict,        {None,             Stack1,       OutDictImm      }},
  { OpKeyset,      {None,             Stack1,       OutKeysetImm    }},
  { OpVec,         {None,             Stack1,       OutVecImm       }},
  { OpNewArray,    {None,             Stack1,       OutArray        }},
  { OpNewMixedArray,  {None,          Stack1,       OutArray        }},
  { OpNewDictArray,   {None,          Stack1,       OutDict         }},
  { OpNewLikeArrayL,  {Local,         Stack1,       OutArray        }},
  { OpNewPackedArray, {StackN,        Stack1,       OutArray        }},
  { OpNewStructArray, {StackN,        Stack1,       OutArray        }},
  { OpNewStructDArray,{StackN,        Stack1,       OutArray        }},
  { OpNewStructDict,  {StackN,        Stack1,       OutDict         }},
  { OpNewVecArray,    {StackN,        Stack1,       OutVec          }},
  { OpNewKeysetArray, {StackN,        Stack1,       OutKeyset       }},
  { OpNewVArray,   {StackN,           Stack1,       OutArray        }},
  { OpNewDArray,   {None,             Stack1,       OutArray        }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutModifiedInput3 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutModifiedInput2 }},
  { OpNewCol,      {None,             Stack1,       OutObject       }},
  { OpNewPair,     {StackTop2,        Stack1,       OutObject       }},
  { OpNewRecord,   {StackN,           Stack1,       OutRecord       }},
  { OpColFromArray,   {Stack1,        Stack1,       OutObject       }},
  { OpCnsE,        {None,             Stack1,       OutCns          }},
  { OpClsCns,      {None,             Stack1,       OutUnknown      }},
  { OpClsCnsD,     {None,             Stack1,       OutUnknown      }},
  { OpFile,        {None,             Stack1,       OutString       }},
  { OpDir,         {None,             Stack1,       OutString       }},
  { OpMethod,      {None,             Stack1,       OutString       }},
  { OpClsRefName,  {None,             Stack1,       OutString       }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString       }},
  { OpConcatN,     {StackN,           Stack1,       OutString       }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith        }},
  { OpSub,         {StackTop2,        Stack1,       OutArith        }},
  { OpMul,         {StackTop2,        Stack1,       OutArith        }},
  /* Arithmetic ops that overflow ints to floats */
  { OpAddO,        {StackTop2,        Stack1,       OutArithO       }},
  { OpSubO,        {StackTop2,        Stack1,       OutArithO       }},
  { OpMulO,        {StackTop2,        Stack1,       OutArithO       }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutUnknown      }},
  { OpMod,         {StackTop2,        Stack1,       OutUnknown      }},
  { OpPow,         {StackTop2,        Stack1,       OutUnknown      }},
  /* Logical ops */
  { OpXor,         {StackTop2,        Stack1,       OutBoolean      }},
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
  { OpCastArray,   {Stack1,           Stack1,       OutArray        }},
  { OpCastObject,  {Stack1,           Stack1,       OutObject       }},
  { OpCastDict,    {Stack1,           Stack1,       OutDict         }},
  { OpCastKeyset,  {Stack1,           Stack1,       OutKeyset       }},
  { OpCastVec,     {Stack1,           Stack1,       OutVec          }},
  { OpCastVArray,  {Stack1,           Stack1,       OutArray        }},
  { OpCastDArray,  {Stack1,           Stack1,       OutArray        }},
  { OpDblAsBits,   {Stack1,           Stack1,       OutInt64        }},
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean      }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutPredBool     }},
  { OpIsLateBoundCls,{Stack1,         Stack1,       OutBoolean      }},
  { OpIsTypeStructC,{StackTop2,       Stack1,       OutBoolean      }},
  { OpThrowAsTypeStructException,
                   {StackTop2,        None,         OutNone         }},
  { OpCombineAndResolveTypeStruct,
                   {StackN,           Stack1,       OutDArray       }},
  { OpSelect,      {StackTop3,        Stack1,       OutUnknown      }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64        }},
  { OpClone,       {Stack1,           Stack1,       OutObject       }},
  { OpExit,        {Stack1,           Stack1,       OutNull         }},
  { OpFatal,       {Stack1,           None,         OutNone         }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone         }},
  { OpJmpNS,       {None,             None,         OutNone         }},
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
  { OpCGetQuietG,  {Stack1,           Stack1,       OutUnknown      }},
  { OpCGetS,       {Stack1,           Stack1,       OutUnknown      }},
  { OpVGetL,       {Local,            Stack1|Local, OutVInputL      }},
  { OpClsRefGetC,  {Stack1,           None,         OutNone         }},
  { OpClsRefGetTS, {Stack1,           None,         OutNone         }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean      }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean      }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean      }},
  { OpIssetS,      {Stack1,           Stack1,       OutBoolean      }},
  { OpEmptyL,      {Local,            Stack1,       OutBoolean      }},
  { OpEmptyG,      {Stack1,           Stack1,       OutBoolean      }},
  { OpEmptyS,      {Stack1,           Stack1,       OutBoolean      }},
  { OpIsTypeC,     {Stack1|
                    DontGuardStack1,  Stack1,       OutBoolean      }},
  { OpIsTypeL,     {Local,            Stack1,       OutIsTypeL      }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput1  }},
  { OpSetG,        {StackTop2,        Stack1,       OutSameAsInput1  }},
  { OpSetS,        {StackTop2,        Stack1,       OutSameAsInput1  }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp        }},
  { OpSetOpG,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpSetOpS,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec       }},
  { OpIncDecG,     {Stack1,           Stack1,       OutUnknown      }},
  { OpIncDecS,     {Stack1,           Stack1,       OutUnknown      }},
  { OpUnsetL,      {Local,            Local,        OutNone         }},
  { OpUnsetG,      {Stack1,           None,         OutNone         }},

  /*** 8. Call instructions ***/

  { OpNewObj,      {None,             Stack1,       OutObject       }},
  { OpNewObjD,     {None,             Stack1,       OutObject       }},
  { OpNewObjRD,    {Stack1,           Stack1,       OutObject       }},
  { OpNewObjS,     {None,             Stack1,       OutObject       }},
  { OpLockObj,     {Stack1,           Stack1,       OutSameAsInput1 }},

  /*
   * FPush* and FCall are special. Like the Ret* instructions, their
   * manipulation of the runtime stack are outside the boundaries of
   * the tracelet abstraction.
   */
  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushFuncD,  {None,             FStack,       OutFDesc        }},
  { OpFPushFuncRD, {Stack1,           FStack,       OutFDesc        }},
  { OpFCallCtor,   {None,             StackN,       OutUnknown      }},
  { OpFCallObjMethod,
                   {Stack1,           StackN,       OutUnknown      }},
  { OpFCallObjMethodD,
                   {None,             StackN,       OutUnknown      }},
  { OpFCallObjMethodRD,
                   {Stack1,           StackN,       OutUnknown      }},
  { OpFPushClsMethod,
                   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushClsMethodS,
                   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushClsMethodSD,
                   {None,             FStack,       OutFDesc        }},
  { OpFPushClsMethodSRD,
                   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc        }},
  { OpFPushClsMethodRD,
                   {Stack1,           FStack,       OutFDesc        }},
  { OpFCall,       {FStack,           StackN,       OutUnknown      }},
  { OpFCallBuiltin,{BStackN|DontGuardAny,
                                      Stack1,       OutUnknown      }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown      }},
  { OpLIterInit,   {Local,            Local,        OutUnknown      }},
  { OpIterInitK,   {Stack1,           Local,        OutUnknown      }},
  { OpLIterInitK,  {Local,            Local,        OutUnknown      }},
  { OpIterNext,    {None,             Local,        OutUnknown      }},
  { OpLIterNext,   {Local,            Local,        OutUnknown      }},
  { OpIterNextK,   {None,             Local,        OutUnknown      }},
  { OpLIterNextK,  {Local,            Local,        OutUnknown      }},
  { OpIterFree,    {None,             None,         OutNone         }},
  { OpLIterFree,   {Local,            None,         OutNone         }},
  { OpIterBreak,   {Local,            None,         OutNone         }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown      }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown      }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown      }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown      }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown      }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown      }},
  { OpDefTypeAlias,{None,             None,         OutNone         }},
  { OpDefCls,      {None,             None,         OutNone         }},
  { OpDefRecord,   {None,             None,         OutNone         }},
  { OpDefCns,      {Stack1,           Stack1,       OutBoolean      }},
  { OpAliasCls,    {Stack1,           Stack1,       OutBoolean      }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject   }},
  { OpBareThis,    {None,             Stack1,       OutUnknown      }},
  { OpCheckThis,   {This,             None,         OutNone         }},
  { OpInitThisLoc,
                   {None,             Local,        OutUnknown      }},
  { OpChainFaults, {StackTop2,        Stack1,       OutObject       }},
  { OpVerifyParamType,
                   {Local,            Local,        OutUnknown      }},
  { OpVerifyParamTypeTS,
                   {Local|Stack1,     Local,        OutUnknown      }},
  { OpVerifyRetTypeC,
                   {Stack1,           Stack1,       OutSameAsInput1  }},
  { OpVerifyRetTypeTS,
                   {StackTop2,        Stack1,       OutSameAsInput2  }},
  { OpVerifyRetNonNullC,
                   {Stack1,           Stack1,       OutSameAsInput1  }},
  { OpVerifyOutType,
                   {Stack1,           Stack1,       OutSameAsInput1  }},
  { OpOODeclExists,
                   {StackTop2,        Stack1,       OutBoolean      }},
  { OpSelf,        {None,             None,         OutNone         }},
  { OpParent,      {None,             None,         OutNone         }},
  { OpLateBoundCls,{None,             None,         OutNone         }},
  { OpRecordReifiedGeneric,
                   {Stack1,           Stack1,       OutVArray       }},
  { OpReifiedName, {Stack1,           Stack1,       OutString       }},
  { OpCheckReifiedGenericMismatch,
                   {Stack1,           None,         OutNone         }},
  { OpNativeImpl,  {None,             None,         OutNone         }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject       }},
  { OpIdx,         {StackTop3,        Stack1,       OutUnknown      }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown      }},
  { OpCheckProp,   {None,             Stack1,       OutBoolean      }},
  { OpInitProp,    {Stack1,           None,         OutNone         }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone         }},
  { OpAssertRATL,  {None,             None,         OutNone         }},
  { OpAssertRATStk,{None,             None,         OutNone         }},
  { OpBreakTraceHint,{None,           None,         OutNone         }},
  { OpGetMemoKeyL, {Local,            Stack1,       OutUnknown      }},
  { OpResolveFunc, {None,             Stack1,       OutFunc         }},
  { OpResolveObjMethod,
                   {StackTop2,        Stack1,       OutVArray        }},
  { OpResolveClsMethod,
                   {StackTop2,        Stack1,       OutClsMeth       }},

  /*** 14. Generator instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutNull         }},
  { OpContEnter,   {Stack1,           Stack1,       OutUnknown      }},
  { OpContRaise,   {Stack1,           Stack1,       OutUnknown      }},
  { OpYield,       {Stack1,           Stack1,       OutUnknown      }},
  { OpYieldK,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpContAssignDelegate,
                   {Stack1,           None,         OutNone         }},
  { OpContEnterDelegate,
                   {Stack1,           None,         OutNone         }},
  { OpYieldFromDelegate,
                   {None,             Stack1,       OutUnknown      }},
  { OpContUnsetDelegate,
                   {None,             None,         OutNone         }},
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
  { OpBaseSC,      {StackI,           MBase,        OutNone         }},
  { OpBaseL,       {Local,            MBase,        OutNone         }},
  { OpBaseC,       {StackI,           MBase,        OutNone         }},
  { OpBaseH,       {None,             MBase,        OutNone         }},
  { OpDim,         {MBase|MKey,       MBase,        OutNone         }},
  { OpQueryM,      {BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpSetM,        {Stack1|BStackN|MBase|MKey,
                                      Stack1,       OutUnknown      }},
  { OpSetRangeM,   {BStackN|MBase,    None,         OutNone         }},
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
};

namespace {
hphp_hash_map<Op, InstrInfo> instrInfo;
bool instrInfoInited;
}

void initInstrInfo() {
  if (!instrInfoInited) {
    for (auto& info : instrInfoSparse) {
      instrInfo[info.op] = info.info;
    }
    if (!RuntimeOption::EvalCheckReturnTypeHints) {
      auto& ii = instrInfo[OpVerifyRetTypeC];
      ii.in = ii.out = None;
      ii.type = OutNone;

      auto& ii2 = instrInfo[OpVerifyRetTypeTS];
      ii2.in = Stack1;
      ii2.out = None;
      ii2.type = OutNone;
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
    DontGuardAny | This | MBase | StackI | MKey | LocalRange |
    DontGuardBase;
  mask &= ~ignore;

  static const uint64_t counts[][2] = {
    {Stack3,       1},
    {Stack2,       1},
    {Stack1,       1},
    {StackIns1,    2},
    {FStack,       kNumActRecCells},
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
    case Op::FPushFunc:
    case Op::FPushFuncD:
    case Op::FPushFuncRD:
    case Op::FPushClsMethod:
    case Op::FPushClsMethodS:
    case Op::FPushClsMethodSD:
    case Op::FPushClsMethodSRD:
    case Op::FPushClsMethodD:
    case Op::FPushClsMethodRD:
      return getImm(pc, 0).u_IVA + countOperands(getInstrInfo(op).in) + 3;

    case Op::FCallCtor:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD:
    case Op::FCallObjMethodRD: {
      auto const fca = getImm(pc, 0).u_FCA;
      auto const nin = countOperands(getInstrInfo(op).in);
      return nin + fca.numArgsInclUnpack() + kNumActRecCells - 1 + fca.numRets;
    }

    case Op::FCall: {
      auto const fca = getImm(pc, 0).u_FCA;
      return fca.numArgsInclUnpack() + fca.numRets + kNumActRecCells - 1;
    }

    case Op::QueryM:
    case Op::IncDecM:
    case Op::UnsetM:
    case Op::NewPackedArray:
    case Op::NewVecArray:
    case Op::NewKeysetArray:
    case Op::NewVArray:
    case Op::ConcatN:
    case Op::CombineAndResolveTypeStruct:
    case Op::FCallBuiltin:
    case Op::CreateCl:
      return getImm(pc, 0).u_IVA;

    case Op::SetM:
    case Op::SetOpM:
      return getImm(pc, 0).u_IVA + 1;

    case Op::NewRecord:
    case Op::NewStructArray:
    case Op::NewStructDArray:
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
    case Op::FPushFunc:
    case Op::FPushFuncD:
    case Op::FPushFuncRD:
    case Op::FPushClsMethod:
    case Op::FPushClsMethodS:
    case Op::FPushClsMethodSD:
    case Op::FPushClsMethodSRD:
    case Op::FPushClsMethodD:
    case Op::FPushClsMethodRD:
      return getImm(pc, 0).u_IVA + kNumActRecCells;
    case Op::FCall:
    case Op::FCallCtor:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD:
    case Op::FCallObjMethodRD:
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
  case Op::DefClsNop:
  case Op::Nop:
  case Op::CGetCUNop:
  case Op::UGetCUNop:
  case Op::EntryNop:
    return true;
  case Op::VerifyRetTypeC:
    return !RuntimeOption::EvalCheckReturnTypeHints;
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
// Iterator bytecodes are handled specially here
#define LA(n) assertx(idx == 0xff); idx = n;
#define MA(n)
#define BLA(n)
#define SLA(n)
#define ILA(n)
#define I32LA(n)
#define IVA(n)
#define I64A(n)
#define IA(n)
#define CAR(n)
#define CAW(n)
#define DA(n)
#define SA(n)
#define AA(n)
#define RATA(n)
#define BA(n)
#define OA(op) BA
#define VSA(n)
#define KA(n)
#define LAR(n)
#define FCA(n)
#define O(name, imm, ...) case Op::name: imm break;

size_t localImmIdx(Op op) {
  switch (op) {
    case Op::LIterInit:
    case Op::LIterInitK:
    case Op::LIterNext:
    case Op::LIterNextK:
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
#undef KA
#define LA(n)
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
#undef LA
#undef MA
#undef BLA
#undef SLA
#undef ILA
#undef I32LA
#undef IVA
#undef I64A
#undef IA
#undef CAR
#undef CAW
#undef DA
#undef SA
#undef AA
#undef RATA
#undef BA
#undef OA
#undef VSA
#undef KA
#undef LAR
#undef FCA
#undef O

/*
 * Get location metadata for the inputs of `ni'.
 */
InputInfoVec getInputs(const NormalizedInstruction& ni, FPInvOffset bcSPOff) {
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

  if (isLegacyFCall(ni.op())) {
    stackOff -= ni.imm[0].u_FCA.numArgsInclUnpack();  // arguments consumed
    stackOff -= kNumActRecCells;  // ActRec is torn down as well
  }

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
        to<FPInvOffset>(bcSPOff)
    });
  }
  if (flags & StackN) {
    int numArgs = (ni.op() == Op::NewPackedArray ||
                   ni.op() == Op::NewVecArray ||
                   ni.op() == Op::NewKeysetArray ||
                   ni.op() == Op::NewVArray ||
                   ni.op() == Op::CombineAndResolveTypeStruct ||
                   ni.op() == Op::ConcatN)
      ? ni.imm[0].u_IVA
      : ni.immVec.numStackValues();

    SKTRACE(1, sk, "getInputs: StackN %d %d\n", stackOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location::Stack { stackOff-- });
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (flags & BStackN) {
    int numArgs = ni.imm[0].u_IVA;

    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", stackOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location::Stack { stackOff-- });
    }
  }
  if (hasFPushEffects(ni.op())) {
    auto const numArgs = isLegacyFPush(ni.op())
      ? ni.imm[0].u_IVA : ni.imm[0].u_FCA.numArgsInclUnpack();

    SKTRACE(1, sk, "getInputs: %s %d %d\n",
            opcodeToName(ni.op()), stackOff.offset, numArgs);
    stackOff -= numArgs + 2;
    switch (ni.op()) {
      case Op::FCallCtor:
      case Op::FCallObjMethod:
      case Op::FCallObjMethodD:
      case Op::FCallObjMethodRD:
        inputs.emplace_back(Location::Stack { stackOff-- });
        break;
      default:
        stackOff--;
        break;
    }
  }

  if (flags & Local) {
    // (Almost) all instructions that take a Local have its index at their
    // first immediate.
    if (ni.op() == Op::IterBreak) {
      for (auto const& it : ni.immIters) {
        if (it.kind != KindOfLIter) continue;
        SKTRACE(1, sk, "getInputs: local %d\n", it.local);
        inputs.emplace_back(Location::Local { uint32_t(it.local) });
      }
    } else {
      auto const loc = ni.imm[localImmIdx(ni.op())].u_IVA;
      SKTRACE(1, sk, "getInputs: local %d\n", loc);
      inputs.emplace_back(Location::Local { uint32_t(loc) });
    }
  }

  if (flags & LocalRange) {
    auto const& range = ni.imm[1].u_LAR;
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
        inputs.emplace_back(Location::Local { uint32_t(mk.iva) });
        break;
      case MEC:
      case MPC:
        inputs.emplace_back(Location::Stack {
          BCSPRelOffset{int32_t(mk.iva)}.to<FPInvOffset>(bcSPOff)
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
  if (flags & MBase) {
    inputs.emplace_back(Location::MBase{});
    if (flags & DontGuardBase) {
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackOff.offset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if ((flags & DontGuardAny) || dontGuardAnyInputs(ni)) {
    for (auto& info : inputs) info.dontGuard = true;
  }
  return inputs;
}

bool dontGuardAnyInputs(const NormalizedInstruction& ni) {
  switch (ni.op()) {
  case Op::IterBreak:
  case Op::IterNext:
  case Op::IterNextK:
  case Op::LIterNext:
  case Op::LIterNextK:
  case Op::IterInitK:
  case Op::IterInit:
  case Op::LIterInitK:
  case Op::LIterInit:
  case Op::JmpZ:
  case Op::JmpNZ:
  case Op::Jmp:
  case Op::JmpNS:
  case Op::FCall:
  case Op::ClsCnsD:
  case Op::FCallBuiltin:
  case Op::NewStructArray:
  case Op::NewStructDArray:
  case Op::NewStructDict:
  case Op::Switch:
  case Op::SSwitch:
  case Op::Lt:
  case Op::Lte:
  case Op::Gt:
  case Op::Gte:
  case Op::Cmp:
  case Op::SetOpL:
  case Op::InitProp:
  case Op::BreakTraceHint:
  case Op::IsTypeL:
  case Op::IsTypeC:
  case Op::IncDecL:
  case Op::DefCls:
  case Op::DefRecord:
  case Op::AliasCls:
  case Op::Eq:
  case Op::Neq:
  case Op::AssertRATL:
  case Op::AssertRATStk:
  case Op::SetL:
  case Op::EmptyL:
  case Op::CastBool:
  case Op::Same:
  case Op::NSame:
  case Op::Yield:
  case Op::YieldK:
  case Op::ContEnter:
  case Op::ContRaise:
  case Op::CreateCont:
  case Op::Await:
  case Op::AwaitAll:
  case Op::BitAnd:
  case Op::BitOr:
  case Op::BitXor:
  case Op::Sub:
  case Op::Mul:
  case Op::SubO:
  case Op::MulO:
  case Op::Add:
  case Op::AddO:
  case Op::ClsRefGetC:
  case Op::ClsRefGetTS:
  case Op::AKExists:
  case Op::AddElemC:
  case Op::AddNewElemC:
  case Op::Array:
  case Op::Dict:
  case Op::Keyset:
  case Op::Vec:
  case Op::ArrayIdx:
  case Op::BareThis:
  case Op::BitNot:
  case Op::CGetG:
  case Op::CGetQuietG:
  case Op::CGetL:
  case Op::CGetQuietL:
  case Op::CGetL2:
  case Op::CGetS:
  case Op::CUGetL:
  case Op::CastArray:
  case Op::CastDouble:
  case Op::CastInt:
  case Op::CastObject:
  case Op::CastString:
  case Op::CastDict:
  case Op::CastKeyset:
  case Op::CastVec:
  case Op::CastVArray:
  case Op::CastDArray:
  case Op::DblAsBits:
  case Op::CheckProp:
  case Op::CheckThis:
  case Op::Clone:
  case Op::CnsE:
  case Op::ColFromArray:
  case Op::CombineAndResolveTypeStruct:
  case Op::RecordReifiedGeneric:
  case Op::ReifiedName:
  case Op::CheckReifiedGenericMismatch:
  case Op::ConcatN:
  case Op::Concat:
  case Op::ContCheck:
  case Op::ContCurrent:
  case Op::ContKey:
  case Op::ContValid:
  case Op::ContGetReturn:
  case Op::CreateCl:
  case Op::DefCns:
  case Op::Dir:
  case Op::Div:
  case Op::Double:
  case Op::Dup:
  case Op::EmptyG:
  case Op::EmptyS:
  case Op::FPushClsMethodD:
  case Op::FPushClsMethodRD:
  case Op::FPushClsMethod:
  case Op::FPushClsMethodS:
  case Op::FPushClsMethodSD:
  case Op::FPushClsMethodSRD:
  case Op::FCallCtor:
  case Op::FPushFunc:
  case Op::FPushFuncD:
  case Op::FPushFuncRD:
  case Op::FCallObjMethodD:
  case Op::FCallObjMethodRD:
  case Op::ResolveFunc:
  case Op::ResolveClsMethod:
  case Op::ResolveObjMethod:
  case Op::False:
  case Op::File:
  case Op::GetMemoKeyL:
  case Op::Idx:
  case Op::InitThisLoc:
  case Op::InstanceOf:
  case Op::InstanceOfD:
  case Op::IsLateBoundCls:
  case Op::IsTypeStructC:
  case Op::Int:
  case Op::IssetG:
  case Op::IssetL:
  case Op::IssetS:
  case Op::IterFree:
  case Op::LIterFree:
  case Op::LateBoundCls:
  case Op::Method:
  case Op::Mod:
  case Op::Pow:
  case Op::ClsRefName:
  case Op::NativeImpl:
  case Op::NewArray:
  case Op::NewCol:
  case Op::NewPair:
  case Op::NewLikeArrayL:
  case Op::NewMixedArray:
  case Op::NewDictArray:
  case Op::NewPackedArray:
  case Op::NewVecArray:
  case Op::NewKeysetArray:
  case Op::NewVArray:
  case Op::NewDArray:
  case Op::NewObj:
  case Op::NewObjD:
  case Op::NewObjRD:
  case Op::NewObjS:
  case Op::NewRecord:
  case Op::Not:
  case Op::Null:
  case Op::NullUninit:
  case Op::OODeclExists:
  case Op::Parent:
  case Op::DiscardClsRef:
  case Op::PopC:
  case Op::PopV:
  case Op::PopU:
  case Op::PopU2:
  case Op::PopL:
  case Op::Print:
  case Op::PushL:
  case Op::RetC:
  case Op::RetCSuspended:
  case Op::Self:
  case Op::SetG:
  case Op::SetS:
  case Op::Shl:
  case Op::Shr:
  case Op::Silence:
  case Op::String:
  case Op::This:
  case Op::Throw:
  case Op::ThrowAsTypeStructException:
  case Op::True:
  case Op::UnsetL:
  case Op::VGetL:
  case Op::VerifyParamType:
  case Op::VerifyParamTypeTS:
  case Op::VerifyRetTypeC:
  case Op::VerifyRetTypeTS:
  case Op::VerifyRetNonNullC:
  case Op::VerifyOutType:
  case Op::WHResult:
  case Op::Xor:
  case Op::BaseGC:
  case Op::BaseGL:
  case Op::BaseSC:
  case Op::BaseL:
  case Op::BaseC:
  case Op::BaseH:
  case Op::Dim:
  case Op::QueryM:
  case Op::SetM:
  case Op::IncDecM:
  case Op::SetOpM:
  case Op::UnsetM:
  case Op::SetRangeM:
  case Op::MemoGet:
  case Op::MemoGetEager:
  case Op::MemoSet:
  case Op::MemoSetEager:
  case Op::RetM:
  case Op::Select:
  case Op::LockObj:
    return false;

  // These are instructions that are always interp-one'd, or are always no-ops.
  case Op::Nop:
  case Op::EntryNop:
  case Op::CGetCUNop:
  case Op::UGetCUNop:
  case Op::ClsCns:
  case Op::Exit:
  case Op::Fatal:
  case Op::SetOpG:
  case Op::SetOpS:
  case Op::IncDecG:
  case Op::IncDecS:
  case Op::UnsetG:
  case Op::FCallObjMethod:
  case Op::Incl:
  case Op::InclOnce:
  case Op::Req:
  case Op::ReqOnce:
  case Op::ReqDoc:
  case Op::Eval:
  case Op::DefClsNop:
  case Op::DefTypeAlias:
  case Op::ChainFaults:
  case Op::ContAssignDelegate:
  case Op::ContEnterDelegate:
  case Op::YieldFromDelegate:
  case Op::ContUnsetDelegate:
    return true;
  }

  always_assert_flog(0, "invalid opcode {}\n", static_cast<uint32_t>(ni.op()));
}

bool instrBreaksProfileBB(const NormalizedInstruction* inst) {
  if (hasFCallEffects(inst->op())) return true;

  if (instrIsNonCallControlFlow(inst->op()) ||
      inst->op() == OpAwait || // may branch to scheduler and suspend execution
      inst->op() == OpAwaitAll || // similar to Await
      inst->op() == OpClsCnsD || // side exits if misses in the RDS
      inst->op() == OpVerifyParamTypeTS || // avoids combinatorial explosion
      inst->op() == OpVerifyParamType) {   // with nullable types
    return true;
  }
  // In profiling mode, don't trace through a control flow merge point,
  // however, allow inlining of default parameter funclets
  assertx(profData());
  if (profData()->anyBlockEndsAt(inst->func(), inst->offset()) &&
      !inst->func()->isEntry(inst->nextSk().offset())) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

#define IMM_BLA(n)     ni.immVec
#define IMM_SLA(n)     ni.immVec
#define IMM_ILA(n)     ni.immIters
#define IMM_I32LA(n)   ni.immVec
#define IMM_VSA(n)     ni.immVec
#define IMM_IVA(n)     ni.imm[n].u_IVA
#define IMM_I64A(n)    ni.imm[n].u_I64A
#define IMM_LA(n)      ni.imm[n].u_LA
#define IMM_IA(n)      ni.imm[n].u_IA
#define IMM_CAR(n)     ni.imm[n].u_CAR
#define IMM_CAW(n)     ni.imm[n].u_CAW
#define IMM_DA(n)      ni.imm[n].u_DA
#define IMM_SA(n)      ni.unit()->lookupLitstrId(ni.imm[n].u_SA)
#define IMM_RATA(n)    ni.imm[n].u_RATA
#define IMM_AA(n)      ni.unit()->lookupArrayId(ni.imm[n].u_AA)
#define IMM_BA(n)      ni.imm[n].u_BA
#define IMM_OA_IMPL(n) ni.imm[n].u_OA
#define IMM_OA(subop)  (subop)IMM_OA_IMPL
#define IMM_KA(n)      ni.imm[n].u_KA
#define IMM_LAR(n)     ni.imm[n].u_LAR
#define IMM_FCA(n)     ni.imm[n].u_FCA

#define ONE(x0)           , IMM_##x0(0)
#define TWO(x0,x1)        , IMM_##x0(0), IMM_##x1(1)
#define THREE(x0,x1,x2)   , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2)
#define FOUR(x0,x1,x2,x3) , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3)
#define FIVE(x0,x1,x2,x3,x4) , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3), IMM_##x4(4)
#define NA                   /*  */

static void translateDispatch(irgen::IRGS& irgs,
                              const NormalizedInstruction& ni) {
#define O(nm, imms, ...) case Op::nm: irgen::emit##nm(irgs imms); return;
  switch (ni.op()) { OPCODES }
#undef O
}

#undef FIVE
#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_I32LA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_CAR
#undef IMM_CAW
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
#undef IMM_FCA

//////////////////////////////////////////////////////////////////////

namespace {

Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return TCell;  // TODO(#3029148) this could be InitCell
    case CUV: return TCell;
    case UV: return TUninit;
    case VV: return TBoxedInitCell;
    case CVV: case CVUV: return TGen;
  }
  not_reached();
}

}

void translateInstr(irgen::IRGS& irgs, const NormalizedInstruction& ni,
                    bool /*checkOuterTypeOnly*/, bool /*firstInst*/
                    ) {
  irgen::prepareForNextHHBC(irgs, &ni, ni.source);

  const Func* builtinFunc = nullptr;
  if (ni.op() == OpFCallBuiltin) {
    auto str = ni.m_unit->lookupLitstrId(ni.imm[2].u_SA);
    builtinFunc = Unit::lookupBuiltin(str);
  }
  auto pc = ni.pc();
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    if (hasFCallEffects(ni.op()) && instrInputFlavor(pc, i) == UV) {
      // This is a hack to deal with the fact that this instruction is
      // actually popping an ActRec in the middle of its "pops." We could
      // assert on the Uninit values on the stack, but the call is going to
      // override them anyway so it's not worth guarding on them.
      break;
    }
    auto const type =
      !builtinFunc ? flavorToType(instrInputFlavor(pc, i)) :
      builtinFunc->byRef(num - i - 1) ? TGen : TCell;
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

  if (ni.forceSurpriseCheck) {
    surpriseCheck(irgs);
  }

  translateDispatch(irgs, ni);

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
    lookupUnit(spath.get(), "", nullptr /* initial_opt */,
               Native::s_noNativeFuncs);
  });
}

///////////////////////////////////////////////////////////////////////////////
}
