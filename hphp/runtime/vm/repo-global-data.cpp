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

#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-unserializer.h"

#include <folly/Format.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void RepoGlobalData::load(bool loadConstantFuncs) const {
  RO::EnableIntrinsicsExtension                 = EnableIntrinsicsExtension;
  RO::PHP7_Builtins                             = PHP7_Builtins;
  RO::PHP7_NoHexNumerics                        = PHP7_NoHexNumerics;
  RO::PHP7_Substr                               = PHP7_Substr;
  RO::EvalCheckPropTypeHints                    = CheckPropTypeHints;
  RO::EnableArgsInBacktraces                    = EnableArgsInBacktraces;
  RO::EvalAbortBuildOnVerifyError               = AbortBuildOnVerifyError;
  RO::StrictArrayFillKeys                       = StrictArrayFillKeys;
  RO::EvalIsCompatibleClsMethType               = IsCompatibleClsMethType;
  RO::EvalEmitClassPointers                     = EmitClassPointers;
  RO::EvalEmitClsMethPointers                   = EmitClsMethPointers;
  RO::EvalForbidDynamicCallsWithAttr            = ForbidDynamicCallsWithAttr;
  RO::EvalHackArrCompatIsVecDictNotices         = HackArrCompatIsVecDictNotices;
  RO::EvalRaiseClassConversionWarning           = RaiseClassConversionWarning;
  RO::EvalClassPassesClassname                  = ClassPassesClassname;
  RO::EvalClassnameNotices                      = ClassnameNotices;
  RO::EvalClassIsStringNotices                  = ClassIsStringNotices;
  RO::EvalRaiseClsMethConversionWarning         = RaiseClsMethConversionWarning;
  RO::EvalNoticeOnCoerceForStrConcat            = NoticeOnCoerceForStrConcat;
  RO::EvalNoticeOnCoerceForBitOp                = NoticeOnCoerceForBitOp;
  RO::EvalHackArrDVArrs                         = true; // TODO(kshaunak): Clean up.

  if (HardGenericsUB) RO::EvalEnforceGenericsUB = 2;

  always_assert(!RO::EvalArrayProvenance);
  always_assert(!RO::EvalLogArrayProvenance);

  if (loadConstantFuncs) {
    RO::ConstantFunctions.clear();
    for (auto const& elm : ConstantFunctions) {
      auto result = RO::ConstantFunctions.emplace(
        elm.first, make_tv<KindOfUninit>()
      );
      if (result.second) {
        tvAsVariant(result.first->second) = unserialize_from_string(
          elm.second, VariableUnserializer::Type::Internal
        );
        tvAsVariant(result.first->second).setEvalScalar();
      }
    }
  }
}

std::string show(const RepoGlobalData& gd) {
  std::string out;
#define SHOW(x) folly::format(&out, "  {}: {}\n", #x, gd.x)
  SHOW(InitialNamedEntityTableSize);
  SHOW(InitialStaticStringTableSize);
  SHOW(CheckPropTypeHints);
  SHOW(HardGenericsUB);
  SHOW(HardPrivatePropInference);
  SHOW(PHP7_NoHexNumerics);
  SHOW(PHP7_Builtins);
  SHOW(PHP7_Substr);
  SHOW(HackArrCompatNotices);
  SHOW(HackArrCompatSerializeNotices);
  SHOW(EnableIntrinsicsExtension);
  SHOW(ForbidDynamicCallsToFunc);
  SHOW(ForbidDynamicCallsToClsMeth);
  SHOW(ForbidDynamicCallsToInstMeth);
  SHOW(ForbidDynamicConstructs);
  SHOW(ForbidDynamicCallsWithAttr);
  SHOW(LogKnownMethodsAsDynamicCalls);
  SHOW(NoticeOnBuiltinDynamicCalls);
  SHOW(AbortBuildOnVerifyError);
  SHOW(EnableArgsInBacktraces);
  SHOW(Signature);
  SHOW(EmitClassPointers);
  SHOW(EmitClsMethPointers);
  SHOW(IsVecNotices);
  SHOW(IsCompatibleClsMethType);
  SHOW(RaiseClassConversionWarning);
  SHOW(ClassPassesClassname);
  SHOW(ClassnameNotices);
  SHOW(ClassIsStringNotices);
  SHOW(RaiseClsMethConversionWarning);
  SHOW(StrictArrayFillKeys);
  SHOW(NoticeOnCoerceForStrConcat);
  SHOW(NoticeOnCoerceForBitOp);
  SHOW(TypeconstInterfaceInheritanceDefaults);
#undef SHOW
  return out;
}

//////////////////////////////////////////////////////////////////////

}
