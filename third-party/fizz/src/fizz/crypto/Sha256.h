/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/Sha256.h>
#include <fizz/crypto/Sha.h>

namespace fizz {

using Sha256 = openssl::Sha256;

} // namespace fizz
