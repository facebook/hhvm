/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gflags/gflags.h>

DECLARE_int64(async_mysql_connect_tcp_timeout_micros);
DECLARE_int64(async_mysql_connect_timeout_micros);
DECLARE_int64(async_mysql_max_connect_timeout_micros);
DECLARE_int64(async_mysql_timeout_micros);
