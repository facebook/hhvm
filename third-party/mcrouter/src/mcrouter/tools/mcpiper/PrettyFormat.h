/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/tools/mcpiper/Color.h"

namespace facebook {
namespace memcache {

/**
 * Pretty format, used to color StyledString.
 */
struct PrettyFormat {
  Color attrColor = Color::MAGENTA;
  Color dataOpColor = Color::DARKYELLOW;
  Color dataKeyColor = Color::YELLOW;
  Color dataValueColor = Color::DARKCYAN;
  Color headerColor = Color::WHITE;
  Color matchColor = Color::RED;
  Color msgAttrColor = Color::GREEN;
};
} // namespace memcache
} // namespace facebook
