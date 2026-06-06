/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/net/NetworkSocket.h>

namespace proxygen::test {

int getTcpMaxSegment(folly::NetworkSocket fd);

int getDefaultLoopbackTcpMaxSegment();

int getDifferentTcpMaxSegment(int defaultTcpMaxSegment);

} // namespace proxygen::test
