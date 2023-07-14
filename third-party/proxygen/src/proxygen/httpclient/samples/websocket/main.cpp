/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/EventBase.h>
#include <proxygen/httpclient/samples/websocket/WebSocketClient.h>
#include <proxygen/lib/http/HTTPConnector.h>
#include <proxygen/lib/utils/URL.h>

using namespace folly;
using namespace proxygen;
using namespace websocketclient;

DEFINE_int32(http_client_connect_timeout,
             1000,
             "connect timeout in milliseconds");
DEFINE_string(url,
              "http://localhost:11000/",
              "URL to perform the HTTP method against");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv, false);

  EventBase evb;
  URL url(FLAGS_url);

  WebSocketClient client(&evb, url);

  auto addr = SocketAddress(url.getHost(), url.getPort(), true);
  LOG(INFO) << "Trying to connect to " << addr;

  // Note: HHWheelTimer is a large object and should be created at most
  // once per thread
  HHWheelTimer::UniquePtr timer{HHWheelTimer::newTimer(
      &evb,
      std::chrono::milliseconds(HHWheelTimer::DEFAULT_TICK_INTERVAL),
      AsyncTimeout::InternalEnum::NORMAL,
      std::chrono::milliseconds(5000))};
  HTTPConnector connector(&client, timer.get());
  static const SocketOptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};

  connector.connect(
      &evb,
      addr,
      std::chrono::milliseconds(FLAGS_http_client_connect_timeout),
      opts);

  evb.loop();

  return EXIT_SUCCESS;
}
