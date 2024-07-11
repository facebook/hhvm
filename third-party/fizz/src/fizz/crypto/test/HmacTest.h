/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/crypto/Hasher.h>
#include <fizz/protocol/Types.h>
#include <map>

namespace fizz::test {

struct HmacTestVector {
  std::string key;
  std::string data;
  std::map<fizz::HashFunction, std::string> hmac;
  std::optional<size_t> truncatedOutSize;
};

extern const std::vector<HmacTestVector> kHmacTestVectors;

void runHmacTest(fizz::HashFunction digestType, fizz::HasherFactory makeHasher);
} // namespace fizz::test
