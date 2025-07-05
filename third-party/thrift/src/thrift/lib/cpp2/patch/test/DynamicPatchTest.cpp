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
#include <thrift/lib/cpp2/protocol/Patch.h>
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
  patch.clear();
  const auto& obj = patch.toObject();
  EXPECT_EQ(obj.size(), 1);
  EXPECT_EQ(obj.at(static_cast<FieldId>(op::PatchOp::Clear)).as_bool(), true);
}

DynamicPatch roundTrip(const DynamicPatch& patch) {
  DynamicPatch ret;
  ret.decode<apache::thrift::CompactProtocolReader>(
      *patch.encode<apache::thrift::CompactProtocolWriter>());
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
    EXPECT_TRUE(patch.isPatchTypeAmbiguous());
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
      const ValueList& src, const ValueList& dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  DynamicSetPatch diffSet(const ValueSet& src, const ValueSet& dst) override {
    cb_(getCurrentPath(), src, dst);
    return {};
  }
  DynamicMapPatch diffMap(const ValueMap& src, const ValueMap& dst) override {
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

        EXPECT_EQ(mask.includes()->size(), 1);
        const Mask& m = mask.includes()->begin()->second;
        switch (FieldId fieldId{mask.includes()->begin()->first}) {
          case FieldId{1}:
            EXPECT_EQ(mask.includes()->at(1), allMask());
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
            } else if (m.includes()->count(21)) {
              EXPECT_EQ(m.includes()->at(21), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 210);
                EXPECT_EQ(to, 211);
              }
              EXPECT_TRUE(cases.insert(4).second);
            } else if (m.includes()->count(22)) {
              EXPECT_EQ(m.includes()->at(22), allMask());
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
              EXPECT_TRUE((std::is_same_v<T, ValueMap>));
              if constexpr (std::is_same_v<T, ValueMap>) {
                EXPECT_EQ(&from, &src[fieldId].as_map());
                EXPECT_EQ(&to, &dst[fieldId].as_map());
              }
              EXPECT_TRUE(cases.insert(6).second);
            } else if (m.includes_string_map()->count("31")) {
              EXPECT_EQ(m.includes_string_map()->at("31"), allMask());
              EXPECT_TRUE((std::is_same_v<T, std::int64_t>));
              if constexpr (std::is_same_v<T, std::int64_t>) {
                EXPECT_EQ(from, 310);
                EXPECT_EQ(to, 311);
              }
              EXPECT_TRUE(cases.insert(7).second);
            } else if (m.includes_string_map()->count("32")) {
              EXPECT_EQ(m.includes_string_map()->at("32"), allMask());
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

  src.s() = "foo";
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(obj[static_cast<FieldId>(op::PatchOp::Clear)].as_bool(), true);

  dst.s() = "foo1";
  obj = testDiffUnion(src, dst).toObject();
  EXPECT_EQ(
      obj[static_cast<FieldId>(op::PatchOp::PatchPrior)]
          .as_object()[FieldId{1}]
          .as_object()[static_cast<FieldId>(op::PatchOp::Put)]
          .as_binary()
          .toString(),
      "1");

  dst.i() = 10;
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
  p.push_back(asValueStruct<type::i32_t>(1));
  p.push_back(asValueStruct<type::i32_t>(2));

  ValueList l;

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

  p.assign({asValueStruct<type::i32_t>(5)});
  p.apply(badge, l);
  EXPECT_EQ(l.size(), 1);
  EXPECT_EQ(l[0].as_i32(), 5);

  p.clear();
  p.apply(badge, l);
  EXPECT_TRUE(l.empty());
}

TEST(DynamicPatch, InvalidListPatch) {
  {
    DynamicListPatch p;
    EXPECT_THROW(
        p.assign(
            {asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)}),
        std::runtime_error);
  }
  {
    DynamicListPatch p;
    p.push_back(asValueStruct<type::i32_t>(1));
    EXPECT_THROW(
        p.push_back(asValueStruct<type::i64_t>(2)), std::runtime_error);
  }
  {
    DynamicListPatch p;
    p.assign(asValueStruct<type::list<type::i32_t>>({1}).as_list());
    EXPECT_THROW(
        p.push_back(asValueStruct<type::i64_t>(2)), std::runtime_error);
  }
}

