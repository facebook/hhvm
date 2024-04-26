/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/ECCurve.h>

namespace fizz {

using P256 = openssl::P256;
using P384 = openssl::P384;
using P521 = openssl::P521;

} // namespace fizz
