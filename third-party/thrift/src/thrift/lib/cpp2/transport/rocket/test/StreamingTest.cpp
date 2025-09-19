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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <folly/Portability.h>
#include <folly/coro/BlockingWait.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/gen/client_cpp.h>
#include <thrift/lib/cpp2/transport/core/testutil/TAsyncSocketIntercepted.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestServiceMock.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

namespace apache::thrift {

namespace {
void waitNoLeak(StreamServiceAsyncClient* client) {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{2};
  do {
    std::this_thread::yield();
    if (client->sync_instanceCount() == 0) {
      // There is a race between decrementing the instance count vs
      // sending the error/complete message, so sleep a bit before returning
      /* sleep override */
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return;
    }
  } while (std::chrono::steady_clock::now() < deadline);
  FAIL() << "waitNoLeak failed";
}
} // namespace

// Testing transport layers for their support to Streaming
class StreamingTest : public TestSetup {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<StrictMock<TestStreamServiceMock>>();
    server_ = createServer(handler_, port_);
  }

  void TearDown() override {
    if (server_) {
      server_->cleanUp();
      server_.reset();
      handler_.reset();
    }
  }

  void connectToServer(
      folly::Function<void(std::unique_ptr<StreamServiceAsyncClient>)> callMe,
      folly::Function<void()> onDetachable = nullptr,
      folly::Function<void(TAsyncSocketIntercepted&)> socketSetup = nullptr) {
    auto channel =
        connectToServer(port_, std::move(onDetachable), std::move(socketSetup));
    callMe(std::make_unique<StreamServiceAsyncClient>(std::move(channel)));
  }

  void callSleep(
      StreamServiceAsyncClient* client,
      int32_t timeoutMs,
      int32_t sleepMs,
      bool withResponse) {
    auto cb = std::make_unique<MockCallback>(false, timeoutMs < sleepMs);
    RpcOptions opts;
    opts.setTimeout(std::chrono::milliseconds(timeoutMs));
    opts.setQueueTimeout(std::chrono::milliseconds(5000));
    if (withResponse) {
      client->sleepWithResponse(opts, std::move(cb), sleepMs);
    } else {
      client->sleepWithoutResponse(opts, std::move(cb), sleepMs);
    }
  }

 private:
  using TestSetup::connectToServer;

 protected:
  std::unique_ptr<ThriftServer> server_;
  std::shared_ptr<testing::StrictMock<TestStreamServiceMock>> handler_;

  uint16_t port_;
};

TEST_F(StreamingTest, ClientStreamBridge) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_EQ(42, client->sync_echo(42));

    {
      auto bufferedStream = client->sync_range(0, 10);

      size_t expected = 0;
      std::move(bufferedStream)
          .subscribeInline([&expected](folly::Try<int32_t>&& next) {
            if (next.hasValue()) {
              EXPECT_EQ(expected++, *next);
            }
          });
      EXPECT_EQ(10, expected);
    }

    {
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setChunkBufferSize(5);
      auto bufferedStream = client->sync_range(rpcOptions, 0, 10);

      size_t expected = 0;
      std::move(bufferedStream)
          .subscribeInline([&expected](folly::Try<int32_t>&& next) {
            if (next.hasValue()) {
              EXPECT_EQ(expected++, *next);
            }
          });
      EXPECT_EQ(10, expected);
    }

#if FOLLY_HAS_COROUTINES
    {
      size_t expected = 0;
      auto gen = client->sync_range(0, 10).toAsyncGenerator();
      folly::coro::blockingWait([&]() mutable -> folly::coro::Task<void> {
        while (auto next = co_await gen.next()) {
          EXPECT_EQ(expected++, *next);
        }
      }());
      EXPECT_EQ(10, expected);
    }

    {
      // exercises logic to request credits after 16kB of payload
      auto gen = client->sync_buffers(17).toAsyncGenerator();
      size_t expected = 0;
      folly::coro::blockingWait([&]() mutable -> folly::coro::Task<void> {
        while (auto next = co_await gen.next()) {
          EXPECT_EQ(1024, next->size());
          expected++;
        }
      }());
      EXPECT_EQ(17, expected);
    }

    // test cancellation
    {
      folly::CancellationSource cancelSource;
      auto gen = client->sync_range(0, 10).toAsyncGenerator();
      folly::coro::blockingWait(folly::coro::co_withCancellation(
          cancelSource.getToken(), [&]() mutable -> folly::coro::Task<void> {
            auto next = co_await gen.next();
            EXPECT_EQ(0, *next);
            cancelSource.requestCancellation();
            EXPECT_THROW(co_await gen.next(), folly::OperationCancelled);
          }()));
    }

    {
      auto bufferedStream = client->sync_slowRange(0, 10, 1000);
      folly::CancellationSource cancelSource;
      auto gen = std::move(bufferedStream).toAsyncGenerator();
      folly::coro::blockingWait(folly::coro::co_withCancellation(
          cancelSource.getToken(),
          folly::coro::co_invoke([&]() mutable -> folly::coro::Task<void> {
            (co_await folly::coro::co_current_executor)->add([&]() {
              cancelSource.requestCancellation();
            });
            EXPECT_THROW(co_await gen.next(), folly::OperationCancelled);
          })));
    }
