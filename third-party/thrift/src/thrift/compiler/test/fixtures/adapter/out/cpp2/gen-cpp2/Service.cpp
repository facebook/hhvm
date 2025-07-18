/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/adapter/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */

#include "thrift/compiler/test/fixtures/adapter/gen-cpp2/Service.h"
#include "thrift/compiler/test/fixtures/adapter/gen-cpp2/Service.tcc"
#include "thrift/compiler/test/fixtures/adapter/gen-cpp2/module_metadata.h"
#include <thrift/lib/cpp2/gen/service_cpp.h>

std::unique_ptr<apache::thrift::AsyncProcessor> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::getProcessor() {
  return std::make_unique<::facebook::thrift::test::ServiceAsyncProcessor>(this);
}

apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::CreateMethodMetadataResult apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::createMethodMetadata() {
  return ::apache::thrift::detail::ap::createMethodMetadataMap<::facebook::thrift::test::ServiceAsyncProcessor>(getServiceRequestInfoMap().value().get());
}


std::optional<std::reference_wrapper<apache::thrift::ServiceRequestInfoMap const>> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::getServiceRequestInfoMap() const {
  return __fbthrift_serviceInfoHolder.requestInfoMap();
}

::facebook::thrift::test::ServiceServiceInfoHolder apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::__fbthrift_serviceInfoHolder;


::facebook::thrift::test::MyI32_4873 apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> /*arg1*/, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> /*arg2*/, std::unique_ptr<::facebook::thrift::test::Foo> /*arg3*/) {
  apache::thrift::detail::si::throw_app_exn_unimplemented("func");
}

::facebook::thrift::test::MyI32_4873 apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::sync_func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  return func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
}

folly::SemiFuture<::facebook::thrift::test::MyI32_4873> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::semifuture_func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  auto expected{apache::thrift::detail::si::InvocationType::SemiFuture};
  __fbthrift_invocation_func.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Sync, std::memory_order_relaxed);
  return sync_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
}

folly::Future<::facebook::thrift::test::MyI32_4873> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::future_func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  auto expected{apache::thrift::detail::si::InvocationType::Future};
  __fbthrift_invocation_func.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::SemiFuture, std::memory_order_relaxed);
  return apache::thrift::detail::si::future(semifuture_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3)), getInternalKeepAlive());
}

#if FOLLY_HAS_COROUTINES
folly::coro::Task<::facebook::thrift::test::MyI32_4873> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::co_func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  auto expected{apache::thrift::detail::si::InvocationType::Coro};
  __fbthrift_invocation_func.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Future, std::memory_order_relaxed);
  folly::throw_exception(apache::thrift::detail::si::UnimplementedCoroMethod::withCapturedArgs<std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> /*arg1*/, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> /*arg2*/, std::unique_ptr<::facebook::thrift::test::Foo> /*arg3*/>(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3)));
}

folly::coro::Task<::facebook::thrift::test::MyI32_4873> apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::co_func(apache::thrift::RequestParams /* params */, std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  auto expected{apache::thrift::detail::si::InvocationType::CoroParam};
  __fbthrift_invocation_func.compare_exchange_strong(expected, apache::thrift::detail::si::InvocationType::Coro, std::memory_order_relaxed);
  return co_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
}
#endif // FOLLY_HAS_COROUTINES

void apache::thrift::ServiceHandler<::facebook::thrift::test::Service>::async_tm_func(apache::thrift::HandlerCallbackPtr<::facebook::thrift::test::MyI32_4873> callback, std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> p_arg1, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> p_arg2, std::unique_ptr<::facebook::thrift::test::Foo> p_arg3) {
  // It's possible the coroutine versions will delegate to a future-based
  // version. If that happens, we need the RequestParams arguments to be
  // available to the future through the thread-local backchannel, so we create
  // a RAII object that sets up RequestParams and clears them on destruction.
  apache::thrift::detail::si::AsyncTmPrep asyncTmPrep(this, callback.get());
#if FOLLY_HAS_COROUTINES
determineInvocationType:
#endif // FOLLY_HAS_COROUTINES
  auto invocationType = __fbthrift_invocation_func.load(std::memory_order_relaxed);
  try {
    switch (invocationType) {
      case apache::thrift::detail::si::InvocationType::AsyncTm:
      {
#if FOLLY_HAS_COROUTINES
        __fbthrift_invocation_func.compare_exchange_strong(invocationType, apache::thrift::detail::si::InvocationType::CoroParam, std::memory_order_relaxed);
        apache::thrift::RequestParams params{callback->getRequestContext(),
          callback->getThreadManager_deprecated(), callback->getEventBase(), callback->getHandlerExecutor()};
        auto task = co_func(params, std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
#else // FOLLY_HAS_COROUTINES
        __fbthrift_invocation_func.compare_exchange_strong(invocationType, apache::thrift::detail::si::InvocationType::Future, std::memory_order_relaxed);
        [[fallthrough]];
#endif // FOLLY_HAS_COROUTINES
      }
      case apache::thrift::detail::si::InvocationType::Future:
      {
        auto fut = future_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
        apache::thrift::detail::si::async_tm_future(std::move(callback), std::move(fut));
        return;
      }
      case apache::thrift::detail::si::InvocationType::SemiFuture:
      {
        auto fut = semifuture_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
        apache::thrift::detail::si::async_tm_semifuture(std::move(callback), std::move(fut));
        return;
      }
#if FOLLY_HAS_COROUTINES
      case apache::thrift::detail::si::InvocationType::CoroParam:
      {
        apache::thrift::RequestParams params{callback->getRequestContext(),
          callback->getThreadManager_deprecated(), callback->getEventBase(), callback->getHandlerExecutor()};
        auto task = co_func(params, std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
      }
      case apache::thrift::detail::si::InvocationType::Coro:
      {
        auto task = co_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3));
        apache::thrift::detail::si::async_tm_coro(std::move(callback), std::move(task));
        return;
      }
#endif // FOLLY_HAS_COROUTINES
      case apache::thrift::detail::si::InvocationType::Sync:
      {
        callback->result(sync_func(std::move(p_arg1), std::move(p_arg2), std::move(p_arg3)));
        return;
      }
      default:
      {
        folly::assume_unreachable();
      }
    }
#if FOLLY_HAS_COROUTINES
  } catch (apache::thrift::detail::si::UnimplementedCoroMethod& ex) {
    std::tie(p_arg1, p_arg2, p_arg3) = std::move(ex).restoreArgs<std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> /*arg1*/, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> /*arg2*/, std::unique_ptr<::facebook::thrift::test::Foo> /*arg3*/>();
    goto determineInvocationType;
#endif // FOLLY_HAS_COROUTINES
  } catch (...) {
    callback->exception(std::current_exception());
  }
}


