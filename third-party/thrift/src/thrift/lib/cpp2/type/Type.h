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

#include <type_traits>

#include <folly/Optional.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/type_rep_types.h>
#include <thrift/lib/thrift/gen-cpp2/type_rep_types_custom_protocol.h>

namespace apache {
namespace thrift {
namespace type {

// A class that can represent any concrete Thrift type.
//
// Can be constructed directly from ThriftType tags:
//
//   auto listOfStringType1 = Type::get<list<string_t>>();
//
// Or built up from other type values:
//
//   auto stringType = Type::get<string_t>();
//   auto listOfStringType2 = Type::create<list_c>(stringType);
//
// Both of these result in the same type:
//
//   listOfStringType1 == listOfStringType2 -> true
//
class Type : public detail::Wrap<TypeStruct> {
  using Base = detail::Wrap<TypeStruct>;

 public:
  Type() = default; // The 'void' type.
  Type(const Type&) = default;
  Type(Type&&) noexcept = default;

  explicit Type(const TypeStruct& type) : Base(type) {}
  explicit Type(TypeStruct&& type) noexcept : Base(std::move(type)) {}

  template <typename Tag, typename = std::enable_if_t<is_concrete_v<Tag>>>
  /* implicit */ Type(Tag) : Base(makeType<Tag>(Tag{})) {}

  // Named types.
  Type(enum_c, std::string name) : Type{makeNamed<enum_c>(std::move(name))} {}
  Type(struct_c, std::string name)
      : Type{makeNamed<struct_c>(std::move(name))} {}
  Type(union_c, std::string name) : Type{makeNamed<union_c>(std::move(name))} {}
  Type(exception_c, std::string name)
      : Type{makeNamed<exception_c>(std::move(name))} {}

  // Parameterized types.
  Type(list_c, Type val) : Type(makeParamed<list_c>(std::move(val.data_))) {}
  Type(set_c, Type key) : Type(makeParamed<set_c>(std::move(key.data_))) {}
  Type(map_c, Type key, Type val)
      : Type(makeParamed<map_c>(std::move(key.data_), std::move(val.data_))) {}

  // Constructs an Type for the given Thrift type Tag, using the given
  // arguments. If the type Tag is not concrete, the additional parameters must
  // be passed in. For example:
  //
  //   Type::get<list<i32>>() ==
  //       Type::create<list_c>(Type::get<i32>());
  //
  //   //Create the type for an IDL-defined, named struct.
  //   Type::create<struct_c>("mydomain.com/my/package/MyStruct");
  //
  template <typename Tag, typename... Args>
  static Type create(Args&&... args) {
    return {Tag{}, std::forward<Args>(args)...};
  }
  template <typename Tag>
  FOLLY_EXPORT static const Type& get() noexcept {
    static_assert(is_concrete_v<Tag>, "");
    static const Type& kInst = *new Type(Tag{});
    return kInst;
  }
  BaseType baseType() const noexcept {
    return BaseType{data_.name()->getType()};
  }

  Type& operator=(const Type&) = default;
  Type& operator=(Type&&) noexcept = default;

  // If the complete and non-empty type information is present.
  //
  // Specifically, that all contained 'type name' values are not empty.
  bool isFull() const { return isFull(data_); }

  // If the Type information is full, contains correct number of valid
  // parameters and have full, human-readable, Thrift URIs.
  bool isValid() const { return isFull(data_, true, true); }

 private:
  static bool isFull(const TypeUri& typeUri, bool validate_uri);
  static bool isFull(const TypeName& typeName, bool validate_uri);
  static bool isFull(
      const TypeStruct& type,
      bool ensure_params = false,
      bool validate_uri = false);

  friend bool operator==(Type lhs, Type rhs) noexcept {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(Type lhs, Type rhs) noexcept {
    return lhs.data_ != rhs.data_;
  }
  friend bool operator<(Type lhs, Type rhs) noexcept {
    return lhs.data_ < rhs.data_;
  }

  static void checkName(const std::string& name);

  template <typename CTag, typename T>
  static decltype(auto) getName(T&& result) {
    // The field ids are 1:1 with the associated BaseType.
    using Id = field_id_tag<static_cast<FieldId>(base_type_v<CTag>)>;
    using Union = folly::remove_cvref_t<decltype(*result.name())>;
    return op::get<Id, Union>(*std::forward<T>(result).name());
  }

  template <typename Tag>
  static TypeStruct makeConcrete() {
    TypeStruct result;
    getName<Tag>(result).ensure();
    return result;
  }

  template <typename CTag>
  static TypeStruct makeNamed(std::string uri) {
    TypeStruct result;
    checkName(uri);
    getName<CTag>(result).ensure().uri_ref() = std::move(uri);
    return result;
  }

  template <typename CTag, typename... TArgs>
  static TypeStruct makeParamed(TArgs&&... paramType) {
    TypeStruct result;
    getName<CTag>(result).ensure();
    result.params()->insert(result.params()->end(), {paramType...});
    return result;
  }

  template <typename Tag>
  static TypeStruct makeType(all_c) {
    return makeConcrete<Tag>();
  }
  template <typename Tag>
  static TypeStruct makeType(void_t) {
    return {};
  }
  template <typename Tag>
  static TypeStruct makeType(structured_c) {
    return makeNamed<Tag>(thrift::uri<standard_type<Tag>>());
  }
  template <typename Tag>
  static TypeStruct makeType(enum_c) {
    // TODO(afuller): Support enums in thrift::uri.
    // return makeNamed<Tag>(thrift::uri<standard_type<Tag>>());
    return makeNamed<Tag>("");
  }
  template <typename Tag>
  struct Helper;
  template <typename Tag>
  static TypeStruct makeType(container_c) {
    return Helper<Tag>::makeType();
  }

  template <typename CTag, typename... PTags>
  struct ParamedTypeHelper {
    static TypeStruct makeType() {
      return makeParamed<CTag>(Type::makeType<PTags>(PTags{})...);
    }
  };
  template <typename VTag>
  struct Helper<list<VTag>> : ParamedTypeHelper<list_c, VTag> {};
  template <typename KTag>
  struct Helper<set<KTag>> : ParamedTypeHelper<set_c, KTag> {};
  template <typename KTag, typename VTag>
  struct Helper<map<KTag, VTag>> : ParamedTypeHelper<map_c, KTag, VTag> {};

  // Skip through adapters, cpp_type, etc.
  template <typename Adapter, typename Tag>
  struct Helper<adapted<Adapter, Tag>> : Helper<Tag> {};
  template <typename T, typename Tag>
  struct Helper<cpp_type<T, Tag>> : Helper<Tag> {};
};

} // namespace type
} // namespace thrift
} // namespace apache

FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type::Type)
