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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/tv-conversions.h"

#include "hphp/runtime/vm/property-profile.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/vm/jit/arg-group.h"
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

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace NativeCalls {

///////////////////////////////////////////////////////////////////////////////

namespace {

constexpr irlower::SyncOptions SNone = irlower::SyncOptions::None;
constexpr irlower::SyncOptions SSync = irlower::SyncOptions::Sync;

constexpr DestType DSSA  = DestType::SSA;
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
using ArrCmpFn = bool (*)(const ArrayData*, const ArrayData*);
using ResCmpFn = bool (*)(const ResourceHdr*, const ResourceHdr*);
using StrIntCmpFn = bool (*)(const StringData*, int64_t);

using StrCmpFnInt = int64_t (*)(const StringData*, const StringData*);
using ObjCmpFnInt = int64_t (*)(const ObjectData*, const ObjectData*);
using ResCmpFnInt = int64_t (*)(const ResourceHdr*, const ResourceHdr*);
using StrIntCmpFnInt = int64_t (*)(const StringData*, int64_t);

}

//////////////////////////////////////////////////////////////////////

#ifdef MSVC_REQUIRE_AUTO_TEMPLATED_OVERLOAD
static auto c_AsyncFunctionWaitHandle_Create_true =
  &c_AsyncFunctionWaitHandle::Create<true>;
static auto c_AsyncFunctionWaitHandle_Create_false =
  &c_AsyncFunctionWaitHandle::Create<false>;
#endif

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
    {ConvFuncToArr,      convCellToArrHelper, DSSA, SNone,
                           {{TV, 0}}},
    {ConvVecToArr,       convVecToArrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    // These two need to sync because of Hack array compat notices
    {ConvDictToArr,      convDictToArrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvKeysetToArr,    convKeysetToArrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToArr,      convCellToArrHelper, DSSA, SSync,
                           {{TV, 0}}},
    {ConvArrToNonDVArr,  convArrToNonDVArrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    // ConvClsMethTo##T to sync due to clsmeth conversion notices
    {ConvClsMethToArr,   convClsMethToArrHealper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvClsMethToVArr,  convClsMethToVArrHealper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvClsMethToDArr,  convClsMethToDArrHealper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvArrToVec,       convArrToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvDictToVec,      convDictToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvKeysetToVec,    convKeysetToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvClsMethToVec,   convClsMethToVecHealper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToVec,       convObjToVecHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvArrToDict,      convArrToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvVecToDict,      convVecToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvKeysetToDict,   convKeysetToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvClsMethToDict,  convClsMethToDictHealper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToDict,      convObjToDictHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvArrToKeyset,    convArrToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvVecToKeyset,    convVecToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvDictToKeyset,   convDictToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvClsMethToKeyset, convClsMethToKeysetHealper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvObjToKeyset,    convObjToKeysetHelper, DSSA, SSync,
                           {{SSA, 0}}},

    {ConvCellToBool,     cellToBool, DSSA, SSync,
                           {{TV, 0}}},

    {ConvArrToDbl,       convArrToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToDbl,       convObjToDblHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvStrToDbl,       convStrToDblHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvResToDbl,       convResToDblHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvCellToDbl,      convCellToDblHelper, DSSA, SSync,
                           {{TV, 0}}},

    {ConvObjToInt,       &ObjectData::toInt64, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvStrToInt,       &StringData::toInt64, DSSA, SNone,
                           {{SSA, 0}, immed(10)}},
    {ConvResToInt,       &ResourceHdr::getId, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvCellToInt,      cellToInt, DSSA, SSync,
                           {{TV, 0}}},

    {ConvDblToStr,       convDblToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvIntToStr,       convIntToStrHelper, DSSA, SNone,
                           {{SSA, 0}}},
    {ConvObjToStr,       convObjToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvResToStr,       convResToStrHelper, DSSA, SSync,
                           {{SSA, 0}}},
    {ConvCellToStr,      cellCastToStringData, DSSA, SSync,
                           {{TV, 0}}},

    {ConcatStrStr,       concat_ss, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStrInt,       concat_si, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatIntStr,       concat_is, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ConcatStr3,         concat_s3, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}}},
    {ConcatStr4,         concat_s4, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    {AddElemIntKey,      addElemIntKeyHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {AddNewElem,         addNewElemHelper, DSSA, SSync,
                           {{SSA, 0}, {TV, 1}}},
    {DictAddElemStrKey,  dictAddElemStringKeyHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},
    {DictAddElemIntKey,  dictAddElemIntKeyHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {TV, 2}}},

    {ArrayAdd,           arrayAdd, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {Box,                boxValue, DSSA, SNone, {{TV, 0}}},
    {Clone,              &ObjectData::clone, DSSA, SSync, {{SSA, 0}}},
    {NewPair,            collections::allocPair, DSSA, SNone,
                           {{TV, 0}, {TV, 1}}},
    {FuncCred,           &FunctionCredential::newInstance, DSSA, SNone,
                           {{SSA, 0}}},
    {AllocObj,           ObjectData::newInstance<true>, DSSA, SSync,
                           {{SSA, 0}}},
    {AllocObjReified,    ObjectData::newInstanceReified<true>, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {InitProps,          &Class::initProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {InitSProps,         &Class::initSProps, DNone, SSync,
                           {{extra(&ClassData::cls)}}},
    {DebugBacktrace,     debug_backtrace_jit, DSSA, SSync, {{SSA, 0}}},
    {DebugBacktraceFast, debug_backtrace_fast, DSSA, SSync, {}},
    {InitThrowableFileAndLine,
                         throwable_init_file_and_line_from_builtin,
                           DNone, debug ? SSync : SNone, {{SSA, 0}}},
    {LdClsCtor,          loadClassCtor, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {LookupClsMethod,    lookupClsMethodHelper, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {LookupClsRDS,       lookupClsRDS, DSSA, SNone, {{SSA, 0}}},
    {PrintStr,           print_string, DNone, SSync, {{SSA, 0}}},
    {PrintInt,           print_int, DNone, SSync, {{SSA, 0}}},
    {PrintBool,          print_boolean, DNone, SSync, {{SSA, 0}}},
    {VerifyParamCls,     VerifyParamTypeSlow, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {VerifyParamRecDesc, VerifyParamRecDescImpl, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {VerifyParamCallable, VerifyParamTypeCallable, DNone, SSync,
                           {{TV, 0}, {SSA, 1}}},
    {VerifyParamFail,    VerifyParamTypeFail, DNone, SSync, {{SSA, 0}}},
    {VerifyParamFailHard,VerifyParamTypeFail, DNone, SSync, {{SSA, 0}}},
    {VerifyRetCls,       VerifyRetTypeSlow, DNone, SSync,
                           {extra(&ParamData::paramId), {SSA, 0}, {SSA, 1},
                            {SSA, 2}, {TV, 3}}},
    {VerifyRetRecDesc,   VerifyRetRecDescImpl, DNone, SSync,
                           {extra(&ParamData::paramId), {SSA, 0}, {SSA, 1},
                             {SSA, 2}, {TV, 3}}},
    {VerifyRetCallable,  VerifyRetTypeCallable, DNone, SSync,
                         {extra(&ParamData::paramId), {TV, 0}}},
    {VerifyRetFail,      VerifyRetTypeFail, DNone, SSync,
                         {extra(&ParamData::paramId), {SSA, 0}}},
    {VerifyRetFailHard,  VerifyRetTypeFail, DNone, SSync,
                         {extra(&ParamData::paramId), {SSA, 0}}},
    {RaiseUninitLoc,     raiseUndefVariable, DNone, SSync, {{SSA, 0}}},
    {RaiseError,         raise_error_sd, DNone, SSync, {{SSA, 0}}},
    {RaiseWarning,       raiseWarning, DNone, SSync, {{SSA, 0}}},
    {RaiseNotice,        raiseNotice, DNone, SSync, {{SSA, 0}}},
    {ThrowArrayIndexException,
                         throwArrayIndexException, DNone, SSync,
                         {{SSA, 0},
                          extra(&ThrowArrayIndexExceptionData::isInOut)}},
    {ThrowArrayKeyException,
                         throwArrayKeyException, DNone, SSync,
                         {{SSA, 0},
                          extra(&ThrowArrayKeyExceptionData::isInOut)}},
    {RaiseUndefProp,     raiseUndefProp, DNone, SSync,
                           {{SSA, 0}, {SSA, 1}}},
    {RaiseMissingArg,    raiseMissingArgument, DNone, SSync,
                           {extra(&FuncArgData::func),
                            extra(&FuncArgData::argNum)}},
    {RaiseTooManyArg,    raiseTooManyArguments, DNone, SSync,
                           {extra(&FuncData::func), {SSA, 0}}},
    {RaiseRxCallViolation, raiseRxCallViolation,
                          DNone, SSync, {{SSA, 0}, {SSA, 1}}},
    {ThrowInvalidOperation, throw_invalid_operation_exception,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowArithmeticError, throw_arithmetic_error,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowCallReifiedFunctionWithoutGenerics,
                          throw_call_reified_func_without_generics,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowDivisionByZeroError, throw_division_by_zero_error,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowDivisionByZeroException, throw_division_by_zero_exception,
                          DNone, SSync, {}},
    {ThrowHasThisNeedStatic, throw_has_this_need_static,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowMissingThis,   throw_missing_this,
                          DNone, SSync, {{SSA, 0}}},
    {ThrowParameterWrongType, throw_parameter_wrong_type, DNone, SSync,
                                {{TV, 0},
                                 extra(&FuncArgTypeData::func),
                                 extra(&FuncArgTypeData::argNum),
                                 extra(&FuncArgTypeData::type)}},
    {ThrowParamRefMismatch, throwParamRefMismatch, DNone, SSync,
                          {{SSA, 0}, extra(&ParamData::paramId)}},
    {ThrowParamRefMismatchRange, throwParamRefMismatchRange, DNone, SSync,
                          {{SSA, 0},
                           extra(&CheckRefsData::firstBit),
                           extra(&CheckRefsData::mask),
                           extra(&CheckRefsData::vals)}},
    {HasToString,        &ObjectData::hasToString, DSSA, SSync,
                          {{SSA, 0}}},

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
    {CmpStr,             static_cast<StrCmpFnInt>(compare), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GtStrInt,           static_cast<StrIntCmpFn>(more), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteStrInt,          static_cast<StrIntCmpFn>(moreEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtStrInt,           static_cast<StrIntCmpFn>(less), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteStrInt,          static_cast<StrIntCmpFn>(lessEqual), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqStrInt,           static_cast<StrIntCmpFn>(equal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqStrInt,          static_cast<StrIntCmpFn>(nequal), DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpStrInt,          static_cast<StrIntCmpFnInt>(compare), DSSA, SSync,
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
    {GtArr,              ArrayData::Gt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteArr,             ArrayData::Gte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtArr,              ArrayData::Lt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteArr,             ArrayData::Lte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqArr,              ArrayData::Equal, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqArr,             ArrayData::NotEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameArr,            ArrayData::Same, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameArr,           ArrayData::NotSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpArr,             ArrayData::Compare, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GtVec,              PackedArray::VecGt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {GteVec,             PackedArray::VecGte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LtVec,              PackedArray::VecLt, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {LteVec,             PackedArray::VecLte, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqVec,              PackedArray::VecEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqVec,             PackedArray::VecNotEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameVec,            PackedArray::VecSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameVec,           PackedArray::VecNotSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {CmpVec,             PackedArray::VecCmp, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqDict,             MixedArray::DictEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqDict,            MixedArray::DictNotEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameDict,           MixedArray::DictSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameDict,          MixedArray::DictNotSame, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {EqKeyset,           SetArray::Equal, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NeqKeyset,          SetArray::NotEqual, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {SameKeyset,         SetArray::Same, DSSA, SSync,
                          {{SSA, 0}, {SSA, 1}}},
    {NSameKeyset,        SetArray::NotSame, DSSA, SSync,
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
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {LdClsPropAddrOrRaise,
                         getSPropOrRaise, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},

    {ProfileProp,        &PropertyProfile::incCount, DNone, SNone,
                           {{SSA, 0}, {SSA, 1}}},

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
    {CreateGen,          &Generator::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async generator support helpers */
    {CreateAGen,         &AsyncGenerator::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},

    /* Async function support helpers */
#ifdef MSVC_REQUIRE_AUTO_TEMPLATED_OVERLOAD
    {CreateAFWH,         c_AsyncFunctionWaitHandle_Create_true, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateAFWHNoVV,     c_AsyncFunctionWaitHandle_Create_false, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
#else
    {CreateAFWH,         &c_AsyncFunctionWaitHandle::Create<true>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
    {CreateAFWHNoVV,     &c_AsyncFunctionWaitHandle::Create<false>, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}, {SSA, 4}}},
#endif
    {CreateAGWH,         &c_AsyncGeneratorWaitHandle::Create, DSSA, SNone,
                           {{SSA, 0}, {SSA, 1}, {SSA, 2}, {SSA, 3}}},
    {CreateSSWH,         &c_StaticWaitHandle::CreateSucceeded, DSSA, SNone,
                           {{TV, 0}}},
    {AFWHPrepareChild,   &c_AsyncFunctionWaitHandle::PrepareChild, DSSA, SSync,
                           {{SSA, 0}, {SSA, 1}}},

    /* MInstrTranslator helpers */
    {SetNewElemArray, MInstrHelpers::setNewElemArray, DNone, SSync,
                      {{SSA, 0}, {TV, 1}}},
    {StringGet, MInstrHelpers::stringGetI, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},

    {PairIsset, MInstrHelpers::pairIsset, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {VectorIsset, MInstrHelpers::vectorIsset, DSSA, SSync,
                  {{SSA, 0}, {SSA, 1}}},
    {ElemVecU, MInstrHelpers::elemVecIU, DSSA, SSync, {{SSA, 0}, {SSA, 1}}},
    {ThrowOutOfBounds, throwOOBException, DNone, SSync, {{TV, 0}, {TV, 1}}},
    {ThrowInvalidArrayKey, invalidArrayKeyHelper, DNone, SSync,
                 {{SSA, 0}, {TV, 1}}},

    /* instanceof checks */
    {ProfileInstanceCheck, &InstanceBits::profile, DNone, SNone, {{SSA, 0}}},
    {InstanceOfIface, &Class::ifaceofDirect, DSSA,
                      SNone, {{SSA, 0}, {SSA, 1}}},
    {InterfaceSupportsArr, IFaceSupportFn{interface_supports_array},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsVec, IFaceSupportFn{interface_supports_vec},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsDict, IFaceSupportFn{interface_supports_dict},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsKeyset, IFaceSupportFn{interface_supports_keyset},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsStr, IFaceSupportFn{interface_supports_string},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsInt, IFaceSupportFn{interface_supports_int},
                             DSSA, SNone, {{SSA, 0}}},
    {InterfaceSupportsDbl, IFaceSupportFn{interface_supports_double},
                             DSSA, SNone, {{SSA, 0}}},
    {OODeclExists, &Unit::classExists, DSSA, SSync,
                     {{SSA, 0}, {SSA, 1}, extra(&ClassKindData::kind)}},

    /* is/as expressions */
    {IsTypeStruct, isTypeStructHelper, DSSA, SSync, {{SSA, 0}, {TV, 1}}},
    {ThrowAsTypeStructException, throwAsTypeStructExceptionHelper, DNone, SSync,
                                   {{SSA, 0}, {TV, 1}}},

    /* surprise flag support */
    {SuspendHookAwaitEF, &EventHook::onFunctionSuspendAwaitEF, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookAwaitEG, &EventHook::onFunctionSuspendAwaitEG, DNone,
                            SSync, {{SSA, 0}}},
    {SuspendHookAwaitR, &EventHook::onFunctionSuspendAwaitR, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookCreateCont, &EventHook::onFunctionSuspendCreateCont, DNone,
                            SSync, {{SSA, 0}, {SSA, 1}}},
    {SuspendHookYield, &EventHook::onFunctionSuspendYield, DNone,
                            SSync, {{SSA, 0}}},
    {ReturnHook, &EventHook::onFunctionReturn, DNone,
                            SSync, {{SSA, 0}, {TV, 1}}},

    /* silence operator support */
    {ZeroErrorLevel, &zero_error_level, DSSA, SNone, {}},
    {RestoreErrorLevel, &restore_error_level, DNone, SNone, {{SSA, 0}}},

    /* count($mixed) */
    {Count, &countHelper, DSSA, SSync, {{TV, 0}}},

    /* method_exists($obj, $meth) */
    {MethodExists, methodExistsHelper, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},

    /* microtime(true) */
    {GetTime, TimeStamp::CurrentSecond, DSSA, SNone, {}},
    /* clock_gettime_ns($clk_id) */
    {GetTimeNs, folly::chrono::clock_gettime_ns, DSSA, SNone, {{SSA, 0}}},

    /* reified generics operations */
    {CheckClsReifiedGenericMismatch, checkClassReifiedGenericMismatch,
                                     DNone, SSync,
                                     {{extra(&ClassData::cls)}, {SSA, 0}}},
    {CheckFunReifiedGenericMismatch, checkFunReifiedGenericMismatch,
                                     DNone, SSync,
                                     {{extra(&FuncData::func)}, {SSA, 0}}},
    {VerifyReifiedLocalType, VerifyReifiedLocalTypeImpl, DNone, SSync,
                             {{extra(&ParamData::paramId)}, {SSA, 0}}},
    {VerifyReifiedReturnType, VerifyReifiedReturnTypeImpl, DNone, SSync,
                              {{TV, 0}, {SSA, 1}}},
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

}}
