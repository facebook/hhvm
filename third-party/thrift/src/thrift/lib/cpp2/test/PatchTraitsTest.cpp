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

#include <thrift/lib/cpp2/op/PatchTraits.h>

#include <type_traits>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache::thrift::op {
namespace {

TEST(PatchTraitsTest, IsPatch) {
  static_assert(is_patch_v<BoolPatch>);
  static_assert(is_patch_v<BytePatch>);
  static_assert(is_patch_v<I16Patch>);
  static_assert(is_patch_v<I32Patch>);
  static_assert(is_patch_v<I64Patch>);
  static_assert(is_patch_v<FloatPatch>);
  static_assert(is_patch_v<DoublePatch>);
  static_assert(is_patch_v<StringPatch>);
  static_assert(is_patch_v<BinaryPatch>);

  static_assert(!is_patch_v<int>);
  static_assert(!is_patch_v<std::string>);
}

TEST(PatchTraitsTest, PatchedTypeTagPrimitives) {
  static_assert(std::is_same_v<BoolPatch::value_type_tag, type::bool_t>);
  static_assert(std::is_same_v<BytePatch::value_type_tag, type::byte_t>);
  static_assert(std::is_same_v<I16Patch::value_type_tag, type::i16_t>);
  static_assert(std::is_same_v<I32Patch::value_type_tag, type::i32_t>);
  static_assert(std::is_same_v<I64Patch::value_type_tag, type::i64_t>);
  static_assert(std::is_same_v<FloatPatch::value_type_tag, type::float_t>);
  static_assert(std::is_same_v<DoublePatch::value_type_tag, type::double_t>);
  static_assert(std::is_same_v<StringPatch::value_type_tag, type::string_t>);
  static_assert(std::is_same_v<
                BinaryPatch::value_type_tag,
                apache::thrift::type::
                    cpp_type<folly::IOBuf, apache::thrift::type::binary_t>>);
}
} // namespace
} // namespace apache::thrift::op
