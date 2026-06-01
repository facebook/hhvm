/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

namespace folly {
class IOThreadPoolExecutorBase;
} // namespace folly

namespace carbon {

struct InternalCarbonConnectionOptions {
  InternalCarbonConnectionOptions() = default;
  size_t maxOutstanding{1024};
  size_t maxOutstandingError{false};
  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreads{nullptr};
};

} // namespace carbon
