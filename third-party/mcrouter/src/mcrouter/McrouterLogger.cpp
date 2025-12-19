/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McrouterLogger.h"

#include <unistd.h>

#include <string>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <folly/json/DynamicConverter.h>
#include <folly/json/dynamic.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/OptionsUtil.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

const char* kStatsSfx = "stats";
const char* kStatsStartupOptionsSfx = "startup_options";
const char* kConfigSourcesInfoFileName = "config_sources_info";

/**
 * Writes string to a file.
 */
void write_file(
    const McrouterOptions& opts,
    const std::string& statsRoot,
    const std::string& suffix,
    const std::string& str) {
  try {
    // In case the dir was deleted some time after mcrouter started
    if (!ensureDirExistsAndWritable(statsRoot)) {
      return;
    }

    boost::filesystem::path path(statsRoot);
    path /= getStatPrefix(opts) + "." + suffix;
    atomicallyWriteFileToDisk(str, path.string());
  } catch (const std::exception& e) {
    VLOG(1) << "Failed to write stats to disk: " << e.what();
  }
}

/**
 * Determines the correct location and file name and writes the stats object
 * to disk in json format. The suffix is the file extension. If the stats root
 * directory is ever removed or is unwriteable, we just give up.
 */
void write_stats_file(
    const McrouterOptions& opts,
    const std::string& statsRoot,
    const std::string& suffix,
    const folly::dynamic& stats) {
  auto statsString = toPrettySortedJson(stats) + "\n";
  write_file(opts, statsRoot, suffix, statsString);
}

void write_stats_to_disk(
    const McrouterOptions& opts,
    const std::string& statsRoot,
    std::vector<stat_t>& stats,
    const folly::dynamic& requestStats) {
  try {
    std::string prefix = getStatPrefix(opts) + ".";
    folly::dynamic jstats = folly::dynamic::object;

    for (size_t i = 0; i < stats.size(); ++i) {
      if (stats[i].group & ods_stats) {
        auto key = prefix + stats[i].name.str();

        switch (stats[i].type) {
          case stat_uint64: {
            jstats[key] = folly::make_atomic_ref(stats[i].data.uint64)
                              .load(std::memory_order_relaxed);
            break;
          }
          case stat_int64: {
            jstats[key] = folly::make_atomic_ref(stats[i].data.int64)
                              .load(std::memory_order_relaxed);
            break;
          }
          case stat_double: {
            jstats[key] = folly::make_atomic_ref(stats[i].data.dbl)
                              .load(std::memory_order_relaxed);
            break;
          }
          default:
            continue;
        }
      }
    }

    for (const auto& kv : requestStats.items()) {
      jstats[folly::to<std::string>(prefix, kv.first.asString())] = kv.second;
    }

    write_stats_file(opts, statsRoot, kStatsSfx, jstats);
  } catch (const std::exception& e) {
    VLOG(1) << "Failed to write stats to disk: " << e.what();
  }
}

void write_config_sources_info_to_disk(
    CarbonRouterInstanceBase& router,
    const std::string& statsRoot) {
  auto config_info_json = router.configApi().getConfigSourcesInfo();

  try {
    boost::filesystem::path path(statsRoot);
    path /= getStatPrefix(router.opts()) + "." + kConfigSourcesInfoFileName;
    atomicallyWriteFileToDisk(
        toPrettySortedJson(config_info_json), path.string());
  } catch (...) {
    LOG(ERROR) << "Error occured while writing configuration info to disk";
  }
}

std::string statsUpdateFunctionName(folly::StringPiece routerName) {
  static std::atomic<uint64_t> uniqueId(0);
  return folly::to<std::string>(
      "carbon-logger-fn-", routerName, "-", uniqueId.fetch_add(1));
}

} // anonymous namespace

McrouterLogger::McrouterLogger(
    CarbonRouterInstanceBase& router,
    std::unique_ptr<AdditionalLoggerIf> additionalLogger)
    : router_(router),
      additionalLogger_(std::move(additionalLogger)),
      functionHandle_(statsUpdateFunctionName(router_.opts().router_name)) {}

McrouterLogger::~McrouterLogger() {
  stop();
}