TEST(DynamicPatch, Set) {
  DynamicSetPatch p;
  p.insert(asValueStruct<type::i32_t>(1));
  p.insert(asValueStruct<type::i32_t>(2));

  ValueSet s;

  p.apply(badge, s);
  EXPECT_EQ(s.size(), 2);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(1)));
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(2)));

  p.apply(badge, s);
  EXPECT_EQ(s.size(), 2);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(1)));
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(2)));

  p.assign({asValueStruct<type::i32_t>(5)});
  p.apply(badge, s);
  EXPECT_EQ(s.size(), 1);
  EXPECT_TRUE(s.contains(asValueStruct<type::i32_t>(5)));

  p.clear();
  p.apply(badge, s);
  EXPECT_TRUE(s.empty());
}

TEST(DynamicPatch, InvalidSetPatch) {
  {
    DynamicSetPatch p;
    EXPECT_THROW(
        p.assign(
            {asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)}),
        std::runtime_error);
  }
  {
    DynamicSetPatch p;
    p.insert(asValueStruct<type::i32_t>(1));
    EXPECT_THROW(p.insert(asValueStruct<type::i64_t>(2)), std::runtime_error);
  }
  {
    DynamicSetPatch p;
    p.assign(asValueStruct<type::set<type::i32_t>>({1}).as_set());
    EXPECT_THROW(p.insert(asValueStruct<type::i64_t>(2)), std::runtime_error);
  }
}

TEST(DynamicPatch, InvalidMapPatch) {
  {
    DynamicMapPatch p;
    EXPECT_THROW(
        p.assign(

            {{asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)},
             {asValueStruct<type::i32_t>(2), asValueStruct<type::i32_t>(1)}}),
        std::runtime_error);
  }
  {
    DynamicMapPatch p;
    EXPECT_THROW(
        p.putMulti(
            {{asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)},
             {asValueStruct<type::i32_t>(2), asValueStruct<type::i32_t>(1)}}),
        std::runtime_error);
  }
  {
    DynamicMapPatch p;
    EXPECT_THROW(
        p.tryPutMulti(
            {{asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)},
             {asValueStruct<type::i32_t>(2), asValueStruct<type::i32_t>(1)}}),
        std::runtime_error);
  }
  auto testInvalidMapPatch = [](auto& p) {
    EXPECT_THROW(
        p.insert_or_assign(
            asValueStruct<type::i64_t>(1), asValueStruct<type::i32_t>(1)),
        std::runtime_error);
    EXPECT_THROW(
        p.insert_or_assign(
            asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)),
        std::runtime_error);
    EXPECT_THROW(
        p.tryPutMulti(
            {{asValueStruct<type::i64_t>(1), asValueStruct<type::i32_t>(1)}}),
        std::runtime_error);
    EXPECT_THROW(
        p.tryPutMulti(
            {{asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)}}),
        std::runtime_error);
    EXPECT_THROW(
        p.putMulti(
            {{asValueStruct<type::i64_t>(1), asValueStruct<type::i32_t>(1)}}),
        std::runtime_error);
    EXPECT_THROW(
        p.putMulti(
            {{asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)}}),
        std::runtime_error);
  };
  {
    DynamicMapPatch p;
    p.insert_or_assign(
        asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1));
    testInvalidMapPatch(p);
  }
  {
    DynamicMapPatch p;
    p.assign({{asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1)}});
    testInvalidMapPatch(p);
  }
  {
    DynamicMapPatch p;
    p.putMulti(
        {{asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1)}});
    testInvalidMapPatch(p);
  }
  {
    DynamicMapPatch p;
    p.tryPutMulti(
        {{asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1)}});
    testInvalidMapPatch(p);
  }
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
    dynamicStringSetPatch.erase(stringFoo());
    dynamicBinarySetPatch.erase(binaryFoo());
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
  ValueMap m;
  m[asValueStruct<type::i32_t>(1)] = asValueStruct<type::i32_t>(10);
  m[asValueStruct<type::i32_t>(2)] = asValueStruct<type::i32_t>(20);

  {
    DynamicMapPatch patch;
    patch.erase(asValueStruct<type::i32_t>(1));
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(20));
  }
  {
    DynamicMapPatch patch;
    op::I32Patch p;
    p += 100;
    patch.patchByKey(asValueStruct<type::i32_t>(1), DynamicPatch{p});
    p += 100;
    patch.patchByKey(asValueStruct<type::i32_t>(2), DynamicPatch{p});
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(220));
  }
  ValueMap add;
  add[asValueStruct<type::i32_t>(2)] = asValueStruct<type::i32_t>(200);
  add[asValueStruct<type::i32_t>(3)] = asValueStruct<type::i32_t>(300);
  {
    DynamicMapPatch patch;
    patch.tryPutMulti(add);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 2);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(220));
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(3)), asValueStruct<type::i32_t>(300));
  }
  {
    DynamicMapPatch patch;
    patch.putMulti(add);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 2);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(200));
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(3)), asValueStruct<type::i32_t>(300));
  }
  {
    ValueSet remove;
    remove.insert(asValueStruct<type::i32_t>(3));
    remove.insert(asValueStruct<type::i32_t>(4));

    DynamicMapPatch patch;
    patch.removeMulti(remove);
    patch.apply(badge, m);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(
        m.at(asValueStruct<type::i32_t>(2)), asValueStruct<type::i32_t>(200));
  }
  {
    DynamicMapPatch patch;
    patch.clear();
    patch.apply(badge, m);
    EXPECT_TRUE(m.empty());
  }
}

