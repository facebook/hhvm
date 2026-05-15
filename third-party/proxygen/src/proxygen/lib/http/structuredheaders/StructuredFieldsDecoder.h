/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/structuredheaders/StructuredFields.h>

#include <string>
#include <string_view>

namespace proxygen {

class StructuredFieldsDecoder {
 public:
  explicit StructuredFieldsDecoder(std::string_view input) : input_(input) {
  }

  StructuredFields::DecodeError decodeItem(StructuredFields::Item& result);

  StructuredFields::DecodeError decodeList(StructuredFields::List& result);

  StructuredFields::DecodeError decodeDictionary(
      StructuredFields::Dictionary& result);

 private:
  StructuredFields::DecodeError parseItemOrInnerList(
      StructuredFields::ListMember& result);

  StructuredFields::DecodeError parseInnerList(
      StructuredFields::InnerList& result);

  StructuredFields::DecodeError parseItem(StructuredFields::Item& result);

  StructuredFields::DecodeError parseBareItem(
      StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseParameters(
      StructuredFields::Parameters& result);

  StructuredFields::DecodeError parseKey(std::string& result);

  StructuredFields::DecodeError parseNumber(StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseString(StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseToken(StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseByteSequence(
      StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseBoolean(
      StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseDate(StructuredFields::BareItem& result);

  StructuredFields::DecodeError parseDisplayString(
      StructuredFields::BareItem& result);

  StructuredFields::DecodeError finishTopLevel();

  void removeSP();

  void removeOWS();

  bool consume(char expected);

  bool consumeChar(char& result);

  [[nodiscard]] bool empty() const;

  [[nodiscard]] char peek() const;

  void advance();

  [[nodiscard]] bool isAsciiInput() const;

  std::string input_;
  size_t pos_{0};
};

} // namespace proxygen
