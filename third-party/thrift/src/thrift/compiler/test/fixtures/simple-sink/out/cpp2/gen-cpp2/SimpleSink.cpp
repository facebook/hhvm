/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/simple-sink/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */

#include "thrift/compiler/test/fixtures/simple-sink/gen-cpp2/SimpleSink.h"
#include "thrift/compiler/test/fixtures/simple-sink/gen-cpp2/SimpleSink.tcc"
#include "thrift/compiler/test/fixtures/simple-sink/gen-cpp2/module_metadata.h"
#include <thrift/lib/cpp2/gen/service_cpp.h>

std::unique_ptr<apache::thrift::AsyncProcessor> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::getProcessor() {
  return std::make_unique<::cpp2::SimpleSinkAsyncProcessor>(this);
}

apache::thrift::ServiceHandler<::cpp2::SimpleSink>::CreateMethodMetadataResult apache::thrift::ServiceHandler<::cpp2::SimpleSink>::createMethodMetadata() {
  return ::apache::thrift::detail::ap::createMethodMetadataMap<::cpp2::SimpleSinkAsyncProcessor>(getServiceRequestInfoMap().value().get());
}


std::optional<std::reference_wrapper<apache::thrift::ServiceRequestInfoMap const>> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::getServiceRequestInfoMap() const {
  return __fbthrift_serviceInfoHolder.requestInfoMap();
}

::cpp2::SimpleSinkServiceInfoHolder apache::thrift::ServiceHandler<::cpp2::SimpleSink>::__fbthrift_serviceInfoHolder;


/* TODO (@sazonovk) */ apache::thrift::ServiceHandler<::cpp2::SimpleSink>::simple() {
  apache::thrift::detail::si::throw_app_exn_unimplemented("simple");
}

/* TODO (@sazonovk) */ apache::thrift::ServiceHandler<::cpp2::SimpleSink>::sync_simple() {
  return simple();
}

folly::SemiFuture</* TODO (@sazonovk) */> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::semifuture_simple() {
  auto expected{apache::thrift::detail::si::InvocationType::SemiFuture};
  __fbthrift_invocation_simple.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Sync, std::memory_order_relaxed);
  return sync_simple();
}

folly::Future</* TODO (@sazonovk) */> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::future_simple() {
  auto expected{apache::thrift::detail::si::InvocationType::Future};
  __fbthrift_invocation_simple.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::SemiFuture, std::memory_order_relaxed);
  return apache::thrift::detail::si::future(semifuture_simple(), getInternalKeepAlive());
}

#if FOLLY_HAS_COROUTINES
folly::coro::Task</* TODO (@sazonovk) */> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::co_simple() {
  auto expected{apache::thrift::detail::si::InvocationType::Coro};
  __fbthrift_invocation_simple.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Future, std::memory_order_relaxed);
  folly::throw_exception(apache::thrift::detail::si::UnimplementedCoroMethod::withCapturedArgs<>());
}

folly::coro::Task</* TODO (@sazonovk) */> apache::thrift::ServiceHandler<::cpp2::SimpleSink>::co_simple(apache::thrift::RequestParams /* params */) {
  auto expected{apache::thrift::detail::si::InvocationType::CoroParam};
  __fbthrift_invocation_simple.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Coro, std::memory_order_relaxed);
  return co_simple();
}
#endif // FOLLY_HAS_COROUTINES

void apache::thrift::ServiceHandler<::cpp2::SimpleSink>::async_tm_simple(apache::thrift::HandlerCallbackPtr</* TODO (@sazonovk) */> callback) {
  // It's possible the coroutine versions will delegate to a future-based
  // version. If that happens, we need the RequestParams arguments to be
  // available to the future through the thread-local backchannel, so we create
  // a RAII object that sets up RequestParams and clears them on destruction.
  apache::thrift::detail::si::AsyncTmPrep asyncTmPrep(this, callback.get());
#if FOLLY_HAS_COROUTINES
determineInvocationType:
#endif // FOLLY_HAS_COROUTINES
  auto invocationType = __fbthrift_invocation_simple.load(std::memory_order_relaxed);
  try {
    switch (invocationType) {
      case apache::thrift::detail::si::InvocationType::AsyncTm:
      {
#if FOLLY_HAS_COROUTINES
        __fbthrift_invocation_simple.compare_exchange_strong(invocationType, apache::thrift::detail::si::InvocationType::CoroParam, std::memory_order_relaxed);
        apache::thrift::RequestParams params{callback->getRequestContext(),
          callback->getThreadManager_deprecated(), callback->getEventBase(), callback->getHandlerExecutor()};
        auto task = co_simple(params);
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
#else // FOLLY_HAS_COROUTINES
        __fbthrift_invocation_simple.compare_exchange_strong(invocationType, apache::thrift::detail::si::InvocationType::Future, std::memory_order_relaxed);
        [[fallthrough]];
#endif // FOLLY_HAS_COROUTINES
      }
      case apache::thrift::detail::si::InvocationType::Future:
      {
        auto fut = future_simple();
        apache::thrift::detail::si::async_tm_future(std::move(callback), std::move(fut));
        return;
      }
      case apache::thrift::detail::si::InvocationType::SemiFuture:
      {
        auto fut = semifuture_simple();
        apache::thrift::detail::si::async_tm_semifuture(std::move(callback), std::move(fut));
        return;
      }
#if FOLLY_HAS_COROUTINES
      case apache::thrift::detail::si::InvocationType::CoroParam:
      {
        apache::thrift::RequestParams params{callback->getRequestContext(),
          callback->getThreadManager_deprecated(), callback->getEventBase(), callback->getHandlerExecutor()};
        auto task = co_simple(params);
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
      }
      case apache::thrift::detail::si::InvocationType::Coro:
      {
        auto task = co_simple();
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
      }
#endif // FOLLY_HAS_COROUTINES
      case apache::thrift::detail::si::InvocationType::Sync:
      {
        callback->result(sync_simple());
        return;
      }
      default:
      {
        folly::assume_unreachable();
      }
    }
#if FOLLY_HAS_COROUTINES
  } catch (apache::thrift::detail::si::UnimplementedCoroMethod& ex) {
    std::tie() = std::move(ex).restoreArgs<>();
    goto determineInvocationType;
#endif // FOLLY_HAS_COROUTINES
  } catch (...) {
    callback->exception(std::current_exception());
  }
}


