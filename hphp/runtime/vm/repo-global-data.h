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
#pragma once

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-autoload-map.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Global repo metadata.
 *
 * Only used in RepoAuthoritative mode.  See loadGlobalData().
 */
struct Repo::GlobalData {
  GlobalData() {}

  /*
   * Copy of InitialNamedEntityTableSize for hhbbc to use.
   */
  uint32_t InitialNamedEntityTableSize = 0;

  /*
   * Copy of InitialStaticStringTableSize for hhbbc to use.
   */
  uint32_t InitialStaticStringTableSize = 0;

  /*
   * Indicates whether the repo was compiled with CheckPropTypeHints.
   */
  int32_t CheckPropTypeHints = 0;

  /*
   * Indicates whether a repo was compiled assumming that UpperBound type-hints
   * will be verified by VerifyParamType and VerifyReturnType instructions
   * at runtime.
   *
   * This changes program behavior because this type hints that are checked
   * at runtime will enable additional HHBBC optimizations.
   */
   bool HardGenericsUB = false;

  /*
   * Indicates whether a repo was compiled with HardPrivatePropInference.
   */
  bool HardPrivatePropInference = false;

  /*
   * Indicates whether hex strings (e.g. "0x20") can be used for numeric
   * operations, e.g. ("0x20" + 1)
   */
  bool PHP7_NoHexNumerics = false;

  /*
   * Indicates whether the repo was compiled with PHP7 builtins enabled.
   */
  bool PHP7_Builtins = false;

  /*
   * Indicates whether the repo was compiled with PHP7 substr behavior which
   * returns an empty string if the stringi length is equal to start characters
   * long, instead of PHP5's false.
   */
  bool PHP7_Substr = false;

  /*
   * Should all functions be interceptable?
   */
  bool EnableRenameFunction = false;

  /*
   * Are Hack array compatibility notices enabled? If so, certain optimizations
   * may be disabled.
   */
  bool HackArrCompatNotices = false;
  bool HackArrCompatIsVecDictNotices = false;
  bool HackArrCompatSerializeNotices = false;

  /*
   * Are d/varrays dicts and vecs?
   */
  bool HackArrDVArrs = false;

  /*
   * Should the extension containing HHVM intrinsics be enabled?
   */
  bool EnableIntrinsicsExtension = false;

  /*
   * Should the runtime emit notices or throw whenever a function is called
   * dynamically and that function has not been marked as allowing that?
   */
  int32_t ForbidDynamicCallsToFunc = 0;
  int32_t ForbidDynamicCallsToClsMeth = 0;
  int32_t ForbidDynamicCallsToInstMeth = 0;
  int32_t ForbidDynamicConstructs = 0;
  bool ForbidDynamicCallsWithAttr = false;

  /*
  * If set to true calls to class methods of form $cls::meth() will not be
  * logged as dynamic calls, which means behavior for such calls would be
  * as if ForbidDynamicCallsToClsMeth was set to 0.
  */
  bool LogKnownMethodsAsDynamicCalls = true;

  /*
   * Should the runtime emit notices whenever a builtin is called dynamically?
   */
  bool NoticeOnBuiltinDynamicCalls = false;

  /*
   * Should HHBBC do build time verification?
   */
  bool AbortBuildOnVerifyError = false;

  /*
   * Should we display function arguments in backtraces?
   */
  bool EnableArgsInBacktraces = false;

  /*
   * A more-or-less unique identifier for the repo
   */
  uint64_t Signature = 0;

  int32_t EmitClassPointers = 0;
  bool EmitClsMethPointers = false;

  /*
   * If clsmeth type may raise,
   * hhbbc IsTypeX optimization may be disabled.
   */
  bool IsVecNotices = false;

  /* Skip ClsMeth type refinement when this is true. */
  bool IsCompatibleClsMethType = false;

  /* Avoid optimizations that interfere with array provenance */
  bool ArrayProvenance = false;

  /*
   * The Hack.Lang.StrictArrayFillKeys option the repo was compiled with.
   */
  HackStrictOption StrictArrayFillKeys = HackStrictOption::OFF;

  std::vector<const StringData*> APCProfile;

  std::vector<std::pair<std::string,TypedValue>> ConstantFunctions;

  std::unique_ptr<RepoAutoloadMap> AutoloadMap = nullptr;

  template<class SerDe> void serde(SerDe& sd) {
    sd(InitialNamedEntityTableSize)
      (InitialStaticStringTableSize)
      (HardGenericsUB)
      (CheckPropTypeHints)
      (HardPrivatePropInference)
      (PHP7_NoHexNumerics)
      (PHP7_Substr)
      (PHP7_Builtins)
      (EnableRenameFunction)
      (HackArrCompatNotices)
      (HackArrCompatIsVecDictNotices)
      (HackArrCompatSerializeNotices)
      (HackArrDVArrs)
      (EnableIntrinsicsExtension)
      (ForbidDynamicCallsToFunc)
      (ForbidDynamicCallsToClsMeth)
      (ForbidDynamicCallsToInstMeth)
      (ForbidDynamicConstructs)
      (ForbidDynamicCallsWithAttr)
      (LogKnownMethodsAsDynamicCalls)
      (NoticeOnBuiltinDynamicCalls)
      (Signature)
      (AbortBuildOnVerifyError)
      (EnableArgsInBacktraces)
      (EmitClassPointers)
      (EmitClsMethPointers)
      (IsVecNotices)
      (IsCompatibleClsMethType)
      (ArrayProvenance)
      (StrictArrayFillKeys)
      ;
  }
};

std::string show(const Repo::GlobalData& gd);

//////////////////////////////////////////////////////////////////////

}