#endif // FOLLY_HAS_COROUTINES

    {
      // exercises logic to request credits after 16kB of payload
      auto bufferedStream = client->sync_buffers(17);

      size_t expected = 0;
      std::move(bufferedStream)
          .subscribeInline([&expected](folly::Try<std::string>&& next) {
            if (next.hasValue()) {
              expected++;
              EXPECT_EQ(1024, next->size());
            }
          });
      EXPECT_EQ(17, expected);
    }

    {
      // exercises logic to request credits after 16kB of payload
      auto bufferedStream = client->sync_buffers(17);
      std::atomic_size_t expected = 0;
      std::move(bufferedStream)
          .subscribeExTry(
              &executor_,
              [&expected](auto&& next) {
                if (next.hasValue()) {
                  EXPECT_EQ(1024, next->size());
                  expected++;
                }
              })
          .join();
      EXPECT_EQ(17, expected);
    }

    {
      auto bufferedStream = client->sync_range(0, 10);

      std::atomic_size_t expected = 0;
      std::move(bufferedStream)
          .subscribeExTry(
              &executor_,
              [&expected](auto&& next) {
                if (next.hasValue()) {
                  EXPECT_EQ(expected++, *next);
                }
              })
          .join();
      EXPECT_EQ(10, expected);
    }

    {
      auto bufferedStream = client->sync_range(0, 10);

      std::atomic_size_t expected = 0;
      std::move(bufferedStream)
          .subscribeExTry(
              &executor_,
              [&expected](auto&& next) {
                if (next.hasValue()) {
                  EXPECT_EQ(expected++, *next);
                }
              })
          .futureJoin()
          .get();
      EXPECT_EQ(10, expected);
    }

    // test cancellation
    {
      auto bufferedStream = client->sync_range(0, 10);

      std::atomic_size_t expected = 0;
      folly::Function<void()> cancel;
      std::optional<folly::fibers::Baton> baton1;
      folly::fibers::Baton baton2;
      baton1.emplace();
      auto sub = std::move(bufferedStream)
                     .subscribeExTry(&executor_, [&](auto&& next) {
                       if (baton1) {
                         baton1->wait();
                         baton1.reset();
                         cancel();
                         baton2.post();
                       }
                       if (next.hasValue()) {
                         EXPECT_EQ(expected++, *next);
                       }
                     });
      cancel = [&sub] { sub.cancel(); };
      baton1->post();
      baton2.wait();
      std::move(sub).join();
      EXPECT_EQ(1, expected);
    }

    // detach
    {
      auto bufferedStream = client->sync_range(0, 10);

      std::atomic_size_t expected = 0;
      folly::fibers::Baton baton;
      std::move(bufferedStream)
          .subscribeExTry(
              &executor_,
              [&](auto&& next) {
                if (next.hasValue()) {
                  EXPECT_EQ(expected++, *next);
                } else {
                  baton.post();
                }
              })
          .detach();
      baton.wait();
      EXPECT_EQ(10, expected);
    }

    // 0 buffersize
    {
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setChunkBufferSize(0);
      auto bufferedStream = client->sync_range(rpcOptions, 0, 10);

      size_t expected = 0;
      std::move(bufferedStream).subscribeInline([&expected](auto&& next) {
        if (next.hasValue()) {
          EXPECT_EQ(expected++, *next);
        }
      });
      EXPECT_EQ(10, expected);

#if FOLLY_HAS_COROUTINES
      bufferedStream = client->sync_range(rpcOptions, 0, 10);

      expected = 0;
      auto gen = std::move(bufferedStream).toAsyncGenerator();
      folly::coro::blockingWait([&]() mutable -> folly::coro::Task<void> {
        while (auto next = co_await gen.next()) {
          EXPECT_EQ(expected++, *next);
        }
      }());
      EXPECT_EQ(10, expected);
#endif // FOLLY_HAS_COROUTINES

      bufferedStream = client->sync_range(rpcOptions, 0, 10);

      expected = 0;
      std::move(bufferedStream)
          .subscribeExTry(
              &executor_,
              [&expected](auto&& next) {
                if (next.hasValue()) {
                  EXPECT_EQ(expected++, *next);
                }
              })
          .join();
      EXPECT_EQ(10, expected);
    }

    // throw
    {
      bool thrown = false;
      auto stream = client->sync_streamThrows(1);
      std::move(stream).subscribeInline([&thrown](auto&& next) {
        thrown = true;
        EXPECT_TRUE(next.hasException());
        EXPECT_TRUE(next.template hasException<FirstEx>());
        EXPECT_TRUE(next.exception().with_exception([](FirstEx& ex) {
          EXPECT_EQ(1, ex.errCode().value());
          EXPECT_STREQ("FirstEx", ex.errMsg().value().c_str());
        }));
      });
      EXPECT_TRUE(thrown);
    }
  });
}

