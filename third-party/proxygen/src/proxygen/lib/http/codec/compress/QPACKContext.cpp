/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/QPACKContext.h>

#include <glog/logging.h>

namespace proxygen {

QPACKContext::QPACKContext(uint32_t tableSize, bool trackReferences)
    : table_(tableSize, trackReferences) {
}

const HPACKHeader& QPACKContext::getHeader(bool isStatic,
                                           uint32_t index,
                                           uint32_t base,
                                           bool aboveBase) {
  if (isStatic) {
    staticRefs_++;
    return getStaticTable().getHeader(index);
  }
  if (aboveBase) {
    CHECK_LE(base, std::numeric_limits<uint32_t>::max() - index);
    base += index;
    index = 1;
  }
  return table_.getHeader(index, base);
}

void QPACKContext::seedHeaderTable(std::vector<HPACKHeader>& headers) {
  for (auto& header : headers) {
    CHECK(table_.add(std::move(header)));
  }
}

void QPACKContext::describe(std::ostream& os) const {
  os << table_;
}

std::ostream& operator<<(std::ostream& os, const QPACKContext& context) {
  context.describe(os);
  return os;
}

} // namespace proxygen
