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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/coro/GtestHelpers.h>

#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/InterceptorFrameworkMetadata.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_clients.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_handlers.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;
using apache::thrift::test::ClientInterceptorTest;
using apache::thrift::test::FrameworkMetadata;

namespace apache::thrift {

namespace {

constexpr std::string_view kInitialFrameworkMetadataValue = "FRAMEWORK_INIT";
constexpr std::string_view kWrittenByInterceptorValue =
    "WRITTEN_BY_INTERCEPTOR";
constexpr std::string_view kPostProcessing = "POSTPROCESSING";

} // namespace

namespace detail {

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    InterceptorFrameworkMetadataStorage,
    initializeInterceptorFrameworkMetadataStorage) {
  FrameworkMetadata metadata;
  metadata.value() = kInitialFrameworkMetadataValue;
  InterceptorFrameworkMetadataStorage storage;
  storage.emplace<FrameworkMetadata>(std::move(metadata));
  return storage;
}

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    void,
    postProcessFrameworkMetadata,
    InterceptorFrameworkMetadataStorage& storage,
    const RpcOptions&) {
  auto& frameworkMetadata = storage.value<FrameworkMetadata>();
  frameworkMetadata.postProcessing() = kPostProcessing;
}

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    std::unique_ptr<folly::IOBuf>,
    serializeFrameworkMetadata,
    InterceptorFrameworkMetadataStorage&& frameworkMetadata) {
  return CompactSerializer::serialize<folly::IOBufQueue>(
             frameworkMetadata.value<FrameworkMetadata>())
      .move();
}

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    InterceptorFrameworkMetadataStorage,
    deserializeFrameworkMetadata,
    const folly::IOBuf& frameworkMetadataBuf) {
  InterceptorFrameworkMetadataStorage ret;
  ret.emplace<FrameworkMetadata>(
      CompactSerializer::deserialize<FrameworkMetadata>(&frameworkMetadataBuf));
  return ret;
}

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    void,
    handleFrameworkMetadata,
    std::unique_ptr<folly::IOBuf>&& frameworkMetadata,
    Cpp2RequestContext* requestContextPtr) {
  Cpp2RequestContextUnsafeAPI(*requestContextPtr)
      .initializeInterceptorFrameworkMetadata(*frameworkMetadata);
}

} // namespace detail

namespace {

struct EmptyRequestState {};

struct WriteFrameworkMetadataInterceptor final
    : public ClientInterceptor<EmptyRequestState> {
 public:
  std::string getName() const override {
    return "WriteFrameworkMetadataInterceptor";
  }

  std::optional<EmptyRequestState> onRequest(RequestInfo requestInfo) override {
    InterceptorFrameworkMetadataStorage& storage =
        *requestInfo.frameworkMetadata;
    FrameworkMetadata& typedMetadata = storage.value<FrameworkMetadata>();
    typedMetadata.value() = kWrittenByInterceptorValue;
    return std::nullopt;
  }
};

class ReadFrameworkMetadataInterceptor final
    : public ServiceInterceptor<EmptyRequestState, EmptyRequestState> {
 public:
  explicit ReadFrameworkMetadataInterceptor()
      : metadataReadByServiceInterceptor_{nullptr} {}

  std::string getName() const override {
    return "ReadFrameworkMetadataInterceptor";
  }

  folly::coro::Task<std::optional<EmptyRequestState>> onRequest(
      EmptyRequestState*, RequestInfo requestInfo) override {
    if (requestInfo.frameworkMetadata != nullptr) {
      metadataReadByServiceInterceptor_ = std::make_unique<FrameworkMetadata>(
          requestInfo.frameworkMetadata->value<FrameworkMetadata>());
    }
    co_return std::nullopt;
  }

  const FrameworkMetadata* getMetadataReadByServiceInterceptor() {
    return metadataReadByServiceInterceptor_.get();
  }

 private:
  std::unique_ptr<FrameworkMetadata> metadataReadByServiceInterceptor_;
};

class TestModule : public ServerModule {
 public:
  explicit TestModule(
      std::shared_ptr<ReadFrameworkMetadataInterceptor> interceptor) {
    interceptors_.emplace_back(std::move(interceptor));
  }

  std::vector<std::shared_ptr<ServiceInterceptorBase>> getServiceInterceptors()
      override {
    return interceptors_;
  }

 private:
  std::string getName() const override { return "TestModule"; }

  std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_{};
};

struct TestHandler : public ServiceHandler<ClientInterceptorTest> {
  explicit TestHandler(folly::coro::Baton& fireAndForgetBaton)
      : fireAndForgetBaton_{fireAndForgetBaton} {}

  folly::coro::Task<void> co_noop(bool) override { co_return; }

  folly::coro::Task<ServerStream<std::int32_t>> co_iota(
      std::int32_t start, bool) override {
    co_return folly::coro::co_invoke(
        [current =
             start]() mutable -> folly::coro::AsyncGenerator<std::int32_t&&> {
          while (true) {
            co_yield current++;
          }
        });
  }

