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

#include <thrift/lib/cpp2/async/processor/HandlerCallbackOneWay.h>

namespace apache::thrift {
void HandlerCallbackOneWay::done() noexcept {
#if FOLLY_HAS_COROUTINES
  if (!shouldProcessServiceInterceptorsOnResponse()) {
    return;
  }
  startOnExecutor(doInvokeServiceInterceptorsOnResponse(sharedFromThis()));
#endif // FOLLY_HAS_COROUTINES
}

void HandlerCallbackOneWay::complete(folly::Try<folly::Unit>&& r) noexcept {
  if (r.hasException()) {
    exception(std::move(r).exception());
  } else {
    done();
  }
}

#if FOLLY_HAS_COROUTINES
/* static */ folly::coro::Task<void>
HandlerCallbackOneWay::doInvokeServiceInterceptorsOnResponse(Ptr callback) {
  folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
      callback->processServiceInterceptorsOnResponse(
          apache::thrift::util::TypeErasedRef::of<folly::Unit>(folly::unit)));
  if (onResponseResult.hasException()) {
    callback->doException(onResponseResult.exception().to_exception_ptr());
  }
}
#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift
