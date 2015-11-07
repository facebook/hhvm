/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/map-walker.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/bc-pattern.h"

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
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

Lease Translator::s_writeLease;

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

  { OpPopA,        {Stack1,           None,         OutNone         }},
  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone         }},
  { OpPopV,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone         }},
  { OpPopR,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone         }},
  { OpDup,         {Stack1,           StackTop2,    OutSameAsInput  }},
  { OpBox,         {Stack1,           Stack1,       OutVInput       }},
  { OpUnbox,       {Stack1,           Stack1,       OutCInput       }},
  { OpBoxR,        {Stack1,           Stack1,       OutVInput       }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput       }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull         }},
  { OpNullUninit,  {None,             Stack1,       OutNullUninit   }},
  { OpTrue,        {None,             Stack1,       OutBooleanImm   }},
  { OpFalse,       {None,             Stack1,       OutBooleanImm   }},
  { OpInt,         {None,             Stack1,       OutInt64        }},
  { OpDouble,      {None,             Stack1,       OutDouble       }},
  { OpString,      {None,             Stack1,       OutStringImm    }},
  { OpArray,       {None,             Stack1,       OutArrayImm     }},
  { OpNewArray,    {None,             Stack1,       OutArray        }},
  { OpNewMixedArray,  {None,          Stack1,       OutArray        }},
  { OpNewLikeArrayL,  {Local,         Stack1,       OutArray        }},
  { OpNewPackedArray, {StackN,        Stack1,       OutArray        }},
  { OpNewStructArray, {StackN,        Stack1,       OutArray        }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray        }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray        }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray        }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray        }},
  { OpNewCol,      {None,             Stack1,       OutObject       }},
  { OpColFromArray,   {Stack1,        Stack1,       OutObject       }},
  { OpMapAddElemC, {StackTop3,        Stack1,       OutObject       }},
  { OpColAddNewElemC, {StackTop2,     Stack1,       OutObject       }},
  { OpCns,         {None,             Stack1,       OutCns          }},
  { OpCnsE,        {None,             Stack1,       OutCns          }},
  { OpCnsU,        {None,             Stack1,       OutCns          }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown      }},
  { OpClsCnsD,     {None,             Stack1,       OutUnknown      }},
  { OpFile,        {None,             Stack1,       OutString       }},
  { OpDir,         {None,             Stack1,       OutString       }},
  { OpNameA,       {Stack1,           Stack1,       OutString       }},

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
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean      }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutPredBool     }},
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
   * RetC and RetV are special. Their manipulation of the runtime stack are
   * outside the boundaries of the tracelet abstraction; since they always end
   * a basic block, they behave more like "glue" between BBs than the
   * instructions in the body of a BB.
   *
   * RetC and RetV consume a value from the stack, and this value's type needs
   * to be known at compile-time.
   */
  { OpRetC,        {AllLocals,        None,         OutNone         }},
  { OpRetV,        {AllLocals,        None,         OutNone         }},
  { OpThrow,       {Stack1,           None,         OutNone         }},
  { OpUnwind,      {None,             None,         OutNone         }},

  /*** 5. Get instructions ***/

  { OpCGetL,       {Local,            Stack1,       OutCInputL      }},
  { OpCGetL2,      {Stack1|DontGuardStack1|
                    Local,            StackIns1,    OutCInputL      }},
  { OpCGetL3,      {StackTop2|Local,  StackIns2,    OutCInputL      }},
  // In OpCUGetL we rely on OutCInputL returning TCell (which covers Uninit
  // values) instead of TInitCell.
  { OpCUGetL,      {Local,            Stack1,       OutCInputL      }},
  { OpPushL,       {Local,            Stack1|Local, OutCInputL      }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown      }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown      }},
  { OpCGetS,       {StackTop2,        Stack1,       OutUnknown      }},
  { OpCGetM,       {MVector,          Stack1,       OutUnknown      }},
  { OpVGetL,       {Local,            Stack1|Local, OutVInputL      }},
  { OpVGetN,       {Stack1,           Stack1|Local, OutVUnknown     }},
  // TODO: In pseudo-main, the VGetG instruction invalidates what we know
  // about the types of the locals because it could cause any one of the
  // local variables to become "boxed". We need to add logic to tracelet
  // analysis to deal with this properly.
  { OpVGetG,       {Stack1,           Stack1,       OutVUnknown     }},
  { OpVGetS,       {StackTop2,        Stack1,       OutVUnknown     }},
  { OpVGetM,       {MVector,          Stack1|Local, OutVUnknown     }},
  { OpAGetC,       {Stack1,           Stack1,       OutClassRef     }},
  { OpAGetL,       {Local,            Stack1,       OutClassRef     }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean      }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean      }},
  { OpIssetN,      {Stack1,           Stack1,       OutBoolean      }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean      }},
  { OpIssetS,      {StackTop2,        Stack1,       OutBoolean      }},
  { OpIssetM,      {MVector,          Stack1,       OutBoolean      }},
  { OpEmptyL,      {Local,            Stack1,       OutBoolean      }},
  { OpEmptyN,      {Stack1,           Stack1,       OutBoolean      }},
  { OpEmptyG,      {Stack1,           Stack1,       OutBoolean      }},
  { OpEmptyS,      {StackTop2,        Stack1,       OutBoolean      }},
  { OpEmptyM,      {MVector,          Stack1,       OutBoolean      }},
  { OpIsTypeC,     {Stack1|
                    DontGuardStack1,  Stack1,       OutBoolean      }},
  { OpIsTypeL,     {Local,            Stack1,       OutIsTypeL      }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput  }},
  { OpSetN,        {StackTop2,        Stack1|Local, OutSameAsInput  }},
  { OpSetG,        {StackTop2,        Stack1,       OutSameAsInput  }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput  }},
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutUnknown      }},
  { OpSetWithRefLM,{MVector|Local ,   Local,        OutNone         }},
  { OpSetWithRefRM,{MVector|Stack1,   Local,        OutNone         }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp        }},
  { OpSetOpN,      {StackTop2,        Stack1|Local, OutUnknown      }},
  { OpSetOpG,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpSetOpS,      {StackTop3,        Stack1,       OutUnknown      }},
  { OpSetOpM,      {MVector|Stack1,   Stack1|Local, OutUnknown      }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec       }},
  { OpIncDecN,     {Stack1,           Stack1|Local, OutUnknown      }},
  { OpIncDecG,     {Stack1,           Stack1,       OutUnknown      }},
  { OpIncDecS,     {StackTop2,        Stack1,       OutUnknown      }},
  { OpIncDecM,     {MVector,          Stack1|Local, OutUnknown      }},
  { OpBindL,       {Stack1|Local|
                    IgnoreInnerType,  Stack1|Local, OutSameAsInput  }},
  { OpBindN,       {StackTop2,        Stack1|Local, OutSameAsInput  }},
  { OpBindG,       {StackTop2,        Stack1,       OutSameAsInput  }},
  { OpBindS,       {StackTop3,        Stack1,       OutSameAsInput  }},
  { OpBindM,       {MVector|Stack1,   Stack1|Local, OutSameAsInput  }},
  { OpUnsetL,      {Local,            Local,        OutNone         }},
  { OpUnsetN,      {Stack1,           Local,        OutNone         }},
  { OpUnsetG,      {Stack1,           None,         OutNone         }},
  { OpUnsetM,      {MVector,          Local,        OutNone         }},

  /*** 8. Call instructions ***/

  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushFuncD,  {None,             FStack,       OutFDesc        }},
  { OpFPushFuncU,  {None,             FStack,       OutFDesc        }},
  { OpFPushObjMethod,
                   {StackTop2,        FStack,       OutFDesc        }},
  { OpFPushObjMethodD,
                   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushClsMethod,
                   {StackTop2,        FStack,       OutFDesc        }},
  { OpFPushClsMethodF,
                   {StackTop2,        FStack,       OutFDesc        }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc        }},
  { OpFPushCtor,   {Stack1,           Stack1|FStack,OutObject       }},
  { OpFPushCtorD,  {None,             Stack1|FStack,OutObject       }},
  { OpFPushCufIter,{None,             FStack,       OutFDesc        }},
  { OpFPushCuf,    {Stack1,           FStack,       OutFDesc        }},
  { OpFPushCufF,   {Stack1,           FStack,       OutFDesc        }},
  { OpFPushCufSafe,{StackTop2|DontGuardAny,
                                      StackTop2|FStack,
                                                    OutFPushCufSafe }},
  { OpFPassCW,     {FuncdRef,         None,         OutSameAsInput  }},
  { OpFPassCE,     {FuncdRef,         None,         OutSameAsInput  }},
  { OpFPassV,      {Stack1|FuncdRef,  Stack1,       OutUnknown      }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR      }},
  { OpFPassL,      {Local|FuncdRef,   Stack1,       OutFInputL      }},
  { OpFPassN,      {Stack1|FuncdRef,  Stack1,       OutUnknown      }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutUnknown      }},
  { OpFPassS,      {StackTop2|FuncdRef,
                                      Stack1,       OutUnknown      }},
  { OpFPassM,      {MVector|FuncdRef, Stack1|Local, OutUnknown      }},
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutUnknown      }},
  { OpFCallD,      {FStack,           Stack1,       OutUnknown      }},
  { OpFCallUnpack, {FStack,           Stack1,       OutUnknown      }},
  { OpFCallArray,  {FStack,           Stack1,       OutUnknown      }},
  { OpFCallBuiltin,{BStackN|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpCufSafeArray,{StackTop3|DontGuardAny,
                                      Stack1,       OutArray        }},
  { OpCufSafeReturn,{StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown      }},
  { OpDecodeCufIter,{Stack1,          None,         OutNone         }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown      }},
  { OpMIterInit,   {Stack1,           Local,        OutUnknown      }},
  { OpWIterInit,   {Stack1,           Local,        OutUnknown      }},
  { OpIterInitK,   {Stack1,           Local,        OutUnknown      }},
  { OpMIterInitK,  {Stack1,           Local,        OutUnknown      }},
  { OpWIterInitK,  {Stack1,           Local,        OutUnknown      }},
  { OpIterNext,    {None,             Local,        OutUnknown      }},
  { OpMIterNext,   {None,             Local,        OutUnknown      }},
  { OpWIterNext,   {None,             Local,        OutUnknown      }},
  { OpIterNextK,   {None,             Local,        OutUnknown      }},
  { OpMIterNextK,  {None,             Local,        OutUnknown      }},
  { OpWIterNextK,  {None,             Local,        OutUnknown      }},
  { OpIterFree,    {None,             None,         OutNone         }},
  { OpMIterFree,   {None,             None,         OutNone         }},
  { OpCIterFree,   {None,             None,         OutNone         }},
  { OpIterBreak,   {None,             None,         OutNone         }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown      }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown      }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown      }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown      }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown      }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown      }},
  { OpDefFunc,     {None,             None,         OutNone         }},
  { OpDefTypeAlias,{None,             None,         OutNone         }},
  { OpDefCls,      {None,             None,         OutNone         }},
  { OpDefCns,      {Stack1,           Stack1,       OutBoolean      }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject   }},
  { OpBareThis,    {None,             Stack1,       OutUnknown      }},
  { OpCheckThis,   {This,             None,         OutNone         }},
  { OpInitThisLoc,
                   {None,             Local,        OutUnknown      }},
  { OpStaticLoc,
                   {None,             Stack1,       OutBoolean      }},
  { OpStaticLocInit,
                   {Stack1,           Local,        OutVUnknown     }},
  { OpCatch,       {None,             Stack1,       OutObject       }},
  { OpVerifyParamType,
                   {Local,            Local,        OutUnknown      }},
  { OpVerifyRetTypeV,
                   {Stack1,           Stack1,       OutSameAsInput  }},
  { OpVerifyRetTypeC,
                   {Stack1,           Stack1,       OutSameAsInput  }},
  { OpOODeclExists,
                   {StackTop2,        Stack1,       OutBoolean      }},
  { OpSelf,        {None,             Stack1,       OutClassRef     }},
  { OpParent,      {None,             Stack1,       OutClassRef     }},
  { OpLateBoundCls,{None,             Stack1,       OutClassRef     }},
  { OpNativeImpl,  {None,             None,         OutNone         }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject       }},
  { OpIncStat,     {None,             None,         OutNone         }},
  { OpIdx,         {StackTop3,        Stack1,       OutUnknown      }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown      }},
  { OpCheckProp,   {None,             Stack1,       OutBoolean      }},
  { OpInitProp,    {Stack1,           None,         OutNone         }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone         }},
  { OpAssertRATL,  {None,             None,         OutNone         }},
  { OpAssertRATStk,{None,             None,         OutNone         }},
  { OpBreakTraceHint,{None,           None,         OutNone         }},
  { OpGetMemoKey,  {Stack1,           Stack1,       OutUnknown      }},

  /*** 14. Generator instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutNull         }},
  { OpContEnter,   {Stack1,           Stack1,       OutUnknown      }},
  { OpContRaise,   {Stack1,           Stack1,       OutUnknown      }},
  { OpYield,       {Stack1,           Stack1,       OutUnknown      }},
  { OpYieldK,      {StackTop2,        Stack1,       OutUnknown      }},
  { OpContCheck,   {None,             None,         OutNone         }},
  { OpContValid,   {None,             Stack1,       OutBoolean      }},
  { OpContStarted, {None,             Stack1,       OutBoolean      }},
  { OpContKey,     {None,             Stack1,       OutUnknown      }},
  { OpContCurrent, {None,             Stack1,       OutUnknown      }},
  { OpContGetReturn,
                   {None,             Stack1,       OutUnknown      }},

  /*** 15. Async functions instructions ***/

  { OpWHResult,    {Stack1,           Stack1,       OutUnknown      }},
  { OpAwait,       {Stack1,           Stack1,       OutUnknown      }},

  /*** 16. Member instructions ***/

  { OpBaseNC,      {StackI,           MBase,        OutNone         }},
  { OpBaseNL,      {Local,            MBase,        OutNone         }},
  { OpBaseGC,      {StackI,           MBase,        OutNone         }},
  { OpBaseGL,      {Local,            MBase,        OutNone         }},
  { OpBaseSC,      {StackI|IdxA,      MBase|IdxA,   OutUnknown      }},
  { OpBaseSL,      {Local|IdxA,       MBase|IdxA,   OutUnknown      }},
  { OpBaseL,       {Local,            MBase,        OutNone         }},
  { OpBaseC,       {StackI,           MBase,        OutNone         }},
  { OpBaseR,       {StackI,           MBase,        OutNone         }},
  { OpBaseH,       {None,             MBase,        OutNone         }},
  { OpDimL,        {Local|MBase,      MBase,        OutNone         }},
  { OpDimC,        {StackI|MBase,     MBase,        OutNone         }},
  { OpDimInt,      {MBase,            MBase,        OutNone         }},
  { OpDimStr,      {MBase,            MBase,        OutNone         }},
  { OpDimNewElem,  {MBase,            MBase,        OutNone         }},
  { OpQueryML,     {BStackN|Local|MBase,
                                      Stack1,       OutUnknown      }},
  { OpQueryMC,     {BStackN|MBase,    Stack1,       OutUnknown      }},
  { OpQueryMInt,   {BStackN|MBase,    Stack1,       OutUnknown      }},
  { OpQueryMStr,   {BStackN|MBase,    Stack1,       OutUnknown      }},
  { OpSetML,       {Stack1|BStackN|Local|MBase,
                                      Stack1,       OutUnknown      }},
  { OpSetMC,       {Stack1|BStackN|MBase,
                                      Stack1,       OutUnknown      }},
  { OpSetMInt,     {Stack1|BStackN|MBase,
                                      Stack1,       OutUnknown      }},
  { OpSetMStr,     {Stack1|BStackN|MBase,
                                      Stack1,       OutUnknown      }},
  { OpSetMNewElem, {Stack1|MBase,     Stack1,       OutUnknown      }},
};

