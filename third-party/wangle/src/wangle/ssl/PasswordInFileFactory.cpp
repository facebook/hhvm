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

#include <wangle/ssl/PasswordInFileFactory.h>

namespace wangle {
std::shared_ptr<folly::PasswordInFile>
PasswordInFileFactory::getPasswordCollector(const std::string& passwordPath) {
  // Check if we've got one saved
  auto it = collectors_.find(passwordPath);
  if (it != collectors_.end()) {
    return it->second;
  }

  // No saved one, make a new one.
  auto sslPassword = std::make_shared<folly::PasswordInFile>(passwordPath);
  collectors_[passwordPath] = sslPassword;
  return sslPassword;
}
} // namespace wangle
