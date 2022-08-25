/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace carbon {

struct ExternalCarbonConnectionStats {
  std::string destHost;
  std::string key;
  uint16_t port;
  std::string protocol;
  std::string resultCode;
  int32_t reqSizeBytes;
  std::string reqType;
  int32_t respSizeBytes;
  std::string routerName;
  std::string securityMechanism;
  bool hasCatToken;
  std::string srcHost;
};

struct ExternalCarbonConnectionLoggerOptions {
  bool enabled{false};
  uint32_t log_sample_rate{10000};
  uint32_t log_error_sample_rate{0};
  uint32_t rate_limit_per_hour{3600};
  uint32_t max_burst{500};

  explicit ExternalCarbonConnectionLoggerOptions() {}

  explicit ExternalCarbonConnectionLoggerOptions(
      bool enableLogger,
      uint32_t logSampleRate,
      uint32_t logErrorSampleRate,
      uint32_t rateLimitPerHour,
      uint32_t maxBurst)
      : enabled(enableLogger),
        log_sample_rate(logSampleRate),
        log_error_sample_rate(logErrorSampleRate),
        rate_limit_per_hour(rateLimitPerHour),
        max_burst(maxBurst) {}
};

} // namespace carbon
