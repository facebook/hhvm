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
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/bc-pattern.h"

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/irgen.h"

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
  { OpNewLikeArrayL,  {Local,         Stack1,       OutArray,          1 }},
  { OpNewPackedArray, {StackN,        Stack1,       OutArray,          0 }},
  { OpNewStructArray, {StackN,        Stack1,       OutArray,          0 }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpNewCol,      {None,             Stack1,       OutObject,         1 }},
  { OpColFromArray,   {Stack1,        Stack1,       OutObject,         0 }},
  { OpMapAddElemC, {StackTop3,        Stack1,       OutObject,        -2 }},
  { OpColAddNewElemC, {StackTop2,     Stack1,       OutObject,        -1 }},
  { OpCns,         {None,             Stack1,       OutCns,            1 }},
  { OpCnsE,        {None,             Stack1,       OutCns,            1 }},
  { OpCnsU,        {None,             Stack1,       OutCns,            1 }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpClsCnsD,     {None,             Stack1,       OutUnknown,           1 }},
  { OpFile,        {None,             Stack1,       OutString,         1 }},
  { OpDir,         {None,             Stack1,       OutString,         1 }},
  { OpNameA,       {Stack1,           Stack1,       OutString,         0 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  { OpConcatN,     {StackN,           Stack1,       OutString,         0 }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Arithmetic ops that overflow ints to floats */
  { OpAddO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpSubO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpMulO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpPow,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
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
  // In OpCUGetL we rely on OutCInputL returning TCell (which covers Uninit
  // values) instead of TInitCell.
  { OpCUGetL,      {Local,            Stack1,       OutCInputL,        1 }},
  { OpPushL,       {Local,            Stack1|Local, OutCInputL,        1 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutUnknown,        1 }},
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
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutUnknown,        0 }},
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
  { OpFCall,       {FStack,           Stack1,       OutUnknown,        0 }},
  { OpFCallD,      {FStack,           Stack1,       OutUnknown,        0 }},
  { OpFCallUnpack, {FStack,           Stack1,       OutUnknown,        0 }},
  { OpFCallArray,  {FStack,           Stack1,       OutUnknown,
                                                   -(int)kNumActRecCells }},
  { OpFCallBuiltin,{BStackN,          Stack1,       OutUnknown,        0 }},
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
  { OpCheckProp,   {None,             Stack1,       OutBoolean,        1 }},
  { OpInitProp,    {Stack1,           None,         OutNone,          -1 }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone,           0 }},
  { OpAssertRATL,  {None,             None,         OutNone,           0 }},
  { OpAssertRATStk,{None,             None,         OutNone,           0 }},
  { OpBreakTraceHint,{None,           None,         OutNone,           0 }},
  { OpGetMemoKey,  {Stack1,           Stack1,       OutUnknown,        0 }},

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

  { OpWHResult,    {Stack1,           Stack1,       OutUnknown,        0 }},
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
  assertx(instrInfoInited);
  return instrInfo[op];
}

