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

#pragma once

#include <thrift/lib/cpp2/type/Id.h>

namespace apache::thrift {
namespace type {
// Forward declare to avoid circular dep with thrift/lib/thrift/id.thrift.
enum class FieldId : ::std::int16_t;
} // namespace type

using FieldId = type::FieldId;
template <std::underlying_type_t<FieldId> id>
using field_id = type::field_id_tag<FieldId(id)>;

using FieldOrdinal = type::Ordinal;
template <std::underlying_type_t<FieldOrdinal> ord>
using field_ordinal = type::ordinal_tag<FieldOrdinal(ord)>;

// TODO(dokwon): Change FieldContext to use strong FieldId type.
template <typename Struct, int16_t FieldId>
struct FieldContext {
  static constexpr int16_t kFieldId = FieldId;
  Struct& object;
};

} // namespace apache::thrift
