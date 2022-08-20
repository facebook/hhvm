/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "mcrouter/CallbackPool.h"
#include "mcrouter/ConfigApiIf.h"

namespace folly {
struct dynamic;
class Executor;
} // namespace folly

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

class FileDataProvider;

/**
 * Incapsulates logic of fetching configs from files.
 */
class ConfigApi : public ConfigApiIf {
 public:
  typedef std::function<void()> Callback;
  typedef CallbackPool<>::CallbackHandle CallbackHandle;
  struct PartialUpdate;

  static const char* const kFilePrefix;

  explicit ConfigApi(const McrouterOptions& opts);

  /**
   * Subscribe a callback that will be called whenever some files changed
   *
   * @return Callback handle that is used to unsubsribe. Once the handle is
   *         destroyed provided callback won't be called anymore.
   */
  CallbackHandle subscribe(Callback callback);

  /**
   * Subscribe an additional callback to run in config thread loop when
   * it is scheduled.
   */
  CallbackHandle subscribeAdditionalCallback(Callback callback);

  /**
   * Schedule to run additional callbacks in next config thread loop
   */
  void scheduleAdditionalCallback();

  /**
   * Reads config from 'path'.
   *
   * @return true on success, false otherwise
   */
  bool get(ConfigType type, const std::string& path, std::string& contents)
      override;

  bool partialReconfigurableSource(
      const std::string& configPath,
      std::string& path) override;

  /**
   * All files we 'get' after this call will be marked as 'tracked'. Once
   * we call 'subscribeToTrackedSources', we'll receive updates only for
   * 'tracked' files.
   */
  void trackConfigSources();

  /**
   * Changes set of files we're observing to 'tracked' files.
   */
  virtual void subscribeToTrackedSources();

  /**
   * Discard 'tracked' files, keep observing files we had before
   * 'trackConfigSources' call.
   */
  virtual void abandonTrackedSources();

  /**
   * Reads configuration file according to mcrouter options.
   *
   * @param[out] config Will contain contents of configuration file on success
   * @param[out] path Will contain path of configuration file we tried to read
   * @return true on success, false otherwise
   */
  bool getConfigFile(std::string& config, std::string& path) override;

  /**
   * @return dynamic object with information about files used in configuration.
   */
  virtual folly::dynamic getConfigSourcesInfo();

  /**
   * Starts observing for file changes
   */
  virtual void startObserving();

  /**
   * Allow disabling of security config parsing
   */
  virtual bool enableSecurityConfig() const {
    return true;
  }

  virtual std::string getFailureDomainStr(uint32_t /* unused */) const {
    return "";
  }

  /**
   * Stops observing for file changes
   */
  virtual void stopObserving(pid_t pid) noexcept;

  virtual std::vector<PartialUpdate> releasePartialUpdatesLocked();

  virtual bool updatePartialConfigSource(std::vector<PartialUpdate> updates);
  virtual void addPartialUpdateForTest(PartialUpdate& update);

  ~ConfigApi() override;

  /**
   * Enable a behavior that forces this class to read config from
   * backup files instead of from the original source.
   * When this feature is enabled, the original source will only be read if
   * the backup file is not found.
   *
   * NOTE: This behavior is automacally disabled after a successful config.
   *
   * @throw logic_error   When trying to enable this feature for configurations
   *                      other than the first.
   */
  void enableReadingFromBackupFiles();

  struct PartialUpdate {
    std::string tierName;
    std::string oldApString;
    std::string newApString;
    uint32_t oldFailureDomain;
    uint32_t newFailureDomain;
    int64_t version;
    std::string hostname;
    int64_t serviceId;
  };

 protected:
  const McrouterOptions& opts_;
  CallbackPool<> callbacks_;
  std::atomic<bool> tracking_;

  /**
   * Informs whether this is the first time mcrouter is being configured.
   */
  bool isFirstConfig() const;

  /**
   * @return true, if files have update since last call, false otherwise
   */
  virtual bool checkFileUpdate();

  /**
   * Save a piece of config source to disk.
   *
   * @param sourcePrefix  Where this config comes from (e.g. file).
   * @param name          Name of this peice of config.
   *                      NOTE: sourcePrefix + name should uniquely identify
   *                      this config source.
   * @param contents      The actual config.
   * @param md5OrVersion  A piece of metadata that can "uniquely" identify the
   *                      "contents" provided.
   */
  void dumpConfigSourceToDisk(
      const std::string& sourcePrefix,
      const std::string& name,
      std::string contents,
      const std::string& md5OrVersion);

  /**
   * Tells whether or not we should read config sources from backup files.
   */
  bool shouldReadFromBackupFiles() const;

  /**
   * Reads the given config source from backup file.
   *
   * @param sourcePrefix  Where this config comes from (e.g. file).
   * @param name          Name of this peice of config.
   * @param contents      Output parameter that will hold the content of the
   *                      backup file
   *
   * @return              True if the file was read successfully.
   *                      False otherwise.
   */
  bool readFromBackupFile(
      const std::string& sourcePrefix,
      const std::string& name,
      std::string& contents) const;

 private:
  struct FileInfo {
    std::string path;
    std::string md5;
    ConfigType type{ConfigType::ConfigFile};
    std::unique_ptr<FileDataProvider> provider;
    time_t lastMd5Check{0}; // last hash check in seconds since epoch
    std::string content;

    bool checkMd5Changed();
  };
  /// file path -> FileInfo
  std::unordered_map<std::string, FileInfo> fileInfos_;
  std::unordered_map<std::string, FileInfo> trackedFiles_;

  std::thread configThread_;
  std::mutex fileInfoMutex_;

  std::mutex finishMutex_;
  std::condition_variable finishCV_;
  std::atomic<bool> finish_;

  std::atomic<bool> additionalCallbacksScheduled_{false};
  CallbackPool<> additionalCallbacks_;

  // file path -> md5
  std::unordered_map<std::string, std::string> backupFiles_;

  bool isFirstConfig_{true};

  std::unique_ptr<folly::Executor> dumpConfigToDiskExecutor_;
  bool readFromBackupFiles_{false};

  void configThreadRun();

  bool readFile(const std::string& path, std::string& contents);

  void sleepForPostReconfiguration();
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
