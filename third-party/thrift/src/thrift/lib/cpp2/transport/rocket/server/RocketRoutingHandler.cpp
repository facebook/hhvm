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

#include <chrono>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>

THRIFT_FLAG_DEFINE_bool(rocket_set_idle_connection_timeout, true);
THRIFT_FLAG_DEFINE_int64(
    socket_process_time_us_logging_threshold, 10 * 1000 * 1000); // 10s

namespace apache {
namespace thrift {
namespace detail {

#define THRIFT_DETAIL_REGISTER_SERVER_EXTENSION_DEFAULT(FUNC)     \
  THRIFT_PLUGGABLE_FUNC_REGISTER(                                 \
      std::unique_ptr<apache::thrift::rocket::SetupFrameHandler>, \
      FUNC,                                                       \
      apache::thrift::ThriftServer&) {                            \
    return {};                                                    \
  }

THRIFT_DETAIL_REGISTER_SERVER_EXTENSION_DEFAULT(
    createRocketDebugSetupFrameHandler)
THRIFT_DETAIL_REGISTER_SERVER_EXTENSION_DEFAULT(
    createRocketMonitoringSetupFrameHandler)
THRIFT_DETAIL_REGISTER_SERVER_EXTENSION_DEFAULT(
    createRocketProfilingSetupFrameHandler)

#undef THRIFT_DETAIL_REGISTER_SERVER_EXTENSION_DEFAULT

} // namespace detail

RocketRoutingHandler::RocketRoutingHandler(ThriftServer& server) {
  auto addSetupFramehandler = [&](auto&& handlerFactory) {
    if (auto handler = handlerFactory(server)) {
      setupFrameHandlers_.push_back(std::move(handler));
    }
  };
  addSetupFramehandler(detail::createRocketDebugSetupFrameHandler);
  addSetupFramehandler(detail::createRocketMonitoringSetupFrameHandler);
  addSetupFramehandler(detail::createRocketProfilingSetupFrameHandler);
}

RocketRoutingHandler::~RocketRoutingHandler() {
  stopListening();
}

void RocketRoutingHandler::stopListening() {
  listening_ = false;
}

bool RocketRoutingHandler::canAcceptConnection(
    const std::vector<uint8_t>& bytes, const wangle::TransportInfo&) {
  class FrameHeader {
   public:
    /*
     * Sample start of an Rsocket frame (version 1.0) in Octal:
     * 0x0000 2800 0000 0004 0000 0100 00....
     * Rsocket frame length - 24 bits
     * StreamId             - 32 bits
     * Frame type           -  6 bits
     * Flags                - 10 bits
     * Major version        - 16 bits
     * Minor version        - 16 bits
     */
    static uint16_t getMajorVersion(const std::vector<uint8_t>& bytes) {
      return bytes[9] << 8 | bytes[10];
    }
    static uint16_t getMinorVersion(const std::vector<uint8_t>& bytes) {
      return bytes[11] << 8 | bytes[12];
    }
    static rocket::FrameType getType(const std::vector<uint8_t>& bytes) {
      return rocket::FrameType(bytes[7] >> 2);
    }
  };

  return listening_ &&
      // This only supports Rsocket protocol version 1.0
      FrameHeader::getMajorVersion(bytes) == 1 &&
      FrameHeader::getMinorVersion(bytes) == 0 &&
      FrameHeader::getType(bytes) == rocket::FrameType::SETUP;
}

bool RocketRoutingHandler::canAcceptEncryptedConnection(
    const std::string& protocolName) {
  return listening_ && protocolName == "rs";
}

void RocketRoutingHandler::handleConnection(
    wangle::ConnectionManager* connectionManager,
    folly::AsyncTransport::UniquePtr sock,
    const folly::SocketAddress* address,
    const wangle::TransportInfo& tinfo,
    std::shared_ptr<Cpp2Worker> worker) {
  if (!listening_) {
    return;
  }

  auto* const server = worker->getServer();

  rocket::RocketServerConnection::Config cfg;
  cfg.socketWriteTimeout = server->getSocketWriteTimeout();
  cfg.streamStarvationTimeout = server->getStreamExpireTime();
  cfg.writeBatchingInterval = server->getWriteBatchingInterval();
  cfg.writeBatchingSize = server->getWriteBatchingSize();
  cfg.writeBatchingByteSize = server->getWriteBatchingByteSize();
  cfg.egressBufferBackpressureThreshold =
      server->getEgressBufferBackpressureThreshold();
  cfg.egressBufferBackpressureRecoveryFactor =
      server->getEgressBufferRecoveryFactor();
  cfg.socketOptions = &server->getPerConnectionSocketOptions();
  cfg.parserAllocator = server->getCustomAllocatorForParser();

  auto* const sockPtr = sock.get();
  auto* const connection = new rocket::RocketServerConnection(
      std::move(sock),
      std::make_unique<rocket::ThriftRocketServerHandler>(
          worker, *address, sockPtr, setupFrameHandlers_),
      worker->getIngressMemoryTracker(),
      worker->getEgressMemoryTracker(),
      cfg);
  onConnection(*connection);
  connectionManager->addConnection(
      connection,
      THRIFT_FLAG(rocket_set_idle_connection_timeout),
      /* connectionAgeTimeout */ true);

  if (auto* observer = server->getObserver()) {
    if (auto threshold = THRIFT_FLAG(socket_process_time_us_logging_threshold);
        threshold > 0) {
      auto now = std::chrono::steady_clock::now();
      auto dur = now - tinfo.timeBeforeEnqueue;
      auto durCnt =
          std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
      if (durCnt > threshold) {
        THRIFT_CONNECTION_EVENT(insane_socket_process_time)
            .log(*server, *address, [&tinfo, now, durCnt] {
              folly::dynamic metadata = folly::dynamic::object;
              auto beforeEnqueue =
                  tinfo.timeBeforeEnqueue.time_since_epoch().count();
              auto afterDequeue = tinfo.acceptTime.time_since_epoch().count();
              metadata["time_before_enqueue_since_epoch"] = beforeEnqueue;
              metadata["time_after_dequeue_since_epoch"] = afterDequeue;
              metadata["socket_queue_time_us"] =
                  std::chrono::duration_cast<std::chrono::microseconds>(
                      tinfo.acceptTime - tinfo.timeBeforeEnqueue)
                      .count();
              metadata["socket_sucess_process_time_us"] = durCnt;
              metadata["now_since_epoch"] = now.time_since_epoch().count();
              metadata["ssl_setup_time_ms"] = tinfo.sslSetupTime.count();
              return metadata;
            });
      }
    }

    observer->connAccepted(tinfo);
    observer->activeConnections(
        connectionManager->getNumConnections() *
        server->getNumIOWorkerThreads());
  }
}

} // namespace thrift
} // namespace apache
