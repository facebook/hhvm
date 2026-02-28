/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <thrift/lib/cpp2/FieldRef.h>
#include "mcrouter/lib/carbon/MessageCommon.h"
#include "mcrouter/lib/carbon/gen-cpp2/carbon_result_types.h"

namespace facebook {
namespace memcache {
struct AccessPoint;
} // namespace memcache
} // namespace facebook

namespace carbon {

class ReplyCommon : public MessageCommon {
 public:
  const std::shared_ptr<const facebook::memcache::AccessPoint>& destination()
      const noexcept;

  void setDestination(
      std::shared_ptr<const facebook::memcache::AccessPoint> ap) noexcept;

  // Store region string in an optional field.
  void setRegion(const std::string& region);

  // Get region string if set
  const std::optional<std::string>& getRegion() const;

 private:
  std::shared_ptr<const facebook::memcache::AccessPoint> destination_;
  std::optional<std::string> region_;
};

class ReplyCommonThrift : public ReplyCommon {
 public:
  explicit ReplyCommonThrift(carbon::Result result__ = carbon::Result::UNKNOWN);

  carbon::Result result() const;

  carbon::Result& result();

  auto result_ref() const& {
    return apache::thrift::detail::make_field_ref(
        this->result_, __isset.result);
  }

  auto result_ref() & {
    return apache::thrift::detail::make_field_ref(
        this->result_, __isset.result);
  }

 private:
  struct __isset {
    uint8_t result;
  } __isset = {};
  carbon::Result result_{carbon::Result::UNKNOWN};
};

} // namespace carbon
