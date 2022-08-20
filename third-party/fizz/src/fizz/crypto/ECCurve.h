/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/evp.h>

namespace fizz {

struct P256 {
  static const int curveNid{NID_X9_62_prime256v1};
  /**
   * See RFC8446 Section 4.2.8.2
   */
  static const int coordinateLength = 32;
  static const int keyShareLength = coordinateLength * 2 + 1;
};

struct P384 {
  static const int curveNid{NID_secp384r1};
  static const int coordinateLength = 48;
  static const int keyShareLength = coordinateLength * 2 + 1;
};

struct P521 {
  static const int curveNid{NID_secp521r1};
  static const int coordinateLength = 66;
  static const int keyShareLength = coordinateLength * 2 + 1;
};

} // namespace fizz
