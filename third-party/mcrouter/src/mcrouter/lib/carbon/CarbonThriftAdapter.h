/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

namespace carbon::util {

template <class Carbon, class Thrift>
struct CarbonThriftAdapter {
  static Carbon fromThrift(Thrift value) {
    Carbon result;
    static_cast<Thrift&>(result) = std::move(value);
    return result;
  }

  static const Thrift& toThrift(const Carbon& value) {
    return value;
  }

  static Thrift& toThrift(Carbon& value) {
    return value;
  }
};

} // namespace carbon::util
