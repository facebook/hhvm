/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstanceBase.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

#include <boost/filesystem/operations.hpp>
#include <fmt/format.h>

#include <folly/FileUtil.h>
#include <folly/Singleton.h>

#include "mcrouter/AsyncWriter.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyConfigBuilder.h"
#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

struct CarbonRouterInstanceBaseFunctionSchedulerTag {};
folly::Singleton<
    folly::FunctionScheduler,
    CarbonRouterInstanceBaseFunctionSchedulerTag>
    globalFunctionScheduler([]() {
      auto scheduler = std::make_unique<folly::FunctionScheduler>();
      scheduler->start();
      scheduler->setThreadName("carbon-global-scheduler");
      return scheduler.release();
    });

struct CarbonRouterLoggingAsyncWriter {};
folly::Singleton<AsyncWriter, CarbonRouterLoggingAsyncWriter>
    sharedLoggingAsyncWriter([]() {
      // Queue size starts at 1, we'll make it unlimited if requested.
      auto writer = std::make_unique<AsyncWriter>(1);
      if (!writer->start("mcrtr-statsw")) {
        throw std::runtime_error("Failed to spawn async stats logging thread");
      }
      return writer.release();
    });

struct CarbonRouterAsyncWriter {};
folly::Singleton<AsyncWriter, CarbonRouterAsyncWriter> sharedAsyncWriter([]() {
  auto writer = std::make_unique<AsyncWriter>();
  if (!writer->start("mcrtr-awriter")) {
    throw std::runtime_error("Failed to spawn mcrouter awriter thread");
  }
  return writer.release();
});

std::string statsUpdateFunctionName(folly::StringPiece routerName) {
  static std::atomic<uint64_t> uniqueId(0);
  return folly::to<std::string>(
      "carbon-stats-update-fn-", routerName, "-", uniqueId.fetch_add(1));
}

bool isDumpPreprocessedConfigEnabled(const McrouterOptions& opts) {
  if (!opts.dump_preprocessed_config_enabled) {
    return false;
  }
  if (opts.config_dump_root.empty()) {
    return false;
  }
  if (opts.service_name.empty() || opts.router_name.empty()) {
    return false;
  }
  LOG(WARNING) << "Dump preprocess config is enabled.";
  return true;
}

McrouterOptions finalizeOpts(McrouterOptions&& opts) {
  facebook::memcache::mcrouter::finalizeOptions(opts);
  return std::move(opts);
}

boost::filesystem::path getBackupConfigDirectory(const McrouterOptions& opts) {
  return boost::filesystem::path(opts.config_dump_root) / opts.service_name /
      opts.router_name;
}

std::string getPpcFilename(
    const std::string& serviceName,
    const std::string& flavorName,
    std::optional<int64_t> timestampMs = std::nullopt) {
  if (timestampMs.has_value()) {
    return fmt::format(
        "libmcrouter.{}.{}.ppc_{}.json", serviceName, flavorName, *timestampMs);
  }
  return fmt::format("libmcrouter.{}.{}.ppc.json", serviceName, flavorName);
}

