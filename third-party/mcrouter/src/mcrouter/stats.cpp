/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "stats.h"

#include <dirent.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <limits>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/experimental/StringKeyedUnorderedMap.h>
#include <folly/json.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestination.h"
#include "mcrouter/ProxyDestinationMap.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/StatsReply.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

/**                             .__
 * __  _  _______ _______  ____ |__| ____    ____
 * \ \/ \/ /\__  \\_  __ \/    \|  |/    \  / ___\
 *  \     /  / __ \|  | \/   |  \  |   |  \/ /_/  >
 *   \/\_/  (____  /__|  |___|  /__|___|  /\___  /
 *               \/           \/        \//_____/
 *
 * Read the following code with proper care for life and limb.
 */

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

char* gStandaloneArgs = nullptr;

const char* clientStateToStr(ProxyDestinationBase::State state) {
  switch (state) {
    case ProxyDestinationBase::State::Up:
      return "up";
    case ProxyDestinationBase::State::New:
      return "new";
    case ProxyDestinationBase::State::Closed:
      return "closed";
    case ProxyDestinationBase::State::Down:
      return "down";
    case ProxyDestinationBase::State::NumStates:
      assert(false);
  }
  return "unknown";
}

struct ServerStat {
  uint64_t results[static_cast<size_t>(carbon::Result::NUM_RESULTS)] = {0};
  size_t states[(size_t)ProxyDestinationBase::State::NumStates] = {0};
  bool isHardTko{false};
  bool isSoftTko{false};
  double sumLatencies{0.0};
  size_t cntLatencies{0};
  size_t pendingRequestsCount{0};
  size_t inflightRequestsCount{0};
  double sumRetransPerKByte{0.0};
  size_t cntRetransPerKByte{0};
  double maxRetransPerKByte{0.0};
  double minRetransPerKByte{std::numeric_limits<double>::infinity()};

  std::string toString() const {
    double avgLatency = cntLatencies == 0 ? 0 : sumLatencies / cntLatencies;
    auto res = folly::format("avg_latency_us:{:.3f}", avgLatency).str();
    folly::format(" pending_reqs:{}", pendingRequestsCount).appendTo(res);
    folly::format(" inflight_reqs:{}", inflightRequestsCount).appendTo(res);
    if (isHardTko) {
      res.append(" hard_tko; ");
    } else if (isSoftTko) {
      res.append(" soft_tko; ");
    }
    if (cntRetransPerKByte > 0) {
      double avgRetransPerKByte = sumRetransPerKByte / cntRetransPerKByte;
      folly::format(
          " avg_retrans_ratio:{} max_retrans_ratio:{} min_retrans_ratio:{}",
          avgRetransPerKByte,
          maxRetransPerKByte,
          minRetransPerKByte)
          .appendTo(res);
    }
    for (size_t i = 0; i < (size_t)ProxyDestinationBase::State::NumStates;
         ++i) {
      if (states[i] > 0) {
        auto state =
            clientStateToStr(static_cast<ProxyDestinationBase::State>(i));
        folly::format(" {}:{}", state, states[i]).appendTo(res);
      }
    }
    bool firstResult = true;
    for (size_t i = 0; i < static_cast<size_t>(carbon::Result::NUM_RESULTS);
         ++i) {
      if (results[i] > 0) {
        folly::StringPiece result(
            carbon::resultToString(static_cast<carbon::Result>(i)));
        result.removePrefix("mc_res_");
        folly::format("{} {}:{}", firstResult ? ";" : "", result, results[i])
            .appendTo(res);
        firstResult = false;
      }
    }
    return res;
  }
};

int get_num_bins_used(const CarbonRouterInstanceBase& router) {
  if (router.opts().num_proxies > 0) {
    auto anyProxy = router.getProxyBase(0);
    if (anyProxy) {
      return anyProxy->stats().numBinsUsed();
    }
  }
  return 0;
}