TEST(DynamicPatch, TestEmptyPatch) {
  Object obj;
  DynamicPatch patch = DynamicPatch::fromObject(obj);
  EXPECT_TRUE(patch.isPatchTypeAmbiguous());
}

TEST(DynamicPatch, TestClearPatch) {
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Clear)].emplace_bool(true);
  DynamicPatch patch = DynamicPatch::fromObject(obj);
  EXPECT_TRUE(patch.isPatchTypeAmbiguous());
}

template <class Tag, class PatchType = op::patch_type<Tag>>
void testDynamicUnknownPatch(const auto& t) {
  auto v = asValueStruct<Tag>(t);
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Assign)] = v;
  DynamicPatch assignPatch = DynamicPatch::fromObject(obj);
  EXPECT_TRUE(assignPatch.holds_alternative<PatchType>(badge));

  obj = {};
  DynamicPatch emptyPatch = DynamicPatch::fromObject(obj);
  emptyPatch.apply(v);
  EXPECT_EQ(v, asValueStruct<Tag>(t));
  EXPECT_TRUE(emptyPatch.isPatchTypeAmbiguous());
  emptyPatch.getStoredPatchByTag<Tag>();
  EXPECT_FALSE(emptyPatch.isPatchTypeAmbiguous());
  emptyPatch.apply(v);
  EXPECT_EQ(v, asValueStruct<Tag>(t));

  auto checkClearPatch = [&]() {
    if (std::is_base_of_v<type::struct_c, Tag>) {
      // If Tag is a struct, `clear` will remove all fields in protocol::Value,
      // which won't match static patch behavior which only removes optional
      // field.
      EXPECT_TRUE(v.as_object().empty());
    } else {
      EXPECT_EQ(v, asValueStruct<Tag>({}));
    }
  };

  obj[static_cast<FieldId>(op::PatchOp::Clear)].emplace_bool(true);
  DynamicPatch clearPatch = DynamicPatch::fromObject(obj);
  clearPatch.apply(v);
  checkClearPatch();
  EXPECT_TRUE(clearPatch.isPatchTypeAmbiguous());

  v = asValueStruct<Tag>(t);
  clearPatch.getStoredPatchByTag<Tag>();
  EXPECT_FALSE(clearPatch.isPatchTypeAmbiguous());
  clearPatch.apply(v);
  checkClearPatch();
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
  u.s() = "1";
  testDynamicUnknownPatch<type::union_t<MyUnion>, DynamicUnknownPatch>(u);
}

TEST(DynamicPatch, FromAnyPatch) {
  op::AnyPatch anyPatch;
  anyPatch.assign(type::AnyData::toAny<type::union_t<MyUnion>>({}).toThrift());
  DynamicPatch dynPatch = DynamicPatch::fromObject(anyPatch.toObject());
  EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());

  // Assert it is an AnyPatch.
  MyUnionPatch patch;
  patch.patch<ident::s>() = "hello world";
  dynPatch.getStoredPatchByTag<type::struct_t<type::AnyStruct>>().patchIfTypeIs(
      patch);
  auto anyValue = asValueStruct<type::struct_t<type::AnyStruct>>({});
  dynPatch.apply(anyValue);

  auto any = fromValueStruct<type::struct_t<type::AnyStruct>>(anyValue);
  MyUnion u = type::AnyData{any}.get<type::union_t<MyUnion>>();
  EXPECT_TRUE(u.s().has_value());
  EXPECT_EQ(u.s().value(), "hello world");
}

