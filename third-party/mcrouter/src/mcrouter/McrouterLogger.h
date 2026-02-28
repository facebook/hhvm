/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace facebook {
namespace memcache {
namespace mcrouter {

class CarbonRouterInstanceBase;
struct stat_t;

class AdditionalLoggerIf {
 public:
  AdditionalLoggerIf() = default;
  virtual ~AdditionalLoggerIf() = default;
  AdditionalLoggerIf(const AdditionalLoggerIf&) = delete;
  AdditionalLoggerIf& operator=(const AdditionalLoggerIf&) = delete;
  AdditionalLoggerIf(AdditionalLoggerIf&&) = delete;
  AdditionalLoggerIf& operator=(AdditionalLoggerIf&&) = delete;

  virtual void log(const std::vector<stat_t>& stats) = 0;

  /**
   * Get the backup stats root path to use when the default stats_root is not
   * available. This allows platform-specific implementations (e.g., Tupperware)
   * to provide alternative paths.
   *
   * @return The backup stats root path, or nullopt if no backup is
   *         available.
   */
  virtual std::optional<std::string> getBackupStatsRootPath() const {
    return std::nullopt;
  }
};

class McrouterLogger {
 public:
  /*
   * Constructs a McrouterLogger for the specified router.
   *
   * @param router The router to log for.
   * @param additionalLogger Additional logger that is called everytime a log
   *                         is written.
   */
  explicit McrouterLogger(
      CarbonRouterInstanceBase& router,
      std::unique_ptr<AdditionalLoggerIf> additionalLogger = nullptr);

  ~McrouterLogger();

  /**
   * Starts the logger thread.
   *
   * @return True if logger thread was successfully started, false otherwise.
   */
  bool start();

  /**
   * Stops the logger thread and join it.
   * Note: this is a blocking call.
   */
  void stop() noexcept;

 private:
  CarbonRouterInstanceBase& router_;

  std::unique_ptr<AdditionalLoggerIf> additionalLogger_;

  bool loggedStartupOptions_{false};
  // Name of the periodic function registered with the function scheduler.
  const std::string functionHandle_;

  /**
   * The resolved stats root path (either the default or the TW backup path)
   */
  std::string statsRoot_;

  /**
   * File paths of stats we want to touch and keep their mtimes up-to-date
   */
  std::vector<std::string> touchStatsFilepaths_;

  /**
   * Writes router's logs.
   */
  void log();

  /**
   * Writes startup options.
   */
  void logStartupOptions();
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
