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

#include <chrono>
#include <thread>
#include <vector>

#include <folly/Memory.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/futures/Future.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>
#include <wangle/client/persistence/LRUPersistentCache.h>

using namespace folly;
using namespace std;
using namespace testing;
using namespace wangle;

using TestPersistenceLayer = CachePersistence;

using MutexTypes = ::testing::Types<std::mutex, folly::SharedMutex>;
TYPED_TEST_CASE(LRUPersistentCacheTest, MutexTypes);

template <typename T>
static shared_ptr<LRUPersistentCache<string, string, T>> createCache(
    size_t capacity,
    uint32_t syncMillis,
    std::unique_ptr<TestPersistenceLayer> persistence = nullptr,
    bool loadPersistenceInline = true) {
  using TestCache = LRUPersistentCache<string, string, T>;
  return std::make_shared<TestCache>(
      PersistentCacheConfig::Builder()
          .setCapacity(capacity)
          .setSyncInterval(std::chrono::milliseconds(syncMillis))
          .setSyncRetries(3)
          .setInlinePersistenceLoading(loadPersistenceInline)
          .build(),
      std::move(persistence));
}

template <typename T>
static shared_ptr<LRUPersistentCache<string, string, T>>
createCacheWithExecutor(
    std::shared_ptr<folly::Executor> executor,
    std::unique_ptr<TestPersistenceLayer> persistence,
    std::chrono::milliseconds syncInterval,
    int retryLimit,
    bool loadPersistenceInline = true) {
  return std::make_shared<LRUPersistentCache<string, string, T>>(
      PersistentCacheConfig::Builder()
          .setCapacity(10)
          .setExecutor(std::move(executor))
          .setSyncInterval(syncInterval)
          .setSyncRetries(retryLimit)
          .setInlinePersistenceLoading(loadPersistenceInline)
          .build(),
      std::move(persistence));
}

class MockPersistenceLayer : public TestPersistenceLayer {
 public:
  ~MockPersistenceLayer() override {
    LOG(ERROR) << "ok.";
  }
  bool persist(const dynamic& obj) noexcept override {
    return persist_(obj);
  }
  folly::Optional<dynamic> load() noexcept override {
    return load_();
  }
  CacheDataVersion getLastPersistedVersionConcrete() const {
    return TestPersistenceLayer::getLastPersistedVersion();
  }
  void setPersistedVersionConcrete(CacheDataVersion version) {
    TestPersistenceLayer::setPersistedVersion(version);
  }
  MOCK_METHOD0(clear, void());
  MOCK_METHOD1(persist_, bool(const dynamic&));
  MOCK_METHOD0(load_, folly::Optional<dynamic>());
  MOCK_CONST_METHOD0(getLastPersistedVersion, CacheDataVersion());
  MOCK_METHOD(void, setPersistedVersion, (CacheDataVersion), (noexcept));
};

template <typename MutexT>
class LRUPersistentCacheTest : public Test {
 protected:
  void SetUp() override {
    persistence = make_unique<MockPersistenceLayer>();
    ON_CALL(*persistence, getLastPersistedVersion())
        .WillByDefault(Invoke(
            persistence.get(),
            &MockPersistenceLayer::getLastPersistedVersionConcrete));
    ON_CALL(*persistence, setPersistedVersion(_))
        .WillByDefault(Invoke(
            persistence.get(),
            &MockPersistenceLayer::setPersistedVersionConcrete));
    manualExecutor = std::make_shared<folly::ManualExecutor>();
    inlineExecutor = std::make_shared<folly::InlineExecutor>();
  }

  unique_ptr<MockPersistenceLayer> persistence;
  std::shared_ptr<folly::ManualExecutor> manualExecutor;
  std::shared_ptr<folly::InlineExecutor> inlineExecutor;
};

TYPED_TEST(LRUPersistentCacheTest, NullPersistence) {
  // make sure things sync even without a persistence layer
  auto cache = createCache<TypeParam>(10, 1, nullptr);
  cache->init();
  cache->put("k0", "v0");
  makeFuture()
      .delayed(std::chrono::milliseconds(20))
      .thenValue([cache](auto&&) {
        auto val = cache->get("k0");
        EXPECT_TRUE(val);
        EXPECT_EQ(*val, "v0");
        EXPECT_FALSE(cache->hasPendingUpdates());
      });
}

