/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/Range.h>
#include <proxygen/lib/http/HTTPMessage.h>

namespace proxygen {

folly::Optional<HTTPPriority> httpPriorityFromString(
    folly::StringPiece priority);

folly::Optional<HTTPPriority> httpPriorityFromHTTPMessage(
    const HTTPMessage& message);

} // namespace proxygen
