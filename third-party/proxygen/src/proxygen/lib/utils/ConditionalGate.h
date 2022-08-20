/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <bitset>
#include <folly/Function.h>
#include <glog/logging.h>
#include <ostream>
#include <vector>

namespace proxygen {

/**
 * ConditionalGate is used to gate the execution of a function or functions
 * on one or more conditions that must be satisfied first.  It's logically
 * equivalent to using promise/future, but without allocation overhead, and
 * designed for single threaded use cases.
 *
 * enum class Things { Thing1, Thing2 };
 *
 * ConditionalGate<Things, 2> thingsDone;
 *
 * thingsDone.then([] { std::cout << "Everything is done"; });
 *
 * thingsDone.set(Things::Thing1);
 * thingsDone.set(Things::Thing2);
 *
 */
template <typename E, size_t N>
class ConditionalGate {
  static_assert(std::is_enum<E>(), "ConditionalGate type must be enum");
  static_assert(N > 0, "N must be greater than 0");

 public:
  /* Return where this gate is open */
  bool allConditionsMet() const {
    return conditions_.all();
  }

  /* Run the function f when all conditions in this gate are true */
  void then(folly::Function<void()> f) {
    if (conditions_.all()) {
      f();
    } else {
      functions_.emplace_back(std::move(f));
    }
  }

  /* Set the condition to true */
  void set(E e) {
    set(static_cast<size_t>(e));
  }
  void set(size_t i = 0) {
    CHECK_LT(i, conditions_.size());
    CHECK(!conditions_[i]);
    conditions_[i] = true;
    if (conditions_.all()) {
      invoke();
    }
  }

  /* Get the current state of the condition */
  bool get(E e) const {
    return get(static_cast<size_t>(e));
  }
  bool get(size_t i = 0) const {
    CHECK_LT(i, conditions_.size());
    return conditions_[i];
  }

  void describe(std::ostream& os) const {
    for (size_t i = 0; i < conditions_.size(); i++) {
      os << i << "=" << static_cast<uint8_t>(conditions_[i]) << " ";
    }
  }

 private:
  void invoke() {
    for (auto& f : functions_) {
      f();
    }
    functions_.clear();
  }

  std::bitset<N> conditions_;
  std::vector<folly::Function<void()>> functions_;
};

template <typename E, size_t N>
inline std::ostream& operator<<(std::ostream& os,
                                const ConditionalGate<E, N>& g) {
  g.describe(os);
  return os;
}

namespace detail {
enum class ReadyEnum { Ready = 0 };
}
/**
 * ReadyGate is a simple specialization of ConditionalGate with a single
 * condition.
 *
 * ReadyGate thingReady_;

 * thingReady_.then([] { std::cout << "It's ready!"; });
 *
 * thingReady_.set();
 */
using ReadyGate = ConditionalGate<detail::ReadyEnum, 1>;

} // namespace proxygen
