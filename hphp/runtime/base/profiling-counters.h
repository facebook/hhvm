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

#include <unordered_map>
#include <string>
#include <cstdint>


#include "hphp/util/optional.h"
#include "hphp/util/service-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ProfilingTimeseriesWithOutliers {
  ProfilingTimeseriesWithOutliers(
      const std::string& keyName,
      const std::vector<uint64_t>& thresholdsMs
  );

  void update(const uint64_t valueUs);

private:
  ServiceData::ExportedTimeSeries* mTimeseriesUs;
  std::unordered_map<uint64_t, ServiceData::ExportedTimeSeries*> mOutlierTimeseriesMsMap;
};
      

struct PspAndNonPspProfilingTimeseries {
  PspAndNonPspProfilingTimeseries(
      const std::string& keyBaseName,
      const std::string& totalUsNoteName,
      const std::string& pspUsNoteName,
      const std::vector<uint64_t>& thresholdsMs
  );

  void update();

private:
  std::string mTotalUsNoteName;
  std::string mPspUsNoteName;
  ProfilingTimeseriesWithOutliers mTotalTimeseries;
  ProfilingTimeseriesWithOutliers mNonPspTimeseries;
};

struct ProfilingCounters {
  static void UpdateCountersBasedOnServerNotes() {
    /**
     * STATIC -- this is where the following timeseries are created:
     *   - {BASE}_us.sum.60: total duration from entire request, in us
     *   - {BASE}_non_psp_us.sum.60: duration from only non-PSP, in us
     *   - {BASE}.over_{THRESHOLD}ms.sum.60: count of requests with duration
     *     over THRESHOLD ms
     *   - {BASE}_non_psp.over_{THRESHOLD}ms.sum.60: count of requests with 
     *     duration from non-PSP over THRESHOLD ms
     * NOTE: duration timeseries are in us, while outlier timeseries are in ms
     * for consistency with the old dyno timeseries they're replacing.
     **/
    static std::vector<PspAndNonPspProfilingTimeseries> timeseriesDefinitions = { 
      // Memcache
      PspAndNonPspProfilingTimeseries(
        "memcache.get_duration", // base name
        "DYNO_CACHE_GET_DURATION", // server note for total duration, in us
        "DYNO_CACHE_GET_DURATION_PSP", // server note for PSP duration only, in us
        {100} // count outliers over 100ms
      ),

      // Tao
      PspAndNonPspProfilingTimeseries(
        "tao.dispatch_duration",
        "DYNO_TAO_DISPATCH_DURATION", // total duration in us
        "DYNO_TAO_DISPATCH_DURATION_PSP", // PSP only in us
        {1000} // count outliers over 1000ms
      ),
      PspAndNonPspProfilingTimeseries(
        "tao.update_duration",
        "DYNO_TAO_UPDATE_DURATION",
        "DYNO_TAO_UPDATE_DURATION_PSP", 
        {1000}
      )
    };

    // update the values of all the timeseries
    for (auto& timeseries : timeseriesDefinitions) {
      timeseries.update();
    }
  }

  static Optional<uint64_t> getStatFromServerNote(const std::string& name);
};


///////////////////////////////////////////////////////////////////////////////
}
