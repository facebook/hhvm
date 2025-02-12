/**
 * Autogenerated by Thrift for hsmodule.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include <thrift/compiler/test/fixtures/namespace/gen-py3/hsmodule/services_wrapper.h>
#include <thrift/compiler/test/fixtures/namespace/gen-py3/hsmodule/services_api.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace cpp2 {

HsTestServiceWrapper::HsTestServiceWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    import_my__namespacing__test__hsmodule__services();
  }


void HsTestServiceWrapper::async_tm_init(
  apache::thrift::HandlerCallbackPtr<int64_t> callback
    , int64_t int1
) {
  auto ctx = callback->getRequestContext();
  folly::via(
    this->executor,
    [this, ctx,
     callback = std::move(callback),
     int1
    ]() mutable {
        auto [promise, future] = folly::makePromiseContract<int64_t>();
        call_cy_HsTestService_init(
            this->if_object,
            ctx,
            std::move(promise),
            int1        );
        std::move(future).via(this->executor).thenTry([callback = std::move(callback)](folly::Try<int64_t>&& t) {
          (void)t;
          callback->complete(std::move(t));
        });
    });
}
std::shared_ptr<apache::thrift::ServerInterface> HsTestServiceInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<HsTestServiceWrapper>(if_object, exc);
}
folly::SemiFuture<folly::Unit> HsTestServiceWrapper::semifuture_onStartServing() {
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  call_cy_HsTestService_onStartServing(
      this->if_object,
      std::move(promise)
  );
  return std::move(future);
}
folly::SemiFuture<folly::Unit> HsTestServiceWrapper::semifuture_onStopRequested() {
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  call_cy_HsTestService_onStopRequested(
      this->if_object,
      std::move(promise)
  );
  return std::move(future);
}
} // namespace cpp2
