/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>

namespace fizz {

/**
 * Keeps track of the handshake transcript and provides access to the
 * handshake context.
 */
class HandshakeContext {
 public:
  virtual ~HandshakeContext() = default;

  /**
   * Appends transcript to the current handshake transcript.
   */
  virtual void appendToTranscript(const Buf& transcript) = 0;

  /**
   * Returns the handshake context for the current transcript.
   */
  virtual Buf getHandshakeContext() const = 0;

  /**
   * Returns the finished verify_data from the current handshake context and
   * baseKey.
   */
  virtual Buf getFinishedData(folly::ByteRange baseKey) const = 0;

  /**
   * Returns the handshake context for an empty transcript.
   */
  virtual folly::ByteRange getBlankContext() const = 0;

  virtual std::unique_ptr<HandshakeContext> clone() const = 0;
};

template <typename Hash>
class HandshakeContextImpl : public HandshakeContext {
 public:
  HandshakeContextImpl(const std::string& hkdfLabelPrefix);

  void appendToTranscript(const Buf& data) override;

  Buf getHandshakeContext() const override;

  Buf getFinishedData(folly::ByteRange baseKey) const override;

  folly::ByteRange getBlankContext() const override {
    return Hash::BlankHash;
  }

  virtual std::unique_ptr<HandshakeContext> clone() const override {
    auto newCtx = std::make_unique<HandshakeContextImpl>(hkdfLabelPrefix_);
    newCtx->hashState_ = hashState_;
    return newCtx;
  }

 private:
  Hash hashState_;
  std::string hkdfLabelPrefix_;
};
} // namespace fizz

#include <fizz/protocol/HandshakeContext-inl.h>
