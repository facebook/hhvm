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

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/FastHandlerCallback.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

// Records the invocations made by the FastHandlerCallback's static
// function-pointer dispatch. The opaque ThriftServerAppAdapter* slot is
// reinterpreted to point at this struct so the callbacks can record into it
// without depending on the real adapter type.
struct CallRecorder {
  int resultCount{0};
  int doneCount{0};
  int exceptionCount{0};
  uint32_t lastStreamId{0};
  int lastValue{0};
  std::string lastExceptionMessage;
};

CallRecorder& asRecorder(ThriftServerAppAdapter* p) {
  return *reinterpret_cast<CallRecorder*>(p);
}

ThriftServerAppAdapter* asAdapter(CallRecorder* r) {
  return reinterpret_cast<ThriftServerAppAdapter*>(r);
}

void onResult(ThriftServerAppAdapter* a, uint32_t streamId, int value) {
  auto& r = asRecorder(a);
  r.resultCount++;
  r.lastStreamId = streamId;
  r.lastValue = value;
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void onException(
    ThriftServerAppAdapter* a, uint32_t streamId, folly::exception_wrapper ew) {
  auto& r = asRecorder(a);
  r.exceptionCount++;
  r.lastStreamId = streamId;
  r.lastExceptionMessage = ew.what().toStdString();
}

void onDone(ThriftServerAppAdapter* a, uint32_t streamId) {
  auto& r = asRecorder(a);
  r.doneCount++;
  r.lastStreamId = streamId;
}

constexpr uint32_t kStreamId = 42;

} // namespace

TEST(FastHandlerCallbackTest, ResultInvokesResultFnAndSuppressesDestructor) {
  CallRecorder rec;
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, asAdapter(&rec), kStreamId, &evb);
    cb->result(123);
  }
  // Single success invocation, no synthetic destructor exception.
  EXPECT_EQ(rec.resultCount, 1);
  EXPECT_EQ(rec.exceptionCount, 0);
  EXPECT_EQ(rec.lastStreamId, kStreamId);
  EXPECT_EQ(rec.lastValue, 123);
}

TEST(
    FastHandlerCallbackTest,
    ExceptionInvokesExceptionFnAndSuppressesDestructor) {
  CallRecorder rec;
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, asAdapter(&rec), kStreamId, &evb);
    cb->exception(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::UNKNOWN_METHOD, "boom"));
  }
  EXPECT_EQ(rec.resultCount, 0);
  EXPECT_EQ(rec.exceptionCount, 1);
  EXPECT_EQ(rec.lastStreamId, kStreamId);
  EXPECT_NE(rec.lastExceptionMessage.find("boom"), std::string::npos);
}

TEST(FastHandlerCallbackTest, DestructorFiresExceptionWhenNotCompleted) {
  CallRecorder rec;
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, asAdapter(&rec), kStreamId, &evb);
    // Drop without completing — destructor must synthesize an error so the
    // peer never hangs.
  }
  EXPECT_EQ(rec.resultCount, 0);
  EXPECT_EQ(rec.exceptionCount, 1);
  EXPECT_EQ(rec.lastStreamId, kStreamId);
  EXPECT_NE(rec.lastExceptionMessage.find("not completed"), std::string::npos);
}

TEST(FastHandlerCallbackTest, VoidDoneInvokesDoneFnAndSuppressesDestructor) {
  CallRecorder rec;
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<void>>(
        &onDone, &onException, asAdapter(&rec), kStreamId, &evb);
    cb->done();
  }
  EXPECT_EQ(rec.doneCount, 1);
  EXPECT_EQ(rec.exceptionCount, 0);
  EXPECT_EQ(rec.lastStreamId, kStreamId);
}

TEST(FastHandlerCallbackTest, VoidDestructorFiresExceptionWhenNotCompleted) {
  CallRecorder rec;
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<void>>(
        &onDone, &onException, asAdapter(&rec), kStreamId, &evb);
  }
  EXPECT_EQ(rec.doneCount, 0);
  EXPECT_EQ(rec.exceptionCount, 1);
  EXPECT_NE(rec.lastExceptionMessage.find("not completed"), std::string::npos);
}

TEST(FastHandlerCallbackTest, GetEventBaseReturnsConfiguredEventBase) {
  CallRecorder rec;
  folly::EventBase evb;
  auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
      &onResult, &onException, asAdapter(&rec), kStreamId, &evb);
  EXPECT_EQ(cb->getEventBase(), &evb);
  cb->result(0); // suppress destructor exception
}

} // namespace apache::thrift::fast_thrift::thrift
