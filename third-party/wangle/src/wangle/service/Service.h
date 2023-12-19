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

#include <folly/Memory.h>
#include <folly/futures/Future.h>

#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/Pipeline.h>

namespace wangle {

/**
 * A Service is an asynchronous function from Request to
 * Future<Response>. It is the basic unit of the RPC interface.
 */
template <typename Req, typename Resp = Req>
class Service {
 public:
  virtual folly::Future<Resp> operator()(Req request) = 0;
  virtual ~Service() = default;
  virtual folly::Future<folly::Unit> close() {
    return folly::makeFuture();
  }
  virtual bool isAvailable() {
    return true;
  }
};

/**
 * A Filter acts as a decorator/transformer of a service. It may apply
 * transformations to the input and output of that service:
 *
 *          class MyService
 *
 * ReqA  -> |
 *          | -> ReqB
 *          | <- RespB
 * RespA <- |
 *
 * For example, you may have a service that takes Strings and parses
 * them as Ints.  If you want to expose this as a Network Service via
 * Thrift, it is nice to isolate the protocol handling from the
 * business rules. Hence you might have a Filter that converts back
 * and forth between Thrift structs:
 *
 * [ThriftIn -> (String  ->  Int) -> ThriftOut]
 */
template <
    typename ReqA,
    typename RespA,
    typename ReqB = ReqA,
    typename RespB = RespA>
class ServiceFilter : public Service<ReqA, RespA> {
 public:
  explicit ServiceFilter(std::shared_ptr<Service<ReqB, RespB>> service)
      : service_(service) {}
  ~ServiceFilter() override = default;

  folly::Future<folly::Unit> close() override {
    return service_->close();
  }

  bool isAvailable() override {
    return service_->isAvailable();
  }

 protected:
  std::shared_ptr<Service<ReqB, RespB>> service_;
};

/**
 * A factory that creates services, given a client.  This lets you
 * make RPC calls on the Service interface over a client's pipeline.
 *
 * Clients can be reused after you are done using the service.
 */
template <typename Pipeline, typename Req, typename Resp>
class ServiceFactory {
 public:
  virtual folly::Future<std::shared_ptr<Service<Req, Resp>>> operator()(
      std::shared_ptr<ClientBootstrap<Pipeline>> client) = 0;

  virtual ~ServiceFactory() = default;
};

template <typename Pipeline, typename Req, typename Resp>
class ConstFactory : public ServiceFactory<Pipeline, Req, Resp> {
 public:
  explicit ConstFactory(std::shared_ptr<Service<Req, Resp>> service)
      : service_(service) {}

  folly::Future<std::shared_ptr<Service<Req, Resp>>> operator()(
      std::shared_ptr<ClientBootstrap<Pipeline>> /* client */) override {
    return service_;
  }

 private:
  std::shared_ptr<Service<Req, Resp>> service_;
};

template <
    typename Pipeline,
    typename ReqA,
    typename RespA,
    typename ReqB = ReqA,
    typename RespB = RespA>
class ServiceFactoryFilter : public ServiceFactory<Pipeline, ReqA, RespA> {
 public:
  explicit ServiceFactoryFilter(
      std::shared_ptr<ServiceFactory<Pipeline, ReqB, RespB>> serviceFactory)
      : serviceFactory_(std::move(serviceFactory)) {}

  ~ServiceFactoryFilter() override = default;

 protected:
  std::shared_ptr<ServiceFactory<Pipeline, ReqB, RespB>> serviceFactory_;
};

template <typename Pipeline, typename Req, typename Resp = Req>
class FactoryToService : public Service<Req, Resp> {
 public:
  explicit FactoryToService(
      std::shared_ptr<ServiceFactory<Pipeline, Req, Resp>> factory)
      : factory_(factory) {}
  ~FactoryToService() override = default;

  folly::Future<Resp> operator()(Req request) override {
    DCHECK(factory_);
    return ((*factory_)(nullptr))
        .thenValue([=, this](std::shared_ptr<Service<Req, Resp>> service) {
          return (*service)(std::move(request)).ensure([this]() {
            this->close();
          });
        });
  }

 private:
  std::shared_ptr<ServiceFactory<Pipeline, Req, Resp>> factory_;
};

} // namespace wangle
