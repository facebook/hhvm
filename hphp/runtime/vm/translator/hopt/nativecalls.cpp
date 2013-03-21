/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/vm/translator/hopt/nativecalls.h"

#include "runtime/vm/runtime.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-runtime.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT { namespace NativeCalls {

using namespace HPHP::VM::Transl;

static const SyncOptions SNone = kNoSyncPoint;
static const SyncOptions SSync = kSyncPoint;
static const SyncOptions SSyncAdj1 = kSyncPointAdjustOne;

static const DestType DSSA = DestType::SSA;
static const DestType DTV = DestType::TV;
static const DestType DNone = DestType::None;

/*
 * The table passed to s_callMap's constructor describes helpers calls
 * used by translated code. Each row consists of the following values:
 *
 * Opcode
 *   The opcode that uses the call
 * Func
 *   A value describing the function to call:
 *     (TCA)<function pointer> - Raw function pointer
 *     {FSSA, idx} - Use a const TCA from inst->getSrc(idx)
 * Dest
 *   DSSA - The helper returns a single-register value
 *   DTV  - The helper returns a TypedValue in two registers
 *   DNone - The helper does not return a value
 * SyncPoint
 *   SNone - The helper does not need a sync point
 *   SSync - The helper needs a normal sync point
 *   SSyncAdj1 - The helper needs a sync point that skips top of stack on unwind
 * Args
 *   A list of tuples describing the arguments to pass to the helper
 *     {SSA, idx} - Pass the value in inst->getSrc(idx)
 *     {TV, idx} - Pass the value in inst->getSrc(idx) as a
 *                 TypedValue, in two registers
 *     {VecKeyS, idx} - Like TV, but Str values are passed as a raw
 *                      StringData*, in a single register
 *     {VecKeyIS, idx} - Like VecKeyS, including Int
 */
static CallMap s_callMap({
    /* Opcode, Func, Dest, SyncPoint, Args */
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
    {PrintStr,           (TCA)print_string, DNone, SNone, {{SSA, 0}}},
    {PrintInt,           (TCA)print_int, DNone, SNone, {{SSA, 0}}},
    {PrintBool,          (TCA)print_boolean, DNone, SNone, {{SSA, 0}}},
    {RaiseUninitLoc,     (TCA)raiseUndefVariable, DNone, SSync, {{SSA, 0}}},
    {WarnNonObjProp,     (TCA)raisePropertyOnNonObject, DNone, SSync, {}},
    {ThrowNonObjProp,    (TCA)throw_null_object_prop, DNone, SSync, {}},
    {RaiseUndefProp,     (TCA)raiseUndefProp, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}}},

    /* Switch helpers */
    {LdSwitchDblIndex,   (TCA)switchDoubleHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchStrIndex,   (TCA)switchStringHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchObjIndex,   (TCA)switchObjHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* Continuation support helpers */
    {CreateCont,         {FSSA, 0}, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {FillContLocals, (TCA)&VMExecutionContext::fillContinuationVars,
              DNone, SNone,
              {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},

    /* VectorTranslator helpers */
    {BaseG,    {FSSA, 0}, DSSA, SSync, {{TV, 1}, {SSA, 2}}},
    {PropX,    {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {PropDX,   {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {CGetProp, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {VGetProp, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {BindProp, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}, {SSA, 5}}},
    {SetProp,  {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {TV, 4}}},
    {SetOpProp,{FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyS, 2}, {TV, 3}, {SSA, 4}}},
    {IncDecProp,{FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}, {SSA, 4}}},
    {EmptyProp,{FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}}},
    {IssetProp,{FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {VecKeyS, 3}}},
    {ElemX,    {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {ElemDX,   {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {ArrayGet, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {CGetElem, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {VGetElem, {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {BindElem, {FSSA, 0}, DNone, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}, {SSA, 4}}},
    {ArraySet, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ArraySetRef, {FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {SetElem,  {FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {TV, 3}}},
    {SetOpElem,{FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {TV, 3}, {SSA, 4}}},
    {IncDecElem,{FSSA, 0}, DTV, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {SetNewElem, (TCA)setNewElem, DTV, SSync, {{SSA, 0}, {TV, 1}}},
    {BindNewElem, (TCA)bindNewElemIR, DNone, SSync,
                 {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {IssetElem,{FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},
    {EmptyElem,{FSSA, 0}, DSSA, SSync,
                 {{SSA, 1}, {VecKeyIS, 2}, {SSA, 3}}},

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

const CallInfo& CallMap::getInfo(Opcode op) {
  auto it = s_callMap.m_map.find(op);
  assert(it != s_callMap.m_map.end());
  return it->second;
}

} } } }
