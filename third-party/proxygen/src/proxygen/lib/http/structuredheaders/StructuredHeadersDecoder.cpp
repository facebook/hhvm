/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>

namespace proxygen {

using namespace StructuredHeaders;

DecodeError StructuredHeadersDecoder::decodeItem(StructuredHeaderItem& result) {
  auto err = buf_.parseItem(result);
  if (err != DecodeError::OK) {
    return err;
  }
  return buf_.isEmpty()
             ? DecodeError::OK
             : buf_.handleDecodeError(DecodeError::INVALID_CHARACTER);
}

DecodeError StructuredHeadersDecoder::decodeList(
    std::vector<StructuredHeaderItem>& result) {

  while (!buf_.isEmpty()) {

    StructuredHeaderItem item;
    auto err = buf_.parseItem(item);
    if (err != DecodeError::OK) {
      return err;
    }

    result.push_back(item);

    buf_.removeOptionalWhitespace();

    if (buf_.isEmpty()) {
      return DecodeError::OK;
    }

    err = buf_.removeSymbol(",", true);
    if (err != DecodeError::OK) {
      return err;
    }

    buf_.removeOptionalWhitespace();

    if (buf_.isEmpty()) {
      return buf_.handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
    }
  }

  return buf_.handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
}

DecodeError StructuredHeadersDecoder::decodeDictionary(Dictionary& result) {
  return decodeMap(result, MapType::DICTIONARY);
}

DecodeError StructuredHeadersDecoder::decodeParameterisedList(
    ParameterisedList& result) {

  while (!buf_.isEmpty()) {

    ParameterisedIdentifier primaryIdentifier;

    auto err = buf_.parseIdentifier(primaryIdentifier.identifier);
    if (err != DecodeError::OK) {
      return err;
    }

    buf_.removeOptionalWhitespace();

    err = decodeMap(primaryIdentifier.parameterMap, MapType::PARAMETERISED_MAP);
    if (err != DecodeError::OK) {
      return err;
    }

    result.emplace_back(primaryIdentifier);

    buf_.removeOptionalWhitespace();

    if (buf_.isEmpty()) {
      return DecodeError::OK;
    }

    err = buf_.removeSymbol(",", true);
    if (err != DecodeError::OK) {
      return err;
    }

    buf_.removeOptionalWhitespace();
  }

  return buf_.handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
}

DecodeError StructuredHeadersDecoder::decodeMap(
    std::unordered_map<std::string, StructuredHeaderItem>& result,
    MapType mapType) {

  std::string delimiter = (mapType == MapType::PARAMETERISED_MAP) ? ";" : ",";

  buf_.removeOptionalWhitespace();

  if ((mapType == MapType::PARAMETERISED_MAP) &&
      (buf_.removeSymbol(delimiter, false) != DecodeError::OK)) {
    return DecodeError::OK;
  }

  while (!buf_.isEmpty()) {

    buf_.removeOptionalWhitespace();

    std::string thisKey;
    auto err = buf_.parseIdentifier(thisKey);
    if (err != DecodeError::OK) {
      return err;
    }

    if (result.find(thisKey) != result.end()) {
      return buf_.handleDecodeError(DecodeError::DUPLICATE_KEY);
    }

    err = buf_.removeSymbol("=", false /* strict */);
    if (err != DecodeError::OK) {
      StructuredHeaderItem value;
      value.tag = StructuredHeaderItem::Type::BOOLEAN;
      value.value = true;
      result[thisKey] = value;
    } else {
      StructuredHeaderItem value;
      err = buf_.parseItem(value);
      if (err != DecodeError::OK) {
        return err;
      }

      result[thisKey] = value;
    }

    if (buf_.isEmpty()) {
      return DecodeError::OK;
    }

    buf_.removeOptionalWhitespace();

    err = buf_.removeSymbol(delimiter, mapType == MapType::DICTIONARY);
    if (err != DecodeError::OK) {
      if (mapType == MapType::PARAMETERISED_MAP) {
        return DecodeError::OK;
      } else {
        return err;
      }
    }
  }

  return buf_.handleDecodeError(DecodeError::UNEXPECTED_END_OF_BUFFER);
}

} // namespace proxygen
