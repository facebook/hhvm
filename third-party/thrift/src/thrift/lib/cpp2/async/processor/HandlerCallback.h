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

#include <thrift/lib/cpp2/async/BiDiStream.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamFactory.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>

#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>

namespace apache::thrift {

namespace detail {
template <typename T>
struct HandlerCallbackHelper;

template <typename T>
struct IsUniquePtr {
  using inner_type = T;
  constexpr static bool value = false;
};

template <typename T>
struct IsUniquePtr<std::unique_ptr<T>> {
  using inner_type = T;
  constexpr static bool value = true;
};

template <typename T>
concept ResponseIsUniquePtr = IsUniquePtr<T>::value;

} // namespace detail

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

 protected:
  using cob_ptr = typename Helper::CobPtr;

 public:
  using Ptr = HandlerCallbackPtr<T>;
  using ResultType = std::decay_t<typename Helper::InputType>;
  using DecoratorAfterCallback = typename Helper::DecoratorAfterCallback;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback()
      : cp_(nullptr), decoratorCallback_(DecoratorAfterCallback::noop()) {}

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop());

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop());

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
    decoratorCallback_.invoke(getRequestContext(), r);
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

 private:
  DecoratorAfterCallback decoratorCallback_;
};

template <>
class HandlerCallback<void> : public HandlerCallbackBase {
 protected:
  using HandlerCallbackBase::exnw_ptr;
  using HandlerCallbackBase::MethodNameInfo;
  using cob_ptr = SerializedResponse (*)(ContextStack*);

 public:
  using Ptr = HandlerCallbackPtr<void>;
  using ResultType = void;

  using DecoratorAfterCallback = detail::DecoratorAfterCallbackNoResult;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback()
      : cp_(nullptr), decoratorCallback_(DecoratorAfterCallback::noop()) {}

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop());

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop());

#if FOLLY_HAS_COROUTINES
  static folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(
      Ptr callback) {
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
    decoratorCallback_.invoke(getRequestContext());
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

 private:
  DecoratorAfterCallback decoratorCallback_;
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
struct InteractionInnerResponseHelper {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<InteractionIf, Response>& result) {
    return result.response;
  }
};

template <typename InteractionIf, typename Response>
struct InteractionInnerResponseHelper<
    InteractionIf,
    std::unique_ptr<Response>> {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<InteractionIf, std::unique_ptr<Response>>& result) {
    return *result.response;
  }
};

template <typename InteractionIf>
struct InteractionInnerResponseHelper<InteractionIf, void> {
  using DecoratorArgType = void;
};

template <typename InteractionIf, typename Response, typename StreamItem>
struct InteractionInnerResponseHelper<
    InteractionIf,
    ResponseAndServerStream<Response, StreamItem>> {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<
          InteractionIf,
          ResponseAndServerStream<Response, StreamItem>>& result) {
    return result.response.response;
  }
};

template <typename InteractionIf, typename Response, typename StreamItem>
struct InteractionInnerResponseHelper<
    InteractionIf,
    ResponseAndServerStream<std::unique_ptr<Response>, StreamItem>> {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<
          InteractionIf,
          ResponseAndServerStream<std::unique_ptr<Response>, StreamItem>>&
          result) {
    return *result.response.response;
  }
};

template <typename InteractionIf, typename StreamItem>
struct InteractionInnerResponseHelper<InteractionIf, ServerStream<StreamItem>> {
  using DecoratorArgType = void;
};

template <
    typename InteractionIf,
    typename Response,
    typename SinkElement,
    typename FinalResponse>
struct InteractionInnerResponseHelper<
    InteractionIf,
    ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>> {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<
          InteractionIf,
          ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>>&
          result) {
    return result.response.response;
  }
};

template <
    typename InteractionIf,
    typename Response,
    typename SinkElement,
    typename FinalResponse>
struct InteractionInnerResponseHelper<
    InteractionIf,
    ResponseAndSinkConsumer<
        std::unique_ptr<Response>,
        SinkElement,
        FinalResponse>> {
  using DecoratorArgType = typename detail::DecoratorArgType<Response>::type;

  static constexpr DecoratorArgType extractInnerResponse(
      const TileAndResponse<
          InteractionIf,
          ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>>&
          result) {
    return *result.response.response;
  }
};

