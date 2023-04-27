/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include <folly/experimental/FunctionScheduler.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

// File observation will continue so long as FileObserverHandle has a ref count
// greater than zero.
struct FileObserverToken {};
using FileObserverHandle = std::shared_ptr<FileObserverToken>;

/**
 * Starts a periodic thread that watches the given file path for changes.
 *
 * @param filePath path to the file to watch (can be a symlink)
 * @param pollPeriod milliseconds to wait between asking inotify if
 *        any updates happened
 * @param sleepAfterUpdate milliseconds to wait before calling onUpdate
 *        once an inotify event happens (as a crude protection against
 *        partial writes race condition).
 * @param onUpdate callback function to call when there is a update seen
 * @return handle that will disable observation once it has a ref count of zero.
 */
FileObserverHandle startObservingFile(
    const std::string& filePath,
    const std::shared_ptr<folly::FunctionScheduler>& scheduler,
    std::chrono::milliseconds pollPeriod,
    std::chrono::milliseconds sleepBeforeUpdateMs,
    std::function<void(std::string)> onUpdate);
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
