/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>

namespace fizz {

/*
 * This class allows for Extensions to be added to Client Hello, and then
 * checked against EncryptedExtensions negotiated by the server.
 */
class ClientExtensions {
 public:
  virtual ~ClientExtensions() = default;

  /**
   * Returns a list of extensions to be added to the ClientHello. Note that this
   * can be called multiple times on a single connection (if a HelloRetryRequest
   * is received).
   */
  virtual std::vector<Extension> getClientHelloExtensions() const = 0;

  /**
   * Called with the extensions present in EncryptedExtensions.
   */
  virtual void onEncryptedExtensions(
      const std::vector<Extension>& extensions) = 0;
};
} // namespace fizz
