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

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <folly/CPortability.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/Reserve.h>
#include <folly/container/View.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedForwardTypes.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

namespace apache::thrift::detail {

using FieldID = std::int16_t;

template <typename T>
FOLLY_ERASE constexpr VoidPtrFuncPtr eraseFuncPtr(T t) {
  return reinterpret_cast<VoidPtrFuncPtr>(t);
}

template <typename T, typename U = void>
using enable_if_smart_ptr_t = std::enable_if_t<is_shared_or_unique_ptr_v<T>, U>;

template <typename T, typename U = void>
using enable_if_not_smart_ptr_t =
    std::enable_if_t<!is_shared_or_unique_ptr_v<T>, U>;

template <typename T, typename Enable = void>
struct maybe_get_element_type {
  using type = T;
};

template <typename T>
struct maybe_get_element_type<T, enable_if_smart_ptr_t<T>>
    : maybe_get_element_type<std::remove_cv_t<typename T::element_type>> {};

template <typename T>
using maybe_get_element_type_t = typename maybe_get_element_type<T>::type;

enum class StringFieldType {
  String,
  StringView,
  BinaryStringView,
  Binary,
  IOBuf,
  IOBufPtr,
  IOBufObj,
};

// A required field is handled as an unqualified field, because the required
// qualifier is deprecated and has same serialization semantic with the
// unqualified qualifier.
enum class FieldQualifier {
  Unqualified,
  Optional,
  Terse,
};

struct FieldInfo final {
  // Field id in thrift definition.
  FieldID id;

  // Unqualified fields need to be differentiated from optional fields to always
  // write unqualified fields despite the value of __isset.
  FieldQualifier qualifier;

  const char* name;

  // Offset into the data member of the field in the struct.
  ptrdiff_t memberOffset;

  /**
   * If the owning `StructInfo` specifies a `getIsset` and/or `setIsset`
   * function(s), then this value will be passed as the `offset` argument.
   *
   * For either operation, if the owning `StructInfo` does not have a function
   * pointer set, then this field should hold the offset (in bytes) from the
   * beginning of the corresponding struct object, of memory that can be
   * reintepreted as a `bool` holding the "isset" information for this field.
   *
   * If the owning `StructInfo` is a union, this field is set to 0 (and, in
   * practice, never used).
   */
  ptrdiff_t issetOffset;

  const TypeInfo* typeInfo;
};

using VoidVoidPtrFuncPtr = void (*)(void* /* object */);

struct UnionExt final {
  /**
   * Clears the given union data object.
   *
   * Should be called prior to setting any field.
   */
  VoidVoidPtrFuncPtr clear;

  /**
   * Offset (in bytes) from the start of the corresponding union data object to
   * the `int` memory holding the ID of the currently active (i.e., present)
   * field.
   *
   * This is used (and should be non-zero) iff `getActiveId` or `setActiveId`
   * below are nullptr, in which case the value of the active field ID will be
   * written and read directly into that memory.
   */
  ptrdiff_t unionTypeOffset;

  /**
   * Returns the ID of the currently active field for the given union data
   * object, or 0 if none.
   *
   * If this (or `setActiveId`) are non-nullptr, `unionTypeOffset` is not used.
   */
  int (*getActiveId)(const void* /* object */);

  /**
   * Sets the active field ID of the union data `object` to the given value.
   *
   * If this (or `getActiveId`) are non-nullptr, `unionTypeOffset` is not used.
   */
  void (*setActiveId)(void* /* object */, int /* fieldId */);

  FOLLY_PUSH_WARNING
  FOLLY_CLANG_DISABLE_WARNING("-Wc99-extensions")

  /**
   * Optional field member initialization methods.
   *
   * This array can be empty (i.e., `initMember[0] == nullptr`), in which case
   * it is ignored.
   *
   * If not empty, this array must have exactly one (function pointer) value for
   * each field in the union, ordered by field id (i.e., the index of the
   * function for a given field in this array is the same as in the
   * corresponding `StructInfo.fieldInfos`).  These functions will be called
   * when deserializing a Thrift union (whose `StructInfo.unionExt` points to
   * this), with a single argument: a (type-erased, i.e. `void*`) pointer to the
   * value for this field.
   *
   * In practice, when this array is populated, it is done so by leveraging the
   * `placementNewUnionValue()` template methods, to default-create new values
   * directly in the given (allocated) memory (via placement-new).
   */
  VoidVoidPtrFuncPtr initMember[];

