/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "eden/common/telemetry/ScribeLogger.h"
#include "eden/common/telemetry/ScubaStructuredLogger.h"
#include "eden/common/telemetry/SessionInfo.h"
#include "eden/common/telemetry/StructuredLogger.h"

namespace watchman {

using DynamicEvent = facebook::eden::DynamicEvent;
using SessionInfo = facebook::eden::SessionInfo;
using ScribeLogger = facebook::eden::ScribeLogger;
using ScubaStructuredLogger = facebook::eden::ScubaStructuredLogger;
using StructuredLogger = facebook::eden::StructuredLogger;

class WatchmanStructuredLogger : public ScubaStructuredLogger {
 public:
  explicit WatchmanStructuredLogger(
      std::shared_ptr<ScribeLogger> scribeLogger,
      SessionInfo sessionInfo);
  virtual ~WatchmanStructuredLogger() override = default;

 protected:
  virtual DynamicEvent populateDefaultFields(const char* type) override;
};

std::shared_ptr<StructuredLogger> getLogger();

} // namespace watchman
