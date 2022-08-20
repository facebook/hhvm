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

/**
 * This class defines an interface which allows for Extensions to be extracted
 * from ClientHello without the ServerProtocol needing to know what extensions
 * exactly are being negotiated.
 */
class ServerExtensions {
 public:
  virtual ~ServerExtensions() = default;

  /**
   * Returns a list of extensions to be added to EncryptedExtensions, given a
   * ClientHello.
   */
  virtual std::vector<Extension> getExtensions(const ClientHello& chlo) = 0;
};
} // namespace fizz