  FOLLY_POP_WARNING
};

/**
 * Convenience template class, to initialize a `UnionExt` with the correct
 * layout for the number of specified fields.
 */
template <std::int16_t NumFields>
struct UnionExtN final {
  void (*clear)(void* /* object */);
  ptrdiff_t unionTypeOffset;
  int (*getActiveId)(const void*);
  void (*setActiveId)(void*, int);
  VoidVoidPtrFuncPtr initMember[NumFields];
};

/**
 * Holds all information and operations required to (de)serialize a type-erased
 * "target object" corresponding to a structured Thrift type (i.e., a Thrift
 * struct, union or exception).
 *
 * Instances of this type are core to the "Table-Based" serialization logic, as
 * they bridge the gap between the generic (de)serialization logic and the
 * type-specific API that is unknown to the former (since the "target object"
 * is a type-erased `void*`).
 */
struct StructInfo final {
  /**
   * Number of fields in `fieldInfos`.
   */
  std::int16_t numFields;

  /**
   * Name of this structured type.
   */
  const char* name;

  /**
   * Information and operations specific to Thrift unions, if and only if the
   * type of the target object for this `StructInfo` is a Thrift union.
   *
   * Otherwise (i.e., if the target object type is not a Thrift union), this
   * field must be `nullptr`.
   */
  const UnionExt* unionExt = nullptr;

  /**
   * Returns the value of the "isset" flag at the given `offset` for the given
   * `targetObject`.
   *
   * This function can only be present (i.e., not nullptr) if this StructInfo
   * does NOT correspond to a Thrift union (i.e., `unionExt` is null).
   *
   * @param object A (type-erased) target object whose actual type corresponds
   *        to this `StructInfo`.
   *
   * @param offset of the isset flag to check for a given field, corresponding
   *        to the value of `FieldInfo::issetOffset` for that field.
   */
  bool (*getIsset)(const void* /* targetObject */, ptrdiff_t /* offset */);

  /**
   * Analoguous to `getIsset`, but sets the flag to the given value.
   */
  void (*setIsset)(
      void* /* targetObject */, ptrdiff_t /* offset */, bool /* set */);

  /**
   * If set, this function returns the base address of a contiguous area of
   * memory from which the values of the fields for the given Thrift struct
   * `targetObject` can be retrieved (see below).
   *
   * If not set, this is equivalent to returning the address of the input
   * argument itself (i.e., `targetObject`).
   *
   * The value of a specific field for a given target object is accessed as
   * follows:
   *   1. Compute the "field values base pointer" by calling the method below
   *      (i.e., `StructInfo.getFieldValuesBasePtr()`) with the target object
   *      as argument.
   *   2. Compute the specific "field value pointer" by adding the value of
   *      `FieldInfo.memberOffset` (where `FieldInfo` corresponds to the desired
   *      field from this `StructInfo.fieldInfos[]`) to the previously obtained
   *      "base pointer", i.e.:
   *      ```
   *      fieldValuePtr = fieldValuesBasePtr + fieldInfo.memberOffset
   *      ```
   *   3. Finally, call the `get` or `set` methods of the corresponding
   *      `TypeInfo`, passing the previously obtained `fieldValuePtr` as
   *      argument, i.e.:
   *      ```
   *      fieldInfo.typeInfo->get(fieldValuePtr, *fieldInfo.typeInfo);
   *      ```
   */
  const void* (*getFieldValuesBasePtr)(const void* /* targetObject */) =
      nullptr;

  // Use for other languages to pass in additional information.
  const void* customExt;

