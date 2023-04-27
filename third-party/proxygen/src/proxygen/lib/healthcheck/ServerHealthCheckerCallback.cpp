/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/healthcheck/ServerHealthCheckerCallback.h>

namespace proxygen {

const std::string serverDownInfoStr(ServerDownInfo info) {
  switch (info) {
    case ServerDownInfo::NONE:
      return "None";
    case ServerDownInfo::PASSIVE_HEALTHCHECK_FAIL:
      return "Passive HealthCheck Failed";
    case ServerDownInfo::HEALTHCHECK_TIMEOUT:
      return "Active HealthCheck Timed Out";
    case ServerDownInfo::HEALTHCHECK_BODY_MISMATCH:
      return "Active HealthCheck Body Mismatch";
    case ServerDownInfo::HEALTHCHECK_NON200_STATUS:
      return "Active HealthCheck Non-200 Status";
    case ServerDownInfo::HEALTHCHECK_MESSAGE_ERROR:
      return "Active HealthCheck Message Error";
    case ServerDownInfo::HEALTHCHECK_WRITE_ERROR:
      return "Active HealthCheck Write Error";
    case ServerDownInfo::HEALTHCHECK_UPGRADE_ERROR:
      return "Active HealthCheck Unexpected Upgrade";
    case ServerDownInfo::HEALTHCHECK_EOF:
      return "Active HealthCheck Server EOF";
    case ServerDownInfo::HEALTHCHECK_CONNECT_ERROR:
      return "Active HealthCheck Connect Failed";
    case ServerDownInfo::FEEDBACK_LOOP_HIGH_LOAD:
      return "Feedback Loop High Load";
    case ServerDownInfo::HEALTHCHECK_UNKNOWN_ERROR:
      return "Unknown HealthCheck Error";
    case ServerDownInfo::HEALTH_UNKNOWN:
      return "Server Health Unknown";
    default:
      return "[missing down info string!]";
  }
}

} // namespace proxygen
