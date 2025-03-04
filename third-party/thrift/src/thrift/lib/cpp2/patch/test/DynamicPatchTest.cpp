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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/patch/DynamicPatch.h>
#include <thrift/lib/cpp2/patch/detail/PatchBadge.h>
#include <thrift/lib/cpp2/patch/test/gen-cpp2/gen_patch_DynamicPatchTest_types.h>
#include <thrift/lib/cpp2/patch/test/gen-cpp2/gen_patch_OldTerseWrite_types.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

namespace apache::thrift::protocol {
using detail::badge;

class DemoDiffVisitor : public DiffVisitorBase {
 public:
  DynamicPatch diffStructured(const Object& src, const Object& dst) override {
    if (isUnion(getCurrentPath())) {
      return DynamicPatch{DiffVisitorBase::diffUnion(src, dst)};
    }

    if (isAny(getCurrentPath())) {
      return DynamicPatch{DiffVisitorBase::diffAny(src, dst)};
    }

    if (!isFieldEdge(getCurrentPath())) {
      return DiffVisitorBase::diffStructured(src, dst);
    }

    DynamicStructPatch output;
    std::unordered_set<FieldId> ids;

    for (auto&& [id, _] : src) {
      ids.insert(FieldId{id});
    }

    for (auto&& [id, _] : dst) {
      ids.insert(FieldId{id});
    }

    for (auto id : ids) {
      diffField(src, dst, id, output);
    }

    return DynamicPatch{output};
  }

 private:
  bool isUnion(const Mask&) { return false; }

  bool isAny(const Mask&) { return false; }

