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

#include <gtest/gtest.h>

#include <folly/Format.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/InterceptorFlags.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>

namespace apache::thrift {

namespace detail {

THRIFT_PLUGGABLE_FUNC_SET(
    InterceptorFrameworkMetadataStorage,
    initializeInterceptorFrameworkMetadataStorage) {
  std::string value = "test";
  InterceptorFrameworkMetadataStorage storage;
  storage.emplace<std::string>(value);
  return storage;
}

THRIFT_PLUGGABLE_FUNC_SET(
    void,
    postProcessFrameworkMetadata,
    InterceptorFrameworkMetadataStorage& storage,
    const RpcOptions&) {
  storage.emplace<std::string>(
      folly::sformat("{}_postprocessed", storage.value<std::string>()));
}

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    std::unique_ptr<folly::IOBuf>,
    serializeFrameworkMetadata,
    InterceptorFrameworkMetadataStorage&& storage) {
  EXPECT_TRUE(storage.has_value());
  EXPECT_TRUE(storage.holds_alternative<std::string>());
  return folly::IOBuf::fromString(std::move(storage.value<std::string>()));
}

} // namespace detail

namespace test {

using EventHandlerList = std::vector<std::shared_ptr<TProcessorEventHandler>>;

TEST(ContextStack, NoAllocationWhenEmpty) {
  {
    auto contextStack = ContextStack::create(
        nullptr /* handlers */,
        "Service",
        "Service.method",
        nullptr /* connectionContext */);
    EXPECT_EQ(contextStack, nullptr);
  }

  {
    auto handlers = std::make_shared<EventHandlerList>();
    auto contextStack = ContextStack::create(
        handlers, "Service", "Service.method", nullptr /* connectionContext */);
    EXPECT_EQ(contextStack, nullptr);
  }
}

TEST(ContextStack, LegacyEventHandlersInvoked) {
  auto handler1 = std::make_shared<TrackingTProcessorEventHandler>();
  auto handler2 = std::make_shared<TrackingTProcessorEventHandler>();
  auto handlers =
      std::make_shared<EventHandlerList>(EventHandlerList{handler1, handler2});

  auto contextStack = ContextStack::create(
      handlers, "Service", "Service.method", nullptr /* connectionContext */);
  ASSERT_NE(contextStack, nullptr);

  contextStack->preRead();
  contextStack->preWrite();

  static const std::vector<std::string> kExpected = {
      "getServiceContext('Service', 'Service.method')",
      "preRead('Service.method')",
      "preWrite('Service.method')",
  };
  EXPECT_EQ(handler1->getHistory(), kExpected);
  EXPECT_EQ(handler2->getHistory(), kExpected);

  // TrackingTProcessorEventHandler::getServiceContext returns `this`
  auto unsafeAPI = detail::ContextStackUnsafeAPI(*contextStack);
  EXPECT_EQ(unsafeAPI.contextAt(0), handler1.get());
  EXPECT_EQ(unsafeAPI.contextAt(1), handler2.get());
}

TEST(ContextStack, ClientHeaders) {
  class HeaderSettingEventHandler : public TrackingTProcessorEventHandler {
   private:
    using Base = TrackingTProcessorEventHandler;

    void* getServiceContext(
        std::string_view serviceName,
        std::string_view methodName,
        apache::thrift::TConnectionContext* connectionContext) override {
      Base::getServiceContext(serviceName, methodName, connectionContext);
      return connectionContext;
    }

    void preRead(void* ctx, std::string_view functionName) override {
      auto* connectionContext =
          static_cast<apache::thrift::TConnectionContext*>(ctx);
      connectionContext->setHeader("preRead", "1");
      Base::preRead(ctx, functionName);
    }

    void preWrite(void* ctx, std::string_view functionName) override {
      auto* connectionContext =
          static_cast<apache::thrift::TConnectionContext*>(ctx);
      connectionContext->setHeader("preWrite", "1");
      return Base::preWrite(ctx, functionName);
    }
  };

  for (bool copyNames : {false, true}) {
    transport::THeader header;
    auto handler = std::make_shared<HeaderSettingEventHandler>();
    auto contextStack = [&]() {
      auto handlers =
          std::make_shared<EventHandlerList>(EventHandlerList{handler});
      return copyNames ? ContextStack::createWithClientContextCopyNames(
                             handlers,
                             nullptr /* clientInterceptors */,
                             "Service",
                             "method",
                             header)
                       : ContextStack::createWithClientContext(
                             handlers,
                             nullptr /* clientInterceptors */,
                             "Service",
                             "Service.method",
                             header);
    }();
    ASSERT_NE(contextStack, nullptr);

    contextStack->preRead();
    contextStack->preWrite();

    static const std::vector<std::string> kExpected = {
        "getServiceContext('Service', 'Service.method')",
        "preRead('Service.method')",
        "preWrite('Service.method')",
    };
    EXPECT_EQ(handler->getHistory(), kExpected);
    auto writeHeaders = header.releaseWriteHeaders();
    EXPECT_EQ(writeHeaders.at("preRead"), "1");
    EXPECT_EQ(writeHeaders.at("preWrite"), "1");
  }
}

TEST(ContextStack, FrameworkMetadataInitialized) {
  THRIFT_FLAG_SET_MOCK(enable_client_interceptor_framework_metadata, true);
  auto handler1 = std::make_shared<TrackingTProcessorEventHandler>();
  auto handler2 = std::make_shared<TrackingTProcessorEventHandler>();
  auto handlers =
      std::make_shared<EventHandlerList>(EventHandlerList{handler1, handler2});

  auto contextStack = ContextStack::create(
      handlers, "Service", "Service.method", nullptr /* connectionContext */);
  ASSERT_NE(contextStack, nullptr);

  std::unique_ptr<folly::IOBuf> metadataBuf =
      detail::ContextStackUnsafeAPI(*contextStack)
          .getInterceptorFrameworkMetadata(RpcOptions());
  EXPECT_TRUE(metadataBuf != nullptr);

  std::string metadataStr = metadataBuf->toString();
  EXPECT_EQ(metadataStr, "test_postprocessed");
}

} // namespace test

} // namespace apache::thrift