TEST(DynamicPatch, FromSetOrMapPatch) {
  Object obj;
  obj[static_cast<FieldId>(op::PatchOp::Remove)].emplace_set() = {
      asValueStruct<type::i32_t>(1)};
  {
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.isPatchTypeAmbiguous());
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::Add)].emplace_set() = {
        asValueStruct<type::i32_t>(1)};
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicSetPatch>(badge));
  }
  {
    obj.erase(static_cast<FieldId>(op::PatchOp::Add));
    obj[static_cast<FieldId>(op::PatchOp::Put)].emplace_set() = {
        asValueStruct<type::i32_t>(1)};
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicSetPatch>(badge));
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::Put)].emplace_map() = {
        {asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(2)}};
    DynamicPatch patch = DynamicPatch::fromObject(obj);
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
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.isPatchTypeAmbiguous());

    MyUnion u;
    u.i() = 5;
    auto value = asValueStruct<type::union_t<MyUnion>>(u);
    patch.apply(value);
    EXPECT_EQ(value.as_object()[FieldId{2}].as_i32(), 25);

    patch.get<DynamicUnknownPatch>().assign(Object{});
    patch.apply(value);
    EXPECT_TRUE(value.as_object().empty());
  }
  {
    obj[static_cast<FieldId>(op::PatchOp::EnsureUnion)]
        .emplace_object()[FieldId{2}]
        .emplace_i32(1);
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicUnionPatch>(badge));
  }
  {
    obj.erase(static_cast<FieldId>(op::PatchOp::EnsureUnion));
    obj[static_cast<FieldId>(op::PatchOp::EnsureStruct)]
        .emplace_object()[FieldId{1}]
        .emplace_i32(1);
    DynamicPatch patch = DynamicPatch::fromObject(obj);
    EXPECT_TRUE(patch.holds_alternative<DynamicStructPatch>(badge));
  }
}

TEST(PatchMergeTest, DynamicStructPatch) {
  DynamicStructPatch p;

  op::I32Patch foo;
  foo += 1;
  p.patchIfSet<type::i32_t>(FieldId{1}).merge(foo);

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
  p.patchIfSet<type::i32_t>(FieldId{1}).merge(foo);

  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 5);
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 7);

  // In dynamic patch, we only use Remove operation to remove field
  // Clear operation will just set field to intrinsic default
  foo.clear();
  p.patchIfSet<type::i32_t>(FieldId{1}).merge(foo);
  p.apply(badge, obj);
  EXPECT_EQ(obj[FieldId(1)].as_i32(), 0);

  p.remove(FieldId(1));
  p.apply(badge, obj);
  EXPECT_FALSE(obj.contains(FieldId(1)));

  p.ensure(FieldId(1), detail::asValueStruct<type::i32_t>(10));
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
    auto mask2 = mask1.includes().value().at(1);
    if (mask2 == allMask()) {
      return DynamicPatch{diffAny(src, dst)};
    }

    auto mask3 = mask2.includes_type().value().at(type::union_t<MyUnion>());
    EXPECT_EQ(mask3, allMask());
    return DiffVisitorBase::diffStructured(src, dst);
  }
  op::I32Patch diffI32(std::int32_t src, std::int32_t dst) override {
    auto mask1 = getCurrentPath();
    auto mask2 = mask1.includes().value().at(1);
    auto mask3 = mask2.includes_type().value().at(type::union_t<MyUnion>());
    auto mask4 = mask3.includes().value().at(2);
    EXPECT_EQ(mask4, allMask());
    EXPECT_EQ(src, 100);
    EXPECT_EQ(dst, 101);
    op::I32Patch patch;
    patch += 1;
    return patch;
  }
};

TEST(DynamicPatchTest, ToPatchType) {
  EXPECT_EQ(
      toPatchType(type::Type::get<type::struct_t<MyStruct>>()),
      type::Type::get<type::infer_tag<MyStructPatch>>());
  EXPECT_EQ(
      toPatchType(type::Type::get<type::union_t<MyUnion>>()),
      type::Type::get<type::infer_tag<MyUnionPatch>>());
}

TEST(DynamicPatchTest, InvalidToPatchType) {
  type::Type type = type::Type::get<type::union_t<MyUnion>>();
  type.toThrift().name()->unionType()->scopedName() = "scoped.name";
  EXPECT_THROW(toPatchType(type), std::runtime_error);
  EXPECT_THROW(
      toPatchType(type::Type::get<type::infer_tag<MyStructPatch>>()),
      std::runtime_error);
  EXPECT_THROW(
      toPatchType(type::Type::get<type::struct_t<MyStructSafePatch>>()),
      std::runtime_error);
  EXPECT_THROW(toPatchType(type::Type::get<type::i32_t>()), std::runtime_error);
}

