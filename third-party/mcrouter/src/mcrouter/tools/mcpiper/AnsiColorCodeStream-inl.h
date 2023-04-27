/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace facebook {
namespace memcache {

template <class T>
void AnsiColorCodeEncoder::writePlain(const T& t) {
  if (!isReset_) {
    reset();
  }

  out_ << t;
}
} // namespace memcache
} // namespace facebook
