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
#include "hphp/runtime/vm/jit/cprof-replay.h"
#include "hphp/runtime/vm/jit/cprof-target-profile.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/prof-data-target-profile.h"
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
#include "hphp/util/boot-stats.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"

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

auto const s_startupPreparedFuncs =
  ServiceData::createCounter("jit.cprof.startup.prepared_funcs");
auto const s_startupPublishedFuncs =
  ServiceData::createCounter("jit.cprof.startup.published_funcs");

struct LoadedUnits {
  size_t pathsRequested{0};
  std::map<std::string, Unit*> units;
};

LoadedUnits loadCandidateUnits(
  const std::vector<ContProfProfileRecord>& records
) {
  LoadedUnits result{};

  std::set<std::string> paths;
  auto const addPaths = [&] (const ContProfFuncKey& key) {
    paths.emplace(key.resolutionUnitPath);
    if (key.bytecodeUnitPath) paths.emplace(*key.bytecodeUnitPath);
  };
  for (auto const& record : records) {
    auto const& key = record.header.funcKey;
    // Skip class-scoped closures to avoid populating the cache.
    if (key.closureContextName) continue;
    addPaths(key);
    for (auto const& translation : record.translations) {
      for (auto const& profile : translation.targetProfiles) {
        for (auto const& callee : contProfTargetProfileFuncKeys(profile)) {
          if (!callee.closureContextName) addPaths(callee);
        }
      }
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

void discardConflictingMethProfiles(ContProfStartupCandidate& candidate) {
  using Site = std::pair<Offset, const StringData*>;
  std::map<Site, const Func*> methods;
  for (auto const& translation : candidate.translations) {
    for (auto const& profile : translation.targetProfiles) {
      if (!profile.meth) continue;
      auto const func = profile.meth->uniqueMeth();
      auto const [it, inserted] = methods.emplace(
        Site{profile.bytecodeOffset, profile.name}, func
      );
      if (!inserted && it->second != func) it->second = nullptr;
    }
  }

  // Different methods could reduce to BaseMeth/InterfaceMeth, whose broader
  // dispatch assumptions aren't established by the imported function keys.
  for (auto& translation : candidate.translations) {
    std::erase_if(translation.targetProfiles, [&] (auto const& profile) {
      return profile.meth &&
        !methods.at(Site{profile.bytecodeOffset, profile.name});
    });
  }
}

ContProfStartupCandidate prepareCandidate(
  const ContProfProfileRecord& record,
  Func& func,
  const std::map<std::string, Unit*>& units
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
        Type{guard.type, PtrLocation::Bottom},
        guard.category,
      });
    }

    PostConditions postConditions{};

    for (auto const& post : translation.localPostConditions) {
      auto const location = RegionDesc::TypedLocation{
        Location::Local{post.localId},
        Type{post.type, PtrLocation::Bottom},
      };

      if (post.changed) {
        postConditions.changed.push_back(location);
      } else {
        postConditions.refined.push_back(location);
      }
    }

    std::vector<ContProfPreparedTargetProfile> targetProfiles;
    targetProfiles.reserve(translation.targetProfiles.size());

    for (auto const& profile : translation.targetProfiles) {
      auto prepared = prepareContProfTargetProfile(profile, units);
      if (!prepared) continue;

      targetProfiles.push_back(std::move(*prepared));
    }

    result.translations.push_back({
      *start,
      std::move(region),
      static_cast<int64_t>(translation.executionCount),
      translation.incoming,
      std::move(postConditions),
      std::move(targetProfiles),
    });
  }

  discardConflictingMethProfiles(result);
  return result;
}

void installCandidateProfileData(
  ProfData& profData,
  const ContProfStartupCandidate& candidate,
  int64_t counterDefault
) {
  auto const numTranslations = candidate.translations.size();

  std::vector<TransID> transIds;
  transIds.reserve(numTranslations);
  for (size_t i = 0; i < numTranslations; ++i) {
    transIds.push_back(profData.allocTransID());
  }

  auto const targetProfiles = profData.targetProfile();
  assertx(targetProfiles);

  for (size_t i = 0; i < numTranslations; ++i) {
    auto const& translation = candidate.translations[i];

    for (auto const& profile : translation.targetProfiles) {
      (void)installContProfTargetProfile(
        profile,
        transIds[i],
        *targetProfiles
      );
    }

    // Predecessor indices are record-local and were bounds-checked against
    // record.translations.size() during validation.
    RegionDesc::BlockIdSet incoming;
    incoming.reserve(translation.incoming.size());
    for (auto const predecessor : translation.incoming) {
      incoming.insert(transIds.at(predecessor));
    }

    *profData.transCounterAddr(transIds[i]) =
      counterDefault - translation.executionCount;

    translation.region->incoming(std::move(incoming));
    profData.addTransProfile(
      transIds[i],
      translation.region,
      translation.postConditions,
      0
    );
  }
}

