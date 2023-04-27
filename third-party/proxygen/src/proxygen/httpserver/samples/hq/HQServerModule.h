/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/samples/hq/HQCommandLine.h>

namespace quic::samples {

void startServer(const HQToolServerParams& params);

} // namespace quic::samples