  bool isFieldEdge(const Mask&) { return false; }
};

TEST(DynamicPatchTest, Demo) {
  EXPECT_TRUE(DemoDiffVisitor{}.diff(Object{}, Object{}).toObject().empty());
}

template <typename T>
struct DynamicPatchesTest : testing::Test {};

using DynamicPatches = ::testing::Types<
    DynamicListPatch,
    DynamicSetPatch,
    DynamicMapPatch,
    DynamicStructPatch,
    DynamicUnionPatch,
    DynamicUnknownPatch>;
TYPED_TEST_SUITE(DynamicPatchesTest, DynamicPatches);

TYPED_TEST(DynamicPatchesTest, Clear) {
  TypeParam patch;
  patch.clear(badge);
  const auto& obj = patch.toObject();
  EXPECT_EQ(obj.size(), 1);
  EXPECT_EQ(obj.at(static_cast<FieldId>(op::PatchOp::Clear)).as_bool(), true);
}

DynamicPatch roundTrip(const DynamicPatch& patch) {
  DynamicPatch ret;
  ret.decode<apache::thrift::CompactProtocolReader>(
      badge, *patch.encode<apache::thrift::CompactProtocolWriter>(badge));
  return ret;
}

template <class Tag, class PatchType, class T = type::native_type<Tag>>
void testOneWay(T src, T dst) {
  Value v1 = asValueStruct<Tag>(src);
  Value v2 = asValueStruct<Tag>(dst);
  auto patch = DiffVisitorBase{}.diff(badge, v1, v2);
  applyPatch(patch.toObject(), v1);
  EXPECT_EQ(v1, v2);
  EXPECT_TRUE(patch.holds_alternative<PatchType>(badge));
  auto other =
      detail::createPatchFromObject<PatchType>(badge, patch.toObject());
  EXPECT_EQ(other.toObject(), patch.toObject());
  EXPECT_EQ(patch.empty(), src == dst);
  auto other2 = roundTrip(patch);
  EXPECT_EQ(other.toObject(), patch.toObject());
  EXPECT_EQ(patch.empty(), src == dst);
}

TEST(DynamicPatchTest, Binary) {
  testOneWay<type::binary_t, op::BinaryPatch>("", "");
  testOneWay<type::binary_t, op::BinaryPatch>("", "1");
  testOneWay<type::binary_t, op::BinaryPatch>("1", "");
  testOneWay<type::binary_t, op::BinaryPatch>("1", "1");
  testOneWay<type::binary_t, op::BinaryPatch>("1", "11");
  testOneWay<type::binary_t, op::BinaryPatch>("11", "1");
  testOneWay<type::binary_t, op::BinaryPatch>("11", "11");
  testOneWay<type::binary_t, op::BinaryPatch>("2", "121");
  testOneWay<type::binary_t, op::BinaryPatch>("2", "12");
  testOneWay<type::binary_t, op::BinaryPatch>("2", "21");
  testOneWay<type::binary_t, op::BinaryPatch>("1", "2");
}

TEST(DynamicPatchTest, List) {
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({}, {});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({}, {1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1}, {});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1}, {1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1}, {1, 1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1, 1}, {1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1, 1}, {1, 1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({2}, {1, 2, 1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({2}, {1, 2});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({2}, {2, 1});
  testOneWay<type::list<type::i32_t>, DynamicListPatch>({1}, {1});
}

TEST(DynamicPatchTest, Set) {
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({}, {});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({}, {1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1}, {});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1}, {1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1}, {1, 1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1, 1}, {1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1, 1}, {1, 1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({2}, {1, 2, 1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({2}, {1, 2});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({2}, {2, 1});
  testOneWay<type::set<type::i32_t>, DynamicSetPatch>({1}, {1});
}

void testMapAndObject(
    std::map<std::int16_t, std::int32_t> src,
    std::map<std::int16_t, std::int32_t> dst) {
  testOneWay<type::map<type::i16_t, type::i32_t>, DynamicMapPatch>(src, dst);

  Object objSrc, objDst;
  for (auto&& [k, v] : src) {
    objSrc[static_cast<FieldId>(k)] = asValueStruct<type::i32_t>(v);
  }
  for (auto&& [k, v] : dst) {
    objDst[static_cast<FieldId>(k)] = asValueStruct<type::i32_t>(v);
  }

  const bool mightBeUnion = objSrc.size() <= 1 && objDst.size() <= 1;

  auto patch = DiffVisitorBase{}.diff(objSrc, objDst);
  applyPatch(patch.toObject(), objSrc);
  EXPECT_EQ(objSrc, objDst);
  EXPECT_EQ(patch.empty(), src == dst);

  if (mightBeUnion) {
    EXPECT_TRUE(patch.holds_alternative<DynamicUnknownPatch>(badge));
    auto other = detail::createPatchFromObject<DynamicUnknownPatch>(
        badge, patch.toObject());
    EXPECT_EQ(other.toObject(), patch.toObject());
  } else {
    EXPECT_TRUE(patch.holds_alternative<DynamicStructPatch>(badge));
  }

  // The generated patch from diff is always a legit struct patch (even though
  // it might actually be an union patch).
  auto other = detail::createPatchFromObject<DynamicStructPatch>(
      badge, patch.toObject());
  EXPECT_EQ(other.toObject(), patch.toObject());
}

TEST(DynamicPatchTest, MapAndObject) {
  testMapAndObject({}, {});
  testMapAndObject({}, {{1, 10}});
  testMapAndObject({{1, 10}}, {});
  testMapAndObject({{1, 10}}, {{1, 10}});
  testMapAndObject({{1, 10}}, {{1, 100}});
  testMapAndObject({{1, 10}, {2, 20}}, {{1, 100}});
  testMapAndObject({{1, 10}, {2, 20}}, {{1, 100}, {2, 20}});
  testMapAndObject({{2, 20}}, {{1, 10}, {2, 20}});
  testMapAndObject({{2, 20}}, {{1, 10}});
}

template <class Callback>
class MaskAndValueCallback : public DiffVisitorBase {
 public:
  explicit MaskAndValueCallback(Callback&& cb) : cb_(std::move(cb)) {}
  op::BoolPatch diffBool(bool src, bool dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::BytePatch diffByte(std::int8_t src, std::int8_t dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::I16Patch diffI16(std::int16_t src, std::int16_t dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::I32Patch diffI32(std::int32_t src, std::int32_t dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::I64Patch diffI64(std::int64_t src, std::int64_t dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::FloatPatch diffFloat(float src, float dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::DoublePatch diffDouble(double src, double dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  op::BinaryPatch diffBinary(
      const folly::IOBuf& src, const folly::IOBuf& dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  DynamicListPatch diffList(
      const detail::ValueList& src, const detail::ValueList& dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  DynamicSetPatch diffSet(
      const detail::ValueSet& src, const detail::ValueSet& dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  DynamicMapPatch diffMap(
      const detail::ValueMap& src, const detail::ValueMap& dst) override {
    cb_(getCurrentPath(), src, dst);
    return DiffVisitorBase::diffMap(src, dst);
  }
  DynamicPatch diffStructured(const Object& src, const Object& dst) override {
    cb_(getCurrentPath(), src, dst);
    return DiffVisitorBase::diffStructured(src, dst);
  }

 private:
  Callback cb_;
};

TEST(DiffVisitorTest, path) {
  /*
  src = Struct {
    1: 10,
    2: Struct {
      21: 210,
      22: 220,
    },
    3: Map {
      "31": 310,
      "32": 320,
    },
  }

  dst = Struct {
    1: 11,
    2: Struct {
      21: 211,
      22: 221,
    },
    3: Map {
      "31": 311,
      "32": 321,
    },
  }
  */
  auto makeObj = [&](int delta) {
    Object obj;

    // Insert field ids. We can't insert it on-the-fly since `Object::member` is
    // F14Map and that will invalidate references.
    *obj.members() = {{1, {}}, {2, {}}, {3, {}}};

    auto& field1 = obj[static_cast<FieldId>(1)].emplace_i64();
    auto& field2 = obj[static_cast<FieldId>(2)].emplace_object();
    auto& field3 = obj[static_cast<FieldId>(3)].emplace_map();
    field1 = 10 + delta;
    field2[static_cast<FieldId>(21)].emplace_i64(210 + delta);
    field2[static_cast<FieldId>(22)].emplace_i64(220 + delta);
    field3[asValueStruct<type::binary_t>("31")].emplace_i64(310 + delta);
    field3[asValueStruct<type::binary_t>("32")].emplace_i64(320 + delta);
    return obj;
  };
  Object src = makeObj(0);
  Object dst = makeObj(1);

  std::set<size_t> cases;

  auto diff = MaskAndValueCallback(
      [&]<class T>(const Mask& mask, const T& from, const T& to) {
        if (mask == allMask()) {
          // The path is empty -- we are on the top level
          EXPECT_TRUE((std::is_same_v<T, Object>));
          if constexpr (std::is_same_v<T, Object>) {
            // Compare address to make sure we did not accidentally make a copy.
            EXPECT_EQ(&from, &src);
            EXPECT_EQ(&to, &dst);
          }

          // Make sure each case happened once and only once
          EXPECT_TRUE(cases.insert(1).second);

          return;
        }

        EXPECT_EQ(mask.includes_ref()->size(), 1);
        const Mask& m = mask.includes_ref()->begin()->second;
        switch (FieldId fieldId{mask.includes_ref()->begin()->first}) {
          case FieldId{1}:
            EXPECT_EQ(mask.includes_ref()->at(1), allMask());
            EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
            if constexpr (std::is_same_v<T, std::int64_t>) {
              EXPECT_EQ(from, 10);
              EXPECT_EQ(to, 11);
              EXPECT_TRUE(cases.insert(2).second);
            }
            break;

          case FieldId{2}:
            if (m == allMask()) {
              EXPECT_TRUE((std::is_same_v<T, Object>));
              if constexpr (std::is_same_v<T, Object>) {
                EXPECT_EQ(&from, &src[fieldId].as_object());
                EXPECT_EQ(&to, &dst[fieldId].as_object());
              }
              EXPECT_TRUE(cases.insert(3).second);
            } else if (m.includes_ref()->count(21)) {
              EXPECT_EQ(m.includes_ref()->at(21), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 210);
                EXPECT_EQ(to, 211);
              }
              EXPECT_TRUE(cases.insert(4).second);
            } else if (m.includes_ref()->count(22)) {
              EXPECT_EQ(m.includes_ref()->at(22), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 220);
                EXPECT_EQ(to, 221);
              }
              EXPECT_TRUE(cases.insert(5).second);
            }
            break;
          case FieldId{3}:
            if (m == allMask()) {
              EXPECT_TRUE((std::is_same_v<T, detail::ValueMap>));
              if constexpr (std::is_same_v<T, detail::ValueMap>) {
                EXPECT_EQ(&from, &src[fieldId].as_map());
                EXPECT_EQ(&to, &dst[fieldId].as_map());
              }
              EXPECT_TRUE(cases.insert(6).second);
            } else if (m.includes_string_map_ref()->count("31")) {
              EXPECT_EQ(m.includes_string_map_ref()->at("31"), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 310);
                EXPECT_EQ(to, 311);
              }
              EXPECT_TRUE(cases.insert(7).second);
            } else if (m.includes_string_map_ref()->count("32")) {
              EXPECT_EQ(m.includes_string_map_ref()->at("32"), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 320);
                EXPECT_EQ(to, 321);
              }
              EXPECT_TRUE(cases.insert(8).second);
            }
            break;

          default:
            EXPECT_TRUE(false);
        }
      });

  (void)diff.diff(src, dst);
  EXPECT_EQ(cases.size(), 8);
}

class UnionDiffVisitor : public DiffVisitorBase {
 public:
  DynamicPatch diffStructured(const Object& src, const Object& dst) override {
    return DynamicPatch{diffUnion(src, dst)};
  }
};

DynamicPatch testDiffUnion(MyUnion src, MyUnion dst) {
  auto v1 = asValueStruct<type::union_c>(src).as_object();
  auto v2 = asValueStruct<type::union_c>(dst).as_object();
  auto patch = UnionDiffVisitor{}.diff(v1, v2);

  applyPatch(patch.toObject(), v1);
  EXPECT_EQ(v1, v2);

  auto u = fromObjectStruct<type::infer_tag<MyUnionPatch>>(patch.toObject());
  u.apply(src);
  EXPECT_EQ(src, dst);

  return patch;
}

TEST(DiffVisitorTest, Union) {
  MyUnion src, dst;
  auto obj = testDiffUnion(src, dst).toObject();
  EXPECT_TRUE(obj.empty());

  src.s_ref() = "foo";
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(obj[static_cast<FieldId>(op::PatchOp::Clear)].as_bool(), true);

  dst.s_ref() = "foo1";
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(
      obj[static_cast<FieldId>(op::PatchOp::PatchPrior)]
          .as_object()[FieldId{1}]
          .as_object()[static_cast<FieldId>(op::PatchOp::Put)]
          .as_binary()
          .toString(),
      "1");

  dst.i_ref() = 10;
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(
      obj[static_cast<FieldId>(op::PatchOp::Assign)],
      asValueStruct<type::struct_c>(dst));

  apache::thrift::clear(src);
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(
      obj[static_cast<FieldId>(op::PatchOp::Assign)],
      asValueStruct<type::struct_c>(dst));
}

TEST(DynamicPatch, List) {
  DynamicListPatch p;
  p.push_back(badge, asValueStruct<type::i32_t>(1));
  p.push_back(badge, asValueStruct<type::i32_t>(2));

  detail::ValueList l;

  p.apply(badge, l);
  EXPECT_EQ(l.size(), 2);
  EXPECT_EQ(l[0].as_i32(), 1);
  EXPECT_EQ(l[1].as_i32(), 2);

  p.apply(badge, l);
  EXPECT_EQ(l.size(), 4);
  EXPECT_EQ(l[0].as_i32(), 1);
  EXPECT_EQ(l[1].as_i32(), 2);
  EXPECT_EQ(l[2].as_i32(), 1);
  EXPECT_EQ(l[3].as_i32(), 2);

  p.assign(badge, {asValueStruct<type::i32_t>(5)});
  p.apply(badge, l);
  EXPECT_EQ(l.size(), 1);
  EXPECT_EQ(l[0].as_i32(), 5);

  p.clear(badge);
  p.apply(badge, l);
  EXPECT_TRUE(l.empty());
}

TEST(DynamicPatch, Set) {
  DynamicSetPatch p;
  p.insert(badge, asValueStruct<type::i32_t>(1));
  p.insert(badge, asValueStruct<type::i32_t>(2));

  detail::ValueSet s;

  p.apply(badge, s);
  EXPECT_EQ(s.size(), 2);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(1)));
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(2)));

  p.apply(badge, s);
  EXPECT_EQ(s.size(), 2);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(1)));
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(2)));

  p.assign(badge, {asValueStruct<type::i32_t>(5)});
  p.apply(badge, s);
  EXPECT_EQ(s.size(), 1);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(5)));

  p.clear(badge);
  p.apply(badge, s);
  EXPECT_TRUE(s.empty());
}

struct StringVsBinaryTest : testing::Test {
  using StringSetPatch = std::remove_cvref_t<
      decltype(op::patch_type<Sets>().patch<ident::stringSet>())>;
  using BinarySetPatch = std::remove_cvref_t<
      decltype(op::patch_type<Sets>().patch<ident::binarySet>())>;

