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

#include <folly/portability/GTest.h>

#include <folly/experimental/coro/Sleep.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/tests/util/Util.h>

namespace apache {
namespace thrift {

using namespace testutil::testservice;

struct SinkServiceTest
    : public AsyncTestSetup<TestSinkService, Client<TestSinkService>> {};

struct SinkServiceTestAllocFn
    : public AsyncTestSetup<TestSinkService, Client<TestSinkService>> {
  void SetUp() override {
    AsyncTestSetup<TestSinkService, Client<TestSinkService>>::SetUp();

    // add the custom alloc function
    folly::Function<BaseThriftServer::AllocIOBufFn> fn = [&](size_t size) {
      void* p = folly::aligned_malloc(size, 4096);
      CHECK(!!p);

      handler_->addBuf(p);

      auto iobuf = folly::IOBuf::takeOwnership(
          p, size, size, [](void* p, void*) { folly::aligned_free(p); });

      return iobuf;
    };

    server_->setAllocIOBufFn(std::move(fn));
  }
};

folly::coro::Task<bool> waitNoLeak(Client<TestSinkService>& client) {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{2};
  do {
    bool unsubscribed = co_await client.co_isSinkUnSubscribed();
    if (unsubscribed) {
      co_return true;
    }
  } while (std::chrono::steady_clock::now() < deadline);
  co_return false;
}

TEST_F(SinkServiceTest, SimpleSink) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_range(0, 100);
        bool finalResponse =
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              // @lint-ignore CLANGTIDY bugprone-use-after-move
              for (int i = 0; i <= 100; i++) {
                co_yield std::move(i);
              }
            }());
        EXPECT_TRUE(finalResponse);
      });
}

TEST_F(SinkServiceTest, SinkThrow) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_rangeThrow(0, 100);
        EXPECT_THROW(
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              co_yield 0;
              co_yield 1;
              throw std::runtime_error("test");
            }()),
            std::exception);
        co_await client.co_purge();
      });
}

TEST_F(SinkServiceTest, SinkThrowStruct) {
  connectToServer([](Client<TestSinkService>& client) -> folly::coro::Task<void> {
    auto sink = co_await client.co_sinkThrow();
    bool exceptionThrown = false;
    try {
      co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
        co_yield 0;
        co_yield 1;
        SinkException e;
        e.reason_ref() = "test";
        throw e;
      }());
    } catch (const SinkThrew& ex) {
      exceptionThrown = true;
      EXPECT_EQ(TApplicationException::UNKNOWN, ex.getType());
      EXPECT_EQ(
          "testutil::testservice::SinkException: ::testutil::testservice::SinkException",
          ex.getMessage());
    }
    EXPECT_TRUE(exceptionThrown);
    co_await client.co_purge();
  });
}

TEST_F(SinkServiceTest, SinkFinalThrow) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_rangeFinalResponseThrow(0, 100);
        bool throwed = false;
        try {
          co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
            // @lint-ignore CLANGTIDY bugprone-use-after-move
            for (int i = 0; i <= 100; i++) {
              co_yield std::move(i);
            }
          }());
        } catch (const std::exception& ex) {
          throwed = true;
          EXPECT_EQ("std::runtime_error: test", std::string(ex.what()));
        }
        EXPECT_TRUE(throwed);
      });
}

TEST_F(SinkServiceTest, SinkFinalThrowStruct) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_sinkFinalThrow();
        bool throwed = false;
        try {
          co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
            // @lint-ignore CLANGTIDY bugprone-use-after-move
            for (int i = 0; i <= 100; i++) {
              co_yield std::move(i);
            }
          }());
        } catch (const FinalException& ex) {
          throwed = true;
          EXPECT_EQ("test", *ex.reason_ref());
        }
        EXPECT_TRUE(throwed);
      });
}

TEST_F(SinkServiceTest, SinkEarlyFinalResponse) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_rangeEarlyResponse(0, 100, 20);

        int finalResponse =
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              // @lint-ignore CLANGTIDY bugprone-use-after-move
              for (int i = 0; i <= 100; i++) {
                co_yield std::move(i);
              }
            }());
        EXPECT_EQ(20, finalResponse);
      });
}

TEST_F(SinkServiceTest, SinkUnimplemented) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        EXPECT_THROW(co_await client.co_unimplemented(), std::exception);
      });
}

TEST_F(SinkServiceTest, SinkNotCalled) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        // even though don't really call sink.sink(...),
        // after sink get out of scope, the sink should be cancelled properly
        co_await client.co_unSubscribedSink();
        EXPECT_TRUE(co_await waitNoLeak(client));
      });
}

TEST_F(SinkServiceTest, SinkInitialThrows) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        try {
          co_await client.co_initialThrow();
        } catch (const MyException& ex) {
          EXPECT_EQ("reason", *ex.reason_ref());
        }
        // connection should still be alive after initial throw
        co_await client.co_purge();
      });
}

