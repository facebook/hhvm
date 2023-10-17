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

#include "hphp/runtime/vm/jit/native-calls.h"

#include <folly/ClockGettimeWrappers.h>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/property-profile.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/assertions.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

namespace NativeCalls {

///////////////////////////////////////////////////////////////////////////////

namespace {

constexpr irlower::SyncOptions SNone = irlower::SyncOptions::None;
constexpr irlower::SyncOptions SSync = irlower::SyncOptions::Sync;

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
using ObjCmpFn = bool (*)(const ObjectData*, const ObjectData*);
using ResCmpFn = bool (*)(const ResourceHdr*, const ResourceHdr*);

using StrCmpFnInt = int64_t (*)(const StringData*, const StringData*);
using ObjCmpFnInt = int64_t (*)(const ObjectData*, const ObjectData*);
using ResCmpFnInt = int64_t (*)(const ResourceHdr*, const ResourceHdr*);

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
    {ConvArrLikeToVec,   convArrLikeToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToVec,       convObjToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvArrLikeToDict,  convArrLikeToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToDict,      convObjToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvArrLikeToKeyset, convArrLikeToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToKeyset,     convObjToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvTVToBool,     tvToBool, DSSA, SSync,
                           {{TV, 0}}},

    {ConvObjToDbl,       convObjToDblHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvStrToDbl,       convStrToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvResToDbl,       convResToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvTVToDbl,      convTVToDblHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvObjToInt,       &ObjectData::toInt64, DSSA, SSync, {{SSA, 0}}},
    {ConvStrToInt,       &StringData::toInt64, DSSA, SNone,
                           {{SSA, 0}, immed(10)}},
    {ConvResToInt,       &ResourceHdr::getId, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvTVToInt,        tvToInt, DSSA, SSync, {{TV, 0}}},

    {ConvDblToStr,       convDblToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvIntToStr,       convIntToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToStr,       convObjToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvTVToStr,        static_cast<StringData* (*)(
                             TypedValue,
                             const ConvNoticeLevel,
                             const StringData*)>(tvCastToStringData), DSSA, SSync,
                           {{TV, 0}, extra(&ConvNoticeData::level),
                            extra(&ConvNoticeData::reasonIntVal)}},

