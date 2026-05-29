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

namespace apache::thrift::detail::st {

FOLLY_NOINLINE void translate_field_name(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_table& table) noexcept {
  for (size_t i = 0; i < table.size; ++i) {
    if (fname == table.names[i]) {
      fid = table.ids[i];
      ftype = table.types[i];
      break;
    }
  }
}

translate_field_name_hash_table::translate_field_name_hash_table(
    size_t size,
    const std::string_view* names,
    const int16_t* ids,
    const protocol::TType* types) {
  map.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    map.emplace(names[i], std::pair(ids[i], types[i]));
  }
}

void translate_field_name(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_hash_table& table) noexcept {
  auto it = table.map.find(fname);
  if (it != table.map.end()) {
    fid = it->second.first;
    ftype = it->second.second;
  }
}

} // namespace apache::thrift::detail::st
