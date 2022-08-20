/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HPACKCodec.h>
#include <proxygen/lib/http/codec/compress/NoPathIndexingStrategy.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionScheme.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/HPACKQueue.h>

namespace proxygen { namespace compress {

/**
 * Compression scheme for HPACK with a prepended sequence number
 */
class HPACKScheme : public CompressionScheme {
 public:
  explicit HPACKScheme(CompressionSimulator* sim, uint32_t tableSize)
      : CompressionScheme(sim) {
    client_.setEncodeHeadroom(2);
    client_.setHeaderIndexingStrategy(NoPathIndexingStrategy::getInstance());
    server_.setHeaderIndexingStrategy(NoPathIndexingStrategy::getInstance());
    client_.setEncoderHeaderTableSize(tableSize);
    server_.setDecoderHeaderTableMaxSize(tableSize);
    allowOOO_ = (tableSize == 0);
  }

  ~HPACKScheme() {
    CHECK_EQ(serverQueue_.getQueuedBytes(), 0);
  }

  // HPACK has no ACKs
  std::unique_ptr<Ack> getAck(uint16_t /*seqn*/) override {
    return nullptr;
  }
  void recvAck(std::unique_ptr<Ack> /*ack*/) override {
  }

  std::pair<FrameFlags, std::unique_ptr<folly::IOBuf>> encode(
      bool /*newPacket*/,
      std::vector<compress::Header> allHeaders,
      SimStats& stats) override {
    auto block = client_.encode(allHeaders);
    block->prepend(sizeof(uint16_t));
    folly::io::RWPrivateCursor c(block.get());
    c.writeBE<uint16_t>(index++);
    stats.uncompressed += client_.getEncodedSize().uncompressed;
    stats.compressed += client_.getEncodedSize().compressed;
    // OOO is allowed with 0 table size
    FrameFlags flags{allowOOO_};
    return {flags, std::move(block)};
  }

  void decode(FrameFlags flags,
              std::unique_ptr<folly::IOBuf> encodedReq,
              SimStats& stats,
              SimStreamingCallback& callback) override {
    folly::io::Cursor cursor(encodedReq.get());
    auto seqn = cursor.readBE<uint16_t>();
    callback.seqn = seqn;
    VLOG(1) << "Decoding request=" << callback.requestIndex
            << " header seqn=" << seqn;
    auto len = cursor.totalLength();
    encodedReq->trimStart(sizeof(uint16_t));
    serverQueue_.enqueueHeaderBlock(
        seqn, std::move(encodedReq), len, &callback, flags.allowOOO);
    callback.maybeMarkHolDelay();
    if (serverQueue_.getQueuedBytes() > stats.maxQueueBufferBytes) {
      stats.maxQueueBufferBytes = serverQueue_.getQueuedBytes();
    }
  }

  uint32_t getHolBlockCount() const override {
    return serverQueue_.getHolBlockCount();
  }

  HPACKCodec client_{TransportDirection::UPSTREAM};
  HPACKCodec server_{TransportDirection::DOWNSTREAM};
  HPACKQueue serverQueue_{server_};
  bool allowOOO_{false};
};
}} // namespace proxygen::compress
