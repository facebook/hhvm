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

  void update(const uint64_t valueUs, const uint64_t num_samples);

private:
  ServiceData::ExportedTimeSeries* mTimeseriesUs;
  std::unordered_map<uint64_t, ServiceData::ExportedTimeSeries*> mOutlierTimeseriesMsMap;
};
      

struct PspAndNonPspProfilingTimeseries {
  PspAndNonPspProfilingTimeseries(
      const std::string& keyBaseName,
      const std::string& totalUsNoteName,
      const std::string& totalCountNoteName,
      const std::string& pspUsNoteName,
      const std::string& pspCountNoteName,
      const std::vector<uint64_t>& thresholdsMs
  );

  void update();

private:
  std::string mTotalUsNoteName;
  std::string mTotalCountNoteName;
  std::string mPspUsNoteName;
  std::string mPspCountNoteName;
  ProfilingTimeseriesWithOutliers mTotalTimeseries;
  ProfilingTimeseriesWithOutliers mNonPspTimeseries;
};

struct ProfilingCounters {
  static void UpdateCountersBasedOnServerNotes() {
    /**
     * STATIC -- this is where the following timeseries are created:
     *   - {BASE}_us.sum.60: total duration from entire request, in us
     *   - {BASE}_non_psp_us.sum.60: duration from only non-PSP, in us
     *   - {BASE}.over_{THRESHOLD}ms.count.60: count of www requests with duration 
     *       of all {BASE} calls in a single request over {THRESHOLD} ms
     *   - {BASE}_non_psp.over_{THRESHOLD}ms.count.60: count of requests with 
     *     duration from non-PSP over THRESHOLD ms
     * NOTE: duration timeseries are in us, while outlier timeseries are in ms
     * for consistency with the old dyno timeseries they're replacing.
     * 
     * Example outlier:  hhvm.tao.dispatch_duration.over_1000_ms.count
     * if a single www request has 3 tao calls of 500ms, 600 ms, 700 ms, then
     * hhvm.tao.dispatch_duration_us.avg sum that request will be 1_800_000 us
     * hhvm.tao.dispatch_duration_us.count for that request will be 3
     * hhvm.tao.dispatch_duration_us.avg for that request will be 600_000 us
     * total time will be 1_800 ms so the outlier counter will be bumped by 1
     **/
    static std::vector<PspAndNonPspProfilingTimeseries> timeseriesDefinitions = { 
      // Memcache
      PspAndNonPspProfilingTimeseries(
        "memcache.get_duration", // base name
        "DYNO_CACHE_GET_DURATION", // server note for total duration, in us
        "DYNO_CACHE_GET_COUNT", // server note for count
        "DYNO_CACHE_GET_DURATION_PSP", // server note for PSP duration only, in us
        "DYNO_CACHE_GET_COUNT_PSP", // server note for PSP count
        {100} // count outliers over 100ms
      ),
      PspAndNonPspProfilingTimeseries(
        "memcache.arith_duration",
        "DYNO_CACHE_ARITH_DURATION",
        "DYNO_CACHE_ARITH_COUNT",
        "DYNO_CACHE_ARITH_DURATION_PSP",
        "DYNO_CACHE_ARITH_COUNT_PSP",
        {100}
      ),
      PspAndNonPspProfilingTimeseries(
        "memcache.delete_duration",
        "DYNO_CACHE_DELETE_DURATION",
        "DYNO_CACHE_DELETE_COUNT",
        "DYNO_CACHE_DELETE_DURATION_PSP",
        "DYNO_CACHE_DELETE_COUNT_PSP",
        {100}
      ),
      PspAndNonPspProfilingTimeseries(
        "memcache.update_duration",
        "DYNO_CACHE_UPDATE_DURATION",
        "DYNO_CACHE_UPDATE_COUNT",
        "DYNO_CACHE_UPDATE_DURATION_PSP",
        "DYNO_CACHE_UPDATE_COUNT_PSP",
        {100}
      ),
      PspAndNonPspProfilingTimeseries(
        "memcache.warm_duration",
        "DYNO_CACHE_WARM_DURATION",
        "DYNO_CACHE_WARM_COUNT",
        "DYNO_CACHE_WARM_DURATION_PSP",
        "DYNO_CACHE_WARM_COUNT_PSP",
        {100}
      ),

      // Tao
      PspAndNonPspProfilingTimeseries(
        "tao.dispatch_duration",
        "DYNO_TAO_DISPATCH_DURATION", 
        "DYNO_TAO_DISPATCH_COUNT",
        "DYNO_TAO_DISPATCH_DURATION_PSP", 
        "DYNO_TAO_DISPATCH_COUNT_PSP",
        {1000} 
      ),
      PspAndNonPspProfilingTimeseries(
        "tao.update_duration",
        "DYNO_TAO_UPDATE_DURATION",
        "DYNO_TAO_UPDATE_COUNT",
        "DYNO_TAO_UPDATE_DURATION_PSP", 
        "DYNO_TAO_UPDATE_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "tao.delete_duration",
        "DYNO_TAO_DELETE_DURATION",
        "DYNO_TAO_DELETE_COUNT",
        "DYNO_TAO_DELETE_DURATION_PSP",
        "DYNO_TAO_DELETE_COUNT_PSP",
        {1000}
      ),
      
      // Database queries
      PspAndNonPspProfilingTimeseries(
        "db_query.read_duration",
        "DYNO_QUERY_READ_DURATION",
        "DYNO_QUERY_READ_COUNT",
        "DYNO_QUERY_READ_DURATION_PSP",
        "DYNO_QUERY_READ_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "db_query.write_duration",
        "DYNO_QUERY_WRITE_DURATION",
        "DYNO_QUERY_WRITE_COUNT",
        "DYNO_QUERY_WRITE_DURATION_PSP",
        "DYNO_QUERY_WRITE_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "db_query.other_duration",
        "DYNO_QUERY_OTHER_DURATION",
        "DYNO_QUERY_OTHER_COUNT",
        "DYNO_QUERY_OTHER_DURATION_PSP",
        "DYNO_QUERY_OTHER_COUNT_PSP",
        {1000}
      ),
      
      // Curl operations
      PspAndNonPspProfilingTimeseries(
        "curl.get_duration",
        "DYNO_CURL_GET_DURATION",
        "DYNO_CURL_GET_COUNT",
        "DYNO_CURL_GET_DURATION_PSP",
        "DYNO_CURL_GET_COUNT_PSP",
        {1000}
      ),
      
      // Thrift operations
      // NB: These are called thrift_calls to disambiguate with thrift counters
      // already existing
      PspAndNonPspProfilingTimeseries(
        "thrift_calls.flush_duration",
        "DYNO_THRIFT_FLUSH_DURATION",
        "DYNO_THRIFT_FLUSH_COUNT",
        "DYNO_THRIFT_FLUSH_DURATION_PSP",
        "DYNO_THRIFT_FLUSH_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "thrift_calls.open_duration",
        "DYNO_THRIFT_OPEN_DURATION",
        "DYNO_THRIFT_OPEN_COUNT",
        "DYNO_THRIFT_OPEN_DURATION_PSP",
        "DYNO_THRIFT_OPEN_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "thrift_calls.read_duration",
        "DYNO_THRIFT_READ_DURATION",
        "DYNO_THRIFT_READ_COUNT",
        "DYNO_THRIFT_READ_DURATION_PSP",
        "DYNO_THRIFT_READ_COUNT_PSP",
        {1000}
      ),
      PspAndNonPspProfilingTimeseries(
        "thrift_calls.write_duration",
        "DYNO_THRIFT_WRITE_DURATION",
        "DYNO_THRIFT_WRITE_COUNT",
        "DYNO_THRIFT_WRITE_DURATION_PSP",
        "DYNO_THRIFT_WRITE_COUNT_PSP",
        {1000}
      ),
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
