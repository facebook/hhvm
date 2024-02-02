/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <unordered_map>

#include <folly/Range.h>
#include <folly/synchronization/AtomicRef.h>

#include "mcrouter/TargetHooks.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

// make sure MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND can be exactly divided by
// MOVING_AVERAGE_BIN_SIZE_IN_SECOND
// the window size within which average stat rate is calculated
#define MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND (60 * 4)

// the bin size for average stat rate
#define MOVING_AVERAGE_BIN_SIZE_IN_SECOND (1)

// define stat_name_t
#define STAT(name, ...) name##_stat,
#define STUI STAT
#define STSI STAT
#define STSS STAT
#define EXTERNAL_STAT(name)
enum stat_name_t {
#include "stat_list.h"
  num_stats,
};
#undef STAT
#undef STUI
#undef STSI
#undef STSS
#undef EXTERNAL_STAT

// Forward declarations
class CarbonRouterInstanceBase;
class ProxyBase;

/** statistics ftw */

struct stat_s;
typedef std::string (*string_fn_t)(void*);

enum stat_type_t {
  stat_string,
  stat_uint64,
  stat_int64,
  stat_double,
  //  stat_percentile, // TBD
  num_stat_types
};

enum stat_group_t {
  basic_stats = 0x1,
  detailed_stats = 0x2,
  cmd_error_stats = 0x20,
  ods_stats = 0x40,
  rate_stats = 0x100,
  count_stats = 0x200,
  max_stats = 0x800,
  max_max_stats = 0x1000,
  avg_stats = 0x2000,
  all_stats = 0xffff,
  server_stats = 0x10000,
  suspect_server_stats = 0x40000,
  external_stats = 0x80000,
  unknown_stats = 0x10000000,
};

/** defines a statistic: name, type, and data */
struct stat_t {
  folly::StringPiece name;
  int group;
  stat_type_t type;
  int aggregate;
  union {
    char* string;
    uint64_t uint64;
    int64_t int64;
    double dbl;
    void* pointer;
  } data;

  stat_t() = default;

  stat_t(const stat_t& from)
      : name(from.name),
        group(from.group),
        type(from.type),
        aggregate(from.aggregate) {
    // It doesn't matter which field we take (as long as it's a maximum width
    // field), we just want to copy these bytes atomically.
    data.uint64 =
        folly::make_atomic_ref(const_cast<uint64_t&>(from.data.uint64))
            .load(std::memory_order_relaxed);
  }

  // No particular reason to disable, but just unnecessary.
  stat_t& operator=(const stat_t& from) = delete;
};

namespace detail {

FOLLY_ALWAYS_INLINE void
stat_incr_internal(stat_t* stats, stat_name_t stat_num, int64_t amount) {
  auto ref = folly::make_atomic_ref(stats[stat_num].data.uint64);
  ref.store(
      ref.load(std::memory_order_relaxed) + amount, std::memory_order_relaxed);
}

FOLLY_ALWAYS_INLINE void
stat_incr_internal(stat_t* stats, stat_name_t stat_num, double amount) {
  auto ref = folly::make_atomic_ref(stats[stat_num].data.dbl);
  ref.store(
      ref.load(std::memory_order_relaxed) + amount, std::memory_order_relaxed);
}

} // namespace detail

/**
 * Interface for custom stats handling.
 */
class StatsApi {
 public:
  virtual ~StatsApi() = default;

  /**
   * Called once on startup with the router instance
   */
  virtual void init(const CarbonRouterInstanceBase&) = 0;

  /**
   * Called on every increment or decrement for the stat.
   *
   * MT-safety: must be able to be called concurrently from multiple
   * threads.
   */
  virtual void addSample(stat_name_t, double) = 0;

  /**
   * Called when setting the absolute value for the stat.
   *
   * MT-safety: must be able to be called concurrently from multiple
   * threads.
   */
  virtual void setValue(stat_name_t, double) = 0;
};

void init_stats(stat_t* stats);

FOLLY_ALWAYS_INLINE uint64_t
stat_fetch_add(stat_t* stats, stat_name_t stat_num, int64_t amount) {
  if (gStatsApiHook) {
    gStatsApiHook().addSample(stat_num, amount);
  }
  auto ref = folly::make_atomic_ref(stats[stat_num].data.uint64);
  return ref.fetch_add(amount, std::memory_order_relaxed);
}

FOLLY_ALWAYS_INLINE void
stat_incr(stat_t* stats, stat_name_t stat_num, int64_t amount) {
  if (gStatsApiHook) {
    gStatsApiHook().addSample(stat_num, amount);
  }
  detail::stat_incr_internal(stats, stat_num, amount);
}

FOLLY_ALWAYS_INLINE void
stat_incr(stat_t* stats, stat_name_t stat_num, double amount) {
  if (gStatsApiHook) {
    gStatsApiHook().addSample(stat_num, amount);
  }
  detail::stat_incr_internal(stats, stat_num, amount);
}

FOLLY_ALWAYS_INLINE
void stat_set(stat_t* stats, stat_name_t stat_num, uint64_t value) {
  if (gStatsApiHook) {
    gStatsApiHook().setValue(stat_num, value);
  }
  stat_t* stat = &stats[stat_num];
  assert(stat->type == stat_uint64);
  folly::make_atomic_ref(stat->data.uint64)
      .store(value, std::memory_order_relaxed);
}

FOLLY_ALWAYS_INLINE
void stat_set(stat_t* stats, stat_name_t stat_num, double value) {
  if (gStatsApiHook) {
    gStatsApiHook().setValue(stat_num, value);
  }
  stat_t* stat = &stats[stat_num];
  assert(stat->type == stat_double);
  folly::make_atomic_ref(stat->data.dbl)
      .store(value, std::memory_order_relaxed);
}

FOLLY_ALWAYS_INLINE uint64_t
stat_get_uint64(stat_t* stats, stat_name_t stat_num) {
  return folly::make_atomic_ref(stats[stat_num].data.uint64)
      .load(std::memory_order_relaxed);
}

/**
 * Current aggregation of rate of stats[idx] (which must be an aggregated
 * rate stat), units will be per second.
 */
double stats_aggregate_rate_value(
    const CarbonRouterInstanceBase& router,
    int idx);

/**
 * Current max between all buckets of stats[idx] (which must be an aggregated
 * max stat)
 */
uint64_t stats_aggregate_max_value(
    const CarbonRouterInstanceBase& router,
    int idx);

/**
 * Current max between all proxies amongst all buckets of stats[idx]
 */
uint64_t stats_aggregate_max_max_value(
    const CarbonRouterInstanceBase& router,
    int idx);

McStatsReply stats_reply(ProxyBase*, folly::StringPiece);
void prepare_stats(CarbonRouterInstanceBase& router, stat_t* stats);
void append_pool_stats(
    CarbonRouterInstanceBase& router,
    std::vector<stat_t>& stats);

void set_standalone_args(folly::StringPiece args);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
