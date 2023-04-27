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

#include <thrift/lib/cpp2/protocol/Patch.h>

#include <stdexcept>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_fatal_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>
#include <thrift/test/testset/Testset.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

namespace apache::thrift::protocol {
namespace {

using ListPatch = std::decay_t<
    decltype(*std::declval<test::patch::MyStructFieldPatch>()->optListVal())>;
using SetPatch = std::decay_t<
    decltype(*std::declval<test::patch::MyStructFieldPatch>()->optSetVal())>;
using MapPatch = std::decay_t<
    decltype(*std::declval<test::patch::MyStructFieldPatch>()->optMapVal())>;

class PatchTest : public testing::Test {
 protected:
  static Value asVal(bool val) { return asValueStruct<type::bool_t>(val); }

  template <typename S>
  static Object convertToObject(const S& structObj) {
    // Serialize to compact.
    std::string buffer;
    CompactSerializer::serialize(structObj, &buffer);
    auto binaryObj = folly::IOBuf::wrapBuffer(buffer.data(), buffer.size());
    // Parse to Object.
    return parseObject<CompactProtocolReader>(*binaryObj);
  }

  template <typename Tag, typename S>
  static Value convertToValueObject(const S& structObj) {
    if constexpr (type::structured_types::template contains<Tag>()) {
      Value value;
      value.objectValue_ref() = convertToObject(structObj);
      return value;
    }
    return asValueStruct<Tag>(structObj);
  }

  template <typename P>
  static Object convertPatchToObject(const P& patchStruct) {
    return convertToObject(patchStruct.toThrift());
  }

  template <typename Tag, typename T, typename Patch>
  T applyGeneratedPatch(T value, Patch patch) {
    auto valueObject = convertToValueObject<Tag>(value);
    applyPatch(convertPatchToObject(patch), valueObject);
    auto buffer = serializeValue<CompactProtocolWriter>(valueObject);

    T patched;
    CompactSerializer::deserialize(buffer.get(), patched);
    return patched;
  }

  template <typename P>
  static Value apply(const P& patchStruct, Value val) {
    Object patchObj = convertPatchToObject(patchStruct);
    applyPatch(patchObj, val);
    return val;
  }

  template <typename P, typename V>
  static Value apply(const P& patchStruct, V&& val) {
    return apply(patchStruct, asVal(val));
  }

  static bool checkReadWriteMask(
      const ExtractedMasks& masks, const Mask& read, const Mask& write) {
    return masks.read == read && masks.write == write;
  }
  static bool checkReadWriteMask(
      const ExtractedMasks& masks, const Mask& mask) {
    return checkReadWriteMask(masks, mask, mask);
  }

  static bool isMaskNoop(const protocol::Object& patchObj) {
    return checkReadWriteMask(extractMaskViewFromPatch(patchObj), noneMask()) &&
        checkReadWriteMask(extractMaskFromPatch(patchObj), noneMask());
  }

  static bool isMaskReadWriteOperation(const protocol::Object& patchObj) {
    return checkReadWriteMask(extractMaskViewFromPatch(patchObj), allMask()) &&
        checkReadWriteMask(extractMaskFromPatch(patchObj), allMask());
  }

  static bool isMaskWriteOperation(const protocol::Object& patchObj) {
    return checkReadWriteMask(
               extractMaskViewFromPatch(patchObj), noneMask(), allMask()) &&
        checkReadWriteMask(
               extractMaskFromPatch(patchObj), noneMask(), allMask());
  }

  template <typename PatchType, typename F>
  void testNumericPatchObject(Value value, F unpacker) {
    // Noop
    EXPECT_EQ(42, unpacker(apply(PatchType{}, value)));

    // Assign
    EXPECT_EQ(43, unpacker(apply(PatchType{} = 43, value)));

    // Add
    EXPECT_EQ(43, unpacker(apply(PatchType{} + 1, value)));

    // Subtract
    EXPECT_EQ(41, unpacker(apply(PatchType{} - 1, value)));

    // Wrong patch provided
    EXPECT_THROW(apply(!op::BoolPatch{}, value), std::runtime_error);

    // Wrong object to patch
    EXPECT_THROW(apply(PatchType{} = 42, true), std::runtime_error);

    // Test getting mask from patch
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(PatchType{})));
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(PatchType{} + 0)));
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(PatchType{} - 0)));
    EXPECT_TRUE(isMaskWriteOperation(convertPatchToObject(PatchType{} = 43)));
    EXPECT_TRUE(
        isMaskReadWriteOperation(convertPatchToObject(PatchType{} + 1)));
    EXPECT_TRUE(
        isMaskReadWriteOperation(convertPatchToObject(PatchType{} - 1)));
  }

  static Object patchAddOperation(Object&& patch, auto operation, auto value) {
    auto opId = static_cast<int16_t>(operation);
    patch.members()[opId] = value;
    return std::move(patch);
  }

  static Object makePatch(auto operation, auto value) {
    return patchAddOperation(Object{}, operation, value);
  }

  static Value applyContainerPatch(Object patch, Value value) {
    auto buffer = serializeObject<CompactProtocolWriter>(patch);
    Object patchObj = parseObject<CompactProtocolReader>(*buffer);

    applyPatch(patchObj, value);
    return value;
  }

  static bool isBinaryEqual(Value& value, std::string_view expected) {
    const auto buf =
        folly::IOBuf::wrapBufferAsValue(expected.data(), expected.size());
    return folly::IOBufEqualTo{}(value.as_binary(), buf);
  }
};

TEST_F(PatchTest, Bool) {
  // Noop
  EXPECT_TRUE(*apply(op::BoolPatch{}, true).boolValue_ref());
  EXPECT_FALSE(*apply(op::BoolPatch{}, false).boolValue_ref());

  // Assign
  EXPECT_TRUE(*apply(op::BoolPatch{} = true, true).boolValue_ref());
  EXPECT_TRUE(*apply(op::BoolPatch{} = true, false).boolValue_ref());
  EXPECT_FALSE(*apply(op::BoolPatch{} = false, true).boolValue_ref());
  EXPECT_FALSE(*apply(op::BoolPatch{} = false, false).boolValue_ref());

  // Invert
  EXPECT_TRUE(*apply(!op::BoolPatch{}, false).boolValue_ref());
  EXPECT_FALSE(*apply(!op::BoolPatch{}, true).boolValue_ref());

  // Wrong patch provided
  EXPECT_THROW(apply(op::I16Patch{} += 1, true), std::runtime_error);

  // Wrong object to patch
  EXPECT_THROW(
      apply(!op::BoolPatch{}, asValueStruct<type::i16_t>(42)),
      std::runtime_error);

  // Test getting mask from patch
  EXPECT_TRUE(isMaskNoop(convertPatchToObject(op::BoolPatch{})));
  EXPECT_TRUE(
      isMaskWriteOperation(convertPatchToObject(op::BoolPatch{} = true)));
  EXPECT_TRUE(
      isMaskWriteOperation(convertPatchToObject(op::BoolPatch{} = false)));
  EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(!op::BoolPatch{})));

  // Should we check non-patch objects passed as patch? Previous checks kind of
  // cover this.
}

TEST_F(PatchTest, Byte) {
  testNumericPatchObject<op::BytePatch>(
      asValueStruct<type::byte_t>(42),
      [](auto val) { return *val.byteValue_ref(); });
}

