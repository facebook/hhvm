/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SlowWarmUpRouteSettings.h"

#include <string>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

double asNumber(const folly::dynamic* maybeNumber, const std::string& name) {
  checkLogic(
      maybeNumber->isNumber(),
      "SlowWarmUpSettings: '{}' must be a number, but was {}",
      name,
      maybeNumber->typeName());
  return maybeNumber->asDouble();
}

double getDouble01(const folly::dynamic& json, std::string name) {
  auto valJson = json.get_ptr(name);
  checkLogic(valJson, "SlowWarmUpSettings: Couldn't find '{}' in config", name);
  double val = asNumber(valJson, name);
  checkLogic(
      val >= 0.0 && val <= 1.0,
      "SlowWarmUpSettings: '{}' must be a double in [0,1] range",
      name);
  return val;
}

} // anonymous namespace

SlowWarmUpRouteSettings::SlowWarmUpRouteSettings(const folly::dynamic& json) {
  enableThreshold_ = getDouble01(json, "enable_threshold");
  disableThreshold_ = getDouble01(json, "disable_threshold");
  checkLogic(
      enableThreshold_ < disableThreshold_,
      "SlowWarmUpSettings: "
      "'enable_threshold' must be strictly less than 'disable_threshold'.");

  const std::string startName = "start";
  if (auto startPtr = json.get_ptr(startName)) {
    start_ = asNumber(startPtr, startName);
    checkLogic(
        start_ > 0.0 && start_ < 1.0,
        "SlowWarmUpSettings: '{}' must be a double in (0,1) range, but was {}",
        startName,
        start_);
  }

  const std::string stepName = "step";
  if (auto stepPtr = json.get_ptr(stepName)) {
    step_ = asNumber(stepPtr, stepName);
    checkLogic(
        step_ > 0.0 && step_ < 100.0,
        "SlowWarmUpSettings: '{}' must be in the (0,100) range, but was {}",
        stepName,
        step_);
  }

  const std::string minReqsName = "min_requests";
  if (auto minReqsPtr = json.get_ptr(minReqsName)) {
    checkLogic(
        minReqsPtr->isInt(),
        "SlowWarmUpSettings: '{}' must be an integer, but was {}",
        minReqsName,
        minReqsPtr->typeName());
    minRequests_ = minReqsPtr->getInt();
    checkLogic(
        minRequests_ > 0,
        "SlowWarmUpSettings: '{}' must be an integer greater than 0",
        minReqsName);
  }
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
