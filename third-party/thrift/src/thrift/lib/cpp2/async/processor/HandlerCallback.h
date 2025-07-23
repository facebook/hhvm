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

#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/async/processor/RequestTask.h>
#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>

namespace apache::thrift {

namespace detail {
template <typename T>
struct HandlerCallbackHelper;
}

template <class T>
class HandlerCallback;

template <class T>
using HandlerCallbackPtr = util::IntrusiveSharedPtr<
    HandlerCallback<T>,
    HandlerCallbackBase::IntrusiveSharedPtrAccess>;

template <typename T>
class HandlerCallback : public HandlerCallbackBase {
  using Helper = apache::thrift::detail::HandlerCallbackHelper<T>;
  using InnerType = typename Helper::InnerType;
  using InputType = typename Helper::InputType;
  using cob_ptr = typename Helper::CobPtr;

 public:
  using Ptr = HandlerCallbackPtr<T>;
  using ResultType = std::decay_t<typename Helper::InputType>;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {});

  HandlerCallback(
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
      TilePtr&& interaction = {});

#if FOLLY_HAS_COROUTINES
  static folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(
      Ptr callback, std::decay_t<InputType> result) {
    folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
        callback->processServiceInterceptorsOnResponse(
            apache::thrift::util::TypeErasedRef::of<InnerType>(result)));
    if (onResponseResult.hasException()) {
      callback->doException(onResponseResult.exception().to_exception_ptr());
    } else {
      callback->doResult(std::move(result));
    }
  }
#endif

  void result(InnerType r) {
#if FOLLY_HAS_COROUTINES
    if (!shouldProcessServiceInterceptorsOnResponse()) {
      // Some service code (especially unit tests) assume that doResult() is
      // called synchronously within a result() call. This check exists simply
      // for backwards compatibility with those services. As an added bonus, we
      // get to avoid allocating a coroutine frame + Future core in the case
      // where they will be unused.
      doResult(std::forward<InputType>(r));
    } else {
      startOnExecutor(doInvokeServiceInterceptorsOnResponse(
          sharedFromThis(), std::decay_t<InputType>(std::move(r))));
    }
#else
    doResult(std::forward<InputType>(r));
#endif // FOLLY_HAS_COROUTINES
  }
  [[deprecated("Pass the inner value directly to result()")]] void result(
      std::unique_ptr<ResultType> r);

  void complete(folly::Try<T>&& r);

 protected:
  virtual void doResult(InputType r);

  cob_ptr cp_;
};

template <>
class HandlerCallback<void> : public HandlerCallbackBase {
  using cob_ptr = SerializedResponse (*)(ContextStack*);

 public:
  using Ptr = HandlerCallbackPtr<void>;
  using ResultType = void;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {});

  HandlerCallback(
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
      TilePtr&& interaction = {});

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(Ptr callback) {
    folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
        callback->processServiceInterceptorsOnResponse(
            apache::thrift::util::TypeErasedRef::of<folly::Unit>(folly::unit)));
    if (onResponseResult.hasException()) {
      callback->doException(onResponseResult.exception().to_exception_ptr());
    } else {
      callback->doDone();
    }
  }
#endif // FOLLY_HAS_COROUTINES

  void done() {
#if FOLLY_HAS_COROUTINES
    if (!shouldProcessServiceInterceptorsOnResponse()) {
      // Some service code (especially unit tests) assume that doResult() is
      // called synchronously within a result() call. This check exists simply
      // for backwards compatibility with those services. As an added bonus, we
      // get to avoid allocating a coroutine frame + Future core in the case
      // where they will be unused.
      doDone();
    } else {
      startOnExecutor(doInvokeServiceInterceptorsOnResponse(sharedFromThis()));
    }
#else
    doDone();
#endif // FOLLY_HAS_COROUTINES
  }

  void complete(folly::Try<folly::Unit>&& r);

 protected:
  virtual void doDone();

  cob_ptr cp_;
};

template <typename InteractionIf, typename Response>
struct TileAndResponse {
  std::unique_ptr<InteractionIf> tile;
  Response response;
};
template <typename InteractionIf>
struct TileAndResponse<InteractionIf, void> {
  std::unique_ptr<InteractionIf> tile;
};

