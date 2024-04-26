/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/exchange/ECCurveKeyExchange.h>
#include <fizz/crypto/ECCurve.h>
#include <fizz/crypto/exchange/OpenSSLKeyExchange.h>

namespace fizz {

using P256KeyExchange = openssl::P256KeyExchange;
using P256PublicKeyDecoder = openssl::P256PublicKeyDecoder;
using P256PublicKeyEncoder = openssl::P256PublicKeyEncoder;

using P384KeyExchange = openssl::P384KeyExchange;
using P384PublicKeyDecoder = openssl::P384PublicKeyDecoder;
using P384PublicKeyEncoder = openssl::P384PublicKeyEncoder;

using P521KeyExchange = openssl::P521KeyExchange;
using P521PublicKeyDecoder = openssl::P521PublicKeyDecoder;
using P521PublicKeyEncoder = openssl::P521PublicKeyEncoder;

} // namespace fizz
