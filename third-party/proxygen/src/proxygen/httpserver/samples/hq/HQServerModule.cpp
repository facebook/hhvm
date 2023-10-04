/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/H2Server.h>
#include <proxygen/httpserver/samples/hq/HQServerModule.h>
#include <proxygen/httpserver/samples/hq/SampleHandlers.h>
#include <proxygen/lib/http/session/HQSession.h>

using namespace proxygen;

namespace {
void sendKnobFrame(HQSession* session, const folly::StringPiece str) {
  if (str.empty()) {
    return;
  }
  uint64_t knobSpace = 0xfaceb00c;
  uint64_t knobId = 200;
  quic::Buf buf(folly::IOBuf::create(str.size()));
  memcpy(buf->writableData(), str.data(), str.size());
  buf->append(str.size());
  VLOG(10) << "Sending Knob Frame to peer. KnobSpace: " << std::hex << knobSpace
           << " KnobId: " << std::dec << knobId << " Knob Blob" << str;
  const auto knobSent = session->sendKnob(0xfaceb00c, 200, std::move(buf));
  if (knobSent.hasError()) {
    LOG(ERROR) << "Failed to send Knob frame to peer. Received error: "
               << knobSent.error();
  }
}
} // namespace

namespace quic::samples {

void startServer(
    const HQToolServerParams& params,
    std::unique_ptr<quic::QuicTransportStatsCallbackFactory>&& statsFactory) {
  // Run H2 server in a separate thread
  Dispatcher dispatcher(HandlerParams(
      params.protocol, params.port, params.httpVersion.canonical));
  auto dispatchFn = [&dispatcher](proxygen::HTTPMessage* request) {
    return dispatcher.getRequestHandler(request);
  };
  auto h2server = H2Server::run(params, dispatchFn);
  // Run HQ server
  std::function<void(HQSession*)> onTransportReadyFn;
  if (params.sendKnobFrame) {
    onTransportReadyFn = [](HQSession* session) {
      sendKnobFrame(session, ("Hello, World from Server!"));
    };
  }
  HQServer server(params, dispatchFn, std::move(onTransportReadyFn));
  if (statsFactory) {
    server.setStatsFactory(std::move(statsFactory));
  }

  server.start();
  // Wait until the quic server initializes
  server.getAddress();
  h2server.join();
  server.stop();
}

} // namespace quic::samples
