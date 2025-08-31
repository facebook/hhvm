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

#pragma once

#include <folly/Unit.h>
#include <folly/coro/Task.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>

namespace apache::thrift {

class HandlerCallbackOneWay : public HandlerCallbackBase {
 public:
  using Ptr =
      util::IntrusiveSharedPtr<HandlerCallbackOneWay, IntrusiveSharedPtrAccess>;
  using HandlerCallbackBase::HandlerCallbackBase;

 private:
#if FOLLY_HAS_COROUTINES
  static folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(
      Ptr callback);
#endif // FOLLY_HAS_COROUTINES

  Ptr sharedFromThis() noexcept {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  void done() noexcept;
  void complete(folly::Try<folly::Unit>&& r) noexcept;

  class CompletionGuard {
   public:
    explicit CompletionGuard(Ptr&& callback) noexcept
        : callback_(std::move(callback)) {}
    CompletionGuard(CompletionGuard&& other) noexcept
        : callback_(other.release()) {}
    CompletionGuard& operator=(CompletionGuard&& other) noexcept {
      callback_ = other.release();
      return *this;
    }

    ~CompletionGuard() noexcept {
      if (callback_ == nullptr) {
        return;
      }
      if (auto ex = folly::current_exception()) {
        callback_->exception(std::move(ex));
      } else {
        callback_->done();
      }
    }

    Ptr release() noexcept { return std::exchange(callback_, nullptr); }

   private:
    Ptr callback_;
  };
};

} // namespace apache::thrift
