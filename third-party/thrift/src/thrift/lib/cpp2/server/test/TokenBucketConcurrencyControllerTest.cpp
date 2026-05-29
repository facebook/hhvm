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
#include <mutex>
#include <thread>

#include <gtest/gtest.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/synchronization/Baton.h>
#include <folly/synchronization/Latch.h>

#include <thrift/lib/cpp2/server/TokenBucketConcurrencyController.h>

using namespace std;
using namespace std::literals;
using namespace apache::thrift;

// ---------------------------------------------------------------------------
// Test helpers (same patterns as ParallelConcurrencyControllerTest.cpp)
// ---------------------------------------------------------------------------

using Func = std::function<void(
    ServerRequest&&, const AsyncProcessorFactory::MethodMetadata&)>;

class MockAsyncProcessor : public AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {}

  void executeRequest(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata) override {
    if (executeRequestFunc_) {
      executeRequestFunc_(std::move(request), methodMetadata);
    } else {
      LOG(FATAL) << "Unimplemented executeRequest called";
    }
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {
    LOG(FATAL) << "Not supported";
  }

  void setFunc(Func func) { executeRequestFunc_ = std::move(func); }

 private:
  Func executeRequestFunc_;
};

class FIFORequestPile : public RequestPileInterface {
 public:
  std::optional<ServerRequestRejection> enqueue(
      ServerRequest&& request) override {
    queue_.enqueue(std::move(request));
    return std::nullopt;
  }

  std::optional<ServerRequest> dequeue() override {
    if (auto res = queue_.try_dequeue()) {
      return std::move(*res);
    }
    return std::nullopt;
  }

  void onRequestFinished(ServerRequestData&) override {}
  uint64_t requestCount() const override { return queue_.size(); }
  std::string describe() const override { return "{FIFORequestPile}"; }

 private:
  folly::UMPMCQueue<ServerRequest, false, 5> queue_;
};

class MockResponseChannelRequest : public ResponseChannelRequest {
 public:
  explicit MockResponseChannelRequest(
      bool shouldStartProcessing = true, bool oneway = false)
      : shouldStartProcessing_(shouldStartProcessing), oneway_(oneway) {}

  bool isActive() const override { return true; }
  bool isOneway() const override { return oneway_; }
  bool includeEnvelope() const override { return false; }
  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}
  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}
  bool tryStartProcessing() override { return shouldStartProcessing_; }

 private:
  bool shouldStartProcessing_;
  bool oneway_;
};

namespace {

// Thread-safe context storage for concurrent tests.
class Cpp2RequestContextStorage {
 public:
  Cpp2RequestContext* makeContext() {
    auto context = std::make_unique<Cpp2RequestContext>(nullptr);
    auto* rawPtr = context.get();
    std::lock_guard lock(mu_);
    contexts_.push_back(std::move(context));
    return rawPtr;
  }

 private:
  std::mutex mu_;
  std::vector<std::unique_ptr<Cpp2RequestContext>> contexts_;
};

ServerRequest getRequest(
    AsyncProcessor* ap,
    Cpp2RequestContext* context,
    bool shouldStartProcessing = true,
    bool oneway = false) {
  return ServerRequest(
      ResponseChannelRequest::UniquePtr(
          new MockResponseChannelRequest(shouldStartProcessing, oneway)),
      SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
      context,
      static_cast<protocol::PROTOCOL_TYPES>(0),
      nullptr,
      ap,
      nullptr);
}

std::unique_ptr<AsyncProcessor> makeAP(Func func) {
  auto mockAP = std::make_unique<MockAsyncProcessor>();
  mockAP->setFunc(std::move(func));
  return mockAP;
}

void enqueueRequest(
    FIFORequestPile& pile,
    TokenBucketConcurrencyController& cc,
    ServerRequest&& request) {
  pile.enqueue(std::move(request));
  cc.onEnqueued();
}

} // namespace

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class TokenBucketConcurrencyControllerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    executor_ = std::make_shared<folly::CPUThreadPoolExecutor>(4);
    pile_ = std::make_unique<FIFORequestPile>();
    cc_ =
        std::make_unique<TokenBucketConcurrencyController>(*pile_, *executor_);
  }

  void TearDown() override {
    cc_->stop();
    executor_->join();
  }

  std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;
  std::unique_ptr<FIFORequestPile> pile_;
  std::unique_ptr<TokenBucketConcurrencyController> cc_;
  Cpp2RequestContextStorage ctxStorage_;
};

