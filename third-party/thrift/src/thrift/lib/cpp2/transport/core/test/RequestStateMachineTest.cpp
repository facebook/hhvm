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

#include <thrift/lib/cpp2/transport/core/RequestStateMachine.h>

#include <gtest/gtest.h>

#include <folly/io/async/EventBase.h>
#include <folly/observer/SimpleObservable.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

namespace apache::thrift {
namespace {

class RequestStateMachineTest : public testing::Test {
 protected:
  RequestStateMachine makeStateMachine() {
    return RequestStateMachine(
        /*includeInRecentRequests=*/false, controller_, nullptr);
  }

  folly::EventBase eventBase_;

 private:
  folly::observer::SimpleObservable<AdaptiveConcurrencyController::Config>
      config_{AdaptiveConcurrencyController::Config{}};
  folly::observer::SimpleObservable<uint32_t> maxRequests_{0u};
  ThriftServerConfig serverConfig_;
  AdaptiveConcurrencyController controller_{
      config_.getObserver(), maxRequests_.getObserver(), serverConfig_};
};

TEST_F(RequestStateMachineTest, FreshRequestIsActiveAndNotTerminated) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.isActive());
  EXPECT_EQ(
      stateMachine.getTerminationCause(),
      RequestTerminationCause::NotTerminated);
}

TEST_F(RequestStateMachineTest, NoArgTerminationRecordsUnknown) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(&eventBase_));

  EXPECT_FALSE(stateMachine.isActive());
  EXPECT_EQ(
      stateMachine.getTerminationCause(), RequestTerminationCause::Unknown);
}

TEST_F(RequestStateMachineTest, RequestFinishedCauseIsObservable) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::RequestFinished));

  EXPECT_FALSE(stateMachine.isActive());
  EXPECT_EQ(
      stateMachine.getTerminationCause(),
      RequestTerminationCause::RequestFinished);
}

TEST_F(RequestStateMachineTest, TaskTimeoutCauseIsObservable) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::TaskTimeout));

  EXPECT_FALSE(stateMachine.isActive());
  EXPECT_EQ(
      stateMachine.getTerminationCause(), RequestTerminationCause::TaskTimeout);
}

TEST_F(RequestStateMachineTest, QueueTimeoutCauseIsObservable) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::QueueTimeout));

  EXPECT_FALSE(stateMachine.isActive());
  EXPECT_EQ(
      stateMachine.getTerminationCause(),
      RequestTerminationCause::QueueTimeout);
}

TEST_F(RequestStateMachineTest, UnknownFirstBlocksLaterSpecificCause) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(&eventBase_));
  EXPECT_FALSE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::TaskTimeout));

  EXPECT_EQ(
      stateMachine.getTerminationCause(), RequestTerminationCause::Unknown);
}

TEST_F(RequestStateMachineTest, SpecificCauseFirstBlocksLaterUnknown) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::TaskTimeout));
  EXPECT_FALSE(stateMachine.tryTerminate(&eventBase_));

  EXPECT_EQ(
      stateMachine.getTerminationCause(), RequestTerminationCause::TaskTimeout);
}

TEST_F(RequestStateMachineTest, NotTerminatedCauseIsRejected) {
  auto stateMachine = makeStateMachine();

  EXPECT_DEATH(
      static_cast<void>(stateMachine.tryTerminate(
          &eventBase_, RequestTerminationCause::NotTerminated)),
      "must record a terminal cause");
}

TEST_F(RequestStateMachineTest, ProcessingCannotStartAfterTermination) {
  auto stateMachine = makeStateMachine();

  EXPECT_TRUE(stateMachine.tryTerminate(
      &eventBase_, RequestTerminationCause::TaskTimeout));

  EXPECT_FALSE(stateMachine.tryStartProcessing());
}

// A leaf request whose termination cause is set by the test.
class FakeRequest : public ResponseChannelRequest {
 public:
  explicit FakeRequest(RequestTerminationCause cause) : cause_(cause) {}

  bool isActive() const override {
    return cause_ == RequestTerminationCause::NotTerminated;
  }
  RequestTerminationCause getTerminationCause() const override {
    return cause_;
  }
  bool isOneway() const override { return false; }
  bool includeEnvelope() const override { return false; }
  bool tryStartProcessing() override { return false; }
  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}
  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}

 private:
  const RequestTerminationCause cause_;
};

// Mirrors the transparent decorators that forward the request interface to an
// inner request.
class ForwardingRequest : public ResponseChannelRequest {
 public:
  explicit ForwardingRequest(std::unique_ptr<FakeRequest> inner)
      : inner_(std::move(inner)) {}

  bool isActive() const override { return inner_->isActive(); }
  RequestTerminationCause getTerminationCause() const override {
    return inner_->getTerminationCause();
  }
  bool isOneway() const override { return inner_->isOneway(); }
  bool includeEnvelope() const override { return inner_->includeEnvelope(); }
  bool tryStartProcessing() override { return inner_->tryStartProcessing(); }
  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}
  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}

 private:
  const std::unique_ptr<FakeRequest> inner_;
};

TEST(ResponseChannelRequestTest, DefaultTerminationCauseIsUnknown) {
  ForwardingRequest request(
      std::make_unique<FakeRequest>(RequestTerminationCause::NotTerminated));

  EXPECT_EQ(
      request.ResponseChannelRequest::getTerminationCause(),
      RequestTerminationCause::Unknown);
}

TEST(ResponseChannelRequestTest, ForwardingDecoratorPropagatesTaskTimeout) {
  ForwardingRequest request(
      std::make_unique<FakeRequest>(RequestTerminationCause::TaskTimeout));

  EXPECT_EQ(
      request.getTerminationCause(), RequestTerminationCause::TaskTimeout);
}

TEST(ResponseChannelRequestTest, ForwardingDecoratorPropagatesRequestFinished) {
  ForwardingRequest request(
      std::make_unique<FakeRequest>(RequestTerminationCause::RequestFinished));

  EXPECT_EQ(
      request.getTerminationCause(), RequestTerminationCause::RequestFinished);
}

} // namespace
} // namespace apache::thrift