TEST_F(PatchTest, I16) {
  testNumericPatchObject<op::I16Patch>(
      asValueStruct<type::i16_t>(42),
      [](auto val) { return *val.i16Value_ref(); });
}

TEST_F(PatchTest, I32) {
  testNumericPatchObject<op::I32Patch>(
      asValueStruct<type::i32_t>(42),
      [](auto val) { return *val.i32Value_ref(); });
}

TEST_F(PatchTest, I64) {
  testNumericPatchObject<op::I64Patch>(
      asValueStruct<type::i64_t>(42),
      [](auto val) { return *val.i64Value_ref(); });
}

TEST_F(PatchTest, Float) {
  testNumericPatchObject<op::FloatPatch>(
      asValueStruct<type::float_t>(42),
      [](auto val) { return *val.floatValue_ref(); });
}

TEST_F(PatchTest, Double) {
  testNumericPatchObject<op::DoublePatch>(
      asValueStruct<type::double_t>(42),
      [](auto val) { return *val.doubleValue_ref(); });
}

TEST_F(PatchTest, Binary) {
  std::string data = "test", patch = "best";
  auto toPatch = folly::IOBuf::wrapBufferAsValue(data.data(), data.size());
  auto patchValue = folly::IOBuf::wrapBufferAsValue(patch.data(), patch.size());
  auto binaryData = asValueStruct<type::binary_t>(toPatch);
  // Noop
  EXPECT_TRUE(folly::IOBufEqualTo{}(
      toPatch, *apply(op::BinaryPatch{}, binaryData).binaryValue_ref()));

  // Assign
  EXPECT_TRUE(apply(op::BinaryPatch{} = folly::IOBuf(), binaryData)
                  .binaryValue_ref()
                  ->empty());
  EXPECT_TRUE(folly::IOBufEqualTo{}(
      patchValue,
      *apply(op::BinaryPatch{} = patchValue, binaryData).binaryValue_ref()));

  // Append
  {
    op::BinaryPatch binPatch;
    binPatch.append(patchValue);
    std::string appended = data + patch;
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        folly::IOBuf::wrapBufferAsValue(appended.data(), appended.size()),
        *apply(binPatch, binaryData).binaryValue_ref()));
    EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(binPatch)));
  }
  {
    op::BinaryPatch binPatch;
    binPatch.append("");
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        toPatch, *apply(binPatch, binaryData).binaryValue_ref()));
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(binPatch)));
  }

  // Prepend
  {
    op::BinaryPatch binPatch;
    binPatch.prepend(patch);
    std::string appended = patch + data;
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        folly::IOBuf::wrapBufferAsValue(appended.data(), appended.size()),
        *apply(binPatch, binaryData).binaryValue_ref()));
    EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(binPatch)));
  }
  {
    op::BinaryPatch binPatch;
    binPatch.prepend("");
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        toPatch, *apply(binPatch, binaryData).binaryValue_ref()));
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(binPatch)));
  }

  // Wrong patch provided
  EXPECT_THROW(apply(op::I16Patch{} = 42, binaryData), std::runtime_error);

  // Wrong object to patch
  EXPECT_THROW(
      apply(op::BinaryPatch{} = patchValue, asValueStruct<type::i16_t>(42)),
      std::runtime_error);

  // Test getting mask from patch
  EXPECT_TRUE(isMaskNoop(convertPatchToObject(op::BinaryPatch{})));
  EXPECT_TRUE(isMaskWriteOperation(
      convertPatchToObject(op::BinaryPatch{} = folly::IOBuf())));
}

TEST_F(PatchTest, String) {
  std::string data = "test", patch = "best";
  auto stringData = asValueStruct<type::string_t>(data);
  // Noop
  EXPECT_EQ(data, *apply(op::StringPatch{}, stringData).stringValue_ref());
  EXPECT_TRUE(isMaskNoop(convertPatchToObject(op::StringPatch{})));

  // Assign
  EXPECT_EQ(
      patch, *apply(op::StringPatch{} = patch, stringData).stringValue_ref());
  EXPECT_TRUE(
      isMaskWriteOperation(convertPatchToObject(op::StringPatch{} = patch)));

  // Clear
  {
    op::StringPatch strPatch;
    strPatch.clear();
    EXPECT_TRUE(apply(strPatch, stringData).stringValue_ref()->empty());
    EXPECT_TRUE(isMaskWriteOperation(convertPatchToObject(strPatch)));
  }

  // Append
  {
    op::StringPatch strPatch;
    strPatch.append(patch);
    EXPECT_EQ(data + patch, *apply(strPatch, stringData).stringValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(strPatch)));
  }
  {
    op::StringPatch strPatch;
    strPatch.append("");
    EXPECT_EQ(data, *apply(strPatch, stringData).stringValue_ref());
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(op::StringPatch{})));
  }

  // Prepend
  {
    op::StringPatch strPatch;
    strPatch.prepend(patch);
    EXPECT_EQ(patch + data, *apply(strPatch, stringData).stringValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(strPatch)));
  }
  {
    op::StringPatch strPatch;
    strPatch.prepend("");
    EXPECT_EQ(data, *apply(strPatch, stringData).stringValue_ref());
    EXPECT_TRUE(isMaskNoop(convertPatchToObject(op::StringPatch{})));
  }

  // Clear, Append and Prepend in one
  {
    op::StringPatch strPatch;
    strPatch.clear();
    strPatch.append(patch);
    strPatch.prepend(patch);
    EXPECT_EQ(patch + patch, *apply(strPatch, stringData).stringValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(convertPatchToObject(strPatch)));
  }

  // Wrong patch provided
  EXPECT_THROW(apply(op::I16Patch{} = 42, stringData), std::runtime_error);

  // Wrong object to patch
  EXPECT_THROW(
      apply(op::StringPatch{} = patch, asValueStruct<type::i16_t>(42)),
      std::runtime_error);
}

