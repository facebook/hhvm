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
#include "hphp/util/configs/eval.h"
#include "hphp/util/configs/php7.h"
#include "hphp/util/logger.h"

#include <fmt/core.h>
#include <fmt/ranges.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void RepoGlobalData::load(bool loadConstantFuncs) const {
  Cfg::LoadFromGlobalData(*this);

  RO::EnableArgsInBacktraces                       = EnableArgsInBacktraces;
  RO::EvalAbortBuildOnVerifyError                  = AbortBuildOnVerifyError;
  RO::EvalCoeffectEnforcementLevels                = EvalCoeffectEnforcementLevels;

  if (!Cfg::Eval::BuildMayNoticeOnMethCallerHelperIsObject) {
    Cfg::Eval::NoticeOnMethCallerHelperIsObject = false;
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
#define SHOW(x) fmt::format_to(std::back_inserter(out), "  {}: {}\n", #x, gd.x);
  SHOW(AbortBuildOnVerifyError)
  SHOW(EnableArgsInBacktraces)
  SHOW(Signature)
#undef SHOW

#define C(_, Name, ...) fmt::format_to(std::back_inserter(out), "  {}: {}\n", #Name, gd.Name);
  CONFIGS_FOR_REPOGLOBALDATA()
#undef C

  fmt::format_to(
    std::back_inserter(out),
    "  SourceRootForFileBC: {}\n",
    gd.SourceRootForFileBC.value_or("*")
  );
  return out;
}

//////////////////////////////////////////////////////////////////////

}
