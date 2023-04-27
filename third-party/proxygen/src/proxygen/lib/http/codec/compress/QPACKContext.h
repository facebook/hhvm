/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/QPACKHeaderTable.h>
#include <proxygen/lib/http/codec/compress/QPACKStaticHeaderTable.h>

namespace proxygen {

class QPACKContext {
 public:
  QPACKContext(uint32_t tableSize, bool trackReferences);
  ~QPACKContext() {
  }

  /**
   * @return header at the given index by composing dynamic and static tables
   */
  const HPACKHeader& getHeader(bool isStatic,
                               uint32_t index,
                               uint32_t base,
                               bool aboveBase);

  const QPACKHeaderTable& getTable() const {
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

  uint32_t getInsertCount() const {
    return table_.getInsertCount();
  }

  uint32_t getBlockedInserts() const {
    return blockedInsertions_;
  }

  uint32_t getDuplications() const {
    return duplications_;
  }

  uint32_t getStaticRefs() const {
    return staticRefs_;
  }

  void seedHeaderTable(std::vector<HPACKHeader>& headers);

  void describe(std::ostream& os) const;

 protected:
  static uint32_t getMaxEntries(uint32_t tableSize) {
    return tableSize / HPACKHeader::kMinLength;
  }

  const StaticHeaderTable& getStaticTable() const {
    return QPACKStaticHeaderTable::get();
  }

  QPACKHeaderTable table_;
  uint32_t blockedInsertions_{0};
  uint32_t duplications_{0};
  uint32_t staticRefs_{0};
};

std::ostream& operator<<(std::ostream& os, const QPACKContext& context);

} // namespace proxygen
