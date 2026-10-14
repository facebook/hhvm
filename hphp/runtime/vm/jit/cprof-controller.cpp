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

#include "hphp/runtime/vm/jit/cprof-controller.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <variant>
#include <vector>

#include <folly/Synchronized.h>

#include "hphp/runtime/base/rds-symbol.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/cprof-capture.h"
#include "hphp/runtime/vm/jit/cprof-checkpoint.h"
#include "hphp/runtime/vm/jit/cprof-target-profile.h"
#include "hphp/util/assertions.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/service-data.h"

namespace HPHP::jit::cprof {

namespace {

// Cumulative process totals. Opportunities are sync/async profile optimizations
// with capture configured, including those skipped because the writer is not
// active. Disabled capture does not touch any counters.
auto const s_captureOpportunities = ServiceData::createCounter(
  "jit.cprof.capture.opportunities"
);
// The async-only subset, counted after the queued optimization is accepted,
// not when it is enqueued (enqueueing can deduplicate or reject requests).
auto const s_asyncCaptureOpportunities = ServiceData::createCounter(
  "jit.cprof.capture.async_opportunities"
);
auto const s_captureSkippedInactive = ServiceData::createCounter(
  "jit.cprof.capture.skipped_inactive"
);
// Attempts count calls to captureContProfProfile. Snapshot outcomes precede
// duplicate detection, so successes = stored_records + duplicates. Imported
// records retained for future checkpoints do not contribute to these totals.
auto const s_captureAttempts = ServiceData::createCounter(
  "jit.cprof.capture.attempts"
);
auto const s_snapshotSuccesses = ServiceData::createCounter(
  "jit.cprof.capture.snapshot_successes"
);
auto const s_snapshotFailures = ServiceData::createCounter(
  "jit.cprof.capture.snapshot_failures"
);
auto const s_storedRecords = ServiceData::createCounter(
  "jit.cprof.capture.stored_records"
);
auto const s_duplicates = ServiceData::createCounter(
  "jit.cprof.capture.duplicates"
);

// Rejection scopes and first-rejection semantics are defined by
// ContProfCaptureStats. Flush at most once per nonzero reason per function.
struct RejectionCounter {
  ContProfCaptureRejection reason;
  ServiceData::ExportedCounter* counter;
};
using Rejection = ContProfCaptureRejection;
RejectionCounter const s_rejections[] = {
  {Rejection::InvalidFuncKey, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_func_key")},
  {Rejection::NoProfileTranslations, ServiceData::createCounter(
    "jit.cprof.capture.rejected.no_profile_translations")},
  {Rejection::PrologueOrResumedTranslation, ServiceData::createCounter(
    "jit.cprof.capture.rejected.prologue_or_resumed_translation")},
  {Rejection::UnexpectedRegion, ServiceData::createCounter(
    "jit.cprof.capture.rejected.unexpected_region")},
  {Rejection::UnexpectedBlock, ServiceData::createCounter(
    "jit.cprof.capture.rejected.unexpected_block")},
  {Rejection::InvalidEntry, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_entry")},
  {Rejection::InvalidLocalTypeGuards, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_local_type_guards")},
  {Rejection::InvalidEntryTypeGuards, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_entry_type_guards")},
  {Rejection::InvalidLocalPostconditions, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_local_postconditions")},
  {Rejection::ZeroExecutionCount, ServiceData::createCounter(
    "jit.cprof.capture.rejected.zero_execution_count")},
  {Rejection::NoEligibleCandidates, ServiceData::createCounter(
    "jit.cprof.capture.rejected.no_eligible_candidates")},
  {Rejection::InvalidFinalRecord, ServiceData::createCounter(
    "jit.cprof.capture.rejected.invalid_final_record")},
};

/*
 * A captured record plus what the target-profile sweep needs later.
 * `sourceTransIds` is cleared once the sweep has run, so empty means the
 * record is final. `bytecodeLength` is kept so the sweep does not need the
 * Func.
 */
struct CapturedRecord {
  ContProfProfileRecord record;
  std::vector<TransID> sourceTransIds;
  Offset bytecodeLength{0};
};

struct TargetProfileDestination {
  ContProfProfileTranslation* translation;
  Offset bytecodeLength;
};

using RecordMap = std::map<ContProfFuncKey, CapturedRecord>;
folly::Synchronized<RecordMap> s_records;

void snapshotTargetProfiles(std::vector<CapturedRecord>& records) {
  size_t numPendingTranslations = 0;
  for (auto const& captured : records) {
    numPendingTranslations += captured.sourceTransIds.size();
  }

  hphp_fast_map<TransID, TargetProfileDestination> destinations;
  destinations.reserve(numPendingTranslations);

  for (auto& captured : records) {
    if (captured.sourceTransIds.empty()) continue;

    assertx(
      captured.sourceTransIds.size() == captured.record.translations.size()
    );

    for (size_t i = 0; i < captured.sourceTransIds.size(); ++i) {
      auto& translation = captured.record.translations[i];
      translation.targetProfiles.clear();
      auto const DEBUG_ONLY inserted = destinations.emplace(
        captured.sourceTransIds[i],
        TargetProfileDestination{&translation, captured.bytecodeLength}
      ).second;
      assertx(inserted);
    }
  }

  if (destinations.empty()) return;

  rds::visitSymbols([&](
      const rds::Symbol& symbol,
      rds::Handle handle,
      uint32_t allocationSize) {
    auto const profile = std::get_if<rds::Profile>(&symbol);
    if (!profile) return;

    auto const it = destinations.find(profile->transId);
    if (it == destinations.end()) return;

    auto const& destination = it->second;
    if (profile->bcOff >= destination.bytecodeLength) return;

    auto snapshot = snapshotContProfTargetProfile(
      *profile,
      handle,
      allocationSize
    );
    if (!snapshot) return;

    destination.translation->targetProfiles.push_back(std::move(*snapshot));
  });

  for (auto& captured : records) {
    if (captured.sourceTransIds.empty()) continue;

    for (auto& translation : captured.record.translations) {
      std::sort(
        translation.targetProfiles.begin(),
        translation.targetProfiles.end(),
        contProfTargetProfileKeyLess
      );
    }

    // Capture produces valid payloads, and RDS profile keys are
    // unique, so sorting must leave the record valid.
    assertx(isValidContProfProfileRecord(captured.record));
  }
}

}

void maybeCaptureContProfProfile(
  const ProfData& profData,
  const Func& func,
  bool async
) {
  if (!Cfg::Jit::ContProfCaptureEnabled) return;
  s_captureOpportunities->increment();
  if (async) s_asyncCaptureOpportunities->increment();
  if (!contProfActive()) {
    s_captureSkippedInactive->increment();
    return;
  }
  captureContProfProfile(profData, func);
}

bool captureContProfProfile(const ProfData& profData, const Func& func) {
  s_captureAttempts->increment();
  ContProfCaptureStats stats;
  std::vector<TransID> sourceTransIds;
  auto record = snapshotContProfProfileRecord(
    profData, func, &sourceTransIds, &stats
  );
  for (auto const& rejection : s_rejections) {
    auto const count = stats.rejections[static_cast<size_t>(rejection.reason)];
    if (count) rejection.counter->addValue(count);
  }
  if (!record) {
    s_snapshotFailures->increment();
    return false;
  }
  s_snapshotSuccesses->increment();

  auto key = record->header.funcKey;
  auto records = s_records.wlock();
  auto const inserted = records->try_emplace(
    std::move(key),
    CapturedRecord{
      std::move(*record),
      std::move(sourceTransIds),
      func.bclen(),
    }
  ).second;
  (inserted ? s_storedRecords : s_duplicates)->increment();
  return inserted;
}

size_t numContProfProfileRecords() {
  auto records = s_records.rlock();
  return records->size();
}

std::vector<ContProfProfileRecord> snapshotContProfProfileRecords() {
  std::vector<CapturedRecord> capturedRecords;
  {
    auto records = s_records.rlock();
    capturedRecords.reserve(records->size());

    for (auto const& [_, captured] : *records) {
      capturedRecords.push_back(captured);
    }
  }

  snapshotTargetProfiles(capturedRecords);

  {
    auto records = s_records.wlock();

    for (auto& captured : capturedRecords) {
      if (captured.sourceTransIds.empty()) continue;

      // Records are only inserted, never removed or replaced. Another
      // snapshot may have finalized this one while the sweep was running.
      auto& stored = records->at(captured.record.header.funcKey);
      if (stored.sourceTransIds.empty()) continue;

      stored.record = captured.record;
      stored.sourceTransIds = {};
    }
  }

  std::vector<ContProfProfileRecord> result;
  result.reserve(capturedRecords.size());

  for (auto& captured : capturedRecords) {
    result.push_back(std::move(captured.record));
  }

  return result;
}

}
