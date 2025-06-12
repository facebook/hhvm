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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_layouts.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_types_custom_protocol.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace {
using namespace apache::thrift;
using namespace apache::thrift::frozen;
using namespace apache::thrift::test;
using namespace testing;
using Fixed8 = apache::thrift::frozen::FixedSizeString<8>;
using Fixed2 = apache::thrift::frozen::FixedSizeString<2>;

template <class T>
std::string toString(const T& x) {
  return debugString(x);
}
template <class T>
std::string toString(const Layout<T>& x) {
  std::ostringstream xStr;
  xStr << x;
  return xStr.str();
}

#define EXPECT_PRINTED_EQ(a, b) EXPECT_EQ(toString(a), toString(b))

EveryLayout stressValue2 = [] {
  EveryLayout x;
  *x.aBool() = true;
  *x.aInt() = 2;
  *x.aList() = {3, 5};
  *x.aSet() = {7, 11};
  *x.aHashSet() = {13, 17};
  *x.aMap() = {{19, 23}, {29, 31}};
  *x.aHashMap() = {{37, 41}, {43, 47}};
  x.optInt() = 53;
  *x.aFloat() = 59.61;
  x.optMap() = {{2, 4}, {3, 9}};
  return x;
}();

template <class T>
Layout<T>&& layout(const T& x, Layout<T>&& layout = Layout<T>()) {
  size_t size = LayoutRoot::layout(x, layout);
  (void)size;
  return std::move(layout);
}

template <class T>
Layout<T> layout(const T& x, size_t& size) {
  Layout<T> layout;
  size = LayoutRoot::layout(x, layout);
  return std::move(layout);
}

auto tom1 = [] {
  Pet1 max;
  *max.name() = "max";
  Pet1 ed;
  *ed.name() = "ed";
  Person1 tom;
  *tom.name() = "tom";
  *tom.height() = 1.82f;
  tom.age() = 30;
  tom.pets()->push_back(max);
  tom.pets()->push_back(ed);
  return tom;
}();
auto tom2 = [] {
  Pet2 max;
  *max.name() = "max";
  Pet2 ed;
  *ed.name() = "ed";
  Person2 tom;
  *tom.name() = "tom";
  *tom.weight() = 169;
  tom.age() = 30;
  tom.pets()->push_back(max);
  tom.pets()->push_back(ed);
  return tom;
}();

TEST(Frozen, EndToEnd) {
  auto view = freeze(tom1);
  EXPECT_EQ(*tom1.name(), view.name());
  ASSERT_TRUE(view.name_ref().has_value());
  EXPECT_EQ(*tom1.name(), *view.name_ref());
  ASSERT_TRUE(view.age().has_value());
  EXPECT_EQ(*tom1.age(), view.age().value());
  EXPECT_EQ(*tom1.height(), view.height());
  EXPECT_EQ(view.pets()[0].name(), *tom1.pets()[0].name());
  auto& pets = *tom1.pets();
  auto fpets = view.pets();
  ASSERT_EQ(pets.size(), fpets.size());
  for (size_t i = 0; i < tom1.pets()->size(); ++i) {
    EXPECT_EQ(*pets[i].name(), fpets[i].name());
  }
  Layout<Person1> layout;
  LayoutRoot::layout(tom1, layout);
  auto ttom = view.thaw();
  EXPECT_TRUE(ttom.name().has_value());
  EXPECT_PRINTED_EQ(tom1, ttom);
}

TEST(Frozen, Comparison) {
  EXPECT_EQ(frozenSize(tom1), frozenSize(tom2));
  auto view1 = freeze(tom1);
  auto view2 = freeze(tom2);
  // name is optional now, and was __isset=true, so we don't write it
  ASSERT_TRUE(view2.age().has_value());
  EXPECT_EQ(*tom2.age(), view2.age().value());
  EXPECT_EQ(*tom2.name(), view2.name());
  EXPECT_EQ(tom2.name()->size(), view2.name().size());
  auto ttom2 = view2.thaw();
  EXPECT_EQ(tom2, ttom2) << debugString(tom2) << debugString(ttom2);
}

TEST(Frozen, Compatibility) {
  // Make sure Person1 works well with Person2
  schema::MemorySchema schema;
  Layout<Person1> person1;
  Layout<Person2> person2;

  size_t size = LayoutRoot::layout(tom1, person1);

  saveRoot(person1, schema);
  loadRoot(person2, schema);

  std::string storage(size, 'X');
  folly::MutableStringPiece charRange(&storage.front(), size);
  const folly::MutableByteRange bytes(charRange);
  folly::MutableByteRange freezeRange = bytes;

  ByteRangeFreezer::freeze(person1, tom1, freezeRange);
  auto view12 = person1.view({bytes.begin(), 0});
  auto view21 = person2.view({bytes.begin(), 0});
  EXPECT_EQ(view12.name(), view21.name());
  EXPECT_EQ(view12.age(), view21.age());
  EXPECT_TRUE(view12.height());
  EXPECT_FALSE(view21.weight());
  ASSERT_GE(view12.pets().size(), 2);
  EXPECT_EQ(view12.pets()[0].name(), view21.pets()[0].name());
  EXPECT_EQ(view12.pets()[1].name(), view21.pets()[1].name());
}

// It's important to make sure the hash function not change, other wise the
// existing indexed data will be messed up.
TEST(Frozen, HashCompatibility) {
  // int
  std::hash<int64_t> intHash;
  for (int64_t i = -10000; i < 10000; ++i) {
    EXPECT_EQ(Layout<int64_t>::hash(i), intHash(i));
  }

  // string
  using StrLayout = Layout<std::string>;
  using View = StrLayout::View;

  auto follyHash = [](const View& v) {
    return folly::hash::fnv64_buf_BROKEN(v.begin(), v.size());
  };

  std::vector<std::string> strs{
      "hello", "WOrld", "luckylook", "facebook", "Let it go!!"};
  for (auto&& s : strs) {
    View v(s);
    EXPECT_EQ(StrLayout::hash(v), follyHash(v));
  }
}

TEST(Frozen, EmbeddedSchema) {
  std::string storage;
  {
    schema::Schema schema;
    schema::MemorySchema memSchema;

    Layout<Person1> person1a;

    size_t size;
    size = LayoutRoot::layout(tom1, person1a);
    saveRoot(person1a, memSchema);

    schema::convert(memSchema, schema);

    CompactSerializer::serialize(schema, &storage);
    size_t start = storage.size();
    storage.resize(size + storage.size());

    folly::MutableStringPiece charRange(&storage[start], size);
    folly::MutableByteRange bytes(charRange);
    ByteRangeFreezer::freeze(person1a, tom1, bytes);
  }
  {
    schema::Schema schema;
    schema::MemorySchema memSchema;
    Layout<Person2> person2;

    size_t start = CompactSerializer::deserialize(storage, schema);

    schema::convert(std::move(schema), memSchema);

    loadRoot(person2, memSchema);

    folly::StringPiece charRange(&storage[start], storage.size() - start);
    folly::ByteRange bytes(charRange);
    auto view = person2.view({bytes.begin(), 0});
    EXPECT_EQ(*tom1.name(), view.name());
    ASSERT_EQ(tom1.age().has_value(), view.age().has_value());
    if (auto age = tom1.age()) {
      EXPECT_EQ(*age, view.age().value());
    }
    EXPECT_EQ(*tom1.pets()[0].name(), view.pets()[0].name());
    EXPECT_EQ(*tom1.pets()[1].name(), view.pets()[1].name());
  }
}

TEST(Frozen, NoLayout) {
  ViewPosition null{nullptr, 0};

  EXPECT_FALSE(Layout<bool>().view(null));
  EXPECT_EQ(0, Layout<int>().view(null));
  EXPECT_EQ(0.0f, Layout<float>().view(null));
  EXPECT_EQ(
      apache::thrift::frozen::OptionalFieldView<int>(),
      Layout<folly::Optional<int>>().view(null));
  EXPECT_EQ(std::string(), Layout<std::string>().view(null));
  EXPECT_EQ(std::vector<int>(), Layout<std::vector<int>>().view(null).thaw());
  EXPECT_EQ(Person1(), Layout<Person1>().view(null).thaw());
  EXPECT_EQ(Pet1(), Layout<Pet1>().view(null).thaw());
  EXPECT_EQ(std::set<int>(), Layout<std::set<int>>().view(null).thaw());
  EXPECT_EQ(
      (std::map<int, int>()), (Layout<std::map<int, int>>().view(null).thaw()));

  Layout<Person1> emptyPersonLayout;
  std::array<uint8_t, 100> storage;
  folly::MutableByteRange bytes(storage.begin(), storage.end());
  EXPECT_THROW(
      ByteRangeFreezer::freeze(emptyPersonLayout, tom1, bytes),
      LayoutException);
}

template <class T>
void testMaxLayout(const T& value) {
  auto minLayout = Layout<T>();
  auto valLayout = minLayout;
  auto maxLayout = maximumLayout<T>();
  LayoutRoot::layout(value, valLayout);
  EXPECT_GT(valLayout.size, 0);
  ASSERT_GT(maxLayout.size, 0);
  std::array<uint8_t, 1000> storage;
  folly::MutableByteRange bytes(storage.begin(), storage.end());
  EXPECT_THROW(
      ByteRangeFreezer::freeze(minLayout, value, bytes), LayoutException);
  auto f = ByteRangeFreezer::freeze(maxLayout, value, bytes);
  auto check = f.thaw();
  EXPECT_EQ(value, check);
}

TEST(Frozen, MaxLayoutVector) {
  testMaxLayout(std::vector<int>{99, 24});
}

TEST(Frozen, MaxLayoutPairTree) {
  using std::make_pair;
  auto p1 = make_pair(5, 2.3);
  auto p2 = make_pair(4, p1);
  auto p3 = make_pair(3, p2);
  auto p4 = make_pair(2, p3);
  auto p5 = make_pair(1, p4);
  auto p6 = make_pair(0, p5);
  testMaxLayout(p6);
}

TEST(Frozen, MaxLayoutStress) {
  testMaxLayout(stressValue2);
}

TEST(Frozen, String) {
  std::string str = "Hello";
  auto fstr = freeze(str);
  EXPECT_EQ(str, folly::StringPiece(fstr));
  EXPECT_EQ(std::string(), folly::StringPiece(freeze(std::string())));
}

TEST(Frozen, VectorString) {
  std::vector<std::string> strs{"hello", "sara"};
  auto fstrs = freeze(strs);
  EXPECT_EQ(strs[0], fstrs[0]);
  EXPECT_EQ(strs[1], fstrs[1]);
  EXPECT_EQ(strs.size(), fstrs.size());
  std::vector<std::string> check;
}

TEST(Frozen, BigMap) {
  PlaceTest t;
  for (int i = 0; i < 1000; ++i) {
    auto& place = t.places()[i * i * i % 757368944];
    *place.name() = folly::to<std::string>(i);
    for (int j = 0; j < 200; ++j) {
      ++place.popularityByHour()[rand() % (24 * 7)];
    }
  }
  folly::IOBufQueue bq(folly::IOBufQueue::cacheChainLength());
  CompactSerializer::serialize(t, &bq);
  auto compactSize = bq.chainLength();
  auto frozenSize = ::frozenSize(t);
  EXPECT_EQ(t, freeze(t).thaw());
  EXPECT_LT(frozenSize, compactSize * 0.7);
}
Tiny tiny1 = [] {
  Tiny obj;
  *obj.a() = "just a";
  return obj;
}();
Tiny tiny2 = [] {
  Tiny obj;
  *obj.a() = "two";
  *obj.b() = "set";
  return obj;
}();
Tiny tiny4 = [] {
  Tiny obj;
  *obj.a() = "four";
  *obj.b() = "itty";
  *obj.c() = "bitty";
  *obj.d() = "strings";
  return obj;
}();

TEST(Frozen, Tiny) {
  EXPECT_EQ(tiny4, freeze(tiny4).thaw());
  EXPECT_EQ(24, frozenSize(tiny4));
}

TEST(Frozen, SchemaSaving) {
  // calculate a layout
  Layout<EveryLayout> stressLayoutCalculated;
  CHECK(LayoutRoot::layout(stressValue2, stressLayoutCalculated));

  // save it
  schema::MemorySchema schemaSaved;
  saveRoot(stressLayoutCalculated, schemaSaved);

  // reload it
  Layout<EveryLayout> stressLayoutLoaded;
  loadRoot(stressLayoutLoaded, schemaSaved);

  // make sure the two layouts are identical (via printing)
  EXPECT_PRINTED_EQ(stressLayoutCalculated, stressLayoutLoaded);

  // make sure layouts round-trip
  schema::MemorySchema schemaLoaded;
  saveRoot(stressLayoutLoaded, schemaLoaded);
  EXPECT_EQ(schemaSaved, schemaLoaded);
}

TEST(Frozen, Enum) {
  Person1 he, she;
  *he.gender() = Gender::Male;
  *she.gender() = Gender::Female;
  EXPECT_EQ(he, freeze(he).thaw());
  EXPECT_EQ(she, freeze(she).thaw());
}

TEST(Frozen, EnumAsKey) {
  EnumAsKeyTest thriftObj;
  thriftObj.enumSet()->insert(Gender::Male);
  thriftObj.enumMap()->emplace(Gender::Female, 1219);
  thriftObj.outsideEnumSet()->insert(Animal::DOG);
  thriftObj.outsideEnumMap()->emplace(Animal::CAT, 7779);

  auto frozenObj = freeze(thriftObj);
  EXPECT_THAT(frozenObj.enumSet(), Contains(Gender::Male));
  EXPECT_THAT(frozenObj.outsideEnumSet(), Contains(Animal::DOG));
  EXPECT_EQ(frozenObj.enumMap().at(Gender::Female), 1219);
  EXPECT_EQ(frozenObj.outsideEnumMap().at(Animal::CAT), 7779);
}

template <class T>
size_t frozenBits(const T& value) {
  Layout<T> layout;
  LayoutRoot::layout(value, layout);
  return layout.bits;
}

TEST(Frozen, Bool) {
  Pet1 meat, vegan, dunno;
  meat.vegan() = false;
  vegan.vegan() = true;
  // Always-empty optionals take 0 bits.
  // Sometimes-full optionals take >=1 bits.
  // Always-false bools take 0 bits.
  // Sometimes-true bools take 1 bits.
  // dunno => Nothing => 0 bits.
  // meat => Just(False) => 1 bit.
  // vegan => Just(True) => 2 bits.
  EXPECT_LT(frozenBits(dunno), frozenBits(meat));
  EXPECT_LT(frozenBits(meat), frozenBits(vegan));
  EXPECT_FALSE(*freeze(meat).vegan());
  EXPECT_TRUE(*freeze(vegan).vegan());
  EXPECT_FALSE(freeze(dunno).vegan().has_value());
  EXPECT_EQ(meat, freeze(meat).thaw());
  EXPECT_EQ(vegan, freeze(vegan).thaw());
  EXPECT_EQ(dunno, freeze(dunno).thaw());
}

TEST(Frozen, ThawPart) {
  auto f = freeze(tom1);
  EXPECT_EQ(f.pets()[0].name(), "max");
  EXPECT_EQ(f.pets()[1].name(), "ed");

  auto max = f.pets()[0].thaw();
  auto ed = f.pets()[1].thaw();
  EXPECT_EQ(typeid(max), typeid(Pet1));
  EXPECT_EQ(typeid(ed), typeid(Pet1));
  EXPECT_EQ(*max.name(), "max");
  EXPECT_EQ(*ed.name(), "ed");
}

TEST(Frozen, SchemaConversion) {
  schema::MemorySchema memSchema;
  schema::Schema schema;

  Layout<EveryLayout> stressLayoutCalculated;
  CHECK(LayoutRoot::layout(stressValue2, stressLayoutCalculated));

  schema::MemorySchema schemaSaved;
  saveRoot(stressLayoutCalculated, schemaSaved);

  schema::convert(schemaSaved, schema);
  schema::convert(std::move(schema), memSchema);

  EXPECT_EQ(memSchema, schemaSaved);
}

TEST(Frozen, SparseSchema) {
  {
    auto l = layout(tiny1);
    schema::MemorySchema schema;
    saveRoot(l, schema);
    EXPECT_LE(schema.getLayouts().size(), 4);
  }
  {
    auto l = layout(tiny2);
    schema::MemorySchema schema;
    saveRoot(l, schema);
    EXPECT_LE(schema.getLayouts().size(), 7);
  }
}

TEST(Frozen, DedupedSchema) {
  {
    auto l = layout(tiny4);
    schema::MemorySchema schema;
    saveRoot(l, schema);
    EXPECT_LE(schema.getLayouts().size(), 7); // 13 layouts originally
  }
  {
    auto l = layout(stressValue2);
    schema::MemorySchema schema;
    saveRoot(l, schema);
    EXPECT_LE(schema.getLayouts().size(), 24); // 49 layouts originally
  }
}

TEST(Frozen, TypeHelpers) {
  auto f = freeze(tom1);
  View<Pet1> m = f.pets()[0];
  EXPECT_EQ(m.name(), "max");
}

TEST(Frozen, RangeTrivialRange) {
  auto data = std::vector<float>{3.0, 4.0, 5.0};
  auto view = freeze(data);
  auto r = folly::Range<const float*>(view.range());
  EXPECT_EQ(data, std::vector<float>(r.begin(), r.end()));
}

TEST(Frozen, PaddingLayout) {
  using std::vector;
  // The 'distance' field of the vector<double> is small and sensitive to
  // padding adjustments. If actual distances are returned in
  // layoutBytesDistance instead of worst-case distances, the below structure
  // will successfully freeze at offset zero but fail at later offsets.
  vector<vector<vector<double>>> test(10);
  test.push_back({{1.0}});
  size_t size;
  auto testLayout = layout(test, size);
  for (size_t offset = 0; offset < 8; ++offset) {
    std::unique_ptr<byte[]> store(new byte[size + offset + 16]);
    folly::MutableByteRange bytes(store.get() + offset, size + 16);

    auto view = ByteRangeFreezer::freeze(testLayout, test, bytes);
    auto range = view[10][0].range();
    EXPECT_EQ(range[0], 1.0);
    EXPECT_EQ(reinterpret_cast<intptr_t>(range.begin()) % alignof(double), 0);
  }
}

TEST(Frozen, Bundled) {
  using String = Bundled<std::string>;
  String s("Hello");

  EXPECT_EQ("Hello", s);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(nullptr, s.findFirstOfType<int>());

  s.hold(47);
  s.hold(11);

  EXPECT_EQ(47, *s.findFirstOfType<int>());
  EXPECT_EQ(nullptr, s.findFirstOfType<std::string>());
}

TEST(Frozen, TrivialCopyable) {
  TriviallyCopyableStruct s;
  s.field() = 42;
  auto view = freeze(s);
  EXPECT_EQ(view.field(), 42);
}

MATCHER(PairStrEq, "") {
  *result_listener << std::get<0>(arg).first << ", " << std::get<0>(arg).second
                   << ", vs " << std::get<1>(arg).first << ", "
                   << std::get<1>(arg).second;
  return std::get<0>(arg).first == std::get<1>(arg).first &&
      std::get<0>(arg).second == std::get<1>(arg).second;
}

TEST(Frozen, FixedSizeString) {
  // Good example.
  {
    TestFixedSizeString s;
    s.bytes8() = "01234567";
    // bytes4 field is optional and can be unset.
    auto view = freeze(s);
    ASSERT_TRUE(view.bytes8_ref().has_value());
    EXPECT_EQ(view.bytes8_ref()->toString(), "01234567");
  }
  // Good example.
  {
    TestFixedSizeString s;
    s.bytes8() = "01234567";
    s.bytes4() = "0123";
    auto view = freeze(s);
    ASSERT_TRUE(view.bytes8_ref().has_value());
    EXPECT_EQ(view.bytes8_ref()->toString(), "01234567");
    ASSERT_TRUE(view.bytes4().has_value());
    EXPECT_EQ(view.bytes4()->toString(), "0123");
  }
  // Throws if an unqualified FixedSizeString field is unset.
  {
    TestFixedSizeString s;
    s.bytes4() = "0123";
    // bytes8 field is unqualified and must be set.
    EXPECT_THROW(
        [&s]() { auto view = freeze(s); }(),
        apache::thrift::frozen::detail::FixedSizeMismatchException);
  }
  // Throws if a FixedSizeString field doesn't have the expected size.
  {
    EXPECT_THROW(
        []() {
          TestFixedSizeString s;
          s.bytes8() = "01234567";
          s.bytes4() = "0";
          auto view = freeze(s);
        }(),
        apache::thrift::frozen::detail::FixedSizeMismatchException);
    EXPECT_THROW(
        []() {
          TestFixedSizeString s;
          s.bytes8() = "0";
          s.bytes4() = "0123";
          auto view = freeze(s);
        }(),
        apache::thrift::frozen::detail::FixedSizeMismatchException);
  }
  // Tests FixedSizeString as both the key_type and the value_type in a hashmap.
  {
    TestFixedSizeString s;
    s.bytes8() = "01234567";
    std::unordered_map<std::string, std::string> rawMap = {
        {"76543210", "ab"},
        {"12345670", "cd"},
        {"hellosnw", "ef"},
        {"facebook", "hi"},
    };
    for (const auto& [key, val] : rawMap) {
      s.aMapToFreeze()[apache::thrift::frozen::FixedSizeString<8>(key)] = val;
      s.aMap()[apache::thrift::frozen::FixedSizeString<8>(key)] = val;
    }

    // Tests aMap before serialization.
    {
      EXPECT_THAT(*s.aMap(), testing::UnorderedPointwise(PairStrEq(), rawMap));
      auto iter = s.aMap()->find(Fixed8("hellosnw"));
      ASSERT_NE(iter, s.aMap()->end());
      EXPECT_THAT(
          *iter,
          testing::Pair(
              testing::Eq(Fixed8("hellosnw")), testing::Eq(Fixed2("ef"))));
    }

    auto view = freeze(s);

    auto extractToUnorderedMap = [](const auto& frozenMap) {
      std::unordered_map<std::string, std::string> result;
      for (auto iter = frozenMap.begin(); iter != frozenMap.end(); ++iter) {
        result[iter->first().toString()] = iter->second().toString();
      }
      return result;
    };

    ASSERT_TRUE(view.aMap_ref().has_value());
    EXPECT_THAT(
        extractToUnorderedMap(*view.aMap_ref()),
        testing::UnorderedElementsAreArray(rawMap.begin(), rawMap.end()));
    ASSERT_TRUE(view.aMapToFreeze_ref().has_value());
    EXPECT_THAT(
        extractToUnorderedMap(*view.aMapToFreeze_ref()),
        testing::UnorderedElementsAreArray(rawMap.begin(), rawMap.end()));

    {
      std::string key = "hellosnw";
      auto iter = view.aMap_ref()->find(folly::ByteRange{
          reinterpret_cast<const uint8_t*>(key.data()), key.size()});
      ASSERT_NE(iter, view.aMap_ref()->end());
      EXPECT_EQ(iter->first().toString(), "hellosnw");
      EXPECT_EQ(iter->second().toString(), "ef");
    }
  }
}

TEST(Frozen, Empty) {
  Empty s;
  auto view = freeze(s);
  (void)view;
}

TEST(Frozen, Excluded) {
  ContainsExcluded excludedUnset;
  ContainsExcluded excludedSet;
  excludedSet.excluded().ensure();
  (void)freeze(excludedUnset);
  EXPECT_THROW(freeze(excludedSet), LayoutExcludedException);
}

} // namespace
