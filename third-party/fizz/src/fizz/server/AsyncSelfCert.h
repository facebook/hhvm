/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Certificate.h>
#include <folly/futures/Future.h>

namespace fizz {
namespace server {
class State;
}

/**
 * SelfCert with an asynchronous sign method. This is useful when the private
 * key for a certificate is not locally available.
 */
class AsyncSelfCert : public SelfCert {
 public:
  virtual folly::SemiFuture<folly::Optional<Buf>> signFuture(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      std::unique_ptr<folly::IOBuf> toBeSigned) const = 0;
};
} // namespace fizz
