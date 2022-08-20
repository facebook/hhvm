/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <glog/logging.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersEncoder.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersUtilities.h>

namespace proxygen {

using namespace StructuredHeaders;

EncodeError StructuredHeadersEncoder::encodeList(
    const std::vector<StructuredHeaderItem>& input) {

  if (input.empty()) {
    return handleEncodeError(EncodeError::EMPTY_DATA_STRUCTURE);
  }

  for (auto it = input.begin(); it != input.end(); it++) {
    auto err = encodeItem(*it);
    if (err != EncodeError::OK) {
      return err;
    }

    if (std::next(it, 1) != input.end()) {
      outputStream_ << ", ";
    }
  }

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeDictionary(
    const Dictionary& input) {

  if (input.empty()) {
    return handleEncodeError(EncodeError::EMPTY_DATA_STRUCTURE);
  }

  for (auto it = input.begin(); it != input.end(); it++) {
    auto err = encodeIdentifier(it->first);
    if (err != EncodeError::OK) {
      return err;
    }

    if (!itemTypeMatchesContent(it->second)) {
      return handleEncodeError(EncodeError::ITEM_TYPE_MISMATCH);
    }
    if (!skipBoolean(it->second)) {
      outputStream_ << "=";
      err = encodeItem(it->second);
      if (err != EncodeError::OK) {
        return err;
      }
    }

    if (std::next(it, 1) != input.end()) {
      outputStream_ << ", ";
    }
  }

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeParameterisedList(
    const ParameterisedList& input) {

  if (input.empty()) {
    return handleEncodeError(EncodeError::EMPTY_DATA_STRUCTURE);
  }

  for (auto it1 = input.begin(); it1 != input.end(); it1++) {
    auto err = encodeIdentifier(it1->identifier);
    if (err != EncodeError::OK) {
      return err;
    }

    for (auto it2 = it1->parameterMap.begin(); it2 != it1->parameterMap.end();
         it2++) {

      outputStream_ << "; ";

      err = encodeIdentifier(it2->first);
      if (err != EncodeError::OK) {
        return err;
      }

      if (it2->second.tag != StructuredHeaderItem::Type::NONE) {
        if (!itemTypeMatchesContent(it2->second)) {
          return handleEncodeError(EncodeError::ITEM_TYPE_MISMATCH);
        }
        if (!skipBoolean(it2->second)) {
          outputStream_ << "=";
          err = encodeItem(it2->second);
        }

        if (err != EncodeError::OK) {
          return err;
        }
      }
    }

    if (std::next(it1, 1) != input.end()) {
      outputStream_ << ", ";
    }
  }

  return EncodeError::OK;
}

StructuredHeadersEncoder::StructuredHeadersEncoder()
    : output_(), buf_(output_), outputStream_(&buf_) {
  outputStream_.precision(kMaxValidFloatLength - 1);
}

EncodeError StructuredHeadersEncoder::encodeItem(
    const StructuredHeaderItem& input) {

  if (!itemTypeMatchesContent(input)) {
    return handleEncodeError(EncodeError::ITEM_TYPE_MISMATCH);
  }

  switch (input.tag) {
    case StructuredHeaderItem::Type::STRING:
      return encodeString(boost::get<std::string>(input.value));
    case StructuredHeaderItem::Type::INT64:
      return encodeInteger(boost::get<int64_t>(input.value));
    case StructuredHeaderItem::Type::BOOLEAN:
      return encodeBoolean(boost::get<bool>(input.value));
    case StructuredHeaderItem::Type::DOUBLE:
      return encodeFloat(boost::get<double>(input.value));
    case StructuredHeaderItem::Type::BINARYCONTENT:
      return encodeBinaryContent(boost::get<std::string>(input.value));
    default:
      return handleEncodeError(EncodeError::ENCODING_NULL_ITEM);
  }

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeBinaryContent(
    const std::string& input) {

  outputStream_ << "*";
  outputStream_ << encodeBase64(input);
  outputStream_ << "*";

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeString(const std::string& input) {

  if (!isValidString(input)) {
    return handleEncodeError(EncodeError::BAD_STRING, input);
  }

  outputStream_ << "\"";
  for (char c : input) {
    if (c == '"' || c == '\\') {
      outputStream_ << "\\";
    }
    outputStream_ << c;
  }

  outputStream_ << "\"";

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeInteger(int64_t input) {

  outputStream_ << input;

  return EncodeError::OK;
}

bool StructuredHeadersEncoder::skipBoolean(const StructuredHeaderItem& input) {
  return input.tag == StructuredHeaderItem::Type::BOOLEAN &&
         boost::get<bool>(input.value);
}

EncodeError StructuredHeadersEncoder::encodeBoolean(bool input) {

  outputStream_ << '?' << (input ? '1' : '0');

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeFloat(double input) {

  outputStream_ << input;

  return EncodeError::OK;
}

EncodeError StructuredHeadersEncoder::encodeIdentifier(
    const std::string& input) {

  if (!isValidIdentifier(input)) {
    return handleEncodeError(EncodeError::BAD_IDENTIFIER, input);
  }
  outputStream_ << input;
  return EncodeError::OK;
}

// Used to print an error when a string type (eg: a string or binary content)
// was involved in the error
EncodeError StructuredHeadersEncoder::handleEncodeError(
    EncodeError err, const std::string& culprit) {

  LOG_EVERY_N(ERROR, 1000)
      << "Error message: " << encodeErrorDescription.at(err)
      << " .The culprit was: " << culprit;
  return err;
}

// Used to print more general error messages (eg: empty data structure)
EncodeError StructuredHeadersEncoder::handleEncodeError(EncodeError err) {
  LOG_EVERY_N(ERROR, 1000) << "Error message: "
                           << encodeErrorDescription.at(err);
  return err;
}

std::string StructuredHeadersEncoder::get() {
  outputStream_.flush();
  return std::move(output_);
}

} // namespace proxygen
