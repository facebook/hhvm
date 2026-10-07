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
#include "hphp/runtime/vm/jit/cprof-target-profile.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"

namespace HPHP::jit::cprof {

namespace {

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

bool captureContProfProfile(const ProfData& profData, const Func& func) {
  std::vector<TransID> sourceTransIds;
  auto record = snapshotContProfProfileRecord(profData, func, &sourceTransIds);
  if (!record) return false;

  auto key = record->header.funcKey;
  auto records = s_records.wlock();
  return records->try_emplace(
    std::move(key),
    CapturedRecord{
      std::move(*record),
      std::move(sourceTransIds),
      func.bclen(),
    }
  ).second;
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
