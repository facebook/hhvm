/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/logger/DBEventLogger.h"

#include <glog/logging.h>

namespace facebook {
namespace db {

void DBSimpleLogger::logQuerySuccess(
    const QueryLoggingData& data,
    const SquangleLoggingData&) const {
  VLOG(2) << "[" << api_name_ << "]"
          << " query (\"" << data.query << "\") succeeded.";
}

void DBSimpleLogger::logQueryFailure(
    const QueryLoggingData& data,
    FailureReason,
    unsigned int,
    const std::string&,
    const SquangleLoggingData&) const {
  VLOG(2) << "[" << api_name_ << "]"
          << " query (\"" << data.query << "\") failed.";
}

void DBSimpleLogger::logConnectionSuccess(
    const CommonLoggingData&,
    const SquangleLoggingData& connInfo) const {
  VLOG(2) << "[" << api_name_ << "]"
          << " connection with " << connInfo.connKey->host() << " succeeded";
}

void DBSimpleLogger::logConnectionFailure(
    const CommonLoggingData&,
    FailureReason,
    unsigned int,
    const std::string&,
    const SquangleLoggingData& connInfo) const {
  VLOG(2) << "[" << api_name_ << "]"
          << " connection with " << connInfo.connKey->host() << " failed";
}
} // namespace db
} // namespace facebook
