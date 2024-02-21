/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FailoverErrorsSettingsBase.h"

#include <memory>
#include <vector>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

FailoverErrorsSettingsBase::List::List(std::vector<std::string> errors) {
  init(std::move(errors));
}

// copy
FailoverErrorsSettingsBase::List::List(const List& other) {
  init(other.failover_);
}
FailoverErrorsSettingsBase::List& FailoverErrorsSettingsBase::List::operator=(
    const List& other) {
  init(other.failover_);
  return *this;
}

FailoverErrorsSettingsBase::List::List(const folly::dynamic& json) {
  checkLogic(json.isArray(), "List of failover errors is not an array.");

  std::vector<std::string> errors;
  errors.reserve(json.size());
  for (const auto& elem : json) {
    checkLogic(elem.isString(), "Failover error {} is not a string", elem);
    errors.push_back(elem.getString());
  }

  init(std::move(errors));
}

bool FailoverErrorsSettingsBase::List::shouldFailover(
    const carbon::Result result) const {
  if (failover_ != nullptr) {
    return (*failover_)[static_cast<size_t>(result)];
  }
  return isFailoverErrorResult(result);
}

void FailoverErrorsSettingsBase::List::init(std::vector<std::string> errors) {
  failover_ = std::make_unique<
      std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>();

  for (const auto& error : errors) {
    size_t i;
    for (i = 0; i < static_cast<size_t>(carbon::Result::NUM_RESULTS); ++i) {
      carbon::Result errorType = static_cast<carbon::Result>(i);
      folly::StringPiece errorName(carbon::resultToString(errorType));
      errorName.removePrefix("mc_res_");

      if (isErrorResult(errorType) && error == errorName) {
        (*failover_)[i] = true;
        break;
      }
    }

    checkLogic(
        i < static_cast<size_t>(carbon::Result::NUM_RESULTS),
        "Failover error '{}' is not a valid error type.",
        error);
  }
}

void FailoverErrorsSettingsBase::List::init(
    const std::unique_ptr<
        std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>&
        otherFailover) {
  if (otherFailover) {
    failover_ = std::make_unique<
        std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>();
    *failover_ = *otherFailover;
  }
}

FailoverErrorsSettingsBase::FailoverErrorsSettingsBase(
    std::vector<std::string> errors)
    : gets_(errors), updates_(errors), deletes_(std::move(errors)) {}

FailoverErrorsSettingsBase::FailoverErrorsSettingsBase(
    std::vector<std::string> errorsGet,
    std::vector<std::string> errorsUpdate,
    std::vector<std::string> errorsDelete)
    : gets_(std::move(errorsGet)),
      updates_(std::move(errorsUpdate)),
      deletes_(std::move(errorsDelete)) {}

FailoverErrorsSettingsBase::FailoverErrorsSettingsBase(
    const folly::dynamic& json) {
  checkLogic(
      json.isObject() || json.isArray(),
      "Failover errors must be either an array or an object.");

  if (json.isObject()) {
    if (auto jsonGets = json.get_ptr("gets")) {
      gets_ = FailoverErrorsSettingsBase::List(*jsonGets);
    }
    if (auto jsonUpdates = json.get_ptr("updates")) {
      updates_ = FailoverErrorsSettingsBase::List(*jsonUpdates);
    }
    if (auto jsonDeletes = json.get_ptr("deletes")) {
      deletes_ = FailoverErrorsSettingsBase::List(*jsonDeletes);
    }
  } else if (json.isArray()) {
    gets_ = FailoverErrorsSettingsBase::List(json);
    updates_ = FailoverErrorsSettingsBase::List(json);
    deletes_ = FailoverErrorsSettingsBase::List(json);
  }
}

} // namespace memcache
} // namespace facebook
