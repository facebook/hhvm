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
// function-pointer dispatch. Subclasses ThriftServerAppAdapter so the
// callback's adapterGuard_ (a DestructorGuard) operates on a real
// DelayedDestruction; the static dispatch fns downcast to read counters.
class RecordingAdapter : public ThriftServerAppAdapter {
 public:
  int resultCount{0};
  int doneCount{0};
  int exceptionCount{0};
  uint32_t lastStreamId{0};
  int lastValue{0};
  std::string lastExceptionMessage;

 protected:
  ~RecordingAdapter() override = default;
};

RecordingAdapter& asRecorder(ThriftServerAppAdapter* p) {
  return *static_cast<RecordingAdapter*>(p);
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

using RecordingAdapterPtr =
    folly::DelayedDestructionUniquePtr<RecordingAdapter>;

RecordingAdapterPtr makeRecorder() {
  return folly::makeDelayedDestructionUniquePtr<RecordingAdapter>();
}

constexpr uint32_t kStreamId = 42;

} // namespace

TEST(FastHandlerCallbackTest, ResultInvokesResultFnAndSuppressesDestructor) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, rec.get(), kStreamId, &evb, nullptr);
    cb->result(123);
  }
  // Single success invocation, no synthetic destructor exception.
  EXPECT_EQ(rec->resultCount, 1);
  EXPECT_EQ(rec->exceptionCount, 0);
  EXPECT_EQ(rec->lastStreamId, kStreamId);
  EXPECT_EQ(rec->lastValue, 123);
}

TEST(
    FastHandlerCallbackTest,
    ExceptionInvokesExceptionFnAndSuppressesDestructor) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, rec.get(), kStreamId, &evb, nullptr);
    cb->exception(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::UNKNOWN_METHOD, "boom"));
  }
  EXPECT_EQ(rec->resultCount, 0);
  EXPECT_EQ(rec->exceptionCount, 1);
  EXPECT_EQ(rec->lastStreamId, kStreamId);
  EXPECT_NE(rec->lastExceptionMessage.find("boom"), std::string::npos);
}

TEST(FastHandlerCallbackTest, DestructorFiresExceptionWhenNotCompleted) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
        &onResult, &onException, rec.get(), kStreamId, &evb, nullptr);
    // Drop without completing — destructor must synthesize an error so the
    // peer never hangs.
  }
  EXPECT_EQ(rec->resultCount, 0);
  EXPECT_EQ(rec->exceptionCount, 1);
  EXPECT_EQ(rec->lastStreamId, kStreamId);
  EXPECT_NE(rec->lastExceptionMessage.find("not completed"), std::string::npos);
}

TEST(FastHandlerCallbackTest, VoidDoneInvokesDoneFnAndSuppressesDestructor) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<void>>(
        &onDone, &onException, rec.get(), kStreamId, &evb, nullptr);
    cb->done();
  }
  EXPECT_EQ(rec->doneCount, 1);
  EXPECT_EQ(rec->exceptionCount, 0);
  EXPECT_EQ(rec->lastStreamId, kStreamId);
}

TEST(FastHandlerCallbackTest, VoidDestructorFiresExceptionWhenNotCompleted) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  {
    auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<void>>(
        &onDone, &onException, rec.get(), kStreamId, &evb, nullptr);
  }
  EXPECT_EQ(rec->doneCount, 0);
  EXPECT_EQ(rec->exceptionCount, 1);
  EXPECT_NE(rec->lastExceptionMessage.find("not completed"), std::string::npos);
}

TEST(FastHandlerCallbackTest, GetEventBaseReturnsConfiguredEventBase) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
      &onResult, &onException, rec.get(), kStreamId, &evb, nullptr);
  EXPECT_EQ(cb->getEventBase(), &evb);
  cb->result(0); // suppress destructor exception
}

TEST(FastHandlerCallbackTest, RequestContextAccessorReturnsStoredPointer) {
  auto rec = makeRecorder();
  folly::EventBase evb;
  auto requestContext = std::make_unique<ThriftRequestContext>();
  auto* requestContextPtr = requestContext.get();
  auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
      &onResult,
      &onException,
      rec.get(),
      kStreamId,
      &evb,
      std::move(requestContext));
  EXPECT_EQ(cb->requestContext(), requestContextPtr);
  cb->result(0); // suppress destructor exception
}

// =============================================================================
// Force-close behavior: FHC keeps adapter alive across straggler completions
// =============================================================================

TEST(FastHandlerCallbackTest, OutlivingFHCKeepsAdapterAlive) {
  // Simulates the force-close scenario: the upper layer drops its
  // adapter Ptr while an FHC is still alive. The FHC's DG must hold
  // the adapter object alive so the straggler completion is safe.
  folly::EventBase evb;
  auto rec = makeRecorder();
  RecordingAdapter* recPtr = rec.get();

  auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
      &onResult, &onException, recPtr, kStreamId, &evb, nullptr);

  // Owner drops its Ptr while the FHC is still alive — the FHC's
  // adapterGuard_ must hold the adapter live.
  rec.reset();

  // Recorder is still alive: the static dispatch fn writes into it
  // successfully (would UAF if adapter had been freed).
  cb->result(99);
  EXPECT_EQ(recPtr->resultCount, 1);
  EXPECT_EQ(recPtr->lastValue, 99);

  // Releasing the FHC drops the last DG; adapter dies cleanly now.
  cb.reset();
}

TEST(FastHandlerCallbackTest, UncompletedFHCDestructorIsSafeAfterOwnerDrop) {
  // FHC dtor synthesizes an INTERNAL_ERROR and dispatches it through
  // the adapter. After the upper-layer Ptr is dropped, the FHC's
  // adapterGuard_ must keep the adapter alive across this dispatch.
  // ASAN would catch a UAF here; passing means the lifetime chain is
  // correct.
  //
  // We hold a separate DG so the adapter survives past the FHC's
  // destruction, letting us inspect the recorded exception state.
  folly::EventBase evb;
  auto rec = makeRecorder();
  RecordingAdapter* recPtr = rec.get();

  auto cb = folly::makeDelayedDestructionUniquePtr<FastHandlerCallback<int>>(
      &onResult, &onException, recPtr, kStreamId, &evb, nullptr);

  // Test-only: keep the adapter alive past cb.reset() so we can
  // observe state. In production, dropping the owner Ptr leaves only
  // the FHC's DG; the adapter destructs the moment the FHC does.
  folly::DelayedDestruction::DestructorGuard keepAlive(recPtr);
  rec.reset();
  cb.reset();

  EXPECT_EQ(recPtr->exceptionCount, 1);
  EXPECT_NE(
      recPtr->lastExceptionMessage.find("not completed"), std::string::npos);
}

} // namespace apache::thrift::fast_thrift::thrift
