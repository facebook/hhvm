/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <fizz/record/Types.h>

namespace fizz::test {

struct SignatureTestData {
  fizz::SignatureScheme sigScheme;
  std::string sig;
  bool validSig;
  std::string certDer;
  bool validCert;
  std::string msg;
  std::string msgHash;
};

// test data created using openssl cli
extern const std::vector<SignatureTestData> kSignatureTestVectors;
} // namespace fizz::test
