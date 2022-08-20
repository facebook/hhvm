/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <ostream>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace proxygen {

using namespace StructuredHeaders;

using string_buf = boost::iostreams::stream_buffer<
    boost::iostreams::back_insert_device<std::string>>;

class StructuredHeadersEncoder {

 public:
  StructuredHeadersEncoder();

  EncodeError encodeParameterisedList(const ParameterisedList& input);

  EncodeError encodeDictionary(const Dictionary& input);

  EncodeError encodeList(const std::vector<StructuredHeaderItem>& input);

  EncodeError encodeItem(const StructuredHeaderItem& input);

  EncodeError encodeIdentifier(const std::string& input);

  std::string get();

 private:
  EncodeError encodeBinaryContent(const std::string& input);

  EncodeError encodeString(const std::string& input);

  EncodeError encodeInteger(int64_t input);

  EncodeError encodeBoolean(bool input);

  bool skipBoolean(const StructuredHeaderItem& input);

  EncodeError encodeFloat(double input);

  EncodeError handleEncodeError(EncodeError err, const std::string& badContent);

  EncodeError handleEncodeError(EncodeError err);

  std::string output_;
  string_buf buf_;
  std::ostream outputStream_;
};

} // namespace proxygen