double stats_rate_value(ProxyBase* proxy, int idx) {
  const stat_t& stat = proxy->stats().getStat(idx);
  double rate = 0;

  if (proxy->stats().numBinsUsed() != 0) {
    if (stat.aggregate) {
      rate = stats_aggregate_rate_value(proxy->router(), idx);
    } else {
      rate = (double)proxy->stats().getStatValueWithinWindow(idx) /
          (proxy->stats().numBinsUsed() * MOVING_AVERAGE_BIN_SIZE_IN_SECOND);
    }
  }

  return rate;
}

uint64_t stats_max_value(ProxyBase* proxy, int idx) {
  return stats_aggregate_max_value(proxy->router(), idx);
}

} // anonymous namespace

// This is a subset of what's in proc(5).
struct proc_stat_data_t {
  unsigned long num_minor_faults;
  unsigned long num_major_faults;
  double user_time_sec;
  double system_time_sec;
  unsigned long vsize;
  unsigned long rss;
};

double stats_aggregate_rate_value(
    const CarbonRouterInstanceBase& router,
    int idx) {
  double rate = 0;
  int num_bins_used = get_num_bins_used(router);

  if (num_bins_used != 0) {
    uint64_t num = 0;
    for (size_t i = 0; i < router.opts().num_proxies; ++i) {
      num += router.getProxyBase(i)->stats().getStatValueWithinWindow(idx);
    }
    rate = (double)num / (num_bins_used * MOVING_AVERAGE_BIN_SIZE_IN_SECOND);
  }

  return rate;
}

uint64_t stats_aggregate_max_value(
    const CarbonRouterInstanceBase& router,
    int idx) {
  uint64_t max = 0;
  int num_bins_used = get_num_bins_used(router);

  for (int j = 0; j < num_bins_used; ++j) {
    uint64_t binSum = 0;
    for (size_t i = 0; i < router.opts().num_proxies; ++i) {
      binSum += router.getProxyBase(i)->stats().getStatBinValue(idx, j);
    }
    max = std::max(max, binSum);
  }

  return max;
}

uint64_t stats_aggregate_max_max_value(
    const CarbonRouterInstanceBase& router,
    int idx) {
  uint64_t max = 0;
  int num_bins_used = get_num_bins_used(router);

  for (int j = 0; j < num_bins_used; ++j) {
    for (size_t i = 0; i < router.opts().num_proxies; ++i) {
      max = std::max(
          max, router.getProxyBase(i)->stats().getStatBinValue(idx, j));
    }
  }
  return max;
}

static std::string rate_stat_to_str(ProxyBase* proxy, int idx) {
  return folly::stringPrintf("%g", stats_rate_value(proxy, idx));
}

static std::string max_stat_to_str(ProxyBase* proxy, int idx) {
  return folly::to<std::string>(stats_max_value(proxy, idx));
}

static std::string max_max_stat_to_str(ProxyBase* proxy, int idx) {
  return folly::to<std::string>(
      stats_aggregate_max_max_value(proxy->router(), idx));
}

/**
 * Write a stat into a buffer.
 *
 * @param stat_t* stat the stat to write
 * @param char* buf the already allocated buffer to write into
 * @param void* ptr the ptr to the structure that has the stat to be written
 *
 * @eturn the length of the string written, excluding terminator
 */
static std::string stat_to_str(const stat_t* stat, void* /* ptr */) {
  switch (stat->type) {
    case stat_string:
      return stat->data.string ? stat->data.string : "";
    case stat_uint64:
      return folly::to<std::string>(stat->data.uint64);
    case stat_int64:
      return folly::to<std::string>(stat->data.int64);
    case stat_double:
      return folly::stringPrintf("%g", stat->data.dbl);
    default:
      LOG(ERROR) << "unknown stat type " << stat->type << " (" << stat->name
                 << ")";
      return "";
  }
}

