/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/aead/AESGCM128.h>

namespace fizz {
using AESGCM128 = openssl::AESGCM128;
} // namespace fizz
