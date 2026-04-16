/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/gen/module_types_cpp.h>

#include <limits>
#include <thrift/lib/cpp/protocol/TProtocolException.h>

namespace apache::thrift::detail::st {

namespace {
FOLLY_ALWAYS_INLINE bool fieldIdExists(int16_t id) {
  // Users are unlikely to set field id explicitly
  return FOLLY_UNLIKELY(id != std::numeric_limits<int16_t>::min());
}
} // namespace

void checkFieldIdConflict(int16_t expected, int16_t actual) {
  if (fieldIdExists(actual) && actual != expected) {
    TProtocolException::throwInvalidFieldData();
  }
}

FOLLY_NOINLINE void translate_field_name_or_id(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_table& table) {
  if (FOLLY_UNLIKELY(fname.empty())) {
    // Only field ID provided (e.g., "(30)" or "30"). Look up by ID.
    for (size_t i = 0; i < table.size; ++i) {
      if (fid == table.ids[i]) {
        ftype = table.types[i];
        return;
      }
    }
    return;
  }

  // Field name provided. Look up by name.
  for (size_t i = 0; i < table.size; ++i) {
    if (fname == table.names[i]) {
      checkFieldIdConflict(table.ids[i], fid);
      fid = table.ids[i];
      ftype = table.types[i];
      return;
    }
  }

  if (fieldIdExists(fid)) {
    for (size_t i = 0; i < table.size; ++i) {
      if (fid == table.ids[i]) {
        TProtocolException::throwInvalidFieldData();
      }
    }
  }
}

translate_field_name_hash_table::translate_field_name_hash_table(
    size_t size,
    const std::string_view* names,
    const int16_t* ids,
    const protocol::TType* types) {
  map.reserve(size);
  idMap.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    map.emplace(names[i], std::pair(ids[i], types[i]));
    idMap.emplace(ids[i], types[i]);
  }
}

void translate_field_name_or_id(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_hash_table& table) {
  if (FOLLY_UNLIKELY(fname.empty())) {
    // Only field ID provided (e.g., "(30)" or "30"). Look up by ID.
    auto it = table.idMap.find(fid);
    if (it != table.idMap.end()) {
      ftype = it->second;
    }
    return;
  }

  // Field name provided. Look up by name.
  auto it = table.map.find(fname);
  if (it != table.map.end()) {
    checkFieldIdConflict(it->second.first, fid);
    fid = it->second.first;
    ftype = it->second.second;
    return;
  }

  if (fieldIdExists(fid) && table.idMap.contains(fid)) {
    // Id found in local schema but field-name not exist.
    TProtocolException::throwInvalidFieldData();
  }
}

} // namespace apache::thrift::detail::st