    {ConcatStrStr,       concat_ss, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStrInt,       concat_si, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatIntStr,       concat_is, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStr3,         concat_s3, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ConcatStr4,         concat_s4, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {Clone,              &ObjectData::clone, DSSA, SSync, {{SSA, 0}}},
    {NewRFunc,           RFuncData::newInstance, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}}},
    {NewRClsMeth,        RClsMethData::create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {NewPair,            collections::allocPair, DSSA, SNone,
                           {{TV, 0}, {TV, 1}}},
    {FuncCred,           &FunctionCredential::newInstance, DSSA, SNone,
                           {{SSA, 0}}},
    {AllocObj,           ObjectData::newInstance<true>, DSSA, SSync,
                           {{SSA, 0}}},
    {InitProps,          &Class::initProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {InitSProps,         &Class::initSProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {DebugBacktrace,     debug_backtrace_jit, DSSA, SSync, {{SSA, 0}}},
    {InitThrowableFileAndLine,
                         throwable_init_file_and_line_from_builtin,
                           DNone, debug ? SSync : SNone, {{SSA, 0}}},
    {LdClsCtor,          loadClassCtor, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {LookupClsMethod,    lookupClsMethodHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {LookupClsCns,       lookupClsCns, DTV, SSync, {{SSA, 0}, {SSA, 1}}},
    {LookupClsCtxCns,    lookupClsCtxCns, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {PrintStr,           print_string, DNone, SSync, {{SSA, 0}}},
    {PrintInt,           print_int, DNone, SSync, {{SSA, 0}}},
    {PrintBool,          print_boolean, DNone, SSync, {{SSA, 0}}},
    {VerifyParamCls,     VerifyParamTypeCls, DNone, SSync,
                           {{SSA, 0}, {SSA, 2},
                            extra(&FuncParamWithTCData::func),
                            extra(&FuncParamWithTCData::paramId),
                            extra(&FuncParamWithTCData::tcAsInt)}},
    {VerifyParamCallable, VerifyParamTypeCallable, DNone, SSync,
                           {{TV, 0},
                            extra(&FuncParamData::func),
                            extra(&FuncParamData::paramId)}},
    {VerifyRetCls,       VerifyRetTypeCls, DNone, SSync,
                           {{SSA, 0}, {SSA, 2},
                            extra(&FuncParamWithTCData::func),
                            extra(&FuncParamWithTCData::paramId),
                            extra(&FuncParamWithTCData::tcAsInt)}},
    {VerifyRetCallable,  VerifyRetTypeCallable, DNone, SSync,
                           {{TV, 0},
                            extra(&FuncParamData::func),
                            extra(&FuncParamData::paramId)}},
    {ThrowUninitLoc,     throwUndefVariable, DNone, SSync, {{SSA, 0}}},
    {RaiseError,         raise_error_sd, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       raiseWarning, DNone, SSync, {{SSA, 0}}},
    {RaiseNotice,        raiseNotice, DNone, SSync, 
                         {extra(&SampleRateData::sampleRate), {SSA, 0}}},
    {ThrowCannotModifyReadonlyCollection,
                         throw_cannot_modify_readonly_collection, DNone, SSync,
                         {}},
    {ThrowMustBeEnclosedInReadonly,
                         throwMustBeEnclosedInReadonlyException, DNone, SSync,
                         {extra(&ClassData::cls), {SSA, 0}}},
    {ThrowMustBeReadonlyException,
                         throwMustBeReadonlyException, DNone, SSync,
                         {extra(&ClassData::cls), {SSA, 0}}},
    {ThrowMustBeMutableException,
                         throwMustBeMutableException, DNone, SSync,
                         {extra(&ClassData::cls), {SSA, 0}}},
    {ThrowMustBeValueTypeException,
                         throwMustBeValueTypeException, DNone, SSync,
                         {extra(&ClassData::cls), {SSA, 0}}},
    {ThrowLocalMustBeValueTypeException,
                         throwOrWarnLocalMustBeValueTypeException, DNone, SSync,
                         {{SSA, 0}}},
    {ThrowArrayKeyException,
                         throwArrayKeyException, DNone, SSync,
                         {{SSA, 0}, {SSA, 1}}},
    {ThrowUndefPropException,
                         throwUndefPropException, DNone, SSync,
                         {{SSA, 0}, {SSA, 1}}},
    {RaiseTooManyArg,    raiseTooManyArgumentsPrologue, DNone, SSync,
                           {extra(&FuncData::func), {SSA, 0}}},
    {RaiseCoeffectsFunParamTypeViolation, raiseCoeffectsFunParamTypeViolation,
                          DNone, SSync, {{TV, 0}, extra(&ParamData::paramId)}},
    {RaiseCoeffectsFunParamCoeffectRulesViolation,
                          raiseCoeffectsFunParamCoeffectRulesViolation,
                          DNone, SSync, {{SSA, 0}}},
    {RaiseImplicitContextStateInvalid,
                          raiseImplicitContextStateInvalidDispatch,
                          DNone, SSync, {extra(&FuncData::func)}},
    {ThrowInvalidOperation, throw_invalid_operation_exception,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowCallReifiedFunctionWithoutGenerics,
                          throw_call_reified_func_without_generics,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowDivisionByZeroException, throw_division_by_zero_exception,
                          DNone, SSync, {}},
    {ThrowHasThisNeedStatic, throw_has_this_need_static,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowMissingArg,    throwMissingArgument, DNone, SSync,
                           {extra(&FuncArgData::func),
                            extra(&FuncArgData::argNum)}},
    {ThrowMissingThis,   throw_missing_this,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowParameterWrongType, throw_parameter_wrong_type, DNone, SSync,
                                {{TV, 0},
                                 extra(&FuncArgTypeData::func),
                                 extra(&FuncArgTypeData::argNum),
                                 extra(&FuncArgTypeData::type)}},
    {CheckInOutMismatch, checkInOutMismatch, DNone, SSync,
                          {{SSA, 0},
                           extra(&BoolVecArgsData::numArgs),
                           extra(&BoolVecArgsData::args)}},
    {ThrowInOutMismatch, throwParamInOutMismatch, DNone, SSync,
                          {{SSA, 0},
                           extra(&ParamData::paramId)}},
    {CheckReadonlyMismatch, checkReadonlyMismatch, DNone, SSync,
                          {{SSA, 0},
                           extra(&BoolVecArgsData::numArgs),
                           extra(&BoolVecArgsData::args)}},
    {ThrowReadonlyMismatch, throwReadonlyMismatch, DNone, SSync,
                          {{SSA, 0},
                           extra(&ParamData::paramId)}},
    {HasToString,        &ObjectData::hasToString, DSSA, SNone,
                          {{SSA, 0}}},

    /* Type specialized comparison operators */
    {GtStr,              static_cast<StrCmpFn>(more), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {GteStr,             static_cast<StrCmpFn>(moreEqual), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {LtStr,              static_cast<StrCmpFn>(less), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {LteStr,             static_cast<StrCmpFn>(lessEqual), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {EqStr,              static_cast<StrCmpFn>(equal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqStr,             static_cast<StrCmpFn>(nequal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameStr,            static_cast<StrCmpFn>(same), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameStr,           static_cast<StrCmpFn>(nsame), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpStr,             static_cast<StrCmpFnInt>(compare), DSSA, SNone,
                          {{SSA, 0}, {SSA, 1}}},
    {GtObj,              static_cast<ObjCmpFn>(more), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteObj,             static_cast<ObjCmpFn>(moreEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtObj,              static_cast<ObjCmpFn>(less), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteObj,             static_cast<ObjCmpFn>(lessEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqObj,              static_cast<ObjCmpFn>(equal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqObj,             static_cast<ObjCmpFn>(nequal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpObj,             static_cast<ObjCmpFnInt>(compare), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GtArrLike,          ArrayData::Gt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteArrLike,         ArrayData::Gte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtArrLike,          ArrayData::Lt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteArrLike,         ArrayData::Lte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqArrLike,          ArrayData::Equal, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqArrLike,         ArrayData::NotEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameArrLike,        ArrayData::Same, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameArrLike,       ArrayData::NotSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpArrLike,         ArrayData::Compare, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GtRes,              static_cast<ResCmpFn>(more), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteRes,             static_cast<ResCmpFn>(moreEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtRes,              static_cast<ResCmpFn>(less), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteRes,             static_cast<ResCmpFn>(lessEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpRes,             static_cast<ResCmpFnInt>(compare), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},

    /* Static prop helpers */
    {LdClsPropAddrOrNull,
                         getSPropOrNull, DSSA, SSync,
                           {{extra(&ReadonlyData::op)}, {SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {LdClsPropAddrOrRaise,
                         getSPropOrRaise, DSSA, SSync,
                           {{extra(&ReadonlyData::op)}, {SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},

    {ProfileProp,        &PropertyProfile::incCount, DNone, SNone,
                           {{SSA, 0}, {SSA, 1}}},

    {IncCallCounter,     FuncOrder::incCount, DNone, SNone, {{extra(&FuncData::func)}, {SSA, 0}}},

    /* Global helpers */
    {LdGblAddrDef,       ldGblAddrDefHelper, DSSA, SSync,
                           {{SSA, 0}}},

    /* Generator support helpers */
    {CreateGen,          &Generator::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async generator support helpers */
    {CreateAGen,         &AsyncGenerator::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async function support helpers */
    {CreateAFWH,         &c_AsyncFunctionWaitHandle::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateAGWH,         &c_AsyncGeneratorWaitHandle::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {AFWHPrepareChild,   &c_AsyncFunctionWaitHandle::PrepareChild, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},

    /* SetNewElem helpers */
    {SetNewElemDict, setNewElemDict, DSSA, SSync, {{SSA, 0}, {TV, 1}}},
    {SetNewElemVec,  setNewElemVec, DSSA, SSync, {{SSA, 0}, {TV, 1}}},
    {SetNewElem,     setNewElem, DSSA, SSync, {{SSA, 0}, {TV, 1}}},

    /* AddNewElem helpers */
    {AddNewElemKeyset,   addNewElemKeyset, DSSA, SSync, {{SSA, 0}, {TV, 1}}},
    {AddNewElemVec,      addNewElemVec, DSSA, SNone, {{SSA, 0}, {TV, 1}}},

    /* MInstrTranslator helpers */
    {StringGet, MInstrHelpers::stringGetI, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},

    {PairIsset, MInstrHelpers::pairIsset, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
    {VectorIsset, MInstrHelpers::vectorIsset, DSSA, SNone,
                  {{SSA, 0}, {SSA, 1}}},
    {ThrowOutOfBounds, throwOOBException, DNone, SSync, {{TV, 0}, {TV, 1}}},
    {ThrowInvalidArrayKey, invalidArrayKeyHelper, DNone, SSync,
                 {{SSA, 0}, {TV, 1}}},

    /* instanceof checks */
    {ProfileInstanceCheck, &InstanceBits::profile, DNone, SNone, {{SSA, 0}}},
    {InstanceOfIface, &Class::ifaceofDirect, DSSA,
                      SNone, {{SSA, 0}, {SSA, 1}}},
    {InterfaceSupportsArrLike, IFaceSupportFn{interface_supports_arrlike},
                                 DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsStr, IFaceSupportFn{interface_supports_string},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsInt, IFaceSupportFn{interface_supports_int},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsDbl, IFaceSupportFn{interface_supports_double},
                             DSSA, SNone, {{SSA, 0}}},
    {OODeclExists, &Class::exists, DSSA, SSync,
                     {{SSA, 0}, {SSA, 1}, extra(&ClassKindData::kind)}},

    /* is/as expressions */
    {IsTypeStruct, isTypeStructHelper, DSSA, SSync,
                    {{SSA, 0}, {TV, 1}, {extra(&RDSHandleData::handle)}}},
    {IsTypeStructShallow, isTypeStructShallowHelper, DSSA, SSync,
                    {{SSA, 0}, {TV, 1}, {extra(&RDSHandleData::handle)}}},
    {ThrowAsTypeStructException, throwAsTypeStructExceptionHelper, DNone, SSync,
                                   {{SSA, 0}, {TV, 1}}},

    /* surprise flag support */
    {SuspendHookAwaitEF, &EventHook::onFunctionSuspendAwaitEFJit, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookAwaitEG, &EventHook::onFunctionSuspendAwaitEGJit, DNone,
                            SSync, {{SSA, 0}}},
    {SuspendHookAwaitR, &EventHook::onFunctionSuspendAwaitRJit, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookCreateCont, &EventHook::onFunctionSuspendCreateContJit, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookYield, &EventHook::onFunctionSuspendYieldJit, DNone,
                            SSync, {{SSA, 0}}},
    {ReturnHook, &EventHook::onFunctionReturnJit, DNone,
                            SSync, {{SSA, 0}, {TV, 1}}},

    /* silence operator support */
    {ZeroErrorLevel, &zero_error_level, DSSA, SNone, {}},
    {RestoreErrorLevel, &restore_error_level, DNone, SNone, {{SSA, 0}}},

    /* count($mixed) */
    {Count, &countHelper, DSSA, SSync, {{TV, 0}}},

    /* microtime(true) */
    {GetTime, TimeStamp::CurrentSecond, DSSA, SNone, {}},
    /* clock_gettime_ns($clk_id) */
    {GetTimeNs, folly::chrono::clock_gettime_ns, DSSA, SNone, {{SSA, 0}}},

    /* ClsMethDataRef */
    {CheckClsMethFunc, checkClsMethFuncHelper, DNone, SSync, {{SSA, 0}}},

    /* reified generics operations */
    {CheckClsReifiedGenericMismatch, checkClassReifiedGenericMismatch,
                                     DNone, SSync,
                                     {{SSA, 0}, {SSA, 1}}},
    {CheckFunReifiedGenericMismatch, checkFunReifiedGenericMismatch,
                                     DNone, SSync,
                                     {{SSA, 0}, {SSA, 1}}},
    {CheckClsRGSoft, checkClassReifiedGenericsSoft,
                                     DNone, SSync,
                                     {{SSA, 0}}},
    {GetClsRGProp, getClsReifiedGenericsProp,
                                DSSA, SSync,
                                {{SSA, 0}, {SSA, 1}}},
    {VerifyReifiedLocalType, VerifyReifiedLocalTypeImpl, DNone, SSync,
                               {{TV, 0}, {SSA, 1}, {SSA, 2},
                                extra(&FuncParamData::func),
                                extra(&FuncParamData::paramId)}},
    {VerifyReifiedReturnType, VerifyReifiedReturnTypeImpl, DNone, SSync,
                                {{TV, 0}, {SSA, 1}, {SSA, 2},
                                 extra(&FuncData::func)}},
    {RecordReifiedGenericsAndGetTSList, recordReifiedGenericsAndGetTSList,
                                        DSSA, SSync, {{SSA, 0}}},
    {RaiseErrorOnInvalidIsAsExpressionType,
      errorOnIsAsExpressionInvalidTypesHelper, DSSA, SSync, {{SSA, 0}}},
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

///////////////////////////////////////////////////////////////////////////////

} // NativeCalls

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

}