namespace cpp2 {

/* TODO (@sazonovk) */ SimpleSinkSvNull::simple() { 
  return {};
}


std::string_view SimpleSinkAsyncProcessor::getServiceName() {
  return "SimpleSink";
}

void SimpleSinkAsyncProcessor::getServiceMetadata(apache::thrift::metadata::ThriftServiceMetadataResponse& response) {
  ::apache::thrift::detail::md::ServiceMetadata<::apache::thrift::ServiceHandler<::cpp2::SimpleSink>>::gen(response);
}

void SimpleSinkAsyncProcessor::processSerializedCompressedRequestWithMetadata(apache::thrift::ResponseChannelRequest::UniquePtr req, apache::thrift::SerializedCompressedRequest&& serializedRequest, const apache::thrift::AsyncProcessorFactory::MethodMetadata& methodMetadata, apache::thrift::protocol::PROTOCOL_TYPES protType, apache::thrift::Cpp2RequestContext* context, folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  apache::thrift::detail::ap::process(this, iface_, std::move(req), std::move(serializedRequest), methodMetadata, protType, context, eb, tm);
}

void SimpleSinkAsyncProcessor::executeRequest(apache::thrift::ServerRequest&& request, const apache::thrift::AsyncProcessorFactory::MethodMetadata& methodMetadata) {
  apache::thrift::detail::ap::execute(this, std::move(request), apache::thrift::detail::ServerRequestHelper::protocol(request), methodMetadata);
}

const SimpleSinkAsyncProcessor::ProcessMap& SimpleSinkAsyncProcessor::getOwnProcessMap() {
  return kOwnProcessMap_;
}

const SimpleSinkAsyncProcessor::ProcessMap SimpleSinkAsyncProcessor::kOwnProcessMap_ {
  {"simple",
    {&SimpleSinkAsyncProcessor::setUpAndProcess_simple<apache::thrift::CompactProtocolReader, apache::thrift::CompactProtocolWriter>,
     &SimpleSinkAsyncProcessor::setUpAndProcess_simple<apache::thrift::BinaryProtocolReader, apache::thrift::BinaryProtocolWriter>,
     &SimpleSinkAsyncProcessor::executeRequest_simple<apache::thrift::CompactProtocolReader, apache::thrift::CompactProtocolWriter>,
     &SimpleSinkAsyncProcessor::executeRequest_simple<apache::thrift::BinaryProtocolReader, apache::thrift::BinaryProtocolWriter>}},
};

apache::thrift::ServiceRequestInfoMap const& SimpleSinkServiceInfoHolder::requestInfoMap() const {
  static folly::Indestructible<apache::thrift::ServiceRequestInfoMap> requestInfoMap{staticRequestInfoMap()};
  return *requestInfoMap;
}

apache::thrift::ServiceRequestInfoMap SimpleSinkServiceInfoHolder::staticRequestInfoMap() {
  apache::thrift::ServiceRequestInfoMap requestInfoMap = {
  {"simple",
    { false,
     apache::thrift::RpcKind::SINK,
     "SimpleSink.simple",
     std::nullopt,
     apache::thrift::concurrency::NORMAL,
     std::nullopt}},
  };

  return requestInfoMap;
}
} // namespace cpp2

namespace apache::thrift::detail {
::folly::Range<const ::std::string_view*>(*TSchemaAssociation<::cpp2::SimpleSink, false>::bundle)() =
    nullptr;
}
