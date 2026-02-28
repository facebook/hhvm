/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/KeyDerivation.h>
#include <fizz/protocol/Factory.h>

namespace fizz {

class Exporter {
 public:
  static Buf getExportedKeyingMaterial(
      const Factory& factory,
      CipherSuite cipher,
      folly::ByteRange exporterMaster,
      folly::StringPiece label,
      Buf context,
      uint16_t length);
};
} // namespace fizz
