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

struct HashTestVector {
  std::string message;
  std::map<fizz::HashFunction, std::string> digest;
};

extern const std::vector<HashTestVector> kHashTestVectors;

void runHashTestWithFizzHasher(
    const fizz::HasherFactoryWithMetadata* makeHasher);
} // namespace fizz::test
