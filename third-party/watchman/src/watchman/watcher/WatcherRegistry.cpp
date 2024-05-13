/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>

#include "watchman/watcher/WatcherRegistry.h"

#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/Logging.h"
#include "watchman/QueryableView.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/watcher/Watcher.h"

using namespace watchman;

WatcherRegistry::WatcherRegistry(std::string name, Init init, int priority)
    : name_(std::move(name)), init_(std::move(init)), pri_(priority) {
  registerFactory(*this);
}

std::unordered_map<std::string, WatcherRegistry>&
WatcherRegistry::getRegistry() {
  // Meyers singleton
  static std::unordered_map<std::string, WatcherRegistry> registry;
  return registry;
}

void WatcherRegistry::registerFactory(const WatcherRegistry& factory) {
  auto& reg = getRegistry();
  reg.emplace(factory.name_, factory);

  auto capname = fmt::format("watcher-{}", factory.name_);
  capability_register(capname.c_str());
}

const WatcherRegistry* WatcherRegistry::getWatcherByName(
    const std::string& name) {
  auto& reg = getRegistry();
  const auto& it = reg.find(name);
  if (it == reg.end()) {
    return nullptr;
  }
  return &it->second;
}

// Helper to DRY in the two success paths in the function below
static inline std::shared_ptr<watchman::QueryableView> reportWatcher(
    const std::string& watcherName,
    const w_string& root_path,
    std::shared_ptr<watchman::QueryableView>&& watcher) {
  if (!watcher) {
    throw std::runtime_error(fmt::format(
        "watcher {} returned nullptr, but should throw an exception"
        " to correctly report initialization issues",
        watcherName));
  }
  watchman::log(
      watchman::ERR,
      "root ",
      root_path,
      " using watcher mechanism ",
      watcher->getName(),
      " (",
      watcherName,
      " was requested)\n");
  return std::move(watcher);
}

std::shared_ptr<watchman::QueryableView> WatcherRegistry::initWatcher(
    const w_string& root_path,
    const w_string& fstype,
    const Configuration& config) {
  std::string failureReasons;
  std::string watcherName = config.getString("watcher", "auto");

  if (watcherName != "auto") {
    // If they asked for a specific one, let's try to find it
    auto watcher = getWatcherByName(watcherName);

    if (!watcher) {
      failureReasons.append(
          std::string("no watcher named ") + watcherName + std::string(". "));
    } else {
      try {
        return reportWatcher(
            watcherName, root_path, watcher->init_(root_path, fstype, config));
      } catch (const std::exception& e) {
        failureReasons.append(
            watcherName + std::string(": ") + e.what() + std::string(". "));
      }
    }
  }

  // If we get here, let's do auto-selection; build up a list of the
  // watchers that we didn't try already...
  std::vector<const WatcherRegistry*> watchers;
  for (const auto& it : getRegistry()) {
    if (it.first != watcherName) {
      watchers.emplace_back(&it.second);
    }
  }

  // ... and sort with the highest priority first
  std::sort(
      watchers.begin(),
      watchers.end(),
      [](const WatcherRegistry* a, const WatcherRegistry* b) {
        return a->pri_ > b->pri_;
      });

  // and then work through them, taking the first one that sticks
  for (auto& watcher : watchers) {
    try {
      watchman::log(
          watchman::DBG,
          "attempting to use watcher ",
          watcher->getName(),
          " on ",
          root_path,
          "\n");
      return reportWatcher(
          watcherName, root_path, watcher->init_(root_path, fstype, config));
    } catch (const watchman::TerminalWatcherError& e) {
      failureReasons.append(
          watcher->getName() + std::string(": ") + e.what() +
          std::string(". "));
      // Don't continue our attempt to use other registered watchers
      // in this case
      break;
    } catch (const RootNotConnectedError& rnce) {
      // When Eden watcher is detected, but fails to resolve, we do
      // not attempt to use other registered watchers. Rather, we
      // fail gracefully by throwing RootNotConnectedError.
      watchman::log(
          watchman::DBG,
          "failed to use watcher ",
          watcher->getName(),
          ": ",
          rnce.what(),
          ".\n");
      throw;
    } catch (const std::exception& e) {
      watchman::log(
          watchman::ERR,
          "failed to use watcher ",
          watcher->getName(),
          ": ",
          e.what(),
          ".\n");
      failureReasons.append(
          watcher->getName() + std::string(": ") + e.what() +
          std::string(". "));
    }
  }

  // Nothing worked, report the errors
  throw std::runtime_error(failureReasons);
}

/* vim:ts=2:sw=2:et:
 */