  FOLLY_PUSH_WARNING
  FOLLY_CLANG_DISABLE_WARNING("-Wc99-extensions")
  /**
   * Holds `numFields` entries.
   *
   * The memory for these entries is sequentially allocated with instances of
   * `StructInfo`, so this field MUST be the last in this struct.
   */
  FieldInfo fieldInfos[];
  FOLLY_POP_WARNING
};

// Templatized version to const initialize with the exact array length.
template <std::int16_t NumFields>
struct StructInfoN final {
  std::int16_t numFields = NumFields;
  const char* name;
  const void* unionExt = nullptr;
  bool (*getIsset)(const void*, ptrdiff_t);
  void (*setIsset)(void*, ptrdiff_t, bool);
  const void* (*getFieldValuesBasePtr)(const void*);
  const void* customExt;
  FieldInfo fieldInfos[NumFields];
};

template <std::int16_t NumFields>
FOLLY_ERASE const StructInfo& toStructInfo(
    const StructInfoN<NumFields>& templatizedInfo) {
  return reinterpret_cast<const StructInfo&>(templatizedInfo);
}

struct ListFieldExt final {
  const TypeInfo* valInfo;
  std::uint32_t (*size)(const void* object);
  void (*clear)(void* object);
  void (*consumeElem)(
      const void* context,
      void* object,
      void (*reader)(const void* context, void* val));
  void (*readList)(
      const void* context,
      void* object,
      std::uint32_t listSize,
      void (*reader)(const void* context, void* val));
  size_t (*writeList)(
      const void* context,
      const void* val,
      size_t (*writer)(const void* context, const void* val));
};

struct SetFieldExt final {
  const TypeInfo* valInfo;
  std::uint32_t (*size)(const void* object);
  void (*clear)(void* object);
  void (*consumeElem)(
      const void* context,
      void* object,
      void (*reader)(const void* context, void* val));
  void (*readSet)(
      const void* context,
      void* object,
      std::uint32_t setSize,
      void (*reader)(const void* context, void* val));
  size_t (*writeSet)(
      const void* context,
      const void* object,
      bool protocolSortKeys,
      size_t (*writer)(const void* context, const void* val));
};

struct MapFieldExt final {
  const TypeInfo* keyInfo;
  const TypeInfo* valInfo;
  std::uint32_t (*size)(const void* object);
  void (*clear)(void* object);
  void (*consumeElem)(
      const void* context,
      void* object,
      void (*keyReader)(const void* context, void* key),
      void (*valueReader)(const void* context, void* val));
  void (*readMap)(
      const void* context,
      void* object,
      std::uint32_t mapSize,
      void (*keyReader)(const void* context, void* key),
      void (*valueReader)(const void* context, void* val));
  size_t (*writeMap)(
      const void* context,
      const void* object,
      bool protocolSortKeys,
      size_t (*writer)(
          const void* context, const void* keyElem, const void* valueElem));
};

template <typename ThriftUnion>
void clearUnion(void* object) {
  apache::thrift::clear(*reinterpret_cast<ThriftUnion*>(object));
}

/**
 * Holds a thrift value in its default target type for cpp2.
 */
union ThriftValue {
  explicit ThriftValue(bool value) : boolValue(value) {}
  explicit ThriftValue(std::int8_t value) : int8Value(value) {}
  explicit ThriftValue(std::int16_t value) : int16Value(value) {}
  explicit ThriftValue(std::int32_t value) : int32Value(value) {}
  explicit ThriftValue(std::int64_t value) : int64Value(value) {}
  explicit ThriftValue(float value) : floatValue(value) {}
  explicit ThriftValue(double value) : doubleValue(value) {}
  explicit ThriftValue(const void* value) : object(value) {}
  explicit ThriftValue(folly::IOBuf* value) : iobuf(value) {}
  explicit ThriftValue(folly::StringPiece value) : stringViewValue(value) {}

