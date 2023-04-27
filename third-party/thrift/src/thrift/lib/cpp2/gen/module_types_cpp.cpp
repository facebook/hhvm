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

namespace apache {
namespace thrift {
namespace detail {

namespace st {

template struct enum_find<int>;

static_assert(
    folly::detail::is_instantiation_of_v<
        folly::F14FastMap,
        enum_find<int>::find_name_map_t>,
    "mismatch");
static_assert(
    folly::detail::is_instantiation_of_v<
        folly::F14FastMap,
        enum_find<int>::find_value_map_t>,
    "mismatch");
static_assert(
    !FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE || !folly::has_extended_alignment ||
        sizeof(enum_find<int>) <= folly::cacheline_align_v,
    "oversized");

FOLLY_NOINLINE void translate_field_name(
    folly::StringPiece fname,
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

} // namespace st

} // namespace detail
} // namespace thrift
} // namespace apache
