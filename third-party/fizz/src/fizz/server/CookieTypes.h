/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/protocol/Types.h>
#include <fizz/server/AeadCookieCipher.h>

namespace fizz {
namespace server {
using AES128CookieCipher = AeadCookieCipher;
}
} // namespace fizz
