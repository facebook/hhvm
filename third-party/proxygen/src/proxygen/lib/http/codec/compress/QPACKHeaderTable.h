/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <limits>

#include <glog/logging.h>

#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <proxygen/lib/http/codec/compress/HeaderTable.h>

namespace proxygen {

/**
 * Data structure for maintaining indexed headers, based on a fixed-length ring
 * with FIFO semantics. Externally it acts as an array.
 */

class QPACKHeaderTable : public HeaderTable {
 public:
  static constexpr uint32_t UNACKED = std::numeric_limits<uint32_t>::max();

  QPACKHeaderTable(uint32_t capacityVal, bool trackReferences);

  ~QPACKHeaderTable() override {
  }
  QPACKHeaderTable(const QPACKHeaderTable&) = delete;
  QPACKHeaderTable& operator=(const QPACKHeaderTable&) = delete;

  /**
   * Returns true if the absolute index has not been ack'ed yet.
   */
  bool isVulnerable(uint32_t absIndex) const {
    return (absIndex > ackedInsertCount_);
  }

  /**
   * Returns true if the header can be added to the table.  May be linear
   * in the number of entries
   */
  bool canIndex(const HPACKHeaderName& name, folly::StringPiece value) {
    auto headerBytes = HPACKHeader::bytes(name.size(), value.size());
    auto totalBytes = bytes_ + headerBytes;
    // Don't index headers that would immediately be drained
    return ((headerBytes <= (capacity_ - minFree_)) &&
            (totalBytes <= capacity_ || canEvict(totalBytes - capacity_)));
  }

  /**
   * Returns true if the index should not be used so table space can be freed
   */
  bool isDraining(uint32_t relativeIndex) {
    return relativeToAbsolute(relativeIndex) < minUsable_;
  }

  /**
   * Returns the absolute index for a reference to the header at relativeIndex,
   * along with a boolean indicating if the returned index is a duplicate.  It
   * may return 0 if the entry at relativeIndex was draining and could not be
   * duplicated, or vulnerable references are not allowed.
   */
  std::pair<bool, uint32_t> maybeDuplicate(uint32_t relativeIndex,
                                           bool allowVulnerable);

  /**
   * Add the header entry at the beginning of the table (index=1)
   *
   * @return true if it was able to add the entry
   */
  bool add(HPACKHeader header) override;

  bool setCapacity(uint32_t capacity) override;

  // This API is only for tests, and doesn't work correctly if the table is
  // already populated.
  void setMinFreeForTesting(uint32_t minFree) {
    minFree_ = minFree;
  }

  /**
   * Get the index of the given header, if found.  The index is relative to
   * head/insertCount.  If allowVulnerable is true, the index returned may not
   * have been acknowledged by the decoder.
   *
   * @return 0 in case the header is not found
   */
  uint32_t getIndex(const HPACKHeader& header,
                    bool allowVulnerable = true) const;

  uint32_t getIndex(const HPACKHeaderName& name,
                    folly::StringPiece value,
                    bool allowVulnerable = true) const;

  /**
   * Get the table entry at the given external index.  If base is 0,
   * index is relative to head/insertCount.  If base is non-zero, index is
   * relative to base.
   *
   * @return the header entry
   */
  const HPACKHeader& getHeader(uint32_t index, uint32_t base = 0) const;

  /**
   * Checks if an external index is valid.  If base is 0,
   * index is relative to head/insertCount.  If base is non-zero, index is
   * relative to base.
   */
  bool isValid(uint32_t index, uint32_t base = 0) const;

  /**
   * Get any index of a header that has the given name. From all the
   * headers with the given name we pick the last one added to the header
   * table, but the way we pick the header can be arbitrary.
   *
   * See getIndex for a description of base/allowVulnerable
   */
  uint32_t nameIndex(const HPACKHeaderName& headerName,
                     bool allowVulnerable = true) const;

  bool onInsertCountIncrement(uint32_t numInserts) {
    // compare this way to avoid overflow
    if (numInserts > insertCount_ ||
        ackedInsertCount_ > insertCount_ - numInserts) {
      LOG(ERROR)
          << "Decoder ack'd too much: ackedInsertCount_=" << ackedInsertCount_
          << " insertCount_=" << insertCount_ << " numInserts=" << numInserts;
      return false;
    }
    ackedInsertCount_ += numInserts;
    CHECK_LE(ackedInsertCount_, insertCount_);
    return true;
  }

  void setAcknowledgedInsertCount(uint32_t ackInsertCount) {
    if (ackInsertCount < ackedInsertCount_) {
      return;
    }
    CHECK_LE(ackInsertCount, insertCount_);
    ackedInsertCount_ = ackInsertCount;
  }

  /**
   * Convert a relative index to an absolute index
   */
  uint32_t relativeToAbsolute(uint32_t relativeIndex) const {
    DCHECK(isValid(relativeIndex, 0));
    return insertCount_ - relativeIndex + 1;
  }

  /**
   * Convert an absolute index to a relative index
   */
  uint32_t absoluteToRelative(uint32_t absIndex) const {
    CHECK_LE(absIndex, insertCount_);
    return insertCount_ - absIndex + 1;
  }

  /**
   * Add a reference for the given index.  Entries with non-zero references
   * cannot be evicted
   */
  void addRef(uint32_t absIndex);

  /**
   * Subtract a reference for the given index
   */
  void subRef(uint32_t absIndex);

 private:
  /*
   * Shared implementation for getIndex and nameIndex
   */
  uint32_t getIndexImpl(const HPACKHeaderName& header,
                        folly::StringPiece value,
                        bool nameOnly,
                        bool allowVulnerable = true) const;

  /*
   * Increase table length to newLength
   */
  void increaseTableLengthTo(uint32_t newLength) override;

  void resizeTable(uint32_t newLength) override;

  void updateResizedTable(uint32_t oldTail,
                          uint32_t oldLength,
                          uint32_t newLength) override;
  /**
   * Removes one header entry from the beginning of the header table.
   */
  uint32_t removeLast() override;

  /**
   * Return true if the table can evict needed bytes
   */
  bool canEvict(uint32_t needed);

  /**
   * Evict entries to make space for the needed amount of bytes.
   */
  uint32_t evict(uint32_t needed, uint32_t desiredCapacity) override;

  /**
   * Translate external index to internal one.
   */
  uint32_t toInternal(uint32_t externalIndex, uint32_t base) const;

  uint32_t internalToAbsolute(uint32_t internalIndex) const;
  uint32_t absoluteToInternal(uint32_t absoluteIndex) const;

  uint32_t drainedBytes_{0};
  uint32_t minUsable_{1};
  uint32_t ackedInsertCount_{0};
  uint32_t minFree_{0};
  std::unique_ptr<std::vector<uint16_t>> refCount_;
};

} // namespace proxygen
