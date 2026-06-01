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

#include "hphp/util/service-data.h"

#include <array>
#include <memory>
#include <string_view>
#include <vector>

#include <folly/Conv.h>
#include <folly/MapUtil.h>
#include <folly/container/HeterogeneousAccess.h>
#include <folly/Random.h>
#include <folly/stats/Histogram.h>

#include "hphp/util/hash-map.h"
#include "hphp/util/portability.h"

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
                                   CounterMap& statsMap) {
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

Optional<int64_t>
ExportedTimeSeries::getCounter(StatsType type, int seconds) {
  SYNCHRONIZED(m_timeseries) {
    m_timeseries.update(detail::nowAsSeconds());
    for (unsigned i = 0; i < m_timeseries.numLevels(); ++i) {
      auto& level = m_timeseries.getLevel(i);
      if ((level.isAllTime() && seconds <= 0) ||
          level.duration().count() == seconds) {
        switch (type) {
          case StatsType::AVG:   return level.avg();
          case StatsType::SUM:   return level.sum();
          case StatsType::RATE:  return level.rate();
          case StatsType::COUNT: return level.count();
          case StatsType::PCT:   return level.avg() * 100;
        }
      }
    }
  }
  return std::nullopt;
}

int64_t ExportedTimeSeries::getSum() {
  int64_t sum = 0;
  SYNCHRONIZED(m_timeseries) {
    m_timeseries.update(detail::nowAsSeconds());

    for (int i = 0; i < m_timeseries.numLevels(); ++i) {
      auto& level = m_timeseries.getLevel(i);
      if (level.isAllTime()) {
        sum = m_timeseries.sum(i);
        break;
      }
      sum += m_timeseries.sum(i);
    }
  }
  return sum;
}

int64_t ExportedTimeSeries::getRateByDuration(std::chrono::seconds duration) {
  int64_t rate = 0;
  SYNCHRONIZED(m_timeseries) {
    m_timeseries.update(detail::nowAsSeconds());
    rate = m_timeseries.rate(duration);
  }
  return rate;
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
                                  CounterMap& statsMap) {
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

struct Impl {
  ExportedCounter* getCounterIfExists(const std::string& name) {
    auto it = m_counterMap.find(name);
    if (it != m_counterMap.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  ExportedCounter* createCounter(const std::string& name) {
    auto [it, inserted] = m_counterMap.try_emplace(
      name, std::make_unique<ExportedCounter>());
    return it->second.get();
  }

  CounterHandle registerCounterCallback(
      CounterFunc func, bool expensive, std::string prefix) {
    auto handle = folly::Random::rand32();
    SYNCHRONIZED(m_counterFuncs) {
      while (m_counterFuncs.contains(handle)) ++handle;
      m_counterFuncs.emplace(
        handle,
        CallbackEntry{std::move(func), expensive, std::move(prefix)});
    }
    return handle;
  }

  void deregisterCounterCallback(CounterHandle key) {
    SYNCHRONIZED(m_counterFuncs) {
      assertx(m_counterFuncs.contains(key));
      m_counterFuncs.erase(key);
    }
  }

  ExportedTimeSeries* createTimeSeries(
      const std::string& name,
      const std::vector<ServiceData::StatsType>& types,
      const std::vector<std::chrono::seconds>& levels,
      int numBuckets) {
    auto [it, inserted] = m_timeseriesMap.try_emplace(
      name, std::make_unique<ExportedTimeSeries>(numBuckets, levels, types));
    return it->second.get();
  }

  ExportedHistogram* createHistogram(
      const std::string& name,
      int64_t bucketSize,
      int64_t min,
      int64_t max,
      const std::vector<double>& exportPercentiles) {
    auto [it, inserted] = m_histogramMap.try_emplace(
      name,
      std::make_unique<ExportedHistogram>(
        bucketSize, min, max, exportPercentiles));
    return it->second.get();
  }

  void exportAll(CounterMap& statsMap) {
    for (auto& counter : m_counterMap) {
      statsMap.emplace(counter.first, counter.second->getValue());
    }

    for (auto& ts : m_timeseriesMap) {
      ts.second->exportAll(ts.first, statsMap);
    }

    for (auto& histogram : m_histogramMap) {
      histogram.second->exportAll(histogram.first, statsMap);
    }

    std::vector<CounterFunc> expensiveCallbacks;
    SYNCHRONIZED_CONST(m_counterFuncs) {
      expensiveCallbacks.reserve(m_counterFuncs.size());
      for (auto& [handle, entry] : m_counterFuncs) {
        if (entry.expensive) {
          expensiveCallbacks.push_back(entry.func);
        } else {
          entry.func(statsMap);
        }
      }
    }
    // exportAll still runs expensive callbacks, just not while holding the lock.
    for (auto& cb : expensiveCallbacks) {
      cb(statsMap);
    }
  }

  void exportSelectedCountersByKeys(
    CounterMap& statsMap, const std::vector<std::string>& keys) {

    CounterMap counterFuncMap;
    std::vector<CounterFunc> expensiveCounterFuncs;
    // for tracking which we've already merged
    int nextExpensiveCounterFunc = 0;

    for (const auto& key: keys) {
      if (key.empty()) continue;

      auto const counterIter = m_counterMap.find(key);
      if (counterIter != m_counterMap.end()) {
        statsMap[key] = counterIter->second->getValue();
        continue;
      }

      // build the counterFuncsMap and expensiveCounterFuncs if we haven't already
      if (counterFuncMap.empty()) {
        SYNCHRONIZED_CONST(m_counterFuncs) {
          for (auto& [handle, entry] : m_counterFuncs) {
            // only actually compute the "cheap" ones immediately
            if (!entry.expensive) {
              entry.func(counterFuncMap);
            } else {
              // grab a copy so we don't need to resynchronize.
              // if someone deregisters a counter while we're doing this
              // that's already a race so being opinionated isn't a big deal
              expensiveCounterFuncs.push_back(entry.func);
            }
          }
        }
      }

      // Check if the key was generated by a callback
      auto iter = counterFuncMap.find(key);
      if (iter != counterFuncMap.end()) {
        statsMap.insert(*iter);
        continue;
      }

      // check if it's a time series value
      if (auto ts_val = ExportTimeSeriesCounterForKey(key)) {
        statsMap[key] = *ts_val;
        continue;
      }

      while (nextExpensiveCounterFunc != expensiveCounterFuncs.size()) {
        CounterMap results;
        expensiveCounterFuncs[nextExpensiveCounterFunc](results);

        bool found = false;
        iter = results.find(key);
        if (iter != results.end()) {
          found = true;
          statsMap[key] = iter->second;
        }

        counterFuncMap.insert(results.begin(), results.end());
        ++nextExpensiveCounterFunc;

        // using a bool because iter may be invalidated by the insert
        if (found) break;
      }
    }
  }

  Optional<int64_t> exportCounterByKey(const std::string& key) {
    if (key.empty()) return std::nullopt;
    auto const counterIter = m_counterMap.find(key);
    if (counterIter != m_counterMap.end()) {
      return counterIter->second->getValue();
    }

    // Check cheap callbacks, skipping those whose prefix doesn't match.
    CounterMap statsMap;
    std::vector<CounterFunc> expensiveCounterFuncs;
    SYNCHRONIZED_CONST(m_counterFuncs) {
      for (auto& [handle, entry] : m_counterFuncs) {
        if (!entry.prefix.empty() && !key.starts_with(entry.prefix)) {
          continue;
        }
        if (entry.expensive) {
          expensiveCounterFuncs.push_back(entry.func);
        } else {
          entry.func(statsMap);
        }
      }
    }
    auto iter = statsMap.find(key);
    if (iter != statsMap.end()) return iter->second;

    // See if it's a time series value
    if (auto ts_val = ExportTimeSeriesCounterForKey(key)) return ts_val;

    // Check expensive callbacks that matched the prefix.
    for (const auto& cb: expensiveCounterFuncs) {
      statsMap.clear();
      cb(statsMap);
      iter = statsMap.find(key);
      if (iter != statsMap.end()) return iter->second;
    }

    return std::nullopt;
  }

 private:
  // This is a singleton class. Once constructed, we never destroy it. See the
  // implementation note below.
  ~Impl() = delete;

  // Parses timeseries keys of the form <name>.<type>[.<duration>]
  // where type ∈ {avg, sum, pct, rate, count} and duration is an integer
  // (omitted for all-time counters, i.e. duration 0).
  Optional<int64_t> ExportTimeSeriesCounterForKey(const std::string& key) {
    auto sv = std::string_view(key);
    auto lastDot = sv.rfind('.');
    if (lastDot == sv.npos || lastDot == 0) {
      return std::nullopt;
    }

    auto tail = sv.substr(lastDot + 1);
    int duration = 0;

    // If the trailing segment is numeric, it's the duration — peel it off
    // to expose the type segment underneath.
    if (auto d = folly::tryTo<int>(tail)) {
      duration = *d;
      sv = sv.substr(0, lastDot);
      lastDot = sv.rfind('.');
      if (lastDot == sv.npos || lastDot == 0) {
        return std::nullopt;
      }
      tail = sv.substr(lastDot + 1);
    }

    StatsType type;
    if (tail == "avg") {
      type = StatsType::AVG;
    } else if (tail == "sum") {
      type = StatsType::SUM;
    } else if (tail == "pct") {
      type = StatsType::PCT;
    } else if (tail == "rate") {
      type = StatsType::RATE;
    } else if (tail == "count") {
      type = StatsType::COUNT;
    } else {
      return std::nullopt;
    }

    auto tsIter = m_timeseriesMap.find(sv.substr(0, lastDot));
    if (tsIter == m_timeseriesMap.end()) {
      return std::nullopt;
    }
    return tsIter->second->getCounter(type, duration);
  }

  struct CallbackEntry {
    CounterFunc func;
    bool expensive;
    std::string prefix;
  };
  using CounterFuncMap = folly::F14FastMap<CounterHandle, CallbackEntry>;
  using ExportedCounterMap = folly_concurrent_hash_map_simd<
      std::string, std::unique_ptr<ExportedCounter>>;
  using ExportedTimeSeriesMap = folly_concurrent_hash_map_simd<
      std::string, std::unique_ptr<ExportedTimeSeries>,
      folly::HeterogeneousAccessHash<std::string>,
      folly::HeterogeneousAccessEqualTo<std::string>>;
  using ExportedHistogramMap = folly_concurrent_hash_map_simd<
      std::string, std::unique_ptr<ExportedHistogram>>;

  ExportedCounterMap m_counterMap;
  folly::Synchronized<CounterFuncMap> m_counterFuncs;
  ExportedTimeSeriesMap m_timeseriesMap;
  ExportedHistogramMap m_histogramMap;
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

ExportedCounter* getCounterIfExists(const std::string& name) {
  return getServiceDataInstance().getCounterIfExists(name);
}

CounterHandle registerCounterCallback(
    CounterFunc func, bool expensive, std::string prefix) {
  return getServiceDataInstance().registerCounterCallback(
    std::move(func), expensive, std::move(prefix));
}

void deregisterCounterCallback(CounterHandle key) {
  getServiceDataInstance().deregisterCounterCallback(key);
}

ExportedTimeSeries* createTimeSeries(
    const std::string& name,
    const std::vector<ServiceData::StatsType>& types,
    const std::vector<std::chrono::seconds>& levels,
    int numBuckets) {
  return getServiceDataInstance().createTimeSeries(
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

void exportAll(CounterMap& statsMap) {
  return getServiceDataInstance().exportAll(statsMap);
}

Optional<int64_t> exportCounterByKey(const std::string& key) {
  return getServiceDataInstance().exportCounterByKey(key);
}

void exportSelectedCountersByKeys(
  CounterMap& statsMap, const std::vector<std::string>& keys) {
  return getServiceDataInstance().exportSelectedCountersByKeys(statsMap,keys);
}

}  // namespace ServiceData.

//////////////////////////////////////////////////////////////////////
}