TEST(DynamicPatchTest, FromPatchType) {
  EXPECT_EQ(
      type::Type::get<type::struct_t<MyStruct>>(),
      fromPatchType(type::Type::get<type::infer_tag<MyStructPatch>>(), false));
  EXPECT_EQ(
      type::Type::get<type::union_t<MyUnion>>(),
      fromPatchType(type::Type::get<type::infer_tag<MyUnionPatch>>(), true));
  type::Type type = type::Type::get<type::union_t<MyUnion>>();
  type.toThrift().name()->unionType()->scopedName() = "scoped.name";
  EXPECT_THROW(fromPatchType(type, true), std::runtime_error);
  EXPECT_THROW(
      fromPatchType(type::Type::get<type::infer_tag<MyStruct>>(), false),
      std::invalid_argument);
  // mimic if Patch is mistakenly stored as struct in type.
  EXPECT_THROW(
      fromPatchType(type::Type::get<type::infer_tag<MyUnion>>(), false),
      std::runtime_error);
  EXPECT_THROW(
      fromPatchType(type::Type::get<type::i32_t>(), false), std::runtime_error);
}

TEST(DynamicPatchTest, ToSafePatchType) {
  EXPECT_EQ(
      toSafePatchType(type::Type::get<type::struct_t<MyStruct>>()),
      type::Type::get<type::struct_t<MyStructSafePatch>>());
  EXPECT_EQ(
      toSafePatchType(type::Type::get<type::union_t<MyUnion>>()),
      type::Type::get<type::struct_t<MyUnionSafePatch>>());
  EXPECT_THROW(
      toSafePatchType(type::Type::get<type::i32_t>()), std::runtime_error);
  type::Type unionScopedName = type::Type::get<type::union_t<MyUnion>>();
  unionScopedName.toThrift().name()->unionType()->scopedName() = "scoped.name";
  EXPECT_THROW(toSafePatchType(unionScopedName), std::runtime_error);
  EXPECT_THROW(
      toSafePatchType(type::Type::get<type::infer_tag<MyStructPatch>>()),
      std::runtime_error);
  EXPECT_THROW(
      toSafePatchType(type::Type::get<type::struct_t<MyStructSafePatch>>()),
      std::runtime_error);
}

TEST(DynamicPatchTest, FromSafePatchType) {
  EXPECT_EQ(
      type::Type::get<type::struct_t<MyStruct>>(),
      fromSafePatchType(
          type::Type::get<type::struct_t<MyStructSafePatch>>(), false));
  EXPECT_EQ(
      type::Type::get<type::union_t<MyUnion>>(),
      fromSafePatchType(
          type::Type::get<type::struct_t<MyUnionSafePatch>>(), true));
  EXPECT_THROW(
      fromSafePatchType(type::Type::get<type::i32_t>(), false),
      std::runtime_error);
  type::Type unionScopedName = type::Type::get<type::union_t<MyUnion>>();
  unionScopedName.toThrift().name()->unionType()->scopedName() = "scoped.name";
  EXPECT_THROW(fromSafePatchType(unionScopedName, true), std::runtime_error);
  EXPECT_THROW(
      fromSafePatchType(type::Type::get<type::infer_tag<MyStruct>>(), false),
      std::invalid_argument);
  // mimic if SafePatch is mistakenly stored as struct in type.
  EXPECT_THROW(
      fromSafePatchType(type::Type::get<type::infer_tag<MyUnion>>(), false),
      std::runtime_error);
  EXPECT_THROW(
      fromSafePatchType(type::Type::get<type::i32_t>(), false),
      std::runtime_error);
}

TEST(DynamicPatchTest, PatchTypeConversion) {
  // SafePatch -> Patch
  EXPECT_EQ(
      type::Type::get<type::struct_t<MyStructSafePatch>>(),
      toSafePatchType(fromPatchType(
          type::Type::get<type::infer_tag<MyStructPatch>>(), false)));
  EXPECT_EQ(
      type::Type::get<type::struct_t<MyUnionSafePatch>>(),
      toSafePatchType(fromPatchType(
          type::Type::get<type::infer_tag<MyUnionPatch>>(), true)));
  // Patch -> SafePatch
  EXPECT_EQ(
      type::Type::get<type::infer_tag<MyStructPatch>>(),
      toPatchType(fromSafePatchType(
          type::Type::get<type::struct_t<MyStructSafePatch>>(), false)));
  EXPECT_EQ(
      type::Type::get<type::infer_tag<MyUnionPatch>>(),
      toPatchType(fromSafePatchType(
          type::Type::get<type::struct_t<MyUnionSafePatch>>(), true)));
}

