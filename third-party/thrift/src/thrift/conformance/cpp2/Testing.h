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

// Utilities for unit testing conformance implementions.

#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <folly/Conv.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/conformance/cpp2/AnySerializer.h>
#include <thrift/conformance/cpp2/AnyStructSerializer.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/conformance/if/gen-cpp2/any_types_custom_protocol.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types_custom_protocol.h>

// Helper so gtest prints out the line number when running the given check.
#define THRIFT_SCOPED_CHECK(check) \
  {                                \
    SCOPED_TRACE(#check);          \
    check;                         \
  }

namespace apache::thrift::conformance {

constexpr StandardProtocol kUnknownStdProtocol =
    static_cast<StandardProtocol>(1000);

inline const Protocol& UnknownProtocol() {
  return getStandardProtocol<kUnknownStdProtocol>();
}

// Creates an ThriftTypeInfo for tests using the given name(s).
ThriftTypeInfo testThriftType(const std::string& shortName);
ThriftTypeInfo testThriftType(std::initializer_list<const char*> names);

// Creates an ThriftTypeInfo that is smaller than the associated type id.
ThriftTypeInfo shortThriftType(int ordinal = 1);
// Creates an ThriftTypeInfo that is larger than the associated type id.
ThriftTypeInfo longThriftType(int ordinal = 1);

// Returns the full thrift type name, for the given type.
std::string thriftType(std::string_view type);

std::string toString(const folly::IOBuf& buf);

// Checks round trips using the given serializer.
template <typename S, typename T>
void checkRoundTrip(const S& seralizer, const T& expected) {
  folly::IOBufQueue queue;
  seralizer.encode(expected, folly::io::QueueAppender{&queue, 2 << 4});
  folly::io::Cursor cursor(queue.front());
  T actual = seralizer.template decode<T>(cursor);
  EXPECT_EQ(actual, expected);
}

// Checks round trips for all standard protocols
template <typename T>
void checkRoundTrip(const T& value) {
  THRIFT_SCOPED_CHECK(checkRoundTrip(
      getAnyStandardSerializer<T, StandardProtocol::Binary>(), value));
  THRIFT_SCOPED_CHECK(checkRoundTrip(
      getAnyStandardSerializer<T, StandardProtocol::Compact>(), value));
  THRIFT_SCOPED_CHECK(checkRoundTrip(
      getAnyStandardSerializer<T, StandardProtocol::Json>(), value));
  THRIFT_SCOPED_CHECK(checkRoundTrip(
      getAnyStandardSerializer<T, StandardProtocol::SimpleJson>(), value));
}

// Always serializes integers to the number 1.
class Number1Serializer
    : public BaseTypedAnySerializer<int, Number1Serializer> {
  using Base = BaseTypedAnySerializer<int, Number1Serializer>;

 public:
  static const Protocol kProtocol;

  const Protocol& getProtocol() const override { return kProtocol; }

  using Base::encode;
  void encode(const int&, folly::io::QueueAppender&& appender) const {
    std::string data = "number 1!!";
    appender.push(reinterpret_cast<const uint8_t*>(data.data()), data.size());
  }

  using Base::decode;
  void decode(folly::io::Cursor& cursor, int& value) const {
    cursor.readFixedString(10);
    value = 1;
  }
};

extern const Protocol kFollyToStringProtocol;

// A serializer based on `folly::to<std::string>`.
template <typename T>
class FollyToStringSerializer
    : public BaseTypedAnySerializer<T, FollyToStringSerializer<T>> {
  using Base = BaseTypedAnySerializer<T, FollyToStringSerializer<T>>;

 public:
  const Protocol& getProtocol() const override {
    return kFollyToStringProtocol;
  }
  using Base::encode;
  void encode(const T& value, folly::io::QueueAppender&& appender) const {
    std::string data = folly::to<std::string>(value);
    appender.push(reinterpret_cast<const uint8_t*>(data.data()), data.size());
  }
  using Base::decode;
  void decode(folly::io::Cursor& cursor, T& value) const {
    value = folly::to<T>(cursor.readFixedString(cursor.totalLength()));
  }
};

class MultiSerializer : public AnySerializer {
  using Base = AnySerializer;

 public:
  mutable size_t intEncCount = 0;
  mutable size_t dblEncCount = 0;

  mutable size_t intDecCount = 0;
  mutable size_t dblDecCount = 0;
  mutable size_t anyDecCount = 0;

  using Base::decode;
  using Base::encode;

  const Protocol& getProtocol() const override {
    return kFollyToStringProtocol;
  }
  void encode(
      any_ref value, folly::io::QueueAppender&& appender) const override;
  void decode(
      const std::type_info& typeInfo,
      folly::io::Cursor& cursor,
      any_ref value) const override;

  // Helper functions to check the statis
  void checkAndResetInt(size_t enc, size_t dec) const;
  void checkAndResetDbl(size_t enc, size_t dec) const;
  void checkAndResetAny(size_t dec) const;
  void checkAndResetAll() const;
  void checkAnyDec() const;
  void checkIntEnc() const;
  void checkIntDec() const;
  void checkDblEnc() const;
  void checkDblDec() const;
  void checkAnyIntDec() const;
  void checkAnyDblDec() const;
};

// The different types of ctors/dtor/assignment.
//
// Order from most to least versatile, in terms of what
// contexts they can be used in.
//
// TODO(afuller): Use the option pattern instead. See Boost.Intrusive.
enum class CtorType {
  Trivial, // Implies also NoThrow.
  NoThrow,
  Throw,
  Delete,
};

// Scope for genericly named classes/helpers.
namespace test {

// If the given CtorType can be used in a throw context.
template <CtorType Type>
inline constexpr bool is_throw_compat_v = Type <= CtorType::Throw;

// If the given CtorType can be used in a nothrow context.
template <CtorType Type>
inline constexpr bool is_nothrow_compat_v = Type <= CtorType::NoThrow;

// If the given CtorType can be used in a trivial context.
template <CtorType Type>
inline constexpr bool is_trivial_compat_v = Type <= CtorType::Trivial;

// Constructors may have to call destructors, so the requirements
// combine.
template <CtorType Ctor, CtorType Dtor>
inline constexpr CtorType ctor_type_v =
    Ctor == CtorType::Delete ? CtorType::Delete
    : Ctor > Dtor            ? Ctor
                             : Dtor;

// An empty struct with the given traits.
template <
    CtorType DefaultType = CtorType::Trivial,
    CtorType CopyType = CtorType::Trivial,
    CtorType MoveType = CtorType::Trivial,
    CtorType DestructType = CtorType::Trivial>
struct EmptyStruct;

// A few helpful 'presets'.
using Copyable =
    EmptyStruct<CtorType::Trivial, CtorType::Throw, CtorType::Delete>;

using Moveable =
    EmptyStruct<CtorType::Trivial, CtorType::Delete, CtorType::NoThrow>;

using MoveableThrow =
    EmptyStruct<CtorType::Trivial, CtorType::Delete, CtorType::Throw>;

using NonMoveable =
    EmptyStruct<CtorType::Trivial, CtorType::Delete, CtorType::Delete>;

using NoDefault = EmptyStruct<CtorType::Delete>;

using DestructorThrow = EmptyStruct<
    CtorType::Trivial,
    CtorType::Trivial,
    CtorType::Trivial,
    CtorType::Throw>;

// An empty struct derived from the given base class.
template <typename Base = Copyable>
struct Derived : Base {
  using Base::Base;
};

} // namespace test

// Statically asserts that T has the required properties.
template <typename T, CtorType DefaultType>
constexpr void staticAssertDefaultConstructible() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(
      std::is_default_constructible_v<T> ==
      test::is_throw_compat_v<DefaultType>);
  static_assert( // is_nothrow_default_constructible_v
      std::is_nothrow_default_constructible_v<T> ==
      test::is_nothrow_compat_v<DefaultType>);
  static_assert( // is_trivially_default_constructible_v
      std::is_trivially_default_constructible_v<T> ==
      test::is_trivial_compat_v<DefaultType>);
}

// Statically asserts that T has the required properties.
template <typename T, CtorType CopyType>
constexpr void staticAssertCopyConstructible() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(
      std::is_copy_constructible_v<T> == test::is_throw_compat_v<CopyType>);
  static_assert( // is_nothrow_copy_constructible_v
      std::is_nothrow_copy_constructible_v<T> ==
      test::is_nothrow_compat_v<CopyType>);
  static_assert( // is_trivially_copy_constructible_v
      std::is_trivially_copy_constructible_v<T> ==
      test::is_trivial_compat_v<CopyType>);
}

