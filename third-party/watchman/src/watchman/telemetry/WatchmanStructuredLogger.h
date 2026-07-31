/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "eden/common/telemetry/ScribeLogger.h"
#include "eden/common/telemetry/ScubaStructuredLogger.h"
#include "eden/common/telemetry/SessionInfo.h"
#include "eden/common/telemetry/StructuredLogger.h"

namespace facebook::eden {
class IXplatLogger;
} // namespace facebook::eden

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
  virtual DynamicEvent populateDefaultFields(
      std::optional<const char*> type) override;
};

std::shared_ptr<StructuredLogger> getLogger();

#ifdef WATCHMAN_FACEBOOK_INTERNAL
/// StructuredLogger adapter used when the XplatLogger path is enabled. Reuses
/// WatchmanStructuredLogger identity population (populateDefaultFields) and
/// forwards each fully-populated event to @p xplatLogger under the
/// watchman_events category. Exposed for testing with a fake IXplatLogger.
std::shared_ptr<StructuredLogger> makeWatchmanXplatStructuredLogger(
    SessionInfo sessionInfo,
    std::shared_ptr<facebook::eden::IXplatLogger> xplatLogger);
#endif

} // namespace watchman