  bool boolValue;
  std::int8_t int8Value;
  std::int16_t int16Value;
  std::int32_t int32Value;
  std::int64_t int64Value;
  float floatValue;
  double doubleValue;
  const void* object;
  folly::IOBuf* iobuf;
  folly::StringPiece stringViewValue;
};

using OptionalThriftValue = folly::Optional<ThriftValue>;

template <typename PrimitiveType, typename ObjectType>
enable_if_not_smart_ptr_t<ObjectType, OptionalThriftValue> get(
    const void* object, const TypeInfo& /* typeInfo */) {
  return folly::make_optional<ThriftValue>(
      static_cast<PrimitiveType>(*static_cast<const ObjectType*>(object)));
}

template <typename PrimitiveType, typename PtrType>
enable_if_smart_ptr_t<PtrType, OptionalThriftValue> get(
    const void* object, const TypeInfo& /* typeInfo */) {
  if (const auto* ptr = static_cast<const PtrType*>(object)->get()) {
    return folly::make_optional<ThriftValue>(static_cast<PrimitiveType>(*ptr));
  }
  return folly::none;
}

template <typename PtrType>
enable_if_smart_ptr_t<PtrType, OptionalThriftValue> get(
    const void* object, const TypeInfo& /* typeInfo */) {
  if (const void* ptr = static_cast<const PtrType*>(object)->get()) {
    return folly::make_optional<ThriftValue>(ptr);
  }
  return folly::none;
}

template <typename T, enable_if_not_smart_ptr_t<T, int> = 0>
constexpr auto getDerefFuncPtr() {
  return nullptr;
}

template <typename T, enable_if_smart_ptr_t<T, int> = 0>
constexpr auto getDerefFuncPtr() {
  return get<T>;
}

template <typename ObjectType>
enable_if_not_smart_ptr_t<ObjectType, void*> set(void* object) {
  *static_cast<ObjectType*>(object) = ObjectType();
  return object;
}

template <typename PtrType>
std::enable_if_t<is_shared_ptr_v<PtrType>, void*> set(void* object) {
  using Element = typename PtrType::element_type;
  auto& ptr = *static_cast<PtrType*>(object);
  ptr = std::make_shared<Element>();
  return const_cast<std::remove_const_t<Element>*>(ptr.get());
}

template <typename PtrType>
std::enable_if_t<is_unique_ptr_v<PtrType>, void*> set(void* object) {
  using Element = typename PtrType::element_type;
  auto& ptr = *static_cast<PtrType*>(object);
  ptr = std::make_unique<Element>();
  return const_cast<std::remove_const_t<Element>*>(ptr.get());
}

template <typename ObjectType, typename PrimitiveType>
enable_if_not_smart_ptr_t<ObjectType, void*> set(
    void* object, PrimitiveType val) {
  *static_cast<ObjectType*>(object) = static_cast<ObjectType>(val);
  return nullptr;
}

template <typename PtrType, typename PrimitiveType>
std::enable_if_t<is_unique_ptr_v<PtrType>, void*> set(
    void* object, PrimitiveType val) {
  using Element = typename PtrType::element_type;
  *static_cast<PtrType*>(object) =
      std::make_unique<Element>(static_cast<Element>(val));
  return nullptr;
}

template <typename PtrType, typename PrimitiveType>
std::enable_if_t<is_shared_ptr_v<PtrType>, void*> set(
    void* object, PrimitiveType val) {
  using Element = typename PtrType::element_type;
  *static_cast<PtrType*>(object) =
      std::make_shared<Element>(static_cast<Element>(val));
  return nullptr;
}

template <typename ValueType>
enable_if_not_smart_ptr_t<ValueType> placementNewUnionValue(void* object) {
  ::new (object) ValueType();
}

template <typename PtrType>
enable_if_smart_ptr_t<PtrType> placementNewUnionValue(void* object) {
  ::new (object) PtrType(new typename PtrType::element_type());
}

template <typename List>
size_t writeList(
    const void* context,
    const void* object,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  const List& out = *static_cast<const List*>(object);
  size_t written = 0;
  for (auto& elem : out) {
    written += writer(context, &elem);
  }
  return written;
}

template <typename Set>
size_t writeSet(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  const Set& out = *static_cast<const Set*>(object);
  size_t written = 0;

  if (!folly::is_detected_v<
          ::apache::thrift::detail::pm::detect_key_compare,
          Set> &&
      protocolSortKeys) {
    std::vector<typename Set::const_iterator> iters;
    iters.reserve(out.size());
    for (auto it = out.begin(); it != out.end(); ++it) {
      iters.push_back(it);
    }
    std::sort(
        iters.begin(), iters.end(), [](auto a, auto b) { return *a < *b; });
    for (auto it : iters) {
      written += writer(context, &(*it));
    }
  } else {
    // Support containers with defined but non-FIFO iteration order.
    auto get_view = folly::order_preserving_reinsertion_view_or_default;
    for (const auto& elem : get_view(out)) {
      written += writer(context, &elem);
    }
  }
  return written;
}

template <typename Map>
size_t writeMap(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(
        const void* /*context*/,
        const void* /*keyElem*/,
        const void* /*valueElem*/)) {
  const Map& out = *static_cast<const Map*>(object);
  size_t written = 0;
  if (!folly::is_detected_v<
          ::apache::thrift::detail::pm::detect_key_compare,
          Map> &&
      protocolSortKeys) {
    std::vector<typename Map::const_iterator> iters;
    iters.reserve(out.size());
    for (auto it = out.begin(); it != out.end(); ++it) {
      iters.push_back(it);
    }
    std::sort(iters.begin(), iters.end(), [](auto a, auto b) {
      return a->first < b->first;
    });
    for (auto it : iters) {
      written += writer(context, &it->first, &it->second);
    }
  } else {
    // Support containers with defined but non-FIFO iteration order.
    auto get_view = folly::order_preserving_reinsertion_view_or_default;
    for (auto& elem_pair : get_view(out)) {
      written += writer(context, &elem_pair.first, &elem_pair.second);
    }
  }
  return written;
}

template <typename Container>
std::uint32_t containerSize(const void* object) {
  return folly::to_narrow(
      folly::to_unsigned(static_cast<const Container*>(object)->size()));
}

template <typename Container>
void containerClear(void* object) {
  static_cast<Container*>(object)->clear();
}

template <typename Map>
void consumeMapElem(
    const void* context,
    void* object,
    void (*keyReader)(const void* /*context*/, void* /*key*/),
    void (*valueReader)(const void* /*context*/, void* /*val*/)) {
  Map& out = *static_cast<Map*>(object);
  typename Map::key_type key;
  keyReader(context, &key);
  valueReader(context, &out[key]);
}

template <typename Map>
void readMap(
    const void* context,
    void* object,
    std::uint32_t mapSize,
    void (*keyReader)(const void* /*context*/, void* /*key*/),
    void (*valueReader)(const void* /*context*/, void* /*val*/)) {
  Map& out = *static_cast<Map*>(object);
  folly::reserve_if_available(out, mapSize);

  for (auto i = mapSize; i--;) {
    typename Map::key_type key;
    keyReader(context, &key);
    valueReader(context, &out[key]);
  }
}

template <typename List>
void consumeListElem(
    const void* context,
    void* object,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  List& out = *static_cast<List*>(object);
  out.emplace_back();
  reader(context, &out.back());
}

template <typename Set>
void consumeSetElem(
    const void* context,
    void* object,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  Set& out = *static_cast<Set*>(object);
  typename Set::value_type tmp;
  reader(context, &tmp);
  out.insert(std::move(tmp));
}

template <typename Set>
void readKnownLengthSet(
    const void* context,
    void* object,
    std::uint32_t setSize,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  Set& out = *static_cast<Set*>(object);
  folly::reserve_if_available(out, setSize);

  while (setSize--) {
    consumeSetElem<Set>(context, object, reader);
  }
}

template <typename List>
void readList(
    const void* context,
    void* object,
    std::uint32_t listSize,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  List& out = *static_cast<List*>(object);
  using traits = std::iterator_traits<typename List::iterator>;
  using cat = typename traits::iterator_category;
  if (folly::reserve_if_available(out, listSize) ||
      std::is_same_v<cat, std::bidirectional_iterator_tag>) {
    while (listSize--) {
      consumeListElem<List>(context, object, reader);
    }
  } else {
    out.resize(listSize);
    for (auto& elem : out) {
      reader(context, &elem);
    }
  }
}

#define THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(      \
    TypeClass, Type, ThriftType, TTypeValue)       \
  template <>                                      \
  struct TypeToInfo<type_class::TypeClass, Type> { \
    using underlying_type = ThriftType;            \
    static const TypeInfo typeInfo;                \
  }