// ===========================================================================
// 1. Basic Functionality
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, Construction) {
  EXPECT_EQ(cc_->getQpsLimit(), std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(cc_->requestCount(), 0);
  EXPECT_EQ(cc_->getExecutionLimitRequests(), 0);
}

TEST_F(TokenBucketConcurrencyControllerTest, Describe) {
  cc_->setQpsLimit(42);
  auto desc = cc_->describe();
  EXPECT_NE(desc.find("TokenBucketConcurrencyController"), std::string::npos);
  EXPECT_NE(desc.find("42"), std::string::npos);
}

TEST_F(TokenBucketConcurrencyControllerTest, GetDbgInfo) {
  cc_->setQpsLimit(999);
  auto info = cc_->getDbgInfo();
  EXPECT_NE(
      info.name()->find("TokenBucketConcurrencyController"), std::string::npos);
  EXPECT_EQ(*info.qpsLimit(), 999);
}

TEST_F(TokenBucketConcurrencyControllerTest, SetExecutionLimitIsNoOp) {
  cc_->setExecutionLimitRequests(42);
  EXPECT_EQ(cc_->getExecutionLimitRequests(), 0);
}

TEST_F(TokenBucketConcurrencyControllerTest, SetAndGetQpsLimit) {
  cc_->setQpsLimit(500);
  EXPECT_EQ(cc_->getQpsLimit(), 500);
  cc_->setQpsLimit(1000);
  EXPECT_EQ(cc_->getQpsLimit(), 1000);
}

TEST_F(TokenBucketConcurrencyControllerTest, RequestCountAlwaysZero) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));
  // requestCount() should still be 0 (not implemented)
  EXPECT_EQ(cc_->requestCount(), 0);

  ASSERT_TRUE(done.try_wait_for(5s));
  EXPECT_EQ(cc_->requestCount(), 0);
}

// ===========================================================================
// 2. Fast Path
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, FastPathSingleRequest) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
}

TEST_F(TokenBucketConcurrencyControllerTest, FastPathMultipleRequests) {
  constexpr int kNumRequests = 10;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

TEST_F(TokenBucketConcurrencyControllerTest, FastPathWithHighQpsLimit) {
  cc_->setQpsLimit(100000);

  std::atomic<int> executed{0};
  folly::Baton<> allDone;
  constexpr int kNumRequests = 50;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

// ===========================================================================
// 3. Slow Path / Burst
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, SlowModeProcessesAllRequests) {
  cc_->setQpsLimit(10000);

  constexpr int kNumRequests = 100;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(10s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

TEST_F(TokenBucketConcurrencyControllerTest, BurstUnderRateLimit) {
  cc_->setQpsLimit(50000);

  constexpr int kBurstSize = 200;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kBurstSize) {
      allDone.post();
    }
  });

  for (int i = 0; i < kBurstSize; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(10s));
  EXPECT_EQ(executed.load(), kBurstSize);
}

// ===========================================================================
// 4. QPS Limit = 0 (our fix: unlimited)
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, QpsLimitZeroAllowsRequests) {
  cc_->setQpsLimit(0);

  std::atomic<int> executed{0};
  folly::Baton<> allDone;
  constexpr int kNumRequests = 20;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

// ===========================================================================
// 5. Dynamic QPS Limit Change
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, DynamicQpsLimitChange) {
  cc_->setQpsLimit(100000);

  std::atomic<int> executed{0};
  folly::Baton<> allDone;
  constexpr int kNumRequests = 50;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < 25; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  cc_->setQpsLimit(50000);

  for (int i = 0; i < 25; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

TEST_F(TokenBucketConcurrencyControllerTest, DynamicQpsLimitChangeToZero) {
  cc_->setQpsLimit(100000);

  std::atomic<int> executed{0};
  folly::Baton<> allDone;
  constexpr int kNumRequests = 20;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < 10; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  cc_->setQpsLimit(0); // unlimited

  for (int i = 0; i < 10; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(executed.load(), kNumRequests);
}

// ===========================================================================
// 6. Rate limiting enforcement
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, RateLimitingEnforced) {
  cc_->setQpsLimit(5); // 5 QPS

  constexpr int kNumRequests = 10;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(30s));
  auto elapsed = std::chrono::steady_clock::now() - start;
  auto elapsedMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

  // At 5 QPS with burstSize=5, first 5 are immediate, next 5 need ~1s.
  EXPECT_GE(elapsedMs, 500) << "Rate limiting should slow down processing";
  EXPECT_EQ(executed.load(), kNumRequests);
}

// ===========================================================================
// 7. Callbacks
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, OnExecuteFunctionCalled) {
  std::atomic<int> callbackCount{0};
  cc_->setOnExecuteFunction([&](const ServerRequest&) { callbackCount++; });

  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
  EXPECT_EQ(callbackCount.load(), 1);
}

TEST_F(TokenBucketConcurrencyControllerTest, NullCallbacksDoNotCrash) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
}

TEST_F(
    TokenBucketConcurrencyControllerTest, OnExecuteFunctionCalledForMultiple) {
  std::atomic<int> callbackCount{0};
  cc_->setOnExecuteFunction([&](const ServerRequest&) { callbackCount++; });

  constexpr int kNumRequests = 5;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kNumRequests) {
      allDone.post();
    }
  });

  for (int i = 0; i < kNumRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  ASSERT_TRUE(allDone.try_wait_for(5s));
  EXPECT_EQ(callbackCount.load(), kNumRequests);
}