namespace facebook::thrift::test {

::facebook::thrift::test::MyI32_4873 ServiceSvNull::func(std::unique_ptr<::facebook::thrift::test::StringWithAdapter_7208> /*arg1*/, std::unique_ptr<::facebook::thrift::test::StringWithCppAdapter> /*arg2*/, std::unique_ptr<::facebook::thrift::test::Foo> /*arg3*/) { 
  return 0;
}


std::string_view ServiceAsyncProcessor::getServiceName() {
  return "Service";
}

void ServiceAsyncProcessor::getServiceMetadata(apache::thrift::metadata::ThriftServiceMetadataResponse& response) {
  ::apache::thrift::detail::md::ServiceMetadata<::apache::thrift::ServiceHandler<::facebook::thrift::test::Service>>::gen(response);
}

void ServiceAsyncProcessor::processSerializedCompressedRequestWithMetadata(apache::thrift::ResponseChannelRequest::UniquePtr req, apache::thrift::SerializedCompressedRequest&& serializedRequest, const apache::thrift::AsyncProcessorFactory::MethodMetadata& methodMetadata, apache::thrift::protocol::PROTOCOL_TYPES protType, apache::thrift::Cpp2RequestContext* context, folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  apache::thrift::detail::ap::process(this, iface_, std::move(req), std::move(serializedRequest), methodMetadata, protType, context, eb, tm);
}

void ServiceAsyncProcessor::executeRequest(apache::thrift::ServerRequest&& request, const apache::thrift::AsyncProcessorFactory::MethodMetadata& methodMetadata) {
  apache::thrift::detail::ap::execute(this, std::move(request), apache::thrift::detail::ServerRequestHelper::protocol(request), methodMetadata);
}

const ServiceAsyncProcessor::ProcessMap& ServiceAsyncProcessor::getOwnProcessMap() {
  return kOwnProcessMap_;
}

const ServiceAsyncProcessor::ProcessMap ServiceAsyncProcessor::kOwnProcessMap_ {
  {"func",
    {&ServiceAsyncProcessor::setUpAndProcess_func<apache::thrift::CompactProtocolReader, apache::thrift::CompactProtocolWriter>,
     &ServiceAsyncProcessor::setUpAndProcess_func<apache::thrift::BinaryProtocolReader, apache::thrift::BinaryProtocolWriter>,
     &ServiceAsyncProcessor::executeRequest_func<apache::thrift::CompactProtocolReader, apache::thrift::CompactProtocolWriter>,
     &ServiceAsyncProcessor::executeRequest_func<apache::thrift::BinaryProtocolReader, apache::thrift::BinaryProtocolWriter>}},
};

apache::thrift::ServiceRequestInfoMap const& ServiceServiceInfoHolder::requestInfoMap() const {
  static folly::Indestructible<apache::thrift::ServiceRequestInfoMap> requestInfoMap{staticRequestInfoMap()};
  return *requestInfoMap;
}

apache::thrift::ServiceRequestInfoMap ServiceServiceInfoHolder::staticRequestInfoMap() {
  apache::thrift::ServiceRequestInfoMap requestInfoMap = {
  {"func",
    { false,
     apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
     "Service.func",
     std::nullopt,
     apache::thrift::concurrency::NORMAL,
     std::nullopt}},
  };

  return requestInfoMap;
}
} // namespace facebook::thrift::test

namespace apache::thrift::detail {
::folly::Range<const ::std::string_view*>(*TSchemaAssociation<::facebook::thrift::test::Service, false>::bundle)() =
    nullptr;
}
