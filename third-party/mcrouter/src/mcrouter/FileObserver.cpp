/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FileObserver.h"

#include <memory>
#include <vector>

#include <glog/logging.h>

#include <folly/io/async/EventBase.h>

#include "mcrouter/FileDataProvider.h"
#include "mcrouter/McrouterLogFailure.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

struct FileObserverData {
  FileObserverData(
      const std::shared_ptr<folly::FunctionScheduler>& scheduler__,
      std::string functionName__,
      std::shared_ptr<FileDataProvider> provider__,
      std::function<void(std::string)> onUpdate__,
      std::chrono::milliseconds sleepBeforeUpdate__,
      FileObserverHandle handle)
      : scheduler(scheduler__),
        functionName(functionName__),
        provider(std::move(provider__)),
        onUpdate(std::move(onUpdate__)),
        sleepBeforeUpdate(sleepBeforeUpdate__),
        token(handle) {}
  std::weak_ptr<folly::FunctionScheduler> scheduler;
  std::string functionName;
  std::shared_ptr<FileDataProvider> provider;
  std::function<void(std::string)> onUpdate;
  std::chrono::milliseconds sleepBeforeUpdate;
  std::weak_ptr<FileObserverToken> token;
};

void checkForUpdate(const std::shared_ptr<FileObserverData>& data) {
  const auto scheduler = data->scheduler.lock();
  assert(scheduler);
  if (!scheduler) {
    // Should never happens since we're executed by the scheduler.
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kSystemError,
        "Global function scheduler not available");
    return;
  }
  const auto tokenRef = data->token.lock();
  if (!tokenRef) {
    scheduler->cancelFunction(data->functionName);
    return;
  }
  try {
    if (!data->provider->hasUpdate()) {
      return;
    }
  } catch (...) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kOther,
        "Error while observing file for update. Will stop polling for updates");
    data->token.reset();
    return;
  }

  static std::atomic<uint64_t> updateId(0);
  scheduler->addFunctionOnce(
      [data = data]() {
        try {
          if (const auto ref = data->token.lock()) {
            data->onUpdate(data->provider->load());
          }
        } catch (...) {
          LOG_FAILURE(
              "mcrouter",
              failure::Category::kOther,
              "Error while observing file for update");
        }
      },
      folly::to<std::string>(
          "carbon-file-observer-update-", updateId.fetch_add(1)),
      data->sleepBeforeUpdate);
}

} // anonymous namespace

FileObserverHandle startObservingFile(
    const std::string& filePath,
    const std::shared_ptr<folly::FunctionScheduler>& scheduler,
    std::chrono::milliseconds pollPeriod,
    std::chrono::milliseconds sleepBeforeUpdate,
    std::function<void(std::string)> onUpdate) {
  std::shared_ptr<FileDataProvider> provider;
  try {
    provider = std::make_shared<FileDataProvider>(filePath);

    onUpdate(provider->load());
  } catch (const std::exception& e) {
    VLOG(0) << "Can not start watching " << filePath
            << " for modifications: " << e.what();
    return FileObserverHandle();
  }

  VLOG(0) << "Watching " << filePath << " for modifications.";
  FileObserverHandle handle = std::make_shared<FileObserverToken>();
  static std::atomic<uint64_t> uniqueId(0);
  auto data = std::make_shared<FileObserverData>(
      scheduler,
      folly::to<std::string>("carbon-file-observer-", uniqueId.fetch_add(1)),
      std::move(provider),
      std::move(onUpdate),
      sleepBeforeUpdate,
      handle);
  scheduler->addFunction(
      [data = data]() { checkForUpdate(data); },
      pollPeriod,
      data->functionName,
      pollPeriod);
  return handle;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