static int numHiddenStackInputs(const NormalizedInstruction& ni) {
  assertx(ni.immVec.isValid());
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
  assertx(mask == 0);
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
  assertx((mask & (StackN | BStackN)) == 0);

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
                             int& currentStackOffset,
                             std::vector<InputInfo>& inputs) {
  assertx(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  currentStackOffset -= ni.immVec.numStackValues();
  int localStackOffset = currentStackOffset;

  auto push_stack = [&] {
    ++stackCount;
    inputs.emplace_back(Location(BCSPOffset{localStackOffset++}));
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
static void getInputsImpl(NormalizedInstruction* ni,
                          int& currentStackOffset,
                          InputInfoVec& inputs) {
#ifdef USE_TRACE
  auto sk = ni->source;
#endif
  if (isAlwaysNop(ni->op())) return;

  assertx(inputs.empty());
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
    inputs.emplace_back(Location(BCSPOffset{--currentStackOffset}));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.emplace_back(Location(BCSPOffset{--currentStackOffset}));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.emplace_back(Location(BCSPOffset{--currentStackOffset}));
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
      inputs.emplace_back(Location(BCSPOffset{--currentStackOffset}));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (input & BStackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(BCSPOffset{--currentStackOffset}));
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

  if (input & AllLocals) {
    ni->ignoreInnerType = true;
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

InputInfoVec getInputs(NormalizedInstruction& inst) {
  InputInfoVec infos;
  // MCGenerator expected top of stack to be index -1, with indexes growing
  // down from there. hhir defines top of stack to be index 0, with indexes
  // growing up from there. To compensate we start with a stack offset of 1 and
  // negate the index of any stack input after the call to getInputs.
  int stackOff = 1;
  getInputsImpl(&inst, stackOff, infos);
  for (auto& info : infos) {
    if (!info.loc.isStack()) continue;
    info.loc.bcRelOffset = -info.loc.bcRelOffset;
  }
  return infos;
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
  case Op::Strlen:
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
  , m_useAHot(RuntimeOption::RepoAuthoritative &&
              RuntimeOption::EvalJitAHotSize > 0)
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
    case RV: case FV: case CVV: case CVUV: return TGen;
  }
  not_reached();
}

Type typeToCheckForInput(
  IRGS& irgs,
  const NormalizedInstruction& ni,
  int32_t opndIdx,
  Type predictedType
) {
  auto opc = ni.op();
  auto tc = TypeConstraint(DataTypeSpecific);
  switch (opc) {
    case OpSetS:
    case OpSetG:
    case OpSetL: {
      // stack value
      if (opndIdx == 0) {
        tc = DataTypeCountness;
        break;
      }
      if (opc == OpSetL) {
        // old local value is dec-refed
        assert(opndIdx == 1);
        tc = DataTypeCountness;
        break;
      }
      tc = DataTypeSpecific;
      break;
    }

    case OpUnsetL: {
      tc = DataTypeCountness;
      break;
    }

    case OpCGetL:
    case OpVGetL:
    case OpFPassL: {
      tc = DataTypeCountnessInit;
      break;
    }

    case OpCUGetL: {
      tc = DataTypeCountness;
      break;
    }

    case OpPushL:
    case OpContEnter:
    case OpContRaise: {
      tc = DataTypeGeneric;
      break;
    }

    case OpRetC:
    case OpRetV: {
      tc = DataTypeCountness;
      break;
    }

    case OpFCallArray: {
      tc = DataTypeGeneric;
      break;
    }

    case OpPopC:
    case OpPopV:
    case OpPopR: {
      tc = DataTypeCountness;
      break;
    }

    case OpYield:
    case OpYieldK: {
      // The stack input is teleported to the continuation's m_value field
      tc = DataTypeGeneric;
      break;
    }

    case OpAddElemC: {
      // The stack input is teleported to the array
      tc = opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;
      break;
    }

    case OpIdx:
    case OpArrayIdx: {
      // The default value (w/ opndIdx 0) is simply passed to a helper,
      // which takes care of dec-refing it if needed
      tc = opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;
      break;
    }

    case OpNewPackedArray: {
      tc = DataTypeGeneric;
      break;
    }

    case OpIterInit:
    case OpIterInitK:
    case OpMIterInit:
    case OpMIterInitK:
    case OpWIterInit:
    case OpWIterInitK: {
      // We care about the type of the stack input but not the locals.
      tc = opndIdx == 0 ? DataTypeSpecific : DataTypeGeneric;
      break;
    }

    case OpIterNext:
    case OpIterNextK:
    case OpMIterNext:
    case OpMIterNextK:
    case OpWIterNext:
    case OpWIterNextK: {
      // Don't care about local input types; all we do is pass their address to
      // helpers.
      tc = DataTypeGeneric;
      break;
    }

    default: {
      break;
    }
  }

  if (hasMVector(opc) && opndIdx == getMInstrInfo(ni.mInstrOp()).valCount()) {
    tc = irgen::mInstrBaseConstraint(irgs, predictedType);
  }

  return relaxType(predictedType, tc);
}

void emitInputChecks(
  IRGS& irgs,
  const NormalizedInstruction& ni,
  bool checkOuterTypeOnly
) {
  FTRACE(4, "\n{}: {}\n", ni.offset(), opcodeToName(ni.op()));
  if (isAlwaysNop(ni.op())) return;

  for (auto i = 0; i < ni.inputs.size(); ++i) {
    FTRACE(4, "Input {}: ", i);
    auto loc = ni.inputs[i];
    if (!loc.isLocal() && !loc.isStack()) {
      FTRACE(4, "!isLocal && !isStack, skipping\n");
      continue;
    }
    auto const predictedType = irgen::predictedTypeFromLocation(irgs, loc);
    FTRACE(4, "predicted {}\n", predictedType);

    if (!(predictedType <= TGen) && !(predictedType <= TCls)) {
      FTRACE(4, "predictedType ({}) !<= TGen|TCls, skipping\n", predictedType);
      continue;
    }

    assertx(predictedType != TBottom);

    auto typeToCheck = predictedType <= TCls
      ? predictedType
      : typeToCheckForInput(irgs, ni, i, predictedType);

    // Make sure typeToCheck is checkable.
    if (!(typeToCheck <= TCell || typeToCheck <= TBoxedInitCell)) {
      FTRACE(4, "typeToCheck isn't checkable, skipping\n");
      continue;
    }

    if (typeToCheck == TCountedStr) {
      FTRACE(4, "typeToCheck is CountedStr, widening to Str\n");
      typeToCheck = TStr;
    } else if (typeToCheck <= TStaticArr) {
      FTRACE(4, "typeToCheck is StaticArr, widening to Arr\n");
      typeToCheck = TArr;
    } else if (typeToCheck.clsSpec() &&
        !(typeToCheck.clsSpec().cls()->attrs() & AttrNoOverride)) {
      FTRACE(4, "class specialization could be overridden, widening to Obj\n");
      typeToCheck = TObj;
    } else if (typeToCheck.arrSpec() && typeToCheck.arrSpec().type()) {
      FTRACE(4, "array specialization is RAT, widening to Arr\n");
      typeToCheck = TArr;
    }

    if (loc.isLocal()) {
      FTRACE(4, "Checking local {} for type {}\n", loc.offset, typeToCheck);
      irgen::checkTypeLocal(irgs, loc.offset, typeToCheck, ni.source.offset(),
        checkOuterTypeOnly);
    } else {
      FTRACE(4, "Checking stack offset {} for type {}\n",
          loc.bcRelOffset.offset, typeToCheck);
      irgen::checkTypeStack(irgs, loc.bcRelOffset, typeToCheck,
        ni.source.offset(), checkOuterTypeOnly);
    }
  }
  // Calling checkTypeStack with a Type t such that t <= BoxedCell causes us to
  // spill the stack, so we need to sync the stack with an exception stack
  // boundary here.
  irgs.irb->exceptionStackBoundary();
}

}

void translateInstr(
  IRGS& irgs,
  const NormalizedInstruction& ni,
  bool checkOuterTypeOnly,
  bool needsExitPlaceholder
) {
  irgen::prepareForNextHHBC(
    irgs,
    &ni,
    ni.source,
    ni.endsRegion && !irgen::isInlining(irgs)
  );

  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    // TODO(#5706706): want to use assertTypeLocation, but Location::Stack
    // is a little unsure of itself.
    irgen::assertTypeStack(irgs, BCSPOffset{i}, type);
  }

  if (RuntimeOption::EvalHHIRConstrictGuards) {
    emitInputChecks(irgs, ni, checkOuterTypeOnly);
  }

  FTRACE(1, "\n{:-^60}\n", folly::format("Translating {}: {} with stack:\n{}",
                                         ni.offset(), ni.toString(),
                                         show(irgs)));

  if (needsExitPlaceholder) irgen::makeExitPlaceholder(irgs);

  irgen::ringbufferEntry(irgs, Trace::RBTypeBytecodeStart, ni.source, 2);
  irgen::emitIncStat(irgs, Stats::Instr_TC, 1);
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