  static Value stringFoo() { return asValueStruct<type::string_t>("foo"); }

  static Value binaryFoo() { return asValueStruct<type::binary_t>("foo"); }

  Sets s;
  StringSetPatch staticStringSetPatch;
  BinarySetPatch staticBinarySetPatch;

  DynamicSetPatch dynamicStringSetPatch, dynamicBinarySetPatch;
  Value stringSetValue, binarySetValue;

  void SetUp() override {
    s.stringSet()->insert("foo");
    s.binarySet()->insert("foo");
    staticStringSetPatch.erase("foo");
    staticBinarySetPatch.erase("foo");
    dynamicStringSetPatch.erase(badge, stringFoo());
    dynamicBinarySetPatch.erase(badge, binaryFoo());
    stringSetValue.emplace_set().insert(stringFoo());
    binarySetValue.emplace_set().insert(binaryFoo());
  }
};

TEST_F(StringVsBinaryTest, Basic) {
  EXPECT_TRUE(stringFoo().if_string());
  EXPECT_TRUE(binaryFoo().if_binary());
  EXPECT_NE(stringFoo(), binaryFoo());
  EXPECT_NE(stringSetValue, binarySetValue);

  EXPECT_EQ(staticStringSetPatch.toObject(), dynamicStringSetPatch.toObject());
  EXPECT_EQ(staticBinarySetPatch.toObject(), dynamicBinarySetPatch.toObject());

  // asValueStruct returns binary if the string is in a struct.
  EXPECT_EQ(
      asValueStruct<type::struct_t<Sets>>(s).as_object()[FieldId(1)],
      binarySetValue);
  EXPECT_EQ(
      asValueStruct<type::struct_t<Sets>>(s).as_object()[FieldId(2)],
      binarySetValue);
}

TEST_F(StringVsBinaryTest, PatchingStringWithBinaryStatic) {
  staticBinarySetPatch.apply(*s.stringSet());
  EXPECT_TRUE(s.stringSet()->empty());
}

TEST_F(StringVsBinaryTest, PatchingStringWithBinaryDynamic) {
  dynamicBinarySetPatch.apply(badge, stringSetValue.as_set());
  EXPECT_FALSE(stringSetValue.as_set().empty());
}

TEST_F(StringVsBinaryTest, PatchingStringWithBinaryApplyPatch) {
  applyPatch(staticBinarySetPatch.toObject(), stringSetValue);
  EXPECT_FALSE(stringSetValue.as_set().empty());
}

TEST_F(StringVsBinaryTest, PatchingBinaryWithStringStatic) {
  staticStringSetPatch.apply(*s.binarySet());
  EXPECT_TRUE(s.binarySet()->empty());
}

TEST_F(StringVsBinaryTest, PatchingBinaryWithStringDynamic) {
  dynamicStringSetPatch.apply(badge, binarySetValue.as_set());
  EXPECT_TRUE(binarySetValue.as_set().empty());
}

TEST_F(StringVsBinaryTest, PatchingBinaryWithStringApplyPatch) {
  applyPatch(staticStringSetPatch.toObject(), binarySetValue);
  EXPECT_TRUE(binarySetValue.as_set().empty());
}

TEST_F(StringVsBinaryTest, PatchingStringWithStringStatic) {
  staticStringSetPatch.apply(*s.stringSet());
  EXPECT_TRUE(s.stringSet()->empty());
}

TEST_F(StringVsBinaryTest, PatchingStringWithStringDynamic) {
  dynamicStringSetPatch.apply(badge, stringSetValue.as_set());
  EXPECT_FALSE(stringSetValue.as_set().empty());
}

TEST_F(StringVsBinaryTest, PatchingStringWithStringApplyPatch) {
  applyPatch(staticStringSetPatch.toObject(), stringSetValue);
  EXPECT_FALSE(stringSetValue.as_set().empty());
}

TEST(DynamicPatch, Map) {
  detail::ValueMap m;
  m[asValueStruct<type::i32_t>(1)] = asValueStruct<type::i32_t>(10);
  m[asValueStruct<type::i32_t>(2)] = asValueStruct<type::i32_t>(20);

  {
    DynamicMapPatch patch;
    patch.erase(badge, asValueStruct<type::i32_t>(1));
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(20));
  }
  {
    DynamicMapPatch patch;
    op::I32Patch p;
    p += 100;
    patch.patchByKey(badge, asValueStruct<type::i32_t>(1), DynamicPatch{p});
    p += 100;
    patch.patchByKey(badge, asValueStruct<type::i32_t>(2), DynamicPatch{p});
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(220));
  }
  detail::ValueMap add;
  add[asValueStruct<type::i32_t>(2)] = asValueStruct<type::i32_t>(200);
  add[asValueStruct<type::i32_t>(3)] = asValueStruct<type::i32_t>(300);
  {
    DynamicMapPatch patch;
    patch.tryPutMulti(badge, add);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 2);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(220));
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(3)), asValueStruct<type::i32_t>(300));
  }
  {
    DynamicMapPatch patch;
    patch.putMulti(badge, add);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 2);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(200));
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(3)), asValueStruct<type::i32_t>(300));
  }
  {
    detail::ValueSet remove;
    remove.insert(asValueStruct<type::i32_t>(3));
    remove.insert(asValueStruct<type::i32_t>(4));

    DynamicMapPatch patch;
    patch.removeMulti(badge, remove);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(200));
  }
  {
    DynamicMapPatch patch;
    patch.clear(badge);
    patch.apply(badge, m);
    EXPECT_TRUE(m.empty());
  }
}

