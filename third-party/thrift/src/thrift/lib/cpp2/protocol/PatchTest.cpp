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

  template <typename Tag, typename T, typename Patch>
  T applyGeneratedPatch(T value, Patch patch) {
    auto valueObject = asValueStruct<Tag>(value);
    applyPatch(patch.toObject(), valueObject);
    auto buffer = serializeValue<CompactProtocolWriter>(valueObject);

    T patched;
    CompactSerializer::deserialize(buffer.get(), patched);
    return patched;
  }

  template <typename P>
  static Value apply(const P& patchStruct, Value val) {
    Object patchObj = patchStruct.toObject();
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
    EXPECT_TRUE(isMaskNoop(PatchType{}.toObject()));
    EXPECT_TRUE(isMaskNoop((PatchType{} + 0).toObject()));
    EXPECT_TRUE(isMaskNoop((PatchType{} - 0).toObject()));
    EXPECT_TRUE(isMaskWriteOperation((PatchType{} = 43).toObject()));
    EXPECT_TRUE(isMaskReadWriteOperation((PatchType{} + 1).toObject()));
    EXPECT_TRUE(isMaskReadWriteOperation((PatchType{} - 1).toObject()));
  }

  static Object patchAddOperation(Object&& patch, auto operation, auto value) {
    auto opId = static_cast<FieldId>(operation);
    patch[opId] = value;
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
  EXPECT_TRUE(apply(op::BoolPatch{}, true).as_bool());
  EXPECT_FALSE(apply(op::BoolPatch{}, false).as_bool());

  // Assign
  EXPECT_TRUE(apply(op::BoolPatch{} = true, true).as_bool());
  EXPECT_TRUE(apply(op::BoolPatch{} = true, false).as_bool());
  EXPECT_FALSE(apply(op::BoolPatch{} = false, true).as_bool());
  EXPECT_FALSE(apply(op::BoolPatch{} = false, false).as_bool());

  // Invert
  EXPECT_TRUE(apply(!op::BoolPatch{}, false).as_bool());
  EXPECT_FALSE(apply(!op::BoolPatch{}, true).as_bool());

  // Wrong patch provided
  EXPECT_THROW(apply(op::I16Patch{} += 1, true), std::runtime_error);

  // Wrong object to patch
  EXPECT_THROW(
      apply(!op::BoolPatch{}, asValueStruct<type::i16_t>(42)),
      std::runtime_error);

  // Test getting mask from patch
  EXPECT_TRUE(isMaskNoop(op::BoolPatch{}.toObject()));
  EXPECT_TRUE(isMaskWriteOperation((op::BoolPatch{} = true).toObject()));
  EXPECT_TRUE(isMaskWriteOperation((op::BoolPatch{} = false).toObject()));
  EXPECT_TRUE(isMaskReadWriteOperation((!op::BoolPatch{}).toObject()));

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
      toPatch, apply(op::BinaryPatch{}, binaryData).as_binary()));

  // Assign
  EXPECT_TRUE(apply(op::BinaryPatch{} = folly::IOBuf(), binaryData)
                  .as_binary()
                  .empty());
  EXPECT_TRUE(folly::IOBufEqualTo{}(
      patchValue,
      apply(op::BinaryPatch{} = patchValue, binaryData).as_binary()));

  // Append
  {
    op::BinaryPatch binPatch;
    binPatch.append(patchValue);
    std::string appended = data + patch;
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        folly::IOBuf::wrapBufferAsValue(appended.data(), appended.size()),
        apply(binPatch, binaryData).as_binary()));
    EXPECT_TRUE(isMaskReadWriteOperation(binPatch.toObject()));
  }
  {
    op::BinaryPatch binPatch;
    binPatch.append("");
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        toPatch, apply(binPatch, binaryData).as_binary()));
    EXPECT_TRUE(isMaskNoop(binPatch.toObject()));
  }

  // Prepend
  {
    op::BinaryPatch binPatch;
    binPatch.prepend(patch);
    std::string appended = patch + data;
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        folly::IOBuf::wrapBufferAsValue(appended.data(), appended.size()),
        apply(binPatch, binaryData).as_binary()));
    EXPECT_TRUE(isMaskReadWriteOperation(binPatch.toObject()));
  }
  {
    op::BinaryPatch binPatch;
    binPatch.prepend("");
    EXPECT_TRUE(folly::IOBufEqualTo{}(
        toPatch, apply(binPatch, binaryData).as_binary()));
    EXPECT_TRUE(isMaskNoop(binPatch.toObject()));
  }

  // Wrong patch provided
  EXPECT_THROW(apply(op::I16Patch{} = 42, binaryData), std::runtime_error);

  // Wrong object to patch
  EXPECT_THROW(
      apply(op::BinaryPatch{} = patchValue, asValueStruct<type::i16_t>(42)),
      std::runtime_error);

  // Test getting mask from patch
  EXPECT_TRUE(isMaskNoop(op::BinaryPatch{}.toObject()));
  EXPECT_TRUE(
      isMaskWriteOperation((op::BinaryPatch{} = folly::IOBuf()).toObject()));
}

