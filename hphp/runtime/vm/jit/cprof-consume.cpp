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

#include "hphp/runtime/vm/jit/cprof-consume.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/cprof-key.h"
#include "hphp/runtime/vm/jit/cprof-reader.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/assertions.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/logger.h"

namespace HPHP::jit::cprof {

bool contProfStartupActive() {
  return Cfg::Server::Mode &&
    Cfg::Repo::Authoritative &&
    Cfg::Jit::ContProfStartupMaxFunctions != 0 &&
    !Cfg::Jit::ContProfCheckpointDirectory.empty() &&
    Cfg::Jit::Enabled &&
    Cfg::Jit::PGO &&
    !mcgen::retranslateAllEnabled() &&
    RuntimeOption::EvalJitSerdesMode == JitSerdesMode::Off &&
    !(Cfg::Jit::DisabledByVSDebug && isDebuggerAttachedProcess());
}

namespace {

struct LoadedUnits {
  size_t pathsRequested{0};
  std::map<std::string, Unit*> units;
};

LoadedUnits loadCandidateUnits(
  const std::vector<ContProfProfileRecord>& records
) {
  LoadedUnits result{};

  std::set<std::string> paths;
  for (auto const& record : records) {
    auto const& key = record.header.funcKey;
    // Skip class-scoped closures to avoid populating the cache.
    if (key.closureContextName) continue;
    paths.emplace(key.resolutionUnitPath);
    if (key.bytecodeUnitPath) {
      paths.emplace(*key.bytecodeUnitPath);
    }
  }
  result.pathsRequested = paths.size();

  for (auto const& path : paths) {
    try {
      auto const lookupPath = String{Cfg::Server::SourceRoot + path};
      auto const unit = lookupUnit(
        lookupPath.get(), "", nullptr, nullptr, false
      );
      if (unit) result.units.emplace(path, unit);
    } catch (...) {
      continue;
    }
  }

  // Merge all units before resolving functions so class lookups are available.
  for (auto it = result.units.begin(); it != result.units.end();) {
    try {
      it->second->merge();
      ++it;
    } catch (...) {
      it = result.units.erase(it);
    }
  }

  return result;
}

ContProfStartupCandidate prepareCandidate(
  const ContProfProfileRecord& record,
  Func& func
) {
  ContProfStartupCandidate result{};
  result.func = &func;
  result.translations.reserve(record.translations.size());

  // Rebuild each retained translation as a one-block profiling region.
  for (auto const& translation : record.translations) {
    auto const start = contProfTranslationSrcKey(translation, func);
    assertx(start);

    auto region = std::make_shared<RegionDesc>();
    auto const block = region->addBlock(
      *start,
      static_cast<int>(translation.regionLength),
      SBInvOffset{0}
    );
    for (auto const& guard : translation.localTypeGuards) {
      block->addPreCondition(RegionDesc::GuardedLocation{
        Location::Local{guard.localId},
        Type{guard.type},
        DataTypeSpecific,
      });
    }

    // Explicit empty means no predecessors; null infers from the live SrcDB.
    region->incoming(RegionDesc::BlockIdSet{});

    result.translations.push_back({
      *start,
      std::move(region),
      static_cast<int64_t>(translation.executionCount),
    });
  }

  return result;
}

bool optimizeCandidate(
  ProfData& profData,
  const ContProfStartupCandidate& candidate,
  bool& installing
) {
  auto const func = candidate.func;

  VMProtect protect;
  LeaseHolder lease{func, TransKind::Optimize, true};
  if (!lease) return false;

  auto const funcId = func->getFuncId();

  // Only import into a Func the JIT hasn't touched; the profData identity
  // checks catch a discard racing the caller's check.
  if (!Func::isFuncIdValid(funcId) || Func::fromFuncId(funcId) != func ||
      jit::profData() != &profData || globalProfData() != &profData ||
      profData.profiling(funcId) || profData.optimized(funcId) ||
      !profData.funcProfTransIDs(funcId).empty()) {
    return false;
  }

  auto const counterDefault = profData.counterDefault();
  if (counterDefault < 0) return false;

  // Create every SrcRec before mutating ProfData. Replay supports only
  // empty-stack starts, which capture and compatibility checks enforce.
  for (auto const& translation : candidate.translations) {
    if (!tc::createSrcRec(translation.start, SBInvOffset{0})) {
      return false;
    }
  }

  installing = true;

  // Install synthetic counters and regions using fresh TransIDs.
  for (auto const& translation : candidate.translations) {
    auto const transId = profData.allocTransID();

    *profData.transCounterAddr(transId) =
      counterDefault - translation.executionCount;

    profData.addTransProfile(
      transId,
      translation.region,
      PostConditions{},
      0
    );
  }

  profData.setProfiling(func);
  profData.setOptimized(funcId);
  mcgen::optimizeFunc(func);

  auto const published = std::any_of(
    candidate.translations.begin(),
    candidate.translations.end(),
    [](const ContProfStartupCandidate::Translation& translation) {
      auto const srcRec = tc::findSrcRec(translation.start);
      return srcRec && srcRec->numTrans() != 0;
    }
  );

  installing = false;
  return published;
}

}

ContProfStartupCandidates prepareContProfStartupCandidates(
  const std::vector<ContProfProfileRecord>& records,
  const std::map<std::string, Unit*>& units,
  size_t maxFunctions
) {
  ContProfStartupCandidates result{};
  if (!maxFunctions) return result;
  result.candidates.reserve(std::min(records.size(), maxFunctions));

  std::vector<const ContProfProfileRecord*> ordered;
  ordered.reserve(records.size());
  for (auto const& record : records) ordered.push_back(&record);
  std::sort(ordered.begin(), ordered.end(), [](auto const lhs, auto const rhs) {
    auto const lhsExecutions = lhs->functionExecutions();
    auto const rhsExecutions = rhs->functionExecutions();
    if (lhsExecutions != rhsExecutions) {
      return lhsExecutions > rhsExecutions;
    }
    return lhs->header.funcKey < rhs->header.funcKey;
  });

  for (auto const record : ordered) {
    auto const& key = record->header.funcKey;
    if (key.closureContextName) continue;

    auto const resolutionUnit = units.find(key.resolutionUnitPath);
    if (resolutionUnit == units.end()) continue;
    if (key.bytecodeUnitPath &&
        units.find(*key.bytecodeUnitPath) == units.end()) {
      continue;
    }

    Func* func{nullptr};
    try {
      func = resolveContProfFunc(key, *resolutionUnit->second);
      if (!func) continue;

      ++result.recordsResolved;
      if (!isContProfProfileRecordCompatible(*record, *func)) continue;
    } catch (...) {
      continue;
    }

    result.candidates.push_back(prepareCandidate(*record, *func));
    if (result.candidates.size() == maxFunctions) break;
  }

  return result;
}

void consumeContProfAtStartup() noexcept {
  if (!contProfStartupActive()) {
    Logger::Info("cont-prof startup: inactive");
    return;
  }

  auto installing = false;

  try {
    VMWorker([&] {
      ProfileNonVMThread nonVM;
      HphpSession session{Treadmill::SessionKind::TranslateWorker};

      auto const input = readContProfCheckpointDirectory(
        Cfg::Jit::ContProfCheckpointDirectory
      );
      if (!input) return;

      auto const loaded = loadCandidateUnits(input->records);
      auto const prepared = prepareContProfStartupCandidates(
        input->records,
        loaded.units,
        Cfg::Jit::ContProfStartupMaxFunctions
      );

      auto const profData = jit::profData();
      if (!profData || globalProfData() != profData) return;

      setMayAcquireLease(true);
      SCOPE_EXIT { setMayAcquireLease(false); };

      size_t published{};

      // Optimize candidates in descending function-hotness order.
      for (auto const& candidate : prepared.candidates) {
        if (jit::profData() != profData || globalProfData() != profData) {
          break;
        }

        if (optimizeCandidate(*profData, candidate, installing)) ++published;
      }

      Logger::Info(
        "Cont-prof startup published optimized code for %zu of "
        "%zu prepared functions (%zu candidates "
        "from %zu decoded records in %zu files, %zu resolved, "
        "%zu of %zu units ready)",
        published,
        prepared.candidates.size(),
        input->records.size(),
        input->recordsDecoded,
        input->filesRead,
        prepared.recordsResolved,
        loaded.units.size(),
        loaded.pathsRequested
      );
    }).run();
  } catch (...) {
    always_assert_flog(
      !installing,
      "cont-prof startup failed during ProfData installation"
    );
    Logger::Warning("cont-prof startup consumption failed");
  }
}

}
