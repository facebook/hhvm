/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

namespace fizz {

namespace detail {
// A hack to workaround Boost 1.70 apply_visitor regression:
// https://github.com/boostorg/variant/issues/69
template <class R>
decltype(auto) result_type() {
  struct Inner {
   private:
    struct Uninstantiable {};

   public:
    using result_type = R;
    result_type operator()(Uninstantiable);
  };

  return Inner{};
}
} // namespace detail
} // namespace fizz
