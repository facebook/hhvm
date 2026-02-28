/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/test/TestKeys.h>
#include <fizz/protocol/Types.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

// @lint-ignore-every PRIVATEKEY

namespace fizz {
namespace test {

std::unique_ptr<folly::IOBuf> toIOBuf(folly::StringPiece hexData);

folly::ssl::EvpPkeyUniquePtr getPrivateKey(folly::StringPiece key);

folly::ssl::EvpPkeyUniquePtr getPublicKey(folly::StringPiece key);

folly::ssl::X509UniquePtr getCert(folly::StringPiece cert);

std::unique_ptr<folly::IOBuf> getCertData(folly::StringPiece cert);

void useMockRandom();

std::unique_ptr<Aead> getCipher(CipherSuite cipher);

} // namespace test
} // namespace fizz
