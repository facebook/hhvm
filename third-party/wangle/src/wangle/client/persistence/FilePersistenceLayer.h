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

#include <folly/FileUtil.h>
#include <folly/json/json.h>
#include <folly/portability/Unistd.h>
#include <wangle/client/persistence/LRUPersistentCache.h>

namespace wangle {

class FilePersistenceLayer : public CachePersistence {
 public:
  explicit FilePersistenceLayer(const std::string& file) : file_(file) {}
  ~FilePersistenceLayer() override = default;

  bool persist(const folly::dynamic& arrayOfKvPairs) noexcept override;

  folly::Optional<folly::dynamic> load() noexcept override;

  void clear() override;

 private:
  std::string file_;
};

} // namespace wangle
