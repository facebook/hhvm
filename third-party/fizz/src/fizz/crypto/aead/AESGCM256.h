/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/aead/AESGCM256.h>

namespace fizz {
using AESGCM256 = openssl::AESGCM256;
} // namespace fizz
