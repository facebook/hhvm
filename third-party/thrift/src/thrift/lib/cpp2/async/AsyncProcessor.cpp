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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

#include <fmt/core.h>
#include <fmt/format.h>

#include <folly/io/async/EventBaseAtomicNotificationQueue.h>

#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/server/IResourcePoolAcceptor.h>

namespace apache::thrift {

HandlerCallback<void>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    MethodNameInfo methodNameInfo,
    cob_ptr cp,
    exnw_ptr ewp,
    int32_t protoSeqId,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm,
    Cpp2RequestContext* reqCtx,
    TilePtr&& interaction)
    : HandlerCallbackBase(
          std::move(req),
          std::move(ctx),
          std::move(methodNameInfo),
          ewp,
          eb,
          tm,
          reqCtx,
          std::move(interaction)),
      cp_(cp) {
  this->protoSeqId_ = protoSeqId;
}

HandlerCallback<void>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    MethodNameInfo methodNameInfo,
    cob_ptr cp,
    exnw_ptr ewp,
    int32_t protoSeqId,
    folly::EventBase* eb,
    folly::Executor::KeepAlive<> executor,
    Cpp2RequestContext* reqCtx,
    RequestCompletionCallback* notifyRequestPile,
    RequestCompletionCallback* notifyConcurrencyController,
    ServerRequestData requestData,
    TilePtr&& interaction)
    : HandlerCallbackBase(
          std::move(req),
          std::move(ctx),
          std::move(methodNameInfo),
          ewp,
          eb,
          std::move(executor),
          reqCtx,
          notifyRequestPile,
          notifyConcurrencyController,
          std::move(requestData),
          std::move(interaction)),
      cp_(cp) {
  this->protoSeqId_ = protoSeqId;
}

void HandlerCallback<void>::complete(folly::Try<folly::Unit>&& r) {
  maybeNotifyComplete();
  if (r.hasException()) {
    exception(std::move(r.exception()));
  } else {
    done();
  }
}

void HandlerCallback<void>::doDone() {
  assert(cp_ != nullptr);
  auto queue = cp_(this->ctx_.get());
  this->ctx_.reset();
  sendReply(std::move(queue));
}

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
