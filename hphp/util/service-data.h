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

#include <atomic>
#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <folly/Synchronized.h>
#include <folly/stats/Histogram.h>
#include <folly/stats/MultiLevelTimeSeries.h>

#include "hphp/util/assertions.h"
#include "hphp/util/optional.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * A globally accessible statistics tracking facility. This can be used to keep
 * track of internal runtime statistics in the form of flat counters, timeseries
 * counters or histograms.
 *
 * ServiceData provides a globally accessible entry point to all the internal
 * statistics. A 'statistic counter' of different types could be created by
 * calling createCouter() createTimeSeries() or createHistogram(). The caller
 * can then add values at different time points to the statistic counters. The
 * statistic can then be retrieved and reported via the exportAll() call on
 * ServiceData.
 *
 * Thread safety:
 * ==============
 * All functions in ServiceData namespace are thread safe. It is safe
 * (and recommended) to cache the object returned by create...() methods and
 * repeatedly add data points to it. It is safe to call create...() with the
 * same name from multiple threads. In this case, only one object will be
 * created and passed back to different threads.
 *
 * All objects returned by returned by the various create...() calls are thread
 * safe. It is okay to add data points to it from multiple threads concurrently.
 * These objects are internally synchronized with spin locks.
 *
 * Example Usage:
 * ==============
 * // create a flat counter named foo.
 * auto counter = ServiceData::createCounter("foo");
 * counter->increment();
 *
 * // create timeseries data named bar with default setting (avg value for the
 * // last 1 minute, 10 minute, hour and all time).
 * auto timeseries = ServiceData::createTimeSeries("bar");
 * timeseries->addValue(3);
 *
 * // create a histogram with 10 buckets, min of 1, max of 100 and export the
 * // 50th and 90th percentile value for reporting.
 * auto histogram = ServiceData::createHistogram("blah", 10, 1, 100,
 *                                               {0.5, 0.9});
 * histogram->addValue(10);
 *
 * // You can report the data like so.
 * std::map<std::string, int64_t> statsMap;
 * ServiceData::exportAll(statsMap);
 *
 * and statsMap will contain these keys:
 *
 * "foo"
 * "bar.avg.60", "bar.avg.600", "bar.avg.3600", "bar.avg"
 * "blah.hist.p50", "blah.hist.p90"
 *
 * Anti pattern:
 * =============
 * ServiceData::createCounter("foo")->increment();  // don't do this.
 * Don't do this in performance critical code. You will incur the cost of a
 * std::map look up whenever createCounter() is called. Rather, you should call
 * ServiceData::createCounter("foo") just once, cache the returned pointer and
 * repeatedly adding data points to it.
 */
namespace ServiceData {

struct ExportedCounter;
struct ExportedHistogram;
struct ExportedTimeSeries;

namespace detail {
template <class ClassWithPrivateDestructor>
struct FriendDeleter;
}

enum class StatsType { AVG, SUM, RATE, COUNT, PCT };

/*
 * Create a flat counter named 'name'. Return an existing counter if it has
 * already been created.
 */
ExportedCounter* createCounter(const std::string& name);

/*
 * Callback-based counters
 *
 * Some counters have values that are updated much more frequently than data is
 * requested from ServiceData, or there may not be a good location in the code
 * to periodically update the value. For these cases, a callback-based counter
 * may be used instead.
 *
 * registerCounterCallback() returns a unique handle that may be passed to
 * deregisterCounterCallback() if the callback should be deregistered.
 *
 * The CounterCallback struct is provided as a convenience wrapper to manage
 * the handle. The callback may be provided to its constructor, or to the
 * init() function, if the callback isn't safe to call at CounterCallback's
 * construction. Similarly, deinit() may be be called if the callback should be
 * deregistered before CounterCallback's destruction.
 *
 * Once registered, callbacks must be safe to call at any time, from any
 * thread, and may even be called before registerCounterCallback()
 * returns. Callbacks are passed a reference to the std::map<std::string,
 * int64_t> being populated, so one callback can add many counters (callbacks
 * can also remove or modify existing counters, but this is discouraged).
 */
using CounterMap = std::map<std::string, int64_t>;
using CounterFunc = std::function<void(CounterMap&)>;
using CounterHandle = uint32_t;

CounterHandle registerCounterCallback(CounterFunc func, bool expensive);
void deregisterCounterCallback(CounterHandle key);

template<bool expensive>
struct CounterCallbackBase {
 CounterCallbackBase() = default;

 explicit CounterCallbackBase(CounterFunc func) {
    init(std::move(func));
  }

  ~CounterCallbackBase() {
    deinit();
  }

  void init(CounterFunc func) {
    assertx(!m_key);
    m_key = registerCounterCallback(std::move(func), expensive);
  }