TEST(DynamicPatch, TestEmptyPatch) {
  Object obj;
  DynamicPatch patch;
  patch.fromObject(badge, obj);
  EXPECT_TRUE(patch.holds_alternative<DynamicUnknownPatch>(badge));
}

TEST(DynamicPatch, TestClearPatch) {
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Clear)].emplace_bool(true);
  DynamicPatch patch;
  patch.fromObject(badge, obj);
  EXPECT_TRUE(patch.holds_alternative<DynamicUnknownPatch>(badge));
}

template <class Tag, class PatchType = op::patch_type<Tag>>
void testDynamicUnknownPatch(const auto& t) {
  auto v = asValueStruct<Tag>(t);
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Assign)] = v;
  DynamicPatch assignPatch;
  assignPatch.fromObject(badge, obj);
  EXPECT_TRUE(assignPatch.holds_alternative<PatchType>(badge));

  obj = {};
  DynamicUnknownPatch emptyPatch;
  emptyPatch.fromObject(badge, obj);
  emptyPatch.apply(badge, v);
  EXPECT_EQ(v, asValueStruct<Tag>(t));

  obj[static_cast<FieldId>(op::PatchOp::Clear)].emplace_bool(true);
  DynamicUnknownPatch clearPatch;
  clearPatch.fromObject(badge, obj);
  clearPatch.apply(badge, v);
  if (std::is_base_of_v<type::struct_c, Tag>) {
    // If Tag is a struct, `clear` will remove all fields in protocol::Value,
    // which won't match static patch behavior which only removes optional
    // field.
    EXPECT_TRUE(v.as_object().empty());
  } else {
    EXPECT_EQ(v, asValueStruct<Tag>({}));
  }
}