TEST_F(PatchTest, String) {
  std::string data = "test", patch = "best";
  auto stringData = asValueStruct<type::string_t>(data);
  // Noop
  EXPECT_EQ(data, apply(op::StringPatch{}, stringData).as_string());
  EXPECT_TRUE(isMaskNoop(op::StringPatch{}.toObject()));

  // Assign
  EXPECT_EQ(patch, apply(op::StringPatch{} = patch, stringData).as_string());
  EXPECT_TRUE(isMaskWriteOperation((op::StringPatch{} = patch).toObject()));

  // Clear
  {
    op::StringPatch strPatch;
    strPatch.clear();
    EXPECT_TRUE(apply(strPatch, stringData).as_string().empty());
    EXPECT_TRUE(isMaskWriteOperation(strPatch.toObject()));
  }

  // Append
  {
    op::StringPatch strPatch;
    strPatch.append(patch);
    EXPECT_EQ(data + patch, apply(strPatch, stringData).as_string());
    EXPECT_TRUE(isMaskReadWriteOperation(strPatch.toObject()));
  }
  {
    op::StringPatch strPatch;
    strPatch.append("");
    EXPECT_EQ(data, apply(strPatch, stringData).as_string());
    EXPECT_TRUE(isMaskNoop(op::StringPatch{}.toObject()));
  }

  // Prepend
  {
    op::StringPatch strPatch;
    strPatch.prepend(patch);
    EXPECT_EQ(patch + data, apply(strPatch, stringData).as_string());
    EXPECT_TRUE(isMaskReadWriteOperation(strPatch.toObject()));
  }
  {
    op::StringPatch strPatch;
    strPatch.prepend("");
    EXPECT_EQ(data, apply(strPatch, stringData).as_string());
    EXPECT_TRUE(isMaskNoop(op::StringPatch{}.toObject()));
  }

  // Clear, Append and Prepend in one
  {
    op::StringPatch strPatch;
    strPatch.clear();
    strPatch.append(patch);
    strPatch.prepend(patch);
    EXPECT_EQ(patch + patch, apply(strPatch, stringData).as_string());
    EXPECT_TRUE(isMaskReadWriteOperation(strPatch.toObject()));
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
    EXPECT_EQ(value.as_list(), applyContainerPatch(patchObj, value).as_list());
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
        patchValue.as_list(), applyContainerPatch(patchObj, value).as_list());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_EQ(
        std::vector<Value>{}, applyContainerPatch(patchObj, value).as_list());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // Prepend
  {
    auto expected = patchValue.as_list();
    expected.insert(
        expected.end(), value.as_list().begin(), value.as_list().end());
    Object patchObj = makePatch(op::PatchOp::Add, patchValue);
    EXPECT_EQ(expected, applyContainerPatch(patchObj, value).as_list());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Add, emptyValue);
    expectNoop(patchObj);
  }

  // Append
  {
    auto expected = value.as_list();
    expected.insert(
        expected.end(),
        patchValue.as_list().begin(),
        patchValue.as_list().end());
    Object patchObj = makePatch(op::PatchOp::Put, patchValue);
    EXPECT_EQ(expected, applyContainerPatch(patchObj, value).as_list());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Put, emptyValue);
    expectNoop(patchObj);
  }

  // Add
  {
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>(std::set{"test"}));
    EXPECT_EQ(value.as_list(), applyContainerPatch(patchObj, value).as_list())
        << "Should insert nothing";
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    auto expected = value.as_list();
    expected.insert(expected.begin(), asValueStruct<type::binary_t>("best"));
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>(std::set{"best"}));
    EXPECT_EQ(expected, applyContainerPatch(patchObj, value).as_list());
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    Object patchObj = makePatch(op::PatchOp::Add, emptySet);
    expectNoop(patchObj);
  }
}

