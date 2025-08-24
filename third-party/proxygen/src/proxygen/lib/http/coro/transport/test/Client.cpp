/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/CoroSSLTransport.h"
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <folly/portability/GFlags.h>
#include <iostream>

using namespace proxygen::coro;

class Callback : public folly::AsyncSocket::ConnectCallback {
 public:
  bool connected{false};

  void connectSuccess() noexcept override {
    XLOG(INFO) << "Connected";
    connected = true;
  }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    XLOG(ERR) << "Connect failed: " << ex.what();
  }
};

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  ::gflags::ParseCommandLineFlags(&argc, &argv, false);

  if (argc < 2) {
    XLOG(ERR) << "Usage: async_ssl_transport_client server[:port]";
    return 1;
  }

  folly::SocketAddress serverAddr;
  serverAddr.setFromHostPort(argv[1]);

  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb);
  Callback callback;
  socket->connect(&callback, serverAddr, 1000);
  evb.loop();
  if (callback.connected) {
    auto transport = std::make_unique<CoroSSLTransport>(
        std::make_unique<folly::coro::Transport>(&evb, std::move(socket)),
        std::make_shared<folly::SSLContext>());
    co_withExecutor(
        &evb,
        [](std::unique_ptr<CoroSSLTransport> transport)
            -> folly::coro::Task<void> {
          co_await transport->connect(folly::none, std::chrono::seconds(1));
          std::string getRequest("GET / HTTP/1.0\r\n\r\n");
          co_await transport->write(
              folly::ByteRange(reinterpret_cast<uint8_t*>(getRequest.data()),
                               getRequest.length()));
          folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
          auto nRead = co_await transport->read(
              readBuf, 1000, 4000, std::chrono::seconds(1));
          readBuf.gather(nRead);
          std::cout << readBuf.move()->moveToFbString();
        }(std::move(transport)))
        .start();
    evb.loop();
  }
  return 0;
}