MATCHER_P(DynSize, n, "") {
  return size_t(n) == arg.size();
}

TYPED_TEST(LRUPersistentCacheTest, SettingPersistenceFromCtor) {
  InSequence seq;
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  auto rawPersistence = this->persistence.get();
  EXPECT_CALL(*rawPersistence, load_()).Times(1).WillOnce(Return(data));
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(true));
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
  cache->put("k0", "v0");
}

TYPED_TEST(LRUPersistentCacheTest, SyncOnDestroy) {
  auto persistence = this->persistence.get();
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
  cache->put("k0", "v0");
  EXPECT_CALL(*persistence, persist_(_)).Times(1).WillOnce(Return(true));
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, SyncOnDestroyWithExecutor) {
  auto persistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
  cache->put("k0", "v0");
  EXPECT_CALL(*persistence, persist_(_))
      .Times(AtLeast(1))
      .WillRepeatedly(Return(true));
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, PersistNotCalled) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  EXPECT_CALL(*this->persistence, persist_(_)).Times(0).WillOnce(Return(false));
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
  EXPECT_EQ(cache->size(), 1);
}

TYPED_TEST(LRUPersistentCacheTest, PersistentSetBeforeSyncer) {
  EXPECT_CALL(*this->persistence, getLastPersistedVersion())
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke(
          this->persistence.get(),
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
}

TYPED_TEST(LRUPersistentCacheTest, ClearKeepPersist) {
  EXPECT_CALL(*this->persistence, clear()).Times(0);
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
  cache->clear();
}

TYPED_TEST(LRUPersistentCacheTest, ClearDontKeepPersist) {
  EXPECT_CALL(*this->persistence, clear()).Times(1);
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache->init();
  cache->clear(true);
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheDeallocBeforeAdd) {
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache.reset();
  // Nothing should happen here
  this->manualExecutor->drain();
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheRunTask) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto rawPersistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
  this->manualExecutor->run();
  cache->put("k0", "v0");
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(true));
  this->manualExecutor->run();

  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheRunTaskInline) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto rawPersistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->inlineExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(true));
  cache->put("k0", "v0");

  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  EXPECT_CALL(*rawPersistence, persist_(DynSize(3)))
      .Times(1)
      .WillOnce(Return(true));
  cache->put("k2", "v2");

  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheRetries) {
  EXPECT_CALL(*this->persistence, load_())
      .Times(1)
      .WillOnce(Return(dynamic::array()));
  auto rawPersistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      2);
  cache->init();
  this->manualExecutor->run();
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .WillRepeatedly(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));

  cache->put("k0", "v0");
  EXPECT_CALL(*rawPersistence, persist_(DynSize(1)))
      .Times(1)
      .WillOnce(Return(false));
  this->manualExecutor->run();

  cache->put("k1", "v1");
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(false));
  // reached retry limit, so we will set a version anyway
  EXPECT_CALL(*rawPersistence, setPersistedVersion(_))
      .Times(1)
      .WillOnce(Invoke(
          rawPersistence, &MockPersistenceLayer::setPersistedVersionConcrete));
  this->manualExecutor->run();

  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheSchduledAndDealloc) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
  this->manualExecutor->run();
  cache->put("k0", "v0");
  cache->put("k2", "v2");

  // Kill cache first then try to run scheduled tasks. Nothing will run and no
  // one should crash.
  cache.reset();
  this->manualExecutor->drain();
}

TYPED_TEST(LRUPersistentCacheTest, ExecutorCacheScheduleInterval) {
  EXPECT_CALL(*this->persistence, load_())
      .Times(1)
      .WillOnce(Return(dynamic::array()));
  auto rawPersistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds(60 * 60 * 1000),
      1);
  cache->init();
  this->manualExecutor->run();
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .WillRepeatedly(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));

  cache->put("k0", "v0");
  EXPECT_CALL(*rawPersistence, persist_(DynSize(1)))
      .Times(1)
      .WillOnce(Return(false));
  this->manualExecutor->run();

  // The following put won't trigger a run due to the interval
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2))).Times(0);
  EXPECT_CALL(*rawPersistence, setPersistedVersion(_)).Times(0);
  cache->put("k1", "v1");
  this->manualExecutor->run();

  // But we will sync again upon destroy
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(true));
  cache.reset();
  // Nothing more should happen after this
  this->manualExecutor->drain();
}

