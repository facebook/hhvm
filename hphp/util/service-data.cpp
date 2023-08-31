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
#include <vector>
#include <tbb/concurrent_unordered_map.h>

#include <folly/Conv.h>
#include <folly/MapUtil.h>
#include <folly/Random.h>
#include <folly/stats/Histogram.h>

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

namespace detail {
template <class ClassWithPrivateDestructor>
struct FriendDeleter {
  template <class... Args>
  explicit FriendDeleter(Args&&... args)
      : m_instance(new ClassWithPrivateDestructor(
                     std::forward<Args>(args)...)) {}
  ~FriendDeleter() { delete m_instance; }

  ClassWithPrivateDestructor* get() const { return m_instance; }
  ClassWithPrivateDestructor* release() {
    auto r = m_instance;
    m_instance = nullptr;
    return r;
  }

 private:
  ClassWithPrivateDestructor* m_instance;
};
} // namespace detail

namespace {

// Find 'key' in concurrent_unordered_map 'map'. Return true iff the key is
// found.
template<class Key, class Value>
bool concurrentMapGet(const tbb::concurrent_unordered_map<Key, Value>& map,
                      const Key& key,
                      Value& value) {
  auto iterator = map.find(key);
  if (iterator != map.end()) {
    value = iterator->second;
    return true;
  }
  return false;
}

// Find or insert 'key' into concurrent_unordered_map 'map'.
//
// Return the value pointer from 'map' if it exists. Otherwise, insert it into
// the map by creating a new object on the heap using the supplied arguments.
//
// Note that this function could be called concurrently. If the insertion to
// 'map' is successful, we release the ownership of value object from
// valuePtr. If the key is already in the map because someone else beat us to
// the insertion, we will return the existing value and delete the object we
// created.
//
template <class Key, class Value, class... Args>
Value* getOrCreateWithArgs(tbb::concurrent_unordered_map<Key, Value*>& map,
                           const Key& key,
                           Args&&... args) {
  // Optimistic case: the object might already be created. Do a simple look
  // up.
  Value* ret = nullptr;
  if (concurrentMapGet(map, key, ret)) {
    return ret;
  }

  // We didn't find an existing value for the key. Create it. Hold the new
  // object in a deleter and release it later if the insert is successful.
  detail::FriendDeleter<Value> deleter(std::forward<Args>(args)...);

  auto result = map.insert(std::make_pair(key, deleter.get()));
  if (result.second) {
    // insert successfully. release the memory.
    deleter.release();
  } else {
    // key is already inserted. This can happen if two threads were racing
    // to create the counter. In this case, nothing further needs to be done.
    // valuePtr's object will get destroyed when we go out of scope.
  }
  return result.first->second;
}

struct Impl {
  ExportedCounter* createCounter(const std::string& name) {
    return getOrCreateWithArgs(m_counterMap, name);
  }

  CounterHandle registerCounterCallback(CounterFunc func, bool expensive) {
    auto handle = folly::Random::rand32();
    SYNCHRONIZED(m_counterFuncs) {
      while (m_counterFuncs.count(handle)) ++handle;
      m_counterFuncs.emplace(handle, std::make_pair(std::move(func), expensive));
    }
    return handle;
  }

  void deregisterCounterCallback(CounterHandle key) {
    SYNCHRONIZED(m_counterFuncs) {
      assertx(m_counterFuncs.count(key) == 1);
      m_counterFuncs.erase(key);
    }
  }

  ExportedTimeSeries* createTimeSeries(
      const std::string& name,
      const std::vector<ServiceData::StatsType>& types,
      const std::vector<std::chrono::seconds>& levels,
      int numBuckets) {
    return getOrCreateWithArgs(
      m_timeseriesMap, name, numBuckets, levels, types);
  }

