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

#include <optional>
#include <thrift/lib/cpp2/server/InterceptorData.h>

namespace apache::thrift {

std::optional<ThriftInterceptorToken>
InterceptorData::getThriftInterceptorToken(std::string_view name) {
  auto it = indices_.find(name);
  if (it == indices_.end()) {
    return std::nullopt;
  } else {
    auto token = ThriftInterceptorToken();
    token.interceptorData_ = this;
    token.index_ = it->second;
    return token;
  }
}

ThriftInterceptorToken InterceptorData::generateThriftInterceptorToken(
    std::string_view name) {
  auto token = ThriftInterceptorToken();
  token.interceptorData_ = this;
  auto it = indices_.find(name);
  if (it == indices_.end()) {
    token.index_ = indices_.size();
    indices_.emplace(name, token.index_);
  } else {
    token.index_ = it->second;
  }
  return token;
}

} // namespace apache::thrift
