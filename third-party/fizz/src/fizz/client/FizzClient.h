/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientProtocol.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/client/PskCache.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/protocol/FizzBase.h>

namespace fizz {
namespace client {

template <typename ActionMoveVisitor, typename SM = ClientStateMachine>
class FizzClient : public FizzBase<
                       FizzClient<ActionMoveVisitor, SM>,
                       ActionMoveVisitor,
                       SM> {
 public:
  using FizzBase<FizzClient<ActionMoveVisitor, SM>, ActionMoveVisitor, SM>::
      FizzBase;

  void connect(
      std::shared_ptr<const FizzClientContext> context,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> sni,
      folly::Optional<CachedPsk> cachedPsk,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
      const std::shared_ptr<ClientExtensions>& extensions = nullptr);

  /**
   * Uses the default verifier to verify certificates
   */
  void connect(
      std::shared_ptr<const FizzClientContext> context,
      folly::Optional<std::string> hostname);

  /**
   * Returns an exported key material derived from the early secret of the TLS
   * connection. Throws if the early secret is not available.
   */
  Buf getEarlyEkm(
      const Factory& factory,
      folly::StringPiece label,
      const Buf& context,
      uint16_t length) const;

 protected:
  void visitActions(typename SM::CompletedActions& actions) override;

 private:
  friend class FizzBase<
      FizzClient<ActionMoveVisitor, SM>,
      ActionMoveVisitor,
      SM>;

  void startActions(Actions actions);
};
} // namespace client
} // namespace fizz

#include <fizz/client/FizzClient-inl.h>
