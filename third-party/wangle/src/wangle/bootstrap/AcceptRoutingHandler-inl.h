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

namespace wangle {

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::read(
    Context*,
    AcceptPipelineType conn) {
  if (conn.type() != typeid(ConnInfo&)) {
    return;
  }

  populateAcceptors();

  const auto& connInfo = boost::get<ConnInfo&>(conn);
  auto socket = std::shared_ptr<folly::AsyncTransport>(
      connInfo.sock, folly::DelayedDestruction::Destructor());

  uint64_t connId = nextConnId_++;

  // Create a new routing pipeline for this connection to read from
  // the socket until it parses the routing data
  auto routingPipeline = newRoutingPipeline();
  routingPipeline->addBack(wangle::AsyncSocketHandler(socket));
  routingPipeline->addBack(routingHandlerFactory_->newHandler(connId, this));
  routingPipeline->finalize();

  // Initialize TransportInfo and set it on the routing pipeline
  auto transportInfo = std::make_shared<TransportInfo>(connInfo.tinfo);
  folly::SocketAddress localAddr, peerAddr;
  try {
    socket->getLocalAddress(&localAddr);
    socket->getPeerAddress(&peerAddr);
  } catch (...) {
    VLOG(2) << "Socket is no longer valid.";
    return;
  }
  transportInfo->localAddr = std::make_shared<folly::SocketAddress>(localAddr);
  transportInfo->remoteAddr = std::make_shared<folly::SocketAddress>(peerAddr);
  routingPipeline->setTransportInfo(transportInfo);

  routingPipeline->transportActive();
  routingPipelines_[connId] = std::move(routingPipeline);
}

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::readEOF(Context*) {
  // Null implementation to terminate the call in this handler
}

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::readException(
    Context*,
    folly::exception_wrapper) {
  // Null implementation to terminate the call in this handler
}

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::onRoutingData(
    uint64_t connId,
    typename RoutingDataHandler<R>::RoutingData& routingData) {
  // Get the routing pipeline corresponding to this connection
  auto routingPipelineIter = routingPipelines_.find(connId);
  if (routingPipelineIter == routingPipelines_.end()) {
    VLOG(2) << "Connection has already been closed, "
               "or routed to a worker thread.";
    return;
  }
  auto routingPipeline = std::move(routingPipelineIter->second);
  routingPipelines_.erase(routingPipelineIter);

  // Fetch the socket from the pipeline and pause reading from the
  // socket
  auto socket = std::dynamic_pointer_cast<folly::AsyncTransport>(
      routingPipeline->getTransport());
  CHECK(socket);
  routingPipeline->transportInactive();
  auto originalEvb = socket->getEventBase();
  socket->detachEventBase();

  // Hash based on routing data to pick a new acceptor
  uint64_t hash = std::hash<R>()(routingData.routingData);
  auto acceptor = acceptors_[hash % acceptors_.size()];

  originalEvb->runInLoop(
      [=, this, routingData = std::move(routingData)]() mutable {
        // Switch to the new acceptor's thread
        acceptor->getEventBase()->runInEventBaseThread(
            [=, this, routingData = std::move(routingData)]() mutable {
              socket->attachEventBase(acceptor->getEventBase());

              auto routingHandler =
                  routingPipeline->template getHandler<RoutingDataHandler<R>>();
              DCHECK(routingHandler);
              auto transportInfo = routingPipeline->getTransportInfo();
              auto pipeline = childPipelineFactory_->newPipeline(
                  socket,
                  routingData.routingData,
                  routingHandler,
                  transportInfo);

              auto connection = new
                  typename ServerAcceptor<Pipeline>::ServerConnection(pipeline);
              acceptor->addConnection(connection);

              pipeline->transportActive();

              // Pass in the buffered bytes to the pipeline
              pipeline->read(routingData.bufQueue);
            });
      },
      /* thisIteration = */ true);
}

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::onError(
    uint64_t connId,
    folly::exception_wrapper ex) {
  VLOG(4) << "Exception while parsing routing data: " << ex.what();

  // Notify all handlers of the exception
  auto ctx = getContext();
  auto pipeline =
      CHECK_NOTNULL(dynamic_cast<AcceptPipeline*>(ctx->getPipeline()));
  pipeline->readException(ex);

  // Delete the routing pipeline. This will close and delete the socket as well.
  routingPipelines_.erase(connId);
}

template <typename Pipeline, typename R>
void AcceptRoutingHandler<Pipeline, R>::populateAcceptors() {
  if (!acceptors_.empty()) {
    return;
  }
  CHECK(server_);
  server_->forEachWorker(
      [&](Acceptor* acceptor) { acceptors_.push_back(acceptor); });
}

} // namespace wangle
