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

#include <wangle/util/MultiFilePoller.h>

#include <folly/FileUtil.h>
#include <folly/String.h>
#include <algorithm>

using namespace folly;

namespace wangle {

MultiFilePoller::MultiFilePoller(std::chrono::milliseconds pollInterval)
    : poller_(pollInterval) {}

size_t MultiFilePoller::getNextCallbackId() {
  size_t ret = lastCallbackId_;
  // Order of callback cancellation is arbitrary, so the next available
  // callbackId might not be lastCallbackId_+1.
  while (++ret != lastCallbackId_) {
    if (idsToCallbacks_.find(ret) == idsToCallbacks_.end()) {
      lastCallbackId_ = ret; // Assume write lock is acquired.
      return ret;
    }
  }
  throw std::runtime_error("Run out of callback ID.");
}

MultiFilePoller::CallbackId MultiFilePoller::registerFile(
    std::string path,
    Callback cb) {
  return registerFiles({std::move(path)}, std::move(cb));
}

MultiFilePoller::CallbackId MultiFilePoller::registerFiles(
    const std::vector<std::string>& paths,
    Callback cb) {
  VLOG(4) << "registerFiles({" << join(", ", paths) << "}, cb=" << &cb << ")";
  if (paths.empty()) {
    throw std::invalid_argument("Argument paths must be non-empty.");
  }
  StringReferences cbPaths;
  size_t cbId;
  {
    std::unique_lock wh(rwlock_);
    cbId = getNextCallbackId();
    // Create the bi-directional relation between path and callback.
    for (const auto& path : paths) {
      pathsToCallbackIds_[path].push_back(cbId);
      // Use reference to key of pathsToCallbackIds_ map to avoid duplicates.
      const auto& key = pathsToCallbackIds_.find(path)->first;
      cbPaths.push_back(key);
    }
    idsToCallbacks_.emplace(cbId, CallbackDetail(cbPaths, std::move(cb)));
  }
  // Release rwlock before acquiring filepoller mutex.
  for (const auto& path : cbPaths) {
    const std::string pathCopy = path;
    poller_.addFileToTrack(path, [this, path] { onFileUpdated(path); });
  }

  return MultiFilePoller::CallbackId(cbId);
}

void MultiFilePoller::cancelCallback(const CallbackId& cbId) {
  std::vector<std::string> pathsToErase;
  {
    std::unique_lock wh(rwlock_);

    auto pos = idsToCallbacks_.find(cbId.id_);
    if (pos == idsToCallbacks_.end()) {
      throw std::out_of_range(
          to<std::string>("Callback ", cbId.id_, " not found"));
    }

    // Remove the callback ID from its registered paths.
    for (const auto& path : pos->second.files_) {
      auto& callbackIds = pathsToCallbackIds_[path];
      callbackIds.erase(
          std::remove(callbackIds.begin(), callbackIds.end(), cbId.id_));
      // If the path has no more callbacks, erase it from map.
      if (callbackIds.empty()) {
        pathsToErase.emplace_back(path);
      }
    }

    // Remove the callback.
    idsToCallbacks_.erase(cbId.id_);
    // Remove callback-less paths from pathsToCallbackIds_, if any, at last.
    for (const auto& path : pathsToErase) {
      pathsToCallbackIds_.erase(path);
    }
  }
  // Release rwlock_ before acquiring filepoller mutex.
  for (const auto& path : pathsToErase) {
    poller_.removeFileToTrack(path);
  }
}

void MultiFilePoller::onFileUpdated(const std::string& triggeredPath) {
  VLOG(4) << "onFileUpdated(" << triggeredPath << ").";

  // A temporary read cache. Not worth it making it permanent because
  // files do not change frequently.
  std::unordered_map<std::string, std::string> filePathsToFileContents;
  SharedMutex::ReadHolder rh(rwlock_);

  const auto& callbacks = pathsToCallbackIds_.find(triggeredPath);
  if (callbacks == pathsToCallbackIds_.end()) {
    return;
  }

  for (const auto& cbId : callbacks->second) {
    const auto& cbEnt = idsToCallbacks_.find(cbId);
    // Lazily read all files needed by the callback.
    for (const auto& path : cbEnt->second.files_) {
      if (filePathsToFileContents.find(path) == filePathsToFileContents.end()) {
        std::string data;
        if (readFile(path.get().c_str(), data)) {
          filePathsToFileContents.emplace(path, std::move(data));
        } else {
          VLOG(4) << "Failed to read file " << path.get();
        }
      }
    }
    cbEnt->second.cb_(filePathsToFileContents);
  }
}

} // namespace wangle
