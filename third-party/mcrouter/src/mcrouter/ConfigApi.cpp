/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ConfigApi.h"

#include <memory>

#include <boost/filesystem.hpp>

#include <folly/FileUtil.h>
#include <folly/Random.h>
#include <folly/String.h>
#include <folly/dynamic.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/FileDataProvider.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ThreadUtil.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

const char* const kMcrouterConfigKey = "mcrouter_config";
const char* const kConfigFile = "config_file";
const char* const kConfigImport = "config_import";
const int kConfigReloadInterval = 60;

boost::filesystem::path getBackupConfigDirectory(const McrouterOptions& opts) {
  return boost::filesystem::path(opts.config_dump_root) / opts.service_name /
      opts.router_name;
}

boost::filesystem::path getBackupConfigFileName(
    folly::StringPiece sourcePrefix,
    folly::StringPiece name) {
  sourcePrefix.removeSuffix(':');
  return boost::filesystem::path(folly::sformat(
      "{}-{}", sourcePrefix, folly::uriEscape<std::string>(name)));
}

bool ensureConfigDirectoryExists(boost::filesystem::path directory) {
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
    LOG_FIRST_N(WARNING, 10) << "Parent directory '" << directory.parent_path()
                             << "' does not exist.";
  }
  return false;
}

bool setupDumpConfigToDisk(const McrouterOptions& opts) {
  if (opts.config_dump_root.empty()) {
    return false;
  }
  if (opts.service_name.empty() || opts.router_name.empty()) {
    MC_LOG_FAILURE(
        opts,
        memcache::failure::Category::kOther,
        "Service name or router name not set. Configs won't be saved to disk.");
    return false;
  }
  if (!ensureConfigDirectoryExists(getBackupConfigDirectory(opts))) {
    MC_LOG_FAILURE(
        opts,
        memcache::failure::Category::kOther,
        "Failed to setup directory for dumping configs. "
        "Configs won't be saved to disk.");
    return false;
  }
  return true;
}

template <class Tag>
void logFailureEveryN(
    const McrouterOptions& opts,
    const char* category,
    std::string msg,
    int n) {
  static thread_local uint64_t count = 0;
  if ((count++ % n) == 0) {
    MC_LOG_FAILURE(opts, category, msg);
  }
}
struct DumpFileTag;
struct TouchFileTag;

} // anonymous namespace

const char* const ConfigApi::kFilePrefix = "file:";

ConfigApi::~ConfigApi() {
  dumpConfigToDiskExecutor_.reset();
}

ConfigApi::ConfigApi(const McrouterOptions& opts)
    : opts_(opts),
      finish_(false),
      dumpConfigToDiskExecutor_(
          setupDumpConfigToDisk(opts)
              ? std::make_unique<folly::ScopedEventBaseThread>("mcrcfgdump")
              : nullptr) {}

ConfigApi::CallbackHandle ConfigApi::subscribe(Callback callback) {
  return callbacks_.subscribe(std::move(callback));
}

ConfigApi::CallbackHandle ConfigApi::subscribeAdditionalCallback(
    Callback callback) {
  return additionalCallbacks_.subscribe(std::move(callback));
}

void ConfigApi::scheduleAdditionalCallback() {
  additionalCallbacksScheduled_ = true;
}

void ConfigApi::startObserving() {
  assert(!finish_);
  if (!opts_.disable_reload_configs) {
    configThread_ = std::thread([this]() { configThreadRun(); });
  }
}

void ConfigApi::stopObserving(pid_t pid) noexcept {
  {
    std::lock_guard<std::mutex> lk(finishMutex_);
    finish_ = true;
    finishCV_.notify_one();
  }
  if (configThread_.joinable()) {
    if (getpid() == pid) {
      configThread_.join();
    } else {
      configThread_.detach();
    }
  }
}