TEST_F(PatchTest, GeneratedListPatch) {
  ListPatch patch;
  using Vec = ListPatch::value_type;
  Vec actual{5, 6};
  patch.append({7, 8});
  patch.emplace_back(9);
  patch.push_back(10);

  auto patched = applyGeneratedPatch<type::list<type::i16_t>>(actual, patch);
  EXPECT_EQ(patched, (Vec{5, 6, 7, 8, 9, 10}));

  patched = applyGeneratedPatch<type::list<type::i16_t>>(patched, patch);
  EXPECT_EQ(patched, (Vec{5, 6, 7, 8, 9, 10, 7, 8, 9, 10}));
}

TEST_F(PatchTest, Set) {
  auto value = asValueStruct<type::set<type::binary_t>>({"test"});
  auto patchValue = asValueStruct<type::set<type::binary_t>>({"new value"});
  auto emptySet = asValueStruct<type::set<type::binary_t>>({});

  auto expectNoop = [&](auto& patchObj) {
    EXPECT_EQ(value.as_set(), applyContainerPatch(patchObj, value).as_set());
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
        patchValue.as_set(), applyContainerPatch(patchObj, value).as_set());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_TRUE(applyContainerPatch(patchObj, value).as_set().empty());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(false));
    expectNoop(patchObj);
  }

  // Put
  {
    auto expected = value.as_set();
    expected.insert(patchValue.as_set().begin(), patchValue.as_set().end());
    Object patchObj = makePatch(op::PatchOp::Put, patchValue);
    EXPECT_EQ(expected, applyContainerPatch(patchObj, value).as_set());
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
    EXPECT_TRUE(applyContainerPatch(patchObj, value).as_set().empty());
    // Both read and write mask are allMask, as RemovePatch can't be
    // distinguished between set, map, and struct.
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
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
    EXPECT_EQ(value.as_set(), applyContainerPatch(patchObj, value).as_set())
        << "Should insert nothing";
    EXPECT_TRUE(isMaskReadWriteOperation(patchObj));
  }
  {
    auto expected = value.as_set();
    expected.insert(asValueStruct<type::binary_t>("best test"));
    Object patchObj = makePatch(
        op::PatchOp::Add,
        asValueStruct<type::set<type::binary_t>>({"best test"}));
    auto patchResult = applyContainerPatch(patchObj, value).as_set();
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
    EXPECT_EQ(expected.as_set(), applyContainerPatch(patchObj, value).as_set());
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
  auto emptyMap = folly::F14FastMap<Value, Value>{};
  auto emptySet = asValueStruct<type::set<type::binary_t>>({});
  auto emptyValue =
      asValueStruct<type::map<type::binary_t, type::binary_t>>({});

  auto expectNoop = [&](auto&& patchObj) {
    EXPECT_EQ(value.as_map(), applyContainerPatch(patchObj, value).as_map());
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
        patchValue.as_map(), applyContainerPatch(patchObj, value).as_map());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_EQ(emptyMap, applyContainerPatch(patchObj, value).as_map());
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
    EXPECT_EQ(emptyMap, applyContainerPatch(patchObj, value).as_map());
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
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
    EXPECT_EQ(value.as_map(), applyContainerPatch(patchObj, value).as_map())
        << "Should insert nothing";
    checkMapMask(patchObj, {"key"});
  }
  {
    auto expected = value.as_map();
    expected.emplace(
        asValueStruct<type::binary_t>("new key"),
        asValueStruct<type::binary_t>("new value"));
    Object patchObj = makePatch(
        op::PatchOp::EnsureStruct,
        asValueStruct<type::map<type::binary_t, type::binary_t>>(
            {{"new key", "new value"}}));
    auto patchResult = applyContainerPatch(patchObj, value).as_map();
    EXPECT_EQ(expected, patchResult);
    checkMapMask(patchObj, {"new key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::EnsureStruct, emptyValue);
    expectNoop(patchObj);
  }

  // Put
  {
    auto expected = value.as_map();
    expected[asValueStruct<type::binary_t>("key")] =
        asValueStruct<type::binary_t>("key updated value");
    Object patchObj = makePatch(
        op::PatchOp::Put,
        asValueStruct<type::map<type::binary_t, type::binary_t>>(
            {{"key", "key updated value"}}));
    EXPECT_EQ(expected, applyContainerPatch(patchObj, value).as_map());
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

    EXPECT_EQ(expected.as_map(), applyContainerPatch(patchObj, value).as_map());
  }

  // PatchPrior
  {
    auto value =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", {"test"}}});
    auto elementPatchValue = asValueStruct<type::list<type::binary_t>>({"foo"});
    Value fieldPatchValue;
    fieldPatchValue.emplace_object(
        makePatch(op::PatchOp::Put, elementPatchValue));
    Value mapPatch;
    mapPatch.ensure_map()[asValueStruct<type::binary_t>("key")] =
        fieldPatchValue;
    auto patchObj = makePatch(op::PatchOp::PatchPrior, mapPatch);
    auto expected =
        asValueStruct<type::map<type::binary_t, type::list<type::binary_t>>>(
            {{"key", std::vector<std::string>{"test", "foo"}}});
    EXPECT_EQ(expected.as_map(), applyContainerPatch(patchObj, value).as_map());

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
    fieldPatchValue.emplace_object(makePatch(
        op::PatchOp::Put, asValueStruct<type::list<type::binary_t>>({"foo"})));
    Value mapPatch;
    mapPatch.ensure_map()[asValueStruct<type::binary_t>("new key")] =
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
    EXPECT_EQ(expected.as_map(), applyContainerPatch(patchObj, value).as_map());
    checkMapMask(patchObj, {"new key"});
  }
  {
    Object patchObj = makePatch(op::PatchOp::PatchAfter, emptyValue);
    expectNoop(patchObj);
  }
}

TEST_F(PatchTest, EnsureAndPatchObject) {
  // If field 1 doesn't exist, set it to an object that has field_2, whose
  // value is 3.
  Value ensure;
  ensure.ensure_object()[FieldId{1}].ensure_object()[FieldId{2}].ensure_i32() =
      3;

  // Assign 5 to field 1's field 4.
  Value fieldPatch;
  fieldPatch.ensure_object()[FieldId{1}]
      .ensure_object()[FieldId(op::PatchOp::PatchPrior)]
      .ensure_object()[FieldId{4}]
      .ensure_object()[FieldId(op::PatchOp::Assign)]
      .ensure_i32() = 5;

  auto patchObj = patchAddOperation(
      makePatch(op::PatchOp::EnsureStruct, ensure),
      op::PatchOp::PatchAfter,
      fieldPatch);

  // For read, we need to read entire field.
  // For write, we only need to write what we ensured or field patched.
  Mask readMask, writeMask;
  readMask.includes_ref().emplace()[1] = allMask();
  auto& m = writeMask.includes_ref().emplace()[1];
  m.includes_ref().ensure()[2] = allMask();
  m.includes_ref().ensure()[4] = allMask();

  EXPECT_TRUE(checkReadWriteMask(
      extractMaskViewFromPatch(patchObj), readMask, writeMask));
  EXPECT_TRUE(
      checkReadWriteMask(extractMaskFromPatch(patchObj), readMask, writeMask));
}

TEST_F(PatchTest, EnsureAndPatchObject2) {
  // If field 1 doesn't exist, set it to an object that has field_2, whose
  // value is 3.
  Value ensure;
  ensure.ensure_object()[FieldId{1}].ensure_object()[FieldId{2}].ensure_i32() =
      3;

  // Assign empty struct to field 1.
  Value fieldPatch;
  fieldPatch.ensure_object()[FieldId{1}]
      .ensure_object()[FieldId(op::PatchOp::Assign)]
      .ensure_object();

  auto patchObj = patchAddOperation(
      makePatch(op::PatchOp::EnsureStruct, ensure),
      op::PatchOp::PatchAfter,
      fieldPatch);

  // For read, we need to read entire field.
  // For write, we need to write entire field since field patch assign entire
  // field.
  Mask readMask, writeMask;
  readMask.includes_ref().emplace()[1] = allMask();
  writeMask.includes_ref().emplace()[1] = allMask();

  EXPECT_TRUE(checkReadWriteMask(
      extractMaskViewFromPatch(patchObj), readMask, writeMask));
  EXPECT_TRUE(
      checkReadWriteMask(extractMaskFromPatch(patchObj), readMask, writeMask));
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
        value.as_object(), applyContainerPatch(patchObj, value).as_object());
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
        patchValue.as_object(),
        applyContainerPatch(patchObj, value).as_object());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_TRUE(
        applyContainerPatch(patchObj, value).as_object().members()->empty());
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
    fieldPatchValue.emplace_object(
        makePatch(op, asValueStruct<type::list<type::i32_t>>({3, 2, 1})));
    Value fieldPatch;
    fieldPatch.ensure_object()[FieldId{1}] = fieldPatchValue;
    Object patchObj = makePatch(op::PatchOp::PatchPrior, fieldPatch);
    EXPECT_EQ(
        expected,
        applyContainerPatch(patchObj, value)
            .objectValue_ref()
            .ensure()[FieldId{1}]);

    Mask mask;
    mask.includes_ref().emplace()[1] = allMask();
    EXPECT_TRUE(checkReadWriteMask(extractMaskViewFromPatch(patchObj), mask));
    EXPECT_TRUE(checkReadWriteMask(extractMaskFromPatch(patchObj), mask));
  };

  applyFieldPatchTest(
      op::PatchOp::Assign, patchValue.ensure_object()[FieldId{1}]);

  applyFieldPatchTest(
      op::PatchOp::Put,
      asValueStruct<type::list<type::i32_t>>({1, 2, 3, 3, 2, 1}));

  // Ensure and Patch
  {
    test::testset::struct_with<type::list<type::i32_t>> source;
    auto sourceValue = asValueStruct<type::struct_c>(source);

    Value ensureValuePatch;
    Object ensureObject;
    ensureObject[FieldId{1}] = asValueStruct<type::list<type::i32_t>>({});
    ensureValuePatch.emplace_object(ensureObject);

    Value fieldPatchValue;
    fieldPatchValue.emplace_object(makePatch(
        op::PatchOp::Put, asValueStruct<type::list<type::i32_t>>({42})));
    Value fieldPatch;
    fieldPatch.ensure_object()[FieldId{1}] = fieldPatchValue;

    Object patchObj = patchAddOperation(
        makePatch(op::PatchOp::PatchAfter, fieldPatch),
        op::PatchOp::EnsureStruct,
        ensureValuePatch);

    EXPECT_EQ(
        asValueStruct<type::list<type::i32_t>>({42}),
        applyContainerPatch(patchObj, sourceValue)
            .objectValue_ref()
            .ensure()[FieldId{1}]);

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
    fieldPatch.ensure_object();
    expectNoop(makePatch(op::PatchOp::PatchPrior, fieldPatch));
    expectNoop(makePatch(op::PatchOp::EnsureStruct, fieldPatch));
    expectNoop(makePatch(op::PatchOp::PatchAfter, fieldPatch));
  }
  // Remove
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove, asValueStruct<type::list<type::i16_t>>({1}));
    EXPECT_EQ(
        protocol::Object{}, applyContainerPatch(patchObj, value).as_object());
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
  }
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove, asValueStruct<type::list<type::i16_t>>({}));
    expectNoop(patchObj);
  }
  // TODO: Remove this after migrating to List
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove, asValueStruct<type::set<type::i16_t>>({1}));
    EXPECT_EQ(
        protocol::Object{}, applyContainerPatch(patchObj, value).as_object());
    {
      auto masks = extractMaskViewFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
    {
      auto masks = extractMaskFromPatch(patchObj);
      EXPECT_TRUE(MaskRef{masks.read}.isAllMask());
      EXPECT_TRUE(MaskRef{masks.write}.isAllMask());
    }
  }
  {
    Object patchObj = makePatch(
        op::PatchOp::Remove, asValueStruct<type::set<type::i16_t>>({}));
    expectNoop(patchObj);
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
  patch.ensure<ident::option1>("test");

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
  patch.patchIfSet<ident::option1>() = "Hi";
  patch.ensure<ident::option1>("Bye");
  patch.patchIfSet<ident::option1>() += " World!";

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
  patch.patchIfSet<ident::option3>().patchIfSet<ident::option1>() = "World";

  test::patch::MyUnion a, b;
  a.option3_ref().ensure().option1_ref() = "Hello";
  b.option3_ref().ensure().option1_ref() = "World";

  EXPECT_EQ(applyGeneratedPatch<type::union_c>(a, patch), b);
}

