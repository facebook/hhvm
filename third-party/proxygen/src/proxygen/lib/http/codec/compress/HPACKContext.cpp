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

std::pair<uint32_t, uint32_t> HPACKContext::getIndex(
    const HPACKHeader& header) const {
  return getIndex(header.name, header.value);
}

std::pair<uint32_t, uint32_t> HPACKContext::getIndex(
    const HPACKHeaderName& name,
    folly::StringPiece value,
    bool checkDynamicTable) const {
  // First consult the static header table if applicable
  // Applicability is determined by the following guided optimizations:
  // 1) The set of CommonHeaders includes all StaticTable headers and so we can
  // quickly conclude that we need not check the StaticTable
  // for non-CommonHeaders
  // 2) The StaticTable only contains non empty values for a very small subset
  // of header names.  As getIndex is only meaingful if both name and value
  // match, we know that if our header has a value and is not part of the very
  // small subset of header names, there is no point consulting the StaticTable
  bool getIndexOnStaticTable = false;
  if (value.empty()) {
    // For uncommon static names and empty values, we will send them as a
    // literal with static name reference.  See uncommon list in
    // HPACKContextTests - StaticTableHeaderNamesAreCommon
    getIndexOnStaticTable = name.isCommonHeader();
  } else {
    getIndexOnStaticTable =
        StaticHeaderTable::isHeaderCodeInTableWithNonEmptyValue(
            name.getHeaderCode());
  }
  std::pair<uint32_t, uint32_t> staticIndex{0, 0};
  if (getIndexOnStaticTable) {
    staticIndex = getStaticTable().getIndex(name, value);
    if (staticIndex.first) {
      staticRefs_++;
      return {staticToGlobalIndex(staticIndex.first), 0};
    }
  }

  std::pair<uint32_t, uint32_t> dynamicIndex{0, 0};
  if (checkDynamicTable && table_.capacity() > 0) {
    dynamicIndex = table_.getIndex(name, value);
  }
  if (dynamicIndex.first) {
    return {dynamicToGlobalIndex(dynamicIndex.first), 0};
  } else if (staticIndex.second) {
    staticRefs_++;
    return {0, staticToGlobalIndex(staticIndex.second)};
  } else if (name.getHeaderCode() == HTTP_HEADER_DATE) {
    // The name *may* be in the static table and doing a lookup could save a
    // byte.  But the CPU isn't worth it.  Just hack DATE to make
    // RFCExamplesTest pass
    static uint32_t dateIndex =
        getStaticTable().nameIndex(HPACKHeaderName(HTTP_HEADER_DATE));
    staticRefs_++;
    return {0, staticToGlobalIndex(dateIndex)};
  } else if (dynamicIndex.second) {
    return {0, dynamicToGlobalIndex(dynamicIndex.second)};
  } else if (!getIndexOnStaticTable && name.isCommonHeader()) {
    // Maybe the name is in the static table but we didn't look before
    staticIndex.second = getStaticTable().nameIndex(name);
    if (staticIndex.second) {
      staticRefs_++;
      return {0, staticToGlobalIndex(staticIndex.second)};
    }
  }
  return {0, 0};
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
