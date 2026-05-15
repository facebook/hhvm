/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredFieldsEncoder.h>

#include <folly/Conv.h>
#include <folly/base64.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace proxygen {
namespace {

// Serializer helpers follow RFC 9651, Section 4.1. The output is canonical
// enough for deterministic tests while still matching the RFC parser.
using proxygen::StructuredFields::BareItem;
using proxygen::StructuredFields::EncodeError;

constexpr int64_t kMaxInteger = 999'999'999'999'999;
constexpr uint64_t kMaxDecimalMagnitude = 999'999'999'999'999;

bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isLcAlpha(char c) {
  return c >= 'a' && c <= 'z';
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isKeyChar(char c) {
  return isLcAlpha(c) || isDigit(c) || c == '_' || c == '-' || c == '.' ||
         c == '*';
}

bool isTchar(char c) {
  return isAlpha(c) || isDigit(c) || c == '!' || c == '#' || c == '$' ||
         c == '%' || c == '&' || c == '\'' || c == '*' || c == '+' ||
         c == '-' || c == '.' || c == '^' || c == '_' || c == '`' || c == '|' ||
         c == '~';
}

bool isValidKey(std::string_view input) {
  if (input.empty() || (!isLcAlpha(input.front()) && input.front() != '*')) {
    return false;
  }
  for (char c : input) {
    if (!isKeyChar(c)) {
      return false;
    }
  }
  return true;
}

bool isValidUtf8(std::string_view input) {
  size_t i = 0;
  while (i < input.size()) {
    const auto c = static_cast<unsigned char>(input[i]);
    if (c <= 0x7F) {
      ++i;
      continue;
    }

    uint32_t codePoint = 0;
    size_t continuationCount = 0;
    if (c >= 0xC2 && c <= 0xDF) {
      codePoint = c & 0x1F;
      continuationCount = 1;
    } else if (c >= 0xE0 && c <= 0xEF) {
      codePoint = c & 0x0F;
      continuationCount = 2;
    } else if (c >= 0xF0 && c <= 0xF4) {
      codePoint = c & 0x07;
      continuationCount = 3;
    } else {
      return false;
    }

    if (i + continuationCount >= input.size()) {
      return false;
    }

    for (size_t j = 1; j <= continuationCount; ++j) {
      const auto continuation = static_cast<unsigned char>(input[i + j]);
      if ((continuation & 0xC0) != 0x80) {
        return false;
      }
      codePoint = (codePoint << 6) | (continuation & 0x3F);
    }

    if ((continuationCount == 2 && codePoint < 0x800) ||
        (continuationCount == 3 && codePoint < 0x10000) ||
        (codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > 0x10FFFF) {
      return false;
    }

    i += continuationCount + 1;
  }
  return true;
}

uint64_t magnitude(int64_t input) {
  if (input >= 0) {
    return static_cast<uint64_t>(input);
  }
  return static_cast<uint64_t>(-(input + 1)) + 1;
}

bool isTrueBoolean(const BareItem& input) {
  const auto* value = input.get<bool>();
  return input.type() == BareItem::Type::BOOLEAN && value != nullptr && *value;
}

} // namespace

StructuredFields::EncodeError StructuredFieldsEncoder::encodeItem(
    const StructuredFields::Item& input) {
  return appendItem(input);
}

StructuredFields::EncodeError StructuredFieldsEncoder::encodeList(
    const StructuredFields::List& input) {
  for (auto it = input.begin(); it != input.end(); ++it) {
    auto err = appendListMember(*it);
    if (err != EncodeError::OK) {
      return err;
    }
    if (std::next(it) != input.end()) {
      output_.append(", ");
    }
  }
  return EncodeError::OK;
}

StructuredFields::EncodeError StructuredFieldsEncoder::encodeDictionary(
    const StructuredFields::Dictionary& input) {
  const auto& entries = input.entries();
  for (auto it = entries.begin(); it != entries.end(); ++it) {
    auto err = appendKey(it->key);
    if (err != EncodeError::OK) {
      return err;
    }

    if (const auto* item = std::get_if<StructuredFields::Item>(&it->value);
        item != nullptr && isTrueBoolean(item->bareItem)) {
      err = appendParameters(item->parameters);
    } else {
      output_.push_back('=');
      err = appendListMember(it->value);
    }
    if (err != EncodeError::OK) {
      return err;
    }

    if (std::next(it) != entries.end()) {
      output_.append(", ");
    }
  }
  return EncodeError::OK;
}

std::string StructuredFieldsEncoder::get() const {
  return output_;
}

EncodeError StructuredFieldsEncoder::appendListMember(
    const StructuredFields::ListMember& input) {
  if (const auto* item = std::get_if<StructuredFields::Item>(&input)) {
    return appendItem(*item);
  }
  return appendInnerList(std::get<StructuredFields::InnerList>(input));
}

EncodeError StructuredFieldsEncoder::appendInnerList(
    const StructuredFields::InnerList& input) {
  output_.push_back('(');
  for (auto it = input.items.begin(); it != input.items.end(); ++it) {
    auto err = appendItem(*it);
    if (err != EncodeError::OK) {
      return err;
    }
    if (std::next(it) != input.items.end()) {
      output_.push_back(' ');
    }
  }
  output_.push_back(')');
  return appendParameters(input.parameters);
}

EncodeError StructuredFieldsEncoder::appendItem(
    const StructuredFields::Item& input) {
  auto err = appendBareItem(input.bareItem);
  if (err != EncodeError::OK) {
    return err;
  }
  return appendParameters(input.parameters);
}

EncodeError StructuredFieldsEncoder::appendParameters(
    const StructuredFields::Parameters& input) {
  for (const auto& parameter : input.entries()) {
    output_.push_back(';');
    auto err = appendKey(parameter.key);
    if (err != EncodeError::OK) {
      return err;
    }
    if (isTrueBoolean(parameter.value)) {
      continue;
    }
    output_.push_back('=');
    err = appendBareItem(parameter.value);
    if (err != EncodeError::OK) {
      return err;
    }
  }
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendKey(std::string_view input) {
  if (!isValidKey(input)) {
    return EncodeError::BAD_KEY;
  }
  output_.append(input.data(), input.size());
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendBareItem(const BareItem& input) {
  switch (input.type()) {
    case BareItem::Type::INTEGER:
      if (const auto* value = input.get<int64_t>()) {
        return appendInteger(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::DECIMAL:
      if (const auto* value = input.get<StructuredFields::Decimal>()) {
        return appendDecimal(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::STRING:
      if (const auto* value = input.get<std::string>()) {
        return appendString(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::TOKEN:
      if (const auto* value = input.get<std::string>()) {
        return appendToken(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::BYTE_SEQUENCE:
      if (const auto* value = input.get<std::string>()) {
        return appendByteSequence(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::BOOLEAN:
      if (const auto* value = input.get<bool>()) {
        output_ += *value ? "?1" : "?0";
        return EncodeError::OK;
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::DATE:
      if (const auto* value = input.get<int64_t>()) {
        if (*value < -kMaxInteger || *value > kMaxInteger) {
          return EncodeError::BAD_NUMBER;
        }
        output_.push_back('@');
        folly::toAppend(*value, &output_);
        return EncodeError::OK;
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::DISPLAY_STRING:
      if (const auto* value = input.get<std::string>()) {
        return appendDisplayString(*value);
      } else {
        return EncodeError::ITEM_TYPE_MISMATCH;
      }
    case BareItem::Type::NONE:
      return EncodeError::ENCODING_NULL_ITEM;
  }
  return EncodeError::ENCODING_NULL_ITEM;
}

EncodeError StructuredFieldsEncoder::appendInteger(int64_t input) {
  if (input < -kMaxInteger || input > kMaxInteger) {
    return EncodeError::BAD_NUMBER;
  }
  folly::toAppend(input, &output_);
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendDecimal(
    StructuredFields::Decimal input) {
  const auto absValue = magnitude(input.thousandths);
  if (absValue > kMaxDecimalMagnitude) {
    return EncodeError::BAD_NUMBER;
  }

  if (input.thousandths < 0) {
    output_.push_back('-');
  }

  folly::toAppend(absValue / 1000, &output_);
  output_.push_back('.');

  auto fractional = absValue % 1000;
  if (fractional == 0) {
    output_.push_back('0');
    return EncodeError::OK;
  }

  std::array<char, 3> digits{static_cast<char>('0' + fractional / 100),
                             static_cast<char>('0' + (fractional / 10) % 10),
                             static_cast<char>('0' + fractional % 10)};
  size_t count = digits.size();
  while (count > 1 && digits[count - 1] == '0') {
    --count;
  }
  output_.append(digits.data(), count);
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendString(std::string_view input) {
  output_.push_back('"');
  for (char c : input) {
    if (c < 0x20 || c == 0x7F || static_cast<unsigned char>(c) > 0x7F) {
      return EncodeError::BAD_STRING;
    }
    if (c == '"' || c == '\\') {
      output_.push_back('\\');
    }
    output_.push_back(c);
  }
  output_.push_back('"');
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendToken(std::string_view input) {
  if (input.empty() || (!isAlpha(input.front()) && input.front() != '*')) {
    return EncodeError::BAD_TOKEN;
  }
  for (char c : input) {
    if (!isTchar(c) && c != ':' && c != '/') {
      return EncodeError::BAD_TOKEN;
    }
  }
  output_.append(input.data(), input.size());
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendByteSequence(
    std::string_view input) {
  output_.push_back(':');
  output_ += folly::base64Encode(input);
  output_.push_back(':');
  return EncodeError::OK;
}

EncodeError StructuredFieldsEncoder::appendDisplayString(
    std::string_view input) {
  if (!isValidUtf8(input)) {
    return EncodeError::BAD_DISPLAY_STRING;
  }

  static constexpr std::array<char, 16> kLowerHex{'0',
                                                  '1',
                                                  '2',
                                                  '3',
                                                  '4',
                                                  '5',
                                                  '6',
                                                  '7',
                                                  '8',
                                                  '9',
                                                  'a',
                                                  'b',
                                                  'c',
                                                  'd',
                                                  'e',
                                                  'f'};
  output_ += "%\"";
  for (const auto byte : input) {
    const auto c = static_cast<unsigned char>(byte);
    if (c == '%' || c == '"' || c <= 0x1F || c >= 0x7F) {
      output_.push_back('%');
      output_.push_back(kLowerHex[c >> 4]);
      output_.push_back(kLowerHex[c & 0x0F]);
    } else {
      output_.push_back(static_cast<char>(c));
    }
  }
  output_.push_back('"');
  return EncodeError::OK;
}

} // namespace proxygen
