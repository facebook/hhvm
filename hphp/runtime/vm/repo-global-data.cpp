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
  SHOW(RaiseClsMethConversionWarning);
  SHOW(StrictArrayFillKeys);
  SHOW(NoticeOnCoerceForStrConcat);
  SHOW(NoticeOnCoerceForBitOp);
#undef SHOW
  return out;
}

//////////////////////////////////////////////////////////////////////

}
