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

#include <folly/portability/GFlags.h>

#include <folly/init/Init.h>
#include <wangle/bootstrap/AcceptRoutingHandler.h>
#include <wangle/bootstrap/RoutingDataHandler.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/broadcast/BroadcastHandler.h>
#include <wangle/channel/broadcast/BroadcastPool.h>
#include <wangle/channel/broadcast/ObservingHandler.h>
#include <wangle/codec/ByteToMessageDecoder.h>
#include <wangle/codec/MessageToByteEncoder.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 8080, "Broadcast proxy port");
DEFINE_int32(upstream_port, 8081, "Upstream server port");

/**
 * Steps to run:
 * 1) Run an upstream server that can broadcast messages:
 *
 *    nc -l localhost 8081
 *
 *    This starts a server on localhost:8081.
 *
 * 2) Start the broadcast proxy with the upstream_port set to 8081:
 *
 *    ./broadcast_proxy --port 8080 --upstream_port 8081
 *
 *    This starts the proxy on localhost:8080 and sets the upstream server
 *    as localhost:8081
 *
 * 3) Start a new instances of telnet clients to connect to the broadcast proxy
 *    and listen to the messages broadcasted by the upstream server:
 *
 *    telnet localhost 8080
 *
 *    Send some bytes in the telnet terminals for broadcast_proxy to kick off
 *    the connection.
 *
 * 4) Type something in the nc terminal and notice that it is broadcasted to all
 *    the telnet clients.
 */

/**
 * A simple decoder that decodes bytes in IOBufQueue to std::string.
 * This is used in the BroadcastPipeline to convert bytes read from the
 * upstream server's socket to strings of messages that can be broadcasted
 * to all the clients/observers.
 */
class ByteToStringDecoder : public ByteToMessageDecoder<std::string> {
 public:
  bool decode(Context*, IOBufQueue& buf, std::string& result, size_t&)
      override {
    if (buf.chainLength() > 0) {
      result = buf.move()->moveToFbString().toStdString();
      return true;
    }
    return false;
  }
};

/**
 * A simple encoder that encodes strings of messages to IOBuf.
 * This is used in the ObservingPipeline to encode the messages
 * broadcasted by the upstream to IOBuf so that it can be written
 * to the client socket.
 */
class StringToByteEncoder : public MessageToByteEncoder<std::string> {
 public:
  std::unique_ptr<folly::IOBuf> encode(std::string& msg) override {
    return IOBuf::copyBuffer(msg);
  }
};

/**
 * Simple RoutingDataHandler that sets the client IP as the routing data.
 * All requests from the same client IP will be hashed to the same worker
 * thread.
 */
class ClientIPRoutingDataHandler : public RoutingDataHandler<std::string> {
 public:
  ClientIPRoutingDataHandler(uint64_t connId, Callback* cob)
      : RoutingDataHandler<std::string>(connId, cob) {}

  bool parseRoutingData(folly::IOBufQueue& bufQueue, RoutingData& routingData)
      override {
    auto transportInfo = getContext()->getPipeline()->getTransportInfo();
    const auto& clientIP = transportInfo->remoteAddr->getAddressStr();
    LOG(INFO) << "Using client IP " << clientIP
              << " as routing data to hash to a worker thread";

    routingData.routingData = clientIP;
    routingData.bufQueue.append(bufQueue);
    return true;
  }
};

class ClientIPRoutingDataHandlerFactory
    : public RoutingDataHandlerFactory<std::string> {
 public:
  std::shared_ptr<RoutingDataHandler<std::string>> newHandler(
      uint64_t connId,
      RoutingDataHandler<std::string>::Callback* cob) override {
    return std::make_shared<ClientIPRoutingDataHandler>(connId, cob);
  }
};

/**
 * Implementation of a broadcast ServerPool that establishes connection
 * to an upstream server.
 */
