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
#include <folly/Conv.h>
#include <vector>

#include "hphp/runtime/base/profiling-counters.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/server/server-note.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* static */ Optional<uint64_t> 
ProfilingCounters::getStatFromServerNote(const std::string& name) {
  String note = ServerNote::Get(name);
  if (note.isNull()) {
    return std::nullopt;
  }
  return folly::to<uint64_t>(note.data());
}

ProfilingTimeseriesWithOutliers::ProfilingTimeseriesWithOutliers(
    const std::string& keyName,
    const std::vector<uint64_t>& thresholdsMs
) : mTimeseriesUs(ServiceData::createTimeSeries(
      keyName + "_us",
      {ServiceData::StatsType::SUM, ServiceData::StatsType::COUNT, ServiceData::StatsType::AVG},
      {std::chrono::seconds(60)}
    )) {
  for (const auto& thresholdMs : thresholdsMs) {
    std::stringstream ss;
    ss << keyName << ".over_" << thresholdMs << "_ms";
    mOutlierTimeseriesMsMap[thresholdMs] = ServiceData::createTimeSeries(
      ss.str(),
      {ServiceData::StatsType::COUNT},
      {std::chrono::seconds(60)}
    );
  }
}

void ProfilingTimeseriesWithOutliers::update(uint64_t valueUs, uint64_t num_samples) {
  mTimeseriesUs->addValueAggregated(valueUs, num_samples);
  for (const auto& entryMs : mOutlierTimeseriesMsMap) {
    if (valueUs > entryMs.first * 1000) {
      entryMs.second->addValue(1);
    }
  }
}

PspAndNonPspProfilingTimeseries::PspAndNonPspProfilingTimeseries(
    const std::string& keyBaseName,
    const std::string& totalUsNoteName,
    const std::string& totalCountNoteName,
    const std::string& pspUsNoteName,
    const std::string& pspCountNoteName,
    const std::vector<uint64_t>& thresholdsMs
) : mTotalUsNoteName(totalUsNoteName)
  , mTotalCountNoteName(totalCountNoteName)
  , mPspUsNoteName(pspUsNoteName)
  , mPspCountNoteName(pspCountNoteName)
  , mTotalTimeseries(ProfilingTimeseriesWithOutliers(keyBaseName, thresholdsMs))
  , mNonPspTimeseries(ProfilingTimeseriesWithOutliers(keyBaseName + "_non_psp", thresholdsMs)) {}

void PspAndNonPspProfilingTimeseries::update() {
  const auto totalValueUs = ProfilingCounters::getStatFromServerNote(mTotalUsNoteName);
  const auto totalCount = ProfilingCounters::getStatFromServerNote(mTotalCountNoteName);
  const auto pspValueUs = ProfilingCounters::getStatFromServerNote(mPspUsNoteName);
  const auto pspCount = ProfilingCounters::getStatFromServerNote(mPspCountNoteName);
  mTotalTimeseries.update(totalValueUs.value_or(0), totalCount.value_or(0));
  mNonPspTimeseries.update(totalValueUs.value_or(0) - pspValueUs.value_or(0), totalCount.value_or(0) - pspCount.value_or(0));
}
///////////////////////////////////////////////////////////////////////////////
}
