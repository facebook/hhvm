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

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "hphp/runtime/vm/srckey.h"

namespace HPHP {
struct Func;
struct Unit;
}

namespace HPHP::jit {
struct RegionDesc;
}

namespace HPHP::jit::cprof {

struct ContProfProfileRecord;

struct ContProfStartupCandidate {
  struct Translation {
    SrcKey start;
    std::shared_ptr<RegionDesc> region;
    int64_t executionCount{0};
  };

  Func* func{nullptr};
  std::vector<Translation> translations;
};

struct ContProfStartupCandidates {
  // Only includes records resolved before reaching the preparation cap.
  size_t recordsResolved{0};
  std::vector<ContProfStartupCandidate> candidates;
};

/*
 * Resolve records against already merged units and prepare compatible functions
 * in descending hotness order. Only successful preparations count toward the
 * cap; the result does not own the units or functions.
 */
ContProfStartupCandidates prepareContProfStartupCandidates(
  const std::vector<ContProfProfileRecord>& records,
  const std::map<std::string, Unit*>& units,
  size_t maxFunctions
);

/* Whether continuous-profile startup replay is enabled. */
bool contProfStartupActive();

/*
 * Load compatible checkpoints and publish optimized code during server
 * startup.
 */
void consumeContProfAtStartup() noexcept;

}
