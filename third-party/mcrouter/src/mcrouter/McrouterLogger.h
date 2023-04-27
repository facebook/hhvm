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
  virtual ~AdditionalLoggerIf() {}

  virtual void log(const std::vector<stat_t>& stats) = 0;
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
