/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace proxygen::StructuredFields {

// Data model for RFC 9651, "Structured Field Values for HTTP", Sections 3
// and 4. This is intentionally separate from the older StructuredHeaders API,
// which implements a pre-RFC draft.
struct Decimal {
  int64_t thousandths{0};

  bool operator==(const Decimal& other) const {
    return thousandths == other.thousandths;
  }
};

class BareItem {
 public:
  enum class Type : uint8_t {
    NONE,
    INTEGER,
    DECIMAL,
    STRING,
    TOKEN,
    BYTE_SEQUENCE,
    BOOLEAN,
    DATE,
    DISPLAY_STRING,
  };

  using VariantType =
      std::variant<std::monostate, bool, int64_t, Decimal, std::string>;

  static BareItem integer(int64_t value) noexcept {
    return {Type::INTEGER, value};
  }

  static BareItem decimal(Decimal value) noexcept {
    return {Type::DECIMAL, value};
  }

  static BareItem string(std::string value) noexcept {
    return {Type::STRING, std::move(value)};
  }

  static BareItem token(std::string value) noexcept {
    return {Type::TOKEN, std::move(value)};
  }

  static BareItem byteSequence(std::string value) noexcept {
    return {Type::BYTE_SEQUENCE, std::move(value)};
  }

  static BareItem boolean(bool value) noexcept {
    return {Type::BOOLEAN, value};
  }

  static BareItem date(int64_t value) noexcept {
    return {Type::DATE, value};
  }

  static BareItem displayString(std::string value) noexcept {
    return {Type::DISPLAY_STRING, std::move(value)};
  }

  BareItem() noexcept = default;

  template <typename T>
  [[nodiscard]]
  const T* get() const noexcept {
    return std::get_if<T>(&value_);
  }

  [[nodiscard]]
  Type type() const noexcept {
    return type_;
  }

  bool operator==(const BareItem& other) const {
    return type_ == other.type_ && value_ == other.value_;
  }

 private:
  BareItem(Type typeIn, VariantType valueIn) noexcept
      : type_(typeIn), value_(std::move(valueIn)) {
  }

  Type type_{Type::NONE};
  VariantType value_{std::monostate{}};
};

struct Parameter {
  std::string key;
  BareItem value;

  bool operator==(const Parameter& other) const {
    return key == other.key && value == other.value;
  }
};

class Parameters {
 public:
  [[nodiscard]]
  const BareItem* get(std::string_view key) const {
    for (const auto& parameter : entries_) {
      if (parameter.key == key) {
        return &parameter.value;
      }
    }
    return nullptr;
  }

  [[nodiscard]]
  BareItem* get(std::string_view key) {
    for (auto& parameter : entries_) {
      if (parameter.key == key) {
        return &parameter.value;
      }
    }
    return nullptr;
  }

  void set(std::string key, BareItem value) {
    if (auto* existing = get(key)) {
      *existing = std::move(value);
      return;
    }
    entries_.push_back({std::move(key), std::move(value)});
  }

  [[nodiscard]]
  bool empty() const {
    return entries_.empty();
  }

  [[nodiscard]]
  size_t size() const {
    return entries_.size();
  }

  [[nodiscard]]
  const std::vector<Parameter>& entries() const noexcept {
    return entries_;
  }

 private:
  std::vector<Parameter> entries_;
};

struct Item {
  BareItem bareItem;
  Parameters parameters;
};

struct InnerList {
  std::vector<Item> items;
  Parameters parameters;
};

using ListMember = std::variant<Item, InnerList>;
using List = std::vector<ListMember>;

struct DictionaryMember {
  std::string key;
  ListMember value;
};

class Dictionary {
 public:
  [[nodiscard]]
  const ListMember* get(std::string_view key) const {
    for (const auto& member : entries_) {
      if (member.key == key) {
        return &member.value;
      }
    }
    return nullptr;
  }

  [[nodiscard]]
  ListMember* get(std::string_view key) {
    for (auto& member : entries_) {
      if (member.key == key) {
        return &member.value;
      }
    }
    return nullptr;
  }

  void set(std::string key, ListMember value) {
    if (auto* existing = get(key)) {
      *existing = std::move(value);
      return;
    }
    entries_.push_back({std::move(key), std::move(value)});
  }

  void clear() {
    entries_.clear();
  }

  [[nodiscard]]
  bool empty() const {
    return entries_.empty();
  }

  [[nodiscard]]
  size_t size() const {
    return entries_.size();
  }

  [[nodiscard]]
  const std::vector<DictionaryMember>& entries() const noexcept {
    return entries_;
  }

 private:
  std::vector<DictionaryMember> entries_;
};

enum class DecodeError : uint8_t {
  OK = 0,
  VALUE_TOO_LONG = 1,
  INVALID_CHARACTER = 2,
  UNDECODEABLE_BYTE_SEQUENCE = 3,
  UNEXPECTED_END_OF_BUFFER = 4,
  UNPARSEABLE_NUMERIC_TYPE = 5,
  INVALID_UTF8 = 6,
};

enum class EncodeError : uint8_t {
  OK = 0,
  BAD_KEY = 1,
  BAD_STRING = 2,
  BAD_TOKEN = 3,
  BAD_NUMBER = 4,
  BAD_DISPLAY_STRING = 5,
  ITEM_TYPE_MISMATCH = 6,
  ENCODING_NULL_ITEM = 7,
};

} // namespace proxygen::StructuredFields
