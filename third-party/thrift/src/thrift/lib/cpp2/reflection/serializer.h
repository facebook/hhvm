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

#include <memory>
#include <type_traits>

#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/Traits.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

namespace apache {
namespace thrift {
namespace detail {

// is_smart_pointer is a helper for determining if a type is a supported
// pointer type for cpp2.ref fields, while discrimiminating against the
// pointer corner case in Thrift (e.g., a unqiue_pointer<folly::IOBuf>)
template <typename>
struct is_smart_pointer : std::false_type {};
template <typename D>
struct is_smart_pointer<std::unique_ptr<folly::IOBuf, D>> : std::false_type {};

// supported smart pointer types for cpp2.ref_type fields
template <typename T, typename D>
struct is_smart_pointer<std::unique_ptr<T, D>> : std::true_type {};
template <typename T>
struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

template <typename T>
using enable_if_smart_pointer =
    typename std::enable_if<is_smart_pointer<T>::value>::type;

template <typename T>
using disable_if_smart_pointer =
    typename std::enable_if<!is_smart_pointer<T>::value>::type;

} /* namespace detail */

namespace detail {

// helper predicate for determining if a struct's MemberInfo is required
// to be read out of the protocol
struct is_required_field {
  template <typename MemberInfo>
  using apply = std::integral_constant<
      bool,
      MemberInfo::optional::value == optionality::required>;
};

struct extract_descriptor_fid {
  template <typename T>
  using apply = typename T::metadata::id;
};

template <typename T, typename Enable = void>
struct deref;

// General case: methods on deref are no-op, returning their input
template <typename T>
struct deref<T, disable_if_smart_pointer<T>> {
  static T& clear_and_get(T& in) { return in; }
  static T const& get_const(T const& in) { return in; }
};

// Special case: We specifically *do not* dereference a unique pointer to
// an IOBuf, because this is a type that the protocol can (de)serialize
// directly
template <>
struct deref<std::unique_ptr<folly::IOBuf>> {
  using T = std::unique_ptr<folly::IOBuf>;
  static T& clear_and_get(T& in) { return in; }
  static T const& get_const(T const& in) { return in; }
};

// General case: deref returns a reference to what the
// unique pointer contains
template <typename PtrType>
struct deref<PtrType, enable_if_smart_pointer<PtrType>> {
  using T = typename std::remove_const<typename PtrType::element_type>::type;
  static T& clear_and_get(std::shared_ptr<const T>& in) {
    auto t = std::make_shared<T>();
    auto ret = t.get();
    in = std::move(t);
    return *ret;
  }
  static T& clear_and_get(std::shared_ptr<T>& in) {
    in = std::make_shared<T>();
    return *in;
  }
  static T& clear_and_get(std::unique_ptr<T>& in) {
    in = std::make_unique<T>();
    return *in;
  }
  static T const& get_const(const PtrType& in) { return *in; }
};

} // namespace detail

/**
 * Entrypoints for using new serialization methods
 *
 * // C++
 * MyStruct a;
 * MyUnion b;
 * CompactProtocolReader reader;
 * CompactProtocolReader writer;
 *
 * serializer_read(a, reader);
 * serializer_write(b, writer);
 *
 * @author: Dylan Knutson <dymk@fb.com>
 */

template <typename Type, typename Protocol>
std::size_t serializer_read(Type& out, Protocol& protocol) {
  using TypeClass = type_class_of_thrift_class_t<Type>;
  auto xferStart = protocol.getCursorPosition();
  apache::thrift::detail::pm::protocol_methods<TypeClass, Type>::read(
      protocol, out);
  return protocol.getCursorPosition() - xferStart;
}

template <typename Type, typename Protocol>
std::size_t serializer_write(const Type& in, Protocol& protocol) {
  using TypeClass = type_class_of_thrift_class_t<Type>;
  return apache::thrift::detail::pm::protocol_methods<TypeClass, Type>::write(
      protocol, in);
}

template <typename Type, typename Protocol>
std::size_t serializer_serialized_size(const Type& in, Protocol& protocol) {
  using TypeClass = type_class_of_thrift_class_t<Type>;
  return apache::thrift::detail::pm::protocol_methods<TypeClass, Type>::
      template serializedSize<false>(protocol, in);
}

template <typename Type, typename Protocol>
std::size_t serializer_serialized_size_zc(const Type& in, Protocol& protocol) {
  using TypeClass = type_class_of_thrift_class_t<Type>;
  return apache::thrift::detail::pm::protocol_methods<TypeClass, Type>::
      template serializedSize<true>(protocol, in);
}

} // namespace thrift
} // namespace apache
