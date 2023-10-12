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
#include "hphp/util/logger.h"

#include <folly/Format.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void RepoGlobalData::load(bool loadConstantFuncs) const {
  RO::EnableIntrinsicsExtension                    = EnableIntrinsicsExtension;
  RO::PHP7_Builtins                                = PHP7_Builtins;
  RO::PHP7_NoHexNumerics                           = PHP7_NoHexNumerics;
  RO::PHP7_Substr                                  = PHP7_Substr;
  RO::EvalCheckPropTypeHints                       = CheckPropTypeHints;
  RO::EnableArgsInBacktraces                       = EnableArgsInBacktraces;
  RO::EvalAbortBuildOnVerifyError                  = AbortBuildOnVerifyError;
  RO::StrictArrayFillKeys                          = StrictArrayFillKeys;
  RO::EvalEmitClsMethPointers                      = EmitClsMethPointers;
  RO::EvalForbidDynamicCallsWithAttr               = ForbidDynamicCallsWithAttr;
  RO::EvalRaiseClassConversionNoticeSampleRate     = RaiseClassConversionNoticeSampleRate;
  RO::EvalClassPassesClassname                     = ClassPassesClassname;
  RO::EvalClassnameNoticesSampleRate               = ClassnameNoticesSampleRate;
  RO::EvalClassStringHintNoticesSampleRate         = ClassStringHintNoticesSampleRate;
  RO::EvalClassIsStringNotices                     = ClassIsStringNotices;
  RO::EvalTraitConstantInterfaceBehavior           = TraitConstantInterfaceBehavior;
  RO::EvalBuildMayNoticeOnMethCallerHelperIsObject =
    BuildMayNoticeOnMethCallerHelperIsObject;
  RO::EvalDiamondTraitMethods                      = DiamondTraitMethods;
  RO::EvalCoeffectEnforcementLevels                = EvalCoeffectEnforcementLevels;
  RO::EvalEmitBespokeTypeStructures                = EmitBespokeTypeStructures;
  RO::EvalActiveDeployment                         = ActiveDeployment;
  RO::EvalModuleLevelTraits                        = ModuleLevelTraits;
  RO::EvalTreatCaseTypesAsMixed                    = TreatCaseTypesAsMixed;
  RO::EvalJitEnableRenameFunction                  = JitEnableRenameFunction;
  RO::RenamableFunctions                           = RenamableFunctions;
  RO::NonInterceptableFunctions                    = NonInterceptableFunctions;

  if (HardGenericsUB) RO::EvalEnforceGenericsUB = 2;

  if (!RO::EvalBuildMayNoticeOnMethCallerHelperIsObject) {
    RO::EvalNoticeOnMethCallerHelperIsObject = false;
  }

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
  SHOW(InitialTypeTableSize);
  SHOW(InitialFuncTableSize);
  SHOW(InitialStaticStringTableSize);
  SHOW(CheckPropTypeHints);
  SHOW(HardGenericsUB);
  SHOW(PHP7_NoHexNumerics);
  SHOW(PHP7_Builtins);
  SHOW(PHP7_Substr);
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
  SHOW(EmitClsMethPointers);
  SHOW(IsVecNotices);
  SHOW(RaiseClassConversionNoticeSampleRate);
  SHOW(ClassPassesClassname);
  SHOW(ClassnameNoticesSampleRate);
  SHOW(ClassStringHintNoticesSampleRate);
  SHOW(ClassIsStringNotices);
  SHOW(StrictArrayFillKeys);
  SHOW(TraitConstantInterfaceBehavior);
  SHOW(BuildMayNoticeOnMethCallerHelperIsObject);
  SHOW(DiamondTraitMethods);
  SHOW(EmitBespokeTypeStructures);
  SHOW(ModuleLevelTraits);
  SHOW(TreatCaseTypesAsMixed);
  SHOW(JitEnableRenameFunction);
#undef SHOW
  folly::format(
    &out, "  SourceRootForFileBC: {}\n",
    gd.SourceRootForFileBC.value_or("*")
  );
  return out;
}

//////////////////////////////////////////////////////////////////////

}
