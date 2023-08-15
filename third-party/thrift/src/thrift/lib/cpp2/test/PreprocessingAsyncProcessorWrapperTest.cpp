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

#include <type_traits>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/async/PreprocessingAsyncProcessorWrapper.h>

namespace apache::thrift::test {
using namespace ::testing;

class MockAsyncProcessor : public AsyncProcessor {
 public:
  const char* getServiceName() override { return "MockAsyncProcessor"; }

  MOCK_METHOD7(
      processSerializedCompressedRequestWithMetadata,
      void(
          ResponseChannelRequest::UniquePtr,
          SerializedCompressedRequest&&,
          const MethodMetadata&,
          protocol::PROTOCOL_TYPES,
          Cpp2RequestContext*,
          folly::EventBase*,
          concurrency::ThreadManager*));

  MOCK_METHOD2(
      executeRequest,
      void(
          ServerRequest&& request,
          const AsyncProcessorFactory::MethodMetadata& methodMetadata));
};

class TestChannelRequest : public ResponseChannelRequest {
 public:
  using ResponseChannelRequest::ResponseChannelRequest;
  bool isActive() const override { return false; }
  bool includeEnvelope() const override { return false; }
  bool isOneway() const override { return false; }
  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}
  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}

 protected:
  bool tryStartProcessing() override { return false; }
};

class TestPreprocessingAsyncProcessorWrapper
    : public PreprocessingAsyncProcessorWrapper {
 public:
  explicit TestPreprocessingAsyncProcessorWrapper(
      std::unique_ptr<AsyncProcessor> processor)
      : PreprocessingAsyncProcessorWrapper(std::move(processor)) {}

  const char* getServiceName() override {
    return "TestPreprocessingAsyncProcessorWrapper";
  }

  PreprocessingAsyncProcessorWrapper::ProcessSerializedCompressedRequestReturnT
  processSerializedCompressedRequestWithMetadataImpl(
      ResponseChannelRequest::UniquePtr,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata&,
      protocol::PROTOCOL_TYPES,
      Cpp2RequestContext*,
      folly::EventBase*,
      concurrency::ThreadManager*) override {
    return {
        ResponseChannelRequest::UniquePtr(new TestChannelRequest()),
        std::move(serializedRequest)};
  }

  ServerRequest executeRequestImpl(
      ServerRequest&& req,
      const AsyncProcessorFactory::MethodMetadata&) override {
    return std::move(req);
  }
};

TEST(PreprocessingAsyncProcessorWrapperTest, getInnerTest) {
  TestPreprocessingAsyncProcessorWrapper nestedProcessor(
      std::make_unique<MockAsyncProcessor>());
  EXPECT_TRUE(nestedProcessor.inner());
  TestPreprocessingAsyncProcessorWrapper emptyNestedProcessor(nullptr);
  EXPECT_FALSE(emptyNestedProcessor.inner());
}

MATCHER(CanBeUpCastedToTestChannelRequest, "") {
  auto* casted = dynamic_cast<TestChannelRequest*>(arg.get());
  return casted != nullptr;
}

TEST(PreprocessingAsyncProcessorWrapperTest, innerProcessorInvokeTest) {
  auto mockAsyncProcessor = std::make_unique<MockAsyncProcessor>();
  EXPECT_CALL(
      *mockAsyncProcessor,
      processSerializedCompressedRequestWithMetadata(
          CanBeUpCastedToTestChannelRequest(), _, _, _, _, _, _))
      .Times(1);
  EXPECT_CALL(*mockAsyncProcessor, executeRequest).Times(1);
  auto nestedProcessor =
      std::make_unique<TestPreprocessingAsyncProcessorWrapper>(
          std::move(mockAsyncProcessor));
  nestedProcessor->processSerializedCompressedRequestWithMetadata(
      nullptr,
      SerializedCompressedRequest(nullptr),
      AsyncProcessorFactory::MethodMetadata{},
      protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL,
      nullptr,
      nullptr,
      nullptr);
  nestedProcessor->executeRequest(
      ServerRequest{}, AsyncProcessorFactory::MethodMetadata{});
}
} // namespace apache::thrift::test