template <typename InteractionIf, typename SinkElement, typename FinalResponse>
struct InteractionInnerResponseHelper<
    InteractionIf,
    SinkConsumer<SinkElement, FinalResponse>> {
  using DecoratorArgType = void;
};

template <typename InteractionIf, typename Response>
class HandlerCallback<TileAndResponse<InteractionIf, Response>> final
    : public HandlerCallback<Response> {
  using cob_ptr = typename HandlerCallback<Response>::cob_ptr;
  using exnw_ptr = HandlerCallbackBase::exnw_ptr;
  using MethodNameInfo = HandlerCallbackBase::MethodNameInfo;
  using InnerResponseHelper =
      InteractionInnerResponseHelper<InteractionIf, Response>;

 public:
  using Ptr = HandlerCallbackPtr<TileAndResponse<InteractionIf, Response>>;

  struct DecoratorAfterCallback
      : public detail::DecoratorAfterCallbackWithResult<
            DecoratorAfterCallback,
            const TileAndResponse<InteractionIf, Response>&,
            typename InnerResponseHelper::DecoratorArgType> {
    using ArgType = typename InnerResponseHelper::DecoratorArgType;
    static constexpr ArgType extractDecoratorArg(
        const TileAndResponse<InteractionIf, Response>& result) {
      if constexpr (!std::is_void_v<ArgType>) {
        return InnerResponseHelper::extractInnerResponse(result);
      }
    }
  };

  void result(TileAndResponse<InteractionIf, Response>&& r) {
    decoratorCallback_.invoke(
        HandlerCallback<Response>::getRequestContext(), r);
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

  HandlerCallback() : HandlerCallback<Response>() {}

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop())
      : HandlerCallback<Response>(
            std::move(req),
            std::move(ctx),
            std::move(methodNameInfo),
            std::move(cp),
            std::move(ewp),
            protoSeqId,
            eb,
            tm,
            reqCtx,
            std::move(interaction),
            HandlerCallback<Response>::DecoratorAfterCallback::noop()),
        decoratorCallback_(std::move(decoratorCallback)) {}

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
      TilePtr&& interaction = {},
      DecoratorAfterCallback&& decoratorCallback =
          DecoratorAfterCallback::noop())
      : HandlerCallback<Response>(
            std::move(req),
            std::move(ctx),
            std::move(methodNameInfo),
            std::move(cp),
            std::move(ewp),
            protoSeqId,
            eb,
            std::move(executor),
            reqCtx,
            notifyRequestPile,
            notifyConcurrencyController,
            std::move(requestData),
            std::move(interaction),
            HandlerCallback<Response>::DecoratorAfterCallback::noop()),
        decoratorCallback_(std::move(decoratorCallback)) {}

  ~HandlerCallback() override {
    if (this->interaction_) {
      this->breakTilePromise();
    }
  }

 private:
  DecoratorAfterCallback decoratorCallback_;
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
    TilePtr&& interaction,
    DecoratorAfterCallback&& decoratorCallback)
    : HandlerCallbackBase(
          std::move(req),
          std::move(ctx),
          std::move(methodNameInfo),
          ewp,
          eb,
          tm,
          reqCtx,
          std::move(interaction)),
      cp_(cp),
      decoratorCallback_(std::move(decoratorCallback)) {
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
    TilePtr&& interaction,
    DecoratorAfterCallback&& decoratorCallback)
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
      cp_(cp),
      decoratorCallback_(std::move(decoratorCallback)) {
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

  using DecoratorArgType = typename detail::DecoratorArgType<InnerType>::type;
  struct DecoratorAfterCallback : public DecoratorAfterCallbackWithResult<
                                      DecoratorAfterCallback,
                                      DecoratorArgType,
                                      DecoratorArgType> {
    static constexpr DecoratorArgType extractDecoratorArg(
        DecoratorArgType result) {
      return result;
    }
  };
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
          ResponseAndServerStream<Response, StreamItem>> {
  struct DecoratorAfterCallback
      : public DecoratorAfterCallbackWithResult<
            DecoratorAfterCallback,
            const ResponseAndServerStream<Response, StreamItem>&,
            typename DecoratorArgType<
                typename inner_type<Response>::type>::type> {
    using ArgType =
        typename DecoratorArgType<typename inner_type<Response>::type>::type;

    template <typename InnerResponseType>
    static constexpr ArgType extractDecoratorArg(
        const ResponseAndServerStream<InnerResponseType, StreamItem>& result) {
      return result.response;
    }

    template <ResponseIsUniquePtr InnerResponseType>
    static constexpr ArgType extractDecoratorArg(
        const ResponseAndServerStream<InnerResponseType, StreamItem>& result) {
      return *result.response;
    }
  };
};

template <typename StreamItem>
struct HandlerCallbackHelper<ServerStream<StreamItem>>
    : public HandlerCallbackHelperServerStream<ServerStream<StreamItem>> {
  struct DecoratorAfterCallback : public DecoratorAfterCallbackWithResult<
                                      DecoratorAfterCallback,
                                      const ServerStream<StreamItem>&,
                                      void> {};
};

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
          ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>> {
  struct DecoratorAfterCallback
      : public DecoratorAfterCallbackWithResult<
            DecoratorAfterCallback,
            const ResponseAndSinkConsumer<
                Response,
                SinkElement,
                FinalResponse>&,
            typename DecoratorArgType<
                typename inner_type<Response>::type>::type> {
    using ArgType =
        typename DecoratorArgType<typename inner_type<Response>::type>::type;

    template <typename InnerResponseType>
    static constexpr ArgType extractDecoratorArg(const ResponseAndSinkConsumer<
                                                 InnerResponseType,
                                                 SinkElement,
                                                 FinalResponse>& result) {
      return result.response;
    }

    template <ResponseIsUniquePtr InnerResponseType>
    static constexpr ArgType extractDecoratorArg(const ResponseAndSinkConsumer<
                                                 InnerResponseType,
                                                 SinkElement,
                                                 FinalResponse>& result) {
      return *result.response;
    }
  };
};

template <typename SinkElement, typename FinalResponse>
struct HandlerCallbackHelper<SinkConsumer<SinkElement, FinalResponse>>
    : public HandlerCallbackHelperSink<
          SinkConsumer<SinkElement, FinalResponse>> {
  struct DecoratorAfterCallback
      : public DecoratorAfterCallbackWithResult<
            DecoratorAfterCallback,
            const SinkConsumer<SinkElement, FinalResponse>&,
            void> {};
};

template <typename In, typename Out>
struct HandlerCallbackHelper<StreamTransformation<In, Out>> {
  using InnerType = StreamTransformation<In, Out>&&;
  using InputType = StreamTransformation<In, Out>&&;
  using CobPtr = ResponseAndServerBiDiStreamFactory (*)(
      ContextStack*, folly::Executor::KeepAlive<>, InputType);
  static ResponseAndServerBiDiStreamFactory call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, ex, std::move(input));
  }

  struct DecoratorAfterCallback : public DecoratorAfterCallbackWithResult<
                                      DecoratorAfterCallback,
                                      const StreamTransformation<In, Out>&,
                                      void> {};
};

