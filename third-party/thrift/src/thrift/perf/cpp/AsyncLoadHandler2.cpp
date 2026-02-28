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

#include <thrift/perf/cpp/AsyncLoadHandler2.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/Unistd.h>

using namespace apache::thrift::test;

namespace apache::thrift {
static constexpr int64_t US_PER_MS = std::micro::den / std::milli::den;

void AsyncLoadHandler2::async_eb_noop(HandlerCallbackPtr<void> callback) {
  // Note that we could have done this with a sync function,
  // but an inline async op is faster, and we want to maintain
  // parity with the old loadgen for comparison testing
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewayNoop(
    HandlerCallbackOneWay::Ptr callback) {
  HandlerCallbackOneWay::CompletionGuard completionGuard{std::move(callback)};
}

void AsyncLoadHandler2::async_eb_asyncNoop(HandlerCallbackPtr<void> callback) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_sleep(
    HandlerCallbackPtr<void> callback, int64_t microseconds) {
  // May leak if task never finishes
  HandlerCallback<void>* callbackp = callback.unsafeRelease();
  callbackp->getEventBase()->runInEventBaseThread([=]() {
    callbackp->getEventBase()->tryRunAfterDelay(
        [=]() {
          HandlerCallbackPtr<void> cb(
              HandlerCallbackPtr<void>::UnsafelyFromRawPointer(), callbackp);
          cb->done();
        },
        microseconds / US_PER_MS);
  });
}

void AsyncLoadHandler2::async_eb_onewaySleep(
    HandlerCallbackOneWay::Ptr callback, int64_t microseconds) {
  // May leak if task never finishes
  auto eb = callback->getEventBase();
  eb->runInEventBaseThread([=]() {
    eb->tryRunAfterDelay([=]() { callback->done(); }, microseconds / US_PER_MS);
  });
}

void AsyncLoadHandler2::sync_burn(int64_t microseconds) {
  // Slightly different from thrift1, this happens in a
  // thread pool.
  auto start = std::chrono::steady_clock::now();
  auto end = start + std::chrono::microseconds(microseconds);
  auto now = start;
  do {
    now = std::chrono::steady_clock::now();
  } while (now < end);
}

folly::Future<folly::Unit> AsyncLoadHandler2::future_burn(
    int64_t microseconds) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();

  sync_burn(microseconds);
  promise.setValue();

  return future;
}

void AsyncLoadHandler2::sync_onewayBurn(int64_t microseconds) {
  sync_burn(microseconds);
}

folly::Future<folly::Unit> AsyncLoadHandler2::future_onewayBurn(
    int64_t microseconds) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();

  sync_onewayBurn(microseconds);
  promise.setValue();

  return future;
}

void AsyncLoadHandler2::async_eb_badSleep(
    HandlerCallbackPtr<void> callback, int64_t microseconds) {
  usleep(microseconds);
  callback->done();
}

void AsyncLoadHandler2::async_eb_badBurn(
    HandlerCallbackPtr<void> callback, int64_t microseconds) {
  // This is a true (bad) async call.
  sync_burn(microseconds);
  callback->done();
}

void AsyncLoadHandler2::async_eb_throwError(
    HandlerCallbackPtr<void> callback, int32_t code) {
  LoadError error;
  *error.code() = code;
  callback->exception(error);
}

void AsyncLoadHandler2::async_eb_throwUnexpected(
    HandlerCallbackPtr<void> callback, int32_t /* code */) {
  // FIXME: it isn't possible to implement this behavior with the async code
  //
  // Actually throwing an exception from the handler is bad, and EventBase
  // should probably be changed to fatal the entire program if that happens.
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewayThrow(
    HandlerCallbackOneWay::Ptr callback, int32_t code) {
  LoadError error;
  *error.code() = code;
  callback->exception(error);
}

void AsyncLoadHandler2::async_eb_send(
    HandlerCallbackPtr<void> callback,
    std::unique_ptr<std::string> /* data */) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewaySend(
    HandlerCallbackOneWay::Ptr callback,
    std::unique_ptr<std::string> /* data */) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_recv(
    HandlerCallbackPtr<std::unique_ptr<std::string>> callback, int64_t bytes) {
  std::unique_ptr<std::string> ret(new std::string(bytes, 'a'));
  callback->result(std::move(ret));
}

void AsyncLoadHandler2::async_eb_sendrecv(
    HandlerCallbackPtr<std::unique_ptr<std::string>> callback,
    std::unique_ptr<std::string> /* data */,
    int64_t recvBytes) {
  std::unique_ptr<std::string> ret(new std::string(recvBytes, 'a'));
  callback->result(std::move(ret));
}

void AsyncLoadHandler2::sync_echo(
    // Slightly different from thrift1, this happens in a
    // thread pool.
    std::string& output,
    std::unique_ptr<std::string> data) {
  output = std::move(*data);
}

folly::Future<std::unique_ptr<std::string>> AsyncLoadHandler2::future_echo(
    std::unique_ptr<std::string> data) {
  folly::Promise<std::unique_ptr<std::string>> promise;
  auto future = promise.getFuture();

  folly::via(folly::RequestEventBase::get())
      .thenValue([this, promise = std::move(promise), data = std::move(data)](
                     auto&&) mutable {
        std::string output;
        sync_echo(output, std::move(data));
        promise.setValue(std::make_unique<std::string>(std::move(output)));
      });

  return future;
}

void AsyncLoadHandler2::async_eb_largeContainer(
    HandlerCallbackPtr<void> callback,
    std::unique_ptr<std::vector<BigStruct>>) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_iterAllFields(
    HandlerCallbackPtr<std::unique_ptr<std::vector<BigStruct>>> callback,
    std::unique_ptr<std::vector<BigStruct>> items) {
  std::string x;
  for (auto& item : *items) {
    x = *item.stringField();
    for (auto& i : *item.stringList()) {
      x = i;
    }
  }
  callback->result(std::move(items));
}

void AsyncLoadHandler2::async_eb_add(
    HandlerCallbackPtr<int64_t> callback, int64_t a, int64_t b) {
  callback->result(a + b);
}

} // namespace apache::thrift