TEST_F(PatchTest, List) {
  auto value = asValueStruct<type::list<type::binary_t>>({"test"});
  auto patchValue = asValueStruct<type::list<type::binary_t>>({"new value"});
  auto emptyValue = asValueStruct<type::list<type::binary_t>>({});
  auto emptySet = asValueStruct<type::set<type::binary_t>>({});

  auto expectNoop = [&](auto& patchObj) {
    EXPECT_EQ(
        *value.listValue_ref(),
        *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskNoop(patchObj));
  };

  // Noop
  {
    Object patchObj;
    expectNoop(patchObj);
  }

  // Assign
  {
    Object patchObj = makePatch(op::PatchOp::Assign, patchValue);
    EXPECT_EQ(
        *patchValue.listValue_ref(),
        *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_EQ(
        std::vector<Value>{},
        *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // PatchPrior
  {
    auto elementPatchValue = asValueStruct<type::binary_t>("best");
    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() =
        makePatch(op::PatchOp::Put, elementPatchValue);
    Value listElementPatch;
    int32_t zigZag = apache::thrift::util::i32ToZigzag(
        static_cast<int32_t>(type::toOrdinal(0)));
    listElementPatch.mapValue_ref()
        .ensure()[asValueStruct<type::i32_t>(zigZag)] = fieldPatchValue;
    auto patchObj = makePatch(op::PatchOp::PatchPrior, listElementPatch);
    auto patched = *applyContainerPatch(patchObj, value).listValue_ref();
    EXPECT_EQ(
        std::vector<Value>{asValueStruct<type::binary_t>("testbest")}, patched);
    // It is a map mask as Patch can't distinguish between list and map.
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      EXPECT_EQ(((Value*)mask.begin()->first)->as_i32(), zigZag);
      EXPECT_EQ(mask.begin()->second, allMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      EXPECT_EQ(mask.begin()->first, zigZag);
      EXPECT_EQ(mask.begin()->second, allMask());
    }
  }
  {
    auto emptyMapValue =
        asValueStruct<type::map<type::i32_t, type::binary_t>>({});
    Object patchObj = makePatch(op::PatchOp::PatchPrior, emptyMapValue);
    expectNoop(patchObj);
  }

  // Prepend
  {
    auto expected = *patchValue.listValue_ref();
    expected.insert(
        expected.end(),
        value.listValue_ref()->begin(),
        value.listValue_ref()->end());
    Object patchObj = makePatch(op::PatchOp::Add, patchValue);
    EXPECT_EQ(expected, *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Add, emptyValue);
    expectNoop(patchObj);
  }

  // Append
  {
    auto expected = *value.listValue_ref();
    expected.insert(
        expected.end(),
        patchValue.listValue_ref()->begin(),
        patchValue.listValue_ref()->end());
    Object patchObj = makePatch(op::PatchOp::Put, patchValue);
    EXPECT_EQ(expected, *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Put, emptyValue);
    expectNoop(patchObj);
  }

  // Remove
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove,
        asValueStruct<type::set<type::binary_t>>(std::set{"test"}));
    EXPECT_EQ(
        std::vector<Value>{},
        *applyContainerPatch(patchObj, value).listValue_ref());
    // It is a map mask as Remove can't distinguish between list, set, and map.
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      isBinaryEqual(*((Value*)mask.begin()->first), "test");
      EXPECT_EQ(mask.begin()->second, allMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_string_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      EXPECT_EQ(mask.begin()->first, "test");
      EXPECT_EQ(mask.begin()->second, allMask());
    }
  }
  {
    Object patchObj = makePatch(op::PatchOp::Remove, emptySet);
    expectNoop(patchObj);
  }

  // Add
  {
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>(std::set{"test"}));
    EXPECT_EQ(
        *value.listValue_ref(),
        *applyContainerPatch(patchObj, value).listValue_ref())
        << "Shuold insert nothing";
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    auto expected = *value.listValue_ref();
    expected.insert(expected.begin(), asValueStruct<type::binary_t>("best"));
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>(std::set{"best"}));
    EXPECT_EQ(expected, *applyContainerPatch(patchObj, value).listValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Add, emptySet);
    expectNoop(patchObj);
  }
}