void init_stats(stat_t* stats) {
#define STAT(_name, _type, _aggregate, _data_assignment) \
  {                                                      \
    stat_t& s = stats[_name##_stat];                     \
    s.name = #_name;                                     \
    s.group = GROUP;                                     \
    s.type = _type;                                      \
    s.aggregate = _aggregate;                            \
    s.data _data_assignment;                             \
  }
#define STUI(name, value, agg) STAT(name, stat_uint64, agg, .uint64 = value)
#define STSI(name, value, agg) STAT(name, stat_int64, agg, .int64 = value)
#define STSS(name, value, agg) \
  STAT(name, stat_string, agg, .string = (char*)value)
#define EXTERNAL_STAT(name) \
  {}
#include "stat_list.h"
#undef STAT
#undef STUI
#undef STSI
#undef STSS
#undef EXTERNAL_STAT
}

// Returns 0 on success, -1 on failure.  In either case, all fields of
// *data will be initialized to something.
static int get_proc_stat(pid_t pid, proc_stat_data_t* data) {
  data->num_minor_faults = 0;
  data->num_major_faults = 0;
  data->user_time_sec = 0.0;
  data->system_time_sec = 0.0;
  data->vsize = 0;
  data->rss = 0;

  char stat_path[32];
  snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

  FILE* stat_file = fopen(stat_path, "r");
  if (stat_file == nullptr) {
    LOG(ERROR) << "Can't open process status information file: " << stat_path
               << ": " << strerror(errno);
    return -1;
  }

  // Note: the field list in proc(5) on my dev machine was incorrect.
  // I have confirmed that this is correct:
  //
  // http://manpages.ubuntu.com/manpages/lucid/man5/proc.5.html
  //
  // We only report out a few of the fields from the stat file, but it
  // should be easy to add more later if they are desired.

  long rss_pages;
  unsigned long utime_ticks;
  unsigned long stime_ticks;

  int count = fscanf(
      stat_file,
      "%*d (%*[^)]) %*c %*d %*d %*d %*d %*d %*u %lu "
      "%*u %lu %*u %lu %lu %*d %*d %*d %*d %*d "
      "%*d %*u %lu %ld" /* and there's more */,
      &data->num_minor_faults,
      &data->num_major_faults,
      &utime_ticks,
      &stime_ticks,
      &data->vsize,
      &rss_pages);
  fclose(stat_file);

  if (count != 6) {
    return -1;
  }

  data->user_time_sec = ((double)utime_ticks) / sysconf(_SC_CLK_TCK);
  data->system_time_sec = ((double)stime_ticks) / sysconf(_SC_CLK_TCK);

  // rss is documented to be signed, but since negative numbers are
  // nonsensical, and nothing else is in pages, we clamp it and
  // convert to bytes here.

  data->rss =
      rss_pages < 0 ? 0ul : (unsigned long)(rss_pages * sysconf(_SC_PAGESIZE));

  return 0;
}

void append_pool_stats(
    CarbonRouterInstanceBase& router,
    std::vector<stat_t>& stats) {
  folly::F14NodeMap<std::string, stat_t> mergedPoolStatsMap;

  auto mergeMaps = [&mergedPoolStatsMap](
                       folly::F14NodeMap<std::string, stat_t>&& poolStatMap) {
    for (auto& poolStatMapEntry : poolStatMap) {
      auto it = mergedPoolStatsMap.find(poolStatMapEntry.first);
      if (it != mergedPoolStatsMap.end()) {
        auto iref = folly::make_atomic_ref(it->second.data.uint64);
        auto pref = folly::make_atomic_ref(poolStatMapEntry.second.data.uint64);
        iref.store(
            iref.load(std::memory_order_relaxed) +
                pref.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
      } else {
        mergedPoolStatsMap.insert(std::move(poolStatMapEntry));
      }
    }
  };

  for (size_t j = 0; j < router.opts().num_proxies; ++j) {
    auto pr = router.getProxyBase(j);
    mergeMaps(pr->stats().getAggregatedPoolStatsMap());
  }
  for (const auto& mergedPoolStatMapEntry : mergedPoolStatsMap) {
    stats.emplace_back(mergedPoolStatMapEntry.second);
  }
}

void prepare_stats(CarbonRouterInstanceBase& router, stat_t* stats) {
  init_stats(stats);

  uint64_t config_last_success = 0;
  uint64_t destinationBatchesSum = 0;
  uint64_t destinationRequestsSum = 0;
  uint64_t outstandingGetReqsTotal = 0;
  uint64_t outstandingGetReqsHelper = 0;
  uint64_t outstandingGetWaitTimeSumUs = 0;
  uint64_t outstandingUpdateReqsTotal = 0;
  uint64_t outstandingUpdateReqsHelper = 0;
  uint64_t outstandingUpdateWaitTimeSumUs = 0;
  uint64_t retransPerKByteSum = 0;
  uint64_t retransNumTotal = 0;
  uint64_t destinationRequestsDirtyBufferSum = 0;
  uint64_t destinationRequestsTotalSum = 0;

  for (size_t i = 0; i < router.opts().num_proxies; ++i) {
    auto proxy = router.getProxyBase(i);
    config_last_success = std::max(
        config_last_success, proxy->stats().getValue(config_last_success_stat));
    destinationBatchesSum +=
        proxy->stats().getStatValueWithinWindow(destination_batches_sum_stat);
    destinationRequestsSum +=
        proxy->stats().getStatValueWithinWindow(destination_requests_sum_stat);

    outstandingGetReqsTotal += proxy->stats().getStatValueWithinWindow(
        outstanding_route_get_reqs_queued_stat);
    outstandingGetReqsHelper += proxy->stats().getStatValueWithinWindow(
        outstanding_route_get_reqs_queued_helper_stat);
    outstandingGetWaitTimeSumUs += proxy->stats().getStatValueWithinWindow(
        outstanding_route_get_wait_time_sum_us_stat);
    outstandingUpdateReqsTotal += proxy->stats().getStatValueWithinWindow(
        outstanding_route_update_reqs_queued_stat);
    outstandingUpdateReqsHelper += proxy->stats().getStatValueWithinWindow(
        outstanding_route_update_reqs_queued_helper_stat);
    outstandingUpdateWaitTimeSumUs += proxy->stats().getStatValueWithinWindow(
        outstanding_route_update_wait_time_sum_us_stat);

    retransPerKByteSum +=
        proxy->stats().getStatValueWithinWindow(retrans_per_kbyte_sum_stat);
    retransNumTotal +=
        proxy->stats().getStatValueWithinWindow(retrans_num_total_stat);

    destinationRequestsDirtyBufferSum +=
        proxy->stats().getStatValueWithinWindow(
            destination_reqs_dirty_buffer_sum_stat);
    destinationRequestsTotalSum += proxy->stats().getStatValueWithinWindow(
        destination_reqs_total_sum_stat);
  }

  stat_set(
      stats,
      num_suspect_servers_stat,
      router.tkoTrackerMap().getSuspectServersCount());

  double avgBatchSize = 0.0;
  if (destinationBatchesSum != 0) {
    avgBatchSize = destinationRequestsSum / (double)destinationBatchesSum;
  }
  stat_set(stats, destination_batch_size_stat, avgBatchSize);

  double avgRetransPerKByte = 0.0;
  if (retransNumTotal != 0) {
    avgRetransPerKByte = retransPerKByteSum / (double)retransNumTotal;
  }
  stat_set(stats, retrans_per_kbyte_avg_stat, avgRetransPerKByte);

  double reqsDirtyBufferRatio = 0.0;
  if (destinationRequestsTotalSum != 0) {
    reqsDirtyBufferRatio =
        destinationRequestsDirtyBufferSum / (double)destinationRequestsTotalSum;
  }

  stat_set(
      stats, destination_reqs_dirty_buffer_ratio_stat, reqsDirtyBufferRatio);
  stat_set(stats, outstanding_route_get_avg_queue_size_stat, 0.0);
  stat_set(stats, outstanding_route_get_avg_wait_time_sec_stat, 0.0);

  if (outstandingGetReqsTotal > 0) {
    stat_set(
        stats,
        outstanding_route_get_avg_queue_size_stat,
        outstandingGetReqsHelper / (double)outstandingGetReqsTotal);
    stat_set(
        stats,
        outstanding_route_get_avg_wait_time_sec_stat,
        outstandingGetWaitTimeSumUs / (1000000.0 * outstandingGetReqsTotal));
  }

  stat_set(stats, outstanding_route_update_avg_queue_size_stat, 0.0);
  stat_set(stats, outstanding_route_update_avg_wait_time_sec_stat, 0.0);
  if (outstandingUpdateReqsTotal > 0) {
    stat_set(
        stats,
        outstanding_route_update_avg_queue_size_stat,
        outstandingUpdateReqsHelper / (double)outstandingUpdateReqsTotal);
    stat_set(
        stats,
        outstanding_route_update_avg_wait_time_sec_stat,
        outstandingUpdateWaitTimeSumUs /
            (1000000.0 * outstandingUpdateReqsTotal));
  }

  folly::make_atomic_ref(stats[commandargs_stat].data.string)
      .store(gStandaloneArgs, std::memory_order_relaxed);

  uint64_t now = time(nullptr);
  stat_set(stats, time_stat, now);
  stat_set(stats, uptime_stat, now - router.startTime());
  stat_set(stats, config_age_stat, now - config_last_success);
  stat_set(stats, config_last_success_stat, config_last_success);
  stat_set(
      stats,
      config_last_attempt_stat,
      static_cast<uint64_t>(router.lastConfigAttempt()));
  stat_set(stats, config_failures_stat, router.configFailures());
  stat_set(stats, config_full_attempt_stat, router.configFullAttempt());
  stat_set(
      stats,
      configs_from_disk_stat,
      static_cast<uint64_t>(router.configuredFromDisk()));
  stat_set(
      stats,
      config_partial_reconfig_attempt_stat,
      router.partialReconfigAttempt());
  stat_set(
      stats,
      config_partial_reconfig_success_stat,
      router.partialReconfigSuccess());

  folly::make_atomic_ref(stats[pid_stat].data.int64)
      .store(getpid(), std::memory_order_relaxed);
  folly::make_atomic_ref(stats[parent_pid_stat].data.int64)
      .store(getppid(), std::memory_order_relaxed);

  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);

  stat_set(
      stats,
      rusage_user_stat,
      ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0);
  stat_set(
      stats,
      rusage_system_stat,
      ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0);

  proc_stat_data_t ps_data;
  get_proc_stat(getpid(), &ps_data);
  stat_set(stats, ps_num_minor_faults_stat, ps_data.num_minor_faults);
  stat_set(stats, ps_num_major_faults_stat, ps_data.num_major_faults);
  stat_set(stats, ps_user_time_sec_stat, ps_data.user_time_sec);
  stat_set(stats, ps_system_time_sec_stat, ps_data.system_time_sec);
  stat_set(stats, ps_rss_stat, ps_data.rss);
  stat_set(stats, ps_vsize_stat, ps_data.vsize);

  stat_set(stats, fibers_allocated_stat, UINT64_C(0));
  stat_set(stats, fibers_pool_size_stat, UINT64_C(0));
  stat_set(stats, fibers_stack_high_watermark_stat, UINT64_C(0));
  for (size_t i = 0; i < router.opts().num_proxies; ++i) {
    auto pr = router.getProxyBase(i);
    stat_incr(
        stats,
        fibers_allocated_stat,
        static_cast<int64_t>(pr->fiberManager().fibersAllocated()));
    stat_incr(
        stats,
        fibers_pool_size_stat,
        static_cast<int64_t>(pr->fiberManager().fibersPoolSize()));
    stat_set(
        stats,
        fibers_stack_high_watermark_stat,
        std::max(
            stat_get_uint64(stats, fibers_stack_high_watermark_stat),
            pr->fiberManager().stackHighWatermark()));
    stat_incr(stats, duration_us_stat, pr->stats().durationUs().value());
    stat_incr(stats, duration_get_us_stat, pr->stats().durationGetUs().value());
    stat_incr(
        stats, duration_update_us_stat, pr->stats().durationUpdateUs().value());
    stat_incr(
        stats,
        inactive_connection_closed_interval_sec_stat,
        pr->stats().inactiveConnectionClosedIntervalSec().value());
    stat_incr(
        stats,
        client_queue_notify_period_stat,
        static_cast<int64_t>(pr->queueNotifyPeriod()));
    stat_incr(
        stats,
        asynclog_duration_us_stat,
        pr->stats().asyncLogDurationUs().value());
    stat_incr(
        stats,
        axon_proxy_duration_us_stat,
        pr->stats().axonProxyDurationUs().value());
    stat_set(
        stats,
        proxy_queue_full_stat,
        std::max(
            stat_get_uint64(stats, proxy_queue_full_stat),
            static_cast<uint64_t>(pr->messageQueueFull() ? 1 : 0)));
    stat_set(
        stats,
        proxy_queues_all_full_stat,
        std::min(
            i ? stat_get_uint64(stats, proxy_queue_full_stat) : 0,
            static_cast<uint64_t>(pr->messageQueueFull() ? 1 : 0)));
  }

  if (router.opts().num_proxies > 0) {
    // Set the number of proxy threads
    stat_set(stats, num_proxies_stat, router.opts().num_proxies);
    // Set avg proxy cpu
    if (router.opts().proxy_cpu_monitor_ms > 0) {
      stat_set(stats, proxy_cpu_stat, router.getProxyCpu());
    }
    stat_div(stats, duration_us_stat, router.opts().num_proxies);
    stat_div(stats, duration_get_us_stat, router.opts().num_proxies);
    stat_div(stats, duration_update_us_stat, router.opts().num_proxies);
    stat_div(
        stats,
        inactive_connection_closed_interval_sec_stat,
        router.opts().num_proxies);
    stat_div(stats, client_queue_notify_period_stat, router.opts().num_proxies);
    stat_div(stats, asynclog_duration_us_stat, router.opts().num_proxies);
    stat_div(stats, axon_proxy_duration_us_stat, router.opts().num_proxies);
  }

  for (int i = 0; i < num_stats; i++) {
    if (stats[i].aggregate && !(stats[i].group & rate_stats)) {
      for (size_t j = 0; j < router.opts().num_proxies; ++j) {
        auto pr = router.getProxyBase(j);
        if (stats[i].type == stat_uint64) {
          stat_incr(
              stats,
              static_cast<stat_name_t>(i),
              static_cast<int64_t>(
                  folly::make_atomic_ref(pr->stats().getStat(i).data.uint64)
                      .load(std::memory_order_relaxed)));
        } else if (stats[i].type == stat_int64) {
          stat_incr(
              stats,
              static_cast<stat_name_t>(i),
              folly::make_atomic_ref(pr->stats().getStat(i).data.int64)
                  .load(std::memory_order_relaxed));
        } else if (stats[i].type == stat_double) {
          stat_incr(
              stats,
              static_cast<stat_name_t>(i),
              folly::make_atomic_ref(pr->stats().getStat(i).data.dbl)
                  .load(std::memory_order_relaxed));
        } else {
          LOG(FATAL) << "you can't aggregate non-numerical stats!";
        }
      }
    }
  }
}

static stat_group_t stat_parse_group_str(folly::StringPiece str) {
  if (str == "all") {
    return all_stats;
  } else if (str == "detailed") {
    return detailed_stats;
  } else if (str == "cmd-error") {
    return cmd_error_stats;
  } else if (str == "ods") {
    return ods_stats;
  } else if (str == "servers") {
    return server_stats;
  } else if (str == "suspect_servers") {
    return suspect_server_stats;
  } else if (str == "count") {
    return count_stats;
  } else if (str == "external") {
    return external_stats;
  } else if (str.empty()) {
    return basic_stats;
  } else {
    return unknown_stats;
  }
}

/**
 * @param Proxy proxy
 */
McStatsReply stats_reply(ProxyBase* proxy, folly::StringPiece group_str) {
  auto lockGuard = proxy->stats().lock();

  StatsReply reply;

  if (group_str == "version") {
    reply.addStat("mcrouter-version", MCROUTER_PACKAGE_STRING);
    return reply.getReply();
  }

  auto groups = stat_parse_group_str(group_str);
  if (groups == unknown_stats) {
    McStatsReply errorReply(carbon::Result::CLIENT_ERROR);
    errorReply.message_ref() = "bad stats command";
    return errorReply;
  }

  std::vector<stat_t> stats(num_stats);

  prepare_stats(proxy->router(), stats.data());

  for (unsigned int ii = 0; ii < num_stats; ii++) {
    stat_t* stat = &stats[ii];
    if (stat->group & groups) {
      if (stat->group & rate_stats) {
        reply.addStat(stat->name, rate_stat_to_str(proxy, ii));
      } else if (stat->group & max_stats) {
        reply.addStat(stat->name, max_stat_to_str(proxy, ii));
      } else if (stat->group & max_max_stats) {
        reply.addStat(stat->name, max_max_stat_to_str(proxy, ii));
      } else {
        reply.addStat(stat->name, stat_to_str(stat, nullptr));
      }
    }
  }
  append_pool_stats(proxy->router(), stats);

  if (groups & (basic_stats | all_stats | detailed_stats | ods_stats)) {
    folly::dynamic requestStats(folly::dynamic::object());
    const auto& router = proxy->router();
    for (size_t i = 0; i < router.opts().num_proxies; ++i) {
      const auto proxyRequestStats =
          router.getProxyBase(i)->dumpRequestStats(false /* filterZeroes */);
      for (const auto& k : proxyRequestStats.keys()) {
        requestStats.setDefault(k, 0) += proxyRequestStats[k];
      }
    }

    for (const auto& k : requestStats.keys()) {
      if (requestStats[k].isInt()) {
        reply.addStat(
            k.asString(), folly::to<std::string>(requestStats[k].asInt()));
      } else if (requestStats[k].isDouble()) {
        reply.addStat(
            k.asString(), folly::to<std::string>(requestStats[k].asDouble()));
      } else {
        MC_LOG_FAILURE(
            proxy->router().opts(),
            failure::Category::kOther,
            folly::sformat("Couldn't serialize Carbon stat {}", k.asString()));
      }
    }
  }

  if (groups & server_stats) {
    folly::F14NodeMap<std::string, ServerStat> serverStats;
    auto& router = proxy->router();
    for (size_t i = 0; i < router.opts().num_proxies; ++i) {
      router.getProxyBase(i)->destinationMap()->foreachDestinationSynced(
          [&serverStats](const ProxyDestinationBase& pdstn) {
            ProxyDestinationKey key(pdstn);
            auto& stat = serverStats[key.str()];
            stat.isHardTko = pdstn.tracker()->isHardTko();
            stat.isSoftTko = pdstn.tracker()->isSoftTko();
            if (pdstn.stats().results) {
              for (size_t j = 0;
                   j < static_cast<size_t>(carbon::Result::NUM_RESULTS);
                   ++j) {
                stat.results[j] += (*pdstn.stats().results)[j];
              }
            }
            ++stat.states[(size_t)pdstn.stats().state];

            if (pdstn.stats().avgLatency.hasValue()) {
              stat.sumLatencies += pdstn.stats().avgLatency.value();
              ++stat.cntLatencies;
            }

            if (pdstn.stats().retransPerKByte >= 0.0) {
              const auto val = pdstn.stats().retransPerKByte;
              stat.sumRetransPerKByte += val;
              stat.maxRetransPerKByte = std::max(stat.maxRetransPerKByte, val);
              stat.minRetransPerKByte = std::min(stat.minRetransPerKByte, val);
              ++stat.cntRetransPerKByte;
            }
            auto reqStats = pdstn.getRequestStats();
            stat.pendingRequestsCount += reqStats.numPending;
            stat.inflightRequestsCount += reqStats.numInflight;
          });
    }
    for (const auto& it : serverStats) {
      reply.addStat(it.first, it.second.toString());
    }
  }

  if (groups & suspect_server_stats) {
    auto suspectServers = proxy->router().tkoTrackerMap().getSuspectServers();
    for (const auto& it : suspectServers) {
      reply.addStat(
          it.first,
          folly::format(
              "status:{} num_failures:{}",
              it.second.first ? "tko" : "down",
              it.second.second)
              .str());
    }
  }

  if (groups & external_stats) {
    const auto externalStats =
        proxy->router().externalStatsHandler().getStats();
    for (const auto& kv : externalStats) {
      reply.addStat(kv.first, kv.second);
    }
  }

  return reply.getReply();
}

void set_standalone_args(folly::StringPiece args) {
  assert(gStandaloneArgs == nullptr);
  gStandaloneArgs = new char[args.size() + 1];
  ::memcpy(gStandaloneArgs, args.begin(), args.size());
  gStandaloneArgs[args.size()] = 0;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
