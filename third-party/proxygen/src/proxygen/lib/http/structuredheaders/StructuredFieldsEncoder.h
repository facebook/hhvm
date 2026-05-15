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

class StructuredFieldsEncoder {
 public:
  StructuredFields::EncodeError encodeItem(const StructuredFields::Item& input);

  StructuredFields::EncodeError encodeList(const StructuredFields::List& input);

  StructuredFields::EncodeError encodeDictionary(
      const StructuredFields::Dictionary& input);

  [[nodiscard]] std::string get() const;

 private:
  StructuredFields::EncodeError appendListMember(
      const StructuredFields::ListMember& input);

  StructuredFields::EncodeError appendInnerList(
      const StructuredFields::InnerList& input);

  StructuredFields::EncodeError appendItem(const StructuredFields::Item& input);

  StructuredFields::EncodeError appendParameters(
      const StructuredFields::Parameters& input);

  StructuredFields::EncodeError appendKey(std::string_view input);

  StructuredFields::EncodeError appendBareItem(
      const StructuredFields::BareItem& input);

  StructuredFields::EncodeError appendInteger(int64_t input);

  StructuredFields::EncodeError appendDecimal(StructuredFields::Decimal input);

  StructuredFields::EncodeError appendString(std::string_view input);

  StructuredFields::EncodeError appendToken(std::string_view input);

  StructuredFields::EncodeError appendByteSequence(std::string_view input);

  StructuredFields::EncodeError appendDisplayString(std::string_view input);

  std::string output_;
};

} // namespace proxygen