TEST_F(StreamingTest, ClientStreamBridgeClientTimeout) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    RpcOptions rpcOptions;
    rpcOptions.setTimeout(std::chrono::milliseconds{100});
    rpcOptions.setClientOnlyTimeouts(true);
    rpcOptions.setChunkBufferSize(5);
    EXPECT_THROW(
        client->sync_leakCheckWithSleep(rpcOptions, 0, 0, 200),
        TTransportException);
  });
}

TEST_F(StreamingTest, ClientStreamBridgeLifeTimeTesting) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    auto waitForInstanceCount = [&](int count) {
      auto start = std::chrono::steady_clock::now();
      while (std::chrono::steady_clock::now() - start <
             std::chrono::seconds{1}) {
        if (client->sync_instanceCount() == count) {
          break;
        }
      }
      EXPECT_EQ(count, client->sync_instanceCount());
    };

    {
      {
        auto f = client->semifuture_leakCheck(0, 1000);
        waitForInstanceCount(1);
      }

      waitForInstanceCount(0);
    }

    {
      auto f = client->semifuture_leakCheck(0, 1000);
      waitForInstanceCount(1);

      std::move(f).get();
      waitForInstanceCount(0);
    }
  });
}

TEST_F(StreamingTest, ClientStreamBridgeStress) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    for (size_t i = 0; i < 100; ++i) {
      std::atomic<size_t> expected = 0;
      folly::Baton<> baton;
      apache::thrift::RpcOptions opt;
      opt.setChunkBufferSize(i % 10 + 1);
      auto sub =
          client->sync_range(opt, 0, 1000)
              .subscribeExTry(&executor_, [&](folly::Try<int32_t>&& next) {
                if (next.hasValue()) {
                  if (expected == i) {
                    baton.post();
                  }
                  EXPECT_EQ(expected++, *next);
                }
              });
      baton.wait();
      sub.cancel();
      std::move(sub).join();
      EXPECT_LT(expected, 1000);
      EXPECT_GE(expected, i);
    }

    for (size_t i = 0; i < 100; ++i) {
      size_t expected = 0;
      apache::thrift::RpcOptions opt;
      opt.setChunkBufferSize(i % 10 + 1);
      client->sync_range(opt, 0, 20)
          .subscribeInline([&](folly::Try<int32_t>&& next) {
            if (next.hasValue()) {
              EXPECT_EQ(expected++, *next);
            }
          });
      EXPECT_EQ(expected, 20);
    }
  });
}

TEST_F(StreamingTest, SimpleStream) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    auto result = client->sync_range(0, 10);
    int j = 0;
    auto subscription =
        std::move(result).subscribeExTry(&executor_, [&j](auto&& next) mutable {
          if (next.hasValue()) {
            EXPECT_EQ(j++, *next);
          } else if (next.hasException()) {
            FAIL() << "Should not call onError: " << next.exception().what();
          }
        });
    std::move(subscription).join();
    EXPECT_EQ(10, j);
  });
}

