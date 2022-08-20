/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>
#include <string>

#pragma once

namespace proxygen { namespace StructuredHeaders {

bool isLcAlpha(char c);

bool isValidIdentifierChar(char c);

bool isValidEncodedBinaryContentChar(char c);

bool isValidStringChar(char c);

bool isValidIdentifier(const std::string& s);

bool isValidString(const std::string& s);

bool isValidEncodedBinaryContent(const std::string& s);

bool itemTypeMatchesContent(const StructuredHeaderItem& input);

std::string decodeBase64(const std::string& encoded);

std::string encodeBase64(const std::string& input);

}} // namespace proxygen::StructuredHeaders