TEST_F(PatchTest, Union) { // Should mostly behave like a struct
  test::testset::union_with<type::i32_t> valueObject;
  test::testset::union_with<type::i32_t> patchObject;

  valueObject.field_1_ref() = 42;
  patchObject.field_2_ref() = 43;

  auto value = asValueStruct<type::union_c>(valueObject);
  auto patchValue = asValueStruct<type::union_c>(patchObject);

  auto expectNoop = [&](auto&& patchObj) {
    EXPECT_EQ(
        value.as_object(), applyContainerPatch(patchObj, value).as_object());
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
        patchValue.as_object(),
        applyContainerPatch(patchObj, value).as_object());
    EXPECT_TRUE(isMaskWriteOperation(patchObj));
  }

  // Clear
  {
    Object patchObj =
        makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true));
    EXPECT_TRUE(
        applyContainerPatch(patchObj, value).as_object().members()->empty());
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
    ensureObject[FieldId{2}] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.emplace_object(ensureObject);

    Value fieldPatchValue;
    fieldPatchValue.emplace_object(
        makePatch(op::PatchOp::Add, asValueStruct<type::i32_t>(1)));
    Value fieldPatch;
    fieldPatch.ensure_object()[FieldId{2}] = fieldPatchValue;

    Object patchObj = patchAddOperation(
        makePatch(op::PatchOp::PatchAfter, fieldPatch),
        op::PatchOp::EnsureUnion,
        ensureValuePatch);

    auto obj = applyContainerPatch(patchObj, sourceValue).as_object();
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
    ensureObject[FieldId{1}] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.emplace_object(ensureObject);

    Object patchObj = makePatch(op::PatchOp::EnsureUnion, ensureValuePatch);

    auto obj = applyContainerPatch(patchObj, sourceValue).as_object();
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
    ensureObject[FieldId{1}] = asValueStruct<type::i32_t>(43);
    ensureObject[FieldId{2}] = asValueStruct<type::i32_t>(43);
    ensureValuePatch.emplace_object(ensureObject);
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
  fieldPatchValue.emplace_object(
      makePatch(op::PatchOp::Clear, asValueStruct<type::bool_t>(true)));
  Value mapPatch;
  mapPatch.ensure_map()[asValueStruct<type::binary_t>("key")] = fieldPatchValue;
  Value fieldPatchValue2, bytePatchValue;
  bytePatchValue.emplace_object((op::BytePatch{} - 1).toObject());
  fieldPatchValue2.ensure_object()[FieldId{1}] = bytePatchValue;
  Value objectPatchValue;
  objectPatchValue.emplace_object(
      makePatch(op::PatchOp::PatchPrior, fieldPatchValue2));
  Value nestedPatchValue, boolPatchValue, fieldPatchValue3;
  boolPatchValue.emplace_object((op::BoolPatch{} = true).toObject());
  fieldPatchValue3.ensure_map()[asValueStruct<type::binary_t>("a")] =
      boolPatchValue;
  fieldPatchValue3.ensure_map()[asValueStruct<type::binary_t>("b")] =
      objectPatchValue;
  nestedPatchValue.emplace_object(
      makePatch(op::PatchOp::PatchPrior, fieldPatchValue3));
  mapPatch.ensure_map()[asValueStruct<type::binary_t>("key2")] =
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
  boolPatch.emplace_object(
      makePatch(op::PatchOp::Put, asValueStruct<type::bool_t>(true)));
  Value objPatch;
  objPatch.ensure_object()[FieldId{1}] = boolPatch;
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
  boolPatch.emplace_object(
      makePatch(op::PatchOp::Put, asValueStruct<type::bool_t>(true)));
  Value objPatch;
  objPatch.ensure_object()[FieldId{1}] = boolPatch;
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
  auto obj = p.toObject();
  EXPECT_THROW(extractMaskFromPatch(obj), std::runtime_error);
}

