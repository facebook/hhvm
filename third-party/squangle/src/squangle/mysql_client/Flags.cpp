/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Flags.h"

DEFINE_int64(
    async_mysql_connect_timeout_micros,
    1030 * 1000,
    "default timeout, in micros, for mysql connect");

DEFINE_int64(
    async_mysql_timeout_micros,
    10 * 1000 * 1000,
    "default timeout, in micros, for mysql operations");

// Default timeout of 0 is no-op for connect tcp timeout
DEFINE_int64(
    async_mysql_connect_tcp_timeout_micros,
    0,
    "default timeout, in micros, for mysql connect");

DEFINE_int64(
    async_mysql_max_connect_timeout_micros,
#if defined(FOLLY_SANITIZE_ADDRESS) || (FOLLY_SANITIZE_THREAD)
    30 * 1000 * 1000,
#else
    3 * 1000 * 1000,
#endif
    "The maximum connect timeout, to protect customers from themselves");
