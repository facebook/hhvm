/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>

namespace fizz {

template <class T>
using OpenSSLECKeyExchange = openssl::OpenSSLECKeyExchange<T>;

} // namespace fizz