TEST(DynamicPatch, Unknown) {
  testDynamicUnknownPatch<type::bool_t>(1);
  testDynamicUnknownPatch<type::byte_t>(1);
  testDynamicUnknownPatch<type::i16_t>(1);
  testDynamicUnknownPatch<type::i32_t>(1);
  testDynamicUnknownPatch<type::i64_t>(1);
  testDynamicUnknownPatch<type::float_t>(1);
  testDynamicUnknownPatch<type::double_t>(1);
  testDynamicUnknownPatch<type::string_t, op::StringPatch>("1");
  testDynamicUnknownPatch<type::binary_t, op::BinaryPatch>("1");

  Sets s;
  testDynamicUnknownPatch<type::struct_t<Sets>, DynamicUnknownPatch>(s);
  MyUnion u;
  u.s_ref() = "1";
  testDynamicUnknownPatch<type::union_t<MyUnion>, DynamicUnknownPatch>(u);
}

TEST(DynamicPatch, FromSetOrMapPatch) {
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Remove)].emplace_set() = {
      asValueStruct<type::i32_t>(1)};
  {
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicUnknownPatch>(badge));
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::Add)].emplace_set() = {
        asValueStruct<type::i32_t>(1)};
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicSetPatch>(badge));
  }
  {
    obj.erase(static_cast<FieldId>(op::PatchOp::Add));
    obj[static_cast<FieldId>(op::PatchOp::Put)].emplace_set() = {
        asValueStruct<type::i32_t>(1)};
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicSetPatch>(badge));
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::Put)].emplace_map() = {
        {asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(2)}};
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicMapPatch>(badge));
  }
}

