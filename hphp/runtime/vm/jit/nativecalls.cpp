/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/nativecalls.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/targetcache.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP {  namespace JIT { namespace NativeCalls {

using namespace HPHP::Transl;
using namespace HPHP::Transl::TargetCache;

const SyncOptions SNone = SyncOptions::kNoSyncPoint;
const SyncOptions SSync = SyncOptions::kSyncPoint;
const SyncOptions SSyncAdj1 = SyncOptions::kSyncPointAdjustOne;

const DestType DSSA = DestType::SSA;
const DestType DTV = DestType::TV;
const DestType DNone = DestType::None;

/*
 * The table passed to s_callMap's constructor describes helpers calls
 * used by translated code. Each row consists of the following values:
 *
 * Opcode
 *   The opcode that uses the call
 *
 * Func
 *   A value describing the function to call:
 *     (TCA)<function pointer> - Raw function pointer
 *     {FSSA, idx} - Use a const TCA from inst->src(idx)
 *
 * Dest
 *   DSSA - The helper returns a single-register value
 *   DTV  - The helper returns a TypedValue in two registers
 *   DNone - The helper does not return a value
 *
 * SyncPoint
 *   SNone - The helper does not need a sync point
 *   SSync - The helper needs a normal sync point
 *   SSyncAdj1 - The helper needs a sync point that skips top of stack on unwind
 *
 * Args
 *   A list of tuples describing the arguments to pass to the helper
 *     {SSA, idx} - Pass the value in inst->src(idx)
 *     {TV, idx} - Pass the value in inst->src(idx) as a
 *                 TypedValue, in two registers
 *     {VecKeyS, idx} - Like TV, but Str values are passed as a raw
 *                      StringData*, in a single register
 *     {VecKeyIS, idx} - Like VecKeyS, including Int
 */
static CallMap s_callMap({
    /* Opcode, Func, Dest, SyncPoint, Args */
    {ConvBoolToArr,      (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvDblToArr,       (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvIntToArr,       (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvObjToArr,       (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvStrToArr,       (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvCellToArr,       (TCA)convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},

    {ConvArrToBool,      (TCA)convArrToBoolHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvStrToBool,      (TCA)convStrToBoolHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvCellToBool,      (TCA)convCellToBoolHelper, DSSA, SNone,
                           {{TV, 0}}},

    {ConvArrToDbl,       (TCA)convArrToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToDbl,       (TCA)convCellToDblHelper, DSSA, SSync,
                           {{TV, 0}}},
    {ConvStrToDbl,       (TCA)convStrToDblHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToDbl,       (TCA)convCellToDblHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvArrToInt,       (TCA)convArrToIntHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvDblToInt,       (TCA)convDblToIntHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToInt,       (TCA)convCellToIntHelper, DSSA, SSync,
                           {{TV, 0}}},
    {ConvStrToInt,       (TCA)convStrToIntHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvCellToInt,      (TCA)convCellToIntHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvCellToObj,      (TCA)convCellToObjHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvDblToStr,       (TCA)convDblToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvIntToStr,       (TCA)convIntToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToStr,       (TCA)convObjToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToStr,      (TCA)convCellToStrHelper, DSSA, SSync,
                           {{TV, 0}}},

    {AddElemStrKey,      (TCA)addElemStringKeyHelper, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddElemIntKey,      (TCA)addElemIntKeyHelper, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddNewElem,         (TCA)&HphpArray::AddNewElemC, DSSA, SNone,
                           {{SSA, 0}, {TV, 1}}},
    {ArrayAdd,           (TCA)array_add, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {Box,                (TCA)box_value, DSSA, SNone, {{TV, 0}}},
    {NewArray,           (TCA)new_array, DSSA, SNone, {{SSA, 0}}},
    {NewTuple,           (TCA)new_tuple, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {AllocObj,           (TCA)newInstance, DSSA, SSync,
                           {{SSA, 0}}},
    {LdClsCtor,          (TCA)loadClassCtor, DSSA, SSync,
                           {{SSA, 0}}},
    {PrintStr,           (TCA)print_string, DNone, SNone, {{SSA, 0}}},
    {PrintInt,           (TCA)print_int, DNone, SNone, {{SSA, 0}}},
    {PrintBool,          (TCA)print_boolean, DNone, SNone, {{SSA, 0}}},
    {VerifyParamCls,     (TCA)VerifyParamTypeSlow, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {VerifyParamCallable, (TCA)VerifyParamTypeCallable, DNone, SSync,
                           {{TV, 0}, {SSA, 1}}},
    {VerifyParamFail,    (TCA)VerifyParamTypeFail, DNone, SSync, {{SSA, 0}}},
    {RaiseUninitLoc,     (TCA)raiseUndefVariable, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       (TCA)raiseWarning, DNone, SSync, {{SSA, 0}}},
    {WarnNonObjProp,     (TCA)raisePropertyOnNonObject, DNone, SSync, {}},
    {ThrowNonObjProp,    (TCA)throw_null_object_prop, DNone, SSync, {}},
    {RaiseUndefProp,     (TCA)raiseUndefProp, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {RaiseError,         (TCA)raise_error_sd, DNone, SSync, {{SSA, 0}}},
    {IncStatGrouped,     (TCA)Stats::incStatGrouped, DNone, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {StaticLocInit,      (TCA)staticLocInit, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {StaticLocInitCached, (TCA)staticLocInitCached, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}, {SSA, 3}}},
    {LdFuncCached,       (TCA)FixedFuncCache::lookupUnknownFunc, DSSA, SSync,
                           {{SSA, 0}}},
    {CreateCl,           (TCA)createClHelper, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {ArrayIdx,           {FSSA, 0}, DTV, SSync,
                           {{SSA, 1}, {SSA, 2}, {TV, 3}}},

    /* Switch helpers */
    {LdSwitchDblIndex,   (TCA)switchDoubleHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchStrIndex,   (TCA)switchStringHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchObjIndex,   (TCA)switchObjHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* Continuation support helpers */
    {CreateContFunc,     (TCA)&VMExecutionContext::createContFunc, DSSA, SNone,
                         {{SSA, 0}, {SSA, 1}}},
    {CreateContMeth,     (TCA)&VMExecutionContext::createContMeth, DSSA, SNone,
                         {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* VectorTranslator helpers */
    {BaseG,    {FSSA, 0}, DSSA, SSync, {{TV, 1}, {SSA, 2}}},
    {PropX,    {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {PropDX,   {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {CGetProp, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {VGetProp, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {BindProp, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}, {SSA, 5}}},
    {SetProp,  {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {TV, 4}}},
    {UnsetProp, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {SetOpProp, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {TV, 3}, {SSA, 4}, {SSA, 5}}},
    {IncDecProp, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {EmptyProp, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {IssetProp, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ElemX,    {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {ElemDX,   {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {ElemUX,   {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {ArrayGet, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {VectorGet, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {PairGet,  {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {MapGet,   {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {StableMapGet, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {CGetElem, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {VGetElem, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {BindElem, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {SetWithRefElem, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {ArraySet, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {VectorSet, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {MapSet,   {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {StableMapSet, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ArraySetRef, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {SetElem,  {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {TV, 3}}},
    {UnsetElem, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}}},
    {SetOpElem, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {TV, 3}, {SSA, 4}}},
    {IncDecElem, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}}},
    {SetNewElem, (TCA)setNewElem, DNone, SSync, {{SSA, 0}, {TV, 1}}},
    {SetWithRefNewElem, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {BindNewElem, (TCA)bindNewElemIR, DNone, SSync,
                 {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ArrayIsset, {FSSA, 0}, DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {VectorIsset, {FSSA, 0}, DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {PairIsset, {FSSA, 0}, DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {MapIsset,  {FSSA, 0}, DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {StableMapIsset, {FSSA, 0}, DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {IssetElem, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {EmptyElem,{FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},

    /* instanceof checks */
    {InstanceOf, (TCA)instanceOfHelper, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {InstanceOfIface, (TCA)Util::getMethodPtr(&Class::ifaceofDirect), DSSA,
                 SNone, {{SSA, 0}, {SSA, 1}}},

    /* debug assert helpers */
    {DbgAssertPtr, (TCA)assertTv, DNone, SNone, {{SSA, 0}}},
});

CallMap::CallMap(CallInfoList infos) {
  for (auto const& info : infos) {
    m_map[info.op] = info;

    // Check for opcodes that have a version which modifies the stack,
    // and add an entry to the table for that one.
    if (opcodeHasFlags(info.op, HasStackVersion)) {
      Opcode stkOp = getStackModifyingOpcode(info.op);
      assert(opcodeHasFlags(stkOp, ModifiesStack));
      auto& slot = m_map[stkOp];
      slot = info;
      slot.op = stkOp;
    }
  }
}

bool CallMap::hasInfo(Opcode op) {
  return mapContains(s_callMap.m_map, op);
}

const CallInfo& CallMap::info(Opcode op) {
  auto it = s_callMap.m_map.find(op);
  assert(it != s_callMap.m_map.end());
  return it->second;
}

} } }
