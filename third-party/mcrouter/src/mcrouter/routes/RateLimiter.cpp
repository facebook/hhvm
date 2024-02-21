/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RateLimiter.h"

#include <string>
#include <vector>

#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/util.h"

using folly::dynamic;
using std::string;

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

double asPositiveDouble(const dynamic& obj, const string& keyName) {
  checkLogic(obj[keyName].isNumber(), "{} is not a number", keyName);
  auto d = obj[keyName].asDouble();
  checkLogic(d > 0.0, "{} is nonpositive", keyName);
  return d;
}

double
asPositiveDoubleDefault(const dynamic& obj, const string& keyName, double def) {
  if (obj.count(keyName) && obj[keyName].isNumber()) {
    auto d = obj[keyName].asDouble();
    checkLogic(d > 0.0, "{} is nonpositive", keyName);
    return d;
  }
  return def;
}

} // namespace

RateLimiter::RateLimiter(const folly::dynamic& json) {
  checkLogic(json.isObject(), "RateLimiter settings json is not an object");

  auto now = folly::TokenBucket::defaultClockNow();

  if (json.count("gets_rate")) {
    double rate = asPositiveDouble(json, "gets_rate");
    double burst = asPositiveDoubleDefault(json, "gets_burst", rate);
    getsTb_ = folly::TokenBucket(rate, burst, now);
  }

  if (json.count("sets_rate")) {
    double rate = asPositiveDouble(json, "sets_rate");
    double burst = asPositiveDoubleDefault(json, "sets_burst", rate);
    setsTb_ = folly::TokenBucket(rate, burst, now);
  }

  if (json.count("deletes_rate")) {
    double rate = asPositiveDouble(json, "deletes_rate");
    double burst = asPositiveDoubleDefault(json, "deletes_burst", rate);
    deletesTb_ = folly::TokenBucket(rate, burst, now);
  }
}

std::string RateLimiter::toDebugStr() const {
  std::vector<string> pieces;
  if (getsTb_) {
    pieces.push_back(folly::to<string>("gets_rate=", getsTb_->rate()));
    pieces.push_back(folly::to<string>("gets_burst=", getsTb_->burst()));
  }
  if (setsTb_) {
    pieces.push_back(folly::to<string>("sets_rate=", setsTb_->rate()));
    pieces.push_back(folly::to<string>("sets_burst=", setsTb_->burst()));
  }
  if (deletesTb_) {
    pieces.push_back(folly::to<string>("deletes_rate=", deletesTb_->rate()));
    pieces.push_back(folly::to<string>("deletes_burst=", deletesTb_->burst()));
  }
  return folly::join('|', pieces);
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
