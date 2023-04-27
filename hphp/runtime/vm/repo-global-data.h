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

#include "hphp/runtime/base/config.h"

#include "hphp/util/optional.h"

#include <cstdint>
#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Global repo metadata.
 *
 * Only used in RepoAuthoritative mode.  See loadGlobalData().
 */
struct RepoGlobalData {
  /*
   * Copy of InitialNamedEntityTableSize for hhbbc to use.
   */
  uint32_t InitialTypeTableSize = 0;
  uint32_t InitialFuncTableSize = 0;

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
   * will be verified by the parameter type verification and VerifyReturnType
   * instructions at runtime.
   *
   * This changes program behavior because this type hints that are checked
   * at runtime will enable additional HHBBC optimizations.
   */
  bool HardGenericsUB = false;

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

  bool HackArrCompatSerializeNotices = false;

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
  bool ForbidDynamicCallsWithAttr = true;

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

  bool EmitClsMethPointers = true;

  /*
   * If clsmeth type may raise,
   * hhbbc IsTypeX optimization may be disabled.
   */
  bool IsVecNotices = false;

  /* Whether implicit class conversions can raise a warning */
  bool RaiseClassConversionWarning = false;

  /* Whether classname type-hint accepts (lazy) classes */
  bool ClassPassesClassname = false;

  /* Whether passing (lazy) classes to classname can raise a notice */
  bool ClassnameNotices = false;

  /* Whether passing (lazy) classes to string type-hints can raise a notice */
  bool ClassStringHintNotices = false;

  /* Whether checking is string on (lazy) classes can raise a notice */
  bool ClassIsStringNotices = false;

  /* Whether implicit coercions for concat/interp trigger logs/exceptions */
  int32_t NoticeOnCoerceForStrConcat = 0;

  /* Constants from traits behave like constants from interfaces (error on conflict) */
  bool TraitConstantInterfaceBehavior = false;

  /*
   * The Hack.Lang.StrictArrayFillKeys option the repo was compiled with.
   */
  HackStrictOption StrictArrayFillKeys = HackStrictOption::OFF;

  std::vector<std::pair<std::string,std::string>> ConstantFunctions;

  bool BuildMayNoticeOnMethCallerHelperIsObject = false;

  std::unordered_map<std::string, int> EvalCoeffectEnforcementLevels = {};

  /*
   * Describes the active deployment for selecting the set of packages
   */
  std::string ActiveDeployment = "";

  /* Enable a method defined in a trait to be imported multiple times
   * along trait use paths
   */
  bool DiamondTraitMethods = false;

  /* Whether bespoke type structures should be used */
  bool EmitBespokeTypeStructures = false;

  /*
   * If set, const fold the File and Dir bytecodes, using this as the
   * SourceRoot.
   */
  Optional<std::string> SourceRootForFileBC;

  // Load the appropriate options into their matching
  // RuntimeOptions. If `loadConstantFuncs' is true, also deserialize
  // ConstantFunctions and store it in RuntimeOptions (this can only
  // be done if the memory manager is initialized).
  void load(bool loadConstantFuncs = true) const;

  /* Enable the "Module level traits" semantics: methods defined in a trait
   * that belongs to module A, are considered as belonging to module A
   * even if the trait is used by a class that lives in module B.
   */
  bool ModuleLevelTraits = false;

  // NB: Only use C++ types in this struct because we want to be able
  // to serde it before memory manager and family are set up.

  template<class SerDe> void serde(SerDe& sd) {
    sd(InitialTypeTableSize)
      (InitialFuncTableSize)
      (InitialStaticStringTableSize)
      (HardGenericsUB)
      (CheckPropTypeHints)
      (PHP7_NoHexNumerics)
      (PHP7_Substr)
      (PHP7_Builtins)
      (HackArrCompatSerializeNotices)
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
      (EmitClsMethPointers)
      (IsVecNotices)
      (RaiseClassConversionWarning)
      (ClassPassesClassname)
      (ClassnameNotices)
      (ClassStringHintNotices)
      (ClassIsStringNotices)
      (StrictArrayFillKeys)
      (NoticeOnCoerceForStrConcat)
      (TraitConstantInterfaceBehavior)
      (ConstantFunctions)
      (BuildMayNoticeOnMethCallerHelperIsObject)
      (DiamondTraitMethods)
      (EvalCoeffectEnforcementLevels, std::less<std::string>{})
      (SourceRootForFileBC)
      (EmitBespokeTypeStructures)
      (ActiveDeployment)
      (ModuleLevelTraits)
      ;
  }
};

std::string show(const RepoGlobalData& gd);

//////////////////////////////////////////////////////////////////////

}
