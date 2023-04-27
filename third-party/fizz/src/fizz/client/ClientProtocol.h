/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/Actions.h>
#include <fizz/client/ClientExtensions.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/client/PskCache.h>
#include <fizz/client/State.h>
#include <fizz/protocol/ech/Types.h>

namespace fizz {
namespace client {

class ClientStateMachine {
 public:
  using StateType = State;
  using ProcessingActions = Actions;
  using CompletedActions = Actions;

  virtual ~ClientStateMachine() = default;

  virtual Actions processConnect(
      const State&,
      std::shared_ptr<const FizzClientContext> context,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> sni,
      folly::Optional<CachedPsk> cachedPsk,
      const std::shared_ptr<ClientExtensions>& extensions,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs);

  virtual Actions
  processSocketData(const State&, folly::IOBufQueue&, Aead::AeadOptions);

  virtual Actions processWriteNewSessionTicket(
      const State&,
      WriteNewSessionTicket);

  virtual Actions processAppWrite(const State&, AppWrite);

  virtual Actions processEarlyAppWrite(const State&, EarlyAppWrite);

  virtual Actions processAppClose(const State&);

  virtual Actions processAppCloseImmediate(const State&);

  virtual Actions processKeyUpdateInitiation(
      const State&,
      KeyUpdateInitiation keyUpdateInitiation);
};

namespace detail {

Actions processEvent(const State& state, Param param);

Actions handleError(
    const State& state,
    ReportError error,
    folly::Optional<AlertDescription> alertDesc);

Actions handleAppCloseImmediate(const State& state);
Actions handleAppClose(const State& state);

Actions handleInvalidEvent(const State& state, Event event, Param param);
} // namespace detail

struct ClientTypes {
  using State = fizz::client::State;
  using StateEnum = fizz::client::StateEnum;
  using Event = fizz::Event;
  using Param = fizz::Param;
  using Actions = fizz::client::Actions;
  static constexpr auto NumStates =
      static_cast<std::size_t>(fizz::client::StateEnum::NUM_STATES);
  static constexpr auto NumEvents = static_cast<std::size_t>(Event::NUM_EVENTS);
  static constexpr auto InvalidEventHandler =
      &client::detail::handleInvalidEvent;
};
} // namespace client
} // namespace fizz
