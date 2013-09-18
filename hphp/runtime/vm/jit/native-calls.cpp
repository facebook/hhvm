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

#include "hphp/runtime/vm/jit/native-calls.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/runtime-type-profiler.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP {  namespace JIT { namespace NativeCalls {

using namespace HPHP::Transl;
using namespace HPHP::Transl::TargetCache;

namespace {

constexpr SyncOptions SNone = SyncOptions::kNoSyncPoint;
constexpr SyncOptions SSync = SyncOptions::kSyncPoint;
constexpr SyncOptions SSyncAdj1 = SyncOptions::kSyncPointAdjustOne;

constexpr DestType DSSA = DestType::SSA;
constexpr DestType DTV = DestType::TV;
constexpr DestType DNone = DestType::None;

template<class EDType, class MemberType>
Arg extra(MemberType EDType::*ptr) {
  auto fun = [ptr] (IRInstruction* inst) {
    auto const extra = inst->extra<EDType>();
    return constToBits(extra->*ptr);
  };
  return Arg(fun);
}

Arg immed(intptr_t imm) { return Arg(ArgType::Imm, imm); }

FuncPtr fssa(uint64_t i) { return FuncPtr { FuncType::SSA, i }; }

template<class Ret, class T, class... Args>
FuncPtr method(Ret (T::*fp)(Args...) const) {
  return FuncPtr(reinterpret_cast<TCA>(getMethodPtr(fp)));
}

template<class Ret, class T, class... Args>
FuncPtr method(Ret (T::*fp)(Args...)) {
  return FuncPtr(reinterpret_cast<TCA>(getMethodPtr(fp)));
}

auto constexpr SSA      = ArgType::SSA;
auto constexpr TV       = ArgType::TV;
auto constexpr MemberKeyS  = ArgType::MemberKeyS;
auto constexpr MemberKeyIS = ArgType::MemberKeyIS;

}

//////////////////////////////////////////////////////////////////////

/*
 * The table passed to s_callMap's constructor describes helpers calls
 * used by translated code. Each row consists of the following values:
 *
 * Opcode
 *   The opcode that uses the call
 *
 * Func
 *   A value describing the function to call:
 *     <function pointer>          - Raw function pointer
 *     method(<pointer to member>) - Dispatch to a C++ member function---the
 *                                   function must be non-virtual.
 *     fssa(idx)                   - Use a const TCA from inst->src(idx)
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
 *     {SSA, idx}               - Pass the value in inst->src(idx)
 *     {TV, idx}                - Pass the value in inst->src(idx) as a
 *                                TypedValue, in two registers
 *     {MemberKeyS, idx}           - Like TV, but Str values are passed as a raw
 *                                StringData*, in a single register
 *     {MemberKeyIS, idx}          - Like MemberKeyS, including Int
 *     extra(&EDStruct::member) - extract an immediate from extra data
 *     immed(int64_t)           - constant immediate
 */