template <typename InteractionIf, typename Response>
class HandlerCallback<TileAndResponse<InteractionIf, Response>> final
    : public HandlerCallback<Response> {
 public:
  using Ptr = HandlerCallbackPtr<TileAndResponse<InteractionIf, Response>>;

  void result(TileAndResponse<InteractionIf, Response>&& r) {
    if (this->fulfillTilePromise(std::move(r.tile))) {
      if constexpr (!std::is_void_v<Response>) {
        HandlerCallback<Response>::result(std::move(r.response));
      } else {
        this->done();
      }
    }
  }
  void complete(folly::Try<TileAndResponse<InteractionIf, Response>>&& r) {
    if (r.hasException()) {
      this->exception(std::move(r.exception()));
    } else {
      this->result(std::move(r.value()));
    }
  }

  using HandlerCallback<Response>::HandlerCallback;

  ~HandlerCallback() override {
    if (this->interaction_) {
      this->breakTilePromise();
    }
  }
};

////
// Implementation details
////

template <typename T>
HandlerCallback<T>::HandlerCallback(
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

template <typename T>
HandlerCallback<T>::HandlerCallback(
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

template <typename T>
void HandlerCallback<T>::result(std::unique_ptr<ResultType> r) {
  r ? result(std::move(*r))
    : exception(TApplicationException(
          TApplicationException::MISSING_RESULT,
          "nullptr yielded from handler"));
}

template <typename T>
void HandlerCallback<T>::complete(folly::Try<T>&& r) {
  maybeNotifyComplete();
  if (r.hasException()) {
    exception(std::move(r.exception()));
  } else {
    result(std::move(r.value()));
  }
}

template <typename T>
void HandlerCallback<T>::doResult(InputType r) {
  assert(cp_ != nullptr);
  auto reply = Helper::call(
      cp_,
      this->ctx_.get(),
      executor_ ? executor_.get() : eb_,
      std::forward<InputType>(r));
  sendReply(std::move(reply));
}

namespace detail {

// template that typedefs type to its argument, unless the argument is a
// unique_ptr<S>, in which case it typedefs type to S.
template <class S>
struct inner_type {
  using type = S;
};
template <class S>
struct inner_type<std::unique_ptr<S>> {
  using type = S;
};

template <typename T>
struct HandlerCallbackHelper {
  using InnerType = typename apache::thrift::detail::inner_type<T>::type;
  using InputType = const InnerType&;
  using CobPtr =
      apache::thrift::SerializedResponse (*)(ContextStack*, InputType);
  static apache::thrift::SerializedResponse call(
      CobPtr cob, ContextStack* ctx, folly::Executor*, InputType input) {
    return cob(ctx, input);
  }
};

template <typename StreamInputType>
struct HandlerCallbackHelperServerStream {
  using InnerType = StreamInputType&&;
  using InputType = StreamInputType&&;
  using CobPtr = ResponseAndServerStreamFactory (*)(
      ContextStack*, folly::Executor::KeepAlive<>, InputType);
  static ResponseAndServerStreamFactory call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, ex, std::move(input));
  }
};

template <typename Response, typename StreamItem>
struct HandlerCallbackHelper<ResponseAndServerStream<Response, StreamItem>>
    : public HandlerCallbackHelperServerStream<
          ResponseAndServerStream<Response, StreamItem>> {};

template <typename StreamItem>
struct HandlerCallbackHelper<ServerStream<StreamItem>>
    : public HandlerCallbackHelperServerStream<ServerStream<StreamItem>> {};

template <typename SinkInputType>
struct HandlerCallbackHelperSink {
  using InnerType = SinkInputType&&;
  using InputType = SinkInputType&&;
  using CobPtr =
      std::pair<apache::thrift::SerializedResponse, SinkConsumerImpl> (*)(
          ContextStack*, InputType, folly::Executor::KeepAlive<>);
  static std::pair<apache::thrift::SerializedResponse, SinkConsumerImpl> call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, std::move(input), ex);
  }
};

template <typename Response, typename SinkElement, typename FinalResponse>
struct HandlerCallbackHelper<
    ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>>
    : public HandlerCallbackHelperSink<
          ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>> {};

template <typename SinkElement, typename FinalResponse>
struct HandlerCallbackHelper<SinkConsumer<SinkElement, FinalResponse>>
    : public HandlerCallbackHelperSink<
          SinkConsumer<SinkElement, FinalResponse>> {};

} // namespace detail

template <typename ChildType>
void RequestTask<ChildType>::run() {
  // Since this request was queued, reset the processBegin
  // time to the actual start time, and not the queue time.
  req_.requestContext()->getTimestamps().processBegin =
      std::chrono::steady_clock::now();
  if (!oneway_ && !req_.request()->getShouldStartProcessing()) {
    apache::thrift::HandlerCallbackBase::releaseRequest(
        apache::thrift::detail::ServerRequestHelper::request(std::move(req_)),
        apache::thrift::detail::ServerRequestHelper::eventBase(req_));
    return;
  }
  (childClass_->*executeFunc_)(std::move(req_));
}

} // namespace apache::thrift
