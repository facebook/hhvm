/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/DecompressionFilterFactory.h"
#include "proxygen/lib/http/coro/filters/DecompressionFilter.h"

namespace proxygen::coro {

namespace {
template <typename Filter>
Filter* makeDecompressionFilter() {
  auto filter = std::make_unique<Filter>(/*source=*/nullptr);
  XCHECK(filter);
  filter->setHeapAllocated();
  return filter.release();
}
} // namespace

std::pair<HTTPSourceFilter*, HTTPSourceFilter*>
ClientDecompressionFilterFactory::makeFilters() {
  return {
      /* RequestFilter = */ makeDecompressionFilter<
          DecompressionEgressFilter>(),
      /* ResponseFilter = */
      makeDecompressionFilter<DecompressionIngressFilter>(),
  };
}

std::pair<HTTPSourceFilter*, HTTPSourceFilter*>
ServerDecompressionFilterFactory::makeFilters() {
  return {
      /* RequestFilter = */ makeDecompressionFilter<
          DecompressionIngressFilter>(),
      /* ResponseFilter = */ nullptr,
  };
}

} // namespace proxygen::coro