TEST_F(PatchTest, GeneratedListPatch) {
  ListPatch patch;
  patch.prepend({3, 4});
  patch.emplace_front(2);
  patch.push_front(1);
  using Vec = ListPatch::value_type;
  Vec actual{5, 6};
  patch.append({7, 8});
  patch.emplace_back(9);
  patch.push_back(10);

  auto patched = applyGeneratedPatch<type::list<type::i16_t>>(actual, patch);
  EXPECT_EQ(patched, (Vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

  patched = applyGeneratedPatch<type::list<type::i16_t>>(patched, patch);
  EXPECT_EQ(
      patched, (Vec{1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 7, 8, 9, 10}));

  ListPatch erasePatch;
  erasePatch.erase(1);
  patched = applyGeneratedPatch<type::list<type::i16_t>>(
      Vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, erasePatch);
  EXPECT_EQ(patched, (Vec{2, 3, 4, 5, 6, 7, 8, 9, 10}));

  ListPatch removePatch;
  removePatch.remove({1, 2, 3, 4});

  patched = applyGeneratedPatch<type::list<type::i16_t>>(
      Vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, removePatch);
  EXPECT_EQ(patched, (Vec{5, 6, 7, 8, 9, 10}));
}

TEST_F(PatchTest, Set) {
  auto value = asValueStruct<type::set<type::binary_t>>({"test"});
  auto patchValue = asValueStruct<type::set<type::binary_t>>({"new value"});
  auto emptySet = asValueStruct<type::set<type::binary_t>>({});

  auto expectNoop = [&](auto& patchObj) {
    EXPECT_EQ(
        *value.setValue_ref(),
        *applyContainerPatch(patchObj, value).setValue_ref());
    EXPECT_TRUE(isMaskNoop(patchObj));
  };

  // Noop
  {
    Object patchObj;
    expectNoop(patchObj);
  }

  // Assign
  {
    Object patchObj = makePatch(op::PatchOp::Assign, patchValue);
    EXPECT_EQ(
        *patchValue.setValue_ref(),
        *applyContainerPatch(patchObj, value).setValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_EQ(
        std::set<Value>{},
        *applyContainerPatch(patchObj, value).setValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // Put
  {
    auto expected = *value.setValue_ref();
    expected.insert(
        patchValue.setValue_ref()->begin(), patchValue.setValue_ref()->end());
    Object patchObj = makePatch(op::PatchOp::Put, patchValue);
    EXPECT_EQ(expected, *applyContainerPatch(patchObj, value).setValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Put, emptySet);
    expectNoop(patchObj);
  }

  // Remove
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove,
        asValueStruct<type::set<type::binary_t>>({"test"}));
    EXPECT_EQ(
        std::set<Value>{},
        *applyContainerPatch(patchObj, value).setValue_ref());
    // It is a map mask as Remove can't distinguish between list, set, and
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      isBinaryEqual(*((Value*)mask.begin()->first), "test");
      EXPECT_EQ(mask.begin()->second, allMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_EQ(masks.read, masks.write);
      auto mask = masks.read.includes_string_map_ref().value();
      EXPECT_EQ(mask.size(), 1);
      EXPECT_EQ(mask.begin()->first, "test");
      EXPECT_EQ(mask.begin()->second, allMask());
    }
  }
  {
    Object patchObj = makePatch(op::PatchOp::Remove, emptySet);
    expectNoop(patchObj);
  }

  // Add
  {
    Object patchObj = makePatch(
        op::PatchOp::Add, asValueStruct<type::set<type::binary_t>>({"test"}));
    EXPECT_EQ(
        *value.setValue_ref(),
        *applyContainerPatch(patchObj, value).setValue_ref())
        << "Shuold insert nothing";
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    auto expected = *value.setValue_ref();
    expected.insert(asValueStruct<type::binary_t>("best test"));
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>({"best test"}));
    auto patchResult = *applyContainerPatch(patchObj, value).setValue_ref();
    EXPECT_EQ(expected, patchResult);
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Add, emptySet);
    expectNoop(patchObj);
  }

  // Combination
  {
    auto patchObj = patchAddOperation(
        patchAddOperation(
            makePatch(
                op::PatchOp::Add,
                asValueStruct<type::set<type::binary_t>>({"best"})),
            op::PatchOp::Remove,
            asValueStruct<type::set<type::binary_t>>({"test"})),
        op::PatchOp::Put,
        asValueStruct<type::set<type::binary_t>>({"rest"}));

    auto expected = asValueStruct<type::set<type::binary_t>>({"best", "rest"});
    EXPECT_EQ(
        *expected.setValue_ref(),
        *applyContainerPatch(patchObj, value).setValue_ref());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
}

TEST_F(PatchTest, GeneratedSetPatch) {
  SetPatch patch;
  patch.erase("a");
  patch.emplace("a");
  patch.insert("b");
  patch.add({"c"});
  patch.remove({"c", "d"});

  auto patched = applyGeneratedPatch<type::set<type::binary_t>>(
      std::set<std::string>{}, patch);
  EXPECT_EQ(patched, (std::set<std::string>{"a", "b"}));

  patched = applyGeneratedPatch<type::set<type::binary_t>>(
      std::set<std::string>{"a", "d", "e"}, patch);
  EXPECT_EQ(patched, (std::set<std::string>{"a", "b", "e"}));
}

TEST_F(PatchTest, Map) {
  auto value = asValueStruct<type::map<type::binary_t, type::binary_t>>(
      {{"key", "test"}});
  auto patchValue = asValueStruct<type::map<type::binary_t, type::binary_t>>(
      {{"new key", "new value"}});
  auto emptyMap = std::map<Value, Value>{};
  auto emptySet = asValueStruct<type::set<type::binary_t>>({});
  auto emptyValue =
      asValueStruct<type::map<type::binary_t, type::binary_t>>({});

  auto expectNoop = [&](auto&& patchObj) {
    EXPECT_EQ(
        *value.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref());
    EXPECT_TRUE(isMaskNoop(patchObj));
  };

  // Checks if the map mask generated from patchObj contains the expectKeys in
  // both read and write masks.
  auto checkMapMask =
      [&](auto&& patchObj,
          const std::unordered_set<std::string_view>& expectKeys) {
        {
          auto masks = extractMaskViewFromPatch(patchObj);
          EXPECT_EQ(masks.read, masks.write);
          auto mask = masks.read.includes_map_ref().value();
          EXPECT_EQ(mask.size(), expectKeys.size());
          for (auto& kv : mask) {
            EXPECT_TRUE(std::any_of(
                expectKeys.begin(), expectKeys.end(), [&](auto& expectKey) {
                  return isBinaryEqual(*((Value*)kv.first), expectKey);
                }));
            EXPECT_EQ(kv.second, allMask());
          }
        }
        {
          auto masks = extractMaskFromPatch(patchObj);
          EXPECT_EQ(masks.read, masks.write);
          auto mask = masks.read.includes_string_map_ref().value();
          EXPECT_EQ(mask.size(), expectKeys.size());
          for (auto& kv : mask) {
            EXPECT_TRUE(std::any_of(
                expectKeys.begin(), expectKeys.end(), [&](auto& expectKey) {
                  return kv.first == expectKey;
                }));
            EXPECT_EQ(kv.second, allMask());
          }
        }
      };

  // Noop
  {
    Object patchObj;
    expectNoop(patchObj);
  }

  // Assign
  {
    Object patchObj = makePatch(op::PatchOp::Assign, patchValue);
    EXPECT_EQ(
        *patchValue.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_EQ(emptyMap, *applyContainerPatch(patchObj, value).mapValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // Remove
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove, asValueStruct<type::set<type::binary_t>>({"key"}));
    EXPECT_EQ(emptyMap, *applyContainerPatch(patchObj, value).mapValue_ref());
    checkMapMask(patchObj, {"key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::Remove, emptySet);
    expectNoop(patchObj);
  }

  // Ensure
  {
    Object patchObj = makePatch(
        op::PatchOp::EnsureStruct,
        asValueStruct<type::map<type::binary_t, type::binary_t>>(
            {{"key", "test 42"}}));
    EXPECT_EQ(
        *value.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref())
        << "Shuold insert nothing";
    checkMapMask(patchObj, {"key"});
  }
  {
    auto expected = *value.mapValue_ref();
    expected.emplace(
        asValueStruct<type::binary_t>("new key"),
        asValueStruct<type::binary_t>("new value"));
    Object patchObj = makePatch(
        op::PatchOp::EnsureStruct,
        asValueStruct<type::map<type::binary_t, type::binary_t>>(
            {{"new key", "new value"}}));
    auto patchResult = *applyContainerPatch(patchObj, value).mapValue_ref();
    EXPECT_EQ(expected, patchResult);
    checkMapMask(patchObj, {"new key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::EnsureStruct, emptyValue);
    expectNoop(patchObj);
  }

  // Put
  {
    auto expected = *value.mapValue_ref();
    expected[asValueStruct<type::binary_t>("key")] =
        asValueStruct<type::binary_t>("key updated value");
    Object patchObj = makePatch(
        op::PatchOp::Put,
        asValueStruct<type::map<type::binary_t, type::binary_t>>(
            {{"key", "key updated value"}}));
    EXPECT_EQ(expected, *applyContainerPatch(patchObj, value).mapValue_ref());
    checkMapMask(patchObj, {"key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::Put, emptyValue);
    expectNoop(patchObj);
  }

  // Combination
  {
    auto expected = asValueStruct<type::map<type::binary_t, type::binary_t>>(
        {{"new key", "new value"}, {"added key", "overridden value"}});
    auto patchObj = patchAddOperation(
        patchAddOperation(
            makePatch(
                op::PatchOp::EnsureStruct,
                asValueStruct<type::map<type::binary_t, type::binary_t>>(
                    {{"added key", "added value"}})),
            op::PatchOp::Remove,
            asValueStruct<type::set<type::binary_t>>({"key"})),
        op::PatchOp::Put,
        expected);

    EXPECT_EQ(
        *expected.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref());
    checkMapMask(patchObj, {"key", "new key", "added key"});
  }

  // PatchPrior
  {
    auto value =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", {"test"}}});
    auto elementPatchValue = asValueStruct<type::list<type::binary_t>>({"foo"});
    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() =
        makePatch(op::PatchOp::Put, elementPatchValue);
    Value mapPatch;
    mapPatch.mapValue_ref().ensure()[asValueStruct<type::binary_t>("key")] =
        fieldPatchValue;
    auto patchObj = makePatch(op::PatchOp::PatchPrior, mapPatch);
    auto expected =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", std::vector<std::string>{"test", "foo"}}});
    EXPECT_EQ(
        *expected.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref());

    checkMapMask(patchObj, {"key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::PatchPrior, emptyValue);
    expectNoop(patchObj);
  }

  // Ensure and PatchAfter
  {
    auto value =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", {"test"}}});
    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() = makePatch(
        op::PatchOp::Put, asValueStruct<type::list<type::binary_t>>({"foo"}));
    Value mapPatch;
    mapPatch.mapValue_ref().ensure()[asValueStruct<type::binary_t>("new key")] =
        fieldPatchValue;

    auto patchObj = patchAddOperation(
        makePatch(op::PatchOp::PatchAfter, mapPatch),
        op::PatchOp::EnsureStruct,
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"new key", std::vector<std::string>{}}}));

    auto expected =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", std::vector<std::string>{"test"}},
             {"new key", std::vector<std::string>{"foo"}}});
    EXPECT_EQ(
        *expected.mapValue_ref(),
        *applyContainerPatch(patchObj, value).mapValue_ref());
    checkMapMask(patchObj, {"new key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::PatchAfter, emptyValue);
    expectNoop(patchObj);
  }
}

TEST_F(PatchTest, EnsureAndPatchObject) {
  // If field 1 doesn't exist, set it to an object that has field_2, whose
  // value is 3
  Value ensure;
  ensure.emplace_object()[FieldId{1}]
      .emplace_object()[FieldId{2}]
      .emplace_i32() = 3;

  // Assign 5 to field 1's field 4
  Value fieldPatch;
  fieldPatch.emplace_object()[FieldId{1}]
      .emplace_object()[FieldId(op::PatchOp::PatchPrior)]
      .emplace_object()[FieldId{4}]
      .emplace_object()[FieldId(op::PatchOp::Assign)]
      .emplace_i32() = 5;

  auto patchObj = patchAddOperation(
      makePatch(op::PatchOp::EnsureStruct, ensure),
      op::PatchOp::PatchAfter,
      fieldPatch);

  // To ensure field 1, we need to read/write this field
  Mask mask;
  mask.includes_ref().emplace()[1] = allMask();
  EXPECT_TRUE(checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask));
  EXPECT_TRUE(checkReadWriteMask(extractMaskFromPatch(patchObj), mask));
}

TEST_F(PatchTest, GeneratedMapPatch) {
  auto assignPatch = MapPatch::createAssign({{"a", "5"}, {"c", "6"}});
  auto patched = applyGeneratedPatch<type::map<type::string_t, type::string_t>>(
      std::map<std::string, std::string>{}, assignPatch);
  EXPECT_EQ(
      patched, (std::map<std::string, std::string>{{"a", "5"}, {"c", "6"}}));

  MapPatch patch;
  patch.put({{"a", "1"}, {"b", "2"}});
  patch.insert_or_assign("b", "3");
  patch.insert_or_assign("c", "4");

  patched = applyGeneratedPatch<type::map<type::string_t, type::string_t>>(
      std::map<std::string, std::string>{}, patch);
  EXPECT_EQ(
      patched,
      (std::map<std::string, std::string>{{"a", "1"}, {"b", "3"}, {"c", "4"}}));

  assignPatch = MapPatch::createAssign({{"a", "5"}, {"c", "6"}});
  assignPatch.put({{"a", "1"}, {"b", "2"}});
  assignPatch.insert_or_assign("b", "3");
  assignPatch.insert_or_assign("c", "4");

  patched = applyGeneratedPatch<type::map<type::string_t, type::string_t>>(
      std::map<std::string, std::string>{}, assignPatch);
  EXPECT_EQ(
      patched,
      (std::map<std::string, std::string>{{"a", "1"}, {"b", "3"}, {"c", "4"}}));

  MapPatch addPatch;
  addPatch.add({{"a", "1"}, {"b", "2"}});
  patched = applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
      std::map<std::string, std::string>{}, addPatch);
  EXPECT_EQ(
      patched, (std::map<std::string, std::string>{{"a", "1"}, {"b", "2"}}));
  patched = applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
      std::map<std::string, std::string>{{"a", "0"}, {"c", "3"}}, addPatch);
  EXPECT_EQ(
      patched,
      (std::map<std::string, std::string>{{"a", "0"}, {"b", "2"}, {"c", "3"}}));

  MapPatch erasePatch;
  erasePatch.add({{"a", "1"}, {"b", "2"}});
  erasePatch.erase("c");
  patched = applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
      std::map<std::string, std::string>{}, erasePatch);
  EXPECT_EQ(
      patched, (std::map<std::string, std::string>{{"a", "1"}, {"b", "2"}}));
  auto patched2 =
      applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
          std::map<std::string, std::string>{{"a", "0"}, {"c", "3"}},
          erasePatch);
  EXPECT_EQ(
      patched2, (std::map<std::string, std::string>{{"a", "0"}, {"b", "2"}}));

  MapPatch removePatch;
  removePatch.add({{"a", "1"}, {"b", "2"}});
  removePatch.remove({"c", "d"});
  patched = applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
      std::map<std::string, std::string>{}, removePatch);
  EXPECT_EQ(
      patched, (std::map<std::string, std::string>{{"a", "1"}, {"b", "2"}}));
  patched = applyGeneratedPatch<type::map<type::binary_t, type::binary_t>>(
      std::map<std::string, std::string>{{"a", "0"}, {"c", "3"}, {"d", "4"}},
      removePatch);
  EXPECT_EQ(
      patched, (std::map<std::string, std::string>{{"a", "0"}, {"b", "2"}}));
}

TEST_F(PatchTest, Struct) {
  test::testset::struct_with<type::list<type::i32_t>> valueObject;
  test::testset::struct_with<type::list<type::i32_t>> patchObject;

  valueObject.field_1_ref() = std::vector<int>{1, 2, 3};
  patchObject.field_1_ref() = std::vector<int>{3, 2, 1};

  auto value = asValueStruct<type::struct_c>(valueObject);
  auto patchValue = asValueStruct<type::struct_c>(patchObject);

  auto expectNoop = [&](auto&& patchObj) {
    EXPECT_EQ(
        *value.objectValue_ref(),
        *applyContainerPatch(patchObj, value).objectValue_ref());
    EXPECT_TRUE(isMaskNoop(patchObj));
  };

  // Noop
  {
    Object patchObj;
    expectNoop(patchObj);
  }

  // Assign
  {
    Object patchObj = makePatch(op::PatchOp::Assign, patchValue);
    EXPECT_EQ(
        *patchValue.objectValue_ref(),
        *applyContainerPatch(patchObj, value).objectValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_TRUE(applyContainerPatch(patchObj, value)
                    .objectValue_ref()
                    ->members()
                    ->empty());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // PatchPrior
  auto applyFieldPatchTest = [&](auto op, auto expected) {
    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() =
        makePatch(op, asValueStruct<type::list<type::i32_t>>({3, 2, 1}));
    Value fieldPatch;
    fieldPatch.objectValue_ref().ensure().members().ensure()[1] =
        fieldPatchValue;
    Object patchObj = makePatch(op::PatchOp::PatchPrior, fieldPatch);
    EXPECT_EQ(
        expected,
        applyContainerPatch(patchObj, value)
            .objectValue_ref()
            ->members()
            .ensure()[1]);

    Mask mask;
    mask.includes_ref().emplace()[1] = allMask();
    EXPECT_TRUE(checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask));
    EXPECT_TRUE(checkReadWriteMask(extractMaskFromPatch(patchObj), mask));
  };

  applyFieldPatchTest(
      op::PatchOp::Assign, patchValue.objectValue_ref()->members().ensure()[1]);

  applyFieldPatchTest(
      op::PatchOp::Put,
      asValueStruct<type::list<type::i32_t>>({1, 2, 3, 3, 2, 1}));

  // Ensure and Patch
  {
    test::testset::struct_with<type::list<type::i32_t>> source;
    auto sourceValue = asValueStruct<type::struct_c>(source);

    Value ensureValuePatch;
    Object ensureObject;
    ensureObject.members().ensure()[1] =
        asValueStruct<type::list<type::i32_t>>({});
    ensureValuePatch.objectValue_ref() = ensureObject;

    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() = makePatch(
        op::PatchOp::Put, asValueStruct<type::list<type::i32_t>>({42}));
    Value fieldPatch;
    fieldPatch.objectValue_ref().ensure().members().ensure()[1] =
        fieldPatchValue;

    Object patchObj = patchAddOperation(
        makePatch(op::PatchOp::PatchAfter, fieldPatch),
        op::PatchOp::EnsureStruct,
        ensureValuePatch);

    EXPECT_EQ(
        asValueStruct<type::list<type::i32_t>>({42}),
        applyContainerPatch(patchObj, sourceValue)
            .objectValue_ref()
            ->members()
            .ensure()[1]);

    Mask mask;
    mask.includes_ref().emplace()[1] = allMask();
    EXPECT_TRUE(checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask));
    EXPECT_TRUE(checkReadWriteMask(extractMaskFromPatch(patchObj), mask));
  }
  // Ensure Fail
  {
    test::testset::struct_with<type::list<type::i32_t>> source;
    auto sourceValue = asValueStruct<type::struct_c>(source);

    Object patchObj =
        makePatch(op::PatchOp::EnsureStruct, asValueStruct<type::i32_t>(42));

    EXPECT_THROW(
        applyContainerPatch(patchObj, sourceValue), std::runtime_error);
  }
  {
    Value fieldPatch;
    fieldPatch.objectValue_ref().ensure();
    expectNoop(makePatch(op::PatchOp::PatchPrior, fieldPatch));
    expectNoop(makePatch(op::PatchOp::EnsureStruct, fieldPatch));
    expectNoop(makePatch(op::PatchOp::PatchAfter, fieldPatch));
  }
}

TEST_F(PatchTest, GeneratedStructPatch) {
  test::patch::MyStruct original;
  original.boolVal() = true;
  original.byteVal() = 42;
  original.stringVal() = "test";

  test::patch::MyStructPatch patch;
  patch.patchIfSet<ident::boolVal>() = !op::BoolPatch{};
  patch.patchIfSet<ident::byteVal>() = original.byteVal();
  patch.patchIfSet<ident::i16Val>() += 2;
  patch.patchIfSet<ident::i32Val>() += 3;
  patch.patchIfSet<ident::i64Val>() += 4;
  patch.patchIfSet<ident::floatVal>() += 5;
  patch.patchIfSet<ident::doubleVal>() += 6;
  patch.patchIfSet<ident::stringVal>() = "_" + op::StringPatch{} + "_";
  patch.patchIfSet<ident::structVal>().patchIfSet<ident::data1>().append("Na");

  auto patched = applyGeneratedPatch<type::struct_c>(original, patch);

  EXPECT_FALSE(*patched.boolVal());
  EXPECT_EQ("_test_", *patched.stringVal());
  EXPECT_EQ(*original.byteVal(), *patched.byteVal());
  EXPECT_EQ(2, *patched.i16Val());
  EXPECT_EQ(3, *patched.i32Val());
  EXPECT_EQ(4, *patched.i64Val());
  EXPECT_EQ(5, *patched.floatVal());
  EXPECT_EQ(6, *patched.doubleVal());
  EXPECT_EQ("Na", *patched.structVal()->data1());
}

TEST_F(PatchTest, GeneratedUnionEnsurePatch) {
  test::patch::MyUnion original;

  test::patch::MyUnionPatch patch;
  patch.ensure().option1_ref() = "test";

  auto patched = applyGeneratedPatch<type::union_c>(original, patch);
  ASSERT_TRUE(patched.option1_ref().has_value());
  EXPECT_EQ("test", *patched.option1_ref());

  patched.option1_ref() = "updated";
  patched = applyGeneratedPatch<type::union_c>(patched, patch);
  ASSERT_TRUE(patched.option1_ref().has_value());
  EXPECT_EQ("updated", *patched.option1_ref());

  patched.option2_ref() = 42;
  patched = applyGeneratedPatch<type::union_c>(patched, patch);
  ASSERT_TRUE(patched.option1_ref().has_value());
  EXPECT_EQ("test", *patched.option1_ref());
}

TEST_F(PatchTest, GeneratedUnionClearAndAssign) {
  test::patch::MyUnionPatch noop;
  test::patch::MyUnion actual;
  test::patch::MyUnionPatch assignEmpty =
      test::patch::MyUnionPatch::createAssign(actual);

  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(actual, noop), test::patch::MyUnion{});
  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(actual, assignEmpty),
      test::patch::MyUnion{});

  actual.option1_ref() = "test";
  auto assign = test::patch::MyUnionPatch::createAssign(actual);

  EXPECT_EQ(applyGeneratedPatch<type::union_c>(actual, noop), actual);
  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(actual, assignEmpty),
      test::patch::MyUnion{});
  EXPECT_EQ(applyGeneratedPatch<type::union_c>(actual, assign), actual);
  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(test::patch::MyUnion{}, assign),
      actual);
}

