/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/crypto/aead/Aead.h>

namespace fizz {
struct AEGIS {
  /**
   * Returns an AEAD implementing AEGIS-128L.
   *
   * Returns null if fizz was compiled without AEGIS support.
   */
  static std::unique_ptr<Aead> make128L();

  /**
   * Returns an AEAD implementing AEGIS-256
   *
   * Returns null if fizz was compiled without AEGIS support.
   */
  static std::unique_ptr<Aead> make256();
};
} // namespace fizz
