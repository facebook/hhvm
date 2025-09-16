/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fb303/ThreadCachedServiceData.h>
#include <fb303/detail/QuantileStatWrappers.h>

DECLARE_bool(basestats_all_time_timeseries);

namespace proxygen {

/*
 * Counter definitions for use in child classes.  Updating all
 * children thus becomes as simple as updating these definitions.
 * It is thus intended and recommended for all callers to refer to
 * BaseStats::<counter> when wishing to use counters.
 */
class BaseStats {
 private:
  // Private constructor so its clear nothing else should implement this class
  BaseStats() = default;

 public:
  // TODO: given the simple nature of TLCounter and that it is explicitly
  // thread safe via the use of atomics, we may only want single local
  // instance instead of wrapped (per thread) instances.
  using TLCounter = facebook::fb303::CounterWrapper;
  // Please avoid adding DynamicTimeseriesWrapper if we can.
  // At a minimum they require formatters and map lookups for
  // operations and make it easier to violate the constraint that all counters
  // are created at startup.
  using TLTimeseriesMinute = facebook::fb303::MinuteOnlyTimeseriesWrapper;
  // TLTimeseries was exporting as TimeseriesPolymorphicWrapper
  // were are trying to get rid of .600/.3600 counters
  // therefore aliasing it to TLTTimeSeriesMinute which only
  // exports .60 counters
  using TLTimeseries = TLTimeseriesMinute;
  // Wrapper which constructs windowed 60s and conditionally all-time timeseries
  // based on gflag. All-time timeseries' only use case is in prospect tests
  // where windowed timeseries cause test flakiness if the proxygend instance is
  // alive for >60s. The underlying timeseries object is constructed on the heap
  // to work around compiler error of the fb303 objects having no default
  // constructor.
  class TLTimeseriesMinuteAndAllTime final {
   public:
    template <typename... Args>
    explicit TLTimeseriesMinuteAndAllTime(std::string name,
                                          const Args&... args) {
      // Static assertion that will fail at compile time if Args is empty
      static_assert(
          sizeof...(Args) > 0,
          "TLTimeseriesMinuteAndAllTime requires at least one argument "
          "after the name, typically the export type (e.g., "
          "facebook::fb303::SUM)");
      if (FLAGS_basestats_all_time_timeseries) {
        impl_ = std::make_unique<facebook::fb303::MinuteTimeseriesWrapper>(
            std::move(name), args...);
      } else {
        impl_ = std::make_unique<facebook::fb303::MinuteOnlyTimeseriesWrapper>(
            std::move(name), args...);
      }
    }
    void add(int64_t value = 1) {
      impl_->add(value);
    }

   private:
    std::unique_ptr<facebook::fb303::TimeseriesWrapperBase> impl_;
  };

  class LazyQuantileStatWrapper {
   public:
    explicit LazyQuantileStatWrapper(
        folly::StringPiece name,
        folly::Range<const facebook::fb303::ExportType*> stats =
            facebook::fb303::ExportTypeConsts::kCountAvg,
        folly::Range<const double*> quantiles =
            facebook::fb303::QuantileConsts::kP95_P99_P999,
        folly::Range<const size_t*> slidingWindowPeriods =
            facebook::fb303::SlidingWindowPeriodConsts::kOneMin) {
      // We convert these to owned datastructures
      std::vector<facebook::fb303::ExportType> statsVec;
      statsVec.reserve(stats.size());
      std::vector<double> quantilesVec;
      quantilesVec.reserve(quantiles.size());
      std::vector<size_t> slidingWindowPeriodsVec;
      slidingWindowPeriodsVec.reserve(slidingWindowPeriods.size());

      for (auto stat : stats) {
        statsVec.push_back(stat);
      }
      for (auto quantile : quantiles) {
        quantilesVec.push_back(quantile);
      }
      for (auto slidingWindowPeriod : slidingWindowPeriods) {
        slidingWindowPeriodsVec.push_back(slidingWindowPeriod);
      }
      statWrapperInfo_ =
          std::make_unique<QuantileStatWrapperInfo>(QuantileStatWrapperInfo{
              .name = name.toString(),
              .stats = std::move(statsVec),
              .quantiles = std::move(quantilesVec),
              .slidingWindowPeriods = std::move(slidingWindowPeriodsVec)});
    }

    void addValue(double value) {
      folly::call_once(initQuantileStatFlag, [&]() {
        statWrapper_ =
            std::make_unique<facebook::fb303::detail::QuantileStatWrapper>(
                statWrapperInfo_->name,
                statWrapperInfo_->stats,
                statWrapperInfo_->quantiles,
                statWrapperInfo_->slidingWindowPeriods);
        statWrapperInfo_.reset();
      });
      statWrapper_->addValue(value);
    }

   protected:
    // A class to store objects needed to create a QuantileStatWrapper object
    // for lazy construction
    struct QuantileStatWrapperInfo {
      std::string name;
      std::vector<facebook::fb303::ExportType> stats;
      std::vector<double> quantiles;
      std::vector<size_t> slidingWindowPeriods;
    };

    folly::once_flag initQuantileStatFlag;
    std::unique_ptr<QuantileStatWrapperInfo> statWrapperInfo_;
    std::unique_ptr<facebook::fb303::detail::QuantileStatWrapper> statWrapper_;
  };

  using TLHistogram = facebook::fb303::MinuteOnlyHistogram;
  using TLHistogramMinuteAndAllTime = facebook::fb303::HistogramWrapper;
  // Please avoid adding DynamicHistogramWrapper if we can.
  // At a minimum they require formatters and map lookups for
  // operations and make it easier to violate the constraint that all counters
  // are created at startup.

  // Following are helpers to add/increment optional BaseStats types
  template <typename StatT>
  static void addToOptionalStat(StatT& tlStat, int64_t value) {
    if (tlStat) {
      tlStat->add(value);
    }
  }
  template <typename StatT>
  static void addValueToOptionalStat(StatT& tlStat, int64_t value) {
    if (tlStat) {
      tlStat->addValue(value);
    }
  }

  template <typename StatT>
  static void incrementOptionalCounter(std::optional<StatT>& tlCounter,
                                       facebook::fb303::CounterType value) {
    if (tlCounter) {
      tlCounter->incrementValue(value);
    }
  }

  static bool isAllTimeTimeseriesEnabled();
};

} // namespace proxygen
