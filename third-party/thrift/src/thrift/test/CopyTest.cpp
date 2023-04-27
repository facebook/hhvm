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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Copy.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/gen-cpp2/copy_types.h>

namespace apache::thrift::test {

// Tests copy with the given src and dst by incrementing the src field.
// Assumes src field has a value.
// srcFieldRef has field_ref<T&> type while src can be other
// types like field_ref<const T&&>.
template <typename SrcRef, typename Src, typename Dst>
void copyFieldRef(SrcRef srcFieldRef, Src src, Dst dst) {
  srcFieldRef = can_throw(*srcFieldRef) + 1;
  op::copy(src, dst);
  EXPECT_EQ(*dst, *srcFieldRef);
}
// srcPtr has smart_ptr<T>& type while src can be other
// types like const smart_ptr<T>&&.
template <typename SrcPtr, typename Src, typename Dst>
void copyUniquePointer(SrcPtr& srcPtr, Src&& src, Dst&& dst) {
  assert(srcPtr != nullptr);
  *srcPtr = *srcPtr + 1;
  op::copy(std::forward<decltype(src)>(src), std::forward<decltype(dst)>(dst));
  EXPECT_EQ(*dst, *srcPtr);
}

template <typename Src, typename Dst>
void copySharedPointer(Src&& src, Dst&& dst) {
  dst = nullptr;
  op::copy(std::forward<decltype(src)>(src), std::forward<decltype(dst)>(dst));
  EXPECT_EQ(src, dst);
}

template <typename Src, typename Dst, typename Ord>
void testFieldRef(Src src, Dst dst, Ord ord) {
  using Struct = decltype(src);
  auto srcField = op::get<decltype(ord), Struct>(src);
  auto dstField = op::get<decltype(ord), Struct>(dst);
  // test src = field_ref<T&>
  copyFieldRef(srcField, srcField, dstField);
  // test src = field_ref<const T&>
  const auto& srcConstRef = src;
  auto srcConstField = op::get<decltype(ord), Struct>(srcConstRef);
  copyFieldRef(srcField, srcConstField, dstField);
  // test src = field_ref<T&&>
  auto&& srcRvalueRef = std::move(src);
  auto srcRvalueField = op::get<decltype(ord), Struct>(
      std::forward<decltype(srcRvalueRef)>(srcRvalueRef));
  copyFieldRef(srcField, srcRvalueField, dstField);
  // test src = field_ref<const T&&>
  const auto&& srcRvalueConstRef = std::move(src);
  auto srcRvalueConstField = op::get<decltype(ord), Struct>(
      std::forward<decltype(srcRvalueConstRef)>(srcRvalueConstRef));
  copyFieldRef(srcField, srcRvalueConstField, dstField);
}

template <typename Src, typename Dst, typename Ord>
void testCopyNotOptional(Src src, Dst dst, Ord ord) {
  using Struct = decltype(src);
  auto srcField = op::get<decltype(ord), Struct>(src);
  auto dstField = op::get<decltype(ord), Struct>(dst);
  op::copy(srcField, dstField);
  EXPECT_EQ(*dstField, 0);
  testFieldRef(src, dst, ord);
}

template <typename Src, typename Dst, typename Ord>
void testCopyOptional(Src src, Dst dst, Ord ord) {
  using Struct = decltype(src);
  auto srcField = op::get<decltype(ord), Struct>(src);
  auto dstField = op::get<decltype(ord), Struct>(dst);
  // src doesn't have value.
  op::copy(srcField, dstField);
  EXPECT_FALSE(dstField.has_value());
  dstField = 0;
  op::copy(srcField, dstField);
  EXPECT_FALSE(dstField.has_value());
  // src has value.
  srcField = 1;
  testFieldRef(src, dst, ord);
}

template <typename Src, typename Dst, typename Ord>
void testCopyUniquePointer(Src src, Dst dst, Ord ord) {
  using Struct = decltype(src);
  using FieldTag = op::get_field_tag<Struct, decltype(ord)>;
  auto& srcField = op::get<decltype(ord), Struct>(src);
  auto& dstField = op::get<decltype(ord), Struct>(dst);

  // src doesn't have value.
  op::copy(srcField, dstField);
  EXPECT_EQ(dstField, nullptr);
  op::ensure<FieldTag>(dstField, dst);
  op::copy(srcField, dstField);
  EXPECT_EQ(dstField, nullptr);
  // src has value.
  op::ensure<FieldTag>(srcField, src);
  copyUniquePointer(srcField, srcField, dstField);
  copyUniquePointer(srcField, srcField, dstField);

  // src is const unique_ptr&
  const auto& srcConstField = srcField;
  copyUniquePointer(srcField, srcConstField, dstField);
  // src is unique_ptr&&
  auto&& srcRvalueField = std::move(srcField);
  copyUniquePointer(
      srcField,
      std::forward<decltype(srcRvalueField)>(srcRvalueField),
      dstField);
  // src is const unique_ptr&&
  const auto&& srcRvalueConstField = std::move(srcField);
  copyUniquePointer(
      srcField,
      std::forward<decltype(srcRvalueConstField)>(srcRvalueConstField),
      dstField);
}

template <typename Src, typename Dst, typename Ord>
void testCopySharedPointer(Src src, Dst dst, Ord ord) {
  using Struct = decltype(src);
  using FieldTag = op::get_field_tag<Struct, decltype(ord)>;
  auto& srcField = op::get<decltype(ord), Struct>(src);
  auto& dstField = op::get<decltype(ord), Struct>(dst);

  // src doesn't have value.
  op::copy(srcField, dstField);
  EXPECT_EQ(dstField, nullptr);
  op::ensure<FieldTag>(dstField, dst);
  op::copy(srcField, dstField);
  EXPECT_EQ(dstField, nullptr);
  // src has value.
  op::ensure<FieldTag>(srcField, src);
  copySharedPointer(srcField, dstField);

  // src is const shared_ptr&
  const auto& srcConstField = srcField;
  copySharedPointer(srcConstField, dstField);
  // src is shared_ptr&&
  auto&& srcRvalueField = std::move(srcField);
  copySharedPointer(
      std::forward<decltype(srcRvalueField)>(srcRvalueField), dstField);
  // src is const shared_ptr&&
  const auto&& srcRvalueConstField = std::move(srcField);
  copySharedPointer(
      std::forward<decltype(srcRvalueConstField)>(srcRvalueConstField),
      dstField);
}

TEST(CopyTest, FieldRefCopyNotOptional) {
  FieldRefNotOptionalStruct src, dst;
  op::for_each_ordinal<FieldRefNotOptionalStruct>([&](auto fieldOrdinalTag) {
    testCopyNotOptional(src, dst, fieldOrdinalTag);
  });
}

TEST(CopyTest, FieldRefCopyOptional) {
  FieldRefOptionalStruct src, dst;
  op::for_each_ordinal<FieldRefOptionalStruct>([&](auto fieldOrdinalTag) {
    testCopyOptional(src, dst, fieldOrdinalTag);
  });
}

TEST(CopyTest, UniquePointer) {
  UniquePointerStruct src, dst;
  op::for_each_ordinal<UniquePointerStruct>([&](auto fieldOrdinalTag) {
    testCopyUniquePointer(src, dst, fieldOrdinalTag);
  });
}

TEST(CopyTest, SharedPointer) {
  SharedPointerStruct src, dst;
  op::for_each_ordinal<SharedPointerStruct>([&](auto fieldOrdinalTag) {
    testCopySharedPointer(src, dst, fieldOrdinalTag);
  });
}

TEST(CopyTest, Optional) {
  std::optional<int> src, dst;
  op::copy(src, dst);
  EXPECT_FALSE(bool(dst));
  src = 1;
  op::copy(src, dst);
  EXPECT_EQ(dst.value(), 1);
  // test src is const reference
  const std::optional<int>& srcConst = src;
  src = 2;
  op::copy(srcConst, dst);
  EXPECT_EQ(dst.value(), 2);
}
} // namespace apache::thrift::test
