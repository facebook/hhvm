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

#include <vector>
#include <fmt/format.h>
#include <folly/container/F14Map.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_detail_types.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache {
namespace thrift {
namespace op {

class TypeToPatchInternalDoNotUse;

namespace detail {

struct TypeToPatchMapAdapter {
  using StandardType = std::vector<TypeToPatchInternalDoNotUse>;
  using AdaptedType =
      folly::F14FastMap<type::Type, std::vector<type::AnyStruct>>;

  static AdaptedType fromThrift(StandardType&& vec) {
    TypeToPatchMapAdapter::AdaptedType map;
    map.reserve(vec.size());
    for (auto& typeToPatchStruct : vec) {
      if (!map.emplace(
                  typeToPatchStruct.type().value(),
                  std::move(typeToPatchStruct.patches().value()))
               .second) {
        folly::throw_exception<std::runtime_error>(fmt::format(
            "duplicated key: {}",
            debugStringViaEncode(typeToPatchStruct.type().value())));
      }
    }
    return map;
  }

  static StandardType toThrift(const AdaptedType& map) {
    TypeToPatchMapAdapter::StandardType vec;
    vec.reserve(map.size());
    for (const auto& [type, patches] : map) {
      auto& obj = vec.emplace_back();
      obj.type() = type;
      obj.patches() = patches;
    }
    return vec;
  }

  // TODO(dokwon): Add customizations to optimize operators in adapted type.
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