template <typename Response, typename In, typename Out>
struct HandlerCallbackHelper<
    ResponseAndStreamTransformation<Response, In, Out>> {
  using InnerType = ResponseAndStreamTransformation<Response, In, Out>&&;
  using InputType = ResponseAndStreamTransformation<Response, In, Out>&&;
  using CobPtr = ResponseAndServerBiDiStreamFactory (*)(
      ContextStack*, folly::Executor::KeepAlive<>, InputType);
  static ResponseAndServerBiDiStreamFactory call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, ex, std::move(input));
  }

  struct DecoratorAfterCallback
      : public DecoratorAfterCallbackWithResult<
            DecoratorAfterCallback,
            const ResponseAndStreamTransformation<Response, In, Out>&,
            typename DecoratorArgType<
                typename inner_type<Response>::type>::type> {
    using ArgType =
        typename DecoratorArgType<typename inner_type<Response>::type>::type;

    template <typename InnerResponseType>
    static constexpr ArgType extractDecoratorArg(
        const ResponseAndStreamTransformation<InnerResponseType, In, Out>&
            result) {
      return result.response;
    }

    template <ResponseIsUniquePtr InnerResponseType>
    static constexpr ArgType extractDecoratorArg(
        const ResponseAndStreamTransformation<InnerResponseType, In, Out>&
            result) {
      return *result.response;
    }
  };
};

} // namespace detail

} // namespace apache::thrift