TEST_F(PatchTest, GeneratedUnionPatch) {
  test::patch::MyUnionPatch patch;
  *patch.patchIfSet()->option1() = "Hi";
  patch.ensure().option1_ref() = "Bye";
  *patch.patchIfSet()->option1() += " World!";

  test::patch::MyUnion hi, bye;
  hi.option1_ref() = "Hi World!";
  bye.option1_ref() = "Bye World!";

  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(test::patch::MyUnion{}, patch), bye);
  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(
          applyGeneratedPatch<type::union_c>(test::patch::MyUnion{}, patch),
          patch),
      hi);

  test::patch::MyUnion op1;
  op1.option1_ref() = "Yo";
  EXPECT_EQ(applyGeneratedPatch<type::union_c>(op1, patch), hi);
  EXPECT_EQ(
      applyGeneratedPatch<type::union_c>(
          applyGeneratedPatch<type::union_c>(op1, patch), patch),
      hi);

  EXPECT_EQ(applyGeneratedPatch<type::union_c>(bye, patch), hi);
}

TEST_F(PatchTest, GeneratedUnionPatchInner) {
  test::patch::MyUnionPatch patch;
  *patch.patchIfSet()->option3()->patchIfSet()->option1() = "World";

  test::patch::MyUnion a, b;
  a.option3_ref().ensure().option1_ref() = "Hello";
  b.option3_ref().ensure().option1_ref() = "World";

  EXPECT_EQ(applyGeneratedPatch<type::union_c>(a, patch), b);
}

