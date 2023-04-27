/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/ECCurve.h>
#include <fizz/crypto/exchange/OpenSSLKeyExchange.h>
#include <folly/io/IOBuf.h>

namespace fizz {

using P256KeyExchange = OpenSSLECKeyExchange<P256>;
using P256PublicKeyDecoder = detail::OpenSSLECKeyDecoder<P256>;
using P256PublicKeyEncoder = detail::OpenSSLECKeyEncoder;

using P384KeyExchange = OpenSSLECKeyExchange<P384>;
using P384PublicKeyDecoder = detail::OpenSSLECKeyDecoder<P384>;
using P384PublicKeyEncoder = detail::OpenSSLECKeyEncoder;

using P521KeyExchange = OpenSSLECKeyExchange<P521>;
using P521PublicKeyDecoder = detail::OpenSSLECKeyDecoder<P521>;
using P521PublicKeyEncoder = detail::OpenSSLECKeyEncoder;
} // namespace fizz
