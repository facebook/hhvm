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
#include <thrift/lib/cpp/concurrency/Util.h>

using namespace apache::thrift::test;

using apache::thrift::concurrency::Util;

namespace apache {
namespace thrift {

void AsyncLoadHandler2::async_eb_noop(HandlerCallback<void>::Ptr callback) {
  // Note that we could have done this with a sync function,
  // but an inline async op is faster, and we want to maintain
  // parity with the old loadgen for comparison testing
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewayNoop(
    HandlerCallbackBase::Ptr /* callback */) {}

void AsyncLoadHandler2::async_eb_asyncNoop(
    HandlerCallback<void>::Ptr callback) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_sleep(
    HandlerCallback<void>::Ptr callback, int64_t microseconds) {
  callback->getEventBase()->runInEventBaseThread([=]() {
    callback->getEventBase()->tryRunAfterDelay(
        [=]() { callback->done(); }, microseconds / Util::US_PER_MS);
  });
}

void AsyncLoadHandler2::async_eb_onewaySleep(
    HandlerCallbackBase::Ptr callback, int64_t microseconds) {
  auto eb = callback->getEventBase();
  eb->runInEventBaseThread([=]() {
    eb->tryRunAfterDelay([callback]() {}, microseconds / Util::US_PER_MS);
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
    HandlerCallback<void>::Ptr callback, int64_t microseconds) {
  usleep(microseconds);
  callback->done();
}

void AsyncLoadHandler2::async_eb_badBurn(
    HandlerCallback<void>::Ptr callback, int64_t microseconds) {
  // This is a true (bad) async call.
  sync_burn(microseconds);
  callback->done();
}

void AsyncLoadHandler2::async_eb_throwError(
    HandlerCallback<void>::Ptr callback, int32_t code) {
  LoadError error;
  *error.code_ref() = code;
  callback->exception(error);
}

void AsyncLoadHandler2::async_eb_throwUnexpected(
    HandlerCallback<void>::Ptr callback, int32_t /* code */) {
  // FIXME: it isn't possible to implement this behavior with the async code
  //
  // Actually throwing an exception from the handler is bad, and EventBase
  // should probably be changed to fatal the entire program if that happens.
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewayThrow(
    HandlerCallbackBase::Ptr callback, int32_t code) {
  LoadError error;
  *error.code_ref() = code;
  callback->exception(error);
}

void AsyncLoadHandler2::async_eb_send(
    HandlerCallback<void>::Ptr callback,
    std::unique_ptr<std::string> /* data */) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_onewaySend(
    HandlerCallbackBase::Ptr /* callback */,
    std::unique_ptr<std::string> /* data */) {}

void AsyncLoadHandler2::async_eb_recv(
    HandlerCallback<std::unique_ptr<std::string>>::Ptr callback,
    int64_t bytes) {
  std::unique_ptr<std::string> ret(new std::string(bytes, 'a'));
  callback->result(std::move(ret));
}

void AsyncLoadHandler2::async_eb_sendrecv(
    HandlerCallback<std::unique_ptr<std::string>>::Ptr callback,
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
    HandlerCallback<void>::Ptr callback,
    std::unique_ptr<std::vector<BigStruct>>) {
  callback->done();
}

void AsyncLoadHandler2::async_eb_iterAllFields(
    HandlerCallback<std::unique_ptr<std::vector<BigStruct>>>::Ptr callback,
    std::unique_ptr<std::vector<BigStruct>> items) {
  std::string x;
  for (auto& item : *items) {
    x = *item.stringField_ref();
    for (auto& i : *item.stringList_ref()) {
      x = i;
    }
  }
  callback->result(std::move(items));
}

void AsyncLoadHandler2::async_eb_add(
    HandlerCallback<int64_t>::Ptr callback, int64_t a, int64_t b) {
  callback->result(a + b);
}

} // namespace thrift
} // namespace apache
