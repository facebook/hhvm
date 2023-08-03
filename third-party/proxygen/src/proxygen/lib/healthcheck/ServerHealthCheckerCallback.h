/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include <folly/SocketAddress.h>

#include <proxygen/lib/utils/Time.h>

namespace proxygen {

class ServerHealthChecker;

using LoadType = uint32_t;

struct ServerLoadInfo {
  double cpuUser{-1.};
  double cpuSys{-1.};
  double cpuIdle{-1.};
  LoadType queueLen{0};
  bool operator!=(const ServerLoadInfo& rhs) {
    return cpuUser != rhs.cpuUser || cpuSys != rhs.cpuSys ||
           cpuIdle != rhs.cpuIdle || queueLen != rhs.queueLen;
  }
};

enum ServerDownInfo {
  NONE = 0,

  PASSIVE_HEALTHCHECK_FAIL = 1,
  HEALTHCHECK_TIMEOUT = 2,
  HEALTHCHECK_BODY_MISMATCH = 3,
  HEALTHCHECK_NON200_STATUS = 4,
  HEALTHCHECK_MESSAGE_ERROR = 5,
  HEALTHCHECK_WRITE_ERROR = 6,
  HEALTHCHECK_UPGRADE_ERROR = 7,
  HEALTHCHECK_EOF = 8,
  HEALTHCHECK_CONNECT_ERROR = 9,
  FEEDBACK_LOOP_HIGH_LOAD = 10,
  HEALTH_UNKNOWN = 11,

  HEALTHCHECK_UNKNOWN_ERROR = 99,
};

const std::string serverDownInfoStr(ServerDownInfo info);

/*
 * ServerHealthCheckerCallback is the interface for receiving health check
 * responses. The caller may be from a different thread.
 */
class ServerHealthCheckerCallback {
 public:
  // Additional info received from a successful healthcheck (e.g. HTTP headers)
  using ExtraInfo = std::vector<std::pair<std::string, std::string>>;

  virtual void processHealthCheckFailure(
      const TimePoint& startTime,
      ServerDownInfo reason,
      const std::string& extraReasonStr = std::string(),
      const ExtraInfo* extraInfo = nullptr) = 0;

  virtual void processHealthCheckSuccess(
      const TimePoint& startTime,
      LoadType load,
      const ServerLoadInfo* serverLoadInfo = nullptr,
      const ExtraInfo* extraInfo = nullptr) = 0;

  virtual ~ServerHealthCheckerCallback() {
  }
};

} // namespace proxygen