// ===========================================================================
// 8. Observer notification
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, ObserverNotifiedOnExecution) {
  class TestObserver : public ConcurrencyControllerInterface::Observer {
   public:
    std::atomic<int> count{0};
    void onFinishExecution(ServerRequest&) override { count++; }
  };

  auto observer = std::make_shared<TestObserver>();
  cc_->setObserver(observer);

  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
  std::this_thread::sleep_for(50ms);
  EXPECT_EQ(observer->count.load(), 1);
}

TEST_F(TokenBucketConcurrencyControllerTest, NullObserverDoesNotCrash) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
}

// ===========================================================================
// 9. Shutdown (stop())
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, StopBeforeAnyRequests) {
  cc_->stop();
}

TEST_F(TokenBucketConcurrencyControllerTest, StopIdempotent) {
  cc_->stop();
  cc_->stop();
}

TEST_F(TokenBucketConcurrencyControllerTest, StopAfterProcessingRequests) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  auto request = getRequest(ap.get(), ctxStorage_.makeContext());
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
  cc_->stop();
}

TEST_F(TokenBucketConcurrencyControllerTest, StopDuringActiveProcessing) {
  cc_->setQpsLimit(2);

  std::atomic<int> executed{0};
  auto ap = makeAP([&](ServerRequest&&, const auto&) { executed++; });

  for (int i = 0; i < 50; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  std::this_thread::sleep_for(200ms);
  cc_->stop();
  EXPECT_GT(executed.load(), 0);
}

// ===========================================================================
// 10. Oneway requests with tryStartProcessing=false
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, OnewayRequestNeverExpires) {
  folly::Baton<> done;
  auto ap = makeAP([&](ServerRequest&&, const auto&) { done.post(); });

  // isOneway=true, tryStartProcessing=false
  // expired() = !isOneway() && !getShouldStartProcessing() = !true && ... =
  // false So oneway requests never expire and should execute.
  auto request = getRequest(
      ap.get(),
      ctxStorage_.makeContext(),
      /* shouldStartProcessing */ false,
      /* oneway */ true);
  enqueueRequest(*pile_, *cc_, std::move(request));

  ASSERT_TRUE(done.try_wait_for(5s));
}

// ===========================================================================
// 11. Concurrency / Thread Safety
// ===========================================================================

TEST_F(TokenBucketConcurrencyControllerTest, ConcurrentOnEnqueued) {
  constexpr int kThreads = 8;
  constexpr int kRequestsPerThread = 50;
  constexpr int kTotalRequests = kThreads * kRequestsPerThread;

  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kTotalRequests) {
      allDone.post();
    }
  });

  std::vector<std::thread> threads;
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&]() {
      for (int i = 0; i < kRequestsPerThread; ++i) {
        auto request = getRequest(ap.get(), ctxStorage_.makeContext());
        enqueueRequest(*pile_, *cc_, std::move(request));
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  ASSERT_TRUE(allDone.try_wait_for(10s));
  EXPECT_EQ(executed.load(), kTotalRequests);
}

TEST_F(TokenBucketConcurrencyControllerTest, ConcurrentSetQpsLimit) {
  constexpr int kTotalRequests = 100;
  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kTotalRequests) {
      allDone.post();
    }
  });

  std::thread limiter([&]() {
    for (int i = 0; i < 100; ++i) {
      cc_->setQpsLimit(1000 + i);
      std::this_thread::yield();
    }
  });

  for (int i = 0; i < kTotalRequests; ++i) {
    auto request = getRequest(ap.get(), ctxStorage_.makeContext());
    enqueueRequest(*pile_, *cc_, std::move(request));
  }

  limiter.join();
  ASSERT_TRUE(allDone.try_wait_for(10s));
  EXPECT_EQ(executed.load(), kTotalRequests);
}

TEST_F(TokenBucketConcurrencyControllerTest, ConcurrentEnqueueWithLowQpsLimit) {
  // Force slow mode by exhausting tokens, then enqueue concurrently
  cc_->setQpsLimit(100);

  constexpr int kThreads = 8;
  constexpr int kRequestsPerThread = 20;
  constexpr int kTotalRequests = kThreads * kRequestsPerThread;

  std::atomic<int> executed{0};
  folly::Baton<> allDone;

  auto ap = makeAP([&](ServerRequest&&, const auto&) {
    if (++executed == kTotalRequests) {
      allDone.post();
    }
  });

  std::vector<std::thread> threads;
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&]() {
      for (int i = 0; i < kRequestsPerThread; ++i) {
        auto request = getRequest(ap.get(), ctxStorage_.makeContext());
        enqueueRequest(*pile_, *cc_, std::move(request));
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  ASSERT_TRUE(allDone.try_wait_for(30s));
  EXPECT_EQ(executed.load(), kTotalRequests);
}