TEST_F(PatchTest, ApplyPatchToSerializedData) {
  // patch = Patch{1: mapPatch{"key": Put{"foo"}}}
  Value fieldPatchValue;
  fieldPatchValue.emplace_object(
      makePatch(op::PatchOp::Put, asValueStruct<type::binary_t>("foo")));
  Value mapPatchValue;
  mapPatchValue.ensure_map()[asValueStruct<type::binary_t>("key")] =
      fieldPatchValue;
  Value mapPatch;
  mapPatch.emplace_object(makePatch(op::PatchOp::PatchAfter, mapPatchValue));
  Value objPatch;
  objPatch.ensure_object()[FieldId{1}] = mapPatch;
  Object patchObj = makePatch(op::PatchOp::PatchAfter, objPatch);

  // obj{1: map{"key": "string",
  //            "foo": "bar"},
  //     2: 2}
  Object obj;
  obj[FieldId{1}] = asValueStruct<type::map<type::binary_t, type::binary_t>>(
      {{"key", "string"}, {"foo", "bar"}});
  obj[FieldId{2}].emplace_i32(2);
  Value value;
  value.emplace_object(obj);

  auto original = protocol::serializeObject<CompactProtocolWriter>(obj);
  auto serialized = applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj, *original);
  Object modifiedObj = parseObject<CompactProtocolReader>(*serialized);
  // Compare with directly applying the patch to entire object.
  applyPatch(patchObj, value);
  EXPECT_EQ(modifiedObj, value.as_object());
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

  auto patchObject = patch.toObject();
  auto serialized = applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObject, binaryObj);
  Object modifiedObj = parseObject<CompactProtocolReader>(*serialized);
  // Compare with directly applying the patch to entire object.
  applyPatch(patchObject, valueObject);
  EXPECT_EQ(modifiedObj, valueObject.as_object());
}

