/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/coro/HTTPSourceFilter.h>

namespace proxygen::coro {

class FilterFactory {
 public:
  virtual ~FilterFactory() = default;

  /**
   * Returns a pair of filters representing the request and response filters
   * respectively. The server installs these filters into the request and
   * reponse paths in order. Any of the pair of filters can be nullptr and will
   * omit installing that filter in the chain.
   */
  using HTTPSourceFilter = proxygen::coro::HTTPSourceFilter;
  virtual std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() = 0;
};

} // namespace proxygen::coro