bool McrouterLogger::start() {
  if (router_.opts().stats_logging_interval == 0) {
    return false;
  }

  // Try the default stats_root first
  if (ensureDirExistsAndWritable(router_.opts().stats_root)) {
    statsRoot_ = router_.opts().stats_root;
  } else {
    // If default path is not available and TW backup is enabled, try backup
    // path
    if (router_.opts().enable_tw_crash_config_backup_path &&
        additionalLogger_) {
      auto backupPath = additionalLogger_->getBackupStatsRootPath();
      if (backupPath.has_value() &&
          ensureDirExistsAndWritable(backupPath.value())) {
        statsRoot_ = backupPath.value();
        LOG(INFO) << "Using backup path for stats logging: " << statsRoot_;
      } else {
        const char* pathStr =
            backupPath.has_value() ? backupPath->c_str() : "empty";
        LOG(WARNING) << "Can't create or chmod path: " << pathStr
                     << ", disabling stats logging";
        return false;
      }
    } else {
      LOG(WARNING) << "Can't create or chmod " << router_.opts().stats_root
                   << ", disabling stats logging";
      return false;
    }
  }

  boost::filesystem::path path(statsRoot_);
  path /= getStatPrefix(router_.opts()) + "." + kStatsStartupOptionsSfx;
  auto pathStr = path.string();
  if (std::find(
          touchStatsFilepaths_.begin(), touchStatsFilepaths_.end(), pathStr) ==
      touchStatsFilepaths_.end()) {
    touchStatsFilepaths_.push_back(std::move(pathStr));
  }

  auto scheduler = router_.functionScheduler();
  if (!scheduler) {
    MC_LOG_FAILURE(
        router_.opts(),
        memcache::failure::Category::kSystemError,
        "Scheduler not available, disabling stats logging");
    return false;
  }
  scheduler->addFunction(
      [this]() { log(); },
      std::chrono::milliseconds(router_.opts().stats_logging_interval),
      functionHandle_);

  return true;
}

void McrouterLogger::stop() noexcept {
  if (auto scheduler = router_.functionScheduler()) {
    scheduler->cancelFunctionAndWait(functionHandle_);
  }
}

void McrouterLogger::logStartupOptions() {
  auto json_options = folly::toDynamic(router_.getStartupOpts());
  json_options["pid"] = folly::to<std::string>(getpid());
  insertCustomStartupOpts(json_options);
  write_stats_file(
      router_.opts(), statsRoot_, kStatsStartupOptionsSfx, json_options);
}

void McrouterLogger::log() {
  if (!loggedStartupOptions_) {
    logStartupOptions();
    loggedStartupOptions_ = true;
  }

  std::vector<stat_t> stats(num_stats);
  prepare_stats(router_, stats.data());
  append_pool_stats(router_, stats);

  folly::dynamic requestStats(folly::dynamic::object());
  for (size_t i = 0; i < router_.opts().num_proxies; ++i) {
    const auto proxyRequestStats =
        router_.getProxyBase(i)->dumpRequestStats(true /* filterZeroes */);
    for (const auto& k : proxyRequestStats.keys()) {
      requestStats.setDefault(k, 0) += proxyRequestStats[k];
    }
  }
  /* Add standalone stats (does not filter zeroes) */
  {
    const auto externalRequestStats =
        router_.externalStatsHandler().dumpStats(false /* filterZeroes */);
    requestStats.update_missing(externalRequestStats);
  }

  for (int i = 0; i < num_stats; ++i) {
    if (stats[i].group & rate_stats) {
      stats[i].type = stat_double;
      folly::make_atomic_ref(stats[i].data.dbl)
          .store(
              stats_aggregate_rate_value(router_, i),
              std::memory_order_relaxed);
    } else if (stats[i].group & max_stats) {
      stats[i].type = stat_uint64;
      folly::make_atomic_ref(stats[i].data.uint64)
          .store(
              stats_aggregate_max_value(router_, i), std::memory_order_relaxed);
    } else if (stats[i].group & max_max_stats) {
      stats[i].type = stat_uint64;
      folly::make_atomic_ref(stats[i].data.uint64)
          .store(
              stats_aggregate_max_value(router_, i), std::memory_order_relaxed);
    }
  }

  write_stats_to_disk(router_.opts(), statsRoot_, stats, requestStats);
  write_config_sources_info_to_disk(router_, statsRoot_);

  for (const auto& filepath : touchStatsFilepaths_) {
    touchFile(filepath);
  }

  if (additionalLogger_) {
    additionalLogger_->log(stats);
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