// Returns the count of timestamped PPC files and the path to the oldest one.
// Used for cleaning up old timestamped config backups.
std::pair<size_t, boost::filesystem::path> getTimestampedPpcFiles(
    const boost::filesystem::path& directory,
    const std::string& serviceName,
    const std::string& flavorName) {
  std::string basePattern =
      fmt::format("libmcrouter.{}.{}.ppc_", serviceName, flavorName);

  size_t fileCount = 0;
  int64_t oldestTimestamp = std::numeric_limits<int64_t>::max();
  boost::filesystem::path oldestFile;

  if (!boost::filesystem::exists(directory) ||
      !boost::filesystem::is_directory(directory)) {
    return {fileCount, oldestFile};
  }

  boost::filesystem::directory_iterator dirIterator(directory);
  boost::filesystem::directory_iterator endIterator;
  for (; dirIterator != endIterator; ++dirIterator) {
    const auto& entry = *dirIterator;
    if (!boost::filesystem::is_regular_file(entry.path())) {
      continue;
    }
    std::string fname = entry.path().filename().string();
    if (fname.find(basePattern) != 0 || !fname.ends_with(".json")) {
      continue;
    }
    // Extract timestamp from filename
    std::string timestampStr = fname.substr(
        basePattern.length(),
        fname.length() - basePattern.length() - 5); // -5 for ".json"
    try {
      int64_t ts = std::stoll(timestampStr);
      fileCount++;
      // Track the oldest file
      if (ts < oldestTimestamp) {
        oldestTimestamp = ts;
        oldestFile = entry.path();
      }
    } catch (const std::exception&) {
      // Skip files with invalid timestamp format
      continue;
    }
  }

  return {fileCount, oldestFile};
}

bool ensureConfigDirectoryExists(const boost::filesystem::path& directory) {
  if (directory.empty() || boost::filesystem::exists(directory)) {
    return true;
  }
  if (ensureConfigDirectoryExists(directory.parent_path())) {
    auto result = ensureDirExistsAndWritableOrReturnError(directory.string());
    if (!result.hasError()) {
      return true;
    }
    LOG(WARNING) << "Failed to create directory '" << directory
                 << "': " << result.error().what();
  } else {
    VLOG(1) << "Parent directory '" << directory.parent_path()
            << "' does not exist.";
  }
  return false;
}

template <class Tag>
void logFailureEveryN(
    const McrouterOptions& opts,
    const char* category,
    const std::string& msg,
    int n) {
  static thread_local uint64_t count = 0;
  if ((count++ % n) == 0) {
    MC_LOG_FAILURE(opts, category, msg);
  }
}

} // anonymous namespace

