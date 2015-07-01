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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/array/ext_array.h"

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

auto constexpr SSA      = ArgType::SSA;
auto constexpr TV       = ArgType::TV;

using IFaceSupportFn = bool (*)(const StringData*);

using StrCmpFn = bool (*)(const StringData*, const StringData*);

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
 *     extra(&EDStruct::member) - extract an immediate from extra data
 *     immed(int64_t)           - constant immediate
 */
static CallMap s_callMap {
    /* Opcode, Func, Dest, SyncPoint, Args */
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

    {CoerceStrToInt,     coerceStrToIntHelper, DSSA, SSync,
                           {{SSA, 0}, extra(&FuncArgData::argNum),
                            extra(&FuncArgData::func)}},
    {CoerceStrToDbl,     coerceStrToDblHelper, DSSA, SSync,
                           {{SSA, 0}, extra(&FuncArgData::argNum),
                            extra(&FuncArgData::func)}},

    {CoerceCellToInt,    coerceCellToIntHelper, DSSA, SSync,
                           {{TV, 0}, extra(&FuncArgData::argNum),
                            extra(&FuncArgData::func)}},
    {CoerceCellToDbl,    coerceCellToDblHelper, DSSA, SSync,
                           {{TV, 0}, extra(&FuncArgData::argNum),
                            extra(&FuncArgData::func)}},
    {CoerceCellToBool,   coerceCellToBoolHelper, DSSA, SSync,
                           {{TV, 0}, extra(&FuncArgData::argNum),
                            extra(&FuncArgData::func)}},

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
    {NewLikeArray,       MixedArray::MakeReserveLike, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {AllocPackedArray,   MixedArray::MakePackedUninitialized, DSSA, SNone,
                           {{extra(&PackedArrayData::size)}}},
    {ColAddNewElemC,     colAddNewElemCHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}}},
    {MapAddElemC,        colAddElemCHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}, {TV, 2}}},
    {AllocObj,           newInstance, DSSA, SSync,
                           {{SSA, 0}}},
    {InitProps,          &Class::initProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {InitSProps,         &Class::initSProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {RegisterLiveObj,    registerLiveObj, DNone, SNone, {{SSA, 0}}},
    {LdClsCtor,          loadClassCtor, DSSA, SSync,
                           {{SSA, 0}}},
    {LookupClsRDSHandle, lookupClsRDSHandle, DSSA, SNone, {{SSA, 0}}},
    {PrintStr,           print_string, DNone, SSync, {{SSA, 0}}},
    {PrintInt,           print_int, DNone, SSync, {{SSA, 0}}},
    {PrintBool,          print_boolean, DNone, SSync, {{SSA, 0}}},
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
    {RaiseError,         raise_error_sd, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       raiseWarning, DNone, SSync, {{SSA, 0}}},
    {RaiseNotice,        raiseNotice, DNone, SSync, {{SSA, 0}}},
    {RaiseArrayIndexNotice,
                         raiseArrayIndexNotice, DNone, SSync, {{SSA, 0}}},
    {RaiseArrayKeyNotice,
                         raiseArrayKeyNotice, DNone, SSync, {{SSA, 0}}},
    {RaiseUndefProp,     raiseUndefProp, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {RaiseMissingArg,    raiseMissingArgument, DNone, SSync,
                           {extra(&FuncArgData::func),
                            extra(&FuncArgData::argNum)}},
    {IncStatGrouped,     Stats::incStatGrouped, DNone, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ClosureStaticLocInit,
                         closureStaticLocInit, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {GenericIdx,         genericIdx, DTV, SSync,
                          {{TV, 0}, {TV, 1}, {TV, 2}}},
    {MapIdx,             mapIdx, DTV, SSync,
                          {{SSA, 0}, {SSA, 1}, {TV, 2}}},

    /* Type specialized comparison operators */
    {GtStr,              static_cast<StrCmpFn>(more), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteStr,             static_cast<StrCmpFn>(moreEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtStr,              static_cast<StrCmpFn>(less), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteStr,             static_cast<StrCmpFn>(lessEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqStr,              static_cast<StrCmpFn>(equal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqStr,             static_cast<StrCmpFn>(nequal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameStr,            static_cast<StrCmpFn>(same), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameStr,           static_cast<StrCmpFn>(nsame), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},

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
    {CreateCont,         &Generator::Create<false>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async function support helpers */
    {CreateAFWH,         &c_AsyncFunctionWaitHandle::Create<true>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateAFWHNoVV,     &c_AsyncFunctionWaitHandle::Create<false>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateSSWH,         &c_StaticWaitHandle::CreateSucceeded, DSSA, SNone,
                           {{TV, 0}}},
    {AFWHPrepareChild,   &c_AsyncFunctionWaitHandle::PrepareChild, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {ABCUnblock,         &AsioBlockableChain::Unblock, DSSA, SNone,
                           {{SSA, 0}}},

    /* MInstrTranslator helpers */
    {SetOpElem, setOpElem, DTV, SSync,
                 {{SSA, 0}, {TV, 1}, {TV, 2}, {SSA, 3},
                  extra(&SetOpData::op)}},
    {IncDecElem, incDecElem, DTV, SSync,
                 {{SSA, 0}, {TV, 1}, {SSA, 2},
                  extra(&IncDecData::op)}},
    {SetNewElem, setNewElem, DNone, SSync, {{SSA, 0}, {TV, 1}}},
    {SetNewElemArray, setNewElemArray, DNone, SSync, {{SSA, 0}, {TV, 1}}},
    {BindNewElem, bindNewElemIR, DNone, SSync,
                 {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {StringGet, MInstrHelpers::stringGetI, DSSA, SSync,
                 {{SSA, 0}, {SSA, 1}}},
    {PairIsset, MInstrHelpers::pairIsset, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {VectorIsset, MInstrHelpers::vectorIsset, DSSA, SSync,
                  {{SSA, 0}, {SSA, 1}}},
    {BindElem, MInstrHelpers::bindElemC, DNone, SSync,
                 {{SSA, 0}, {TV, 1}, {SSA, 2}, {SSA, 3}}},
    {SetWithRefElem, MInstrHelpers::setWithRefElemC, DNone, SSync,
                 {{SSA, 0}, {TV, 1}, {TV, 2}, {SSA, 3}}},
    {SetWithRefNewElem, MInstrHelpers::setWithRefNewElem, DNone, SSync,
                 {{SSA, 0}, {TV, 1}, {SSA, 2}}},
    {ThrowOutOfBounds, throwOOB, DNone, SSync, {{SSA, 0}}},

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
    {SuspendHookE, &EventHook::onFunctionSuspendE, DNone, SSync,
                            {{SSA, 0}, {SSA, 1}}},
    {SuspendHookR, &EventHook::onFunctionSuspendR, DNone, SSync,
                            {{SSA, 0}, {SSA, 1}}},
    {ReturnHook,  &EventHook::onFunctionReturn, DNone, SSync,
                            {{SSA, 0}, {TV, 1}}},

    /* silence operator support */
    {ZeroErrorLevel, &zero_error_level, DSSA, SNone, {}},
    {RestoreErrorLevel, &restore_error_level, DNone, SNone, {{SSA, 0}}},

    // count($mixed)
    {Count, &countHelper, DSSA, SSync, {{TV, 0}}},

    {GetMemoKey, getMemoKeyHelper, DTV, SSync, {{TV, 0}}},
};

CallMap::CallMap(CallInfoList infos) {
  for (auto const& info : infos) {
    m_map[info.op] = info;
  }
}

bool CallMap::hasInfo(Opcode op) {
  return s_callMap.m_map.count(op) != 0;
}

const CallInfo& CallMap::info(Opcode op) {
  auto it = s_callMap.m_map.find(op);
  assertx(it != s_callMap.m_map.end());
  return it->second;
}

} // NativeCalls

using namespace NativeCalls;
ArgGroup toArgGroup(const CallInfo& info,
                    const StateVector<SSATmp,Vloc>& locs,
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

} }
