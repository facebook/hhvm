/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace openssl {

class X509ChainWithPkey {
 public:
  virtual ~X509ChainWithPkey() = default;

  /**
   * Returns the X509 certificate chain.
   */
  virtual std::vector<folly::ssl::X509UniquePtr> getX509Chain() const = 0;

  /**
   * Returns the EVP_PKEY private key as a unique pointer.
   */
  virtual folly::ssl::EvpPkeyUniquePtr getEVPPkey() const = 0;
};

} // namespace openssl
} // namespace fizz
