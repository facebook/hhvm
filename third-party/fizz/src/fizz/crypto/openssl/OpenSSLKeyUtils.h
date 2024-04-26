/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/crypto/OpenSSLKeyUtils.h>
#include <fizz/crypto/openssl/OpenSSL.h>

namespace fizz {
using OpenSSLKeyUtils = openssl::OpenSSLKeyUtils;

namespace detail {

CREATE_FIZZ_FN_ALIAS(validateECKey, openssl::detail::validateECKey)

#if FIZZ_OPENSSL_HAS_ED25519
CREATE_FIZZ_FN_ALIAS(validateEdKey, openssl::detail::validateEdKey)
#endif

CREATE_FIZZ_FN_ALIAS(generateECKeyPair, openssl::detail::generateECKeyPair)
CREATE_FIZZ_FN_ALIAS(decodeECPublicKey, openssl::detail::decodeECPublicKey)
CREATE_FIZZ_FN_ALIAS(encodeECPublicKey, openssl::detail::encodeECPublicKey)
CREATE_FIZZ_FN_ALIAS(
    generateEvpSharedSecret,
    openssl::detail::generateEvpSharedSecret)
CREATE_FIZZ_FN_ALIAS(getOpenSSLError, openssl::detail::getOpenSSLError)

} // namespace detail
} // namespace fizz
