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