TEST_F(PatchTest, Union) { // Shuold mostly behave like a struct
  test::testset::union_with<type::i32_t> valueObject;
  test::testset::union_with<type::i32_t> patchObject;

  valueObject.field_1_ref() = 42;
  patchObject.field_2_ref() = 43;

  auto value = asValueStruct<type::union_c>(valueObject);
  auto patchValue = asValueStruct<type::union_c>(patchObject);

  auto expectNoop = [&](auto&& patchObj) {
    EXPECT_EQ(
        *value.objectValue_ref(),
        *applyContainerPatch(patchObj, value).objectValue_ref());
    EXPECT_TRUE(isMaskNoop(patchObj));
  };

  // Noop
  {
    Object patchObj;
    expectNoop(patchObj);
  }

  // Assign
  {
    Object patchObj = makePatch(op::PatchOp::Assign, patchValue);
    EXPECT_EQ(
        *patchValue.objectValue_ref(),
        *applyContainerPatch(patchObj, value).objectValue_ref());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_TRUE(applyContainerPatch(patchObj, value)
                    .objectValue_ref()
                    ->members()
                    ->empty());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // Ensure and PatchAfter
  {
    test::testset::union_with<type::i32_t> source;
    source.field_1_ref() = 42;
    auto sourceValue = asValueStruct<type::union_c>(source);

    Value ensureValuePatch;
    Object ensureObject;
    ensureObject.members().ensure()[2] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.objectValue_ref() = ensureObject;

    Value fieldPatchValue;
    fieldPatchValue.objectValue_ref() =
        makePatch(op::PatchOp::Add, asValueStruct<type::i32_t>(1));
    Value fieldPatch;
    fieldPatch.objectValue_ref().ensure().members().ensure()[2] =
        fieldPatchValue;

    Object patchObj = patchAddOperation(
        makePatch(op::PatchOp::PatchAfter, fieldPatch),
        op::PatchOp::EnsureUnion,
        ensureValuePatch);

    auto obj = *applyContainerPatch(patchObj, sourceValue).objectValue_ref();
    EXPECT_TRUE(obj.members()->find(1) == obj.members()->end());
    auto fieldIt = obj.members()->find(2);
    EXPECT_TRUE(fieldIt != obj.members()->end());
    EXPECT_EQ(44, fieldIt->second.as_i32());
  }
  // Ensure member that is already set
  {
    test::testset::union_with<type::i32_t> source;
    source.field_1_ref() = 42;
    auto sourceValue = asValueStruct<type::union_c>(source);

    Value ensureValuePatch;
    Object ensureObject;
    ensureObject.members().ensure()[1] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.objectValue_ref() = ensureObject;

    Object patchObj = makePatch(op::PatchOp::EnsureUnion, ensureValuePatch);

    auto obj = *applyContainerPatch(patchObj, sourceValue).objectValue_ref();
    auto fieldIt = obj.members()->find(1);
    EXPECT_TRUE(obj.members()->find(2) == obj.members()->end());
    EXPECT_TRUE(fieldIt != obj.members()->end());
    EXPECT_EQ(42, fieldIt->second.as_i32());
  }

  // Ensure Fail
  {
    test::testset::union_with<type::i32_t> source;
    source.field_1_ref() = 42;
    auto sourceValue = asValueStruct<type::union_c>(source);

    Object patchObj =
        makePatch(op::PatchOp::EnsureUnion, asValueStruct<type::i32_t>(42));

    EXPECT_THROW(
        applyContainerPatch(patchObj, sourceValue), std::runtime_error);

    // Setting union to two variants at the same time
    Value ensureValuePatch;
    Object ensureObject;
    ensureObject.members().ensure()[1] = asValueStruct<type::i32_t>(43);
    ensureObject.members().ensure()[2] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.objectValue_ref() = ensureObject;
    patchObj = makePatch(op::PatchOp::EnsureUnion, ensureValuePatch);

    EXPECT_THROW(
        applyContainerPatch(patchObj, sourceValue), std::runtime_error);
  }
}

TEST_F(PatchTest, extractMaskViewFromPatchNested) {
  // patch = Patch{"key": Clear,
  //               "key2": Patch{"a": BoolPatch = true,
  //                             "b": Patch{1: BytePatch - 1}}}
  Value fieldPatchValue;
  fieldPatchValue.objectValue_ref() =
      makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
  Value mapPatch;
  mapPatch.mapValue_ref().ensure()[asValueStruct<type::binary_t>("key")] =
      fieldPatchValue;
  Value fieldPatchValue2, bytePatchValue;
  bytePatchValue.objectValue_ref() = convertPatchToObject(op::BytePatch{} - 1);
  fieldPatchValue2.objectValue_ref().ensure().members().ensure()[1] =
      bytePatchValue;
  Value objectPatchValue;
  objectPatchValue.objectValue_ref() =
      makePatch(op::PatchOp::PatchPrior, fieldPatchValue2);
  Value nestedPatchValue, boolPatchValue, fieldPatchValue3;
  boolPatchValue.objectValue_ref() =
      convertPatchToObject(op::BoolPatch{} = true);
  fieldPatchValue3.mapValue_ref().ensure()[asValueStruct<type::binary_t>("a")] =
      boolPatchValue;
  fieldPatchValue3.mapValue_ref().ensure()[asValueStruct<type::binary_t>("b")] =
      objectPatchValue;
  nestedPatchValue.objectValue_ref() =
      makePatch(op::PatchOp::PatchPrior, fieldPatchValue3);
  mapPatch.mapValue_ref().ensure()[asValueStruct<type::binary_t>("key2")] =
      nestedPatchValue;
  auto patchObj = makePatch(op::PatchOp::PatchAfter, mapPatch);

  // readMask = includes_map{"key2": includes_map{"b": includes{1: allMask()}}}
  // writeMask = includes_map{"key": allMask()
  //                         "key2": includes_map{"a": allMask(),
  //                                              "b": includes{1: allMask()}}}
  {
    auto masks = extractMaskViewFromPatch(patchObj);
    auto readMask = masks.read.includes_map_ref().value();
    EXPECT_EQ(readMask.size(), 1);
    for (auto& [key, value] : readMask) {
      EXPECT_TRUE(isBinaryEqual(*((Value*)key), "key2"));
      auto& nestedMask = value.includes_map_ref().value();
      EXPECT_EQ(nestedMask.size(), 1);
      for (auto& [key, value] : nestedMask) {
        isBinaryEqual(*((Value*)key), "b");
        Mask expectedMask;
        expectedMask.includes_ref().emplace()[1] = allMask();
        EXPECT_EQ(value, expectedMask);
      }
    }

    auto writeMask = masks.write.includes_map_ref().value();
    EXPECT_EQ(writeMask.size(), 2);
    for (auto& [key, value] : writeMask) {
      if (isBinaryEqual(*((Value*)key), "key")) {
        EXPECT_EQ(value, allMask());
        continue;
      }
      EXPECT_TRUE(isBinaryEqual(*((Value*)key), "key2"));
      auto& nestedMask = value.includes_map_ref().value();
      EXPECT_EQ(nestedMask.size(), 2);
      for (auto& [key, value] : nestedMask) {
        if (isBinaryEqual(*((Value*)key), "a")) {
          EXPECT_EQ(value, allMask());
          continue;
        }
        isBinaryEqual(*((Value*)key), "b");
        Mask expectedMask;
        expectedMask.includes_ref().emplace()[1] = allMask();
        EXPECT_EQ(value, expectedMask);
      }
    }
  }
  {
    auto masks = extractMaskFromPatch(patchObj);
    auto readMask = masks.read.includes_string_map_ref().value();
    EXPECT_EQ(readMask.size(), 1);
    for (auto& [key, value] : readMask) {
      EXPECT_EQ(key, "key2");
      auto& nestedMask = value.includes_string_map_ref().value();
      EXPECT_EQ(nestedMask.size(), 1);
      for (auto& [key, value] : nestedMask) {
        EXPECT_EQ(key, "b");
        Mask expectedMask;
        expectedMask.includes_ref().emplace()[1] = allMask();
        EXPECT_EQ(value, expectedMask);
      }
    }

    auto writeMask = masks.write.includes_string_map_ref().value();
    EXPECT_EQ(writeMask.size(), 2);
    for (auto& [key, value] : writeMask) {
      if (key == "key") {
        EXPECT_EQ(value, allMask());
        continue;
      }
      EXPECT_EQ(key, "key2");
      auto& nestedMask = value.includes_string_map_ref().value();
      EXPECT_EQ(nestedMask.size(), 2);
      for (auto& [key, value] : nestedMask) {
        if (key == "a") {
          EXPECT_EQ(value, allMask());
          continue;
        }
        EXPECT_EQ(key, "b");
        Mask expectedMask;
        expectedMask.includes_ref().emplace()[1] = allMask();
        EXPECT_EQ(value, expectedMask);
      }
    }
  }
}

TEST_F(PatchTest, extractMaskViewFromPatchEdgeCase) {
  // patch = Patch{1: Put{true}}
  Value boolPatch;
  boolPatch.objectValue_ref() =
      makePatch(op::PatchOp::Put, asValueStruct<type::bool_t>(true));
  Value objPatch;
  objPatch.objectValue_ref().ensure()[FieldId{1}] = boolPatch;
  Object patchObj = makePatch(op::PatchOp::PatchAfter, objPatch);
  // Add noops (this should not make the extractedMask allMask).
  patchObj = patchAddOperation(
      std::move(patchObj),
      op::PatchOp::Clear,
      asValueStruct<type::bool_t>(false));
  patchObj = patchAddOperation(
      std::move(patchObj), op::PatchOp::Add, asValueStruct<type::i32_t>(0));
  patchObj = patchAddOperation(
      std::move(patchObj),
      op::PatchOp::Put,
      asValueStruct<type::set<type::i32_t>>({}));

  Mask mask;
  mask.includes_ref().emplace()[1] = allMask();
  EXPECT_TRUE(checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask));
  EXPECT_TRUE(checkReadWriteMask(extractMaskFromPatch(patchObj), mask));
}