TEST_F(StreamingTest, FutureSimpleStream) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    auto futureRange = client->semifuture_range(0, 10);
    auto stream = std::move(futureRange).get();
    auto result = std::move(stream);
    int j = 0;
    auto subscription =
        std::move(result).subscribeExTry(&executor_, [&j](auto&& next) mutable {
          if (next.hasValue()) {
            EXPECT_EQ(j++, *next);
          } else if (next.hasException()) {
            FAIL() << "Should not call onError: " << next.exception().what();
          }
        });
    std::move(subscription).join();
    EXPECT_EQ(10, j);
  });
}

TEST_F(StreamingTest, ChecksummingRequest) {
  // TODO (T61528332) why does this fail??
  return;
  auto corruptionParams = std::make_shared<TAsyncSocketIntercepted::Params>();
  connectToServer(
      [this,
       corruptionParams](std::unique_ptr<StreamServiceAsyncClient> client) {
        enum class CorruptionType : int {
          NONE = 0,
          REQUESTS = 1,
          RESPONSES = 2,
        };
        auto setCorruption = [&](CorruptionType corruptionType) {
          evbThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
            corruptionParams->corruptLastWriteByte_ =
                corruptionType == CorruptionType::REQUESTS;
            corruptionParams->corruptLastReadByteMinSize_ = 30;
            corruptionParams->corruptLastReadByte_ =
                corruptionType == CorruptionType::RESPONSES;
          });
        };

        static const int kSize = 32 << 10;
        std::string asString(kSize, 'a');
        std::unique_ptr<folly::IOBuf> payload =
            folly::IOBuf::copyBuffer(asString);

        for (CorruptionType testType :
             {CorruptionType::NONE,
              CorruptionType::REQUESTS,
              CorruptionType::RESPONSES}) {
          setCorruption(testType);
          bool didThrow = false;
          try {
            auto futureRet = client->semifuture_requestWithBlob(
                RpcOptions().setChecksum(
                    apache::thrift::RpcOptions::Checksum::SERVER_ONLY_CRC32),
                *payload);
            auto stream = std::move(futureRet).get();
            auto result = std::move(stream);
            auto subscription = std::move(result).subscribeExTry(
                &executor_, [](auto&& next) mutable {
                  if (next.hasValue()) {
                    FAIL() << "Should be empty ";
                  } else if (next.hasException()) {
                    FAIL() << "Should not call onError: "
                           << next.exception().what();
                  }
                });
            std::move(subscription).join();
          } catch (TApplicationException& ex) {
            EXPECT_EQ(TApplicationException::CHECKSUM_MISMATCH, ex.getType());
            didThrow = true;
          }
          EXPECT_EQ(testType != CorruptionType::NONE, didThrow);
        }

        setCorruption(CorruptionType::NONE);
      },
      nullptr,
      [=](TAsyncSocketIntercepted& sock) { sock.setParams(corruptionParams); });
}

TEST_F(StreamingTest, DefaultStreamImplementation) {
  connectToServer([&](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_THROW(
        client->sync_nonImplementedStream("test"),
        apache::thrift::TApplicationException);
  });
}

TEST_F(StreamingTest, ReturnsNullptr) {
  // User function should return a Stream, but it returns a nullptr.
  connectToServer([&](std::unique_ptr<StreamServiceAsyncClient> client) {
    bool success = false;
    client->sync_returnNullptr()
        .subscribeExTry(
            &executor_,
            [&success](auto&& next) mutable {
              if (next.hasValue()) {
                FAIL() << "No value was expected";
              } else if (next.hasException()) {
                FAIL() << "Should not call onError: "
                       << next.exception().what();
              } else {
                success = true;
              }
            })
        .join();
    EXPECT_TRUE(success);
  });
}

TEST_F(StreamingTest, ThrowsWithResponse) {
  connectToServer([&](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_THROW(client->sync_throwError(), Error);
  });
}

TEST_F(StreamingTest, RequestTimeout) {
  // TODO (T61528332) fails due to incompabitibility b/w serverstream / rsocket
  return;
  bool withResponse = false;
  auto test =
      [this, &withResponse](std::unique_ptr<StreamServiceAsyncClient> client) {
        // This test focuses on timeout for the initial response. We will have
        // another test for timeout of each onNext calls.
        callSleep(client.get(), 1, 100, withResponse);
        callSleep(client.get(), 100, 0, withResponse);
        callSleep(client.get(), 1, 100, withResponse);
        callSleep(client.get(), 100, 0, withResponse);
        callSleep(client.get(), 2000, 500, withResponse);
        /* sleep override */
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        callSleep(client.get(), 100, 1000, withResponse);
        callSleep(client.get(), 200, 0, withResponse);
        /* Sleep to give time for all callbacks to be completed */
        /* sleep override */
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      };

  connectToServer(test);
  EXPECT_EQ(3, observer_->taskTimeout_);
  EXPECT_EQ(0, observer_->queueTimeout_);

  withResponse = true;
  connectToServer(test);
  EXPECT_EQ(6, observer_->taskTimeout_);
  EXPECT_EQ(0, observer_->queueTimeout_);
}

