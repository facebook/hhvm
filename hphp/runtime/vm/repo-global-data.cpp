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

#include <folly/Format.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::string show(const Repo::GlobalData& gd) {
  std::string out;
#define SHOW(x) folly::format(&out, "  {}: {}\n", #x, gd.x)
  SHOW(UsedHHBBC);
  SHOW(EnableHipHopSyntax);
  SHOW(InitialNamedEntityTableSize);
  SHOW(InitialStaticStringTableSize);
  SHOW(HardTypeHints);
  SHOW(HardReturnTypeHints);
  SHOW(CheckPropTypeHints);
  SHOW(ThisTypeHintLevel);
  SHOW(HardPrivatePropInference);
  out += "  DisallowDynamicVarEnvFuncs: ";
  switch (gd.DisallowDynamicVarEnvFuncs) {
    case HackStrictOption::OFF:  out += "OFF\n";  break;
    case HackStrictOption::WARN: out += "WARN\n"; break;
    case HackStrictOption::ON:   out += "ON\n";   break;
  }
  SHOW(ElideAutoloadInvokes);
  SHOW(PHP7_IntSemantics);
  SHOW(PHP7_NoHexNumerics);
  SHOW(PHP7_ScalarTypes);
  SHOW(PHP7_Builtins);
  SHOW(PHP7_Substr);
  SHOW(PromoteEmptyObject);
  SHOW(DisableReturnByReference);
  SHOW(EnableRenameFunction);
  SHOW(HackArrCompatNotices);
  SHOW(HackArrCompatIsArrayNotices);
  SHOW(HackArrCompatIsVecDictNotices);
  SHOW(HackArrCompatPromoteNotices);
  SHOW(HackArrCompatTypeHintNotices);
  SHOW(HackArrCompatDVCmpNotices);
  SHOW(HackArrCompatSerializeNotices);
  SHOW(HackArrDVArrs);
  SHOW(EnableIntishCast);
  SHOW(EnableIntrinsicsExtension);
  SHOW(ForbidDynamicCalls);
  SHOW(NoticeOnBuiltinDynamicCalls);
  SHOW(ReffinessInvariance);
  SHOW(AllowObjectDestructors);
  SHOW(AbortBuildOnVerifyError);
  SHOW(UndefinedConstFallback);
  SHOW(Signature);
#undef SHOW
  return out;
}

//////////////////////////////////////////////////////////////////////

}
