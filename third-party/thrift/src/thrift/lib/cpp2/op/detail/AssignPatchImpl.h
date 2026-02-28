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

#include <thrift/lib/cpp2/op/detail/AssignPatch.h>

namespace apache::thrift::op::detail {

template <typename Patch>
void AssignPatch<Patch>::apply(T& val) const {
  if (dynPatch_) {
    auto value = protocol::asValueStruct<Tag>(val);
    dynPatch_->apply(value);
    val = protocol::fromValueStruct<Tag>(value);
  } else if (auto p = data_.assign()) {
    val = data_.assign().value();
  }
}

template <typename Patch>
void AssignPatch<Patch>::merge(AssignPatch other) {
  if (dynPatch_ && other.dynPatch_) {
    XLOG_EVERY_MS(CRITICAL, 10000)
        << "Merging dynamic patch is not implemented. "
           "The merged result will be incorrect.\n"
           "First Patch = "
        << folly::toPrettyJson(
               apache::thrift::protocol::toDynamic(dynPatch_->toObject()))
        << "\nSecond Patch = "
        << folly::toPrettyJson(
               apache::thrift::protocol::toDynamic(
                   other.dynPatch_->toObject()));

    // Do nothing, which is the old behavior
    return;
  }

  if (!other.dynPatch_) {
    if (auto p = other.data_.assign()) {
      assign(std::move(*p));
    }
    return;
  }

  if (auto p = data_.assign()) {
    other.apply(*p);
  } else {
    dynPatch_ = std::move(other.dynPatch_);
  }
}

} // namespace apache::thrift::op::detail