TEST_F(StreamingTest, OnDetachable) {
  // TODO (T61528332) fails due to incompabitibility b/w serverstream / rsocket
  return;
  folly::Promise<folly::Unit> detachablePromise;
  auto detachableFuture = detachablePromise.getSemiFuture();
  connectToServer(
      [&](std::unique_ptr<StreamServiceAsyncClient> client) {
        const auto timeout = std::chrono::milliseconds{100};
        auto stream = client->sync_range(0, 10);
        std::this_thread::sleep_for(timeout);
        EXPECT_FALSE(detachableFuture.isReady());

        folly::Baton<> done;

        std::move(stream)
            .subscribeExTry(
                &executor_,
                [&done](auto&& t) {
                  if (t.hasException()) {
                    FAIL() << "Should not call onError: "
                           << t.exception().what();
                  } else if (!t.hasValue()) {
                    done.post();
                  }
                })
            .detach();

        EXPECT_TRUE(done.try_wait_for(timeout));
        EXPECT_TRUE(std::move(detachableFuture).wait(timeout));
      },
      [promise = std::move(detachablePromise)]() mutable {
        promise.setValue(folly::unit);
      });
}

TEST_F(StreamingTest, ChunkTimeout) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    RpcOptions options;
    options.setChunkTimeout(std::chrono::milliseconds{10});
    auto result = client->sync_streamServerSlow(options);
    bool failed{false};
    auto subscription =
        std::move(result.stream)
            .subscribeExTry(&executor_, [&failed](auto&& next) mutable {
              if (next.hasValue()) {
                FAIL() << "Should have failed.";
              } else if (next.hasException()) {
                next.exception().with_exception([&](TTransportException& tae) {
                  ASSERT_EQ(
                      TTransportException::TTransportExceptionType::TIMED_OUT,
                      tae.getType());
                  failed = true;
                });
              } else {
                FAIL() << "onError should be called";
              }
            });
    std::move(subscription).join();
    EXPECT_TRUE(failed);
    waitNoLeak(client.get());
  });
}

TEST_F(StreamingTest, UserCantBlockIOThread) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    RpcOptions options;
    options.setChunkTimeout(std::chrono::milliseconds{10});
    auto stream = client->sync_range(options, 0, 10);

    bool failed{true};
    int count = 0;
    auto subscription = std::move(stream).subscribeExTry(
        &executor_, [&count, &failed](auto&& next) mutable {
          if (next.hasValue()) {
            // sleep, so that client will be late!
            /* sleep override */
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            ++count;
          } else if (next.hasException()) {
            FAIL() << "no error was expected";
          } else {
            failed = false;
          }
        });
    std::move(subscription).join();
    EXPECT_FALSE(failed);
    // As there is no flow control, all of the messages will be sent from
    // server to client without waiting the user thread.
    EXPECT_EQ(10, count);
  });
}

TEST_F(StreamingTest, TwoRequestsOneTimesOut) {
  // TODO (T61528332) fails due to incompabitibility b/w serverstream / rsocket
  return;
  folly::Promise<folly::Unit> detachablePromise;
  auto detachableFuture = detachablePromise.getSemiFuture();

  connectToServer(
      [&, this](std::unique_ptr<StreamServiceAsyncClient> client) {
        const auto waitForMs = std::chrono::milliseconds{100};

        auto stream = client->sync_registerToMessages();
        int32_t last = 0;
        folly::Baton<> baton;
        auto subscription = std::move(stream).subscribeExTry(
            &executor_, [&last, &baton](auto&& next) {
              if (next.hasValue()) {
                last = *next;
                baton.post();
              }
            });

        client->sync_sendMessage(1, false, false);
        ASSERT_TRUE(baton.try_wait_for(std::chrono::milliseconds(100)));
        baton.reset();
        ASSERT_EQ(last, 1);

        // timeout a single request
        callSleep(client.get(), 1, 100, true);

        // Still there is one stream in the client side
        std::this_thread::sleep_for(waitForMs);
        EXPECT_FALSE(detachableFuture.isReady());

        client->sync_sendMessage(2, true, false);
        ASSERT_TRUE(baton.try_wait_for(waitForMs));
        baton.reset();
        ASSERT_EQ(last, 2);

        std::move(subscription).join();

        // All streams are cleaned up in the client side
        EXPECT_TRUE(std::move(detachableFuture).wait(waitForMs));
      },
      [promise = std::move(detachablePromise)]() mutable {
        promise.setValue(folly::unit);
      });
}

