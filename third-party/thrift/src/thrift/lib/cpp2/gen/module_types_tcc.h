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

#include <cstdint>
#include <memory>

#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/Traits.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializer.h>
#include <thrift/lib/cpp2/protocol/detail/index.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

namespace apache::thrift {

class BinaryProtocolReader;
class BinaryProtocolWriter;
class ComapctProtocolReader;
class CompactProtocolWriter;
class SimpleJSONProtocolReader;
class SimpleJSONProtocolWriter;

namespace detail {

template <typename T>
struct TccStructTraits;

template <typename..., class T>
auto make_mutable_smart_ptr(folly::tag_t<std::unique_ptr<T>>) {
  return std::make_unique<T>();
}

template <typename..., class T, class Alloc>
auto make_mutable_smart_ptr(
    folly::tag_t<std::unique_ptr<
        T,
        folly::allocator_delete<
            typename std::allocator_traits<Alloc>::template rebind_alloc<T>>>>,
    const Alloc& alloc) {
  return folly::allocate_unique<T>(alloc);
}

template <typename..., class T>
auto make_mutable_smart_ptr(folly::tag_t<std::shared_ptr<T>>) {
  return std::make_shared<T>();
}

template <typename..., class T>
auto make_mutable_smart_ptr(folly::tag_t<std::shared_ptr<const T>>) {
  return std::make_shared<T>();
}

template <typename..., class T, class Alloc>
auto make_mutable_smart_ptr(
    folly::tag_t<std::shared_ptr<T>>, const Alloc& alloc) {
  return std::allocate_shared<T>(alloc);
}

template <typename..., class T, class Alloc>
auto make_mutable_smart_ptr(
    folly::tag_t<std::shared_ptr<const T>>, const Alloc& alloc) {
  return std::allocate_shared<T>(alloc);
}

template <typename..., class T>
auto make_mutable_smart_ptr(folly::tag_t<boxed_value_ptr<T>>) {
  // Make sure we invoke a constructor that allocates the unique_ptr so that
  // this overload behaves the same as the other ones.
  return boxed_value_ptr<T>(T());
}

template <typename..., class T>
auto make_mutable_smart_ptr(folly::tag_t<boxed_value<T>>) {
  return boxed_value<T>(std::make_unique<T>());
}

template <class T, class... Args>
auto make_mutable_smart_ptr(Args&&... args) {
  return make_mutable_smart_ptr(folly::tag_t<T>(), std::forward<Args>(args)...);
}

// We introduced new version of writeFieldBegin that accepts previous id as
// parameter. We need adapter until we taught this API to all protocols
template <TType type, int16_t id, int16_t prevId, class ProtocolWriter>
FOLLY_ERASE auto writeFieldBegin(
    ProtocolWriter& prot, const char* name, bool previousFieldHasValue)
    -> decltype(prot.writeFieldBegin(name, type, id, prevId)) {
  if (previousFieldHasValue) {
    return prot.writeFieldBegin(name, type, id, prevId);
  } else {
    return prot.writeFieldBegin(name, type, id);
  }
}

template <TType type, int16_t id, int16_t /* prevId */, class ProtocolWriter>
FOLLY_ERASE auto writeFieldBegin(ProtocolWriter& prot, const char* name, ...) {
  return prot.writeFieldBegin(name, type, id);
}

} // namespace detail

} // namespace apache::thrift
