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

#include "hphp/runtime/vm/jit/native-calls.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/runtime-type-profiler.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/static_wait_handle.h"
#include "hphp/runtime/ext/ext_array.h"

namespace HPHP { namespace jit { namespace NativeCalls {

namespace {

constexpr SyncOptions SNone = SyncOptions::kNoSyncPoint;
constexpr SyncOptions SSync = SyncOptions::kSyncPoint;

constexpr DestType DSSA  = DestType::SSA;
constexpr DestType DTV   = DestType::TV;
constexpr DestType DNone = DestType::None;

template<class EDType, class MemberType>
Arg extra(MemberType EDType::*ptr) {
  auto fun = [ptr] (const IRInstruction* inst) {
    auto const extra = inst->extra<EDType>();
    return Type::cns(extra->*ptr).rawVal();
  };
  return Arg(fun);
}

Arg immed(intptr_t imm) { return Arg(ArgType::Imm, imm); }

FuncPtr fssa(uint64_t i) { return FuncPtr { FuncType::SSA, i }; }

auto constexpr SSA      = ArgType::SSA;
auto constexpr TV       = ArgType::TV;
auto constexpr MemberKeyS  = ArgType::MemberKeyS;
auto constexpr MemberKeyIS = ArgType::MemberKeyIS;

using IFaceSupportFn = bool (*)(const StringData*);

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
 *     <pointer to member>         - Dispatch to a C++ member function---the
 *                                   function must be non-virtual.
 *     fssa(idx)                   - Use a const TCA from inst->src(idx)
 *
 * Dest
 *   DSSA  - The helper returns a single-register value
 *   DTV   - The helper returns a TypedValue in two registers
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
                           {{TV,0}, extra(&TypeProfileData::param), {SSA, 1}}},
    {ConvBoolToArr,      convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvDblToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvIntToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvObjToArr,       convCellToArrHelper, DSSA, SSync,
                           {{TV, 0}}},
    {ConvStrToArr,       convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvCellToArr,      convCellToArrHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvStrToBool,      &StringData::toBoolean, DSSA, SNone,
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
    {ConvStrToInt,       &StringData::toInt64, DSSA, SNone,
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

    {ConcatStrStr,       concat_ss, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStrInt,       concat_si, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatIntStr,       concat_is, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStr3,         concat_s3, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ConcatStr4,         concat_s4, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    {AddElemStrKey,      addElemStringKeyHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddElemIntKey,      addElemIntKeyHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddNewElem,         addNewElemHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}}},
    {ArrayAdd,           arrayAdd, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {Box,                boxValue, DSSA, SNone, {{TV, 0}}},
    {Clone,              &ObjectData::clone, DSSA, SSync, {{SSA, 0}}},
    {NewArray,           MixedArray::MakeReserve, DSSA, SNone, {{SSA, 0}}},
    {NewMixedArray,      MixedArray::MakeReserveMixed, DSSA, SNone, {{SSA, 0}}},
    {NewVArray,         MixedArray::MakeReserveVArray, DSSA, SNone, {{SSA, 0}}},
    {NewMIArray,        MixedArray::MakeReserveIntMap, DSSA, SNone, {{SSA, 0}}},
    {NewMSArray,        MixedArray::MakeReserveStrMap, DSSA, SNone, {{SSA, 0}}},
    {NewLikeArray,       MixedArray::MakeReserveLike, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {NewPackedArray,     MixedArray::MakePacked, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {NewCol,             newColHelper, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ColAddNewElemC,     colAddNewElemCHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}}},
    {ColAddElemC,        colAddElemCHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}, {TV, 2}}},
    {AllocObj,           newInstance, DSSA, SSync,
                           {{SSA, 0}}},
    {CustomInstanceInit, &ObjectData::callCustomInstanceInit,
                           DSSA, SSync, {{SSA, 0}}},
    {InitProps,          &Class::initProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {InitSProps,         &Class::initSProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {RegisterLiveObj,    registerLiveObj, DNone, SNone, {{SSA, 0}}},
    {LdClsCtor,          loadClassCtor, DSSA, SSync,
                           {{SSA, 0}}},
    {LookupClsRDSHandle, lookupClsRDSHandle, DSSA, SNone, {{SSA, 0}}},
    {LookupClsMethod,    lookupClsMethodHelper, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {LdArrFuncCtx,       loadArrayFunctionContext, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdArrFPushCuf,      fpushCufHelperArray, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdStrFPushCuf,      fpushCufHelperString, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {PrintStr,           print_string, DNone, SNone, {{SSA, 0}}},
    {PrintInt,           print_int, DNone, SNone, {{SSA, 0}}},
    {PrintBool,          print_boolean, DNone, SNone, {{SSA, 0}}},
    {VerifyParamCls,     VerifyParamTypeSlow, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {VerifyParamCallable, VerifyParamTypeCallable, DNone, SSync,
                           {{TV, 0}, {SSA, 1}}},
    {VerifyParamFail,    VerifyParamTypeFail, DNone, SSync, {{SSA, 0}}},
    {VerifyRetCls,       VerifyRetTypeSlow, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {TV, 3}}},
    {VerifyRetCallable,  VerifyRetTypeCallable, DNone, SSync, {{TV, 0}}},
    {VerifyRetFail,      VerifyRetTypeFail, DNone, SSync, {{TV, 0}}},
    {RaiseUninitLoc,     raiseUndefVariable, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       raiseWarning, DNone, SSync, {{SSA, 0}}},
    {RaiseNotice,        raiseNotice, DNone, SSync, {{SSA, 0}}},
    {RaiseArrayIndexNotice,
                         raiseArrayIndexNotice, DNone, SSync, {{SSA, 0}}},
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
    {ArrayIdx,           fssa(0), DTV, SSync,
                           {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {GenericIdx,         genericIdx, DTV, SSync,
                          {{TV, 0}, {TV, 1}, {TV, 2}}},

    /* Static prop helpers */
    {LdClsPropAddrOrNull,
                         getSPropOrNull, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdClsPropAddrOrRaise,
                         getSPropOrRaise, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* Global helpers */
    {LdGblAddrDef,       ldGblAddrDefHelper, DSSA, SNone,
                           {{SSA, 0}}},

    /* Switch helpers */
    {LdSwitchDblIndex,   switchDoubleHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchStrIndex,   switchStringHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {LdSwitchObjIndex,   switchObjHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},

    /* Generator support helpers */
    {CreateCont,         &c_Generator::Create<false>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async function support helpers */
    {CreateAFWH,         &c_AsyncFunctionWaitHandle::Create, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateSSWH,         &c_StaticWaitHandle::CreateSucceededVM, DSSA, SNone,
                           {{TV, 0}}},
    {AFWHPrepareChild,   &c_AsyncFunctionWaitHandle::PrepareChild, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {ABCUnblock,         &AsioBlockableChain::Unblock, DSSA, SNone,
                           {{SSA, 0}}},

    /* MInstrTranslator helpers */
    {BaseG,    fssa(0), DSSA, SSync, {{TV, 1}}},
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
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {TV, 4}, {SSA, 5}, {SSA, 6}}},
    {IncDecProp, fssa(0), DTV, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}, {SSA, 5}}},
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
    {MapGet,   fssa(0), DTV, SSync,
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
    {MapSet,   fssa(0), DNone, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}}},
    {ArraySetRef, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {SSA, 2}, {TV, 3}, {SSA, 4}}},
    {SetElem,  fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {TV, 3}}},
    {UnsetElem, fssa(0), DNone, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}}},
    {SetOpElem, setOpElem, DTV, SSync,
                 {{SSA, 0}, {TV, 1}, {TV, 2}, {SSA, 3}, {SSA, 4}}},
    {IncDecElem, incDecElem, DTV, SSync,
                 {{SSA, 0}, {TV, 1}, {SSA, 2}, {SSA, 3}}},
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
    {IssetElem, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},
    {EmptyElem, fssa(0), DSSA, SSync,
                 {{SSA, 1}, {MemberKeyIS, 2}, {SSA, 3}}},

    /* instanceof checks */
    {InstanceOf, &Class::classof, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {InstanceOfIface, &Class::ifaceofDirect, DSSA,
                      SNone, {{SSA, 0}, {SSA, 1}}},
    {InterfaceSupportsArr, IFaceSupportFn{interface_supports_array},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsStr, IFaceSupportFn{interface_supports_string},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsInt, IFaceSupportFn{interface_supports_int},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsDbl, IFaceSupportFn{interface_supports_double},
                             DSSA, SNone, {{SSA, 0}}},
    {OODeclExists, &Unit::classExists, DSSA, SSync,
                     {{SSA, 0}, {SSA, 1}, extra(&ClassKindData::kind)}},

    /* debug assert helpers */
    {DbgAssertPtr, assertTv, DNone, SNone, {{SSA, 0}}},

    /* surprise flag support */
    {SurpriseHook,        &EventHook::CheckSurprise, DNone, SSync, {}},
    {FunctionSuspendHook, &EventHook::onFunctionSuspend, DNone, SSync,
                            {{SSA, 0}, {SSA, 1}}},
    {FunctionReturnHook,  &EventHook::onFunctionReturnJit, DNone, SSync,
                            {{SSA, 0}, {TV, 1}}},

    /* silence operator support */
    {ZeroErrorLevel, &zero_error_level, DSSA, SNone, {}},
    {RestoreErrorLevel, &restore_error_level, DNone, SNone, {{SSA, 0}}},

    // count($mixed)
    {Count, &countHelper, DSSA, SSync, {{TV, 0}}},

    // count($array)
    {CountArray, &ArrayData::size, DSSA, SNone, {{SSA, 0}}},
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
  return s_callMap.m_map.count(op) != 0;
}

const CallInfo& CallMap::info(Opcode op) {
  auto it = s_callMap.m_map.find(op);
  assert(it != s_callMap.m_map.end());
  return it->second;
}

} // NativeCalls

namespace x64 {
using namespace NativeCalls;
ArgGroup toArgGroup(const CallInfo& info, const jit::vector<Vloc>& locs,
                    const IRInstruction* inst) {
  ArgGroup argGroup{inst, locs};
  for (auto const& arg : info.args) {
    switch (arg.type) {
    case ArgType::SSA:
      argGroup.ssa(arg.ival);
      break;
    case ArgType::TV:
      argGroup.typedValue(arg.ival);
      break;
    case ArgType::MemberKeyS:
      argGroup.memberKeyS(arg.ival);
      break;
    case ArgType::MemberKeyIS:
      argGroup.memberKeyIS(arg.ival);
      break;
    case ArgType::ExtraImm:
      argGroup.imm(arg.extraFunc(inst));
      break;
    case ArgType::Imm:
      argGroup.imm(arg.ival);
      break;
    }
  }
  return argGroup;
}
} // X64
namespace arm {
using namespace NativeCalls;
ArgGroup toArgGroup(const CallInfo& info, const RegAllocInfo& regs,
                    const IRInstruction* inst) {
  ArgGroup argGroup{inst, regs[inst]};
  for (auto const& arg : info.args) {
    switch (arg.type) {
    case ArgType::SSA:
      argGroup.ssa(arg.ival);
      break;
    case ArgType::TV:
      argGroup.typedValue(arg.ival);
      break;
    case ArgType::MemberKeyS:
      argGroup.memberKeyS(arg.ival);
      break;
    case ArgType::MemberKeyIS:
      argGroup.memberKeyIS(arg.ival);
      break;
    case ArgType::ExtraImm:
      argGroup.imm(arg.extraFunc(inst));
      break;
    case ArgType::Imm:
      argGroup.imm(arg.ival);
      break;
    }
  }
  return argGroup;
}
} // ARM

} }
