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

#include <wangle/bootstrap/RoutingDataHandler.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/Pipeline.h>

namespace wangle {

/**
 * An AcceptPipeline with the ability to hash connections to
 * a specific worker thread. Hashing can be based on data passed
 * in by the client.
 *
 * For each connection, AcceptRoutingHandler creates and maintains a routing
 * pipeline internally. The routing pipeline should take care of
 * reading from the socket, parsing the data based on which to hash
 * the connection and invoking RoutingDataHandler::Callback::onRoutingData
 * to notify the AcceptRoutingHandler. AcceptRoutingHandler then pauses
 * reads from the socket, moves the connection over to the hashed
 * worker thread, and resumes reading from the socket on the child pipeline.
 */

template <typename Pipeline, typename R>
class RoutingDataPipelineFactory;

template <typename Pipeline, typename R>
class AcceptRoutingHandler : public wangle::InboundHandler<AcceptPipelineType>,
                             public RoutingDataHandler<R>::Callback {
 public:
  AcceptRoutingHandler(
      ServerBootstrap<Pipeline>* server,
      std::shared_ptr<RoutingDataHandlerFactory<R>> routingHandlerFactory,
      std::shared_ptr<RoutingDataPipelineFactory<Pipeline, R>>
          childPipelineFactory)
      : server_(CHECK_NOTNULL(server)),
        routingHandlerFactory_(routingHandlerFactory),
        childPipelineFactory_(childPipelineFactory) {}

  // InboundHandler implementation
  void read(Context* ctx, AcceptPipelineType conn) override;
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper ex) override;

  // RoutingDataHandler::Callback implementation
  void onRoutingData(
      uint64_t connId,
      typename RoutingDataHandler<R>::RoutingData& routingData) override;
  void onError(uint64_t connId, folly::exception_wrapper ex) override;

  size_t getRoutingPipelineCount() const {
    return routingPipelines_.size();
  }

 private:
  void populateAcceptors();
  virtual DefaultPipeline::Ptr newRoutingPipeline() {
    return DefaultPipeline::create();
  }

  ServerBootstrap<Pipeline>* server_;
  std::shared_ptr<RoutingDataHandlerFactory<R>> routingHandlerFactory_;
  std::shared_ptr<RoutingDataPipelineFactory<Pipeline, R>>
      childPipelineFactory_;

  std::vector<Acceptor*> acceptors_;
  std::map<uint64_t, DefaultPipeline::Ptr> routingPipelines_;
  uint64_t nextConnId_{0};
};

template <typename Pipeline, typename R>
class AcceptRoutingPipelineFactory : public AcceptPipelineFactory {
 public:
  AcceptRoutingPipelineFactory(
      ServerBootstrap<Pipeline>* server,
      std::shared_ptr<RoutingDataHandlerFactory<R>> routingHandlerFactory,
      std::shared_ptr<RoutingDataPipelineFactory<Pipeline, R>>
          childPipelineFactory)
      : server_(CHECK_NOTNULL(server)),
        routingHandlerFactory_(routingHandlerFactory),
        childPipelineFactory_(childPipelineFactory) {}

  AcceptPipeline::Ptr newPipeline(Acceptor*) override {
    auto pipeline = AcceptPipeline::create();
    pipeline->addBack(AcceptRoutingHandler<Pipeline, R>(
        server_, routingHandlerFactory_, childPipelineFactory_));
    pipeline->finalize();

    return pipeline;
  }

 protected:
  ServerBootstrap<Pipeline>* server_;
  std::shared_ptr<RoutingDataHandlerFactory<R>> routingHandlerFactory_;
  std::shared_ptr<RoutingDataPipelineFactory<Pipeline, R>>
      childPipelineFactory_;
};

template <typename Pipeline, typename R>
class RoutingDataPipelineFactory {
 public:
  virtual ~RoutingDataPipelineFactory() = default;

  virtual typename Pipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport> socket,
      const R& routingData,
      RoutingDataHandler<R>* routingHandler,
      std::shared_ptr<TransportInfo> transportInfo) = 0;
};

} // namespace wangle

#include <wangle/bootstrap/AcceptRoutingHandler-inl.h>
