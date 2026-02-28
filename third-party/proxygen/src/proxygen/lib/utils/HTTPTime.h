/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <stddef.h>
#include <string>

namespace proxygen {

folly::Optional<int64_t> parseHTTPDateTime(const std::string& s);

} // namespace proxygen
