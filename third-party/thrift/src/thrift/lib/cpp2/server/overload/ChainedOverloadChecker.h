/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <thrift/lib/cpp2/server/overload/IOverloadChecker.h>
#include <thrift/lib/cpp2/server/overload/NoOpOverloadChecker.h>

namespace apache::thrift {

namespace detail {

template <typename T1, typename T2>
struct ChainedChecker {
  ChainedChecker(std::unique_ptr<T1>&& primary, std::unique_ptr<T2>&& secondary)
      : primary_(std::move(primary)), secondary_(std::move(secondary)) {}

  folly::Optional<OverloadResult> checkOverload(
      const IOverloadChecker::CheckOverloadParams params) {
    auto primaryResult = primary_->checkOverload(params);
    if (primaryResult.has_value()) {
      return primaryResult;
    }
    return secondary_->checkOverload(params);
  }

 private:
  std::unique_ptr<T1> primary_;
  std::unique_ptr<T2> secondary_;
};

template <typename T1>
struct ChainedChecker<T1, NoOpOverloadChecker> {
  folly::Optional<OverloadResult> checkOverload(
      const IOverloadChecker::CheckOverloadParams params) {
    return primary_->checkOverload(params);
  }

 private:
  std::shared_ptr<T1> primary_;
};

} // namespace detail

/**
 * ChainedOverloadChecker is a composite overload checker that chains two other
 * overload checkers. It first checks the primary overload checker and returns
 * its result if it is not none. Otherwise, it checks the secondary overload
 * checker and returns its result. If both primary and secondary overload
 * checkers return none, it returns none. Lets you implement the Chain Of
 * Responsibility pattern with overload checkers using the Composite pattern.
 */
template <typename T1, typename T2>
class ChainedOverloadChecker final : public IOverloadChecker {
  static_assert(
      std::conjunction_v<
          std::is_base_of<IOverloadChecker, T1>,
          std::is_base_of<IOverloadChecker, T2>>,
      "T1 and T2 must be an instace of OverloadChecker");

 public:
  ChainedOverloadChecker(
      std::unique_ptr<T1>&& primary, std::unique_ptr<T2>&& secondary)
      : chainedChecker_(std::move(primary), std::move(secondary)) {}

  folly::Optional<OverloadResult> checkOverload(
      const CheckOverloadParams params) override {
    return chainedChecker_.checkOverload(params);
  }

 private:
  detail::ChainedChecker<T1, T2> chainedChecker_;
};

} // namespace apache::thrift