static CallMap s_callMap {
    /* Opcode, Func, Dest, SyncPoint, Args */
    {TypeProfileFunc,    profileOneArgument, DNone, SNone,
                           {{TV,0}, extra(&TypeProfileData::param),
                                    extra(&TypeProfileData::func)}},
    {ConvBoolToArr,      convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvDblToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvIntToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvObjToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvStrToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvCellToArr,      convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},

    {ConvArrToBool,      convArrToBoolHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvStrToBool,      method(&StringData::toBoolean), DSSA, SNone,
                           {{SSA, 0}}},
    {ConvCellToBool,     cellToBool, DSSA, SNone,
                           {{TV, 0}}},

    {ConvArrToDbl,       convArrToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToDbl,       convCellToDblHelper, DSSA, SSync,
                           {{TV, 0}}},
    {ConvStrToDbl,       convStrToDblHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToDbl,       convCellToDblHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvArrToInt,       convArrToIntHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToInt,       cellToInt, DSSA, SSync,
                           {{TV, 0}}},
    {ConvStrToInt,       method(&StringData::toInt64), DSSA, SNone,
                           {{SSA, 0}, immed(10)}},
    {ConvCellToInt,      cellToInt, DSSA, SSync,
                           {{TV, 0}}},

    {ConvCellToObj,      convCellToObjHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvDblToStr,       convDblToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvIntToStr,       convIntToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToStr,       convObjToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvResToStr,       convResToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToStr,      convCellToStrHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConcatStrStr,       concat_ss, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {ConcatStrInt,       concat_si, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {ConcatIntStr,       concat_is, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},

    {AddElemStrKey,      addElemStringKeyHelper, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddElemIntKey,      addElemIntKeyHelper, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddNewElem,         &HphpArray::AddNewElemC, DSSA, SNone,
                           {{SSA, 0}, {TV, 1}}},
    {ArrayAdd,           array_add, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {Box,                box_value, DSSA, SNone, {{TV, 0}}},
    {NewArray,           HphpArray::MakeReserve, DSSA, SNone, {{SSA, 0}}},
    {NewPackedArray,     HphpArray::MakePacked, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {AllocObj,           newInstance, DSSA, SSync,
                           {{SSA, 0}}},
    {LdClsCtor,          loadClassCtor, DSSA, SSync,
                           {{SSA, 0}}},
    {PrintStr,           print_string, DNone, SNone, {{SSA, 0}}},
    {PrintInt,           print_int, DNone, SNone, {{SSA, 0}}},
    {PrintBool,          print_boolean, DNone, SNone, {{SSA, 0}}},
    {VerifyParamCls,     VerifyParamTypeSlow, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {VerifyParamCallable, VerifyParamTypeCallable, DNone, SSync,
                           {{TV, 0}, {SSA, 1}}},
    {VerifyParamFail,    VerifyParamTypeFail, DNone, SSync, {{SSA, 0}}},
    {RaiseUninitLoc,     raiseUndefVariable, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       raiseWarning, DNone, SSync, {{SSA, 0}}},
    {WarnNonObjProp,     raisePropertyOnNonObject, DNone, SSync, {}},
    {ThrowNonObjProp,    throw_null_object_prop, DNone, SSync, {}},
    {RaiseUndefProp,     raiseUndefProp, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {RaiseError,         raise_error_sd, DNone, SSync, {{SSA, 0}}},
    {IncStatGrouped,     Stats::incStatGrouped, DNone, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ClosureStaticLocInit,
                         closureStaticLocInit, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {LdFuncCached,       FixedFuncCache::lookupUnknownFunc, DSSA, SSync,
                           {{SSA, 0}}},
    {LdFuncCachedU,      FixedFuncCache::lookupUnknownFunc, DSSA, SSync,
                           {{SSA, 0}}},
    {ArrayIdx,           fssa(0), DTV, SSync,
                           {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {LdGblAddrDef,       ldGblAddrDefHelper, DSSA, SNone,
                           {{SSA, 0}}},

    /* Switch helpers */
    {LdSwitchDblIndex,   switchDoubleHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchStrIndex,   switchStringHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchObjIndex,   switchObjHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* Continuation support helpers */
    {CreateContFunc,     &VMExecutionContext::createContFunc, DSSA, SNone,
                          { extra(&CreateContData::origFunc),
                            extra(&CreateContData::genFunc) }},
    {CreateContMeth,     &VMExecutionContext::createContMeth, DSSA, SNone,
                          { extra(&CreateContData::origFunc),
                            extra(&CreateContData::genFunc),
                            {SSA, 0} }},

    /* MInstrTranslator helpers */
    {BaseG,    fssa(0), DSSA, SSync, {{TV, 1}, {SSA, 2}}},
    {PropX,    fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {PropDX,   fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {CGetProp, fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {MemberKeyS, 3}, {SSA, 4}}},
    {VGetProp, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {MemberKeyS, 3}, {SSA, 4}}},
    {BindProp, fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}, {SSA, 5}}},
    {SetProp,  fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {TV, 4}}},
    {UnsetProp, fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {SetOpProp, fssa(0), DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {TV, 3}, {SSA, 4}, {SSA, 5}}},
    {IncDecProp, fssa(0), DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {EmptyProp, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {IssetProp, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ElemX,    fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {ElemArray, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {ElemDX,   fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {ElemUX,   fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {ArrayGet, fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {StringGet, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {VectorGet, fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {PairGet,  fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {MapGet,   fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {StableMapGet, fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}}},
    {CGetElem, fssa(0), DTV, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {VGetElem, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {BindElem, fssa(0), DNone, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {SetWithRefElem, fssa(0), DNone, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {ArraySet, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {VectorSet, fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {MapSet,   fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {StableMapSet, fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ArraySetRef, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {SetElem,  fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {TV, 3}}},
    {UnsetElem, fssa(0), DNone, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}}},
    {SetOpElem, fssa(0), DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {TV, 3}, {SSA, 4}}},
    {IncDecElem, fssa(0), DTV, SSync,
                 {{SSA, 1}, {TV, 2}, {SSA, 3}}},
    {SetNewElem, setNewElem, DNone, SSync, {{SSA, 0}, {TV, 1}}},
    {SetNewElemArray, setNewElemArray, DNone, SSync, {{SSA, 0}, {TV, 1}}},
    {SetWithRefNewElem, fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {BindNewElem, bindNewElemIR, DNone, SSync,
                 {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ArrayIsset, fssa(0), DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {VectorIsset, fssa(0), DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {PairIsset, fssa(0), DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {MapIsset,  fssa(0), DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {StableMapIsset, fssa(0), DSSA, SSync, {{SSA, 1}, {SSA, 2}}},
    {IssetElem, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {EmptyElem, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},

    /* instanceof checks */
    {InstanceOf, instanceOfHelper, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {InstanceOfIface, method(&Class::ifaceofDirect), DSSA,
                      SNone, {{SSA, 0}, {SSA, 1}}},

    /* debug assert helpers */
    {DbgAssertPtr, assertTv, DNone, SNone, {{SSA, 0}}},
};

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
