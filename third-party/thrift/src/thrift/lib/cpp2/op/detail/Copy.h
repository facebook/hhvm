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

#include <optional>

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/op/detail/Create.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename Src, typename Dst>
void assertSameType() {
  static_assert(
      std::is_same_v<folly::remove_cvref_t<Src>, folly::remove_cvref_t<Dst>>,
      "src and dst have different types.");
}

template <typename Src, typename Dst, typename SrcRef, typename DstRef>
void copy_field_ref(SrcRef src_ref, DstRef dst_ref) {
  assertSameType<Src, Dst>();
  dst_ref.copy_from(src_ref);
}

// TODO: support adapted field, smart pointers with custom allocators, and union
// We only support l-value reference for dst.
struct Copy {
  template <typename Src, typename Dst>
  void operator()(field_ref<Src> src_ref, field_ref<Dst&> dst_ref) const {
    copy_field_ref<Src, Dst&>(src_ref, dst_ref);
  }
  template <typename Src, typename Dst>
  void operator()(
      required_field_ref<Src> src_ref, required_field_ref<Dst&> dst_ref) const {
    copy_field_ref<Src, Dst&>(src_ref, dst_ref);
  }
  template <typename Src, typename Dst>
  void operator()(
      optional_field_ref<Src> src_ref, optional_field_ref<Dst&> dst_ref) const {
    copy_field_ref<Src, Dst&>(src_ref, dst_ref);
  }
  template <typename Src, typename Dst>
  void operator()(
      optional_boxed_field_ref<Src> src_ref,
      optional_boxed_field_ref<Dst&> dst_ref) const {
    copy_field_ref<Src, Dst&>(src_ref, dst_ref);
  }
  template <typename Src, typename Dst>
  void operator()(
      terse_field_ref<Src> src_ref, terse_field_ref<Dst&> dst_ref) const {
    copy_field_ref<Src, Dst&>(src_ref, dst_ref);
  }

  template <typename T>
  void operator()(
      const std::optional<T>& src_opt, std::optional<T>& dst_opt) const {
    dst_opt = src_opt;
  }

  // Copy on unique pointer constructs a new ptr if src is not nullptr.
  // Copy on shared pointer just shares the same ptr.
  // This is consistent with the copy constructor of thrift struct.
  template <typename T>
  void operator()(
      const std::unique_ptr<T>& src_ptr, std::unique_ptr<T>& dst_ptr) const {
    dst_ptr = !src_ptr ? nullptr : std::make_unique<T>(*src_ptr);
  }

  template <typename T>
  void operator()(
      const std::shared_ptr<T>& src_ptr, std::shared_ptr<T>& dst_ptr) const {
    dst_ptr = src_ptr;
  }
};
} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
