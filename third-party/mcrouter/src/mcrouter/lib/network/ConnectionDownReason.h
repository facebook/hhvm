/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace facebook {
namespace memcache {

enum class ConnectionDownReason {
  ERROR,
  ABORTED,
  CONNECT_TIMEOUT,
  CONNECT_ERROR,
  SERVER_GONE_AWAY,
};

} // namespace memcache
} // namespace facebook