  void deinit() {
    if (m_key) {
      deregisterCounterCallback(*m_key);
      m_key = std::nullopt;
    }
  }

private:
  Optional<CounterHandle> m_key;
};

using CounterCallback = CounterCallbackBase<false>;
using ExpensiveCounterCallback = CounterCallbackBase<true>;

/*
 * Create a timeseries counter named 'name'. Return an existing one if it
 * has already been created.
 *
 * Timeseries data is implemented as a number of buckets (bucketed by time).
 * As data point is added and time rolls forward, new bucket is created and
 * the earliest bucket expires.
 *
 * We keep multiple of timeseries data at different granularity and update
 * them simultaneously. This allows us to commute statistics at different
 * levels. For example, we can simultaneously compute the avg of some counter
 * value over the last 5 minutes, 10 minutes and hour. This is a similar
 * concept to the load counters from the unix 'uptime' command.
 *
 * 'exportTypes' specifies what kind of statistics to export for each level.
 * More export types can be added after the timeseries is created.
 *
 * 'levels' specifies at which granularity should the stats be tracked. The
 * time duration must be strictly increasing. Special value '0' means all
 * time and should always come last.
 *
 * 'numBuckets' specifies how many buckets to keep at each level. More buckets
 * will produce more precise data at the expense of memory.
 */
ExportedTimeSeries* createTimeSeries(
  const std::string& name,
  const std::vector<StatsType>& exportTypes =
  std::vector<StatsType>{ StatsType::AVG },
  const std::vector<std::chrono::seconds>& levels =
  std::vector<std::chrono::seconds>{
    std::chrono::seconds(60),
    std::chrono::seconds(600),
    std::chrono::seconds(3600),
    std::chrono::seconds(0) /* all time */ },
  int numBuckets = 60);

/*
 * Create a histogram counter named 'name'. Return an existing one if it has
 * already been created.
 *
 * 'bucketSize' specifies how many buckets to track for the histogram.
 * 'min' is the minimal value in the histogram.
 * 'max' is the maximal value in the histogram.
 * 'exportPercentile' specifies at what percentile values we should report the
 * stat. A set of doubles between 0 and 1.0. For example, 0.5 means p50 and
 * 0.99 means p99.
 */
ExportedHistogram* createHistogram(
  const std::string& name,
  int64_t bucketSize,
  int64_t min,
  int64_t max,
  const std::vector<double>& exportPercentile);

/*
 * Export all the statistics as simple key, value pairs.
 */
void exportAll(CounterMap& statsMap);

/*
 * Export the selected statistics as simple key, value pairs.
 */
void exportSelectedCountersByKeys(
  CounterMap& statsMap, const std::vector<std::string>& keys);

/*
 * Export a specific counter by key name.
 */
Optional<int64_t> exportCounterByKey(const std::string& key);

// Interface for a flat counter. All methods are thread safe.
struct ExportedCounter {
  ExportedCounter() : m_value(0) {}
  void increment() { m_value.fetch_add(1, std::memory_order_relaxed); }
  void decrement() { m_value.fetch_sub(1, std::memory_order_relaxed); }
  void addValue(int64_t value) {
    m_value.fetch_add(value, std::memory_order_relaxed);
  }
  void setValue(int64_t value) {
    m_value.store(value, std::memory_order_relaxed);
  }
  int64_t getValue() const { return m_value.load(std::memory_order_relaxed); }

 private:
  friend struct detail::FriendDeleter<ExportedCounter>;
  ~ExportedCounter() {}

  std::atomic_int_fast64_t m_value;
};

// Interface for timeseries data. All methods are thread safe.
struct ExportedTimeSeries {
  ExportedTimeSeries(int numBuckets,
                     const std::vector<std::chrono::seconds>& durations,
                     const std::vector<StatsType>& exportTypes);

  void addValue(int64_t value);
  void addValue(int64_t value, int64_t times);
  void addValueAggregated(int64_t sum, int64_t nsamples);

  int64_t getSum();
  int64_t getRateByDuration(std::chrono::seconds duration);

  Optional<int64_t> getCounter(StatsType type, int seconds);

  void exportAll(const std::string& prefix, CounterMap& statsMap);

 private:
  friend struct detail::FriendDeleter<ExportedTimeSeries>;
  ~ExportedTimeSeries() {}

  folly::Synchronized<folly::MultiLevelTimeSeries<int64_t>> m_timeseries;
  const std::vector<ServiceData::StatsType> m_exportTypes;
};

// Interface for histogram data. All methods are thread safe.
struct ExportedHistogram {
  ExportedHistogram(int64_t bucketSize, int64_t min, int64_t max,
                    const std::vector<double>& exportPercentiles);
  void addValue(int64_t value);
  void removeValue(int64_t value);
  void exportAll(const std::string& prefix, CounterMap& statsMap);

 private:
  friend struct detail::FriendDeleter<ExportedHistogram>;
  ~ExportedHistogram() {}

  folly::Synchronized<folly::Histogram<int64_t>> m_histogram;
  const std::vector<double> m_exportPercentiles;
};

}  // namespace ServiceData

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/util/service-data-inl.h"
