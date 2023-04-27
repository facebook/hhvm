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

#include <memory>
#include <string>
#include <unordered_map>

#include <folly/io/async/PasswordInFile.h>

namespace wangle {
/**
 * Password file caching class that provides a very simple cache. Whenever
 * a given password file path is loaded, the collector is saved and returned
 * whenever that path is requested again. It doesn't invalidate previous
 * entries on file changes; it's assumed that the contents of a given file path
 * will not change.
 */
class PasswordInFileFactory {
 public:
  std::shared_ptr<folly::PasswordInFile> getPasswordCollector(
      const std::string& passwordPath);

 private:
  std::unordered_map<std::string, std::shared_ptr<folly::PasswordInFile>>
      collectors_;
};
} // namespace wangle
