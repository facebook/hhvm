/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/aead/AESGCM128.h>

namespace fizz {

using OpenSSLEVPCipher = openssl::OpenSSLEVPCipher;

} // namespace fizz
