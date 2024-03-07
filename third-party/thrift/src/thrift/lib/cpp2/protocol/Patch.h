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

#include <folly/Portability.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/thrift/detail/protocol.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// Latest Thrift Dynamic Patch version that the process is aware of. Note, this
// may differ from `kThriftStaticPatchVersion` if we introduce new operations in
// Thrift State Patch first.
inline constexpr int32_t kThriftDynamicPatchVersion = 1;

} // namespace detail
} // namespace op
namespace protocol {
namespace detail {

struct ApplyPatch {
  // Applies 'patch' to 'value' in-place.
  void operator()(const Object& patch, Value& value) const;
  void operator()(const Object& patch, bool& value) const;
  void operator()(const Object& patch, int8_t& value) const;
  void operator()(const Object& patch, int16_t& value) const;
  void operator()(const Object& patch, int32_t& value) const;
  void operator()(const Object& patch, int64_t& value) const;
  void operator()(const Object& patch, float& value) const;
  void operator()(const Object& patch, double& value) const;
  void operator()(const Object& patch, folly::IOBuf& value) const;
  void operator()(const Object& patch, std::vector<Value>& value) const;
  void operator()(const Object& patch, folly::F14FastSet<Value>& value) const;
  void operator()(
      const Object& patch, folly::F14FastMap<Value, Value>& value) const;
  void operator()(const Object& patch, Object& value) const;
};

} // namespace detail

/**
 * Takes protocol Object and targer protocol Value. Makes sure that Object
 * represents a Patch and tries to apply this patch to the target Value.
 *
 * @param patch Object
 * @param value to be patched
 */
inline constexpr detail::ApplyPatch applyPatch{};

struct ExtractedMasks {
  Mask read; // read mask from patch
  Mask write; // write mask from patch
};

/// Constructs read and write Thrift Mask that only contain fields that are
/// modified by the Patch. It will construct nested Mask for map and object
/// patches. For map, it uses the address of Value key as the key for the
/// integer map mask. Note that Mask contains pointer to `protocol::Value` in
/// patch, so caller needs to make sure Patch has longer lifetime than the mask.
ExtractedMasks extractMaskViewFromPatch(const protocol::Object& patch);

// Extracting mask from a temporary patch is dangerous and should be disallowed.
ExtractedMasks extractMaskViewFromPatch(Object&& patch) = delete;

/// Constructs read and write Thrift Mask that only contain fields that are
/// modified by the Patch. It will construct nested Mask for map and object
/// patches. For map, it only supports integer or string key. If the type of key
/// map is not integer or string, it throws.
ExtractedMasks extractMaskFromPatch(const protocol::Object& patch);

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf> applyPatchToSerializedData(
    const protocol::Object& patch, const folly::IOBuf& buf);

/**
 * Returns a Thrift Dynamic Patch instance corresponding to the (decoded)
 * `SafePatch` in Protocol Object.
 *
 * @throws std::runtime_error if the given `SafePatch` cannot be successfully
 * decoded or safely applied in this process (eg. if the version of the Thrift
 * Patch library in this process is not compatible with the minimum version
 * required by `SafePatch`).
 */
Object fromSafePatch(const protocol::Object& safePatch);

/**
 * Returns a `SafePatch` instance in Protocol Object corresponding to the
 * encoded Thrift Dynamic Patch.
 */
Object toSafePatch(const protocol::Object& patch);

} // namespace protocol
} // namespace thrift
} // namespace apache