TEST(DynamicPatchTest, AnyPatch) {
  StructWithAny src, dst;

  MyUnion srcNested, dstNested;
  srcNested.i() = 100;
  dstNested.i() = 101;

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
  // Check whether the address of the first element is expected.
  // This ensures that we moved the assign data to the patch (not copied).
  void assign(const ValueList& v) {
    EXPECT_EQ(v.size(), 100);
    EXPECT_EQ(&*v.begin(), expected);
  }
  void assign(const ValueSet& v) {
    EXPECT_EQ(v.size(), 100);
    EXPECT_EQ(&*v.begin(), expected);
  }
  void assign(const ValueMap& v) {
    EXPECT_EQ(v.size(), 100);
    EXPECT_EQ(&*v.begin(), expected);
  }
  void assign(const Object& v) {
    EXPECT_EQ(v.size(), 100);
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
  // we moved `t` into p1's assign field
  p1.assign(std::move(t));

  p1.customVisit(checkAssign);

  p2.merge(badge, std::move(p1)); // we moved assign field from p1 to p2
  p2.customVisit(checkAssign);

  DynamicPatch dp{std::move(p2)};
  dp.visitPatch(folly::overload(
      [&](const DynamicListPatch& patch) { patch.customVisit(checkAssign); },
      [&](const DynamicSetPatch& patch) { patch.customVisit(checkAssign); },
      [&](const DynamicMapPatch& patch) { patch.customVisit(checkAssign); },
      [&](const DynamicStructPatch& patch) { patch.customVisit(checkAssign); },
      [&](const auto&) {
        folly::throw_exception<std::runtime_error>("not reachable.");
      }));
  dp.customVisit(badge, checkAssign);
}

TEST(DynamicPatchTest, MergeMovedListPatch) {
  ValueList l;
  for (int i = 0; i < 100; i++) {
    l.emplace_back().emplace_i32(i);
  }
  testMergeMovedPatch<DynamicListPatch>(l);
}

TEST(DynamicPatchTest, MergeMovedSetPatch) {
  ValueSet s;
  for (int i = 0; i < 100; i++) {
    Value v;
    v.emplace_i32(i);
    s.insert(v);
  }
  testMergeMovedPatch<DynamicSetPatch>(s);
}

TEST(DynamicPatchTest, MergeMovedMapPatch) {
  ValueMap m;
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

TEST(DynamicPatch, InvalidGetStoredPatchByTag) {
  {
    MyUnionPatch patch;
    patch.patchIfSet<ident::s>() = "hello world";
    DynamicPatch dynPatch = DynamicPatch::fromObject(patch.toObject());
    EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
    EXPECT_THROW(
        dynPatch.getStoredPatchByTag<type::map_c>(), std::runtime_error);
    EXPECT_THROW(
        dynPatch.getStoredPatchByTag<type::i32_t>(), std::runtime_error);
  }
  {
    DynamicMapPatch patch;
    patch.removeMulti({asValueStruct<type::i32_t>(42)});
    DynamicPatch dynPatch = DynamicPatch::fromObject(patch.toObject());
    EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
    EXPECT_THROW(
        dynPatch.getStoredPatchByTag<type::struct_c>(), std::runtime_error);
    EXPECT_THROW(
        dynPatch.getStoredPatchByTag<type::i32_t>(), std::runtime_error);
  }
}

TEST(DynamicPatch, DynamicSafePatch) {
  MyUnion obj;
  obj.s().emplace("123");

  MyUnionPatch patch;
  patch.patchIfSet<ident::s>() = "hello world";
  MyUnionSafePatch safePatch = patch.toSafePatch();

  // store SafePatch in Thrift Any
  type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

  // round trip
  DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
  EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
  type::AnyStruct rSafePatchAny =
      dynPatch.toSafePatch(type::Type::get<type::union_t<MyUnion>>());

  // apply patch
  MyUnionSafePatch rSafePatch = type::AnyData{std::move(rSafePatchAny)}
                                    .get<type::struct_t<MyUnionSafePatch>>();
  EXPECT_EQ(rSafePatch.version(), 1);
  MyUnionPatch rpatch = MyUnionPatch::fromSafePatch(rSafePatch);
  rpatch.apply(obj);

  EXPECT_EQ(obj.s().value(), "hello world");
}

TEST(DynamicPatch, DynamicSafePatchInvalid) {
  MyUnionSafePatch safePatch;
  safePatch.version() = 42;

  // store SafePatch in Thrift Any
  type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

  EXPECT_THROW(
      (void)DynamicPatch::fromSafePatch(safePatchAny), std::runtime_error);
}

TEST(DynamicPatch, DynamicSafePatchV2) {
  {
    op::AnyPatch anyPatch;
    anyPatch.ensureAny(type::AnyData::toAny(MyUnion{}).toThrift());

    DynamicPatch dynPatch{std::move(anyPatch)};
    type::AnyStruct safePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::union_t<MyUnion>>());

    MyUnionSafePatch safePatch = type::AnyData{std::move(safePatchAny)}
                                     .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(safePatch.version(), 2);
  }
  {
    op::AnyPatch anyPatch;
    // clear is V1 operation
    anyPatch.clear();

    DynamicPatch dynPatch{std::move(anyPatch)};
    type::AnyStruct safePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::union_t<MyUnion>>());

    MyUnionSafePatch safePatch = type::AnyData{std::move(safePatchAny)}
                                     .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(safePatch.version(), 1);
  }
  {
    MyStructPatch patch;
    patch.patchIfSet<ident::any>().ensureAny(
        type::AnyData::toAny(MyUnion{}).toThrift());
    MyStructSafePatch safePatch = patch.toSafePatch();
    type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

    // Round trip
    DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
    type::AnyStruct rSafePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::struct_t<MyStruct>>());
    MyStructSafePatch rSafePatch =
        type::AnyData{std::move(rSafePatchAny)}
            .get<type::struct_t<MyStructSafePatch>>();
    EXPECT_EQ(rSafePatch.version(), 2);
  }
  {
    MyStructPatch patch;
    patch.patchIfSet<ident::any>().ensureAny(
        type::AnyData::toAny(MyUnion{}).toThrift());
    MyStructSafePatch safePatch = patch.toSafePatch();
    type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

    // Round trip
    DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
    type::AnyStruct rSafePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::struct_t<MyStruct>>());
    MyStructSafePatch rSafePatch =
        type::AnyData{std::move(rSafePatchAny)}
            .get<type::struct_t<MyStructSafePatch>>();
    EXPECT_EQ(rSafePatch.version(), 2);
  }
  {
    MyUnionPatch patch;
    patch.patchIfSet<ident::strct>().patchIfSet<ident::any>().ensureAny(
        type::AnyData::toAny(MyUnion{}).toThrift());
    MyUnionSafePatch safePatch = patch.toSafePatch();
    type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

    // round trip
    DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
    EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
    type::AnyStruct rSafePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::union_t<MyUnion>>());

    // apply patch
    MyUnionSafePatch rSafePatch = type::AnyData{std::move(rSafePatchAny)}
                                      .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(rSafePatch.version(), 2);
  }
  {
    MyUnionPatch patch;
    patch.patchIfSet<ident::m>()
        .patchByKey(42)
        .patchIfSet<ident::any>()
        .ensureAny(type::AnyData::toAny(MyUnion{}).toThrift());
    MyUnionSafePatch safePatch = patch.toSafePatch();
    type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

    // Round trip
    DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
    type::AnyStruct rSafePatchAny =
        dynPatch.toSafePatch(type::Type::get<type::union_t<MyUnion>>());
    MyUnionSafePatch rSafePatch = type::AnyData{std::move(rSafePatchAny)}
                                      .get<type::struct_t<MyUnionSafePatch>>();
    EXPECT_EQ(rSafePatch.version(), 2);
  }
}