CarbonRouterInstanceBase::CarbonRouterInstanceBase(McrouterOptions inputOptions)
    : opts_(finalizeOpts(std::move(inputOptions))),
      pid_(getpid()),
      configApi_(createConfigApi(opts_)),
      rtVarsData_(std::make_shared<ObservableRuntimeVars>()),
      leaseTokenMap_(globalFunctionScheduler.try_get()),
      statsUpdateFunctionHandle_(statsUpdateFunctionName(opts_.router_name)),
      statsApi_(gMakeStatsApiHook ? gMakeStatsApiHook(*this) : nullptr) {
  if (auto statsLogger = statsLogWriter()) {
    if (opts_.stats_async_queue_length) {
      statsLogger->increaseMaxQueueSize(opts_.stats_async_queue_length);
    } else {
      statsLogger->makeQueueSizeUnlimited();
    }
  }

  if (!opts_.pool_stats_config_file.empty()) {
    try {
      folly::dynamic poolStatJson =
          readStaticJsonFile(opts_.pool_stats_config_file);
      if (poolStatJson != nullptr) {
        auto jStatsEnabledPools = poolStatJson.get_ptr("stats_enabled_pools");
        if (jStatsEnabledPools && jStatsEnabledPools->isArray()) {
          for (const auto& it : *jStatsEnabledPools) {
            if (it.isString()) {
              statsEnabledPools_.push_back(it.asString());
            } else {
              LOG(ERROR) << "Pool Name is not a string";
            }
          }
        }
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "Invalid pool-stats-config-file : " << e.what();
    }
  }
  if (opts_.ssl_service_identity_authorization_log ||
      opts_.ssl_service_identity_authorization_enforce) {
    setSvcIdentAuthCallbackFunc(
        facebook::memcache::mcrouter::getAuthChecker(opts_));
  }
}

void CarbonRouterInstanceBase::setUpCompressionDictionaries(
    std::unordered_map<uint32_t, CodecConfigPtr>&& codecConfigs) noexcept {
  if (codecConfigs.empty() || compressionCodecManager_ != nullptr) {
    return;
  }
  compressionCodecManager_ =
      std::make_unique<const CompressionCodecManager>(std::move(codecConfigs));
}

void CarbonRouterInstanceBase::setStartupOpts(
    std::unordered_map<std::string, std::string> additionalOpts) {
  DCHECK(!startupOptsInitialized_.load(std::memory_order_acquire));
  additionalStartupOpts_.insert(additionalOpts.begin(), additionalOpts.end());
  startupOptsInitialized_.store(true, std::memory_order_release);
}

std::unordered_map<std::string, std::string>
CarbonRouterInstanceBase::getStartupOpts() const {
  auto result = opts_.toDict();
  if (startupOptsInitialized_.load(std::memory_order_acquire)) {
    result.insert(additionalStartupOpts_.begin(), additionalStartupOpts_.end());
  }
  result.emplace("version", MCROUTER_PACKAGE_STRING);
  return result;
}

size_t CarbonRouterInstanceBase::nextProxyIndex() {
  std::lock_guard<std::mutex> guard(nextProxyMutex_);
  assert(nextProxy_ < opts().num_proxies);
  size_t res = nextProxy_;
  nextProxy_ = (nextProxy_ + 1) % opts().num_proxies;
  return res;
}

void CarbonRouterInstanceBase::registerForStatsUpdates() {
  if (!opts_.num_proxies) {
    return;
  }
  if (auto scheduler = functionScheduler()) {
    scheduler->addFunction(
        [this]() { updateStats(); },
        /*interval=*/std::chrono::seconds(MOVING_AVERAGE_BIN_SIZE_IN_SECOND),
        statsUpdateFunctionHandle_,
        /*startDelay=*/std::chrono::seconds(MOVING_AVERAGE_BIN_SIZE_IN_SECOND));
  }
}

void CarbonRouterInstanceBase::deregisterForStatsUpdates() {
  if (auto scheduler = functionScheduler()) {
    scheduler->cancelFunctionAndWait(statsUpdateFunctionHandle_);
  }
}

void CarbonRouterInstanceBase::updateStats() {
  const int BIN_NUM =
      (MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND /
       MOVING_AVERAGE_BIN_SIZE_IN_SECOND);
  // To avoid inconsistence among proxies, we lock all mutexes together
  std::vector<std::unique_lock<std::mutex>> statsLocks;
  statsLocks.reserve(opts_.num_proxies);
  for (size_t i = 0; i < opts_.num_proxies; ++i) {
    statsLocks.push_back(getProxyBase(i)->stats().lock());
  }

  const auto idx = statsIndex();
  for (size_t i = 0; i < opts_.num_proxies; ++i) {
    auto* const proxy = getProxyBase(i);
    proxy->stats().aggregate(idx);
    proxy->advanceRequestStatsBin();
  }
  statsIndex((idx + 1) % BIN_NUM);
}

folly::ReadMostlySharedPtr<AsyncWriter>
CarbonRouterInstanceBase::statsLogWriter() {
  return sharedLoggingAsyncWriter.try_get_fast();
}

folly::ReadMostlySharedPtr<AsyncWriter>
CarbonRouterInstanceBase::asyncWriter() {
  return sharedAsyncWriter.try_get_fast();
}

std::shared_ptr<folly::FunctionScheduler>
CarbonRouterInstanceBase::functionScheduler() {
  return globalFunctionScheduler.try_get();
}

int32_t CarbonRouterInstanceBase::getStatsEnabledPoolIndex(
    const folly::StringPiece poolName) const {
  if (statsEnabledPools_.size() == 0) {
    return -1;
  }

  int longestPrefixMatchIndex = -1;
  // Do sequential search for longest matching name. Since this is done
  // only once during the initialization and the size of the array is
  // expected to be small, linear search should be OK.
  size_t i = 0;
  for (const auto& s : statsEnabledPools_) {
    if (poolName.subpiece(0, s.length()).compare(s) == 0) {
      if ((longestPrefixMatchIndex == -1) ||
          (statsEnabledPools_[longestPrefixMatchIndex].length() < s.length())) {
        longestPrefixMatchIndex = i;
      }
    }
    ++i;
  }

  return longestPrefixMatchIndex;
}

struct PreprocessedConfigDumpTag;

void CarbonRouterInstanceBase::dumpPreprocessedConfigToDisk() {
  if (!isDumpPreprocessedConfigEnabled(opts_)) {
    return;
  }

  std::string confFile;
  std::string path;
  if (!configApi_->getConfigFile(confFile, path)) {
    LOG(WARNING) << "Can not load config from " << path
                 << " for preprocessed config dump";

    return;
  }

  try {
    // Create ProxyConfigBuilder to get preprocessed config
    ProxyConfigBuilder builder(
        opts_, *configApi_, confFile, std::string(routerInfoName()));

    const auto& preprocessedJson = builder.preprocessedConfig();
    auto jsonContent = toPrettySortedJson(preprocessedJson);

    auto directory = getBackupConfigDirectory(opts_);
    auto filename = getPpcFilename(opts_.service_name, opts_.flavor_name);
    auto filePath = (directory / filename).string();

    // Check if we need to rewrite
    auto contentHash = Md5Hash(jsonContent);
    if (contentHash != preprocessedConfigFileMD5Hash_) {
      if (atomicallyWriteFileToDisk(jsonContent, filePath)) {
        // Set proper permissions
        ensureHasPermission(filePath, 0664);
        preprocessedConfigFileMD5Hash_ = contentHash;
        VLOG(1) << "Successfully dumped preprocessed config to: " << filePath;

        // Create timestamped backup only if max_preprocessed_config_history > 0
        if (opts_.max_preprocessed_config_history > 0) {
          try {
            auto timestampMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
            auto timestampFilename = getPpcFilename(
                opts_.service_name, opts_.flavor_name, timestampMs);
            auto timestampFilePath = (directory / timestampFilename).string();

            if (atomicallyWriteFileToDisk(jsonContent, timestampFilePath)) {
              ensureHasPermission(timestampFilePath, 0664);
              VLOG(1) << "Successfully created timestamped PPC backup: "
                      << timestampFilePath;

              // Clean up old timestamped files, keep only last N (configurable)
              try {
                auto [fileCount, oldestFile] = getTimestampedPpcFiles(
                    directory, opts_.service_name, opts_.flavor_name);

                // If we have more than max_preprocessed_config_history files
                // (including the new one we just created), remove the oldest
                // one
                if (fileCount > opts_.max_preprocessed_config_history &&
                    !oldestFile.empty()) {
                  try {
                    boost::filesystem::remove(oldestFile);
                    VLOG(2) << "Removed oldest timestamped PPC file: "
                            << oldestFile;
                  } catch (const std::exception& e) {
                    LOG(WARNING)
                        << "Failed to remove oldest timestamped PPC file "
                        << oldestFile << ": " << e.what();
                  }
                }
              } catch (const std::exception& e) {
                LOG(WARNING) << "Error cleaning up old timestamped PPC files: "
                             << e.what();
              }
            } else {
              LOG(WARNING) << "Failed to create timestamped PPC backup: "
                           << timestampFilePath;
            }
          } catch (const std::exception& e) {
            LOG(WARNING) << "Error creating timestamped PPC backup: "
                         << e.what();
          }
        }
      } else {
        logFailureEveryN<PreprocessedConfigDumpTag>(
            opts_,
            memcache::failure::Category::kOther,
            fmt::format(
                "Error while dumping preprocessed config to disk. "
                "Failed to write file {}.",
                filePath),
            1000);
        ensureConfigDirectoryExists(directory);
      }
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error processing config for preprocessed dump: " << e.what();
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
