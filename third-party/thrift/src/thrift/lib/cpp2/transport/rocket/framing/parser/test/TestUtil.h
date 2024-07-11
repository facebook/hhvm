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

namespace apache::thrift::rocket {
class FakeOwner {
 public:
  void handleFrame(std::unique_ptr<folly::IOBuf> buf) {
    frames_.push_back(std::move(buf));
  }
  bool incMemoryUsage(uint32_t n) {
    memoryCounter_ += n;
    return true;
  }
  void decMemoryUsage(uint32_t n) { memoryCounter_ -= n; }

  std::vector<std::unique_ptr<folly::IOBuf>> frames_{};

  uint32_t memoryCounter_ = 0;
};
} // namespace apache::thrift::rocket
