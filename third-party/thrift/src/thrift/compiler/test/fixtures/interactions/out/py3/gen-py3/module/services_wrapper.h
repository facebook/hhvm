/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/interactions/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#pragma once
#if __has_include(<thrift/compiler/test/fixtures/interactions/gen-cpp2/MyService.h>)
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/MyService.h>
#else
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h>
#endif
#if __has_include(<thrift/compiler/test/fixtures/interactions/gen-cpp2/Factories.h>)
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/Factories.h>
#else
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h>
#endif
#if __has_include(<thrift/compiler/test/fixtures/interactions/gen-cpp2/Perform.h>)
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/Perform.h>
#else
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h>
#endif
#if __has_include(<thrift/compiler/test/fixtures/interactions/gen-cpp2/InteractWithShared.h>)
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/InteractWithShared.h>
#else
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h>
#endif
#if __has_include(<thrift/compiler/test/fixtures/interactions/gen-cpp2/BoxService.h>)
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/BoxService.h>
#else
#include <thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h>
#endif
#include <folly/python/futures.h>
#include <Python.h>

#include <memory>

namespace cpp2 {

class MyServiceWrapper : virtual public MyServiceSvIf {
  protected:
    PyObject *if_object;
    folly::Executor *executor;
  public:
    explicit MyServiceWrapper(PyObject *if_object, folly::Executor *exc);
    void async_tm_foo(apache::thrift::HandlerCallbackPtr<void> callback) override;
    void async_tm_interact(apache::thrift::HandlerCallbackPtr<void> callback
        , int32_t arg
    ) override;
    void async_tm_interactFast(apache::thrift::HandlerCallbackPtr<int32_t> callback) override;
    void async_tm_serialize(apache::thrift::HandlerCallbackPtr<apache::thrift::ResponseAndServerStream<int32_t,int32_t>> callback) override;
    std::unique_ptr<MyInteractionIf> createMyInteraction() override;
    std::unique_ptr<MyInteractionFastIf> createMyInteractionFast() override;
    std::unique_ptr<SerialInteractionIf> createSerialInteraction() override;
folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;
};

std::shared_ptr<apache::thrift::ServerInterface> MyServiceInterface(PyObject *if_object, folly::Executor *exc);


class FactoriesWrapper : virtual public FactoriesSvIf {
  protected:
    PyObject *if_object;
    folly::Executor *executor;
  public:
    explicit FactoriesWrapper(PyObject *if_object, folly::Executor *exc);
    void async_tm_foo(apache::thrift::HandlerCallbackPtr<void> callback) override;
    void async_tm_interact(apache::thrift::HandlerCallbackPtr<void> callback
        , int32_t arg
    ) override;
    void async_tm_interactFast(apache::thrift::HandlerCallbackPtr<int32_t> callback) override;
    void async_tm_serialize(apache::thrift::HandlerCallbackPtr<apache::thrift::ResponseAndServerStream<int32_t,int32_t>> callback) override;
folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;
};

std::shared_ptr<apache::thrift::ServerInterface> FactoriesInterface(PyObject *if_object, folly::Executor *exc);


class PerformWrapper : virtual public PerformSvIf {
  protected:
    PyObject *if_object;
    folly::Executor *executor;
  public:
    explicit PerformWrapper(PyObject *if_object, folly::Executor *exc);
    void async_tm_foo(apache::thrift::HandlerCallbackPtr<void> callback) override;
    std::unique_ptr<MyInteractionIf> createMyInteraction() override;
    std::unique_ptr<MyInteractionFastIf> createMyInteractionFast() override;
    std::unique_ptr<SerialInteractionIf> createSerialInteraction() override;
folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;
};

std::shared_ptr<apache::thrift::ServerInterface> PerformInterface(PyObject *if_object, folly::Executor *exc);


class InteractWithSharedWrapper : virtual public InteractWithSharedSvIf {
  protected:
    PyObject *if_object;
    folly::Executor *executor;
  public:
    explicit InteractWithSharedWrapper(PyObject *if_object, folly::Executor *exc);
    void async_tm_do_some_similar_things(apache::thrift::HandlerCallbackPtr<std::unique_ptr<::thrift::shared_interactions::DoSomethingResult>> callback) override;
    std::unique_ptr<SharedInteractionIf> createSharedInteraction() override;
    std::unique_ptr<MyInteractionIf> createMyInteraction() override;
folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;
};

std::shared_ptr<apache::thrift::ServerInterface> InteractWithSharedInterface(PyObject *if_object, folly::Executor *exc);


class BoxServiceWrapper : virtual public BoxServiceSvIf {
  protected:
    PyObject *if_object;
    folly::Executor *executor;
  public:
    explicit BoxServiceWrapper(PyObject *if_object, folly::Executor *exc);
    void async_tm_getABoxSession(apache::thrift::HandlerCallbackPtr<std::unique_ptr<::cpp2::ShouldBeBoxed>> callback
        , std::unique_ptr<::cpp2::ShouldBeBoxed> req
    ) override;
folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;
};

std::shared_ptr<apache::thrift::ServerInterface> BoxServiceInterface(PyObject *if_object, folly::Executor *exc);
} // namespace cpp2