TEST(Patch, ManuallyConstruct) {
  protocol::Object s;
  s[FieldId{1}].ensure_string() = "hi";

  protocol::Object patch;
  auto& patchPrior =
      patch[static_cast<FieldId>(op::PatchOp::PatchPrior)].ensure_object();
  auto& stringPatch = patchPrior[FieldId{1}].ensure_object();
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
  ensure = MyStructEnsureStruct {
    stringVal = "",
  },
  patch = MyStructFieldPatch {
    stringVal = StringPatch {
      append = "|ITEM",
    },
  },
})");
}

TEST_F(PatchTest, RemoveField) {
  Object obj;
  obj[FieldId{1}].ensure_i32() = 10;
  obj[FieldId{2}].ensure_i32() = 20;
  obj[FieldId{3}].ensure_i32() = 30;

  Object patch;
  patch[static_cast<FieldId>(op::PatchOp::Remove)].ensure_set() = {};
  applyPatch(patch, obj);
  EXPECT_TRUE(obj.contains(FieldId{1}));
  EXPECT_TRUE(obj.contains(FieldId{2}));
  EXPECT_TRUE(obj.contains(FieldId{3}));

  patch[static_cast<FieldId>(op::PatchOp::Remove)].ensure_set() = {
      asValueStruct<type::i16_t>(1), asValueStruct<type::i16_t>(2)};
  applyPatch(patch, obj);
  EXPECT_FALSE(obj.contains(FieldId{1}));
  EXPECT_FALSE(obj.contains(FieldId{2}));
  EXPECT_TRUE(obj.contains(FieldId{3}));

  patch[static_cast<FieldId>(op::PatchOp::Remove)].ensure_set() = {
      asValueStruct<type::i16_t>(3)};
  applyPatch(patch, obj);
  EXPECT_FALSE(obj.contains(FieldId{1}));
  EXPECT_FALSE(obj.contains(FieldId{2}));
  EXPECT_FALSE(obj.contains(FieldId{3}));

  patch[static_cast<FieldId>(op::PatchOp::Remove)].ensure_set() = {
      asValueStruct<type::i32_t>(1)};
  try {
    applyPatch(patch, obj);
    EXPECT_TRUE(false);
  } catch (std::runtime_error& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("is not `set<i16>` but `set<i32Value>`"), msg.npos)
        << msg;
  }
}

} // namespace
} // namespace apache::thrift::protocol