TEST_F(StreamingTest, StreamThrowsKnownException) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    bool thrown = false;
    auto stream = client->sync_streamThrows(1);
    auto subscription =
        std::move(stream).subscribeExTry(&executor_, [&thrown](auto&& next) {
          if (next.hasException()) {
            auto ew = next.exception();
            thrown = true;
            EXPECT_TRUE(ew.template is_compatible_with<FirstEx>());
            EXPECT_TRUE(ew.with_exception([](FirstEx& ex) {
              EXPECT_EQ(1, ex.errCode().value());
              EXPECT_STREQ("FirstEx", ex.errMsg().value().c_str());
            }));
          }
        });
    std::move(subscription).join();
    EXPECT_TRUE(thrown);
  });
}

TEST_F(StreamingTest, StreamThrowsNonspecifiedException) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    bool thrown = false;
    auto stream = client->sync_streamThrows(2);
    auto subscription =
        std::move(stream).subscribeExTry(&executor_, [&thrown](auto&& next) {
          if (next.hasException()) {
            auto ew = next.exception();
            thrown = true;
            EXPECT_TRUE(
                ew.template is_compatible_with<TApplicationException>());
            EXPECT_TRUE(ew.with_exception([](TApplicationException& ex) {
              EXPECT_STREQ(
                  "testutil::testservice::SecondEx: ::testutil::testservice::SecondEx",
                  ex.what());
            }));
          }
        });
    std::move(subscription).join();
    EXPECT_TRUE(thrown);
  });
}

TEST_F(StreamingTest, StreamThrowsRuntimeError) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    bool thrown = false;
    auto stream = client->sync_streamThrows(3);
    auto subscription =
        std::move(stream).subscribeExTry(&executor_, [&thrown](auto&& next) {
          if (next.hasException()) {
            auto ew = next.exception();
            thrown = true;
            EXPECT_TRUE(
                ew.template is_compatible_with<TApplicationException>());
            EXPECT_TRUE(ew.with_exception([](TApplicationException& ex) {
              EXPECT_STREQ("std::runtime_error: random error", ex.what());
            }));
          }
        });
    std::move(subscription).join();
    EXPECT_TRUE(thrown);
  });
}

TEST_F(StreamingTest, StreamFunctionThrowsImmediately) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_THROW(client->sync_streamThrows(0), SecondEx);
  });
}

TEST_F(StreamingTest, ResponseAndStreamThrowsKnownException) {
  connectToServer([this](std::unique_ptr<StreamServiceAsyncClient> client) {
    bool thrown = false;
    auto responseAndStream = client->sync_responseAndStreamThrows(1);
    auto stream = std::move(responseAndStream.stream);
    auto subscription =
        std::move(stream).subscribeExTry(&executor_, [&thrown](auto&& next) {
          if (next.hasException()) {
            auto ew = next.exception();
            thrown = true;
            EXPECT_TRUE(ew.template is_compatible_with<FirstEx>());
            EXPECT_TRUE(ew.with_exception([](FirstEx& ex) {
              EXPECT_EQ(1, ex.errCode().value());
              EXPECT_STREQ("FirstEx", ex.errMsg().value().c_str());
            }));
          }
        });
    std::move(subscription).join();
    EXPECT_TRUE(thrown);
  });
}

TEST_F(StreamingTest, ResponseAndStreamFunctionThrowsImmediately) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_THROW(client->sync_responseAndStreamThrows(0), SecondEx);
  });
}

