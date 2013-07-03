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

#include "hphp/util/service_data.h"

#include <array>
#include <memory>

#include "folly/Conv.h"
#include "folly/MapUtil.h"
#include "hphp/util/base.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace ServiceData {

ExportedTimeSeries::ExportedTimeSeries(
  int numBuckets,
  const std::vector<std::chrono::seconds>& durations,
  const std::vector<StatsType>& exportTypes)
    : m_timeseries(folly::MultiLevelTimeSeries<int64_t>(numBuckets,
                                                        durations.size(),
                                                        &durations[0])),
      m_exportTypes(exportTypes) {
}

void ExportedTimeSeries::exportAll(const std::string& prefix,
                                   std::map<std::string, int64_t>& statsMap) {
  SYNCHRONIZED(m_timeseries) {
    // must first call update to flush data.
    m_timeseries.update(detail::nowAsSeconds());

    for (int i = 0; i < m_timeseries.numLevels(); ++i) {
      auto& level = m_timeseries.getLevel(i);
      std::string suffix =
        level.isAllTime()  ? "" :
        folly::to<std::string>(".", level.duration().count());

      for (auto type : m_exportTypes) {
        if (type == ServiceData::StatsType::AVG) {
          statsMap.insert(
            std::make_pair(folly::to<std::string>(prefix, ".avg", suffix),
                           level.avg()));
        } else if (type == ServiceData::StatsType::SUM) {
          statsMap.insert(
            std::make_pair(folly::to<std::string>(prefix, ".sum", suffix),
                           level.sum()));
        } else if (type == ServiceData::StatsType::RATE) {
          statsMap.insert(
            std::make_pair(folly::to<std::string>(prefix, ".rate", suffix),
                           level.rate()));
        } else if (type == ServiceData::StatsType::COUNT) {
          statsMap.insert(
            std::make_pair(folly::to<std::string>(prefix, ".count", suffix),
                           level.count()));
        } else if (type == ServiceData::StatsType::PCT) {
          statsMap.insert(
            std::make_pair(folly::to<std::string>(prefix, ".pct", suffix),
                           level.avg() * 100));
        }
      }
    }
  }
}

ExportedHistogram::ExportedHistogram(
  int64_t bucketSize,
  int64_t min,
  int64_t max,
  const std::vector<double>& exportPercentiles)
    : m_histogram(folly::Histogram<int64_t>(bucketSize, min, max)),
      m_exportPercentiles(exportPercentiles) {
}

void ExportedHistogram::exportAll(const std::string& prefix,
                                  std::map<std::string, int64_t>& statsMap) {
  SYNCHRONIZED(m_histogram) {
    for (double percentile : m_exportPercentiles) {
      statsMap.insert(
        std::make_pair(
          folly::to<std::string>(
            prefix, ".hist.p", folly::to<int32_t>(percentile * 100)),
          m_histogram.getPercentileEstimate(percentile)));
    }
  }
}

namespace {

class Impl {
 public:
  ExportedCounter* createCounter(const std::string& name) {
    SYNCHRONIZED(m_counterMap) {
      auto iterator = m_counterMap.find(name);
      if (iterator == m_counterMap.end()) {
        return (m_counterMap[name] = new ExportedCounter());
      }
      return iterator->second;
    }
    // make compiler happy.
    return nullptr;
  }

  ExportedTimeSeries* createTimeseries(
    const std::string& name,
    const std::vector<ServiceData::StatsType>& types,
    const std::vector<std::chrono::seconds>& levels,
    int numBuckets) {
    SYNCHRONIZED(m_timeseriesMap) {
      ExportedTimeSeries* counter = nullptr;
      auto iterator = m_timeseriesMap.find(name);
      if (iterator == m_timeseriesMap.end()) {
        counter = new ExportedTimeSeries(numBuckets, levels, types);
        m_timeseriesMap[name] = counter;
      } else {
        counter = iterator->second;
      }

      return counter;
    }
    // make compiler happy.
    return nullptr;
  }

  ExportedHistogram* createHistogram(
    const std::string& name,
    int64_t bucketSize,
    int64_t min,
    int64_t max,
    const std::vector<double>& exportPercentiles) {

    SYNCHRONIZED(m_histogramMap) {
      ExportedHistogram* histogram;
      auto iterator = m_histogramMap.find(name);
      if (iterator == m_histogramMap.end()) {
        histogram = new ExportedHistogram(bucketSize, min, max,
                                          exportPercentiles);
        m_histogramMap[name] = histogram;
      } else {
        histogram = iterator->second;
      }

      return histogram;
    }
    // make compiler happy.
    return nullptr;
  }