  ExportedHistogram* createHistogram(
      const std::string& name,
      int64_t bucketSize,
      int64_t min,
      int64_t max,
      const std::vector<double>& exportPercentiles) {
    return getOrCreateWithArgs(
      m_histogramMap, name, bucketSize, min, max, exportPercentiles);
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

    SYNCHRONIZED_CONST(m_counterFuncs) {
      for (auto& pair : m_counterFuncs) {
        // we don't care if it's expensive because we're exporting everything
        pair.second.first(statsMap);
      }
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
          for (auto& pair : m_counterFuncs) {
            // only actually compute the "cheap" ones immediately
            if (!pair.second.second) {
              pair.second.first(counterFuncMap);
            } else {
              // grab a copy so we don't need to resynchronize.
              // if someone deregisters a counter while we're doing this
              // that's already a race so being opinionated isn't a big deal
              expensiveCounterFuncs.push_back(pair.second.first);
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

        counterFuncMap.merge(results);
        ++nextExpensiveCounterFunc;

        // checking iter against results.end() after a merge operation seems
        // sus, so using a bool to be safe
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

    // Check cheap callbacks
    CounterMap statsMap;
    std::vector<CounterFunc> expensiveCounterFuncs;
    SYNCHRONIZED_CONST(m_counterFuncs) {
      for (auto& pair : m_counterFuncs) {
        // only run the "cheap" ones initially
        if (pair.second.second) {
          expensiveCounterFuncs.push_back(pair.second.first);
        } else {
          pair.second.first(statsMap);
        }
      }
    }
    auto iter = statsMap.find(key);
    if (iter != statsMap.end()) return iter->second;

    // See if it's a time series value
    if (auto ts_val = ExportTimeSeriesCounterForKey(key)) return ts_val;

    // check expensive callbacks
    for (const auto& cb: expensiveCounterFuncs) {
      // run one at a time and check for result so we can avoid running
      // unecessary ones if we hit the key early.
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

  Optional<int64_t> ExportTimeSeriesCounterForKey(const std::string& key) {
    auto const data = key.c_str();
    ServiceData::StatsType type = ServiceData::StatsType::AVG;
    int duration = 0;
    size_t index = key.size() - 1;
    while (isdigit(data[index])) {
      if (index == 0) return std::nullopt;
      --index;
    }
    if (data[index] == '.') {
      sscanf(data + index + 1, "%d", &duration);
      if (index == 0) return std::nullopt;
      --index;
    }
    // Find the StatsType from: avg, sum, pct, rate, count
    auto const typeEnd = index;
    while (index > 0 && data[index] != '.') --index;
    if (index == 0) return std::nullopt;
    if (typeEnd - index == 3) {
      if (!memcmp(data + index, ".avg", 4)) {
        type = ServiceData::StatsType::AVG;
      } else if (!memcmp(data + index, ".sum", 4)) {
        type = ServiceData::StatsType::SUM;
      } else if (!memcmp(data + index, ".pct", 4)) {
        type = ServiceData::StatsType::PCT;
      } else {
        return std::nullopt;
      }
    } else if (typeEnd - index == 4) {
      if (!memcmp(data + index, ".rate", 5)) {
        type = ServiceData::StatsType::RATE;
      } else {
        return std::nullopt;
      }
    } else if (typeEnd - index == 5) {
      if (!memcmp(data + index, ".count", 6)) {
        type = ServiceData::StatsType::COUNT;
      } else {
        return std::nullopt;
      }
    }
    auto const tsName = key.substr(0, index);
    auto const tsIter = m_timeseriesMap.find(tsName);
    if (tsIter == m_timeseriesMap.end()) return std::nullopt;
    auto const ts = tsIter->second;
    return ts->getCounter(type, duration);
  }

  using ExportedCounterMap = tbb::concurrent_unordered_map<std::string, ExportedCounter*>;
  using CounterFuncMap = std::unordered_map<CounterHandle, std::pair<CounterFunc, bool>>;
  using ExportedTimeSeriesMap = tbb::concurrent_unordered_map<std::string, ExportedTimeSeries*>;
  using ExportedHistogramMap = tbb::concurrent_unordered_map<std::string, ExportedHistogram*>;

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

CounterHandle registerCounterCallback(CounterFunc func, bool expensive) {
  return getServiceDataInstance().registerCounterCallback(std::move(func), expensive);
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