static hphp_hash_map<Op, InstrInfo> instrInfo;
static bool instrInfoInited;
static void initInstrInfo() {
  if (!instrInfoInited) {
    for (auto& info : instrInfoSparse) {
      instrInfo[info.op] = info.info;
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
  assertx(instrInfoInited);
  return instrInfo[op];
}

namespace {
int64_t countOperands(uint64_t mask) {
  const uint64_t ignore = FuncdRef | Local | Iter | AllLocals |
    DontGuardStack1 | IgnoreInnerType | DontGuardAny | This |
    MBase | StackI | IdxA;
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
  assertx(mask == 0);
  return count;
}
}

int64_t getStackPopped(PC pc) {
  auto const op = peek_op(pc);
  switch (op) {
    case Op::FCall:        return getImm(pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallD:       return getImm(pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallArray:   return kNumActRecCells + 1;

    case Op::QueryML:   case Op::QueryMC:
    case Op::QueryMInt: case Op::QueryMStr:
    case Op::NewPackedArray:
    case Op::ConcatN:
    case Op::FCallBuiltin:
    case Op::CreateCl:     return getImm(pc, 0).u_IVA;

    case Op::SetML:   case Op::SetMC:
    case Op::SetMInt: case Op::SetMStr: case Op::SetMNewElem:
      return getImm(pc, 0).u_IVA + 1;

    case Op::NewStructArray: return getImmVector(pc).size();

    case Op::BaseSC: case Op::BaseSL: return getImm(pc, 1).u_IVA + 1;

    default:             break;
  }

  uint64_t mask = getInstrInfo(op).in;
  int64_t count = 0;

  // All instructions with these properties are handled above
  assertx((mask & (StackN | BStackN)) == 0);

  if (mask & MVector) {
    count += getImmVector(pc).numStackValues();
    mask &= ~MVector;
  }

  return count + countOperands(mask);
}

int64_t getStackPushed(PC pc) {
  auto const op = peek_op(pc);
  if (op == Op::BaseSC || op == Op::BaseSL) return getImm(pc, 1).u_IVA;

  return countOperands(getInstrInfo(op).out);
}

bool isAlwaysNop(Op op) {
  switch (op) {
  case Op::BoxRNop:
  case Op::DefClsNop:
  case Op::FPassC:
  case Op::FPassVNop:
  case Op::Nop:
  case Op::UnboxRNop:
  case Op::RGetCNop:
    return true;
  case Op::VerifyRetTypeC:
  case Op::VerifyRetTypeV:
    return !RuntimeOption::EvalCheckReturnTypeHints;
  default:
    return false;
  }
}

static void addMVectorInputs(NormalizedInstruction& ni,
                             BCSPOffset& spOff,
                             InputInfoVec& inputs) {
  assertx(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  spOff += ni.immVec.numStackValues();
  auto localSpOff = spOff - 1;

  auto push_stack = [&] {
    ++stackCount;
    inputs.emplace_back(Location(localSpOff--));
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
  auto opPtr = ni.source.pc();
  auto const location = getMLocation(opPtr);
  auto const lcode = location.lcode;

  const bool trailingClassRef = lcode == LSL || lcode == LSC;

  switch (numLocationCodeStackVals(lcode)) {
  case 0: {
    if (lcode == LH) {
      inputs.emplace_back(Location(Location::This));
    } else {
      assertx(lcode == LL || lcode == LGL || lcode == LNL);
      if (location.hasImm()) {
        push_local(location.imm);
      }
    }
  } break;
  case 1:
    if (lcode == LSL) {
      // We'll get the trailing stack value after pushing all the
      // member vector elements.
      assertx(location.hasImm());
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
        assertx(memberCodeImmIsInt(mcode));
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

  assertx(stackCount == ni.immVec.numStackValues());

  SKTRACE(2, ni.source, "M-vector using %d hidden stack "
                        "inputs, %d locals\n", stackCount, localCount);
}

/*
 * getInputs --
 *   Returns locations for this instruction's inputs.
 */
InputInfoVec getInputs(NormalizedInstruction& ni) {
  InputInfoVec inputs;
  auto UNUSED sk = ni.source;
  if (isAlwaysNop(ni.op())) return inputs;

  assertx(inputs.empty());
  always_assert_flog(
    instrInfo.count(ni.op()),
    "Invalid opcode in getInputsImpl: {}\n",
    opcodeToName(ni.op())
  );
  const InstrInfo& info = instrInfo[ni.op()];
  Operands input = info.in;
  BCSPOffset spOff{0};
  if (input & FuncdRef) {
    inputs.needsRefCheck = true;
  }
  if (input & Iter) {
    inputs.emplace_back(Location(Location::Iter, ni.imm[0].u_IVA));
  }
  if (input & FStack) {
    spOff += ni.imm[0].u_IVA; // arguments consumed
    spOff += kNumActRecCells; // ActRec is torn down as well
  }
  if (input & IgnoreInnerType) ni.ignoreInnerType = true;
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", spOff.offset);
    inputs.emplace_back(Location(spOff++));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", spOff.offset);
      inputs.emplace_back(Location(spOff++));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", spOff.offset);
        inputs.emplace_back(Location(spOff++));
      }
    }
  }
  if (input & StackI) {
    inputs.emplace_back(Location(BCSPOffset{ni.imm[0].u_IVA}));
  }
  if (input & StackN) {
    int numArgs = (ni.op() == Op::NewPackedArray ||
                   ni.op() == Op::ConcatN)
      ? ni.imm[0].u_IVA
      : ni.immVec.numStackValues();

    SKTRACE(1, sk, "getInputs: stackN %d %d\n", spOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(spOff++));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (input & BStackN) {
    int numArgs = ni.imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", spOff.offset, numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(spOff++));
    }
  }
  if (input & MVector) {
    addMVectorInputs(ni, spOff, inputs);
  }
  if (input & Local) {
    // (Almost) all instructions that take a Local have its index at
    // their first immediate.
    int loc;
    auto insertAt = inputs.end();
    switch (ni.op()) {
      case OpSetWithRefLM:
        insertAt = inputs.begin();
        // fallthrough
      case OpFPassL:
        loc = ni.imm[1].u_IVA;
        break;
      case OpQueryML:
      case OpSetML:
        loc = ni.imm[3].u_IVA;
        break;

      default:
        loc = ni.imm[0].u_IVA;
        break;
    }
    SKTRACE(1, sk, "getInputs: local %d\n", loc);
    inputs.emplace(insertAt, Location(Location::Local, loc));
  }

  if (input & AllLocals) {
    ni.ignoreInnerType = true;
  }

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", spOff.offset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if (inputs.size() &&
      ((input & DontGuardAny) || dontGuardAnyInputs(ni.op()))) {
    for (int i = inputs.size(); i--; ) {
      inputs[i].dontGuard = true;
    }
  }
  if (input & This) {
    inputs.emplace_back(Location(Location::This));
  }
  return inputs;
}

bool dontGuardAnyInputs(Op op) {
  switch (op) {
  case Op::IterBreak:
  case Op::DecodeCufIter:
  case Op::IterNext:
  case Op::IterNextK:
  case Op::WIterInit:
  case Op::WIterInitK:
  case Op::WIterNext:
  case Op::WIterNextK:
  case Op::MIterInit:
  case Op::MIterInitK:
  case Op::MIterNext:
  case Op::MIterNextK:
  case Op::IterInitK:
  case Op::IterInit:
  case Op::JmpZ:
  case Op::JmpNZ:
  case Op::Jmp:
  case Op::JmpNS:
  case Op::BindM:
  case Op::CGetM:
  case Op::EmptyM:
  case Op::FPassM:
  case Op::IncDecM:
  case Op::IssetM:
  case Op::SetM:
  case Op::SetOpM:
  case Op::SetWithRefLM:
  case Op::SetWithRefRM:
  case Op::UnsetM:
  case Op::VGetM:
  case Op::FCallArray:
  case Op::FCall:
  case Op::FCallD:
  case Op::ClsCnsD:
  case Op::FPassCW:
  case Op::FPassCE:
  case Op::FPassR:
  case Op::FPassV:
  case Op::FPassG:
  case Op::FPassL:
  case Op::FPassS:
  case Op::FCallBuiltin:
  case Op::NewStructArray:
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
  case Op::FPushCuf:
  case Op::FPushCufF:
  case Op::FPushCufSafe:
  case Op::IncStat:
  case Op::Eq:
  case Op::Neq:
  case Op::AssertRATL:
  case Op::AssertRATStk:
  case Op::SetL:
  case Op::BindL:
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
  case Op::BitAnd:
  case Op::BitOr:
  case Op::BitXor:
  case Op::Sub:
  case Op::Mul:
  case Op::SubO:
  case Op::MulO:
  case Op::Add:
  case Op::AddO:
  case Op::AGetC:
  case Op::AGetL:
  case Op::AKExists:
  case Op::AddElemC:
  case Op::AddNewElemC:
  case Op::Array:
  case Op::ArrayIdx:
  case Op::BareThis:
  case Op::BindG:
  case Op::BindS:
  case Op::BitNot:
  case Op::CGetG:
  case Op::CGetL:
  case Op::CGetL2:
  case Op::CGetS:
  case Op::CUGetL:
  case Op::CIterFree:
  case Op::CastArray:
  case Op::CastDouble:
  case Op::CastInt:
  case Op::CastObject:
  case Op::CastString:
  case Op::CheckProp:
  case Op::CheckThis:
  case Op::Clone:
  case Op::Cns:
  case Op::CnsE:
  case Op::CnsU:
  case Op::MapAddElemC:
  case Op::ColAddNewElemC:
  case Op::ColFromArray:
  case Op::ConcatN:
  case Op::Concat:
  case Op::ContCheck:
  case Op::ContCurrent:
  case Op::ContKey:
  case Op::ContValid:
  case Op::ContStarted:
  case Op::ContGetReturn:
  case Op::CreateCl:
  case Op::DefCns:
  case Op::DefFunc:
  case Op::Dir:
  case Op::Div:
  case Op::Double:
  case Op::Dup:
  case Op::EmptyG:
  case Op::EmptyS:
  case Op::FPushClsMethodD:
  case Op::FPushClsMethod:
  case Op::FPushClsMethodF:
  case Op::FPushCtor:
  case Op::FPushCtorD:
  case Op::FPushCufIter:
  case Op::FPushFunc:
  case Op::FPushFuncD:
  case Op::FPushFuncU:
  case Op::FPushObjMethodD:
  case Op::False:
  case Op::File:
  case Op::GetMemoKey:
  case Op::Idx:
  case Op::InitThisLoc:
  case Op::InstanceOf:
  case Op::InstanceOfD:
  case Op::Int:
  case Op::IssetG:
  case Op::IssetL:
  case Op::IssetS:
  case Op::IterFree:
  case Op::LateBoundCls:
  case Op::MIterFree:
  case Op::Mod:
  case Op::Pow:
  case Op::NameA:
  case Op::NativeImpl:
  case Op::NewArray:
  case Op::NewCol:
  case Op::NewLikeArrayL:
  case Op::NewMixedArray:
  case Op::NewPackedArray:
  case Op::Not:
  case Op::Null:
  case Op::NullUninit:
  case Op::OODeclExists:
  case Op::Parent:
  case Op::PopA:
  case Op::PopC:
  case Op::PopR:
  case Op::PopV:
  case Op::Print:
  case Op::PushL:
  case Op::RetC:
  case Op::RetV:
  case Op::Self:
  case Op::SetG:
  case Op::SetS:
  case Op::Shl:
  case Op::Shr:
  case Op::Silence:
  case Op::StaticLoc:
  case Op::StaticLocInit:
  case Op::String:
  case Op::This:
  case Op::True:
  case Op::Unbox:
  case Op::UnboxR:
  case Op::UnsetL:
  case Op::VGetG:
  case Op::VGetL:
  case Op::VGetS:
  case Op::VerifyParamType:
  case Op::VerifyRetTypeC:
  case Op::VerifyRetTypeV:
  case Op::WHResult:
  case Op::Xor:
  case Op::BaseNC:
  case Op::BaseNL:
  case Op::BaseGC:
  case Op::BaseGL:
  case Op::BaseSC:
  case Op::BaseSL:
  case Op::BaseL:
  case Op::BaseC:
  case Op::BaseR:
  case Op::BaseH:
  case Op::DimL:
  case Op::DimC:
  case Op::DimInt:
  case Op::DimStr:
  case Op::DimNewElem:
  case Op::QueryML:
  case Op::QueryMC:
  case Op::QueryMInt:
  case Op::QueryMStr:
  case Op::SetML:
  case Op::SetMC:
  case Op::SetMInt:
  case Op::SetMStr:
  case Op::SetMNewElem:
    return false;

  // These are instructions that are always interp-one'd, or are always no-ops.
  case Op::LowInvalid:
  case Op::Nop:
  case Op::Box:
  case Op::BoxR:
  case Op::BoxRNop:
  case Op::UnboxRNop:
  case Op::RGetCNop:
  case Op::AddElemV:
  case Op::AddNewElemV:
  case Op::ClsCns:
  case Op::Exit:
  case Op::Fatal:
  case Op::Unwind:
  case Op::Throw:
  case Op::CGetL3:
  case Op::CGetN:
  case Op::VGetN:
  case Op::IssetN:
  case Op::EmptyN:
  case Op::SetN:
  case Op::SetOpN:
  case Op::SetOpG:
  case Op::SetOpS:
  case Op::IncDecN:
  case Op::IncDecG:
  case Op::IncDecS:
  case Op::BindN:
  case Op::UnsetN:
  case Op::UnsetG:
  case Op::FPushObjMethod:
  case Op::FPassC:
  case Op::FPassVNop:
  case Op::FPassN:
  case Op::FCallUnpack:
  case Op::CufSafeArray:
  case Op::CufSafeReturn:
  case Op::Incl:
  case Op::InclOnce:
  case Op::Req:
  case Op::ReqOnce:
  case Op::ReqDoc:
  case Op::Eval:
  case Op::DefClsNop:
  case Op::DefTypeAlias:
  case Op::Catch:
  case Op::HighInvalid:
    return true;
  }

  always_assert_flog(0, "invalid opcode {}\n", static_cast<uint32_t>(op));
}

const StaticString s_http_response_header("http_response_header");
const StaticString s_php_errormsg("php_errormsg");
const StaticString s_extract("extract");
const StaticString s_extractNative("__SystemLib\\extract");
const StaticString s_parse_str("parse_str");
const StaticString s_parse_strNative("__SystemLib\\parse_str");
const StaticString s_assert("assert");
const StaticString s_assertNative("__SystemLib\\assert");
const StaticString s_set_frame_metadata("HH\\set_frame_metadata");

bool funcByNameDestroysLocals(const StringData* fname) {
  if (!fname) return false;
  return fname->isame(s_extract.get()) ||
         fname->isame(s_extractNative.get()) ||
         fname->isame(s_parse_str.get()) ||
         fname->isame(s_parse_strNative.get()) ||
         fname->isame(s_assert.get()) ||
         fname->isame(s_assertNative.get()) ||
         fname->isame(s_set_frame_metadata.get());
}

bool builtinFuncDestroysLocals(const Func* callee) {
  assertx(callee && callee->isCPPBuiltin());
  auto const fname = callee->name();
  return funcByNameDestroysLocals(fname);
}

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  // We don't handle these two cases, because we don't compile functions
  // containing them:
  assertx(caller->lookupVarId(s_php_errormsg.get()) == -1);
  assertx(caller->lookupVarId(s_http_response_header.get()) == -1);

  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);
    return funcByNameDestroysLocals(str);
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assertx(fpi);
  auto const fpushPc = unit->at(fpi->m_fpushOff);
  auto const op = peek_op(fpushPc);

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
      inst->op() == OpAwait || // may branch to scheduler and suspend execution
      inst->op() == OpClsCnsD) { // side exits if misses in the RDS
    return true;
  }
  // In profiling mode, don't trace through a control flow merge point,
  // however, allow inlining of default parameter funclets
  if (mcg->tx().profData()->anyBlockEndsAt(inst->func(), inst->offset()) &&
      !inst->func()->isEntry(inst->nextSk().offset())) {
    return true;
  }
  return false;
}

Translator::Translator()
  : uniqueStubs{}
  , m_createdTime(HPHP::Timer::GetCurrentTimeMicros())
  , m_mode(TransKind::Invalid)
  , m_profData(nullptr)
  , m_useAHot(RuntimeOption::RepoAuthoritative && CodeCache::AHotSize > 0)
{
  initInstrInfo();
  if (RuntimeOption::EvalJitPGO) {
    m_profData.reset(new ProfData());
  }
}

bool
Translator::isSrcKeyInBL(SrcKey sk) {
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
    pc = (pc == nullptr) ? unit->at(sk.offset()) : pc + instrLen(pc);
    if (m_dbgBLPC.checkPC(pc)) {
      m_dbgBLSrcKey.insert(sk);
      return true;
    }
  } while (!opcodeBreaksBB(peek_op(pc)));
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

const char* show(TranslateResult r) {
  switch (r) {
  case TranslateResult::Failure: return "Failure";
  case TranslateResult::Retry:   return "Retry";
  case TranslateResult::Success: return "Success";
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

#define IMM_MA(n)      0 /* ignored, but we need something (for commas) */
#define IMM_BLA(n)     ni.immVec
#define IMM_SLA(n)     ni.immVec
#define IMM_ILA(n)     ni.immVec
#define IMM_VSA(n)     ni.immVec
#define IMM_IVA(n)     ni.imm[n].u_IVA
#define IMM_I64A(n)    ni.imm[n].u_I64A
#define IMM_LA(n)      ni.imm[n].u_LA
#define IMM_IA(n)      ni.imm[n].u_IA
#define IMM_DA(n)      ni.imm[n].u_DA
#define IMM_SA(n)      ni.unit()->lookupLitstrId(ni.imm[n].u_SA)
#define IMM_RATA(n)    ni.imm[n].u_RATA
#define IMM_AA(n)      ni.unit()->lookupArrayId(ni.imm[n].u_AA)
#define IMM_BA(n)      ni.imm[n].u_BA
#define IMM_OA_IMPL(n) ni.imm[n].u_OA
#define IMM_OA(subop)  (subop)IMM_OA_IMPL

#define ONE(x0)           , IMM_##x0(0)
#define TWO(x0,x1)        , IMM_##x0(0), IMM_##x1(1)
#define THREE(x0,x1,x2)   , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2)
#define FOUR(x0,x1,x2,x3) , IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3)
#define NA                   /*  */

static void translateDispatch(IRGS& irgs,
                              const NormalizedInstruction& ni) {
#define O(nm, imms, ...) case Op::nm: irgen::emit##nm(irgs imms); return;
  switch (ni.op()) { OPCODES }
#undef O
}

#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA

//////////////////////////////////////////////////////////////////////

namespace {

Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return TCell;  // TODO(#3029148) this could be InitCell
    case CUV: return TCell;
    case UV: return TUninit;
    case VV: return TBoxedInitCell;
    case AV: return TCls;
    case RV: case CRV: case FV: case CVV: case CVUV: return TGen;
  }
  not_reached();
}

}

void translateInstr(
  IRGS& irgs,
  const NormalizedInstruction& ni,
  bool checkOuterTypeOnly,
  bool firstInst
) {
  irgen::prepareForNextHHBC(
    irgs,
    &ni,
    ni.source,
    ni.endsRegion && !irgen::isInlining(irgs)
  );

  const Func* builtinFunc = nullptr;
  if (ni.op() == OpFCallBuiltin) {
    auto str = ni.m_unit->lookupLitstrId(ni.imm[2].u_SA);
    builtinFunc = Unit::lookupFunc(str);
  }
  auto pc = ni.pc();
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type =
      !builtinFunc ? flavorToType(instrInputFlavor(pc, i)) :
      builtinFunc->byRef(num - i - 1) ? TGen : TCell;
    // TODO(#5706706): want to use assertTypeLocation, but Location::Stack
    // is a little unsure of itself.
    irgen::assertTypeStack(irgs, BCSPOffset{i}, type);
  }

  FTRACE(1, "\nTranslating {}: {} with state:\n{}\n",
         ni.offset(), ni, show(irgs));

  irgen::ringbufferEntry(irgs, Trace::RBTypeBytecodeStart, ni.source, 2);
  irgen::emitIncStat(irgs, Stats::Instr_TC, 1);
  if (Stats::enableInstrCount()) {
    irgen::emitIncStat(irgs, Stats::opToTranslStat(ni.op()), 1);
  }
  if (Trace::moduleEnabledRelease(Trace::llvm_count, 1) ||
      RuntimeOption::EvalJitLLVMCounters) {
    irgen::gen(irgs, CountBytecode);
  }

  if (isAlwaysNop(ni.op())) return;
  if (ni.interp || RuntimeOption::EvalJitAlwaysInterpOne) {
    irgen::interpOne(irgs, ni);
    return;
  }

  translateDispatch(irgs, ni);

  FTRACE(3, "\nTranslated {}: {} with state:\n{}\n",
         ni.offset(), ni, show(irgs));
}

