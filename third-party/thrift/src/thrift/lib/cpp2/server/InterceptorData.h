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

#include <cstdint>
#include <string>
#include <folly/container/F14Map.h>

namespace apache::thrift {

class InterceptorData;

/**
 * Handle similar to folly::RequestToken, will be used to fetch intercepted data
 * from Cpp2RequestContext. Unlike folly::RequestToken, this type doesn't have a
 * global scope, but instead is unique per InterceptorData instance.
 * InterceptorData should be created once per ThriftServer instance.
 */
class ThriftInterceptorToken {
 public:
  ThriftInterceptorToken() = default;
  bool operator==(const ThriftInterceptorToken& other) const {
    return interceptorData_ == other.interceptorData_ && index_ == other.index_;
  }
  bool operator!=(const ThriftInterceptorToken& other) const {
    return !(*this == other);
  }
  uint32_t get() const { return index_; }

 private:
  friend class InterceptorData;
  uint32_t index_{0};
  InterceptorData* interceptorData_{nullptr};
};

class InterceptorData {
 public:
  /**
   * returns a token if it exists, otherwise returns an empty optional
   */
  std::optional<ThriftInterceptorToken> getThriftInterceptorToken(
      std::string_view name);
  /**
   * Generates a token. if the name passed to the token already exists, then
   * return the token associated with it
   */
  ThriftInterceptorToken generateThriftInterceptorToken(std::string_view name);

 private:
  folly::F14FastMap<std::string, uint32_t> indices_;
};

} // namespace apache::thrift