bool installCandidate(
  ProfData& profData,
  const ContProfStartupCandidate& candidate
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

  SCOPE_FAIL {
    always_assert_flog(
      false, "cont-prof startup failed during ProfData installation"
    );
  };
  installCandidateProfileData(profData, candidate, counterDefault);
  profData.setProfiling(func);
  return true;
}

bool optimizeCandidate(
  ProfData& profData,
  const ContProfStartupCandidate& candidate
) {
  auto const func = candidate.func;

  VMProtect protect;
  LeaseHolder lease{func, TransKind::Optimize, true};
  if (!lease) return false;

  auto const funcId = func->getFuncId();
  if (!Func::isFuncIdValid(funcId) || Func::fromFuncId(funcId) != func ||
      jit::profData() != &profData || globalProfData() != &profData ||
      !profData.profiling(funcId) || profData.optimized(funcId) ||
      profData.funcProfTransIDs(funcId).empty()) {
    return false;
  }

  SCOPE_FAIL {
    always_assert_flog(
      false, "cont-prof startup failed during optimization"
    );
  };
  profData.setOptimized(funcId);
  mcgen::optimizeFunc(func);

  return std::any_of(
    candidate.translations.begin(),
    candidate.translations.end(),
    [](const ContProfStartupCandidate::Translation& translation) {
      auto const srcRec = tc::findSrcRec(translation.start);
      return srcRec && srcRec->numTrans() != 0;
    }
  );
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

    result.candidates.push_back(prepareCandidate(*record, *func, units));
    if (result.candidates.size() == maxFunctions) break;
  }

  return result;
}

std::vector<const ContProfStartupCandidate*> installContProfStartupCandidates(
  ProfData& profData,
  const std::vector<ContProfStartupCandidate>& candidates
) {
  std::vector<const ContProfStartupCandidate*> installed;
  installed.reserve(candidates.size());

  if (candidates.empty() || jit::profData() != &profData ||
      globalProfData() != &profData) {
    return installed;
  }

  if (!profData.targetProfile()) {
    // Replay covers only a subset of sites, so a miss falls back to live
    // RDS rather than meaning "no data".
    profData.setTargetProfile(
      std::make_unique<ProfDataTargetProfile>(/*liveFallback=*/true)
    );
  }

  for (auto const& candidate : candidates) {
    if (jit::profData() != &profData || globalProfData() != &profData) break;
    if (installCandidate(profData, candidate)) installed.push_back(&candidate);
  }
  return installed;
}

void consumeContProfAtStartup() noexcept {
  if (!contProfStartupActive()) {
    Logger::Info("cont-prof startup: inactive");
    return;
  }

  try {
    BootStats::Block timer{"cont_prof_startup", Cfg::Server::Mode, true};
    VMWorker([&] {
      ProfileNonVMThread nonVM;
      HphpSession session{Treadmill::SessionKind::TranslateWorker};

      ContProfReplayGuard replay{Cfg::Jit::ContProfCheckpointDirectory};
      if (!replay.active()) return;
      SCOPE_SUCCESS { replay.complete(); };

      auto const input = [&] {
        BootStats::Block timer{"cont_prof_read", Cfg::Server::Mode, true};
        return readContProfCheckpointDirectory(
          Cfg::Jit::ContProfCheckpointDirectory
        );
      }();
      if (!input) return;

      auto const loaded = [&] {
        BootStats::Block timer{"cont_prof_resolve", Cfg::Server::Mode, true};
        return loadCandidateUnits(input->records);
      }();
      auto const prepared = [&] {
        BootStats::Block timer{"cont_prof_prepare", Cfg::Server::Mode, true};
        return prepareContProfStartupCandidates(
          input->records,
          loaded.units,
          Cfg::Jit::ContProfStartupMaxFunctions
        );
      }();
      s_startupPreparedFuncs->setValue(prepared.candidates.size());

      auto const profData = jit::profData();
      if (!profData || globalProfData() != profData) return;

      setMayAcquireLease(true);
      SCOPE_EXIT { setMayAcquireLease(false); };

      // Make every imported callee's profiles available before compiling callers.
      auto const installed = [&] {
        BootStats::Block timer{"cont_prof_install", Cfg::Server::Mode, true};
        return installContProfStartupCandidates(*profData, prepared.candidates);
      }();

      size_t published{};

      {
        BootStats::Block timer{
          "cont_prof_optimize_serial", Cfg::Server::Mode, true
        };
        // Optimize candidates in descending function-hotness order.
        for (auto const candidate : installed) {
          if (jit::profData() != profData || globalProfData() != profData) {
            break;
          }

          if (optimizeCandidate(*profData, *candidate)) ++published;
        }
      }
      s_startupPublishedFuncs->setValue(published);

      Logger::Info(
        "Cont-prof startup published optimized code for %zu of "
        "%zu imported functions (%zu prepared, %zu candidates "
        "from %zu decoded records in %zu files, %zu resolved, "
        "%zu of %zu units ready)",
        published,
        installed.size(),
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
    Logger::Warning("cont-prof startup consumption failed");
  }
}

}