class SimpleServerPool : public ServerPool<std::string> {
 public:
  Future<DefaultPipeline*> connect(
      BaseClientBootstrap<DefaultPipeline>* client,
      const std::string& /* routingData */) noexcept override {
    SocketAddress address;
    address.setFromLocalPort(FLAGS_upstream_port);

    LOG(INFO) << "Connecting to upstream server " << address
              << " for subscribing to broadcast";
    return client->connect(address);
  }
};

/**
 * BroadcastPipeline maintains the upstream connection and broadcasts
 * messages sent by the upstream server to all the observers/clients.
 */
class SimpleBroadcastPipelineFactory
    : public BroadcastPipelineFactory<std::string, std::string> {
 public:
  DefaultPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> socket) override {
    LOG(INFO) << "Creating a new BroadcastPipeline for upstream server";

    auto pipeline = DefaultPipeline::create();
    pipeline->addBack(AsyncSocketHandler(socket));
    pipeline->addBack(ByteToStringDecoder());
    pipeline->addBack(BroadcastHandler<std::string, std::string>());
    pipeline->finalize();
    return pipeline;
  }

  BroadcastHandler<std::string, std::string>* getBroadcastHandler(
      DefaultPipeline* pipeline) noexcept override {
    return pipeline->getHandler<BroadcastHandler<std::string, std::string>>();
  }

  void setRoutingData(
      DefaultPipeline* /* pipeline */,
      const std::string& /* routingData */) noexcept override {}
};

using SimpleObservingPipeline = ObservingPipeline<std::string>;

/**
 * An ObservingPipeline that maintains the client socket connection and
 * subscribes to the BroadcastPipeline to receive messages sent by the
 * upstream server. A new ObservingPipeline is created for each client
 * connection.
 */
class SimpleObservingPipelineFactory
    : public ObservingPipelineFactory<std::string, std::string> {
 public:
  SimpleObservingPipelineFactory(
      std::shared_ptr<SimpleServerPool> serverPool,
      std::shared_ptr<SimpleBroadcastPipelineFactory> broadcastPipelineFactory)
      : ObservingPipelineFactory<std::string, std::string>(
            serverPool,
            broadcastPipelineFactory) {}

  SimpleObservingPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> socket,
      const std::string& routingData,
      RoutingDataHandler<std::string>*,
      std::shared_ptr<TransportInfo> transportInfo) override {
    LOG(INFO) << "Creating a new ObservingPipeline for client "
              << *(transportInfo->remoteAddr);

    auto pipeline = SimpleObservingPipeline::create();
    pipeline->addBack(AsyncSocketHandler(socket));
    pipeline->addBack(StringToByteEncoder());
    pipeline->addBack(
        std::make_shared<ObservingHandler<std::string, std::string>>(
            routingData, broadcastPool()));
    pipeline->finalize();
    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  auto serverPool = std::make_shared<SimpleServerPool>();

  // A unique BroadcastPipeline for each upstream server to fan-out the
  // upstream messages to ObservingPipelines corresponding to each client.
  auto broadcastPipelineFactory =
      std::make_shared<SimpleBroadcastPipelineFactory>();

  // A unique ObservingPipeline is created for each client to subscribe
  // to the broadcast.
  auto observingPipelineFactory =
      std::make_shared<SimpleObservingPipelineFactory>(
          serverPool, broadcastPipelineFactory);

  // RoutingDataHandlerFactory for creating the RoutingDataHandler that sets
  // client IP as the routing data.
  auto routingHandlerFactory =
      std::make_shared<ClientIPRoutingDataHandlerFactory>();

  ServerBootstrap<SimpleObservingPipeline> server;

  // AcceptRoutingPipelineFactory for creating accept pipelines hash the
  // client connection to a worker thread based on client IP.
  auto acceptPipelineFactory = std::make_shared<
      AcceptRoutingPipelineFactory<SimpleObservingPipeline, std::string>>(
      &server, routingHandlerFactory, observingPipelineFactory);

  server.pipeline(acceptPipelineFactory);
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