  SinkConsumer<std::int32_t, std::int32_t> dump() override {
    return SinkConsumer<std::int32_t, std::int32_t>{
        [](folly::coro::AsyncGenerator<std::int32_t&&> gen)
            -> folly::coro::Task<std::int32_t> {
          std::int32_t count = 0;
          while (auto chunk = co_await gen.next()) {
            count++;
          }
          co_return count;
        }};
  }

  folly::coro::Task<void> co_fireAndForget(std::int32_t) override {
    fireAndForgetBaton_.post();
    co_return;
  }

  folly::coro::Baton& fireAndForgetBaton_;
};

class InterceptorFrameworkMetadataTest : public Test {
 public:
  explicit InterceptorFrameworkMetadataTest()
      : serviceInterceptor_{std::make_shared<
            ReadFrameworkMetadataInterceptor>()},
        handler_{std::make_shared<TestHandler>(fireAndForgetBaton_)},
        server_{std::make_shared<ScopedServerInterfaceThread>(
            handler_, [this](ThriftServer& server) {
              server.addModule(
                  std::make_unique<TestModule>(serviceInterceptor_));
            })} {
    THRIFT_FLAG_SET_MOCK(enable_client_interceptor_framework_metadata, true);
    THRIFT_FLAG_SET_MOCK(enable_service_interceptor_framework_metadata, true);
  }

  std::unique_ptr<Client<ClientInterceptorTest>> client() {
    using InterceptorPtrVec =
        std::vector<std::shared_ptr<ClientInterceptorBase>>;
    InterceptorPtrVec interceptorPtrVec = {
        std::make_shared<WriteFrameworkMetadataInterceptor>()};

    return server_->newClientWithInterceptors<Client<ClientInterceptorTest>>(
        folly::getGlobalIOExecutor().get(),
        [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
          return RocketClientChannel::newChannel(std::move(socket));
        },
        protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL,
        std::make_shared<InterceptorPtrVec>(std::move(interceptorPtrVec)));
  }

  folly::coro::Baton& getFireAndForgetBaton() { return fireAndForgetBaton_; }

  const FrameworkMetadata* getMetadataReadByServiceInterceptor() {
    return serviceInterceptor_->getMetadataReadByServiceInterceptor();
  }

 private:
  folly::coro::Baton fireAndForgetBaton_;
  std::shared_ptr<ReadFrameworkMetadataInterceptor> serviceInterceptor_;
  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<ScopedServerInterfaceThread> server_;
};

} // namespace

CO_TEST_F(InterceptorFrameworkMetadataTest, RocketChannelRequestResponse) {
  co_await client()->co_noop(false);
  const FrameworkMetadata* interceptedMetadata =
      getMetadataReadByServiceInterceptor();
  EXPECT_NE(interceptedMetadata, nullptr);
  EXPECT_EQ(*interceptedMetadata->value(), kWrittenByInterceptorValue);
  EXPECT_EQ(*interceptedMetadata->postProcessing(), kPostProcessing);
}

CO_TEST_F(InterceptorFrameworkMetadataTest, RocketChannelStream) {
  {
    auto testClient = client();
    auto stream = (co_await testClient->co_iota(1, false)).toAsyncGenerator();
    EXPECT_EQ((co_await stream.next()).value(), 1);
    // close stream
  }
  const FrameworkMetadata* interceptedMetadata =
      getMetadataReadByServiceInterceptor();
  EXPECT_NE(interceptedMetadata, nullptr);
  EXPECT_EQ(*interceptedMetadata->value(), kWrittenByInterceptorValue);
  EXPECT_EQ(*interceptedMetadata->postProcessing(), kPostProcessing);
}

CO_TEST_F(InterceptorFrameworkMetadataTest, RocketChannelSink) {
  {
    auto testClient = client();
    auto sink = co_await testClient->co_dump();
    std::size_t response =
        co_await sink.sink([]() -> folly::coro::AsyncGenerator<std::int32_t&&> {
          co_yield 1;
          co_yield 2;
          co_yield 3;
        }());
    EXPECT_EQ(response, 3);
  }
  const FrameworkMetadata* interceptedMetadata =
      getMetadataReadByServiceInterceptor();
  EXPECT_NE(interceptedMetadata, nullptr);
  EXPECT_EQ(*interceptedMetadata->value(), kWrittenByInterceptorValue);
  EXPECT_EQ(*interceptedMetadata->postProcessing(), kPostProcessing);
}

CO_TEST_F(InterceptorFrameworkMetadataTest, RocketChannelRequestNoResponse) {
  co_await client()->co_fireAndForget(0);
  co_await getFireAndForgetBaton();

  const FrameworkMetadata* interceptedMetadata =
      getMetadataReadByServiceInterceptor();
  EXPECT_NE(interceptedMetadata, nullptr);
  EXPECT_EQ(*interceptedMetadata->value(), kWrittenByInterceptorValue);
  EXPECT_EQ(*interceptedMetadata->postProcessing(), kPostProcessing);
}

} // namespace apache::thrift
