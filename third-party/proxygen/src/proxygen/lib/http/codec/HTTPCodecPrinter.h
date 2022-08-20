/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPCodecFilter.h>

namespace proxygen {

/**
 * This class enforces certain higher-level HTTP semantics. It does not enforce
 * conditions that require state to decide. That is, this class is stateless and
 * only examines the calls and callbacks that go through it.
 */

class HTTPCodecPrinter : public PassThroughHTTPCodecFilter {
 public:
  /*
   * Called from HTTP2Codec::parseIngress()
   *             HTTP2Codec::onIngress()
   * when SPDY and HTTP/2 frame headers are parsed
   */
  void onFrameHeader(StreamID stream_id,
                     uint8_t flags,
                     uint64_t length,
                     uint64_t type,
                     uint16_t version = 0) override;

  /*
   * Called from HTTP2Codec::failSession()
   *             HTTP2Codec::checkConnectionError()
   */
  void onError(StreamID stream,
               const HTTPException& error,
               bool newStream = false) override;

  /*
   * Called from HTTP2Codec::parseIngress()
   *             HTTP2Codec::parseData()
   */
  void onBody(StreamID stream,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override;

  /*
   * Called from HTTP2Codec::parseIngress()
   *             HTTP2Codec::handleEndStream()
   */
  void onMessageComplete(StreamID stream, bool upgrade) override;

  /*
   * Called from HTTP2Codec::onSynCommon()
   *             HTTP2Codec::HTTP2Codec::parseHeadersImpl()
   */
  void onHeadersComplete(StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override;

  /*
   * Called from HTTP2Codec::onRstStream()
   *             HTTP2Codec::parseRstStream()
   */
  void onAbort(StreamID stream, ErrorCode code) override;

  /*
   * Called from HTTP2Codec::onWindowUpdate() with different arguments
   *             HTTP2Codec::parseWindowUpdate()
   */
  void onWindowUpdate(StreamID stream, uint32_t amount) override;

  /*
   * Called from HTTP2Codec::onSettings()
   *             HTTP2Codec::parseSettings()
   */
  void onSettings(const SettingsList& settings) override;

  /*
   * Called from HTTP2Codec::parseSettings()
   */
  void onSettingsAck() override;

  /*
   * Called from HTTP2Codec::onGoaway() with different arguments
   *             HTTP2Codec::parseGoaway()
   */
  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  /*
   * Called from HTTP2Codec::onPing()
   *             HTTP2Codec::parsePing()
   */
  void onPingRequest(uint64_t data) override;

  /*
   * Called from HTTP2Codec::onPing()
   *             HTTP2Codec::parsePing()
   */
  void onPingReply(uint64_t data) override;

 protected:
  void printPing(uint64_t data);
};

} // namespace proxygen
