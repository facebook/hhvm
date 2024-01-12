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

#include <folly/executors/ManualExecutor.h>
#include <folly/futures/Barrier.h>
#include <wangle/client/persistence/FilePersistentCache.h>
#include <wangle/client/persistence/test/TestUtil.h>

using namespace std;
using namespace testing;
using namespace wangle;

using folly::futures::Barrier;

template <typename MutexT>
class FilePersistentCacheTest : public Test {};

using MutexTypes = ::testing::Types<std::mutex, folly::SharedMutex>;
TYPED_TEST_CASE(FilePersistentCacheTest, MutexTypes);

TYPED_TEST(FilePersistentCacheTest, stringTypesGetPutTest) {
  using KeyType = string;
  using ValType = string;
  vector<KeyType> keys = {"key1", "key2"};
  vector<ValType> values = {"value1", "value2"};
  testSimplePutGet<KeyType, ValType, TypeParam>(keys, values);
}

TYPED_TEST(FilePersistentCacheTest, basicTypeGetPutTest) {
  using KeyType = int;
  using ValType = double;
  vector<KeyType> keys = {1, 2};
  vector<ValType> values = {3.0, 4.0};
  testSimplePutGet<KeyType, ValType, TypeParam>(keys, values);
}

TYPED_TEST(FilePersistentCacheTest, stringCompositeGetPutTest) {
  using KeyType = string;
  using ValType = list<string>;
  vector<KeyType> keys = {"key1", "key2"};
  vector<ValType> values = {
      ValType({"fma", "shijin"}), ValType({"foo", "bar"})};
  testSimplePutGet<KeyType, ValType, TypeParam>(keys, values);
}

TYPED_TEST(FilePersistentCacheTest, stringNestedValGetPutTest) {
  using KeyType = string;
  using ValType = map<string, list<string>>;
  vector<KeyType> keys = {"cool", "not cool"};
  vector<ValType> values = {
      ValType({{"NYC", {"fma", "shijin"}}, {"MPK", {"ranjeeth", "dsp"}}}),
      ValType({{"MPK", {"subodh", "blake"}}, {"STL", {"pgriess"}}}),
  };
  testSimplePutGet<KeyType, ValType, TypeParam>(keys, values);
}

TYPED_TEST(FilePersistentCacheTest, stringNestedKeyValGetPutTest) {
  using KeyType = pair<string, string>;
  using ValType = map<string, list<string>>;
  vector<KeyType> keys = {
      make_pair("cool", "what the=?"),
      make_pair("not_cool", "how on *& earth?")};
  vector<ValType> values = {
      ValType({{"NYC", {"fma", "shijin kong$"}}, {"MPK", {"ranjeeth", "dsp"}}}),
      ValType({{"MPK", {"subodh", "blake"}}, {"STL", {"pgriess"}}}),
  };
  testSimplePutGet<KeyType, ValType, TypeParam>(keys, values);
}