TEST_F(SinkServiceTest, SinkInitialThrowsOnFinalResponseCalled) {
  class Callback : public SinkClientCallback {
   public:
    Callback(
        bool onFirstResponseBool,
        folly::coro::Baton& responseReceived,
        folly::coro::Baton& onFinalResponseCalled)
        : onFirstResponseBool_(onFirstResponseBool),
          responseReceived_(responseReceived),
          onFinalResponseCalled_(onFinalResponseCalled) {}
    bool onFirstResponse(
        FirstResponsePayload&&,
        folly::EventBase*,
        SinkServerCallback* serverCallback) override {
      SCOPE_EXIT { responseReceived_.post(); };
      if (!onFirstResponseBool_) {
        serverCallback->onSinkError(std::runtime_error("stop sink"));
        return false;
      }
      return true;
    }
    void onFinalResponse(StreamPayload&&) override {
      if (onFirstResponseBool_) {
        onFinalResponseCalled_.post();
      } else {
        FAIL() << "onFinalResponse called when onFirstResponse returned false";
      }
    }
    bool onFirstResponseBool_;
    folly::coro::Baton& responseReceived_;
    folly::coro::Baton& onFinalResponseCalled_;

    // unused
    void onFirstResponseError(folly::exception_wrapper) override {}
    bool onSinkRequestN(uint64_t) override { return true; }
    void onFinalResponseError(folly::exception_wrapper) override {}
    void resetServerCallback(SinkServerCallback&) override {}
  };
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        ThriftPresult<false> pargs;
        auto req = CompactSerializer::serialize<std::string>(pargs);
        for (auto onFirstResponseBool : {true, false}) {
          folly::coro::Baton responseReceived, onFinalResponseCalled;
          Callback callback(
              onFirstResponseBool, responseReceived, onFinalResponseCalled);
          client.getChannelShared()->sendRequestSink(
              RpcOptions(),
              "initialThrow",
              SerializedRequest(folly::IOBuf::copyBuffer(req)),
              std::make_shared<transport::THeader>(),
              &callback);
          co_await responseReceived;
          if (onFirstResponseBool) {
            co_await onFinalResponseCalled;
          }
        }
      });
}

TEST_F(SinkServiceTest, SinkChunkTimeout) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_rangeChunkTimeout();

        bool exceptionThrown = false;
        try {
          co_await [&]() -> folly::coro::Task<void> {
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              // @lint-ignore CLANGTIDY bugprone-use-after-move
              for (int i = 0; i <= 100; i++) {
                if (i == 20) {
                  co_await folly::coro::sleep(std::chrono::milliseconds{500});
                }
                co_yield std::move(i);
              }
            }());
          }();
        } catch (const TApplicationException& ex) {
          exceptionThrown = true;
          EXPECT_EQ(TApplicationException::TIMEOUT, ex.getType());
        }
        EXPECT_TRUE(exceptionThrown);
      });
}

TEST_F(SinkServiceTest, ClientTimeoutNoLeak) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        EXPECT_THROW(
            co_await client.co_unSubscribedSinkSlowReturn(), std::exception);
        EXPECT_TRUE(co_await waitNoLeak(client));
      });
}

TEST_F(SinkServiceTest, AssignmentNoLeak) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          auto sink = co_await client.co_unSubscribedSink();
          sink = co_await client.co_unSubscribedSink();
        }
        EXPECT_TRUE(co_await waitNoLeak(client));
      });
}

TEST_F(SinkServiceTest, AlignedSink) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          apache::thrift::RpcOptions option;
          option.setMemAllocType(
              apache::thrift::RpcOptions::MemAllocType::ALLOC_PAGE_ALIGN);
          std::string s = "abcdefghijk";
          auto sink = co_await client.co_alignment(option, s);
          int32_t alignment = co_await sink.sink(
              [s]() -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
                co_yield std::move(*folly::IOBuf::copyBuffer(s));
              }());
          EXPECT_EQ(alignment, 0);
        }
      });
}

TEST_F(SinkServiceTest, CustomAllocSink) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          apache::thrift::RpcOptions option;
          option.setMemAllocType(
              apache::thrift::RpcOptions::MemAllocType::ALLOC_CUSTOM);
          std::string s = "abcdefghijk";
          auto sink = co_await client.co_custom(option, s);
          bool custom = co_await sink.sink(
              [s]() -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
                co_yield std::move(*folly::IOBuf::copyBuffer(s));
              }());
          EXPECT_FALSE(custom);
        }
      });
}

TEST_F(SinkServiceTest, CustomAllocSinkDefault) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          apache::thrift::RpcOptions option;
          option.setMemAllocType(
              apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT);
          std::string s = "abcdefghijk";
          auto sink = co_await client.co_custom(option, s);
          bool custom = co_await sink.sink(
              [s]() -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
                co_yield std::move(*folly::IOBuf::copyBuffer(s));
              }());
          EXPECT_FALSE(custom);
        }
      });
}