TEST_F(PatchTest, extractMaskViewFromPatchFieldPatch) {
  // Test the case when writeMask is allMask but needs to process FieldPatch.
  // patch = Clear, Patch{1: Put{true}}
  Value boolPatch;
  boolPatch.objectValue_ref() =
      makePatch(op::PatchOp::Put, asValueStruct<type::bool_t>(true));
  Value objPatch;
  objPatch.objectValue_ref().ensure()[FieldId{1}] = boolPatch;
  Object patchObj = makePatch(op::PatchOp::PatchAfter, objPatch);
  patchObj = patchAddOperation(
      std::move(patchObj),
      op::PatchOp::Clear,
      asValueStruct<type::bool_t>(true));

  Mask mask;
  mask.includes_ref().emplace()[1] = allMask();
  EXPECT_TRUE(
      checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask, allMask()));
  EXPECT_TRUE(
      checkReadWriteMask(extractMaskFromPatch(patchObj), mask, allMask()));
}

TEST_F(PatchTest, extractMaskFromPatchInvalidMapMaskKey) {
  test::patch::InvalidMapMaskKeyStructPatch p;
  p.patch<ident::field1>().ensureAndPatchByKey(1) += 1;
  auto obj = convertToObject(p.toThrift());
  EXPECT_THROW(extractMaskFromPatch(obj), std::runtime_error);
}

