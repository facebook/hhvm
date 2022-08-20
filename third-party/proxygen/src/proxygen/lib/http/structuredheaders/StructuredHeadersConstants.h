/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/variant.hpp>
#include <map>
#include <unordered_map>
#include <vector>

namespace proxygen { namespace StructuredHeaders {

static const int kMaxValidIntegerLength = 19;
static const int kMaxValidFloatLength = 16;

/* tagged union for an item in a structured header */
class StructuredHeaderItem {
 public:
  enum class Type {
    NONE,
    STRING,
    BINARYCONTENT,
    IDENTIFIER,
    DOUBLE,
    INT64,
    BOOLEAN
  };

  using VariantType = boost::variant<bool, int64_t, double, std::string>;

  StructuredHeaderItem() = default;
  StructuredHeaderItem(Type tagIn, VariantType valueIn)
      : tag(tagIn), value(valueIn) {
  }

  template <typename T>
  bool operator!=(const T& other) const {
    return !operator==(other);
  }

  template <typename T>
  bool operator==(const T& other) const {
    try {
      return boost::get<T>(value) == other;
    } catch (boost::bad_get&) {
      return false;
    }
  }

  template <typename T>
  T get() const {
    return boost::get<T>(value);
  }

  Type tag;
  VariantType value;
};

struct ParameterisedIdentifier {
  std::string identifier;
  std::unordered_map<std::string, StructuredHeaderItem> parameterMap;
};

using ParameterisedList =
    std::vector<StructuredHeaders::ParameterisedIdentifier>;

using Dictionary = std::unordered_map<std::string, StructuredHeaderItem>;

enum class DecodeError : uint8_t {
  OK = 0,
  VALUE_TOO_LONG = 1,
  INVALID_CHARACTER = 2,
  UNDECODEABLE_BINARY_CONTENT = 3,
  UNEXPECTED_END_OF_BUFFER = 4,
  UNPARSEABLE_NUMERIC_TYPE = 5,
  DUPLICATE_KEY = 6
};

enum class EncodeError : uint8_t {
  OK = 0,
  EMPTY_DATA_STRUCTURE = 1,
  BAD_IDENTIFIER = 2,
  BAD_STRING = 3,
  ITEM_TYPE_MISMATCH = 4,
  ENCODING_NULL_ITEM = 5
};

static const std::map<DecodeError, std::string> decodeErrorDescription{
    {DecodeError::OK, "No error"},
    {DecodeError::VALUE_TOO_LONG, "Numeric value is too long"},
    {DecodeError::INVALID_CHARACTER, "Invalid character"},
    {DecodeError::UNDECODEABLE_BINARY_CONTENT, "Undecodable binary content"},
    {DecodeError::UNEXPECTED_END_OF_BUFFER, "Unexpected end of buffer"},
    {DecodeError::UNPARSEABLE_NUMERIC_TYPE, "Unparseable numeric type"},
    {DecodeError::DUPLICATE_KEY, "Duplicate key found"}};

static const std::map<EncodeError, std::string> encodeErrorDescription{
    {EncodeError::OK, "No error"},
    {EncodeError::EMPTY_DATA_STRUCTURE, "Empty data structure"},
    {EncodeError::BAD_IDENTIFIER, "Bad identifier"},
    {EncodeError::BAD_STRING, "Bad string"},
    {EncodeError::ITEM_TYPE_MISMATCH, "Item type mismatch"},
    {EncodeError::ENCODING_NULL_ITEM, "Tried to encode null item"}};

}} // namespace proxygen::StructuredHeaders
