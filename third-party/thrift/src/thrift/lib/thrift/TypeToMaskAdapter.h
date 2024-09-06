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

#include <folly/Exception.h>
#include <folly/container/F14Map.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::protocol::detail {

std::string debugFormatType(const type::Type& type);

/**
 * Adapter to convert a List<<Type, Mask>> into a Map<Type, Mask>
 *
 * Because structured keys are not well supported by some langauges, we cannot
 * use Map<Type, Mask> directly in the IDL.
 *
 * This Adapter attempts to maintain a sane user-experience in C++
 */

template <typename Entry, typename Mask>
class TypeToMaskAdapter {
  using ThriftType = std::vector<Entry>;

#if FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE
  using AdaptedType = folly::F14VectorMap<type::Type, Mask>;
#else
  /**
   * F14Map fallsback to unordered_map (which cannot deal with incomplete
   * ValueType). So use std::map instead
   */
  using AdaptedType = std::map<type::Type, Mask>;
#endif
 public:
  static AdaptedType fromThrift(const ThriftType& thriftVal) {
    AdaptedType adaptedVal;
    adaptedVal.reserve(thriftVal.size());
    for (const Entry& e : thriftVal) {
      if (!adaptedVal.emplace(*e.type_ref(), *e.mask_ref()).second) {
        folly::throw_exception<std::runtime_error>(
            "type-mask has mulitple entries for the same type: " +
            debugFormatType(*e.type_ref()));
      }
    }
    return adaptedVal;
  }

  static ThriftType toThrift(const AdaptedType& adaptedVal) {
    ThriftType thriftVal;
    thriftVal.reserve(adaptedVal.size());
    for (const auto& [type, mask] : adaptedVal) {
      thriftVal.emplace_back();
      thriftVal.back().type_ref() = type;
      thriftVal.back().mask_ref() = mask;
    }
    return thriftVal;
  }

  // TODO: Customize encode/decode to improve performance of serde (i.e. avoid
  // materializing intermediate thrift struct)
};

} // namespace apache::thrift::protocol::detail
