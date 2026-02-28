/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/FizzBase.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/ServerProtocol.h>

namespace fizz {
namespace server {

bool looksLikeV2ClientHello(const folly::IOBufQueue& queue);

template <typename ActionMoveVisitor, typename SM = ServerStateMachine>
class FizzServer : public FizzBase<
                       FizzServer<ActionMoveVisitor, SM>,
                       ActionMoveVisitor,
                       SM> {
 public:
  using FizzBase<FizzServer<ActionMoveVisitor, SM>, ActionMoveVisitor, SM>::
      FizzBase;

  void accept(
      folly::Executor* executor,
      std::shared_ptr<const FizzServerContext> context,
      std::shared_ptr<ServerExtensions> extensions = nullptr);

  void newTransportData();

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
      FizzServer<ActionMoveVisitor, SM>,
      ActionMoveVisitor,
      SM>;

  void startActions(AsyncActions actions);

  bool checkV2Hello_{false};
};
} // namespace server
} // namespace fizz

#include <fizz/server/FizzServer-inl.h>