// Statically asserts that T has the required properties.
template <typename T, CtorType CopyType>
constexpr void staticAssertCopyAssignable() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(
      std::is_copy_assignable_v<T> == test::is_throw_compat_v<CopyType>);
  static_assert( // is_nothrow_copy_assignable_v
      std::is_nothrow_copy_assignable_v<T> ==
      test::is_nothrow_compat_v<CopyType>);
  static_assert( // is_trivially_copy_assignable_v
      std::is_trivially_copy_assignable_v<T> ==
      test::is_trivial_compat_v<CopyType>);
}

// Statically asserts that T has the required properties.
template <typename T, CtorType MoveType>
constexpr void staticAssertMoveConstructible() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(
      std::is_move_constructible_v<T> == test::is_throw_compat_v<MoveType>);
  static_assert( // is_nothrow_move_constructible_v
      std::is_nothrow_move_constructible_v<T> ==
      test::is_nothrow_compat_v<MoveType>);
  static_assert( // is_trivially_move_constructible_v
      std::is_trivially_move_constructible_v<T> ==
      test::is_trivial_compat_v<MoveType>);
}

// Statically asserts that T has the required properties.
template <typename T, CtorType MoveType>
constexpr void staticAssertMoveAssignable() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(
      std::is_move_assignable_v<T> == test::is_throw_compat_v<MoveType>);
  static_assert( // is_nothrow_move_assignable_v
      std::is_nothrow_move_assignable_v<T> ==
      test::is_nothrow_compat_v<MoveType>);
  static_assert( // is_trivially_move_assignable_v
      std::is_trivially_move_assignable_v<T> ==
      test::is_trivial_compat_v<MoveType>);
}

