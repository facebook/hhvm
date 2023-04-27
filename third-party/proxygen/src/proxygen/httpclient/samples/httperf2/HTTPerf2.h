/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>

DECLARE_string(ip);
DECLARE_string(server);
DECLARE_int32(port);

namespace proxygen {

int httperf2(folly::Optional<folly::SocketAddress> bindAddr = folly::none);

}