template <typename K, typename V, typename MutexT>
void testEmptyFile() {
  string filename = getPersistentCacheFilename();
  size_t cacheCapacity = 10;
  int fd = folly::openNoInt(
      filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  EXPECT_TRUE(fd != -1);
  using CacheType = FilePersistentCache<K, V, MutexT>;
  CacheType cache(
      filename,
      PersistentCacheConfig::Builder()
          .setCapacity(cacheCapacity)
          .setSyncInterval(chrono::seconds(1))
          .build());
  EXPECT_EQ(cache.size(), 0);
  EXPECT_TRUE(folly::closeNoInt(fd) != -1);
  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

TYPED_TEST(FilePersistentCacheTest, stringTypesEmptyFile) {
  using KeyType = string;
  using ValType = string;
  testEmptyFile<KeyType, ValType, TypeParam>();
}

TYPED_TEST(FilePersistentCacheTest, stringNestedValEmptyFile) {
  using KeyType = string;
  using ValType = map<string, list<string>>;
  testEmptyFile<KeyType, ValType, TypeParam>();
}

// TODO_ranjeeth : integrity, should we sign the file somehow t3623725
template <typename K, typename V, typename MutexT>
void testInvalidFile(const std::string& content) {
  string filename = getPersistentCacheFilename();
  size_t cacheCapacity = 10;
  int fd = folly::openNoInt(
      filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  EXPECT_TRUE(fd != -1);
  EXPECT_EQ(
      folly::writeFull(fd, content.data(), content.size()), content.size());
  using CacheType = FilePersistentCache<K, V, MutexT>;
  CacheType cache(
      filename,
      PersistentCacheConfig::Builder()
          .setCapacity(cacheCapacity)
          .setSyncInterval(chrono::seconds(1))
          .build());
  EXPECT_EQ(cache.size(), 0);
  EXPECT_TRUE(folly::closeNoInt(fd) != -1);
  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

TYPED_TEST(FilePersistentCacheTest, stringTypesInvalidFile) {
  using KeyType = string;
  using ValType = string;
  testInvalidFile<KeyType, ValType, TypeParam>(string("{\"k1\":\"v1\",1}"));
}

TYPED_TEST(FilePersistentCacheTest, stringNestedValInvalidFile) {
  using KeyType = string;
  using ValType = map<string, list<string>>;
  testInvalidFile<KeyType, ValType, TypeParam>(string("{\"k1\":\"v1\"}"));
}

// TODO_ranjeeth : integrity, should we sign the file somehow t3623725
template <typename K, typename V, typename MutexT>
void testValidFile(
    const std::string& content,
    const vector<K>& keys,
    const vector<V>& values) {
  string filename = getPersistentCacheFilename();
  size_t cacheCapacity = 10;
  int fd = folly::openNoInt(
      filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  EXPECT_TRUE(fd != -1);
  EXPECT_EQ(
      folly::writeFull(fd, content.data(), content.size()), content.size());
  using CacheType = FilePersistentCache<K, V, MutexT>;
  CacheType cache(
      filename,
      PersistentCacheConfig::Builder()
          .setCapacity(cacheCapacity)
          .setSyncInterval(chrono::seconds(1))
          .build());
  EXPECT_EQ(cache.size(), keys.size());
  for (size_t i = 0; i < keys.size(); ++i) {
    EXPECT_EQ(cache.get(keys[i]).value(), values[i]);
  }
  EXPECT_TRUE(folly::closeNoInt(fd) != -1);
  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

TYPED_TEST(FilePersistentCacheTest, stringTypesValidFileTest) {
  using KeyType = string;
  using ValType = string;
  vector<KeyType> keys = {"key1", "key2"};
  vector<ValType> values = {"value1", "value2"};
  std::string content = "[[\"key1\",\"value1\"], [\"key2\",\"value2\"]]";
  testValidFile<KeyType, ValType, TypeParam>(content, keys, values);
}

TYPED_TEST(FilePersistentCacheTest, basicEvictionTest) {
  string filename = getPersistentCacheFilename();
  {
    size_t cacheCapacity = 10;
    FilePersistentCache<int, int, TypeParam> cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(chrono::seconds(1))
            .build());
    for (int i = 0; i < 10; ++i) {
      cache.put(i, i);
    }
    EXPECT_EQ(cache.size(), 10); // MRU to LRU : 9, 8, ...1, 0

    cache.put(10, 10); // evicts 0
    EXPECT_EQ(cache.size(), 10);
    EXPECT_FALSE(cache.get(0).hasValue());
    EXPECT_EQ(cache.get(10).value(), 10); // MRU to LRU : 10, 9, ... 2, 1

    EXPECT_EQ(cache.get(1).value(), 1); // MRU to LRU : 1, 10, 9, ..., 3, 2
    cache.put(11, 11); // evicts 2
    EXPECT_EQ(cache.size(), 10);
    EXPECT_FALSE(cache.get(2).hasValue());
    EXPECT_EQ(cache.get(11).value(), 11);
  }

  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

// serialization has changed, so test if things will be alright afterwards
TYPED_TEST(FilePersistentCacheTest, backwardCompatiblityTest) {
  using KeyType = string;
  using ValType = string;

  // write an old style file
  vector<KeyType> keys = {"key1", "key2"};
  vector<ValType> values = {"value1", "value2"};
  std::string content = "{\"key1\":\"value1\", \"key2\":\"value2\"}";
  using CacheType = FilePersistentCache<KeyType, ValType, TypeParam>;
  string filename = getPersistentCacheFilename();
  size_t cacheCapacity = 10;
  int fd = folly::openNoInt(
      filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  EXPECT_TRUE(fd != -1);
  EXPECT_EQ(
      folly::writeFull(fd, content.data(), content.size()), content.size());
  EXPECT_TRUE(folly::closeNoInt(fd) != -1);

  {
    // it should fail to load
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(chrono::seconds(1))
            .build());
    EXPECT_EQ(cache.size(), 0);

    // .. but new entries should work
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get("key1").value(), "value1");
    EXPECT_EQ(cache.get("key2").value(), "value2");
  }
  {
    // new format persists
    CacheType cache(
        filename,
        PersistentCacheConfig::Builder()
            .setCapacity(cacheCapacity)
            .setSyncInterval(chrono::seconds(1))
            .build());
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get("key1").value(), "value1");
    EXPECT_EQ(cache.get("key2").value(), "value2");
  }
  EXPECT_TRUE(unlink(filename.c_str()) != -1);
}

TYPED_TEST(FilePersistentCacheTest, destroyBeforeLoaded) {
  using CacheType = FilePersistentCache<int, int, TypeParam>;
  std::string cacheFile = getPersistentCacheFilename();
  auto config = PersistentCacheConfig::Builder()
                    .setCapacity(10)
                    .setSyncInterval(std::chrono::seconds(3))
                    .setInlinePersistenceLoading(false)
                    .setExecutor(std::make_shared<folly::ManualExecutor>())
                    .build();
  auto cache1 = std::make_unique<CacheType>(cacheFile, config);
  cache1.reset();
  auto cache2 = std::make_unique<CacheType>(cacheFile, config);
  cache2.reset();
}

TYPED_TEST(FilePersistentCacheTest, destroyAfterLoaded) {
  using CacheType = FilePersistentCache<int, int, TypeParam>;
  std::string cacheFile = getPersistentCacheFilename();
  auto config = PersistentCacheConfig::Builder()
                    .setCapacity(10)
                    .setSyncInterval(std::chrono::seconds(3))
                    .build();
  auto cache1 = std::make_unique<CacheType>(cacheFile, config);
  cache1.reset();
  auto cache2 = std::make_unique<CacheType>(cacheFile, config);
  cache2.reset();
}

TYPED_TEST(FilePersistentCacheTest, threadstress) {
  // make sure no errors crop up when hitting the same cache
  // with multiple threads

  // spin up a few of threads that add and remove
  // their own set of values on the same cache
  using CacheType = FilePersistentCache<string, int, TypeParam>;
  auto cacheFile = getPersistentCacheFilename();
  auto sharedCache = std::make_unique<CacheType>(
      cacheFile,
      PersistentCacheConfig::Builder()
          .setCapacity(10)
          .setSyncInterval(std::chrono::seconds(10))
          .build());

  int numThreads = 3;
  Barrier b(numThreads);
  auto threadFunc = [&b, numThreads](CacheType* cache, int tNum) {
    auto key1 = folly::to<string>("key", tNum);
    auto nextNum = (tNum + 1) % numThreads;
    auto key2 = folly::to<string>("key", nextNum);
    b.wait().get();
    for (int i = 0; i < 100; ++i) {
      cache->put(key1, i);
      cache->put(key2, i);
      cache->remove(key1);
      cache->remove(key2);
    }
    // wait til everyone is done
    b.wait().get();
    cache->put(key1, tNum);
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < numThreads; ++i) {
    threads.push_back(std::thread(threadFunc, sharedCache.get(), i));
  }
  for (int i = 0; i < numThreads; ++i) {
    threads[i].join();
  }
  EXPECT_EQ(sharedCache->size(), numThreads);
  for (auto i = 0; i < numThreads; ++i) {
    auto key = folly::to<string>("key", i);
    auto val = sharedCache->get(key);
    EXPECT_TRUE(val.hasValue());
    EXPECT_EQ(*val, i);
  }
}
