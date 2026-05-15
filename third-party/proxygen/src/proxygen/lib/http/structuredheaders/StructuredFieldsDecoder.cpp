/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredFieldsDecoder.h>

#include <proxygen/lib/utils/UtilInl.h>

#include <folly/Conv.h>
#include <folly/base64.h>

#include <cstdint>
#include <string>

namespace proxygen {
namespace {

// Parser helpers follow RFC 9651, Section 4.2. Appendix C ABNF is informative;
// the step-by-step parsing algorithms are the source of truth.
using proxygen::StructuredFields::BareItem;
using proxygen::StructuredFields::Decimal;
using proxygen::StructuredFields::DecodeError;
using proxygen::StructuredFields::Item;

bool isLcAlpha(char c) {
  return c >= 'a' && c <= 'z';
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isInvalidAsciiChar(char c) {
  const auto byte = static_cast<unsigned char>(c);
  return byte < 0x20 || byte >= 0x7F;
}

bool isInvalidFieldValueChar(char c) {
  const auto byte = static_cast<unsigned char>(c);
  return (byte < 0x20 && c != '\t') || byte >= 0x7F;
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

bool isBase64Char(char c) {
  return isAlpha(c) || isDigit(c) || c == '+' || c == '/' || c == '=';
}

bool isValidBase64Padding(std::string_view input) {
  bool paddingSeen = false;
  for (char c : input) {
    if (c == '=') {
      paddingSeen = true;
    } else if (paddingSeen) {
      return false;
    }
  }
  return true;
}

bool isLowerHex(char c) {
  return isDigit(c) || (c >= 'a' && c <= 'f');
}

uint8_t lowerHexValue(char c) {
  return isDigit(c) ? c - '0' : c - 'a' + 10;
}

bool parseUint64(std::string_view digits, uint64_t& result) {
  const auto parsed =
      folly::tryTo<uint64_t>(digits.data(), digits.data() + digits.size());
  if (!parsed.hasValue()) {
    return false;
  }
  result = parsed.value();
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

} // namespace

DecodeError StructuredFieldsDecoder::decodeItem(
    StructuredFields::Item& result) {
  if (!isAsciiInput()) {
    return DecodeError::INVALID_CHARACTER;
  }
  removeSP();
  auto err = parseItem(result);
  if (err != DecodeError::OK) {
    return err;
  }
  return finishTopLevel();
}

DecodeError StructuredFieldsDecoder::decodeList(
    StructuredFields::List& result) {
  if (!isAsciiInput()) {
    return DecodeError::INVALID_CHARACTER;
  }

  removeSP();
  while (!empty()) {
    StructuredFields::ListMember member;
    auto err = parseItemOrInnerList(member);
    if (err != DecodeError::OK) {
      return err;
    }
    result.push_back(std::move(member));

    removeOWS();
    if (empty()) {
      return DecodeError::OK;
    }
    if (!consume(',')) {
      return DecodeError::INVALID_CHARACTER;
    }
    removeOWS();
    if (empty()) {
      return DecodeError::UNEXPECTED_END_OF_BUFFER;
    }
  }

  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::decodeDictionary(
    StructuredFields::Dictionary& result) {
  if (!isAsciiInput()) {
    return DecodeError::INVALID_CHARACTER;
  }

  removeSP();
  while (!empty()) {
    std::string key;
    auto err = parseKey(key);
    if (err != DecodeError::OK) {
      return err;
    }

    StructuredFields::ListMember member;
    if (consume('=')) {
      err = parseItemOrInnerList(member);
      if (err != DecodeError::OK) {
        return err;
      }
    } else {
      Item item;
      item.bareItem = BareItem::boolean(true);
      err = parseParameters(item.parameters);
      if (err != DecodeError::OK) {
        return err;
      }
      member = std::move(item);
    }
    result.set(std::move(key), std::move(member));

    removeOWS();
    if (empty()) {
      return DecodeError::OK;
    }
    if (!consume(',')) {
      return DecodeError::INVALID_CHARACTER;
    }
    removeOWS();
    if (empty()) {
      return DecodeError::UNEXPECTED_END_OF_BUFFER;
    }
  }

  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseItemOrInnerList(
    StructuredFields::ListMember& result) {
  if (!empty() && peek() == '(') {
    StructuredFields::InnerList innerList;
    auto err = parseInnerList(innerList);
    if (err != DecodeError::OK) {
      return err;
    }
    result = std::move(innerList);
    return DecodeError::OK;
  }

  Item item;
  auto err = parseItem(item);
  if (err != DecodeError::OK) {
    return err;
  }
  result = std::move(item);
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseInnerList(
    StructuredFields::InnerList& result) {
  if (!consume('(')) {
    return DecodeError::INVALID_CHARACTER;
  }

  while (!empty()) {
    removeSP();
    if (!empty() && consume(')')) {
      return parseParameters(result.parameters);
    }

    Item item;
    auto err = parseItem(item);
    if (err != DecodeError::OK) {
      return err;
    }
    result.items.push_back(std::move(item));

    if (empty() || (peek() != ' ' && peek() != ')')) {
      return DecodeError::INVALID_CHARACTER;
    }
  }

  return DecodeError::UNEXPECTED_END_OF_BUFFER;
}

DecodeError StructuredFieldsDecoder::parseItem(Item& result) {
  auto err = parseBareItem(result.bareItem);
  if (err != DecodeError::OK) {
    return err;
  }
  return parseParameters(result.parameters);
}

DecodeError StructuredFieldsDecoder::parseBareItem(BareItem& result) {
  if (empty()) {
    return DecodeError::UNEXPECTED_END_OF_BUFFER;
  }

  const char first = peek();
  if (first == '-' || isDigit(first)) {
    return parseNumber(result);
  }
  if (first == '"') {
    return parseString(result);
  }
  if (isAlpha(first) || first == '*') {
    return parseToken(result);
  }
  if (first == ':') {
    return parseByteSequence(result);
  }
  if (first == '?') {
    return parseBoolean(result);
  }
  if (first == '@') {
    return parseDate(result);
  }
  if (first == '%') {
    return parseDisplayString(result);
  }
  return DecodeError::INVALID_CHARACTER;
}

DecodeError StructuredFieldsDecoder::parseParameters(
    StructuredFields::Parameters& result) {
  while (!empty() && peek() == ';') {
    advance();
    removeSP();

    std::string key;
    auto err = parseKey(key);
    if (err != DecodeError::OK) {
      return err;
    }

    BareItem value = BareItem::boolean(true);
    if (consume('=')) {
      err = parseBareItem(value);
      if (err != DecodeError::OK) {
        return err;
      }
    }
    result.set(std::move(key), std::move(value));
  }
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseKey(std::string& result) {
  if (empty() || (!isLcAlpha(peek()) && peek() != '*')) {
    return DecodeError::INVALID_CHARACTER;
  }

  while (!empty() && isKeyChar(peek())) {
    result.push_back(peek());
    advance();
  }

  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseNumber(BareItem& result) {
  int64_t sign = 1;
  if (consume('-')) {
    sign = -1;
  }

  if (empty() || !isDigit(peek())) {
    return DecodeError::INVALID_CHARACTER;
  }

  bool isDecimal = false;
  std::string inputNumber;
  while (!empty()) {
    const char current = peek();
    if (isDigit(current)) {
      inputNumber.push_back(current);
      advance();
    } else if (!isDecimal && current == '.') {
      if (inputNumber.size() > 12) {
        return DecodeError::VALUE_TOO_LONG;
      }
      inputNumber.push_back(current);
      isDecimal = true;
      advance();
    } else {
      break;
    }

    if (inputNumber.size() > 15 + static_cast<size_t>(isDecimal)) {
      return DecodeError::VALUE_TOO_LONG;
    }
  }

  if (!isDecimal) {
    uint64_t integer = 0;
    if (!parseUint64(inputNumber, integer)) {
      return DecodeError::VALUE_TOO_LONG;
    }
    result = BareItem::integer(sign * static_cast<int64_t>(integer));
    return DecodeError::OK;
  }

  if (inputNumber.back() == '.') {
    return DecodeError::INVALID_CHARACTER;
  }

  const auto dot = inputNumber.find('.');
  const auto fractionalDigits = inputNumber.size() - dot - 1;
  if (fractionalDigits > 3) {
    return DecodeError::VALUE_TOO_LONG;
  }

  uint64_t integerPart = 0;
  uint64_t fractionalPart = 0;
  if (!parseUint64(std::string_view(inputNumber).substr(0, dot), integerPart) ||
      !parseUint64(std::string_view(inputNumber).substr(dot + 1),
                   fractionalPart)) {
    return DecodeError::UNPARSEABLE_NUMERIC_TYPE;
  }
  for (size_t i = fractionalDigits; i < 3; ++i) {
    fractionalPart *= 10;
  }

  const auto magnitude = integerPart * 1000 + fractionalPart;
  const auto signedMagnitude = sign * static_cast<int64_t>(magnitude);
  result = BareItem::decimal(Decimal{signedMagnitude});
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseString(BareItem& result) {
  if (!consume('"')) {
    return DecodeError::INVALID_CHARACTER;
  }

  std::string output;
  while (!empty()) {
    char current = 0;
    consumeChar(current);
    if (current == '\\') {
      if (empty()) {
        return DecodeError::UNEXPECTED_END_OF_BUFFER;
      }
      char next = 0;
      consumeChar(next);
      if (next != '"' && next != '\\') {
        return DecodeError::INVALID_CHARACTER;
      }
      output.push_back(next);
    } else if (current == '"') {
      result = BareItem::string(std::move(output));
      return DecodeError::OK;
    } else if (isInvalidAsciiChar(current)) {
      return DecodeError::INVALID_CHARACTER;
    } else {
      output.push_back(current);
    }
  }

  return DecodeError::UNEXPECTED_END_OF_BUFFER;
}

DecodeError StructuredFieldsDecoder::parseToken(BareItem& result) {
  if (empty() || (!isAlpha(peek()) && peek() != '*')) {
    return DecodeError::INVALID_CHARACTER;
  }

  std::string output;
  while (!empty() && (isTchar(peek()) || peek() == ':' || peek() == '/')) {
    output.push_back(peek());
    advance();
  }

  result = BareItem::token(std::move(output));
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseByteSequence(BareItem& result) {
  if (!consume(':')) {
    return DecodeError::INVALID_CHARACTER;
  }

  std::string encoded;
  while (!empty() && peek() != ':') {
    if (!isBase64Char(peek())) {
      return DecodeError::INVALID_CHARACTER;
    }
    encoded.push_back(peek());
    advance();
  }

  if (!consume(':')) {
    return DecodeError::UNEXPECTED_END_OF_BUFFER;
  }

  if (!isValidBase64Padding(encoded)) {
    return DecodeError::UNDECODEABLE_BYTE_SEQUENCE;
  }
  if (encoded.size() % 4 == 1) {
    return DecodeError::UNDECODEABLE_BYTE_SEQUENCE;
  }
  while (encoded.size() % 4 != 0) {
    encoded.push_back('=');
  }

  try {
    result = BareItem::byteSequence(folly::base64Decode(encoded));
  } catch (...) {
    return DecodeError::UNDECODEABLE_BYTE_SEQUENCE;
  }
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseBoolean(BareItem& result) {
  if (!consume('?')) {
    return DecodeError::INVALID_CHARACTER;
  }
  if (empty()) {
    return DecodeError::UNEXPECTED_END_OF_BUFFER;
  }

  if (consume('1')) {
    result = BareItem::boolean(true);
    return DecodeError::OK;
  }
  if (consume('0')) {
    result = BareItem::boolean(false);
    return DecodeError::OK;
  }
  return DecodeError::INVALID_CHARACTER;
}

DecodeError StructuredFieldsDecoder::parseDate(BareItem& result) {
  if (!consume('@')) {
    return DecodeError::INVALID_CHARACTER;
  }

  BareItem number;
  auto err = parseNumber(number);
  if (err != DecodeError::OK) {
    return err;
  }
  const auto* value = number.get<int64_t>();
  if (number.type() != BareItem::Type::INTEGER || value == nullptr) {
    return DecodeError::INVALID_CHARACTER;
  }

  result = BareItem::date(*value);
  return DecodeError::OK;
}

DecodeError StructuredFieldsDecoder::parseDisplayString(BareItem& result) {
  if (!consume('%') || !consume('"')) {
    return DecodeError::INVALID_CHARACTER;
  }

  std::string bytes;
  while (!empty()) {
    char current = 0;
    consumeChar(current);
    if (isInvalidAsciiChar(current)) {
      return DecodeError::INVALID_CHARACTER;
    }
    if (current == '%') {
      char high = 0;
      char low = 0;
      if (!consumeChar(high) || !consumeChar(low) || !isLowerHex(high) ||
          !isLowerHex(low)) {
        return DecodeError::INVALID_CHARACTER;
      }
      bytes.push_back(
          static_cast<char>((lowerHexValue(high) << 4) | lowerHexValue(low)));
    } else if (current == '"') {
      if (!isValidUtf8(bytes)) {
        return DecodeError::INVALID_UTF8;
      }
      result = BareItem::displayString(std::move(bytes));
      return DecodeError::OK;
    } else {
      bytes.push_back(current);
    }
  }

  return DecodeError::UNEXPECTED_END_OF_BUFFER;
}

DecodeError StructuredFieldsDecoder::finishTopLevel() {
  removeSP();
  return empty() ? DecodeError::OK : DecodeError::INVALID_CHARACTER;
}

void StructuredFieldsDecoder::removeSP() {
  while (!empty() && peek() == ' ') {
    advance();
  }
}

void StructuredFieldsDecoder::removeOWS() {
  while (!empty() && (peek() == ' ' || peek() == '\t')) {
    advance();
  }
}

bool StructuredFieldsDecoder::consume(char expected) {
  if (empty() || peek() != expected) {
    return false;
  }
  advance();
  return true;
}

bool StructuredFieldsDecoder::consumeChar(char& result) {
  if (empty()) {
    return false;
  }
  result = peek();
  advance();
  return true;
}

bool StructuredFieldsDecoder::empty() const {
  return pos_ >= input_.size();
}

char StructuredFieldsDecoder::peek() const {
  return input_[pos_];
}

void StructuredFieldsDecoder::advance() {
  ++pos_;
}

bool StructuredFieldsDecoder::isAsciiInput() const {
  for (const auto c : input_) {
    if (isInvalidFieldValueChar(c)) {
      return false;
    }
  }
  return true;
}

} // namespace proxygen