TEST(DynamicPatch, FromStructOrUnionPatch) {
  op::I32Patch fieldPatch;
  fieldPatch += 10;
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::PatchPrior)]
      .emplace_object()[FieldId{2}] =
      asValueStruct<type::infer_tag<op::I32Patch>>(fieldPatch);
  obj[static_cast<FieldId>(op::PatchOp::PatchAfter)]
      .emplace_object()[FieldId{2}] =
      asValueStruct<type::infer_tag<op::I32Patch>>(fieldPatch);
  {
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicUnknownPatch>(badge));

    MyUnion u;
    u.i_ref() = 5;
    auto value = asValueStruct<type::union_t<MyUnion>>(u);
    patch.apply(value);
    EXPECT_EQ(value.as_object()[FieldId{2}].as_i32(), 25);
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::EnsureUnion)]
        .emplace_object()[FieldId{2}]
        .emplace_i32(1);
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicUnionPatch>(badge));
  }
  {
    obj.erase(static_cast<FieldId>(op::PatchOp::EnsureUnion));
    obj[static_cast<FieldId>(op::PatchOp::EnsureStruct)]
        .emplace_object()[FieldId{1}]
        .emplace_i32(1);
    DynamicPatch patch;
    patch.fromObject(badge, obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicStructPatch>(badge));
  }
}

TEST(PatchMergeTest, DynamicStructPatch) {
  DynamicStructPatch p;

  op::I32Patch foo;
  foo += 1;
  p.patchIfSet<type::i32_t>(badge, FieldId{1}).merge(foo);

  Object obj;
  obj[FieldId(1)].emplace_i32(3);
  obj[FieldId(2)].emplace_i32(3);

  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 4);
  EXPECT_EQ(obj[FieldId(2)].as_i32(), 3);

  obj.erase(FieldId(1));
  p.apply(badge, obj);
  EXPECT_FALSE(obj[FieldId(1)].is_i32());

  obj[FieldId(1)].emplace_i32(3);

  // patch becomes += 2
  p.patchIfSet<type::i32_t>(badge, FieldId{1}).merge(foo);

  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 5);
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 7);

  // In dynamic patch, we only use Remove operation to remove field
  // Clear operation will just set field to intrinsic default
  foo.clear();
  p.patchIfSet<type::i32_t>(badge, FieldId{1}).merge(foo);
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 0);

  p.remove(badge, FieldId(1));
  p.apply(badge, obj);
  EXPECT_FALSE(obj.contains(FieldId(1)));

  p.ensure(badge, FieldId(1), detail::asValueStruct<type::i32_t>(10));
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 10);
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 10);
  EXPECT_EQ(obj[FieldId(2)].as_i32(), 3);
}

class AnyDiffVisitor : public DiffVisitorBase {
 public:
  DynamicPatch diffStructured(const Object& src, const Object& dst) override {
    auto mask1 = getCurrentPath();
    if (mask1 == allMask()) {
      return DiffVisitorBase::diffStructured(src, dst);
    }
    auto mask2 = mask1.includes_ref().value().at(1);
    if (mask2 == allMask()) {
      return DynamicPatch{diffAny(src, dst)};
    }

    auto mask3 = mask2.includes_type_ref().value().at(type::union_t<MyUnion>());
    EXPECT_EQ(mask3, allMask());
    return DiffVisitorBase::diffStructured(src, dst);
  }
  op::I32Patch diffI32(std::int32_t src, std::int32_t dst) override {
    auto mask1 = getCurrentPath();
    auto mask2 = mask1.includes_ref().value().at(1);
    auto mask3 = mask2.includes_type_ref().value().at(type::union_t<MyUnion>());
    auto mask4 = mask3.includes_ref().value().at(2);
    EXPECT_EQ(mask4, allMask());
    EXPECT_EQ(src, 100);
    EXPECT_EQ(dst, 101);
    op::I32Patch patch;
    patch += 1;
    return patch;
  }
};

