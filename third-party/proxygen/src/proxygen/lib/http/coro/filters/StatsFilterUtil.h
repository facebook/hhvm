/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

namespace proxygen {
class HttpServerStatsIf;
}

namespace proxygen::coro {

class HTTPSourceFilter;

struct StatsFilterUtil {
  static std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters(
      HttpServerStatsIf* stats);
};

} // namespace proxygen::coro
