/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/carbon/ExternalCarbonConnectionStats.h"

namespace carbon {

class NoopExternalConnectionAdditionalLogger {
 public:
  explicit NoopExternalConnectionAdditionalLogger(
      carbon::ExternalCarbonConnectionLoggerOptions&) {}

  void log(carbon::ExternalCarbonConnectionStats&) {}

  bool shouldLog() const {
    return false;
  }

  void setEnabledStatus(bool /* unused */) {}
  bool getEnabledStatus() {
    return false;
  }
};
} // namespace carbon
