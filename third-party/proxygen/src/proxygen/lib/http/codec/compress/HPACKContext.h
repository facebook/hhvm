/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/HeaderTable.h>
#include <proxygen/lib/http/codec/compress/StaticHeaderTable.h>

namespace proxygen {

class HPACKContext {
 public:
  explicit HPACKContext(uint32_t tableSize);
  ~HPACKContext() {
  }

  /**
   * get the index of the given header by looking into both dynamic and static
   * header table
   *
   * @return 0 if cannot be found
   */
  std::pair<uint32_t, uint32_t> getIndex(const HPACKHeader& header) const;

  std::pair<uint32_t, uint32_t> getIndex(const HPACKHeaderName& name,
                                         folly::StringPiece value,
                                         bool checkDynamicTable = true) const;

  /**
   * index of a header entry with the given name from dynamic or static table
   *
   * @return 0 if name not found
   */
  uint32_t nameIndex(const HPACKHeaderName& headerName) const;

  /**
   * @return true if the given index points to a static header entry
   */
  bool isStatic(uint32_t index) const;

  /**
   * @return header at the given index by composing dynamic and static tables
   */
  const HPACKHeader& getHeader(uint32_t index);

  const HeaderTable& getTable() const {
    return table_;
  }

  uint32_t getTableSize() const {
    return table_.capacity();
  }

  uint32_t getBytesStored() const {
    return table_.bytes();
  }

  uint32_t getHeadersStored() const {
    return table_.size();
  }

  void seedHeaderTable(std::vector<HPACKHeader>& headers);

  void describe(std::ostream& os) const;

  uint32_t getStaticRefs() const {
    return staticRefs_;
  }

  uint32_t getInsertCount() const {
    return table_.getInsertCount();
  }

 protected:
  const StaticHeaderTable& getStaticTable() const {
    return StaticHeaderTable::get();
  }

  uint32_t globalToDynamicIndex(uint32_t index) const {
    return index - getStaticTable().size();
  }
  uint32_t globalToStaticIndex(uint32_t index) const {
    return index;
  }
  uint32_t dynamicToGlobalIndex(uint32_t index) const {
    return index + getStaticTable().size();
  }
  uint32_t staticToGlobalIndex(uint32_t index) const {
    return index;
  }

  HeaderTable table_;
  mutable uint32_t staticRefs_{0};
};

std::ostream& operator<<(std::ostream& os, const HPACKContext& context);

} // namespace proxygen
