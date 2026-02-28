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

#include <thrift/lib/cpp2/protocol/detail/FieldMaskUtil.h>

namespace apache::thrift::protocol::detail {

void ensure_fields(MaskRef ref, type::AnyStruct& t) {
  if (ref.isTypeMask()) {
    folly::throw_exception<std::runtime_error>(
        "Cannot ensure Any field (schemaless ensure is unsafe)");
  }

  // backwards compatiblity with field mask
  return ensure_fields<type::AnyStruct>(ref, t);
}

void clear_fields(MaskRef ref, type::AnyStruct& t) {
  if (ref.isFieldMask()) {
    // Retain field-mask support for backwards compatibility
    clear_fields<type::AnyStruct>(ref, t);
    return;
  }

  if (!ref.isTypeMask()) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible");
  }

  // The type of Any could be a hashed-URI so use getViaIdenticalType
  auto nested = ref.getViaIdenticalType_INTERNAL_DO_NOT_USE(*t.type());
  if (!nested.isNoneMask()) {
    auto val = parseValueFromAny(t);
    nested.clear(val);
    t = toAny(val, *t.type(), *t.protocol()).toThrift();
  }
}

bool filter_fields(
    MaskRef ref, const type::AnyStruct& t, type::AnyStruct& ret) {
  if (ref.isFieldMask()) {
    // Retain field-mask support for backwards compatibility
    return filter_fields<type::AnyStruct>(ref, t, ret);
  }

  if (!ref.isTypeMask()) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible");
  }

  // The type of Any could be a hashed-URI so use getViaIdenticalType
  auto nested = ref.getViaIdenticalType_INTERNAL_DO_NOT_USE(*t.type());
  if (nested.isNoneMask()) {
    return false;
  }

  // recurse
  ret = toAny(nested.filter(parseValueFromAny(t)), *t.type(), *t.protocol())
            .toThrift();
  return true;
}

Mask fieldPathToMask(const FieldPath& path, const Mask& other) {
  if (path.empty()) {
    folly::throw_exception<std::runtime_error>("Empty field path.");
  }
  Mask mask;
  Mask* m = &mask;
  for (auto id : path) {
    m = &m->includes().emplace()[static_cast<int16_t>(id)];
  }
  *m = other;
  return mask;
}

} // namespace apache::thrift::protocol::detail
