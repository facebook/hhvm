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

#include <wangle/client/persistence/FilePersistenceLayer.h>

#include <exception>

#include <folly/FileUtil.h>
#include <folly/json/json.h>
#include <folly/portability/Unistd.h>

namespace wangle {

bool FilePersistenceLayer::persist(const folly::dynamic& dynObj) noexcept {
  std::string serializedCache;
  try {
    folly::json::serialization_opts opts;
    opts.allow_non_string_keys = true;
    serializedCache = folly::json::serialize(dynObj, opts);
  } catch (...) {
    LOG(ERROR) << "Serializing to JSON failed with error: "
               << folly::exceptionStr(std::current_exception());
    return false;
  }
  bool persisted = false;
  const auto fd =
      folly::openNoInt(file_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd == -1) {
    return false;
  }
  const auto nWritten =
      folly::writeFull(fd, serializedCache.data(), serializedCache.size());
  persisted = nWritten >= 0 &&
      (static_cast<size_t>(nWritten) == serializedCache.size());
  if (!persisted) {
    LOG(ERROR) << "Failed to write to " << file_ << ":";
    if (nWritten == -1) {
      LOG(ERROR) << "write failed with errno " << errno;
    }
  }
  if (folly::fdatasyncNoInt(fd) != 0) {
    LOG(ERROR) << "Failed to sync " << file_ << ": errno " << errno;
    persisted = false;
  }
  if (folly::closeNoInt(fd) != 0) {
    LOG(ERROR) << "Failed to close " << file_ << ": errno " << errno;
    persisted = false;
  }
  return persisted;
}

folly::Optional<folly::dynamic> FilePersistenceLayer::load() noexcept {
  std::string serializedCache;
  // not being able to read the backing storage means we just
  // start with an empty cache. Failing to deserialize, or write,
  // is a real error so we report errors there.
  try {
    if (!folly::readFile(file_.c_str(), serializedCache)) {
      return folly::none;
    }

    folly::json::serialization_opts opts;
    opts.allow_non_string_keys = true;
    return folly::parseJson(serializedCache, opts);
  } catch (...) {
    LOG(ERROR) << "Deserialization of cache file " << file_
               << "failed: " << folly::exceptionStr(std::current_exception());
    return folly::none;
  }
}

void FilePersistenceLayer::clear() {
  // This may fail but it's ok
  ::unlink(file_.c_str());
}

} // namespace wangle
