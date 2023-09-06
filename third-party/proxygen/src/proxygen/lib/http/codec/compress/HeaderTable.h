/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <list>
#include <string>
#include <vector>

#include <folly/container/F14Map.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>

namespace proxygen {

/**
 * Data structure for maintaining indexed headers, based on a fixed-length ring
 * with FIFO semantics. Externally it acts as an array.
 */

class HeaderTable {
 public:
  // TODO: std::vector might be faster than std::list in the use case?
  using names_map = folly::F14FastMap<HPACKHeaderName, std::list<uint32_t>>;

  explicit HeaderTable(uint32_t capacityVal) {
    init(capacityVal);
  }

  virtual ~HeaderTable() {
  }
  HeaderTable(const HeaderTable&) = delete;
  HeaderTable& operator=(const HeaderTable&) = delete;

  /**
   * Return Insert Count - the total number of headers inserted to this table,
   * including evictions
   */
  uint32_t getInsertCount() const {
    return insertCount_;
  }

  void disableNamesIndex() {
    indexNames_ = false;
  }

  /**
   * Add the header entry at the beginning of the table (index=1)
   *
   * @return true if it was able to add the entry
   */
  virtual bool add(HPACKHeader header);

  /**
   * Get the index of the given header, if found, and the index of a header
   * with the same name if not.
   *
   * @return a pair containing <index, 0>, <0, nameIndex>, or <0, 0>
   */
  [[nodiscard]] std::pair<uint32_t, uint32_t> getIndex(
      const HPACKHeader& header) const;

  /**
   * Get the index of the given header, if found, and the index of a header
   * with the same name if not.
   *
   * @return a pair containing <index, 0>, <0, nameIndex>, or <0, 0>
   */
  [[nodiscard]] std::pair<uint32_t, uint32_t> getIndex(
      const HPACKHeaderName& name, folly::StringPiece value) const;

  /**
   * Get the table entry at the given external index.
   *
   * @return the header entry
   */
  const HPACKHeader& getHeader(uint32_t index) const;

  /**
   * Checks if an external index is valid.
   */
  bool isValid(uint32_t index) const;

  /**
   * @return true if there is at least one header with the given name
   */
  bool hasName(const HPACKHeaderName& headerName);

  /**
   * @return the map holding the indexed names
   */
  const names_map& names() const {
    return names_;
  }

  /**
   * Get any index of a header that has the given name. From all the
   * headers with the given name we pick the last one added to the header
   * table, but the way we pick the header can be arbitrary.
   */
  uint32_t nameIndex(const HPACKHeaderName& headerName) const;

  /**
   * Table capacity, or maximum number of bytes we can hold.
   */
  uint32_t capacity() const {
    return capacity_;
  }

  /**
   * Returns the maximum table length required to support HPACK headers given
   * the specified capacity bytes
   */
  uint32_t getMaxTableLength(uint32_t capacityVal) const;

  /**
   * Sets the current capacity of the header table, and evicts entries
   * if needed.  Returns false if eviction failed.
   */
  virtual bool setCapacity(uint32_t capacity);

  /**
   * @return number of valid entries
   */
  uint32_t size() const {
    return size_;
  }

  /**
   * @return size in bytes, the sum of the size of all entries
   */
  uint32_t bytes() const {
    return bytes_;
  }

  /**
   * @return how many slots we have in the table
   */
  size_t length() const {
    return table_.size();
  }

  bool operator==(const HeaderTable& other) const;

  /**
   * Static versions of the methods that translate indices.
   */
  static uint32_t toExternal(uint32_t head,
                             uint32_t length,
                             uint32_t internalIndex);

  static uint32_t toInternal(uint32_t head,
                             uint32_t length,
                             uint32_t externalIndex);

 protected:
  /**
   * Initialize with a given capacity.
   */
  void init(uint32_t capacityVal);

  /*
   * Increase table length to newLength
   */
  virtual void increaseTableLengthTo(uint32_t newLength);

  virtual void resizeTable(uint32_t newLength);

  virtual void updateResizedTable(uint32_t oldTail,
                                  uint32_t oldLength,
                                  uint32_t newLength);

  /**
   * Removes one header entry from the beginning of the header table.
   *
   * Returns the size of the removed header
   */
  virtual uint32_t removeLast();

  /**
   * Empties the underlying header table
   */
  void reset();

  /**
   * Evict entries to make space for the needed amount of bytes.
   */
  virtual uint32_t evict(uint32_t needed, uint32_t desiredCapacity);

  /**
   * Move the index to the right.
   */
  uint32_t next(uint32_t i) const;

  /**
   * Get the index of the tail element of the table.
   */
  uint32_t tail() const;

  /**
   * Translate internal index to external one, including a static version.
   */
  uint32_t toExternal(uint32_t internalIndex) const;

  /**
   * Translate external index to internal one.
   */
  uint32_t toInternal(uint32_t externalIndex) const;

  uint32_t capacity_{0};
  uint32_t bytes_{0}; // size in bytes of the current entries
  std::vector<HPACKHeader> table_;

  uint32_t size_{0}; // how many entries we have in the table
  uint32_t head_{0}; // points to the first element of the ring
  uint32_t insertCount_{0};
  bool indexNames_{true};

  names_map names_;

 private:
  /*
   * Shared implementation for getIndex and nameIndex
   */
  [[nodiscard]] std::pair<uint32_t, uint32_t> getIndexImpl(
      const HPACKHeaderName& header,
      folly::StringPiece value,
      bool nameOnly) const;

  uint32_t initialTableLength(uint32_t capacity);
};

std::ostream& operator<<(std::ostream& os, const HeaderTable& table);

} // namespace proxygen
