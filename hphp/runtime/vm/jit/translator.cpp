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
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/bc-pattern.h"

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
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

int locPhysicalOffset(Location l, const Func* f) {
  f = f ? f : liveFunc();
  assert_not_implemented(l.space == Location::Stack ||
                         l.space == Location::Local ||
                         l.space == Location::Iter);
  int localsToSkip = l.space == Location::Iter ? f->numLocals() : 0;
  int iterInflator = l.space == Location::Iter ? kNumIterCells : 1;
  return -((l.offset + 1) * iterInflator + localsToSkip);
}

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           const Class* ctx, const Class*& baseClass,
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
  auto const lookup = baseClass->getDeclPropIndex(ctx, name);
  auto const idx = lookup.prop;

  // If we couldn't find a property that is accessible in the current context,
  // bail out
  if (idx == kInvalidSlot || !lookup.accessible) return PropInfo();

  // If it's a declared property we're good to go: even if a subclass redefines
  // an accessible property with the same name it's guaranteed to be at the same
  // offset.
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
  { OpClsCnsD,     {None,             Stack1,       OutUnknown,           1 }},
  { OpFile,        {None,             Stack1,       OutString,         1 }},
  { OpDir,         {None,             Stack1,       OutString,         1 }},
  { OpNameA,       {Stack1,           Stack1,       OutString,         0 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  { OpConcatN,     {StackN,           Stack1,       OutString,         0 }},
  /* Arithmetic ops */
  { OpAbs,         {Stack1,           Stack1,       OutUnknown,        0 }},
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
  { OpSqrt,        {Stack1,           Stack1,       OutUnknown,        0 }},
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
static void getInputsImpl(SrcKey startSk,
                          NormalizedInstruction* ni,
                          int& currentStackOffset,
                          InputInfoVec& inputs) {
#ifdef USE_TRACE
  auto sk = ni->source;
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

InputInfoVec getInputs(SrcKey startSk, NormalizedInstruction& inst) {
  InputInfoVec infos;
  // MCGenerator expected top of stack to be index -1, with indexes growing
  // down from there. hhir defines top of stack to be index 0, with indexes
  // growing up from there. To compensate we start with a stack offset of 1 and
  // negate the index of any stack input after the call to getInputs.
  int stackOff = 1;
  getInputsImpl(startSk, &inst, stackOff, infos);
  for (auto& info : infos) {
    if (info.loc.isStack()) info.loc.offset = -info.loc.offset;
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
  case Op::Abs:
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
  case Op::CIterFree:
  case Op::CastArray:
  case Op::CastDouble:
  case Op::CastInt:
  case Op::CastObject:
  case Op::CastString:
  case Op::Ceil:
  case Op::CheckProp:
  case Op::CheckThis:
  case Op::Clone:
  case Op::Cns:
  case Op::CnsE:
  case Op::CnsU:
  case Op::ColAddElemC:
  case Op::ColAddNewElemC:
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
  case Op::Floor:
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
  case Op::NewVArray:
  case Op::NewMIArray:
  case Op::NewMSArray:
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
  case Op::Sqrt:
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

bool funcByNameDestroysLocals(const StringData* fname) {
  if (!fname) return false;
  return fname->isame(s_extract.get()) ||
         fname->isame(s_extractNative.get()) ||
         fname->isame(s_parse_str.get()) ||
         fname->isame(s_parse_strNative.get()) ||
         fname->isame(s_assert.get()) ||
         fname->isame(s_assertNative.get());
}

bool builtinFuncDestroysLocals(const Func* callee) {
  assert(callee && callee->isCPPBuiltin());
  auto const fname = callee->name();
  return funcByNameDestroysLocals(fname);
}

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  // We don't handle these two cases, because we don't compile functions
  // containing them:
  assert(caller->lookupVarId(s_php_errormsg.get()) == -1);
  assert(caller->lookupVarId(s_http_response_header.get()) == -1);

  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);
    return funcByNameDestroysLocals(str);
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

/*
 * Create a map from RegionDesc::BlockId -> IR Block* for all region blocks.
 */
static void createBlockMap(HTS& hts,
                           const RegionDesc& region,
                           BlockIdToIRBlockMap& blockIdToIRBlock) {
  auto& irb = *hts.irb;
  blockIdToIRBlock.clear();
  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    auto rBlock = blocks[i];
    auto id = rBlock->id();
    DEBUG_ONLY Offset bcOff = rBlock->start().offset();
    assert(IMPLIES(i == 0, bcOff == irb.unit().bcOff()));

    // NB: This maps the region entry block to a new IR block, even though
    // we've already constructed an IR entry block. We'll make the IR entry
    // block jump to this block.
    Block* iBlock = irb.unit().defBlock();

    blockIdToIRBlock[id] = iBlock;
    FTRACE(1,
           "createBlockMaps: RegionBlock {} => IRBlock {} (BC offset = {})\n",
           id, iBlock->id(), bcOff);
  }
}

/*
 * Set IRBuilder's Block associated to blockId's block according to
 * the mapping in blockIdToIRBlock.
 */
static void setIRBlock(HTS& hts,
                       RegionDesc::BlockId blockId,
                       const RegionDesc& region,
                       const BlockIdToIRBlockMap& blockIdToIRBlock) {
  auto& irb = *hts.irb;
  auto rBlock = region.block(blockId);
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
static void setSuccIRBlocks(HTS& hts,
                            const RegionDesc& region,
                            RegionDesc::BlockId srcBlockId,
                            const BlockIdToIRBlockMap& blockIdToIRBlock) {
  FTRACE(3, "setSuccIRBlocks: srcBlockId = {}\n", srcBlockId);
  auto& irb = *hts.irb;
  irb.resetOffsetMapping();
  for (auto dstBlockId : region.succs(srcBlockId)) {
    setIRBlock(hts, dstBlockId, region, blockIdToIRBlock);
  }
}

/*
 * Check if `i' is an FPush{Func,ClsMethod}D followed by an FCall{,D} to a
 * function with a singleton pattern, and if so, inline it.  Returns true if
 * this succeeds, else false.
 */
static bool tryTranslateSingletonInline(HTS& hts,
                                        const NormalizedInstruction& i,
                                        const Func* funcd) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  if (!funcd) return false;

  // Make sure we have an acceptable FPush and non-null callee.
  assert(i.op() == Op::FPushFuncD ||
         i.op() == Op::FPushClsMethodD);

  auto fcall = i.nextSk();

  // Check if the next instruction is an acceptable FCall.
  if ((fcall.op() != Op::FCall && fcall.op() != Op::FCallD) ||
      funcd->isResumable() || funcd->isReturnRef()) {
    return false;
  }

  // First, check for the static local singleton pattern...

  // Lambda to check if CGetL and StaticLocInit refer to the same local.
  auto has_same_local = [] (PC pc, const Captures& captures) {
    if (captures.size() == 0) return false;

    auto cgetl = (const Op*)pc;
    auto sli = (const Op*)captures[0];

    assert(*cgetl == Op::CGetL);
    assert(*sli == Op::StaticLocInit);

    return (getImm(sli, 0).u_IVA == getImm(cgetl, 0).u_IVA);
  };

  auto cgetl = Atom(Op::CGetL).onlyif(has_same_local);
  auto retc  = Atom(Op::RetC);

  // Look for a static local singleton pattern.
  auto result = BCPattern {
    Atom(Op::Null),
    Atom(Op::StaticLocInit).capture(),
    Atom(Op::IsTypeL),
    Atom::alt(
      Atom(Op::JmpZ).taken({cgetl, retc}),
      Atom::seq(Atom(Op::JmpNZ), cgetl, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      irgen::inlSingletonSLoc(
        hts,
        funcd,
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sloc] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  // Not found; check for the static property pattern.

  // Factory for String atoms that are required to match another captured
  // String opcode.
  auto same_string_as = [&] (int i) {
    return Atom(Op::String).onlyif([=] (PC pc, const Captures& captures) {
      auto string1 = (const Op*)pc;
      auto string2 = (const Op*)captures[i];
      assert(*string1 == Op::String);
      assert(*string2 == Op::String);

      auto const unit = funcd->unit();
      auto sd1 = unit->lookupLitstrId(getImmPtr(string1, 0)->u_SA);
      auto sd2 = unit->lookupLitstrId(getImmPtr(string2, 0)->u_SA);

      return (sd1 && sd1 == sd2);
    });
  };

  auto stringProp = same_string_as(0);
  auto stringCls  = same_string_as(1);
  auto agetc = Atom(Op::AGetC);
  auto cgets = Atom(Op::CGetS);

  // Look for a class static singleton pattern.
  result = BCPattern {
    Atom(Op::String).capture(),
    Atom(Op::String).capture(),
    Atom(Op::AGetC),
    Atom(Op::CGetS),
    Atom(Op::IsTypeC),
    Atom::alt(
      Atom(Op::JmpZ).taken({stringProp, stringCls, agetc, cgets, retc}),
      Atom::seq(Atom(Op::JmpNZ), stringProp, stringCls, agetc, cgets, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      irgen::inlSingletonSProp(
        hts,
        funcd,
        (const Op*)result.getCapture(1),
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sprop] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  return false;
}

/*
 * Returns whether offset is a control-flow merge within region.
 */
static bool isMergePoint(Offset offset, const RegionDesc& region) {
  for (auto const block : region.blocks()) {
    auto const bid = block->id();
    if (block->start().offset() == offset) {
      auto inCount = region.preds(bid).size();
      // NB: The entry block is a merge point if it has one predecessor.
      if (block == region.entry()) ++inCount;
      if (inCount >= 2) return true;
    }
  }
  return false;
}

static bool blockIsLoopHeader(
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

static void translateDispatch(HTS& hts,
                              const NormalizedInstruction& ni) {
#define O(nm, imms, ...) case Op::nm: irgen::emit##nm(hts imms); return;
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

static Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return Type::Cell;  // TODO(#3029148) this could be InitCell
    case UV: return Type::Uninit;
    case VV: return Type::BoxedCell;
    case AV: return Type::Cls;
    case RV: case FV: case CVV: case CVUV: return Type::Gen;
  }
  not_reached();
}

void translateInstr(HTS& hts, const NormalizedInstruction& ni) {
  /*
   * These generate AssertStks, which define new StkPtrs, and we don't allow IR
   * lowering functions to gen instructions that may throw after defining a new
   * StkPtrs.  This means right now they must be before we prepare for the next
   * HHBC opcode (it's fine that they will have a marker on the previous
   * instruction).
   *
   * TODO(#4810319): this will go away once we stop threading StkPtrs around.
   */
  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    if (type != Type::Gen) {
      // TODO(#5706706): want to use assertTypeLocation, but Location::Stack
      // is a little unsure of itself.
      irgen::assertTypeStack(hts, i, type);
    }
  }

  irgen::prepareForNextHHBC(
    hts,
    &ni,
    ni.source.offset(),
    ni.endsRegion && !irgen::isInlining(hts)
  );
  FTRACE(1, "\n{:-^60}\n", folly::format("Translating {}: {} with stack:\n{}",
                                         ni.offset(), ni.toString(),
                                         show(hts)));

  irgen::ringbuffer(hts, Trace::RBTypeBytecodeStart, ni.source, 2);
  irgen::emitIncStat(hts, Stats::Instr_TC, 1);

  if (RuntimeOption::EvalHHIRGenerateAsserts >= 2) {
    hts.irb->gen(DbgAssertRetAddr);
  }

  if (isAlwaysNop(ni.op())) {
    // Do nothing
  } else if (ni.interp || RuntimeOption::EvalJitAlwaysInterpOne) {
    irgen::interpOne(hts, ni);
  } else {
    translateDispatch(hts, ni);
  }
}

//////////////////////////////////////////////////////////////////////

TranslateResult translateRegion(HTS& hts,
                                const RegionDesc& region,
                                RegionBlacklist& toInterp,
                                TransFlags trflags) {
  const Timer translateRegionTimer(Timer::translateRegion);
  FTRACE(1, "translateRegion starting with:\n{}\n", show(region));

  std::string errorMsg;
  always_assert_log(check(region, errorMsg),
                    [&] { return errorMsg + "\n" + show(region); });

  auto& irb = *hts.irb;

  auto const startSk = region.start();

  BlockIdToIRBlockMap blockIdToIRBlock;
  if (RuntimeOption::EvalHHIRBytecodeControlFlow) {
    hts.mode = IRGenMode::CFG;
    createBlockMap(hts, region, blockIdToIRBlock);

    // Make the IR entry block jump to the IR block we mapped the region entry
    // block to (they are not the same!).

    auto const entry = irb.unit().entry();
    irb.startBlock(entry, entry->front().marker(), false /* isLoopHeader */);

    auto const irBlock = blockIdToIRBlock[region.entry()->id()];
    always_assert(irBlock != entry);

    irb.gen(Jmp, irBlock);
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
    hts.profTransID = profTransId;

    bool isLoopHeader = false;

    if (hts.mode == IRGenMode::CFG) {
      Block* irBlock = blockIdToIRBlock[blockId];
      isLoopHeader = blockIsLoopHeader(region, blockId, processedBlocks);
      always_assert(IMPLIES(isLoopHeader, RuntimeOption::EvalJitLoops));

      BCMarker marker(sk, block->initialSpOffset(), profTransId);
      if (!irb.startBlock(irBlock, marker, isLoopHeader)) {
        FTRACE(1, "translateRegion: block {} is unreachable, skipping\n",
               blockId);
        processedBlocks.insert(blockId);
        continue;
      }
      setSuccIRBlocks(hts, region, blockId, blockIdToIRBlock);
    }

    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      irgen::prepareForNextHHBC(hts, nullptr, sk.offset(), false);

      // Emit prediction guards. If this is the first instruction in the
      // region, and the region's entry block is not a loop header, the guards
      // will go to a retranslate request. Otherwise, they'll go to a side
      // exit.
      auto const isEntry = block == region.entry();
      auto const useGuards = (isEntry && !isLoopHeader && i == 0);
      if (useGuards) irgen::ringbuffer(hts, Trace::RBTypeTraceletGuards, sk);

      // Emit type guards.
      while (typePreds.hasNext(sk)) {
        auto const& pred = typePreds.next();
        auto type = pred.type;
        auto loc  = pred.location;
        if (type <= Type::Cls) {
          // Do not generate guards for class; instead assert the type.
          assert(loc.tag() == RegionDesc::Location::Tag::Stack);
          irgen::assertTypeLocation(hts, loc, type);
        } else if (useGuards) {
          bool checkOuterTypeOnly = mcg->tx().mode() != TransKind::Profile;
          irgen::guardTypeLocation(hts, loc, type, checkOuterTypeOnly);
        } else {
          irgen::checkTypeLocation(hts, loc, type, sk.offset());
        }
      }

      while (refPreds.hasNext(sk)) {
        auto const& pred = refPreds.next();
        if (useGuards) {
          irgen::guardRefs(hts, pred.arSpOffset, pred.mask, pred.vals);
        } else {
          irgen::checkRefs(hts, pred.arSpOffset, pred.mask, pred.vals,
            sk.offset());
        }
      }

      // Finish emitting guards, and emit profiling counters.
      if (useGuards) {
        irb.gen(EndGuards);
        if (RuntimeOption::EvalJitTransCounters) {
          irgen::incTransCounter(hts);
        }

        if (mcg->tx().mode() == TransKind::Profile) {
          if (block->func()->isEntry(block->start().offset())) {
            irgen::checkCold(hts, mcg->tx().profData()->curTransID());
          } else {
            irgen::incProfCounter(hts, mcg->tx().profData()->curTransID());
          }
        }
        irgen::ringbuffer(hts, Trace::RBTypeTraceletBody, sk);
      }

      // In the entry block, hhbc-translator gets a chance to emit some code
      // immediately after the initial guards/checks on the first instruction.
      if (isEntry && i == 0) {
        switch (arch()) {
        case Arch::X64:
          irgen::prepareEntry(hts);
          break;
        case Arch::ARM:
          // Don't do this for ARM, because it can lead to interpOne on the
          // first SrcKey in a translation, which isn't allowed.
          break;
        }
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
        if (hts.mode == IRGenMode::Trace &&
            instrIsNonCallControlFlow(inst.op()) &&
            b < blocks.size() - 1) {
          inst.nextOffset = blocks[b+1]->start().offset();
        }
      }

      // We can get a more precise output type for interpOne if we know all of
      // its inputs, so we still populate the rest of the instruction even if
      // this is true.
      inst.interp = toInterp.count(ProfSrcKey{profTransId, sk});

      auto const inputInfos = getInputs(startSk, inst);

      // Populate the NormalizedInstruction's input vector, using types from
      // HhbcTranslator.
      std::vector<DynLocation> dynLocs;
      dynLocs.reserve(inputInfos.size());
      auto newDynLoc = [&] (const InputInfo& ii) {
        dynLocs.emplace_back(
          ii.loc,
          irgen::predictedTypeFromLocation(hts, ii.loc)
        );
        FTRACE(2, "predictedTypeFromLocation: {} -> {}\n",
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
               show(hts));
        auto returnSk = inst.nextSk();
        auto returnFuncOff = returnSk.offset() - block->func()->base();
        irgen::beginInlining(hts, inst.imm[0].u_IVA, callee, returnFuncOff);
        // "Fallthrough" into the callee's first block
        irgen::endBlock(hts, blocks[b + 1]->start().offset(), inst.nextIsMerge);
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
            return tryTranslateSingletonInline(hts, inst, topFunc);
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
        if (!skipTrans) translateInstr(hts, inst);
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk{profTransId, sk};
        always_assert_log(
          !toInterp.count(psk),
          [&] {
            std::ostringstream oss;
            oss << folly::format("IR generation failed with {}\n", exn.what());
            print(oss, hts.unit);
            return oss.str();
          });
        toInterp.insert(psk);
        return TranslateResult::Retry;
      }

      skipTrans = false;

      // In CFG mode, insert a fallthrough jump at the end of each block.
      if (hts.mode == IRGenMode::CFG && i == block->length() - 1) {
        if (instrAllowsFallThru(inst.op())) {
          auto nextOffset = inst.offset() + instrLen((Op*)(inst.pc()));
          // prepareForSideExit is done later in Trace mode, but it
          // needs to happen here or else we generate the SpillStack
          // after the fallthrough jump, which is just weird.
          if (b < blocks.size() - 1 && region.isSideExitingBlock(blockId)) {
            irgen::prepareForSideExit(hts);
          }
          irgen::endBlock(hts, nextOffset, inst.nextIsMerge);
        } else if (b < blocks.size() - 1 &&
                   (isRet(inst.op()) || inst.op() == OpNativeImpl)) {
          // "Fallthrough" from inlined return to the next block
          irgen::endBlock(hts, blocks[b + 1]->start().offset(),
                          inst.nextIsMerge);
        }
        if (region.isExit(blockId)) {
          irgen::endRegion(hts);
        }
      }
    }

    if (hts.mode == IRGenMode::Trace) {
      if (b < blocks.size() - 1 && region.isSideExitingBlock(blockId)) {
        irgen::prepareForSideExit(hts);
      }
    }

    processedBlocks.insert(blockId);

    assert(!typePreds.hasNext());
    assert(!byRefs.hasNext());
    assert(!refPreds.hasNext());
    assert(!knownFuncs.hasNext());
  }

  if (hts.mode == IRGenMode::Trace) irgen::endRegion(hts);

  irGenTimer.end();

  try {
    mcg->traceCodeGen(hts);
    if (mcg->tx().mode() == TransKind::Profile) {
      mcg->tx().profData()->setProfiling(startSk.func()->getFuncId());
    }
  } catch (const FailedCodeGen& exn) {
    SrcKey sk{exn.vmFunc, exn.bcOff, exn.resumed};
    ProfSrcKey psk{exn.profTransId, sk};
    always_assert_log(
      !toInterp.count(psk),
      [&] {
        std::ostringstream oss;
        oss << folly::format("code generation failed with {}\n", exn.what());
        print(oss, hts.irb->unit());
        return oss.str();
      });
    toInterp.insert(psk);
    return TranslateResult::Retry;
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      assert(mcg->tx().useAHot());
      mcg->tx().setUseAHot(false);
      // We can't return Retry here because the code block selection
      // will still say hot.
      return TranslateResult::Failure;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
  return TranslateResult::Success;
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
                                  const Class* ctx) {
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
