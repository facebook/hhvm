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
#include <thrift/lib/cpp2/protocol/detail/Patch.h>
#include <thrift/lib/thrift/detail/protocol.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>

namespace apache::thrift {

namespace op::detail {

// Latest Thrift Dynamic Patch version that the process is aware of. Note, this
// may differ from `kThriftStaticPatchVersion` if we introduce new operations in
// Thrift State Patch first.
inline constexpr int32_t kThriftDynamicPatchVersion = 2;

} // namespace op::detail

namespace protocol {
namespace detail {

struct ApplyPatch {
 public:
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
  void operator()(const Object& patch, folly::F14VectorSet<Value>& value) const;
  void operator()(
      const Object& patch, folly::F14FastMap<Value, Value>& value) const;
  void operator()(const Object& patch, Object& value) const;
  void operator()(Object&& patch, Object& value) const;
};

/**
 * Returns the minimum version of Thrift Patch library required to safely decode
 * and apply the given Thrift Dynamic Patch.
 */
int32_t calculateMinSafePatchVersion(const protocol::Object& patch);

} // namespace detail

/**
 * Takes protocol Object and targer protocol Value. Makes sure that Object
 * represents a Patch and tries to apply this patch to the target Value.
 *
 * @param patch Object
 * @param value to be patched
 */
[[deprecated(
    "Use DynamicPatch::apply instead.")]] inline constexpr detail::ApplyPatch
    applyPatch{};

/**
 * Extracted Thrift Masks from Thrift Patch. Read Thrift Mask contains fields or
 * map elements that are need to be known to apply Thrift Patch to the target
 * value. Write Thrift Mask contains fields or map elements that are affected by
 * Thrift Patch. Read Thrift Mask is always subset of Write Thrift Mask. Read
 * and write Thrift Masks are useful to gain better insight on the given Thrift
 * Patch. For example, it is used in `protocol::applyPatchToSerializedData` to
 * avoid full deserialization when applying Thrift Patch to serialized data in a
 * binary blob.
 */
using detail::ExtractedMasksFromPatch;

/// Constructs read and write Thrift Mask that only contain fields that are
/// modified by the Patch. It will construct nested Mask for map and object
/// patches. For map, it only supports integer or string key. If the type of key
/// map is not integer or string, it throws.
[[deprecated(
    "Use DynamicPatch::extractMaskFromPatch instead.")]] ExtractedMasksFromPatch
extractMaskFromPatch(const protocol::Object& patch);

/// @cond
// Constructs read and write Thrift Map Mask of a given patch originating from
// the field, map entry, or type entry specified mask. YOU PROBABLY SHOULDN'T BE
// CALLING THIS!
[[deprecated]] ExtractedMasksFromPatch extractMapMaskFromPatch_DO_NOT_USE(
    const protocol::Object& patch, const Mask& mask);
/// @endcond

template <type::StandardProtocol Protocol>
[[deprecated("Use DynamicPatch::applyToSerializedObject instead.")]] std::
    unique_ptr<folly::IOBuf>
    applyPatchToSerializedData(
        const protocol::Object& patch, const folly::IOBuf& buf);

/**
 * Returns a Thrift Dynamic Patch instance corresponding to the (decoded)
 * `SafePatch` in Protocol Object. Note, `SafePatch` needs to be converted to
 * Thrift Dynamic Patch before applying or extracting field mask.
 *
 * @throws std::runtime_error if the given `SafePatch` cannot be successfully
 * decoded or safely applied in this process (eg. if the version of the Thrift
 * Patch library in this process is not compatible with the minimum version
 * required by `SafePatch`).
 */
[[deprecated("Use DynamicPatch::fromSafePatch instead.")]] Object fromSafePatch(
    const protocol::Object& safePatch);

/**
 * Returns a `SafePatch` instance in Protocol Object corresponding to the
 * encoded Thrift Dynamic Patch.
 */
[[deprecated("Use DynamicPatch::toSafePatch instead.")]] Object toSafePatch(
    const protocol::Object& patch);

} // namespace protocol
} // namespace apache::thrift