folly::coro::Task<void> neverStream() {
  folly::coro::Baton baton;
  folly::CancellationCallback cb{
      co_await folly::coro::co_current_cancellation_token,
      [&] { baton.post(); }};
  co_await baton;
}

TEST_F(SinkServiceTest, SinkEarlyFinalResponseWithLongWait) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        // return final response once received two numbers
        auto sink = co_await client.co_rangeEarlyResponse(0, 5, 2);
        int finalResponse =
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              co_yield 0;
              co_yield 1;
              // this long wait should get cancelled by final response
              co_await neverStream();
            }());
        EXPECT_EQ(2, finalResponse);
      });
}

TEST_F(SinkServiceTest, SinkEarlyClose) {
  std::vector<std::thread> ths;
  for (int i = 0; i < 100; i++) {
    ths.push_back(std::thread([this]() {
      connectToServer(
          [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
            auto sink = co_await client.co_range(0, 100);
          });
    }));
  }
  for (auto& th : ths) {
    th.join();
  }
}

TEST_F(SinkServiceTest, SinkEmptyRocketExceptionCrash) {
  connectToServer(
      [&](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_range(0, 100);
        server_->setIngressMemoryLimit(1);
        server_->setMinPayloadSizeToEnforceIngressMemoryLimit(0);
        EXPECT_THROW(co_await client.co_test(), TApplicationException);
        EXPECT_THROW(co_await sink.sink({}), rocket::RocketException);
      });
}

TEST_F(SinkServiceTest, SinkServerCancellation) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        // client sends values 0..100, server initiates cancellation at value 5
        auto sink = co_await client.co_rangeCancelAt(0, 100, 5);
        bool finalResponse =
            co_await sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
              // enter wait after 5 values, server should cancel
              // @lint-ignore CLANGTIDY bugprone-use-after-move
              for (int i = 0; i <= 5; i++) {
                co_yield std::move(i);
              }
              co_await neverStream();
            }());
        // server sends false as finalResponse after canceling sink
        EXPECT_FALSE(finalResponse);
      });
}

TEST_F(SinkServiceTest, SinkClientCancellation) {
  // cancel when async generator get stuck
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_unSubscribedSink();
        folly::CancellationSource cancelSource;

        folly::coro::co_invoke([&cancelSource]() -> folly::coro::Task<void> {
          co_await folly::coro::sleep(std::chrono::milliseconds{200});
          cancelSource.requestCancellation();
        })
            .scheduleOn(co_await folly::coro::co_current_executor)
            .start();

        EXPECT_THROW(
            co_await folly::coro::co_withCancellation(
                cancelSource.getToken(),
                sink.sink([]() -> folly::coro::AsyncGenerator<int&&> {
                  co_await neverStream();
                  // @lint-ignore CLANGTIDY bugprone-use-after-move
                  for (int i = 0; i <= 10; i++) {
                    co_yield std::move(i);
                  }
                }())),
            folly::OperationCancelled);
        co_await waitNoLeak(client);
      });

  // cancel when final response being slow
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        auto sink = co_await client.co_rangeSlowFinalResponse(0, 10);
        folly::CancellationSource cancelSource;

        EXPECT_THROW(
            co_await folly::coro::co_withCancellation(
                cancelSource.getToken(),
                sink.sink([&]() -> folly::coro::AsyncGenerator<int&&> {
                  // @lint-ignore CLANGTIDY bugprone-use-after-move
                  for (int i = 0; i <= 10; i++) {
                    co_yield std::move(i);
                  }
                  cancelSource.requestCancellation();
                }())),
            folly::OperationCancelled);
      });
}

TEST_F(SinkServiceTestAllocFn, CustomAllocSink) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          apache::thrift::RpcOptions option;
          option.setMemAllocType(
              apache::thrift::RpcOptions::MemAllocType::ALLOC_CUSTOM);
          std::string s = "abcdefghijk";
          auto sink = co_await client.co_custom(option, s);
          bool custom = co_await sink.sink(
              [s]() -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
                co_yield std::move(*folly::IOBuf::copyBuffer(s));
              }());
          EXPECT_TRUE(custom);
        }
      });
}

TEST_F(SinkServiceTestAllocFn, CustomAllocSinkDefault) {
  connectToServer(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        {
          apache::thrift::RpcOptions option;
          option.setMemAllocType(
              apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT);
          std::string s = "abcdefghijk";
          auto sink = co_await client.co_custom(option, s);
          bool custom = co_await sink.sink(
              [s]() -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
                co_yield std::move(*folly::IOBuf::copyBuffer(s));
              }());
          EXPECT_FALSE(custom);
        }
      });
}

TEST_F(SinkServiceTest, DuplicateStreamIdThrows) {
  connectToServer<DuplicateWriteSocket>(
      [](Client<TestSinkService>& client) -> folly::coro::Task<void> {
        // dummy request to send setup frame
        co_await client.co_test();
        // sink request frame will now be sent twice with the same stream id
        EXPECT_THROW(co_await client.co_range(0, 100), TTransportException);
      });
}

} // namespace thrift
} // namespace apache
