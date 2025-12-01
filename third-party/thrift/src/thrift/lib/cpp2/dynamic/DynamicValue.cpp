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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>

#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

#include <fmt/core.h>

#include <iostream>
#include <utility>

namespace apache::thrift::dynamic {

DynamicValue::DynamicValue(type_system::TypeRef type, detail::Datum datum)
    : type_(type), datum_(std::move(datum)) {
  auto expectedKind = type.matchKind(
      []<type_system::TypeRef::Kind k>(type_system::TypeRef::KindConstant<k>) {
        return detail::Datum::kind_of_type_kind<k>;
      });
  if (datum_.kind() != expectedKind) {
    throw std::runtime_error(
        fmt::format(
            "Type/datum mismatch: type {} expects datum kind {}, got {}",
            static_cast<int>(type.kind()),
            detail::Datum::nameOf(expectedKind),
            detail::Datum::nameOf(datum_.kind())));
  }
}

DynamicValue DynamicValue::makeBool(bool value) {
  return DynamicValue(
      type_system::TypeSystem::Bool(), detail::Datum::makeBool(value));
}

DynamicValue DynamicValue::makeByte(int8_t value) {
  return DynamicValue(
      type_system::TypeSystem::Byte(), detail::Datum::makeByte(value));
}

DynamicValue DynamicValue::makeI16(int16_t value) {
  return DynamicValue(
      type_system::TypeSystem::I16(), detail::Datum::makeI16(value));
}

DynamicValue DynamicValue::makeI32(int32_t value) {
  return DynamicValue(
      type_system::TypeSystem::I32(), detail::Datum::makeI32(value));
}

DynamicValue DynamicValue::makeI64(int64_t value) {
  return DynamicValue(
      type_system::TypeSystem::I64(), detail::Datum::makeI64(value));
}

DynamicValue DynamicValue::makeFloat(float value) {
  return DynamicValue(
      type_system::TypeSystem::Float(), detail::Datum::makeFloat(value));
}

DynamicValue DynamicValue::makeDouble(double value) {
  return DynamicValue(
      type_system::TypeSystem::Double(), detail::Datum::makeDouble(value));
}

DynamicValue DynamicValue::makeString(
    std::string_view sv, std::pmr::memory_resource* mr) {
  return DynamicValue(
      type_system::TypeSystem::String(), detail::Datum::make(String(sv, mr)));
}

DynamicValue DynamicValue::makeBinary(std::unique_ptr<folly::IOBuf> buf) {
  return DynamicValue(
      type_system::TypeSystem::Binary(),
      detail::Datum::make(Binary(std::move(buf))));
}

DynamicValue DynamicValue::makeEnum(
    const type_system::EnumNode& enumType, int32_t value) {
  return DynamicValue(
      type_system::TypeRef(enumType), detail::Datum::makeI32(value));
}

DynamicValue DynamicValue::makeDefault(
    type_system::TypeRef type, std::pmr::memory_resource* mr) {
  using Kind = type_system::TypeRef::Kind;
  switch (type.kind()) {
    case Kind::BOOL:
      return makeBool(false);
    case Kind::BYTE:
      return makeByte(0);
    case Kind::I16:
      return makeI16(0);
    case Kind::I32:
      return makeI32(0);
    case Kind::ENUM:
      return makeEnum(type.asEnumUnchecked(), 0);
    case Kind::I64:
      return makeI64(0);
    case Kind::FLOAT:
      return makeFloat(0.0f);
    case Kind::DOUBLE:
      return makeDouble(0.0);
    case Kind::STRING:
      return makeString("", mr);
    case Kind::BINARY:
      return DynamicValue(type, detail::Datum::make(Binary(mr)));
    case Kind::LIST:
      return DynamicValue(
          type, detail::Datum::make(makeList(type.asListUnchecked(), mr)));
    case Kind::SET:
      return DynamicValue(
          type, detail::Datum::make(makeSet(type.asSetUnchecked(), mr)));
    case Kind::MAP:
      return DynamicValue(
          type, detail::Datum::make(makeMap(type.asMapUnchecked(), mr)));
    case Kind::STRUCT:
      return DynamicValue(
          type, detail::Datum::make(makeStruct(type.asStructUnchecked(), mr)));
    case Kind::UNION:
      return DynamicValue(
          type, detail::Datum::make(makeUnion(type.asUnionUnchecked(), mr)));
    default:
      throw std::runtime_error(
          fmt::format(
              "makeDefault not yet implemented for type {}", type.id().name()));
  }
}

bool DynamicValue::asBool() const& {
  return datum_.as<detail::Datum::Kind::Bool>();
}

bool& DynamicValue::asBool() & {
  return datum_.as<detail::Datum::Kind::Bool>();
}

int8_t DynamicValue::asByte() const& {
  return datum_.as<detail::Datum::Kind::Byte>();
}

int8_t& DynamicValue::asByte() & {
  return datum_.as<detail::Datum::Kind::Byte>();
}

int16_t DynamicValue::asI16() const& {
  return datum_.as<detail::Datum::Kind::I16>();
}

int16_t& DynamicValue::asI16() & {
  return datum_.as<detail::Datum::Kind::I16>();
}

int32_t DynamicValue::asI32() const& {
  if (type_.kind() != type_system::TypeRef::Kind::I32) {
    throw std::runtime_error("Value is not an i32");
  }
  return datum_.as<detail::Datum::Kind::I32>();
}

int32_t& DynamicValue::asI32() & {
  if (type_.kind() != type_system::TypeRef::Kind::I32) {
    throw std::runtime_error("Value is not an i32");
  }
  return datum_.as<detail::Datum::Kind::I32>();
}

int64_t DynamicValue::asI64() const& {
  return datum_.as<detail::Datum::Kind::I64>();
}

int64_t& DynamicValue::asI64() & {
  return datum_.as<detail::Datum::Kind::I64>();
}

float DynamicValue::asFloat() const& {
  return datum_.as<detail::Datum::Kind::Float>();
}

float& DynamicValue::asFloat() & {
  return datum_.as<detail::Datum::Kind::Float>();
}

double DynamicValue::asDouble() const& {
  return datum_.as<detail::Datum::Kind::Double>();
}

double& DynamicValue::asDouble() & {
  return datum_.as<detail::Datum::Kind::Double>();
}

int32_t DynamicValue::asEnum() const& {
  if (type_.kind() != type_system::TypeRef::Kind::ENUM) {
    throw std::runtime_error("Value is not an enum");
  }
  return datum_.as<detail::Datum::Kind::I32>();
}

const String& DynamicValue::asString() const& {
  return datum_.as<detail::Datum::Kind::String>();
}

String& DynamicValue::asString() & {
  return datum_.as<detail::Datum::Kind::String>();
}

const Binary& DynamicValue::asBinary() const& {
  return datum_.as<detail::Datum::Kind::Binary>();
}

Binary& DynamicValue::asBinary() & {
  return datum_.as<detail::Datum::Kind::Binary>();
}

const Any& DynamicValue::asAny() const& {
  return datum_.as<detail::Datum::Kind::Any>();
}

Any& DynamicValue::asAny() & {
  return datum_.as<detail::Datum::Kind::Any>();
}

const List& DynamicValue::asList() const& {
  return datum_.as<detail::Datum::Kind::List>();
}

List& DynamicValue::asList() & {
  return datum_.as<detail::Datum::Kind::List>();
}

const Set& DynamicValue::asSet() const& {
  return datum_.as<detail::Datum::Kind::Set>();
}

Set& DynamicValue::asSet() & {
  return datum_.as<detail::Datum::Kind::Set>();
}

const Map& DynamicValue::asMap() const& {
  return datum_.as<detail::Datum::Kind::Map>();
}

Map& DynamicValue::asMap() & {
  return datum_.as<detail::Datum::Kind::Map>();
}

const Struct& DynamicValue::asStruct() const& {
  return datum_.as<detail::Datum::Kind::Struct>();
}

Struct& DynamicValue::asStruct() & {
  return datum_.as<detail::Datum::Kind::Struct>();
}

const Union& DynamicValue::asUnion() const& {
  return datum_.as<detail::Datum::Kind::Union>();
}

Union& DynamicValue::asUnion() & {
  return datum_.as<detail::Datum::Kind::Union>();
}

bool DynamicValue::asBool() && {
  return std::move(datum_).asBool();
}

int8_t DynamicValue::asByte() && {
  return std::move(datum_).asByte();
}

int16_t DynamicValue::asI16() && {
  return std::move(datum_).asI16();
}

int32_t DynamicValue::asI32() && {
  return std::move(datum_).asI32();
}

int64_t DynamicValue::asI64() && {
  return std::move(datum_).asI64();
}

float DynamicValue::asFloat() && {
  return std::move(datum_).asFloat();
}

double DynamicValue::asDouble() && {
  return std::move(datum_).asDouble();
}

String DynamicValue::asString() && {
  return std::move(datum_).asString();
}

Binary DynamicValue::asBinary() && {
  return std::move(datum_).asBinary();
}

Any DynamicValue::asAny() && {
  return std::move(datum_).asAny();
}

List DynamicValue::asList() && {
  return std::move(datum_).asList();
}

Set DynamicValue::asSet() && {
  return std::move(datum_).asSet();
}

Map DynamicValue::asMap() && {
  return std::move(datum_).asMap();
}

Struct DynamicValue::asStruct() && {
  return std::move(datum_).asStruct();
}

Union DynamicValue::asUnion() && {
  return std::move(datum_).asUnion();
}

bool operator==(const DynamicValue& lhs, const DynamicValue& rhs) noexcept {
  if (lhs.type_.kind() != rhs.type_.kind()) {
    return false;
  }
  return lhs.datum_ == rhs.datum_;
}

std::string DynamicValue::debugString() const {
  folly::IOBufQueue queue;
  DebugProtocolWriter writer;
  writer.setOutput(&queue);
  serializeValue(writer, *this);
  std::string ret;
  queue.appendToString(ret);
  return ret;
}

std::string DynamicRef::debugString() const {
  return copy().debugString();
}

DynamicRef::DynamicRef(DynamicValue& value)
    : type_(&value.type_), ptr_(&value.datum_) {}

DynamicValue DynamicRef::copy() const {
  return std::visit(
      [this](auto ptr) -> DynamicValue {
        using PtrType = decltype(ptr);
        if constexpr (std::is_same_v<PtrType, detail::Datum*>) {
          return DynamicValue(*type_, *ptr);
        } else {
          return DynamicValue(*type_, detail::Datum::make(*ptr));
        }
      },
      ptr_);
}

void DynamicRef::assign(const DynamicRef& other) {
  // Check types match
  if (!type_->isEqualIdentityTo(*other.type_)) {
    throw std::runtime_error("Cannot assign: types don't match");
  }

  // Copy the value from other to this
  std::visit(
      [&other](auto thisPtr) {
        std::visit(
            [thisPtr](auto otherPtr) {
              using ThisPtrType = decltype(thisPtr);
              using OtherPtrType = decltype(otherPtr);

              if constexpr (
                  std::is_same_v<ThisPtrType, detail::Datum*> &&
                  std::is_same_v<OtherPtrType, detail::Datum*>) {
                // Both are Datum pointers
                *thisPtr = *otherPtr;
              } else if constexpr (std::
                                       is_same_v<ThisPtrType, detail::Datum*>) {
                // This is Datum, other is concrete type
                *thisPtr = detail::Datum::make(*otherPtr);
              } else if constexpr (std::is_same_v<
                                       OtherPtrType,
                                       detail::Datum*>) {
                // This is concrete type, other is Datum
                *thisPtr =
                    otherPtr->template as<std::remove_pointer_t<ThisPtrType>>();
              } else if constexpr (std::is_same_v<ThisPtrType, OtherPtrType>) {
                // Both are same concrete type
                *thisPtr = *otherPtr;
              } else {
                throw std::runtime_error("Type mismatch in assign");
              }
            },
            other.ptr_);
      },
      ptr_);
}

void DynamicRef::assign(DynamicValue&& other) {
  // Check types match
  if (!type_->isEqualIdentityTo(other.type_)) {
    throw std::runtime_error("Cannot assign: types don't match");
  }

  // Move the value from other to this
  std::visit(
      [&other](auto thisPtr) {
        using ThisPtrType = decltype(thisPtr);

        if constexpr (std::is_same_v<ThisPtrType, detail::Datum*>) {
          // This is Datum pointer, move the entire datum
          *thisPtr = std::move(other).datum();
        } else {
          // This is concrete type pointer, extract and move
          *thisPtr = std::move(other)
                         .datum()
                         .template as<std::remove_pointer_t<ThisPtrType>>();
        }
      },
      ptr_);
}

bool& DynamicRef::asBool() {
  return deref<bool>();
}

int8_t& DynamicRef::asByte() {
  return deref<int8_t>();
}

int16_t& DynamicRef::asI16() {
  return deref<int16_t>();
}

int32_t& DynamicRef::asI32() {
  return deref<int32_t>();
}

int64_t& DynamicRef::asI64() {
  return deref<int64_t>();
}

float& DynamicRef::asFloat() {
  return deref<float>();
}

double& DynamicRef::asDouble() {
  return deref<double>();
}

String& DynamicRef::asString() {
  return deref<String>();
}

Binary& DynamicRef::asBinary() {
  return deref<Binary>();
}

Any& DynamicRef::asAny() {
  return deref<Any>();
}

List& DynamicRef::asList() {
  return deref<List>();
}

Set& DynamicRef::asSet() {
  return deref<Set>();
}

Map& DynamicRef::asMap() {
  return deref<Map>();
}

Struct& DynamicRef::asStruct() {
  return deref<Struct>();
}

Union& DynamicRef::asUnion() {
  return deref<Union>();
}

bool operator==(const DynamicRef& lhs, const DynamicRef& rhs) noexcept {
  if (!lhs.type_->isEqualIdentityTo(*rhs.type_)) {
    return false;
  }
  // Compare the values
  return lhs.copy() == rhs.copy();
}

bool operator==(const DynamicRef& lhs, const DynamicValue& rhs) noexcept {
  if (!lhs.type_->isEqualIdentityTo(rhs.type())) {
    return false;
  }
  return lhs.copy() == rhs;
}

std::ostream& operator<<(std::ostream& os, const DynamicRef& value) {
  return os << value.debugString();
}

// ============================================================================
// DynamicConstRef implementation
// ============================================================================

std::string DynamicConstRef::debugString() const {
  return copy().debugString();
}

DynamicConstRef::DynamicConstRef(const DynamicValue& value)
    : type_(&value.type_), ptr_(&value.datum_) {}

DynamicConstRef::DynamicConstRef(const DynamicRef& ref) : type_(ref.type_) {
  // Convert from mutable pointer variant to const pointer variant
  ptr_ = std::visit(
      [](auto ptr) -> PointerVariant {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return static_cast<const T*>(ptr);
      },
      ref.ptr_);
}

DynamicValue DynamicConstRef::copy() const {
  return std::visit(
      [this](auto ptr) -> DynamicValue {
        using PtrType = decltype(ptr);
        if constexpr (std::is_same_v<PtrType, const detail::Datum*>) {
          return DynamicValue(*type_, *ptr);
        } else {
          return DynamicValue(*type_, detail::Datum::make(*ptr));
        }
      },
      ptr_);
}

bool DynamicConstRef::asBool() const {
  return deref<bool>();
}

int8_t DynamicConstRef::asByte() const {
  return deref<int8_t>();
}

int16_t DynamicConstRef::asI16() const {
  return deref<int16_t>();
}

int32_t DynamicConstRef::asI32() const {
  return deref<int32_t>();
}

int64_t DynamicConstRef::asI64() const {
  return deref<int64_t>();
}

float DynamicConstRef::asFloat() const {
  return deref<float>();
}

double DynamicConstRef::asDouble() const {
  return deref<double>();
}

const String& DynamicConstRef::asString() const {
  return deref<String>();
}

const Binary& DynamicConstRef::asBinary() const {
  return deref<Binary>();
}

const Any& DynamicConstRef::asAny() const {
  return deref<Any>();
}

const List& DynamicConstRef::asList() const {
  return deref<List>();
}

const Set& DynamicConstRef::asSet() const {
  return deref<Set>();
}

const Map& DynamicConstRef::asMap() const {
  return deref<Map>();
}

const Struct& DynamicConstRef::asStruct() const {
  return deref<Struct>();
}

const Union& DynamicConstRef::asUnion() const {
  return deref<Union>();
}

bool operator==(
    const DynamicConstRef& lhs, const DynamicConstRef& rhs) noexcept {
  if (!lhs.type_->isEqualIdentityTo(*rhs.type_)) {
    return false;
  }
  return lhs.copy() == rhs.copy();
}

bool operator==(const DynamicConstRef& lhs, const DynamicValue& rhs) noexcept {
  if (!lhs.type_->isEqualIdentityTo(rhs.type())) {
    return false;
  }
  return lhs.copy() == rhs;
}

std::ostream& operator<<(std::ostream& os, const DynamicConstRef& value) {
  return os << value.debugString();
}

std::ostream& operator<<(std::ostream& os, const DynamicValue& value) {
  return os << value.debugString();
}

} // namespace apache::thrift::dynamic
