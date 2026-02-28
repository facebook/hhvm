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

#include <folly/Random.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <wangle/ssl/SSLSessionCacheManager.h>

using namespace folly;
using namespace wangle;

TEST(ShardedLocalSSLSessionCacheTest, TestHash) {
  uint32_t buckets = 10;
  uint32_t cacheSize = 20;
  uint32_t cacheCullSize = 100;

  std::array<uint8_t, 32> id;
  Random::secureRandom(id.data(), id.size());

  ShardedLocalSSLSessionCache cache(buckets, cacheSize, cacheCullSize);
  cache.hash(std::string((char*)id.data(), id.size()));
}
