/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/experimental/crypto/MerkleTree.h>
#include <fizz/record/Types.h>

namespace fizz {

/**
 * Data structure to keep elements of a batch signature for encoding/decoding.
 *
 * Reference: https://datatracker.ietf.org/doc/draft-ietf-tls-batch-signing/
 * Wire format of a batch signature:
 *   opaque Node[Hash.length];
 *   struct {
 *     uint32 index;
 *     Node path<Hash.length..2^16-1>;
 *     opaque root_signature<0..2^16-1>;
 *   } BatchSignature;
 */
class BatchSignature {
 public:
  explicit BatchSignature(MerkleTreePath&& path, Buf signature)
      : path_{std::move(path)}, sig_(std::move(signature)) {}

  size_t getIndex() const {
    return path_.index;
  }

  Buf getPath() const {
    return path_.path->clone();
  }

  Buf getSignature() const {
    return sig_->clone();
  }

  /**
   * Wire decode the signature.
   */
  static BatchSignature decode(folly::io::Cursor& wire) {
    MerkleTreePath path;
    // index, uint32
    detail::read(path.index, wire);
    // path, each element's length is HashLen
    detail::readBuf<uint16_t>(path.path, wire);
    // root_signature
    Buf sig;
    detail::readBuf<uint16_t>(sig, wire);
    return BatchSignature(std::move(path), std::move(sig));
  }

  /**
   * Wire encode the signature.
   */
  Buf encode() {
    auto batchSigLen = sizeof(path_.index) + sizeof(uint16_t) +
        path_.path->computeChainDataLength() + sizeof(uint16_t) +
        sig_->computeChainDataLength();
    auto batchSig = folly::IOBuf::create(batchSigLen);
    folly::io::Appender appender(batchSig.get(), 0);
    // index, uint32
    detail::write(path_.index, appender);
    // path, each element's length is HashLen
    detail::writeBuf<uint16_t>(path_.path, appender);
    // root_signature
    detail::writeBuf<uint16_t>(sig_, appender);
    return batchSig;
  }

  /**
   * Encode the message for signing.
   *
   * https://datatracker.ietf.org/doc/draft-ietf-tls-batch-signing/
   * signature is computed over the concatenation of:
   * - A string that consists of octet 32 (0x20) repeated 64 times.
   * - The context string "TLS batch signature".
   * - A single 0 byte which serves as the separator.
   * - The batch signature algorithm's SignatureScheme code point,
   *   expressed as a big-endian 16-bit integer.  Note this is the code
   *   point of the batch algorithm, not the original base algorithm.
   * - The value at the root of the tree.
   */
  static Buf encodeToBeSigned(Buf rootValue, SignatureScheme scheme) {
    constexpr static std::array<uint8_t, 84> prefix{
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x62, 0x61, 0x74, 0x63,
        0x68, 0x20, 0x73, 0x69, 0x67, 0x6e, 0x61, 0x74, 0x75, 0x72, 0x65, 0x00};
    auto toBeSignedLen = prefix.size() + sizeof(uint16_t) + rootValue->length();
    auto toBeSigned = folly::IOBuf::create(toBeSignedLen);
    folly::io::Appender appender(toBeSigned.get(), 0);
    // octet 32 (0x20) repeated 64 times, context string, single 0 byte
    detail::writeBufWithoutLength(folly::IOBuf::wrapBuffer(prefix), appender);
    // signature scheme code point
    detail::write(static_cast<uint16_t>(scheme), appender);
    // root value of the tree
    detail::writeBufWithoutLength(rootValue, appender);
    return toBeSigned;
  }

 private:
  struct MerkleTreePath path_;
  Buf sig_;
};

} // namespace fizz