TYPED_TEST(LRUPersistentCacheTest, InitCache) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache = createCacheWithExecutor<TypeParam>(
      this->inlineExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1,
      false);
  cache->init();
  EXPECT_FALSE(cache->hasPendingUpdates());
}

TYPED_TEST(LRUPersistentCacheTest, BlockingAccessCanContinueWithExecutor) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1,
      false);
  cache->init();
  std::string value1, value2;
  std::thread willBeBlocked([&]() { value1 = cache->get("k1").value(); });
  // Without running the executor to finish setPersistenceHelper in init,
  // willbeBlocked will be blocked.
  this->manualExecutor->run();
  willBeBlocked.join();
  EXPECT_EQ("v1", value1);

  std::thread wontBeBlocked([&]() { value2 = cache->get("k1").value(); });
  wontBeBlocked.join();
  EXPECT_EQ("v1", value2);
}

TYPED_TEST(LRUPersistentCacheTest, BlockingAccessCanContinueWithThread) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  EXPECT_CALL(*this->persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache =
      createCache<TypeParam>(10, 10, std::move(this->persistence), false);
  cache->init();
  std::string value1;
  std::thread appThread([&]() { value1 = cache->get("k1").value(); });
  appThread.join();
  EXPECT_EQ("v1", value1);
}

TYPED_TEST(
    LRUPersistentCacheTest,
    PersistenceOnlyLoadedOnceFromCtorWithExecutor) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  auto persistence = this->persistence.get();
  EXPECT_CALL(*persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
}

TYPED_TEST(
    LRUPersistentCacheTest,
    PersistenceOnlyLoadedOnceFromCtorWithSyncThread) {
  folly::dynamic data = dynamic::array(dynamic::array("k1", "v1"));
  auto persistence = this->persistence.get();
  EXPECT_CALL(*persistence, load_()).Times(1).WillOnce(Return(data));
  auto cache = createCache<TypeParam>(10, 1, std::move(this->persistence));
  cache->init();
}

TYPED_TEST(LRUPersistentCacheTest, DestroyWithoutInitThreadMode) {
  auto cache = createCache<TypeParam>(10, 10, std::move(this->persistence));
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, DestroyWithoutInitExecutorMode) {
  auto cache = createCacheWithExecutor<TypeParam>(
      this->inlineExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache.reset();
}

TYPED_TEST(LRUPersistentCacheTest, EmptyPersistenceMatchesEmptyCache) {
  auto persistence = this->persistence.get();
  EXPECT_CALL(*persistence, load_()).Times(1).WillOnce(Return(folly::none));
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1);
  cache->init();
  EXPECT_FALSE(cache->hasPendingUpdates());
  EXPECT_EQ(
      kDefaultInitCacheDataVersion, persistence->getLastPersistedVersion());
  this->manualExecutor->drain();

  cache->put("k0", "v0");
  EXPECT_TRUE(cache->hasPendingUpdates());
}

TYPED_TEST(LRUPersistentCacheTest, ZeroSyncIntervalSyncsImmediately) {
  EXPECT_CALL(*this->persistence, load_())
      .Times(1)
      .WillOnce(Return(dynamic::array()));
  auto rawPersistence = this->persistence.get();
  auto cache = createCacheWithExecutor<TypeParam>(
      this->manualExecutor,
      std::move(this->persistence),
      std::chrono::milliseconds::zero(),
      1,
      true);
  cache->init();
  this->manualExecutor->run();
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .WillRepeatedly(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));

  cache->put("k0", "v0");
  EXPECT_CALL(*rawPersistence, persist_(DynSize(1)))
      .Times(1)
      .WillOnce(Return(true));
  this->manualExecutor->run();

  // The following put will trigger a sync because syncImmediatelyWithExecutor
  // is set to true
  EXPECT_CALL(*rawPersistence, getLastPersistedVersion())
      .WillRepeatedly(Invoke(
          rawPersistence,
          &MockPersistenceLayer::getLastPersistedVersionConcrete));
  EXPECT_CALL(*rawPersistence, persist_(DynSize(2)))
      .Times(1)
      .WillOnce(Return(true));
  cache->put("k1", "v1");
  this->manualExecutor->run();

  EXPECT_CALL(*rawPersistence, persist_(_)).Times(0);
  cache.reset();
  // Nothing more should happen after this
  this->manualExecutor->drain();
}