TEST(DynamicPatch, applyToDataFieldInsideAny) {
  MyUnion obj;
  obj.s().emplace("123");

  MyUnionPatch patch;
  patch.patchIfSet<ident::s>() = "hello world";
  MyUnionSafePatch safePatch = patch.toSafePatch();

  // store obj in Thrift Any
  type::AnyStruct objAny = type::AnyData::toAny(obj).toThrift();

  // store SafePatch in Thrift Any
  type::AnyStruct safePatchAny = type::AnyData::toAny(safePatch).toThrift();

  // apply patch
  DynamicPatch dynPatch = DynamicPatch::fromSafePatch(safePatchAny);
  dynPatch.applyToDataFieldInsideAny(objAny);

  EXPECT_EQ(
      type::AnyData{objAny}.get<type::union_t<MyUnion>>().s().value(),
      "hello world");
}

TEST(DynamicPatchTest, Any) {
  constexpr auto kAssignOp = static_cast<FieldId>(op::PatchOp::Assign);
  constexpr auto kRemoveOp = static_cast<FieldId>(op::PatchOp::Remove);
  constexpr auto kEnsureStructOp =
      static_cast<FieldId>(op::PatchOp::EnsureStruct);
  constexpr auto kPatchPriorOp = static_cast<FieldId>(op::PatchOp::PatchPrior);
  constexpr auto kPatchAfterOp = static_cast<FieldId>(op::PatchOp::PatchAfter);

  MyUnion src, dst;
  src.s() = "123";
  dst.s() = "1234";

  auto any = type::AnyData::toAny(src).toThrift();
  const auto srcObj =
      asValueStruct<type::struct_t<type::AnyStruct>>(any).as_object();

  any = type::AnyData::toAny(dst).toThrift();
  const auto dstObj =
      asValueStruct<type::struct_t<type::AnyStruct>>(any).as_object();

  {
    auto src2 = srcObj;
    auto dst2 = dstObj;

    // If src and dst look like thrift.Any, we only use Assign operator
    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_EQ(dynPatch.size(), 1);
    EXPECT_TRUE(dynPatch.contains(kAssignOp));

    src2[FieldId{3}].emplace_binary(folly::IOBuf::wrapBufferAsValue("123", 3));
    dst2[FieldId{3}].emplace_binary(folly::IOBuf::wrapBufferAsValue("1234", 4));

    // We don't check the content, we only check whether type matches.
    patch = DemoDiffVisitor{}.diff(src2, dst2);
    dynPatch = patch.toObject();
    EXPECT_EQ(dynPatch.size(), 1);
    EXPECT_TRUE(dynPatch.contains(kAssignOp));

    src2[FieldId{5}].emplace_bool();
    dst2[FieldId{5}].emplace_bool();

    // Struct with extra fields still look like Any
    patch = DemoDiffVisitor{}.diff(src2, dst2);
    dynPatch = patch.toObject();
    EXPECT_EQ(dynPatch.size(), 1);
    EXPECT_TRUE(dynPatch.contains(kAssignOp));
  }

  {
    auto src2 = srcObj;
    auto dst2 = dstObj;
    src2[FieldId{1}].emplace_binary();
    dst2[FieldId{1}].emplace_binary();

    // If src does not look like thrift.Any, we can use any operations.
    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kPatchPriorOp));
  }

  {
    auto src2 = srcObj;
    auto dst2 = dstObj;
    src2[FieldId{2}].emplace_binary();
    dst2[FieldId{2}].emplace_binary();

    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kPatchPriorOp));
  }

  {
    auto src2 = srcObj;
    auto dst2 = dstObj;
    src2[FieldId{3}].emplace_object()[FieldId{1}].emplace_i32(123);
    dst2[FieldId{3}].emplace_object()[FieldId{1}].emplace_i32(1234);

    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kPatchPriorOp));
  }

  {
    auto src2 = srcObj;
    const auto& dst2 = dstObj;
    src2.erase(FieldId{3});

    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kEnsureStructOp));
    EXPECT_TRUE(dynPatch.contains(kPatchAfterOp));
  }

  {
    const auto& src2 = srcObj;
    auto dst2 = dstObj;
    dst2.erase(FieldId{3});

    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kRemoveOp));
  }

  {
    auto src2 = srcObj;
    const auto& dst2 = dstObj;
    src2[FieldId{4}].emplace_bool();

    auto patch = DemoDiffVisitor{}.diff(src2, dst2);
    auto dynPatch = patch.toObject();
    EXPECT_TRUE(dynPatch.contains(kPatchPriorOp));
  }
}

struct EnsureAndPatchVisitor {
  void assign(const auto&) { EXPECT_FALSE(true); }
  void clear() { EXPECT_FALSE(true); }
  void patchIfSet(FieldId id, const auto&) { patchIds.insert(id); }
  void ensure(FieldId id, const Value&) { ensureIds.insert(id); }
  void remove(FieldId) { EXPECT_FALSE(true); }

  std::set<FieldId> ensureIds;
  std::set<FieldId> patchIds;
};

TEST(DynamicPatchTest, convertAssignPatchToFieldPatch) {
  protocol::Object obj;
  obj[FieldId{1}].emplace_i32(1);
  obj[FieldId{2}].emplace_i32(2);
  DynamicStructPatch patch;
  patch.ensureAndAssignFieldsFromObject(obj);

  EnsureAndPatchVisitor visitor;
  patch.customVisit(visitor);

  auto check = [](const auto& ids) {
    EXPECT_EQ(ids.size(), 2);
    EXPECT_TRUE(ids.contains(FieldId{1}));
    EXPECT_TRUE(ids.contains(FieldId{2}));
  };

  check(visitor.ensureIds);
  check(visitor.patchIds);
}

} // namespace apache::thrift::protocol
