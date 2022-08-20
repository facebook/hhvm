/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace facebook {
namespace memcache {

enum class Color : uint8_t {
  DEFAULT,
  BLACK,
  BLUE,
  DARKBLUE,
  CYAN,
  DARKCYAN,
  GRAY,
  DARKGRAY,
  GREEN,
  DARKGREEN,
  MAGENTA,
  DARKMAGENTA,
  RED,
  DARKRED,
  WHITE,
  YELLOW,
  DARKYELLOW
};
}
} // namespace facebook