// Specializations for numbers.
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int8_t, std::int8_t, T_BYTE);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int16_t, std::int16_t, T_I16);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int32_t, std::int32_t, T_I32);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int64_t, std::int64_t, T_I64);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint8_t, std::int8_t, T_BYTE);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint16_t, std::int16_t, T_I16);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint32_t, std::int32_t, T_I32);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint64_t, std::int64_t, T_I64);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(integral, bool, bool, T_BOOL);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(floating_point, float, float, T_FLOAT);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(floating_point, double, double, T_DOUBLE);

#undef THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO

// Specialization for string.
#define THRIFT_DEFINE_STRING_TYPE_TO_INFO(TypeClass, T, ExtVal) \
  template <>                                                   \
  struct TypeToInfo<type_class::TypeClass, T> {                 \
    static const StringFieldType ext;                           \
    static const TypeInfo typeInfo;                             \
  }

THRIFT_DEFINE_STRING_TYPE_TO_INFO(string, std::string, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    string, folly::fbstring, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(binary, std::string, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    binary, folly::fbstring, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(binary, folly::IOBuf, StringFieldType::IOBuf);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    binary, std::unique_ptr<folly::IOBuf>, StringFieldType::IOBufPtr);

#undef THRIFT_DEFINE_STRING_TYPE_TO_INFO

// Specialization for set.
template <typename ElemTypeClass, typename T>
struct TypeToInfo<type_class::set<ElemTypeClass>, T> {
  using set_type = maybe_get_element_type_t<T>;
  static const SetFieldExt ext;
  static const TypeInfo typeInfo;
};

template <typename ElemTypeClass, typename T>
const SetFieldExt TypeToInfo<type_class::set<ElemTypeClass>, T>::ext = {
    /* .valInfo */ &TypeToInfo<ElemTypeClass, typename set_type::value_type>::
        typeInfo,
    /* .size */ containerSize<set_type>,
    /* .clear */ containerClear<set_type>,
    /* .consumeElem */ consumeSetElem<set_type>,
    /* .readSet */ readKnownLengthSet<set_type>,
    /* .writeSet */ writeSet<set_type>,
};

template <typename ElemTypeClass, typename T>
const TypeInfo TypeToInfo<type_class::set<ElemTypeClass>, T>::typeInfo = {
    /* .type */ protocol::TType::T_SET,
    /* .get */ getDerefFuncPtr<T>(),
    /* .set */ eraseFuncPtr(set<T>),
    /* .typeExt */
    &TypeToInfo<type_class::set<ElemTypeClass>, T>::ext,
};

// Specialization for list.
template <typename ElemTypeClass, typename T>
struct TypeToInfo<type_class::list<ElemTypeClass>, T> {
  using list_type = maybe_get_element_type_t<T>;
  static const ListFieldExt ext;
  static const TypeInfo typeInfo;
};
template <typename ElemTypeClass, typename T>
const ListFieldExt TypeToInfo<type_class::list<ElemTypeClass>, T>::ext = {
    /* .valInfo */ &TypeToInfo<ElemTypeClass, typename list_type::value_type>::
        typeInfo,
    /* .size */ containerSize<list_type>,
    /* .clear */ containerClear<list_type>,
    /* .consumeElem */ consumeListElem<list_type>,
    /* .readList */ readList<list_type>,
    /* .writeList */ writeList<list_type>,
};
template <typename ElemTypeClass, typename T>
const TypeInfo TypeToInfo<type_class::list<ElemTypeClass>, T>::typeInfo = {
    /* .type */ protocol::TType::T_LIST,
    /* .get */ getDerefFuncPtr<T>(),
    /* .set */ eraseFuncPtr(set<T>),
    /* .typeExt */ &ext,
};

// Specialization for map.
template <typename KeyTypeClass, typename ValTypeClass, typename T>
struct TypeToInfo<type_class::map<KeyTypeClass, ValTypeClass>, T> {
  using map_type = maybe_get_element_type_t<T>;
  static const MapFieldExt ext;
  static const TypeInfo typeInfo;
};
template <typename KeyTypeClass, typename ValTypeClass, typename T>
const MapFieldExt
    TypeToInfo<type_class::map<KeyTypeClass, ValTypeClass>, T>::ext = {
        /* .keyInfo */ &TypeToInfo<KeyTypeClass, typename map_type::key_type>::
            typeInfo,
        /* .valInfo */
        &TypeToInfo<ValTypeClass, typename map_type::mapped_type>::typeInfo,
        /* .size */ containerSize<map_type>,
        /* .clear */ containerClear<map_type>,
        /* .consumeElem */ consumeMapElem<map_type>,
        /* .readMap */ readMap<map_type>,
        /* .writeMap */ writeMap<map_type>,
};
template <typename KeyTypeClass, typename ValTypeClass, typename T>
const TypeInfo
    TypeToInfo<type_class::map<KeyTypeClass, ValTypeClass>, T>::typeInfo = {
        /* .type */ protocol::TType::T_MAP,
        /* .get */ getDerefFuncPtr<T>(),
        /* .set */ eraseFuncPtr(set<T>),
        /* .typeExt */ &ext,
};

// Specialization for smart pointers of type class struct and union.
#define THRIFT_DEFINE_STRUCT_PTR_TYPE_INFO(TypeClass)                     \
  template <typename T>                                                   \
  struct TypeToInfo<type_class::TypeClass, T, enable_if_smart_ptr_t<T>> { \
    using struct_type = std::remove_cv_t<typename T::element_type>;       \
    static const TypeInfo typeInfo;                                       \
  };                                                                      \
                                                                          \
  template <typename T>                                                   \
  const TypeInfo TypeToInfo<                                              \
      type_class::TypeClass,                                              \
      T,                                                                  \
      enable_if_smart_ptr_t<T>>::typeInfo = {                             \
      TypeToInfo<type_class::TypeClass, struct_type>::typeInfo.type,      \
      get<T>,                                                             \
      eraseFuncPtr(set<T>),                                               \
      TypeToInfo<type_class::TypeClass, struct_type>::typeInfo.typeExt,   \
  }

THRIFT_DEFINE_STRUCT_PTR_TYPE_INFO(structure);
THRIFT_DEFINE_STRUCT_PTR_TYPE_INFO(variant);
#undef THRIFT_DEFINE_STRUCT_PTR_TYPE_INFO

// Specialization for smart pointers of numerical types.
#define THRIFT_DEFINE_NUMERIC_PTR_TYPE_INFO(TypeClass)                        \
  template <typename T>                                                       \
  struct TypeToInfo<type_class::TypeClass, T, enable_if_smart_ptr_t<T>> {     \
    using numeric_type = std::remove_cv_t<typename T::element_type>;          \
    using underlying_type =                                                   \
        typename TypeToInfo<type_class::TypeClass, numeric_type>::            \
            underlying_type;                                                  \
    static const TypeInfo typeInfo;                                           \
  };                                                                          \
                                                                              \
  template <typename T>                                                       \
  const TypeInfo                                                              \
      TypeToInfo<type_class::TypeClass, T, enable_if_smart_ptr_t<T>>::        \
          typeInfo = {                                                        \
              TypeToInfo<type_class::TypeClass, numeric_type>::typeInfo.type, \
              get<underlying_type, T>,                                        \
              eraseFuncPtr(set<T, underlying_type>),                          \
              nullptr,                                                        \
  }

THRIFT_DEFINE_NUMERIC_PTR_TYPE_INFO(integral);
THRIFT_DEFINE_NUMERIC_PTR_TYPE_INFO(floating_point);
#undef THRIFT_DEFINE_NUMERIC_PTR_TYPE_INFO

template <class ThriftStruct>
constexpr ptrdiff_t fieldOffset(std::int16_t fieldIndex);

template <class ThriftStruct>
constexpr ptrdiff_t issetOffset(std::int16_t fieldIndex);

template <class ThriftUnion>
constexpr ptrdiff_t unionTypeOffset();

template <class TProtocol>
void read(TProtocol* iprot, const StructInfo& structInfo, void* targetObject);

template <class TProtocol>
size_t write(
    TProtocol* iprot, const StructInfo& structInfo, const void* targetObject);

} // namespace apache::thrift::detail
