/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>

#include <folly/Range.h>
#include <folly/json/dynamic.h>

namespace carbon {

enum class RouterStatTypes : uint8_t {
  // Incoming requests seen by Proxy.
  Incoming = 0,
  // Normal requests sent out by CarbonRouter. Does not include failover or
  // shadow requests (see below).
  Outgoing = 1,
  // All requests sent out by CarbonRouter. Includes failover and shadow
  // requests.
  AllOutgoing = 2,
};

/*
 * Per-request moving average and cumulative stats used in Proxy.
 * StatsConfig is expected to look as follows:
 *
 * struct StatsConfig {
 *   // Multiple request types can be grouped into one stat. Array of names
 *   // of such groups.
 *   static constexpr std::array<folly::StringPiece, kNumRequestGroups>
 *       sumStatNames;
 *
 *   // Array of names for rate stats. These must follow a special format
 *   // (incoming names, normal outgoing names, all outgoing names).
 *   // See MemcacheRouterStats.h for an example.
 *   static constexpr std::array<folly::StringPiece, 3 * kNumRequestGroups>
 *       rateStatNames;
 *
 *   // Which group Request belongs to
 *   template <class Request>
 *   static constexpr size_t getStatsGroup();
 * };
 */
template <class StatsConfig>
class Stats {
 public:
  template <class Request>
  void bump(RouterStatTypes statType) {
    sumStats_.template bump<Request>(statType);
    rateStats_.template bump<Request>(statType);
  }

  folly::dynamic dump(bool filterZeroes = false) const {
    return folly::dynamic::merge(
        sumStats_.dump(filterZeroes), rateStats_.dump(filterZeroes));
  }

  void advanceBin() {
    rateStats_.advanceBin();
  }

 private:
  class SumStats {
   public:
    SumStats() {
      static_assert(
          kNumStats == StatsConfig::sumStatNames.size(),
          "Mismatch in expected number of SumStats names. You may need to "
          "re-run the Carbon compiler on your .idl file");
    }

    template <class Request>
    void bump(carbon::RouterStatTypes statType) {
      if (statType != carbon::RouterStatTypes::Incoming) {
        return;
      }
      constexpr auto statGroup = StatsConfig::template getStatGroup<Request>();
      auto& stat = sumStats_[statGroup];
      stat.store(
          stat.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
    }

    folly::dynamic dump(bool filterZeroes) const {
      folly::dynamic rv(folly::dynamic::object());
      for (size_t statGroup = 0; statGroup < kNumStats; ++statGroup) {
        auto sumStat = sumStats_[statGroup].load(std::memory_order_relaxed);
        if (filterZeroes && sumStat == 0) {
          continue;
        }
        rv[kStatNames[statGroup]] = sumStat;
      }
      return rv;
    }

   private:
    // Only one stats class: incoming requests
    static constexpr size_t kNumStatClasses = 1;
    static constexpr size_t kNumRequestGroups = StatsConfig::kNumRequestGroups;
    static constexpr size_t kNumStats = kNumStatClasses * kNumRequestGroups;
    static constexpr std::array<folly::StringPiece, kNumStats> kStatNames =
        StatsConfig::sumStatNames;

    std::array<std::atomic<uint64_t>, kNumStats> sumStats_{};
  };

  class RateStats {
   public:
    RateStats() {
      static_assert(
          kNumStats == StatsConfig::rateStatNames.size(),
          "Mismatch in expected number of RateStats names. You may need to "
          "re-run the Carbon compiler on your .idl file");
    }

    template <class Request>
    void bump(carbon::RouterStatTypes statType) {
      constexpr auto statGroup = StatsConfig::template getStatGroup<Request>();
      const size_t baseOffset =
          static_cast<size_t>(statType) * kNumRequestGroups;
      auto& stat = currentStats_[baseOffset + statGroup];
      stat.store(
          stat.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
    }

    folly::dynamic dump(bool filterZeroes) const {
      folly::dynamic rv(folly::dynamic::object());
      for (size_t statIndex = 0; statIndex < kNumStats; ++statIndex) {
        if (filterZeroes && accumulatedStats_[statIndex] == 0) {
          continue;
        }
        const auto key = kStatNames[statIndex];
        rv[key] = numBinsUsed_ > 0
            ? static_cast<double>(accumulatedStats_[statIndex]) / numBinsUsed_
            : 0.0;
      }
      return rv;
    }

    void advanceBin() {
      binIndex_ = (binIndex_ + 1) % kNumBins;
      if (numBinsUsed_ < kNumBins) {
        ++numBinsUsed_;
      }

      for (size_t statIndex = 0; statIndex < kNumStats; ++statIndex) {
        accumulatedStats_[statIndex] -= rateStats_[statIndex][binIndex_];
        auto currentStat =
            currentStats_[statIndex].load(std::memory_order_relaxed);
        rateStats_[statIndex][binIndex_] = currentStat;
        accumulatedStats_[statIndex] += currentStat;
        currentStats_[statIndex].store(0, std::memory_order_relaxed);
      }
    }

   private:
    // 3 rate classes per stat group:
    // incoming, normal outgoing, all outgoing
    static constexpr size_t kNumStatClasses = 3;
    static constexpr size_t kNumRequestGroups = StatsConfig::kNumRequestGroups;
    static constexpr size_t kNumStats = kNumStatClasses * kNumRequestGroups;
    static constexpr size_t kNumBins = 4 * 60; // 4 minutes of data
    static constexpr std::array<folly::StringPiece, kNumStats> kStatNames =
        StatsConfig::rateStatNames;

    size_t binIndex_{0};
    size_t numBinsUsed_{0};
    std::array<std::atomic<uint64_t>, kNumStats> currentStats_{};
    std::array<uint64_t, kNumStats> accumulatedStats_{};
    std::array<std::array<uint64_t, kNumBins>, kNumStats> rateStats_{};
  };

  SumStats sumStats_;
  RateStats rateStats_;
};

template <class RouterInfo>
constexpr std::array<folly::StringPiece, Stats<RouterInfo>::SumStats::kNumStats>
    Stats<RouterInfo>::SumStats::kStatNames;

template <class RouterInfo>
constexpr std::
    array<folly::StringPiece, Stats<RouterInfo>::RateStats::kNumStats>
        Stats<RouterInfo>::RateStats::kStatNames;

} // namespace carbon