TEST(DynamicPatchTest, AnyPatch) {
  StructWithAny src, dst;

  MyUnion srcNested, dstNested;
  srcNested.i_ref() = 100;
  dstNested.i_ref() = 101;

  src.any() = type::AnyData::toAny(srcNested).toThrift();
  dst.any() = type::AnyData::toAny(dstNested).toThrift();

  auto srcValue = asValueStruct<type::struct_t<StructWithAny>>(src);
  auto dstValue = asValueStruct<type::struct_t<StructWithAny>>(dst);

  auto patch =
      AnyDiffVisitor{}.diff(srcValue.as_object(), dstValue.as_object());

  patch.apply(srcValue);
  EXPECT_EQ(srcValue, dstValue);
}

// We want to test when merging r-value patches, whether we moved the fields
// instead of copying it.
//
// This is done by checking the address of assign field between merged patch and
// the r-value patch we are merging into another patch.
//
// * If we moved the assign field, the address of the first element should be
//   unchanged.
// * If we copied the assign field, the address of the first element should be
//   different.
struct CheckAssign {
  void assign(auto&&) {}
  void assign(detail::Badge, const auto& v) {
    EXPECT_EQ(v.size(), 100);
    // Check whether the address of the first element is expected.
    // This ensures that we moved the assign data to the patch (not copied).
    EXPECT_EQ(&*v.begin(), expected);
  }
  void push_back(auto&&...) {}
  void clear(auto&&...) {}
  void remove(auto&&...) {}
  void removeMulti(auto&&...) {}
  void add(auto&&...) {}
  void tryPutMulti(auto&&...) {}
  void addMulti(auto&&...) {}
  void putMulti(auto&&...) {}
  void patchIfSet(auto&&...) {}
  void patchByKey(auto&&...) {}
  void patchIfTypeIs(auto&&...) {}
  void ensureAny(auto&&...) {}
  void invert() {}
  void prepend(auto&&...) {}
  void append(auto&&...) {}

  void ensure(auto&&...) {}
  const void* expected;
};

template <class Patch, class T>
void testMergeMovedPatch(T t) {
  CHECK_EQ(t.size(), 100);

  CheckAssign checkAssign{&*t.begin()};

  Patch p1, p2;
  p1.assign(badge, std::move(t)); // we moved `t` into p1's assign field
  p1.customVisit(badge, checkAssign);

  p2.merge(badge, std::move(p1)); // we moved assign field from p1 to p2
  p2.customVisit(badge, checkAssign);

  DynamicPatch dp{std::move(p2)};
  dp.visitPatch(
      badge,
      folly::overload(
          [&](const DynamicListPatch& patch) {
            patch.customVisit(badge, checkAssign);
          },
          [&](const DynamicSetPatch& patch) {
            patch.customVisit(badge, checkAssign);
          },
          [&](const DynamicMapPatch& patch) {
            patch.customVisit(badge, checkAssign);
          },
          [&](const DynamicStructPatch& patch) {
            patch.customVisit(badge, checkAssign);
          },
          [&](const auto&) {
            folly::throw_exception<std::runtime_error>("not reachable.");
          }));
  dp.customVisit(badge, checkAssign);
}

TEST(DynamicPatchTest, MergeMovedListPatch) {
  detail::ValueList l;
  for (int i = 0; i < 100; i++) {
    l.emplace_back().emplace_i32(i);
  }
  testMergeMovedPatch<DynamicListPatch>(l);
}

TEST(DynamicPatchTest, MergeMovedSetPatch) {
  detail::ValueSet s;
  for (int i = 0; i < 100; i++) {
    Value v;
    v.emplace_i32(i);
    s.insert(v);
  }
  testMergeMovedPatch<DynamicSetPatch>(s);
}

TEST(DynamicPatchTest, MergeMovedMapPatch) {
  detail::ValueMap m;
  for (int i = 0; i < 100; i++) {
    Value v;
    v.emplace_i32(i);
    m[v].emplace_i32(-i);
  }
  testMergeMovedPatch<DynamicMapPatch>(m);
}

TEST(DynamicPatchTest, MergeMovedStructPatch) {
  Object obj;
  for (int i = 1; i <= 100; i++) {
    obj[static_cast<FieldId>(i)].emplace_i32(-i);
  }
  testMergeMovedPatch<DynamicStructPatch>(obj);
}

