/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/crypto/signature/Signature.h>
#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>

namespace fizz {
using KeyType = openssl::KeyType;

template <KeyType T>
using OpenSSLSignature = openssl::OpenSSLSignature<T>;

template <SignatureScheme Scheme>
using SigAlg = openssl::SigAlg<Scheme>;

namespace detail {
CREATE_FIZZ_FN_ALIAS(ecSign, openssl::detail::ecSign)
CREATE_FIZZ_FN_ALIAS(ecVerify, openssl::detail::ecVerify)

#if FIZZ_OPENSSL_HAS_ED25519
CREATE_FIZZ_FN_ALIAS(edSign, openssl::detail::edSign)
CREATE_FIZZ_FN_ALIAS(edVerify, openssl::detail::edVerify)
#endif

CREATE_FIZZ_FN_ALIAS(rsaPssSign, openssl::detail::rsaPssSign)
CREATE_FIZZ_FN_ALIAS(rsaPssVerify, openssl::detail::rsaPssVerify)
} // namespace detail

} // namespace fizz
