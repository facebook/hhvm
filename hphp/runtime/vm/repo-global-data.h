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
#ifndef incl_HPHP_REPO_GLOBAL_DATA_H_
#define incl_HPHP_REPO_GLOBAL_DATA_H_

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

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
   * Indicates whether a repo was compiled using HHBBC.
   */
  bool UsedHHBBC = false;

  /*
   * Was the repo compiled with EnableHipHopSyntax.
   */
  bool EnableHipHopSyntax = false;

  /*
   * Copy of InitialNamedEntityTableSize for hhbbc to use.
   */
  uint32_t InitialNamedEntityTableSize = 0;

  /*
   * Copy of InitialStaticStringTableSize for hhbbc to use.
   */
  uint32_t InitialStaticStringTableSize = 0;

  /*
   * Indicates whether a repo was compiled with HardTypeHints.
   *
   * If so, we disallow recovering from the E_RECOVERABLE_ERROR we
   * raise if you violate a parameter typehint, because doing so
   * would allow violating assumptions from the optimizer.
   */
  bool HardTypeHints = false;

  /*
   * Indicates whether a repo was compiled with HardReturnTypeHints.
   *
   * If so, we disallow recovering from the E_RECOVERABLE_ERROR we
   * raise if you violate a return typehint, because doing so would
   * allow violating assumptions from the optimizer.
   */
  bool HardReturnTypeHints = false;

  /*
   * Indicates whether the repo was compiled with CheckPropTypeHints.
   */
  int32_t CheckPropTypeHints = 0;

  /*
   * Indicates whether a repo was compiled assumming that `this` types will be
   * verified by Verify*Type instructions at runtime.
   *
   * This changes program behavior because this type hints that are checked
   * at runtime will enable additional HHBBC optimizations.
   */
  int32_t ThisTypeHintLevel = 0;

  /*
   * Indicates whether a repo was compiled with HardPrivatePropInference.
   */
  bool HardPrivatePropInference = false;

  /*
   * Indicates whether the repo was compiled with DisallowDynamicVarEnvFuncs. If
   * so, we assume that '$f()' doesn't read or write over locals (that
   * haven't been passed).
   */
  HackStrictOption DisallowDynamicVarEnvFuncs = HackStrictOption::OFF;

  /*
   * Indicates whether the repo was compiled with ElideAutoloadInvokes. If so,
   * potential invocations of the autoloader may have been optimized away if it
   * could be proven the invocation would not find a viable function.
   */
  bool ElideAutoloadInvokes = true;

  /*
   * Indicates whether the repo was compiled with PHP7 integer semantics. This
   * slightly changes the way certain arithmetic operations are evaluated, in
   * small enough ways that don't warrant new bytecodes, but in ways that do
   * affect everything from hphpc's constant folding up through the JIT, and
   * so need to be kept consistent.
   */
  bool PHP7_IntSemantics = false;

  /*
   * Indicates whether hex strings (e.g. "0x20") can be used for numeric
   * operations, e.g. ("0x20" + 1)
   */
  bool PHP7_NoHexNumerics = false;

  /*
   * Indicates whether the repo was compiled with PHP7 scalar type hint support.
   * In this mode non hh units will default to weak types and scalar types will
   * be available outside the HH namespace.
   */
  bool PHP7_ScalarTypes = false;

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
   * Indicates that generators should be autoprimed and not require an initial
   * call to next() before calling other generator functions.
   */
  bool AutoprimeGenerators = true;

  /*
   * Should emptyish in lval context be promoted to a stdclass object?
   */
  bool PromoteEmptyObject = true;

  /*
   * Disable return by reference in PHP
   */
  bool DisableReturnByReference = true;

  /*
   * Should all functions be interceptable?
   */
  bool EnableRenameFunction = false;

  /*
   * Are Hack array compatibility notices enabled? If so, certain optimizations
   * may be disabled.
   */
  bool HackArrCompatNotices = false;
  bool HackArrCompatIsArrayNotices = false;
  bool HackArrCompatIsVecDictNotices = false;
  bool HackArrCompatPromoteNotices = false;
  bool HackArrCompatTypeHintNotices = false;
  bool HackArrCompatDVCmpNotices = false;
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
  int32_t ForbidDynamicCalls = 0;

  /*
   * Should the runtime emit notices whenever a builtin is called dynamically?
   */
  bool NoticeOnBuiltinDynamicCalls = false;

  /*
   * Should we enforce that reffiness annotations are invaraint in overridden
   * methods?
   */
  bool ReffinessInvariance = false;

  /*
   * Are objects allowed to run destructors?
   */
  bool AllowObjectDestructors = false;

  /*
   * Should HHBBC do build time verification?
   */
  bool AbortBuildOnVerifyError = false;

  /*
   * A more-or-less unique identifier for the repo
   */
  uint64_t Signature = 0;

  std::vector<const StringData*> APCProfile;

  std::vector<std::pair<std::string,Cell>> ConstantFunctions;

  template<class SerDe> void serde(SerDe& sd) {
    sd(UsedHHBBC)
      (EnableHipHopSyntax)
      (InitialNamedEntityTableSize)
      (InitialStaticStringTableSize)
      (HardTypeHints)
      (ThisTypeHintLevel)
      (HardReturnTypeHints)
      (CheckPropTypeHints)
      (HardPrivatePropInference)
      (DisallowDynamicVarEnvFuncs)
      (ElideAutoloadInvokes)
      (PHP7_IntSemantics)
      (PHP7_NoHexNumerics)
      (PHP7_ScalarTypes)
      (PHP7_Substr)
      (PHP7_Builtins)
      (AutoprimeGenerators)
      (PromoteEmptyObject)
      (EnableRenameFunction)
      (HackArrCompatNotices)
      (HackArrCompatIsArrayNotices)
      (HackArrCompatIsVecDictNotices)
      (HackArrCompatPromoteNotices)
      (HackArrCompatTypeHintNotices)
      (HackArrCompatDVCmpNotices)
      (HackArrCompatSerializeNotices)
      (HackArrDVArrs)
      (EnableIntrinsicsExtension)
      (ReffinessInvariance)
      (ForbidDynamicCalls)
      (NoticeOnBuiltinDynamicCalls)
      (AllowObjectDestructors)
      (Signature)
      (DisableReturnByReference)
      (AbortBuildOnVerifyError)
      ;
  }
};

//////////////////////////////////////////////////////////////////////

}


#endif
