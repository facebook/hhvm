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

//
#pragma once

#include <chrono>

#include <folly/FBString.h>
#include <folly/json/DynamicConverter.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace wangle {

struct SSLSessionCacheData {
  folly::fbstring sessionData;
  std::chrono::time_point<std::chrono::system_clock> addedTime;
  folly::fbstring serviceIdentity;
  std::shared_ptr<SSL_SESSION> sessionDuplicateTemplate;
  folly::fbstring peerIdentities;
};

} // namespace wangle

namespace folly {
template <>
folly::dynamic toDynamic(const wangle::SSLSessionCacheData& d);
template <>
wangle::SSLSessionCacheData convertTo(const dynamic& d);
} // namespace folly