TEST_F(StreamingTest, DetachAndAttachEventBase) {
  // TODO (T61528332) why does this fail??
  return;
  folly::EventBase mainEventBase;
  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&mainEventBase, "::1", port_));
  std::shared_ptr<ClientChannel> channel =
      RocketClientChannel::newChannel(std::move(socket));

  folly::Promise<folly::Unit> detachablePromise;
  auto detachableFuture = detachablePromise.getSemiFuture();
  channel->setOnDetachable([promise = std::move(detachablePromise),
                            channelPtr = channel.get()]() mutable {
    // Only set the promise on the first onDetachable() call
    if (auto* c = std::exchange(channelPtr, nullptr)) {
      ASSERT_TRUE(c->isDetachable());
      c->detachEventBase();
      promise.setValue(folly::unit);
    }
  });

  {
    // Establish a stream via mainEventBase and consume it until completion
    StreamServiceAsyncClient client{channel};
    auto stream = client.sync_range(0, 10);
    EXPECT_FALSE(detachableFuture.isReady());

    std::move(stream)
        .subscribeExTry(
            &executor_,
            [](auto&& next) {
              if (next.hasException()) {
                FAIL() << "Unexpected call to onError: " << next.exception();
              }
            })
        .futureJoin()
        .via(&mainEventBase)
        .waitVia(&mainEventBase);
  }

  folly::ScopedEventBaseThread anotherEventBase;
  auto* const evb = anotherEventBase.getEventBase();
  auto keepAlive = channel;
  std::move(detachableFuture)
      .via(evb)
      .thenValue([evb, channel = std::move(channel)](auto&&) mutable {
        // Attach channel to a different EventBase and establish a stream
        EXPECT_TRUE(channel->isDetachable());
        channel->attachEventBase(evb);

        StreamServiceAsyncClient client{std::move(channel)};
        return client.semifuture_range(0, 10);
      })
      .via(evb)
      .thenValue([ex = &executor_](auto&& stream) {
        return std::move(stream)
            .subscribeExTry(
                ex,
                [](auto&& next) {
                  if (next.hasException()) {
                    FAIL() << "Unexpected call to onError: "
                           << next.exception();
                  }
                })
            .futureJoin();
      })
      .via(evb)
      .ensure([keepAlive = std::move(keepAlive)] {})
      .via(&mainEventBase)
      .waitVia(&mainEventBase);
}

TEST_F(StreamingTest, ServerCompletesFirstResponseAfterClientDisconnects) {
  folly::EventBase evb;

  auto makeChannel = [&] {
    return RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(
        new folly::AsyncSocket(&evb, "::1", port_)));
  };

  {
    // Force the client to disconnect before the server completes the first
    // response.
    RpcOptions rpcOptions;
    rpcOptions.setTimeout(std::chrono::milliseconds{100});
    rpcOptions.setClientOnlyTimeouts(true);
    EXPECT_THROW(
        StreamServiceAsyncClient{makeChannel()}.sync_leakCheckWithSleep(
            rpcOptions, 0, 0, 200),
        TTransportException);
  }

  // Wait for the server to finish processing the first response, then check
  // that no streams are leaked.
  StreamServiceAsyncClient client{makeChannel()};
  waitNoLeak(&client);
}

TEST_F(StreamingTest, ServerCompletesFirstResponseAfterClientTimeout) {
  // recreate server with one worker thread for server to make sure leak check
  // only happen after sync_leakCheckWithSleep's handler code returns.
  numWorkerThreads_ = 1;
  server_ = createServer(handler_, port_);
  folly::EventBase evb;

  auto makeChannel = [&] {
    return RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(
        new folly::AsyncSocket(&evb, "::1", port_)));
  };

  StreamServiceAsyncClient client{makeChannel()};
  RpcOptions rpcOptions;
  rpcOptions.setTimeout(std::chrono::milliseconds{100});
  rpcOptions.setClientOnlyTimeouts(true);
  EXPECT_THROW(
      client.sync_leakCheckWithSleep(rpcOptions, 0, 0, 200),
      TTransportException);
  // Wait for server to finish processing, so next call won't trigger queue
  // timeout.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Wait for the server to finish processing the first response, then check
  // that no streams are leaked.
  waitNoLeak(&client);
}

TEST_F(StreamingTest, CloseClientWithMultipleActiveStreams) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
#if FOLLY_HAS_COROUTINES
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setChunkBufferSize(5);
    auto stream1 = client->sync_range(rpcOptions, 0, 10).toAsyncGenerator();
    auto stream2 = client->sync_range(rpcOptions, 0, 10).toAsyncGenerator();
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      co_await stream1.next();
      co_await stream2.next();
    }());
    client = {};
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      EXPECT_ANY_THROW(while (co_await stream1.next()){});
      EXPECT_ANY_THROW(while (co_await stream2.next()){});
    }());
