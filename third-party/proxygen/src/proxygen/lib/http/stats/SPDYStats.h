/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/stats/HTTPCodecStats.h>

namespace proxygen {
using SPDYStats = HTTPCodecStats;
using TLSPDYStats = TLHTTPCodecStats;
} // namespace proxygen
