/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <chrono>
#include <unordered_map>

#include <folly/Function.h>
#include <folly/SharedMutex.h>
#include <wangle/util/FilePoller.h>

namespace wangle {

/**
 * An extension to wangle::FilePoller with the ability to register one or more
 * callback on a file, and to track one or more file in a callback, and to
 * deliver cached file data to callbacks.
 */
class MultiFilePoller {
 public:
  /**
   * A callback:
   *   (1) takes as argument a map from a file path to its latest content.
           Unreadable paths will not show up in the map.
   *   (2) is triggered when any file it registers is changed,
   *       in the context of FilePoller thread.
   *   (3) once registered, cannot have its file list or callback pointer
   *       changed. To make changes, cancel the existing one and register
   *       a new one.
   * Caveat:
   *   (1) If there are N files registered, in worst case the first N-1
   *       callbacks would not get latest content of all N files. It's up to the
   *       callback to determine what to do for those cases.
   *   (2) Once a file, say, F, changes, not only F, but all files needed to
   *       trigger all callbacks that use F will be read. For example,
   *         Callback A needs files {a, b};
   *         Callback B needs files {b, c};
   *         Callback C needs files {d, e};
   *         File {a} changes -> Reads {a, b} -> Calls A(a, b).
   *         File {b} changes -> Reads {a, b, c} -> Calls A(a, b), B(b, c).
   *         File {c} changes -> Reads {b, c} -> Calls B(b, c).
   *         File {d} or {e} changes -> Reads {d, e} -> Calls C(d, e).
   */
  using CallbackArg = std::unordered_map<std::string, std::string>;
  using Callback = folly::Function<void(const CallbackArg& newData) noexcept>;

  /**
   * Type of callback identifier.
   * Use a final class to prevent conversion or modification.
   */
  class CallbackId final {
   private:
    explicit CallbackId(size_t id) : id_(id) {}
    friend class MultiFilePoller;
    size_t id_;
  };

  /**
   * @param pollInterval Interval between polls. Setting a value less than 1_s
   *   may cause undesirable behavior because the minimum granularity for
   *   wangle::FilePoller to detect mtime difference is 1_s.
   */
  explicit MultiFilePoller(std::chrono::milliseconds pollInterval);

  ~MultiFilePoller() = default;

  /**
   * Add a callback to trigger when the specified file changes.
   * @param path The path to monitor.
   * @param cb The callback to trigger.
   * @return An ID of the callback, used for cancellation.
   */
  CallbackId registerFile(std::string path, Callback cb);

  /**
   * Add a callback to trigger when any of the files in paths changes.
   * @param paths The list of paths to monitor. Must be non-empty.
   * @param cb The callback to trigger.
   @ return An ID of the callback, used for cancellation.
   */
  CallbackId registerFiles(const std::vector<std::string>& paths, Callback cb);

  /**
   * Cancel the specified Callback. May throw std::out_of_range if not found.
   * @param cbId ID of the callback returned when registering it.
   */
  void cancelCallback(const CallbackId& cbId);

 private:
  /**
   * The callback dispatcher to be registered to wangle::FilePoller.
   */
  void onFileUpdated(const std::string& triggeredPath);

  /**
   * Find an unused size_t value as callback Id. Caller must acquire wlock.
   */
  size_t getNextCallbackId();

  using StringReferences =
      std::vector<std::reference_wrapper<const std::string>>;

  struct CallbackDetail {
    CallbackDetail(StringReferences files, Callback cb)
        : files_(std::move(files)), cb_(std::move(cb)) {}
    StringReferences files_;
    Callback cb_;
  };

  // The following data structures are protected by the mutex.
  mutable folly::SharedMutex rwlock_;
  size_t lastCallbackId_ = 0;
  std::unordered_map<std::string, std::vector<size_t>> pathsToCallbackIds_;
  std::unordered_map<size_t, CallbackDetail> idsToCallbacks_;

  // The following data structures are set by ctor only.
  wangle::FilePoller poller_;
};

} // namespace wangle