bool ConfigApi::checkFileUpdate() {
  auto now = nowWallSec();
  bool hasUpdate = false;
  std::lock_guard<std::mutex> lock(fileInfoMutex_);
  for (auto& fileIt : fileInfos_) {
    auto& file = fileIt.second;
    // hasUpdate reads events from inotify, so we need to poll all
    // providers to reconfigure only once when multiple files have changed
    try {
      if (file.provider) {
        hasUpdate |= file.provider->hasUpdate();
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "Check " << file.path << " for update failed: " << e.what();
      // check with hash, if it throws error something is totally wrong,
      // reconfiguration thread will log error and finish.
      file.provider.reset();
      if (file.lastMd5Check + kConfigReloadInterval < now) {
        hasUpdate |= file.checkMd5Changed();
      }
    }
  }
  return hasUpdate;
}

void ConfigApi::configThreadRun() {
  mcrouterSetThisThreadName(opts_, "mcrcfg");
  if (opts_.constantly_reload_configs) {
    while (!finish_) {
      LOG(INFO) << "Reload config due to constantly_reload_configs";
      callbacks_.notify();
      {
        std::unique_lock<std::mutex> lk(finishMutex_);
        finishCV_.wait_for(lk, std::chrono::milliseconds(10), [this] {
          return finish_.load();
        });
      }
    }
    return;
  }

  while (!finish_) {
    bool hasUpdate = false;
    try {
      hasUpdate = checkFileUpdate();
    } catch (const std::exception& e) {
      MC_LOG_FAILURE(
          opts_,
          memcache::failure::Category::kOther,
          "Check for config update failed: {}",
          e.what());
    } catch (...) {
      MC_LOG_FAILURE(
          opts_,
          memcache::failure::Category::kOther,
          "Check for config update failed with unknown error");
    }
    // There are a couple of races that can happen here
    // First, the IN_MODIFY event can be fired before the write is complete,
    // resulting in a malformed JSON error. Second, text editors may do
    // some of their own shuffling of the file (e.g. between .swp and the
    // real thing in Vim) after the write. This may can result in a file
    // access error router_configure_from_file below. That's just a theory,
    // but that error does happen. Race 1 can be fixed by changing the
    // watch for IN_MODIFY to IN_CLOSE_WRITE, but Race 2 has no apparent
    // elegant solution. The following jankiness fixes both.

    {
      std::unique_lock<std::mutex> lk(finishMutex_);
      finishCV_.wait_for(
          lk,
          std::chrono::milliseconds(opts_.reconfiguration_delay_ms),
          [this] { return finish_.load(); });
    }

    if (hasUpdate) {
      if (opts_.reconfiguration_jitter_ms) {
        int jitter_ms =
            folly::Random::randDouble01() * opts_.reconfiguration_jitter_ms;
        std::unique_lock<std::mutex> lk(finishMutex_);
        finishCV_.wait_for(
            lk,
            std::chrono::milliseconds(
                opts_.reconfiguration_delay_ms + jitter_ms),
            [this] { return finish_.load(); });
      }
      callbacks_.notify();

      // waits before checking for config updates again.
      sleepForPostReconfiguration();
    }

    if (additionalCallbacksScheduled_.exchange(false) &&
        !additionalCallbacks_.empty()) {
      additionalCallbacks_.notify();
      sleepForPostReconfiguration();
    }

    // Otherwise there was nothing to read, so check that we aren't shutting
    // down, and wait on the FD again.
  }
}

void ConfigApi::sleepForPostReconfiguration() {
  if (opts_.post_reconfiguration_delay_ms > 0) {
    std::unique_lock<std::mutex> lk(finishMutex_);
    finishCV_.wait_for(
        lk,
        std::chrono::milliseconds(opts_.post_reconfiguration_delay_ms),
        [this] { return finish_.load(); });
  }
}

bool ConfigApi::FileInfo::checkMd5Changed() {
  auto previousHash = md5;
  std::string contents;
  if (!folly::readFile(path.data(), contents)) {
    throw std::runtime_error("Error reading from config file " + path);
  }
  md5 = Md5Hash(contents);
  lastMd5Check = nowWallSec();

  return md5 != previousHash;
}

bool ConfigApi::getConfigFile(std::string& contents, std::string& path) {
  folly::StringPiece configStr = opts_.config;
  if (configStr.startsWith(kFilePrefix)) {
    path = configStr.str();
    configStr.removePrefix(kFilePrefix);
    return get(ConfigType::ConfigFile, configStr.str(), contents);
  }
  if (!configStr.empty()) {
    path = shorten(configStr, 64);
    contents = configStr.str();
    return true;
  }
  if (!opts_.config_str.empty()) {
    // explicit config, no automatic reload
    path = shorten(opts_.config_str, 64);
    contents = opts_.config_str;
    return true;
  }
  if (!opts_.config_file.empty()) {
    path = "file:" + opts_.config_file;
    return get(ConfigType::ConfigFile, opts_.config_file, contents);
  }
  return false;
}

bool ConfigApi::readFile(const std::string& path, std::string& contents) {
  bool fileRead = false;
  if (shouldReadFromBackupFiles()) {
    fileRead = readFromBackupFile(kFilePrefix, path, contents);
  }
  if (!fileRead) {
    fileRead = folly::readFile(path.data(), contents);
  }
  return fileRead;
}

bool ConfigApi::get(
    ConfigType type,
    const std::string& path,
    std::string& contents) {
  std::string fullPath;
  folly::StringPiece configPath = path;
  folly::StringPiece configOpt = opts_.config;
  if (type == ConfigType::ConfigImport) {
    if (configPath.startsWith(kFilePrefix)) {
      configPath.removePrefix(kFilePrefix);
      fullPath = configPath.str();
    } else if (!configOpt.empty()) {
      if (configOpt.startsWith(kFilePrefix)) {
        configOpt.removePrefix(kFilePrefix);
      }
      boost::filesystem::path filePath(configOpt.str());
      fullPath = (filePath.parent_path() / path).string();
    } else {
      boost::filesystem::path filePath(opts_.config_file);
      fullPath = (filePath.parent_path() / path).string();
    }
  } else if (type == ConfigType::Pool) {
    if (configPath.startsWith(kFilePrefix)) {
      configPath.removePrefix(kFilePrefix);
      fullPath = configPath.str();
    } else {
      return false;
    }
  } else {
    fullPath = path;
  }

  if (!readFile(fullPath, contents)) {
    return false;
  }

  if (tracking_) {
    std::lock_guard<std::mutex> lock(fileInfoMutex_);
    auto fileInfoIt = trackedFiles_.emplace(fullPath, FileInfo()).first;

    auto& file = fileInfoIt->second;
    file.path = fullPath;
    file.type = type;
    file.lastMd5Check = nowWallSec();
    file.md5 = Md5Hash(contents);
    file.content = contents; // copy file contents so that we can dump it later
  }
  return true;
}

bool ConfigApi::partialReconfigurableSource(const std::string&, std::string&) {
  return false;
}

void ConfigApi::trackConfigSources() {
  std::lock_guard<std::mutex> lock(fileInfoMutex_);
  tracking_ = true;
}

void ConfigApi::subscribeToTrackedSources() {
  std::lock_guard<std::mutex> lock(fileInfoMutex_);
  assert(tracking_);
  auto lastConfigFromBackupFiles = shouldReadFromBackupFiles();
  tracking_ = false;
  isFirstConfig_ = false;
  readFromBackupFiles_ = false;

  if (!opts_.disable_reload_configs) {
    // start watching for updates
    for (auto& it : trackedFiles_) {
      auto& file = it.second;
      if (!lastConfigFromBackupFiles) {
        dumpConfigSourceToDisk(kFilePrefix, file.path, file.content, file.md5);
      }
      try {
        if (!file.provider) {
          // reuse existing providers
          auto fileInfoIt = fileInfos_.find(file.path);
          if (fileInfoIt != fileInfos_.end()) {
            file.provider = std::move(fileInfoIt->second.provider);
          }
        }
        if (!file.provider) {
          file.provider = std::make_unique<FileDataProvider>(file.path);
        }
      } catch (const std::exception& e) {
        // it's not that bad, we will check for change in hash
        LOG(INFO) << "Can not start watching " << file.path
                  << " for modifications: " << e.what();
      }
    }
  }

  fileInfos_ = std::move(trackedFiles_);
  trackedFiles_.clear();
}

void ConfigApi::abandonTrackedSources() {
  std::lock_guard<std::mutex> lock(fileInfoMutex_);
  assert(tracking_);
  tracking_ = false;
  trackedFiles_.clear();
}

folly::dynamic ConfigApi::getConfigSourcesInfo() {
  folly::dynamic reply_val = folly::dynamic::object;

  // we have config="<JSON-string>", write its hash
  if (!opts_.config.empty() &&
      !folly::StringPiece(opts_.config).startsWith(kFilePrefix)) {
    reply_val[kMcrouterConfigKey] = Md5Hash(opts_.config);
  } else if (!opts_.config_str.empty()) {
    // we have config_str, write its hash
    reply_val[kMcrouterConfigKey] = Md5Hash(opts_.config_str);
  }

  std::lock_guard<std::mutex> lock(fileInfoMutex_);

  for (const auto& fileIt : fileInfos_) {
    const auto& file = fileIt.second;
    switch (file.type) {
      case ConfigType::ConfigFile:
        reply_val[kConfigFile] = file.path;
        reply_val[kMcrouterConfigKey] = file.md5;
        break;
      case ConfigType::Pool:
        if (!reply_val.count("pools")) {
          reply_val["pools"] = folly::dynamic::object;
        }
        reply_val["pools"].insert(file.path, file.md5);
        break;
      case ConfigType::ConfigImport:
        if (!reply_val.count(kConfigImport)) {
          reply_val[kConfigImport] = folly::dynamic::object;
        }
        reply_val[kConfigImport].insert(file.path, file.md5);
        break;
    }
  }

  return reply_val;
}

bool ConfigApi::isFirstConfig() const {
  return isFirstConfig_;
}

void ConfigApi::dumpConfigSourceToDisk(
    const std::string& sourcePrefix,
    const std::string& name,
    std::string contents,
    const std::string& md5OrVersion) {
  if (!dumpConfigToDiskExecutor_) {
    return;
  }

  dumpConfigToDiskExecutor_->add([this,
                                  sourcePrefix,
                                  name,
                                  contents = std::move(contents),
                                  md5OrVersion]() {
    auto directory = getBackupConfigDirectory(opts_);
    auto filePath =
        (directory / getBackupConfigFileName(sourcePrefix, name)).string();

    bool shouldRewrite = true;
    auto backupFileIt = backupFiles_.find(filePath);
    if (backupFileIt == backupFiles_.end()) {
      backupFileIt = backupFiles_.emplace(filePath, "").first;
    } else {
      shouldRewrite = (backupFileIt->second != md5OrVersion);
    }

    if (shouldRewrite) {
      if (atomicallyWriteFileToDisk(contents, filePath)) {
        // Makes sure the file has the correct permission for when we decide to
        // run mcrouter with another user.
        ensureHasPermission(filePath, 0664);
        backupFileIt->second = md5OrVersion;
      } else {
        logFailureEveryN<DumpFileTag>(
            opts_,
            memcache::failure::Category::kOther,
            folly::sformat(
                "Error while dumping last valid config to disk. "
                "Failed to write file {}.",
                filePath),
            1000);
        ensureConfigDirectoryExists(directory);
      }
    } else {
      if (!touchFile(filePath)) {
        logFailureEveryN<TouchFileTag>(
            opts_,
            memcache::failure::Category::kOther,
            folly::sformat(
                "Error while touching backup config file {}", filePath),
            1000);
        ensureConfigDirectoryExists(directory);
      }
    }
  });
}

bool ConfigApi::readFromBackupFile(
    const std::string& sourcePrefix,
    const std::string& name,
    std::string& contents) const {
  auto filePath = getBackupConfigDirectory(opts_) /
      getBackupConfigFileName(sourcePrefix, name);
  if (!boost::filesystem::exists(filePath)) {
    LOG(WARNING) << "Backup file '" << filePath << "' not found.";
    return false;
  }

  auto now = nowWallSec();
  auto lastWriteTime = boost::filesystem::last_write_time(filePath);
  if (lastWriteTime + opts_.max_dumped_config_age < now) {
    LOG(WARNING) << "Config backup '" << sourcePrefix << name
                 << "' too old to be trusted. "
                 << "Age: " << now - lastWriteTime << " seconds. "
                 << "Max age allowed: " << opts_.max_dumped_config_age
                 << " seconds.";
    return false;
  }

  VLOG(1) << "Reading config source " << sourcePrefix << name
          << " from backup.";
  return folly::readFile(filePath.c_str(), contents);
}

void ConfigApi::enableReadingFromBackupFiles() {
  if (!isFirstConfig()) {
    throw std::logic_error(
        "Reading from backup files is just allowed on the first configuration");
  }
  if (opts_.max_dumped_config_age == 0) {
    LOG(WARNING) << "Reading configs from backup is disabled "
                    "(max_dumped_config_age == 0)";
    return;
  }
  readFromBackupFiles_ = true;
  LOG(WARNING) << "Enabling read config from backup files.";
}

bool ConfigApi::shouldReadFromBackupFiles() const {
  return readFromBackupFiles_;
}

std::vector<ConfigApi::PartialUpdate> ConfigApi::releasePartialUpdatesLocked() {
  return {};
}

bool ConfigApi::updatePartialConfigSource(
    std::vector<ConfigApi::PartialUpdate> /*updates*/) {
  return true;
}

void ConfigApi::addPartialUpdateForTest(PartialUpdate&) {}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
