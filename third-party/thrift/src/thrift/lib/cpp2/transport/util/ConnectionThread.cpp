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

#include <thrift/lib/cpp2/transport/util/ConnectionThread.h>

#include <glog/logging.h>
#include <folly/portability/GFlags.h>

#include <folly/Conv.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/transport/http2/client/H2ClientConnection.h>

DEFINE_string(transport, "http2", "The transport to use (http2)");
DEFINE_bool(use_ssl, false, "Create an encrypted client connection");

namespace apache {
namespace thrift {

ConnectionThread::~ConnectionThread() {
  getEventBase()->runInEventBaseThreadAndWait(
      [&] { connections_.wlock()->clear(); });
}

std::shared_ptr<ClientConnectionIf> ConnectionThread::getConnection(
    const std::string& addr, uint16_t port) {
  std::string serverKey = folly::to<std::string>(addr, ":", port);
  getEventBase()->runInEventBaseThreadAndWait(
      [&]() { maybeCreateConnection(serverKey, addr, port); });
  return connections_.withWLock(
      [&](auto& connections) { return connections[serverKey]; });
}

void ConnectionThread::maybeCreateConnection(
    const std::string& serverKey, const std::string& addr, uint16_t port) {
  LOG_IF(FATAL, FLAGS_transport == "rocket")
      << "Use RocketClientChannel::newChannel()";

  connections_.withWLock([&, this](auto& connections) {
    std::shared_ptr<ClientConnectionIf>& connection = connections[serverKey];
    if (connection == nullptr || !connection->good()) {
      folly::AsyncSocket::UniquePtr socket(
          new folly::AsyncSocket(this->getEventBase(), addr, port));
      if (FLAGS_use_ssl) {
        auto sslContext = std::make_shared<folly::SSLContext>();
        sslContext->setAdvertisedNextProtocols({"h2", "http"});
        auto sslSocket = folly::AsyncSSLSocket::newSocket(
            sslContext,
            this->getEventBase(),
            socket->detachNetworkSocket(),
            false);
        sslSocket->sslConn(nullptr);
        socket = std::move(sslSocket);
      }
      if (FLAGS_transport != "http2") {
        LOG(ERROR) << "Unknown transport " << FLAGS_transport
                   << ".  Will use http2.";
      }
      connection = H2ClientConnection::newHTTP2Connection(std::move(socket));
    }
  });
}

} // namespace thrift
} // namespace apache