// Statically asserts that T has the required properties.
template <typename T, CtorType DtorType>
constexpr void staticAssertDestructible() {
  static_assert(sizeof(T), "Type is incomplete");
  static_assert(DtorType != CtorType::Delete, "can't delete destructor.");
  static_assert( // is_nothrow_destructible_v
      std::is_nothrow_destructible_v<T> == test::is_nothrow_compat_v<DtorType>);
  static_assert( // is_trivially_destructible_v
      std::is_trivially_destructible_v<T> ==
      test::is_trivial_compat_v<DtorType>);
}

template <CtorType Ctor, CtorType Dtor>
constexpr void ValidateTestCase() {
  static_assert(
      test::ctor_type_v<Ctor, Dtor> == Ctor,
      "Bad test: Constructors cannot have lower requirements than destructors.");
}

// Statically asserts that T has the required properties.
template <
    typename T,
    CtorType DefaultType,
    CtorType CopyType,
    CtorType MoveType,
    CtorType DtorType>
constexpr void staticAssertConstructible() {
  ValidateTestCase<DefaultType, DtorType>();
  ValidateTestCase<CopyType, DtorType>();
  ValidateTestCase<MoveType, DtorType>();
  staticAssertDefaultConstructible<T, DefaultType>();
  staticAssertCopyConstructible<T, CopyType>();
  staticAssertMoveConstructible<T, MoveType>();
  staticAssertDestructible<T, DtorType>();
}

// Statically asserts that T has the required properties.
template <typename T, CtorType CopyType, CtorType MoveType>
constexpr void staticAssertAssignable() {
  staticAssertCopyAssignable<T, CopyType>();
  staticAssertMoveAssignable<T, MoveType>();
}