TEST(DemoDiffVisitor, TerseWriteFieldMismatch1) {
  using test::Foo;
  Foo src, dst;
  dst.bar() = "123";

  // Field exists when generating the patch but not when applying the diff
  auto srcObj = protocol::asValueStruct<type::struct_t<Foo>>(src).as_object();
  auto dstObj = protocol::asValueStruct<type::struct_t<Foo>>(dst).as_object();

  DemoDiffVisitor visitor;
  auto patch = visitor.diff(srcObj, dstObj);

  auto srcBuf = CompactSerializer::serialize<folly::IOBufQueue>(src).move();
  auto dstBuf = CompactSerializer::serialize<folly::IOBufQueue>(dst).move();

  auto srcVal = protocol::parseValue<CompactProtocolReader>(
      *srcBuf, type::BaseType::Struct);
  auto dstVal = protocol::parseValue<CompactProtocolReader>(
      *dstBuf, type::BaseType::Struct);

  protocol::applyPatch(patch.toObject(), srcVal);

  EXPECT_EQ(srcVal, dstVal);
}

TEST(DemoDiffVisitor, TerseWriteFieldMismatch2) {
  using test::Foo;
  Foo src, dst;
  dst.bar() = "123";

  // Field exists when applying the patch but not when generating the diff
  auto srcBuf = CompactSerializer::serialize<folly::IOBufQueue>(src).move();
  auto dstBuf = CompactSerializer::serialize<folly::IOBufQueue>(dst).move();

  auto srcObj = protocol::parseObject<CompactProtocolReader>(*srcBuf);
  auto dstObj = protocol::parseObject<CompactProtocolReader>(*dstBuf);

  DemoDiffVisitor visitor;
  auto patch = visitor.diff(srcObj, dstObj);

  auto srcVal = protocol::asValueStruct<type::struct_t<Foo>>(src);
  auto dstVal = protocol::asValueStruct<type::struct_t<Foo>>(dst);

  protocol::applyPatch(patch.toObject(), srcVal);

  EXPECT_EQ(srcVal, dstVal);
}

template <class P1, class P2>
void testUnmergeablePatches(P1 p1, P2 p2) {
  {
    DynamicPatch src(p1), dst(p2);
    EXPECT_THROW(dst.merge(src), std::runtime_error);
  }
  {
    DynamicPatch src(p2), dst(p1);
    EXPECT_THROW(dst.merge(src), std::runtime_error);
  }
}

TEST(DynamicPatch, MergingIncompatiblePatch) {
  {
    op::I32Patch p1;
    p1 = 10;
    op::I64Patch p2;
    p2 += 20;
    testUnmergeablePatches(p1, p2);
  }
  {
    test::FooPatch patch;
    patch.patchIfSet<ident::bar>() = "123";

    op::AnyPatch p1;
    p1.patchIfTypeIs(patch);

    DynamicUnknownPatch p2;
    p2.fromObject(badge, patch.toObject());

    // p1 is AnyPatch, p2 is StructPatch. They are not mergeable.
    testUnmergeablePatches(p1, p2);
  }
}

TEST(DynamicPatch, DynamicSafePatch) {
  MyUnion obj;
  obj.s_ref().emplace("123");

  MyUnionPatch patch;
  patch.patchIfSet<ident::s>() = "hello world";
  MyUnionSafePatch safePatch = patch.toSafePatch();

  // store SafePatch in Thrift Any
  type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

  // round trip
  DynamicPatch dynPatch;
  dynPatch.fromSafePatch(safePatchAny);
  type::AnyStruct rSafePatchAny =
      dynPatch.toSafePatch(type::Type::get<type::struct_t<MyUnionSafePatch>>());

  // apply patch
  MyUnionSafePatch rSafePatch = type::AnyData{std::move(rSafePatchAny)}
                                    .get<type::struct_t<MyUnionSafePatch>>();
  EXPECT_EQ(rSafePatch.version(), 1);
  MyUnionPatch rpatch = MyUnionPatch::fromSafePatch(rSafePatch);
  rpatch.apply(obj);

  EXPECT_EQ(obj.s_ref().value(), "hello world");
}

TEST(DynamicPatch, DynamicSafePatchInvalid) {
  MyUnionSafePatch safePatch;
  safePatch.version() = 42;

  // store SafePatch in Thrift Any
  type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

  DynamicPatch dynPatch;
  EXPECT_THROW(dynPatch.fromSafePatch(safePatchAny), std::runtime_error);
}

TEST(DynamicPatch, DynamicSafePatchV2) {
  {
    op::AnyPatch anyPatch;
    anyPatch.ensureAny(type::AnyData::toAny(MyUnion{}).toThrift());

    DynamicPatch dynPatch{std::move(anyPatch)};
    type::AnyStruct safePatchAny = dynPatch.toSafePatch(
        type::Type::get<type::struct_t<MyUnionSafePatch>>());

    MyUnionSafePatch safePatch = type::AnyData{std::move(safePatchAny)}
                                     .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(safePatch.version(), 2);
  }
  {
    op::AnyPatch anyPatch;
    // clear is V1 operation
    anyPatch.clear();

    DynamicPatch dynPatch{std::move(anyPatch)};
    type::AnyStruct safePatchAny = dynPatch.toSafePatch(
        type::Type::get<type::struct_t<MyUnionSafePatch>>());

    MyUnionSafePatch safePatch = type::AnyData{std::move(safePatchAny)}
                                     .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(safePatch.version(), 1);
  }
}

} // namespace apache::thrift::protocol
