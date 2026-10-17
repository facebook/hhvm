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

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

#include "hphp/runtime/vm/jit/cprof-record.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
struct Func;
}

namespace HPHP::jit {

struct ProfData;

}

namespace HPHP::jit::cprof {

enum class ContProfCaptureRejection {
  InvalidFuncKey,
  NoProfileTranslations,
  PrologueOrResumedTranslation,
  UnexpectedRegion,
  UnexpectedBlock,
  InvalidEntry,
  InvalidLocalTypeGuards,
  InvalidEntryTypeGuards,
  InvalidLocalPostconditions,
  ZeroExecutionCount,
  NoEligibleCandidates,
  InvalidFinalRecord,
  Count,
};

/*
 * Per-snapshot counts, reset on each call. InvalidFuncKey,
 * NoProfileTranslations, NoEligibleCandidates, and InvalidFinalRecord count
 * terminal function failures. Other reasons count the first rejection of each
 * examined translation, including when the function snapshot succeeds.
 * InvalidEntry also covers malformed bytecode starts.
 * Examined/eligible translation counts precede per-start deduplication.
 * Retained/start-deduplicated counts precede final record validation.
 * Region/block details partition UnexpectedRegion/UnexpectedBlock, using the
 * first failing condition. Missing regions include null and zero-block regions.
 */
struct ContProfCaptureStats {
  size_t translationsExamined{};
  size_t translationsEligible{};
  size_t translationsRetained{};
  size_t translationsStartDeduplicated{};
  size_t missingRegions{};
  size_t multipleBlockRegions{};
  size_t mismatchedBlockStarts{};
  size_t nonzeroStackOffsets{};
  size_t emptyBlocks{};
  std::array<size_t, static_cast<size_t>(ContProfCaptureRejection::Count)>
    rejections{};
};

/*
 * Snapshot the representable, positive-count translations for `func`,
 * retaining one preferred translation per start. Returns nullopt if no valid
 * portable record can be produced. If non-null, `sourceTransIds` receives the
 * original TransID for each retained translation.
 */
std::optional<ContProfProfileRecord>
snapshotContProfProfileRecord(
  const ProfData&,
  const Func&,
  std::vector<TransID>* sourceTransIds = nullptr,
  ContProfCaptureStats* stats = nullptr
);

}
