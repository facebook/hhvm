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
#include "hphp/runtime/base/configs/repo-global-data-generated.h"

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
#define C(_, Name, Type) Type Name;
  CONFIGS_FOR_REPOGLOBALDATA()
#undef C

  /*
   * Should we display function arguments in backtraces?
   */
  bool EnableArgsInBacktraces = false;

  /*
   * A more-or-less unique identifier for the repo
   */
  uint64_t Signature = 0;

  std::vector<std::pair<std::string,std::string>> ConstantFunctions;

  std::unordered_map<std::string, int> EvalCoeffectEnforcementLevels = {};

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

  // NB: Only use C++ types in this struct because we want to be able
  // to serde it before memory manager and family are set up.

  template<class SerDe> void serde(SerDe& sd) {
    sd(Signature)
      (EnableArgsInBacktraces)
      (ConstantFunctions)
      (EvalCoeffectEnforcementLevels, std::less<std::string>{})
      (SourceRootForFileBC)
      ;

#define C(_, Name, ...) sd(Name);
  CONFIGS_FOR_REPOGLOBALDATA()
#undef C
  }
};

std::string show(const RepoGlobalData& gd);

//////////////////////////////////////////////////////////////////////

}
