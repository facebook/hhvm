/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstanceBase.h"

#include <memory>

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
  if (opts.dump_preprocessed_config_interval_sec == 0) {
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
      preprocessedConfigDumpExecutor_(
          isDumpPreprocessedConfigEnabled(opts_)
              ? std::make_unique<folly::ScopedEventBaseThread>("mcr-ppc-dump")
              : nullptr),
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

void CarbonRouterInstanceBase::deregisterForPreprocessedConfigDumps() {
  if (preprocessedConfigDumpExecutor_) {
    preprocessedConfigDumpExecutor_->getEventBase()->runInEventBaseThread(
        [this]() {
          if (periodicTimeout_) {
            periodicTimeout_->cancelTimeout();
          }
        });
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
struct PreprocessedConfigTouchTag;

void CarbonRouterInstanceBase::registerForPreprocessedConfigDumps() {
  if (!preprocessedConfigDumpExecutor_) {
    return;
  }

  periodicTimeout_ = folly::AsyncTimeout::make(
      *preprocessedConfigDumpExecutor_->getEventBase(), [&]() mutable noexcept {
        try {
          dumpPreprocessedConfigToDisk();
        } catch (const std::exception& e) {
          LOG(ERROR) << "Error dumping preprocessed config: " << e.what();
        }
        // Reschedule
        periodicTimeout_->scheduleTimeout(
            opts().dump_preprocessed_config_interval_sec * 1000);
      });

  // Schedule the first task
  preprocessedConfigDumpExecutor_->getEventBase()->runInEventBaseThread(
      [this]() {
        periodicTimeout_->scheduleTimeout(
            opts().dump_preprocessed_config_interval_sec * 1000);
      });
}

void CarbonRouterInstanceBase::dumpPreprocessedConfigToDisk() {
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
    auto filename = fmt::format(
        "libmcrouter.{}.{}.ppc.json", opts_.service_name, opts_.flavor_name);
    auto filePath = (directory / filename).string();

    // Check if we need to rewrite
    auto contentHash = Md5Hash(jsonContent);
    if (contentHash != preprocessedConfigFileMD5Hash_) {
      if (atomicallyWriteFileToDisk(jsonContent, filePath)) {
        // Set proper permissions
        ensureHasPermission(filePath, 0664);
        preprocessedConfigFileMD5Hash_ = contentHash;
        VLOG(1) << "Successfully dumped preprocessed config to: " << filePath;
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
    } else {
      // Content hasn't changed, just touch the file
      if (!touchFile(filePath)) {
        logFailureEveryN<PreprocessedConfigTouchTag>(
            opts_,
            memcache::failure::Category::kOther,
            fmt::format(
                "Error while touching preprocessed config file {}", filePath),
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