// Statically asserts that T has the required properties.
template <
    typename T,
    CtorType DefaultType,
    CtorType CopyType,
    CtorType MoveType,
    CtorType DtorType>
constexpr void staticAssertCtors() {
  staticAssertConstructible<T, DefaultType, CopyType, MoveType, DtorType>();
  staticAssertAssignable<T, CopyType, MoveType>();
}

// Implementation.

namespace test {

// Generate all specializtions for EmptyStruct.
#define _THRIFT_CON_Delete = delete;
#define _THRIFT_CON_Trivial noexcept(true) = default;
#define _THRIFT_CON_NoThrow \
  noexcept(true) {}
#define _THRIFT_CON_Throw \
  noexcept(false) {}
#define _THRIFT_ASSIGN_Delete _THRIFT_CON_Delete
#define _THRIFT_ASSIGN_Trivial _THRIFT_CON_Trivial
#define _THRIFT_ASSIGN_NoThrow \
  noexcept(true) {             \
    return *this;              \
  }
#define _THRIFT_ASSIGN_Throw \
  noexcept(false) {          \
    return *this;            \
  }

#define _THRIFT_CON(type) _THRIFT_CON_##type
#define _THRIFT_ASSIGN(type) _THRIFT_ASSIGN_##type

// clang-format off
#define _THRIFT_INSTANCE(default, dtor, copy, move)                 \
  template <>                                                       \
  struct EmptyStruct<                                               \
      CtorType::default,                                            \
      CtorType::copy,                                               \
      CtorType::move,                                               \
      CtorType::dtor> {                                             \
    EmptyStruct() _THRIFT_CON(default)                              \
    ~EmptyStruct() _THRIFT_CON(dtor)                                \
                                                                    \
    EmptyStruct(const EmptyStruct&) _THRIFT_CON(copy)               \
    EmptyStruct& operator=(const EmptyStruct&) _THRIFT_ASSIGN(copy) \
                                                                    \
    EmptyStruct(EmptyStruct&&) _THRIFT_CON(move)                    \
    EmptyStruct& operator=(EmptyStruct&&) _THRIFT_ASSIGN(move)      \
  }
// clang-format on

#define _THRIFT_ALL_MOVE(default, dtor, copy)     \
  _THRIFT_INSTANCE(default, dtor, copy, Delete);  \
  _THRIFT_INSTANCE(default, dtor, copy, Trivial); \
  _THRIFT_INSTANCE(default, dtor, copy, NoThrow); \
  _THRIFT_INSTANCE(default, dtor, copy, Throw)

#define _THRIFT_ALL_COPY(default, dtor)     \
  _THRIFT_ALL_MOVE(default, dtor, Delete);  \
  _THRIFT_ALL_MOVE(default, dtor, Trivial); \
  _THRIFT_ALL_MOVE(default, dtor, NoThrow); \
  _THRIFT_ALL_MOVE(default, dtor, Throw)

#define _THRIFT_ALL(dtor)          \
  _THRIFT_ALL_COPY(Delete, dtor);  \
  _THRIFT_ALL_COPY(Trivial, dtor); \
  _THRIFT_ALL_COPY(NoThrow, dtor); \
  _THRIFT_ALL_COPY(Throw, dtor)

_THRIFT_ALL(NoThrow);
_THRIFT_ALL(Trivial);
_THRIFT_ALL(Throw);

#undef _THRIFT_CTOR_Delete
#undef _THRIFT_CTOR_Trivial
#undef _THRIFT_CTOR_NoThrow
#undef _THRIFT_CTOR_Throw
#undef _THRIFT_ASSIGN_Delete
#undef _THRIFT_ASSIGN_Trivial
#undef _THRIFT_ASSIGN_NoThrow
#undef _THRIFT_ASSIGN_Throw
#undef _THRIFT_CTOR
#undef _THRIFT_ASSIGN
#undef _THRIFT_INSTANCE
#undef _THRIFT_ALL_MOVE
#undef _THRIFT_ALL_COPY
#undef _THRIFT_ALL

} // namespace test

} // namespace apache::thrift::conformance
