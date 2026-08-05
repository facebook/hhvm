/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/util/Status.h>
#include <folly/Range.h>

#include <string>

namespace fizz {

/**
 * Convert binary data to its lowercase hexadecimal representation, returned as
 * a string.
 *
 * Mirrors folly::hexlify.
 */
std::string hexlify(folly::ByteRange input);

/**
 * Convert a hexadecimal string back to binary data.
 *
 * Mirrors the name and behavior of folly::unhexlify. Both lowercase and
 * uppercase hex digits are accepted. Fails (returning Status::Fail and
 * populating `err`) if the input has an odd length or contains a non-hex
 * character.
 *
 * The decoded bytes are written to `ret`, replacing any prior contents.
 */
Status unhexlify(std::string& ret, Error& err, folly::StringPiece input);

} // namespace fizz
