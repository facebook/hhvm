/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoderBase.h>
#include <vector>

namespace proxygen {

class HPACKEncoder
    : public HPACKEncoderBase
    , public HPACKContext {

 public:
  explicit HPACKEncoder(bool huffman, uint32_t tableSize = HPACK::kTableSize)
      : HPACKEncoderBase(huffman), HPACKContext(tableSize) {
  }

  /**
   * Encode the given headers.
   */

  std::unique_ptr<folly::IOBuf> encode(const std::vector<HPACKHeader>& headers,
                                       uint32_t headroom = 0);

  void encode(const std::vector<HPACKHeader>& headers,
              folly::IOBufQueue& writeBuf);

  void startEncode(folly::IOBufQueue& writeBuf);

  size_t encodeHeader(HTTPHeaderCode code, const std::string& value);

  size_t encodeHeader(HTTPHeaderCode code, folly::fbstring&& value);

  size_t encodeHeader(const std::string& name, const std::string& value);

  void completeEncode();

  void setHeaderTableSize(uint32_t size) {
    HPACKEncoderBase::setHeaderTableSize(table_, size);
  }

 private:
  void encodeAsIndex(uint32_t index);

  // movable name and value
  void encodeHeader(HPACKHeaderName&& name, folly::fbstring&& value);

  // movable name
  void encodeHeader(HPACKHeaderName&& name, folly::StringPiece value);

  // neither movable
  void encodeHeader(const HPACKHeaderName& name, folly::StringPiece value);

  // Returns folly::none if the header was encoded, or a nameIndex to use
  // when encoding the literal if not (may be 0)
  folly::Optional<uint32_t> encodeHeaderImpl(const HPACKHeaderName& name,
                                             folly::StringPiece value,
                                             bool& indexable);

  bool encodeAsLiteral(HPACKHeaderName&& name,
                       uint32_t nameIndex,
                       folly::fbstring&& value,
                       bool indexing);

  bool encodeAsLiteral(HPACKHeaderName&& name,
                       uint32_t nameIndex,
                       folly::StringPiece value,
                       bool indexing);

  bool encodeAsLiteral(const HPACKHeaderName& name,
                       uint32_t nameIndex,
                       folly::StringPiece value,
                       bool indexing);

  void encodeAsLiteralImpl(const HPACKHeaderName& name,
                           uint32_t nameIndex,
                           folly::StringPiece value,
                           bool& indexing);

  void encodeLiteral(const HPACKHeaderName& name,
                     folly::StringPiece value,
                     uint32_t nameIndex,
                     const HPACK::Instruction& instruction);
};

} // namespace proxygen
