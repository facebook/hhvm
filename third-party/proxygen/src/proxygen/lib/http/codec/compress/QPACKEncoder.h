/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <list>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoderBase.h>
#include <proxygen/lib/http/codec/compress/QPACKContext.h>
#include <set>
#include <unordered_map>
#include <vector>

namespace proxygen {

class HPACKDecodeBuffer;

class QPACKEncoder
    : public HPACKEncoderBase
    , public QPACKContext {

 public:
  static constexpr uint32_t kMaxHeaderTableSize = (1u << 16);
  static constexpr uint32_t kDefaultMaxOutstandingListSize = (1u << 8);

  explicit QPACKEncoder(bool huffman, uint32_t tableSize = HPACK::kTableSize);

  /**
   * Encode the given headers.
   */

  using Buf = std::unique_ptr<folly::IOBuf>;
  struct EncodeResult {
    EncodeResult(Buf c, Buf s) : control(std::move(c)), stream(std::move(s)) {
    }
    Buf control;
    Buf stream;
  };

  // Returns a pair of buffers.  One for the control stream and one for the
  // request stream
  EncodeResult encode(
      const std::vector<HPACKHeader>& headers,
      uint32_t headroom,
      uint64_t streamId,
      uint32_t maxEncoderStreamBytes = std::numeric_limits<uint32_t>::max());

  HPACK::DecodeError decodeDecoderStream(std::unique_ptr<folly::IOBuf> buf);

  HPACK::DecodeError decoderStreamEnd();

  HPACK::DecodeError onInsertCountIncrement(uint32_t inserts);

  HPACK::DecodeError onHeaderAck(uint64_t streamId, bool all);

  bool setHeaderTableSize(uint32_t tableSize, bool updateMax = true) {
    if (updateMax) {
      // The peer's max is used whenn encoding RequiredInsertCouunt
      if (maxTableSize_ != 0 && maxTableSize_ != tableSize) {
        LOG(ERROR) << "Cannot change non-zero max header table size, "
                      "maxTableSize_="
                   << maxTableSize_ << " tableSize=" << tableSize;
        return false;
      }
      maxTableSize_ = tableSize;
    }
    if (tableSize > kMaxHeaderTableSize) {
      VLOG(2) << "Limiting table size from " << tableSize << " to "
              << kMaxHeaderTableSize;
      tableSize = kMaxHeaderTableSize;
    }
    HPACKEncoderBase::setHeaderTableSize(table_, tableSize);
    return true;
  }

  uint32_t getMaxHeaderTableSize() const {
    return maxTableSize_;
  }

  void setMaxVulnerable(uint32_t maxVulnerable) {
    maxVulnerable_ = maxVulnerable;
  }

  // This API is only for tests, and doesn't work correctly if the table is
  // already populated.
  void setMinFreeForTesting(uint32_t minFree) {
    table_.setMinFreeForTesting(minFree);
  }

  void setMaxNumOutstandingBlocks(uint32_t value);

  uint32_t startEncode(folly::IOBufQueue& controlQueue,
                       uint32_t headroom,
                       uint32_t maxEncoderStreamBytes);

  size_t encodeHeaderQ(HPACKHeaderName name,
                       folly::StringPiece value,
                       uint32_t baseIndex,
                       uint32_t& requiredInsertCount);

  std::unique_ptr<folly::IOBuf> completeEncode(uint64_t streamId,
                                               uint32_t baseIndex,
                                               uint32_t requiredInsertCount);

 private:
  bool allowVulnerable() const {
    return numVulnerable_ < maxVulnerable_;
  }

  bool shouldIndex(const HPACKHeaderName& name, folly::StringPiece value) const;

  bool dynamicReferenceAllowed() const;

  void encodeControl(const HPACKHeader& header);

  std::pair<bool, uint32_t> maybeDuplicate(uint32_t relativeIndex);

  std::tuple<bool, uint32_t, uint32_t> getNameIndexQ(
      const HPACKHeaderName& headerName);

  size_t encodeStreamLiteralQ(const HPACKHeaderName& name,
                              folly::StringPiece value,
                              bool isStaticName,
                              uint32_t nameIndex,
                              uint32_t absoluteNameIndex,
                              uint32_t baseIndex,
                              uint32_t& requiredInsertCount);

  void encodeInsertQ(const HPACKHeaderName& name,
                     folly::StringPiece value,
                     bool isStaticName,
                     uint32_t nameIndex);

  size_t encodeLiteralQ(const HPACKHeaderName& name,
                        folly::StringPiece value,
                        bool isStaticName,
                        bool postBase,
                        uint32_t nameIndex,
                        const HPACK::Instruction& idxInstr);

  uint32_t encodeLiteralQHelper(HPACKEncodeBuffer& buffer,
                                const HPACKHeaderName& name,
                                folly::StringPiece value,
                                bool isStaticName,
                                uint32_t nameIndex,
                                uint8_t staticFlag,
                                const HPACK::Instruction& idxInstr,
                                const HPACK::Instruction& litInstr);

  void trackReference(uint32_t index, uint32_t& requiredInsertCount);

  void encodeDuplicate(uint32_t index);

  HPACK::DecodeError decodeHeaderAck(HPACKDecodeBuffer& dbuf,
                                     uint8_t prefixLength,
                                     bool all);

  // Returns true if the most recently encoded value (duplicate, insert)
  // fit in the encoder stream's flow control window.  The encoder will only
  // make references to dynamic table entries that fit.  This prevents a nasty
  // deadlock.
  bool lastEntryAvailable() const {
    return maxEncoderStreamBytes_ >= 0;
  }

  HPACKEncodeBuffer controlBuffer_;
  using BlockReferences = std::set<uint32_t>;
  struct OutstandingBlock {
    BlockReferences references;
    bool vulnerable{false};
  };
  // Map streamID -> list of table index references for each outstanding block;
  std::unordered_map<uint64_t, std::list<OutstandingBlock>> outstanding_;
  OutstandingBlock curOutstanding_;
  uint32_t maxDepends_{0};
  uint32_t maxVulnerable_{HPACK::kDefaultBlocking};
  uint32_t numVulnerable_{0};
  uint32_t maxTableSize_{0};
  int64_t maxEncoderStreamBytes_{0};
  folly::IOBufQueue decoderIngress_{folly::IOBufQueue::cacheChainLength()};
  uint32_t numOutstandingBlocks_{0};
  uint32_t maxNumOutstandingBlocks_{kDefaultMaxOutstandingListSize};
};

} // namespace proxygen
