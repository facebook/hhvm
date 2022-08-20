/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <cstdlib>
#include <utility>

namespace fizz {
namespace sm {

template <typename SM>
class StateMachine {
 public:
  using EventHandlerFun =
      typename SM::Actions (*)(const typename SM::State&, typename SM::Param);

  /**
   * Returns the appropriate event handler for event in state.
   */
  static EventHandlerFun getHandler(
      typename SM::StateEnum state,
      typename SM::Event event);

 private:
  template <std::size_t i>
  static constexpr EventHandlerFun getEventHandler();

  template <std::size_t... Indices>
  static constexpr std::array<EventHandlerFun, sizeof...(Indices)>
  getEventHandlers(std::index_sequence<Indices...>) {
    return {{getEventHandler<Indices>()...}};
  }
};
} // namespace sm
} // namespace fizz

#include <fizz/protocol/StateMachine-inl.h>
