/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {
/**
 * Visitor that evaluates blackholing policies
 *
 * Sample json map format:
 *
 *  {
 *    "fieldName1": [
 *      {
 *        "op": "equals", // equals or not_equals
 *        "value": 1234   // int, double, bool or string
 *      }
 *    ],
 *    "fieldName2": [
 *      {
 *        "op": "not_equals",
 *        "value": false
 *      }
 *    ]
 *  }
 */
class BlackholingPolicyVisitor {
 public:
  explicit BlackholingPolicyVisitor(const folly::dynamic& jsonMap)
      : eval_(true), policyMap_(jsonMap) {}

  template <class T>
  bool enterMixin(size_t /* id */, folly::StringPiece /* name */, const T&) {
    return true;
  }

  bool leaveMixin() {
    return true;
  }

  template <class T>
  bool visitField(size_t /* id */, folly::StringPiece name, const T& t) {
    auto itPolicies = policyMap_.find(name.str());
    if (itPolicies != policyMap_.items().end()) {
      // Blackholing policies are validated to be in an array at config time
      eval_ = eval_ && evaluatePolicies(itPolicies->second, t);
    }
    return true;
  }

  bool shouldBlackhole() {
    return eval_;
  }

 private:
  bool eval_;
  const folly::dynamic policyMap_;

  template <class T>
  bool evaluatePolicies(const folly::dynamic& dArray, const T& t) {
    bool specificPolicy = false;
    for (auto dObj : dArray) {
      // for the same field, we have an "or" evaluation policy
      specificPolicy = specificPolicy || evaluatePolicy(dObj, t);
    }
    return specificPolicy;
  }

  template <class T>
  bool evaluatePolicy(const folly::dynamic& dObj, const T& t) {
    auto itOperator = dObj.find("op");
    auto itValue = dObj.find("value");
    if (itOperator == dObj.items().end() || itValue == dObj.items().end()) {
      LOG_EVERY_N(ERROR, 1000) << "Could not find 'op' or 'value' in "
                               << "blackholing routing policy. Will skip "
                               << "blackholing the field";

      // do not balckhole if something went wrong.
      return false;
    }
    return evaluate(itOperator->second.asString(), itValue->second, t);
  }

  template <class T>
  std::enable_if_t<std::is_arithmetic<T>::value, bool>
  evaluate(const std::string& op, const folly::dynamic& dValue, const T& t) {
    if (dValue.isInt()) {
      return evaluateOp(op, dValue.asInt(), t);
    } else if (dValue.isBool()) {
      return evaluateOp(op, dValue.asBool(), t);
    } else if (dValue.isDouble()) {
      return evaluateOp(op, dValue.asDouble(), t);
    } else {
      // unsupported type
      return false;
    }
  }

  template <class T>
  bool evaluate(
      const std::string& op,
      const folly::dynamic& dValue,
      const carbon::Keys<T>& t) {
    if (dValue.isString()) {
      return evaluateOp(op, dValue.asString(), t.fullKey().str());
    }
    // unsupported type
    return false;
  }

  template <class T>
  std::enable_if_t<!std::is_arithmetic<T>::value, bool> evaluate(
      const std::string& /* op */,
      const folly::dynamic& /* dValue */,
      const T& /* t */) {
    // unsupported type
    return false;
  }

  template <class T, class X>
  bool evaluateOp(const std::string& op, const X& x, const T& t) {
    if (op == "equals") {
      return x == static_cast<decltype(x)>(t);
    }
    if (op == "not_equals") {
      return x != static_cast<decltype(x)>(t);
    }
    // invalid operation
    return false;
  }
};
} // namespace detail

template <class R>
bool shouldBlackhole(const R& req, const folly::dynamic& jsonMap) {
  detail::BlackholingPolicyVisitor visitor(jsonMap);
  req.visitFields(visitor);
  return visitor.shouldBlackhole();
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