  void exportAll(std::map<std::string, int64_t>& statsMap) {
    // make a copy of the counter map so we can't hold the lock on the map while
    // we are exporting individual stats.
    hphp_hash_map<std::string, ExportedCounter*> counters;
    m_counterMap.copy(&counters);
    for (auto iter : counters) {
      statsMap.insert(std::make_pair(iter.first, iter.second->getValue()));
    }

    // Same idea here. Make a copy first to iterate over so we don't hold the
    // lock on the map while we export individual timeseries
    hphp_hash_map<std::string, ExportedTimeSeries*> timeseries;
    m_timeseriesMap.copy(&timeseries);

    for (auto iter : timeseries) {
      iter.second->exportAll(iter.first, statsMap);
    }

    // And same here for histograms.
    hphp_hash_map<std::string, ExportedHistogram*> histograms;
    m_histogramMap.copy(&histograms);

    for (auto iter : histograms) {
      iter.second->exportAll(iter.first, statsMap);
    }
  }

 private:
  // This is a singleton class. Once constructed, we never destroy it. See the
  // implementation note below.
  ~Impl() = delete;

  // Delete all the values from a STL style associative container.
  template <typename Container>
  static void containerDeleteSeconds(Container* container) {
    for (auto iter : *container) {
      delete iter.second;
      iter.second = 0;
    }
  }

  typedef hphp_hash_map<std::string, ExportedCounter*> ExportedCounterMap;
  typedef hphp_hash_map<std::string, ExportedTimeSeries*> ExportedTimeSeriesMap;
  typedef hphp_hash_map<std::string, ExportedHistogram*> ExportedHistogramMap;

  folly::Synchronized<ExportedCounterMap, folly::RWSpinLock> m_counterMap;
  folly::Synchronized<ExportedTimeSeriesMap, folly::RWSpinLock> m_timeseriesMap;
  folly::Synchronized<ExportedHistogramMap, folly::RWSpinLock> m_histogramMap;
};

// Implementation note:
//
// Impl data structure is a singleton and globally accessible. We need to
// initialize it before anyone tries to use it. It is possible and likely that
// another statically initialized object will call methods on it to create
// counters. Therefore, we need Impl to be initialized statically before main()
// starts. Unfortunately, there is no initialization order guarantees for the
// statically and globally constructed objects. To get around that, we wrap the
// initialization in a function so s_impl will get initialized the first time it
// gets called.
//
// For the same reason, we need s_impl to be destructed after all other
// statically created objects may reference it in their destructor. We achieve
// that by *intentionally* creating the object on heap and never delete it. It's
// better to leak memory here than to have random crashes on shutdown.
static Impl& getServiceDataInstance() {
  static Impl *s_impl = new Impl();
  return *s_impl;
}
// One problem with getServiceDataInstance() is that it's not thread safe. If
// two threads are accessing this function for the first time concurrently, we
// might end up creating two Impl object. We work around that by making sure we
// trigger this function statically before main() starts.
//
// Note that it's still possible for the race condition to happen if we are
// creating and starting threads statically before main() starts. If that
// happens, we'll have to wrap getServiceDataInstance around a pthread_once and
// pay some runtime synchronization cost.
UNUSED const Impl& s_dummy = getServiceDataInstance();

}  // namespace

ExportedCounter* createCounter(const std::string& name) {
  return getServiceDataInstance().createCounter(name);
}

ExportedTimeSeries* createTimeseries(
  const std::string& name,
  const std::vector<ServiceData::StatsType>& types,
  const std::vector<std::chrono::seconds>& levels,
  int numBuckets) {
  return getServiceDataInstance().createTimeseries(
    name, types, levels, numBuckets);
}

ExportedHistogram* createHistogram(
  const std::string& name,
  int64_t bucketSize,
  int64_t min,
  int64_t max,
  const std::vector<double>& exportPercentile) {
  return getServiceDataInstance().createHistogram(
    name, bucketSize, min, max, exportPercentile);
}

void exportAll(std::map<std::string, int64_t>& statsMap) {
  return getServiceDataInstance().exportAll(statsMap);
}

}  // namespace ServiceData.

//////////////////////////////////////////////////////////////////////
}
