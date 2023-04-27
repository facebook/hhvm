/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>
#include <string>

namespace proxygen {

using namespace StructuredHeaders;

class StructuredHeadersBuffer {
 public:
  explicit StructuredHeadersBuffer(const std::string& s)
      : content_(s), originalContent_(s) {
  }

  explicit StructuredHeadersBuffer(folly::StringPiece s)
      : content_(s), originalContent_(s) {
  }

  /*
   * helper functions used to extract various lower-level items from a sequence
   * of bytes. These will be called from higher level functions which parse
   * dictionaries, lists, and other data structures.
   */

  StructuredHeaders::DecodeError parseIdentifier(StructuredHeaderItem& result);

  StructuredHeaders::DecodeError parseIdentifier(std::string& result);

  StructuredHeaders::DecodeError parseItem(StructuredHeaderItem& result);

  DecodeError removeSymbol(const std::string& symbol, bool strict);

  DecodeError removeOptionalWhitespace();

  bool isEmpty();

  DecodeError handleDecodeError(const DecodeError& err);

 private:
  DecodeError parseBinaryContent(StructuredHeaderItem& result);

  DecodeError parseNumber(StructuredHeaderItem& result);

  DecodeError parseBoolean(StructuredHeaderItem& result);

  DecodeError parseString(StructuredHeaderItem& result);

  DecodeError parseInteger(const std::string& input,
                           StructuredHeaderItem& result);

  DecodeError parseFloat(const std::string& input,
                         StructuredHeaderItem& result);

  char peek();

  void advanceCursor();

  int32_t getNumCharsParsed();

  folly::StringPiece content_;
  folly::StringPiece originalContent_;
};

} // namespace proxygen
