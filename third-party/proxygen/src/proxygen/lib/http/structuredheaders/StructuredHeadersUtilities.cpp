/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cctype>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersUtilities.h>

namespace proxygen { namespace StructuredHeaders {

bool isLcAlpha(char c) {
  return c >= 0x61 && c <= 0x7A;
}

bool isValidIdentifierChar(char c) {
  return isLcAlpha(c) || std::isdigit(c) || c == '_' || c == '-' || c == '*' ||
         c == '/';
}

bool isValidEncodedBinaryContentChar(char c) {
  return std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/' || c == '=';
}

bool isValidStringChar(char c) {
  /*
   * The difference between the character restriction here and that mentioned
   * in section 3.7 of version 6 of the Structured Headers draft is that this
   * function accepts \ and DQUOTE characters. These characters are allowed
   * as long as they are present as a part of an escape sequence, which is
   * checked for in the parseString() function in the StructuredHeadersBuffer.
   */
  return c >= 0x20 && c <= 0x7E;
}

bool isValidIdentifier(const std::string& s) {
  if (s.size() == 0 || !isLcAlpha(s[0])) {
    return false;
  }

  for (char c : s) {
    if (!isValidIdentifierChar(c)) {
      return false;
    }
  }

  return true;
}

bool isValidString(const std::string& s) {
  for (char c : s) {
    if (!isValidStringChar(c)) {
      return false;
    }
  }
  return true;
}

bool isValidEncodedBinaryContent(const std::string& s) {

  if (s.size() % 4 != 0) {
    return false;
  }

  bool equalSeen = false;
  for (auto it = s.begin(); it != s.end(); it++) {
    if (*it == '=') {
      equalSeen = true;
    } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
      return false;
    }
  }

  return true;
}

bool itemTypeMatchesContent(const StructuredHeaderItem& input) {
  switch (input.tag) {
    case StructuredHeaderItem::Type::BINARYCONTENT:
    case StructuredHeaderItem::Type::IDENTIFIER:
    case StructuredHeaderItem::Type::STRING:
      return input.value.type() == typeid(std::string);
    case StructuredHeaderItem::Type::INT64:
      return input.value.type() == typeid(int64_t);
    case StructuredHeaderItem::Type::BOOLEAN:
      return input.value.type() == typeid(bool);
    case StructuredHeaderItem::Type::DOUBLE:
      return input.value.type() == typeid(double);
    case StructuredHeaderItem::Type::NONE:
      return true;
  }

  return false;
}

}} // namespace proxygen::StructuredHeaders
