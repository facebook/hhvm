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

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>

#include <thrift/lib/cpp2/util/TypeErasedRef.h>

namespace apache::thrift::util {

/**
 * A type-safe type-erased const reference to a std::tuple of objects.
 *
 * This type builds on top of TypeErasedRef to provide an equivalent construct
 * for std::tuple<Elements...>.
 *
 * std::get<I>(tuple) is equivalent to TypeErasedTupleRef::get(I).
 * std::tuple_size_v<Tuple> is equivalent to TypeErasedTupleRef::count().
 *
 * TypeErasedTupleRef does not own the referred-to object. The user must ensure
 * that the tuple outlives any access through this object.
 *
 * TypeErasedTupleRef is copyable. The copy points to the same object as the
 * original.
 */
class TypeErasedTupleRef;

namespace detail {

class TypeErasedTupleRefVTable {
 public:
  virtual ~TypeErasedTupleRefVTable() = default;
  virtual std::optional<apache::thrift::util::TypeErasedRef> get(
      std::size_t index, const void* tuple) const noexcept = 0;
};

template <typename... Fields>
class TypeErasedTupleRefVTableImpl;

// This specialization is for methods with no arguments
template <>
class TypeErasedTupleRefVTableImpl<> : public TypeErasedTupleRefVTable {
  std::optional<apache::thrift::util::TypeErasedRef> get(
      std::size_t, const void*) const noexcept override {
    return std::nullopt;
  }
};

template <typename... Fields>
class TypeErasedTupleRefVTableImpl : public TypeErasedTupleRefVTable {
 private:
  static_assert(sizeof...(Fields) > 0);

  using Tuple = std::tuple<Fields...>;

  using DynamicGetFunc =
      std::pair<const void*, const std::type_info&> (*)(const Tuple&);

  // DynamicGetFuncTable allows mapping a runtime index into a table of
  // corresponding function pointers that call std::get<Index> and the
  // corresponding std::type_info at that index.
  // This mechanism allows type-erased (but type-checked) access to a list of
  // arguments at runtime.
  template <typename Indices = std::make_index_sequence<sizeof...(Fields)>>
  struct DynamicGetFuncTable;
  template <std::size_t... Indices>
  struct DynamicGetFuncTable<std::index_sequence<Indices...>> {
    using TypeErasedResult = std::pair<const void*, const std::type_info&>;

    template <size_t Index>
    static TypeErasedResult getTypeErased(const Tuple& result) {
      return TypeErasedResult(
          static_cast<const void*>(std::addressof(std::get<Index>(result))),
          typeid(std::tuple_element_t<Index, Tuple>));
    }
    static inline constexpr DynamicGetFunc dynamicGet[sizeof...(Indices)] = {
        &getTypeErased<Indices>...};
  };

  std::optional<TypeErasedRef> get(
      std::size_t index, const void* tuple) const noexcept override {
    if (index < sizeof...(Fields)) {
      const auto& [ptr, typeInfo] = DynamicGetFuncTable<>::dynamicGet[index](
          *reinterpret_cast<const Tuple*>(tuple));
      return TypeErasedRef::fromTypeInfoUnchecked(ptr, typeInfo);
    }
    return std::nullopt;
  }
};

template <typename... Fields>
inline const TypeErasedTupleRefVTableImpl<Fields...>
    typeErasedTupleRefVTableImpl;

} // namespace detail

class TypeErasedTupleRef {
 public:
  template <typename... Args>
  explicit TypeErasedTupleRef(std::tuple<Args...>& args)
      : vtable_(&detail::typeErasedTupleRefVTableImpl<Args...>),
        typeErasedTuple_(static_cast<const void*>(std::addressof(args))),
        count_(sizeof...(Args)) {}

  TypeErasedTupleRef(const TypeErasedTupleRef&) = default;
  TypeErasedTupleRef& operator=(const TypeErasedTupleRef&) = default;

  std::size_t count() const { return count_; }

  std::optional<TypeErasedRef> get(std::size_t index) const {
    return vtable_->get(index, typeErasedTuple_);
  }

 private:
  const detail::TypeErasedTupleRefVTable* vtable_;
  const void* typeErasedTuple_;
  std::size_t count_;
};

} // namespace apache::thrift::util
