/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>

namespace fizz {
namespace extensions {
struct Padding {
  uint16_t total_bytes;
  static constexpr ExtensionType extension_type = ExtensionType::padding;
};
} // namespace extensions
} // namespace fizz