TEST_F(PatchTest, ApplyPatchToSerializedData) {
  // patch = Patch{1: mapPatch{"key": Put{"foo"}}}
  Value fieldPatchValue;
  fieldPatchValue.objectValue_ref() =
      makePatch(op::PatchOp::Put, asValueStruct<type::binary_t>("foo"));
  Value mapPatchValue;
  mapPatchValue.mapValue_ref().ensure()[asValueStruct<type::binary_t>("key")] =
      fieldPatchValue;
  Value mapPatch;
  mapPatch.objectValue_ref() =
      makePatch(op::PatchOp::PatchAfter, mapPatchValue);
  Value objPatch;
  objPatch.objectValue_ref().ensure()[FieldId{1}] = mapPatch;
  Object patchObj = makePatch(op::PatchOp::PatchAfter, objPatch);

  // obj{1: map{"key": "string",
  //            "foo": "bar"},
  //     2: 2}
  Object obj;
  obj[FieldId{1}] = asValueStruct<type::map<type::binary_t, type::binary_t>>(
      {{"key", "string"}, {"foo", "bar"}});
  obj[FieldId{2}].i32Value_ref() = 2;
  Value value;
  value.objectValue_ref() = obj;

  auto original = protocol::serializeObject<CompactProtocolWriter>(obj);
  auto serialized = applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj, *original);
  Object modifiedObj = parseObject<CompactProtocolReader>(*serialized);
  // Compare with directly applying the patch to entire object.
  applyPatch(patchObj, value);
  EXPECT_EQ(modifiedObj, *value.objectValue_ref());
}

TEST_F(PatchTest, ApplyGeneratedPatchToSerializedData) {
  test::patch::MyStruct original;
  original.boolVal() = true;
  original.byteVal() = 42;
  original.stringVal() = "test";

  test::patch::MyStructPatch patch;
  patch.patchIfSet<ident::boolVal>() = !op::BoolPatch{};
  patch.patchIfSet<ident::byteVal>() = original.byteVal();
  patch.patchIfSet<ident::i16Val>() += 2;
  patch.patchIfSet<ident::i32Val>() += 3;
  patch.patchIfSet<ident::i64Val>() += 4;
  patch.patchIfSet<ident::floatVal>() += 5;
  patch.patchIfSet<ident::doubleVal>() += 6;
  patch.patchIfSet<ident::stringVal>() = "_" + op::StringPatch{} + "_";
  patch.patchIfSet<ident::structVal>().patchIfSet<ident::data1>().append("Na");

  folly::IOBufQueue buffer;
  CompactSerializer::serialize(original, &buffer);
  auto binaryObj = buffer.moveAsValue();
  CompactProtocolReader prot;
  prot.setInput(&binaryObj);
  auto valueObject = detail::parseValue(prot, protocol::T_STRUCT);

  auto patchObject = convertToObject(patch.toThrift());
  auto serialized = applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObject, binaryObj);
  Object modifiedObj = parseObject<CompactProtocolReader>(*serialized);
  // Compare with directly applying the patch to entire object.
  applyPatch(patchObject, valueObject);
  EXPECT_EQ(modifiedObj, *valueObject.objectValue_ref());
}

TEST(Patch, ManuallyConstruct) {
  protocol::Object s;
  s[FieldId{1}].emplace_string() = "hi";

  protocol::Object patch;
  auto& patchPrior =
      patch[static_cast<FieldId>(op::PatchOp::PatchPrior)].emplace_object();
  auto& stringPatch = patchPrior[FieldId{1}].emplace_object();
  stringPatch[static_cast<FieldId>(op::PatchOp::Add)] =
      asValueStruct<type::binary_t>("(");
  stringPatch[static_cast<FieldId>(op::PatchOp::Put)] =
      asValueStruct<type::binary_t>(")");

  protocol::applyPatch(patch, s);

  EXPECT_EQ(s[FieldId{1}].as_string(), "(hi)");
}

TEST_F(PatchTest, PrettyPrintPatch) {
  test::patch::MyStruct original;
  original.boolVal() = true;
  original.byteVal() = 42;
  original.stringVal() = "test";

  test::patch::MyStructPatch patch;
  patch.patchIfSet<ident::stringVal>().append("|");
  patch.patch<ident::stringVal>().append("ITEM");
  EXPECT_EQ(op::prettyPrintPatch(patch), R"(MyStructPatch {
  patchPrior = MyStructFieldPatch {
    stringVal = StringPatch {
      append = "|",
    },
  },
  ensure = MyStructEnsureStruct {
    stringVal = "",
  },
  patch = MyStructFieldPatch {
    stringVal = StringPatch {
      append = "ITEM",
    },
  },
})");
}

} // namespace
} // namespace apache::thrift::protocol