//////////////////////////////////////////////////////////////////////

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
  assertx(id / transCountersPerChunk < m_transCounters.size());
  return &(m_transCounters[id / transCountersPerChunk]
           [id % transCountersPerChunk]);
}

void Translator::addTranslation(const TransRec& transRec) {
  if (Trace::moduleEnabledRelease(Trace::trans, 1)) {
    // Log the translation's size, creation time, SrcKey, and size
    Trace::traceRelease("New translation: %" PRId64 " %s %u %u %d\n",
                        HPHP::Timer::GetCurrentTimeMicros() - m_createdTime,
                        folly::format("{}:{}:{}",
                          transRec.src.unit()->filepath(),
                          transRec.src.funcID(),
                          transRec.src.offset()).str().c_str(),
                        transRec.aLen,
                        transRec.acoldLen,
                        static_cast<int>(transRec.kind));
  }

  if (!isTransDBEnabled()) return;
  uint32_t id = getCurrentTransID();
  m_translations.emplace_back(transRec);
  auto& newTransRec = m_translations[id];
  newTransRec.id = id;

  if (transRec.aLen > 0) {
    m_transDB[transRec.aStart] = id;
  }
  if (transRec.acoldLen > 0) {
    m_transDB[transRec.acoldStart] = id;
  }

  // Optimize storage of the created TransRec.
  newTransRec.optimizeForMemory();
}

uint64_t Translator::getTransCounter(TransID transId) const {
  if (!isTransDBEnabled()) return -1ul;
  assertx(transId < m_translations.size());

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
                                  const Class* ctx) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  if (cls->attrs() & AttrInterface) return nullptr;
  bool privateOnly = false;
  if (!(cls->attrs() & AttrUnique)) {
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

  assertx(res == LookupResult::MethodFoundWithThis ||
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

const Func* lookupImmutableCtor(const Class* cls, const Class* ctx) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  if (!(cls->attrs() & AttrUnique)) {
    if (!ctx || !ctx->classof(cls)) {
      return nullptr;
    }
  }

  auto const func = cls->getCtor();
  if (func && !(func->attrs() & AttrPublic)) {
    auto fcls = func->cls();
    if (fcls != ctx) {
      if (!ctx) return nullptr;
      if ((func->attrs() & AttrPrivate) ||
          !(ctx->classof(fcls) || fcls->classof(ctx))) {
        return nullptr;
      }
    }
  }

  return func;
}

///////////////////////////////////////////////////////////////////////////////
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
    lookupUnit(spath.get(), "", nullptr /* initial_opt */);
  });
}

///////////////////////////////////////////////////////////////////////////////
}
