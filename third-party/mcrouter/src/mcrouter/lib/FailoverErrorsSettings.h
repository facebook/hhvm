/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include "mcrouter/lib/FailoverErrorsSettingsBase.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

class FailoverErrorsSettings : public FailoverErrorsSettingsBase {
 public:
  FailoverErrorsSettings() = default;
  explicit FailoverErrorsSettings(std::vector<std::string> errors)
      : FailoverErrorsSettingsBase(std::move(errors)) {}
  FailoverErrorsSettings(
      std::vector<std::string> errorsGet,
      std::vector<std::string> errorsUpdate,
      std::vector<std::string> errorsDelete)
      : FailoverErrorsSettingsBase(
            std::move(errorsGet),
            std::move(errorsUpdate),
            std::move(errorsDelete)) {}
  explicit FailoverErrorsSettings(const folly::dynamic& json)
      : FailoverErrorsSettingsBase(json) {}

  template <class Request>
  FailoverType shouldFailover(
      const ReplyT<Request>& reply,
      const Request&,
      carbon::DeleteLikeT<Request> = 0) const {
    return deletes_.shouldFailover(*reply.result_ref()) ? FailoverType::NORMAL
                                                        : FailoverType::NONE;
  }

  template <class Request>
  FailoverType shouldFailover(
      const ReplyT<Request>& reply,
      const Request&,
      carbon::GetLikeT<Request> = 0) const {
    return gets_.shouldFailover(*reply.result_ref()) ? FailoverType::NORMAL
                                                     : FailoverType::NONE;
  }

  template <class Request>
  FailoverType shouldFailover(
      const ReplyT<Request>& reply,
      const Request&,
      carbon::UpdateLikeT<Request> = 0) const {
    return updates_.shouldFailover(*reply.result_ref()) ? FailoverType::NORMAL
                                                        : FailoverType::NONE;
  }

  template <class Request>
  FailoverType shouldFailover(
      const ReplyT<Request>& reply,
      const Request&,
      carbon::OtherThanT<
          Request,
          carbon::DeleteLike<>,
          carbon::GetLike<>,
          carbon::UpdateLike<>> = 0) const {
    return isFailoverErrorResult(*reply.result_ref()) ? FailoverType::NORMAL
                                                      : FailoverType::NONE;
  }
};
} // namespace memcache
} // namespace facebook
