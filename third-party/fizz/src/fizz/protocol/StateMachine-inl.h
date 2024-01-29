/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>

#include <glog/logging.h>

namespace fizz {
namespace sm {

template <typename SM, typename SM::StateEnum s1, typename SM::StateEnum s2>
struct StateSame : std::false_type {};
template <typename SM, typename SM::StateEnum s>
struct StateSame<SM, s, s> : std::true_type {};

template <class... Conditions>
struct Or : std::false_type {};

template <class Condition, class... Conditions>
struct Or<Condition, Conditions...>
    : std::conditional<Condition::value, std::true_type, Or<Conditions...>>::
          type {};

template <
    typename SM,
    typename SM::StateEnum state,
    typename SM::Event event,
    typename SM::StateEnum... AllowedStates>
class EventHandlerBase {
 protected:
  template <typename SM::StateEnum to>
  static void Transition(typename SM::State& stateStruct) {
    static_assert(
        Or<StateSame<SM, to, AllowedStates>...>::value, "Transition invalid");
    CHECK_EQ(stateStruct.state(), state);
    VLOG(8) << "Transition from " << toString(state) << " to " << toString(to);
    stateStruct.state() = to;
  }
};

template <typename SM, typename SM::StateEnum state, typename SM::Event event>
class EventHandler : public EventHandlerBase<SM, state, event> {
 public:
  static typename SM::Actions handle(
      const typename SM::State& curState,
      typename SM::Param param) {
    return SM::InvalidEventHandler(curState, event, std::move(param));
  }
};

#define FIZZ_DECLARE_EVENT_HANDLER(sm, statename, eventname, ...)        \
  template <>                                                            \
  class EventHandler<sm, statename, eventname>                           \
      : public EventHandlerBase<sm, statename, eventname, __VA_ARGS__> { \
   public:                                                               \
    static typename sm::Actions handle(                                  \
        const typename sm::State&,                                       \
        typename sm::Param param);                                       \
  }

template <typename SM>
typename StateMachine<SM>::EventHandlerFun StateMachine<SM>::getHandler(
    typename SM::StateEnum state,
    typename SM::Event event) {
  static constexpr auto handlers = getEventHandlers(
      std::make_index_sequence<SM::NumStates * SM::NumEvents>());
  const auto i = static_cast<std::size_t>(state) * SM::NumEvents +
      static_cast<std::size_t>(event);
  CHECK_LT(i, handlers.size()) << "Out of bounds handler requested";
  return handlers[i];
}

template <typename SM>
template <std::size_t i>
constexpr typename StateMachine<SM>::EventHandlerFun
StateMachine<SM>::getEventHandler() {
  return EventHandler<
      SM,
      static_cast<typename SM::StateEnum>(i / SM::NumEvents),
      static_cast<typename SM::Event>(i % SM::NumEvents)>::handle;
}
} // namespace sm
} // namespace fizz