#endif
  });
}

TEST_F(StreamingTest, SetMaxRequests) {
  THRIFT_OMIT_TEST_WITH_RESOURCE_POOLS(/* setMaxRequests concurency controller doesn't extend to stream end yet */);
  server_->setMaxRequests(2);
  connectToServer([&](std::unique_ptr<StreamServiceAsyncClient> client) {
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setChunkBufferSize(0);
    auto stream1 = client->sync_range(rpcOptions, 0, 10);
    auto stream2 = client->sync_range(rpcOptions, 0, 10);

    EXPECT_ANY_THROW(client->sync_range(rpcOptions, 0, 10));

    std::move(stream2).subscribeInline([](auto&&) {});

    auto stream3 = client->sync_range(rpcOptions, 0, 10);

    EXPECT_ANY_THROW(client->sync_range(rpcOptions, 0, 10));
  });
}

TEST_F(StreamingTest, SetMaxRequestsStreamCancel) {
  THRIFT_OMIT_TEST_WITH_RESOURCE_POOLS(/* setMaxRequests concurency controller doesn't extend to stream end yet */);
  server_->setMaxRequests(2);
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setChunkBufferSize(0);
    auto stream1 = client->sync_range(rpcOptions, 0, 10);
    auto stream2 = client->sync_range(rpcOptions, 0, 10);

    EXPECT_ANY_THROW(client->sync_range(rpcOptions, 0, 10));

    { auto _ = std::move(stream2); }

    auto stream3 = client->sync_range(rpcOptions, 0, 10);

    EXPECT_ANY_THROW(client->sync_range(rpcOptions, 0, 10));
  });
}

TEST_F(StreamingTest, LeakCallback) {
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    try {
      client->sync_leakCallback();
      ADD_FAILURE() << "Didn't throw";
    } catch (const TApplicationException& ex) {
      EXPECT_TRUE(folly::StringPiece(ex.what()).contains(
          "HandlerCallback not completed"));
    }
  });
}

TEST_F(StreamingTest, RequestsOrder) {
  connectToServer([&](std::unique_ptr<StreamServiceAsyncClient> client) {
    folly::Baton<> b;
    // Block IO thread to make sure all requests come in a single batch.
    // Make sure we make PooledRequestChannel cache the protocol before we block
    // the EventBase.
    client->getChannel()->getProtocolId();
    ioThread_->add([&] { b.wait(); });

    auto r1 = client->semifuture_orderRequestResponse();
    auto r2 = client->semifuture_orderRequestStream();
    auto r3 = client->semifuture_orderRequestResponse();
    auto r4 = client->semifuture_orderRequestStream();

    b.post();

    EXPECT_EQ(1, std::move(r1).get());
    EXPECT_EQ(2, std::move(r2).get().response);
    EXPECT_EQ(3, std::move(r3).get());
    EXPECT_EQ(4, std::move(r4).get().response);
  });
}

TEST_F(StreamingTest, LeakPublisherCheck) {
  server_->setTaskExpireTime(std::chrono::milliseconds(10));
  server_->setUseClientTimeout(false);
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    EXPECT_THROW(
        client->sync_leakPublisherCheck(),
        apache::thrift::TApplicationException);
    waitNoLeak(client.get());
  });
}

TEST_F(StreamingTest, ConnectionEgressBufferBackpressure) {
  server_->setUseClientTimeout(false);
  // Disable write batching
  server_->setWriteBatchingInterval(std::chrono::milliseconds::zero());
  // Trigger stream pause/resume by generating stream payloads that are too
  // large to send at once, causing AsyncSocket to buffer the rest and invoking
  // the buffer callback on the connection. A low backpressure threshold will
  // then pause streams.
  server_->setEgressBufferBackpressureThreshold(512);
  server_->setEgressBufferRecoveryFactor(0.5);
  connectToServer([](std::unique_ptr<StreamServiceAsyncClient> client) {
    constexpr int kCount = 5;
    constexpr int kSize = 10 * 1024 * 1024; // 10MB
    apache::thrift::RpcOptions rpcOptions;
    auto gen = client->sync_customBuffers(kCount, kSize).toAsyncGenerator();
    size_t received = 0;
    folly::coro::blockingWait([&]() mutable -> folly::coro::Task<void> {
      while (auto next = co_await gen.next()) {
        received++;
      }
    }());
    EXPECT_EQ(kCount, received);
  });
}

} // namespace apache::thrift
