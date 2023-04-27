/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/mc/msg.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

class FailoverErrorsSettingsBase {
 public:
  FailoverErrorsSettingsBase() = default;
  explicit FailoverErrorsSettingsBase(std::vector<std::string> errors);
  FailoverErrorsSettingsBase(
      std::vector<std::string> errorsGet,
      std::vector<std::string> errorsUpdate,
      std::vector<std::string> errorsDelete);
  explicit FailoverErrorsSettingsBase(const folly::dynamic& json);

  class List {
   public:
    List() = default;

    // copyable and movable.
    List(const List&);
    List& operator=(const List&);
    List(List&&) = default;
    List& operator=(List&&) = default;

    explicit List(std::vector<std::string> errors);
    explicit List(const folly::dynamic& json);

    bool shouldFailover(const carbon::Result result) const;

   private:
    std::unique_ptr<
        std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>
        failover_;

    void init(std::vector<std::string> errors);
    void init(
        const std::unique_ptr<
            std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>&
            otherFailover);
  };

  enum class FailoverType {
    NONE,
    NORMAL,
    CONDITIONAL,
  };

 protected:
  FailoverErrorsSettingsBase::List gets_;
  FailoverErrorsSettingsBase::List updates_;
  FailoverErrorsSettingsBase::List deletes_;
};
} // namespace memcache
} // namespace facebook
