/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKContext.h>

namespace proxygen {

HPACKContext::HPACKContext(uint32_t tableSize) : table_(tableSize) {
}

uint32_t HPACKContext::getIndex(const HPACKHeader& header) const {
  return getIndex(header.name, header.value);
}

uint32_t HPACKContext::getIndex(const HPACKHeaderName& name,
                                folly::StringPiece value) const {
  // First consult the static header table if applicable
  // Applicability is determined by the following guided optimizations:
  // 1) The set of CommonHeaders includes all StaticTable headers and so we can
  // quickly conclude that we need not check the StaticTable
  // for non-CommonHeaders
  // 2) The StaticTable only contains non empty values for a very small subset
  // of header names.  As getIndex is only meaingful if both name and value
  // match, we know that if our header has a value and is not part of the very
  // small subset of header names, there is no point consulting the StaticTable
  bool consultStaticTable = false;
  if (value.empty()) {
    // For uncommon static names and empty values, we will send them as a
    // literal with static name reference.  See uncommon list in
    // HPACKContextTests - StaticTableHeaderNamesAreCommon
    consultStaticTable = name.isCommonHeader();
  } else {
    consultStaticTable =
        StaticHeaderTable::isHeaderCodeInTableWithNonEmptyValue(
            name.getHeaderCode());
  }
  if (consultStaticTable) {
    uint32_t staticIndex = getStaticTable().getIndex(name, value);
    if (staticIndex) {
      staticRefs_++;
      return staticToGlobalIndex(staticIndex);
    }
  }

  // Else check the dynamic table
  uint32_t dynamicIndex = table_.getIndex(name, value);
  if (dynamicIndex) {
    return dynamicToGlobalIndex(dynamicIndex);
  } else {
    return dynamicIndex;
  }
}

uint32_t HPACKContext::nameIndex(const HPACKHeaderName& headerName) const {
  uint32_t index = getStaticTable().nameIndex(headerName);
  if (index) {
    staticRefs_++;
    return staticToGlobalIndex(index);
  }
  index = table_.nameIndex(headerName);
  if (index) {
    return dynamicToGlobalIndex(index);
  }
  return 0;
}

bool HPACKContext::isStatic(uint32_t index) const {
  return index <= getStaticTable().size();
}

const HPACKHeader& HPACKContext::getHeader(uint32_t index) {
  if (isStatic(index)) {
    staticRefs_++;
    return getStaticTable().getHeader(globalToStaticIndex(index));
  }
  return table_.getHeader(globalToDynamicIndex(index));
}

void HPACKContext::seedHeaderTable(std::vector<HPACKHeader>& headers) {
  for (auto& header : headers) {
    CHECK(table_.add(std::move(header)));
  }
}

void HPACKContext::describe(std::ostream& os) const {
  os << table_;
}

std::ostream& operator<<(std::ostream& os, const HPACKContext& context) {
  context.describe(os);
  return os;
}

} // namespace proxygen
