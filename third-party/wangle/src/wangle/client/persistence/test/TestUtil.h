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

#include <cstdlib>
#include <list>
#include <vector>

#include <folly/Memory.h>
#include <folly/portability/GTest.h>
#include <folly/portability/Unistd.h>
#include <wangle/client/persistence/FilePersistentCache.h>

namespace wangle {

std::string getPersistentCacheFilename();

template <typename K, typename V, typename MutexT = std::mutex>
void testSimplePutGet(
    const std::vector<K>& keys,
    const std::vector<V>& values) {
  std::string filename = getPersistentCacheFilename();
  typedef FilePersistentCache<K, V, MutexT> CacheType;
  size_t cacheCapacity = 10;
  {
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(std::chrono::seconds(150))
            .build());
    EXPECT_FALSE(cache.get(keys[0]).hasValue());
    EXPECT_FALSE(cache.get(keys[1]).hasValue());
    cache.put(keys[0], values[0]);
    cache.put(keys[1], values[1]);
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(keys[0]).value(), values[0]);
    EXPECT_EQ(cache.get(keys[1]).value(), values[1]);
  }
  {
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(std::chrono::seconds(150))
            .build());
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(keys[0]).value(), values[0]);
    EXPECT_EQ(cache.get(keys[1]).value(), values[1]);
    EXPECT_TRUE(cache.remove(keys[1]));
    EXPECT_FALSE(cache.remove(keys[1]));
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.get(keys[0]).value(), values[0]);
    EXPECT_FALSE(cache.get(keys[1]).hasValue());
  }
  {
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(std::chrono::seconds(150))
            .build());
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.get(keys[0]).value(), values[0]);
    EXPECT_FALSE(cache.get(keys[1]).hasValue());
    cache.clear();
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.get(keys[0]).hasValue());
    EXPECT_FALSE(cache.get(keys[1]).hasValue());
  }
  {
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(std::chrono::seconds(150))
            .build());
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.get(keys[0]).hasValue());
    EXPECT_FALSE(cache.get(keys[1]).hasValue());
  }
  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

} // namespace wangle
