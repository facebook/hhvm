/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>

namespace proxygen::StructuredHeaders {

std::string_view decodeErrToString(DecodeError err) {
  switch (err) {
    case DecodeError::OK: {
      return "No error";
    }
    case DecodeError::VALUE_TOO_LONG: {
      return "Numeric value is too long";
    }
    case DecodeError::INVALID_CHARACTER: {
      return "Invalid character";
    }
    case DecodeError::UNDECODEABLE_BINARY_CONTENT: {
      return "Undecodable binary content";
    }
    case DecodeError::UNEXPECTED_END_OF_BUFFER: {
      return "Unexpected end of buffer";
    }
    case DecodeError::UNPARSEABLE_NUMERIC_TYPE: {
      return "Unparseable numeric type";
    }
    case DecodeError::DUPLICATE_KEY: {
      return "Duplicate key found";
    }
    default:
      return "Unknown error";
  }
}

std::string_view encodeErrToString(EncodeError err) {
  switch (err) {
    case EncodeError::OK: {
      return "No error";
    }
    case EncodeError::EMPTY_DATA_STRUCTURE: {
      return "Empty data structure";
    }
    case EncodeError::BAD_IDENTIFIER: {
      return "Bad identifier";
    }
    case EncodeError::BAD_STRING: {
      return "Bad string";
    }
    case EncodeError::ITEM_TYPE_MISMATCH: {
      return "Item type mismatch";
    }
    case EncodeError::ENCODING_NULL_ITEM: {
      return "Tried to encode null item";
    }
    default:
      return "Unknown error";
  }
}

} // namespace proxygen::StructuredHeaders
