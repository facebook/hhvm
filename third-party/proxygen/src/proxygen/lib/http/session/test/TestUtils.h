/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/io/async/TimeoutManager.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/session/HTTPSession.h>

namespace proxygen {

extern const wangle::TransportInfo mockTransportInfo;
extern const folly::SocketAddress localAddr;
extern const folly::SocketAddress peerAddr;

folly::HHWheelTimer::UniquePtr makeInternalTimeoutSet(folly::EventBase* evb);

folly::HHWheelTimer::UniquePtr makeTimeoutSet(folly::EventBase* evb);

testing::NiceMock<folly::test::MockAsyncTransport>* newMockTransport(
    folly::EventBase* evb);

struct HTTP1xCodecPair {
  using Codec = HTTP1xCodec;
  static const int version = 1;
};

struct HTTP2CodecPair {
  using Codec = HTTP2Codec;
  static const int version = 2;
};

struct MockHTTPCodecPair {
  using Codec = MockHTTPCodec;
  static const int version = 0;
};

} // namespace proxygen
