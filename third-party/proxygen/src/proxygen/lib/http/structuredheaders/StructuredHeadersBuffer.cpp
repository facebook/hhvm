/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersBuffer.h>

#include <boost/lexical_cast.hpp>
#include <cctype>
#include <glog/logging.h>

#include <proxygen/lib/http/structuredheaders/StructuredHeadersUtilities.h>

namespace proxygen {

using namespace StructuredHeaders;

DecodeError StructuredHeadersBuffer::parseItem(StructuredHeaderItem& result) {

  removeOptionalWhitespace();

  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  } else {
    char firstCharacter = peek();
    if (firstCharacter == '"') {
      return parseString(result);
    } else if (firstCharacter == '*') {
      return parseBinaryContent(result);
    } else if (std::isdigit(firstCharacter) || firstCharacter == '-') {
      return parseNumber(result);
    } else if (firstCharacter == '?') {
      return parseBoolean(result);
    } else {
      return handleDecodeError(DecodeError::INVALID_CHARACTER);
    }
  }
}

DecodeError StructuredHeadersBuffer::parseNumber(StructuredHeaderItem& result) {
  auto type = StructuredHeaderItem::Type::INT64;

  bool positive = true;
  std::string input;

  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  if (peek() == '-') {
    advanceCursor();
    positive = false;
    input.push_back('-');
  }

  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  if (!std::isdigit(peek())) {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  }

  while (!isEmpty()) {
    char current = peek();
    if (std::isdigit(current)) {
      input.push_back(current);
      advanceCursor();
    } else if (type == StructuredHeaderItem::Type::INT64 && current == '.') {
      type = StructuredHeaderItem::Type::DOUBLE;
      input.push_back(current);
      advanceCursor();
    } else {
      break;
    }

    int numDigits = input.length() - (positive ? 0 : 1);
    if (type == StructuredHeaderItem::Type::INT64 &&
        numDigits > StructuredHeaders::kMaxValidIntegerLength) {
      return handleDecodeError(DecodeError::VALUE_TOO_LONG);
    } else if (type == StructuredHeaderItem::Type::DOUBLE &&
               numDigits > StructuredHeaders::kMaxValidFloatLength) {
      return handleDecodeError(DecodeError::VALUE_TOO_LONG);
    }
  }

  if (type == StructuredHeaderItem::Type::INT64) {
    return parseInteger(input, result);
  } else if (input.back() == '.') {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  } else {
    return parseFloat(input, result);
  }

  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseBoolean(
    StructuredHeaderItem& result) {
  if (removeSymbol("?", true) != DecodeError::OK) {
    CHECK(false) << "Only invoked after peeking a '?'";
  }
  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  auto ch = peek();
  if (ch != '0' && ch != '1') {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  }

  result.tag = StructuredHeaderItem::Type::BOOLEAN;
  result.value = static_cast<bool>(ch - '0');
  advanceCursor();
  if (!isEmpty()) {
    return handleDecodeError(DecodeError::VALUE_TOO_LONG);
  }
  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseBinaryContent(
    StructuredHeaderItem& result) {

  std::string outputString;
  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  if (peek() != '*') {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  }

  advanceCursor();

  while (!isEmpty()) {
    char current = peek();
    advanceCursor();
    if (current == '*') {
      if (!isValidEncodedBinaryContent(outputString)) {
        return handleDecodeError(DecodeError::UNDECODEABLE_BINARY_CONTENT);
      }

      std::string decodedContent = decodeBase64(outputString);
      if (encodeBase64(decodedContent) != outputString) {
        return handleDecodeError(DecodeError::UNDECODEABLE_BINARY_CONTENT);
      }

      result.value = std::move(decodedContent);
      result.tag = StructuredHeaderItem::Type::BINARYCONTENT;
      return DecodeError::OK;
    } else if (!isValidEncodedBinaryContentChar(current)) {
      return handleDecodeError(DecodeError::INVALID_CHARACTER);
    } else {
      outputString.push_back(current);
    }
  }

  return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
}

DecodeError StructuredHeadersBuffer::parseIdentifier(
    StructuredHeaderItem& result) {

  std::string outputString;

  auto err = parseIdentifier(outputString);
  if (err != DecodeError::OK) {
    return err;
  }

  result.value = outputString;
  result.tag = StructuredHeaderItem::Type::IDENTIFIER;

  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseIdentifier(std::string& result) {

  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  if (!isLcAlpha(peek())) {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  }

  while (!isEmpty()) {
    char current = peek();
    if (!isValidIdentifierChar(current)) {
      break;
    } else {
      advanceCursor();
      result.push_back(current);
    }
  }

  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseInteger(
    const std::string& input, StructuredHeaderItem& result) {

  try {
    result.value = boost::lexical_cast<int64_t>(input);
    result.tag = StructuredHeaderItem::Type::INT64;
  } catch (boost::bad_lexical_cast&) {
    return handleDecodeError(DecodeError::UNPARSEABLE_NUMERIC_TYPE);
  }
  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseFloat(const std::string& input,
                                                StructuredHeaderItem& result) {

  try {
    result.value = boost::lexical_cast<double>(input);
    result.tag = StructuredHeaderItem::Type::DOUBLE;
  } catch (boost::bad_lexical_cast&) {
    return handleDecodeError(DecodeError::UNPARSEABLE_NUMERIC_TYPE);
  }
  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::parseString(StructuredHeaderItem& result) {

  std::string outputString;

  if (isEmpty()) {
    return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
  }

  if (peek() != '"') {
    return handleDecodeError(DecodeError::INVALID_CHARACTER);
  }

  advanceCursor();

  while (!isEmpty()) {
    char current = peek();
    if (current == '\\') {
      advanceCursor();
      if (isEmpty()) {
        return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
      } else {
        char nextChar = peek();
        advanceCursor();
        if (nextChar != '"' && nextChar != '\\') {
          return handleDecodeError(DecodeError::INVALID_CHARACTER);
        }
        outputString.push_back(nextChar);
      }
    } else if (current == '"') {
      advanceCursor();
      result.value = outputString;
      result.tag = StructuredHeaderItem::Type::STRING;
      return DecodeError::OK;
    } else if (!isValidStringChar(current)) {
      return handleDecodeError(DecodeError::INVALID_CHARACTER);
    } else {
      advanceCursor();
      outputString.push_back(current);
    }
  }

  return handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
}

DecodeError StructuredHeadersBuffer::removeOptionalWhitespace() {
  while (!isEmpty() && (peek() == ' ' || peek() == '\t')) {
    advanceCursor();
  }
  return DecodeError::OK;
}

DecodeError StructuredHeadersBuffer::removeSymbol(const std::string& symbol,
                                                  bool strict) {

  if (content_.startsWith(symbol)) {
    content_.advance(symbol.length());
    return DecodeError::OK;
  } else {
    if (strict) {
      // Do some error logging
      return handleDecodeError(DecodeError::INVALID_CHARACTER);
    }
    return DecodeError::INVALID_CHARACTER;
  }
}

DecodeError StructuredHeadersBuffer::handleDecodeError(const DecodeError& err) {
  LOG_EVERY_N(ERROR, 1000)
      << "Error message: " << decodeErrorDescription.at(err)
      << ". Number of characters parsed before error:" << getNumCharsParsed()
      << ". Header Content:" << originalContent_.str();
  return err;
}

char StructuredHeadersBuffer::peek() {
  return *content_.begin();
}

void StructuredHeadersBuffer::advanceCursor() {
  content_.advance(1);
}

bool StructuredHeadersBuffer::isEmpty() {
  return content_.begin() == content_.end();
}

int32_t StructuredHeadersBuffer::getNumCharsParsed() {
  return std::distance(originalContent_.begin(), content_.begin());
}

} // namespace proxygen
