/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/futures/Future.h>

#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Params.h>
#include <fizz/record/RecordLayer.h>
#include <fizz/server/Actions.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/ServerExtensions.h>
#include <fizz/server/State.h>

namespace fizz {
namespace server {

class ServerStateMachine {
 public:
  using StateType = State;
  using ProcessingActions = AsyncActions;
  using CompletedActions = Actions;

  virtual ~ServerStateMachine() = default;

  virtual AsyncActions processAccept(
      const State&,
      folly::Executor* executor,
      std::shared_ptr<const FizzServerContext> context,
      const std::shared_ptr<ServerExtensions>& extensions);

  virtual AsyncActions
  processSocketData(const State&, folly::IOBufQueue&, Aead::AeadOptions);

  virtual AsyncActions processWriteNewSessionTicket(
      const State&,
      WriteNewSessionTicket);

  virtual AsyncActions processAppWrite(const State&, AppWrite);

  virtual AsyncActions processEarlyAppWrite(const State&, EarlyAppWrite);

  virtual Actions processAppClose(const State&);
  virtual Actions processAppCloseImmediate(const State&);

  virtual AsyncActions processKeyUpdateInitiation(
      const State&,
      KeyUpdateInitiation keyUpdateInitiation);
};

namespace detail {

AsyncActions processEvent(const State& state, Param param);

Actions handleError(
    const State& state,
    ReportError error,
    folly::Optional<AlertDescription> alertDesc);

Actions handleAppCloseImmediate(const State& state);
Actions handleAppClose(const State& state);

Actions handleInvalidEvent(const State& state, Event event, Param param);
} // namespace detail

struct ServerTypes {
  using State = fizz::server::State;
  using StateEnum = fizz::server::StateEnum;
  using Event = fizz::Event;
  using Param = fizz::Param;
  using Actions = fizz::server::AsyncActions;
  static constexpr auto NumStates =
      static_cast<std::size_t>(StateEnum::NUM_STATES);
  static constexpr auto NumEvents = static_cast<std::size_t>(Event::NUM_EVENTS);
  static constexpr auto InvalidEventHandler =
      &server::detail::handleInvalidEvent;
};
} // namespace server
} // namespace fizz
