/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/tokenbinding/Types.h>

namespace fizz {
namespace extensions {

class TokenBindingUtils {
 public:
  static constexpr uint8_t kP256EcKeySize = 64;
  static constexpr uint8_t kEd25519KeySize = 32;

  static Buf constructMessage(
      const TokenBindingType& type,
      const TokenBindingKeyParameters& keyParams,
      const Buf& ekm) {
    Buf concatenatedBuf = folly::IOBuf::create(
        kTokenBindingEkmSize + sizeof(TokenBindingKeyParameters) +
        sizeof(TokenBindingType));
    folly::io::Appender appender(concatenatedBuf.get(), 20);

    detail::write(type, appender);
    detail::write(keyParams, appender);
    appender.push(ekm->coalesce());
    return concatenatedBuf;
  }
};
} // namespace extensions
} // namespace fizz
