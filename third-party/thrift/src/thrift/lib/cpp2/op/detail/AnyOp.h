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

#include <iterator>
#include <stdexcept>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/op/detail/BaseOp.h>
#include <thrift/lib/cpp2/op/detail/ContainerOp.h>
#include <thrift/lib/cpp2/op/detail/StructOp.h>
#include <thrift/lib/cpp2/op/detail/ValueOp.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::op::detail {

// Compile-time and type-erased Thrift operator implementations.
template <typename Tag, typename>
struct AnyOp : BaseOp<Tag> {
  static_assert(type::is_concrete_v<Tag>);
  using Base = BaseOp<Tag>;

  // TODO(afuller): Implement all Tags and remove runtime throwing fallback.
  using Base::unimplemented;
  [[noreturn]] static void append(void*, const Dyn&) { unimplemented(); }
  [[noreturn]] static bool add(void*, const Dyn&) { unimplemented(); }
  [[noreturn]] static bool put(void*, FieldId, size_t, const Dyn&, const Dyn&) {
    unimplemented();
  }
  [[noreturn]] static Ptr ensure(void*, FieldId, const Dyn&, const Dyn&) {
    unimplemented();
  }
  [[noreturn]] static Ptr get(void*, FieldId, size_t, const Dyn&) {
    unimplemented();
  }
  [[noreturn]] static size_t size(const void*) { unimplemented(); }
};

// Create a AnyOp-based Thrift type info.
template <typename Tag>
const TypeInfo& getAnyTypeInfo() {
  return type::detail::getTypeInfo<AnyOp<Tag>, Tag>();
}

} // namespace apache::thrift::op::detail
