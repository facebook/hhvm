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

#include <thrift/lib/cpp2/frozen/FrozenTestUtil.h>
#include <thrift/lib/cpp2/frozen/VectorAssociative.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_layouts.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_types_custom_protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace apache::thrift;
using namespace apache::thrift::frozen;
using namespace apache::thrift::test;

TEST(FrozenVectorTypes, VectorAsMap) {
  VectorAsMap<int, int> dm;
  dm.insert({9, 81});
  dm.insert({5, 25});
  dm.insert({3, 9});
  dm.insert({7, 49});
  dm.insert(dm.end(), {1, 1});
  // ensure it gets sorted
  auto fdm = freeze(dm);
  EXPECT_EQ(1, fdm.at(1));
  EXPECT_EQ(81, fdm.at(9));
  EXPECT_EQ(25, fdm.at(5));
  {
    auto found = fdm.find(3);
    ASSERT_NE(found, fdm.end());
    EXPECT_EQ(found->second(), 9);
  }
  {
    auto found = fdm.find(2);
    EXPECT_EQ(found, fdm.end());
  }
}

TEST(FrozenVectorTypes, VectorAsHashMap) {
  VectorAsHashMap<int, int> dm;
  dm.insert({1, 2});
  dm.insert(dm.end(), {3, 4});
  auto fdm = freeze(dm);
  EXPECT_EQ(2, fdm.at(1));
  EXPECT_EQ(4, fdm.at(3));
  {
    auto found = fdm.find(3);
    ASSERT_NE(found, fdm.end());
    EXPECT_EQ(found->second(), 4);
  }
  {
    auto found = fdm.find(2);
    EXPECT_EQ(found, fdm.end());
  }
}

TEST(FrozenVectorTypes, OptionalVectorAsHashMap) {
  folly::Optional<VectorAsHashMap<int, int>> dm;
  dm.emplace();
  dm->insert({1, 2});
  dm->insert(dm->end(), {3, 4});
  auto fdm = freeze(dm);
  EXPECT_EQ(2, fdm->at(1));
  EXPECT_EQ(4, fdm->at(3));
  {
    auto found = fdm->find(3);
    ASSERT_NE(found, fdm->end());
    EXPECT_EQ(found->second(), 4);
  }
  {
    auto found = fdm.value().find(3);
    ASSERT_NE(found, fdm.value().end());
    EXPECT_EQ(found->second(), 4);
  }
  {
    auto found = fdm->find(2);
    EXPECT_EQ(found, fdm->end());
  }
}

TEST(FrozenVectorTypes, VectorAsSet) {
  VectorAsSet<int> dm;
  dm.insert(3);
  dm.insert(dm.end(), 7);
  auto fdm = freeze(dm);
  EXPECT_EQ(1, fdm.count(3));
  EXPECT_EQ(1, fdm.count(7));
  EXPECT_EQ(0, fdm.count(4));
}

TEST(FrozenVectorTypes, VectorBoolAsSet) {
  // This is a silly use case, but vector<bool>::reference is not a
  // reference which is an interesting case to support.
  VectorAsSet<bool> dm;
  dm.insert(true);
  auto fdm = freeze(dm);
  EXPECT_EQ(1, fdm.count(true));
  EXPECT_EQ(0, fdm.count(false));
}

TEST(FrozenVectorTypes, VectorAsHashSet) {
  VectorAsHashSet<int> dm;
  dm.insert(3);
  dm.insert(dm.end(), 7);
  auto fdm = freeze(dm);
  EXPECT_EQ(1, fdm.count(3));
  EXPECT_EQ(1, fdm.count(7));
  EXPECT_EQ(0, fdm.count(4));
}

TEST(FrozenVectorTypes, DistinctChecking) {
  VectorAsHashMap<int, int> hm{{1, 2}, {1, 3}};
  VectorAsHashSet<int> hs{4, 4};
  VectorAsMap<int, int> om{{5, 6}, {5, 7}};
  VectorAsSet<int> os{8, 8};
  EXPECT_THROW(freeze(hm), std::domain_error);
  EXPECT_THROW(freeze(hs), std::domain_error);
  EXPECT_THROW(freeze(om), std::domain_error);
  EXPECT_THROW(freeze(os), std::domain_error);
}

TEST(FrozenVectorTypes, DistinctCheckingShouldPass) {
  VectorAsHashMap<int, int> hm{{1, 2}, {2, 3}};
  VectorAsHashSet<int> hs{4, 5};
  VectorAsMap<int, int> om{{5, 6}, {6, 7}};
  VectorAsSet<int> os{8, 9};
  auto fhm = freeze(hm);
  auto fhs = freeze(hs);
  auto fom = freeze(om);
  auto fos = freeze(os);
  EXPECT_EQ(2, fhm.size());
  EXPECT_EQ(2, fhs.size());
  EXPECT_EQ(2, fom.size());
  EXPECT_EQ(2, fos.size());
}

template <class TestType>
void populate(TestType& x) {
  x.aList()->push_back(1);
  x.aSet()->insert(2);
  x.aMap()[3] = 4;
  x.aHashSet()->insert(5);
  x.aHashMap()[6] = 7;
  x.fbVector()->push_back(8);
}

template <class T>
class FrozenStructsWithVectors : public ::testing::Test {};
TYPED_TEST_CASE_P(FrozenStructsWithVectors);

TYPED_TEST_P(FrozenStructsWithVectors, Serializable) {
  TypeParam input;
  populate(input);
  auto serialized = CompactSerializer::serialize<std::string>(input);
  auto output = CompactSerializer::deserialize<TypeParam>(serialized);
  EXPECT_EQ(input, output);
}

TYPED_TEST_P(FrozenStructsWithVectors, Freezable) {
  TypeParam input;
  populate(input);
  auto f = freeze(input);
  EXPECT_EQ(f.aList()[0], 1);
  EXPECT_EQ(f.aSet().count(1), 0);
  EXPECT_EQ(f.aSet().count(2), 1);
  EXPECT_EQ(f.aMap().getDefault(3, 9), 4);
  EXPECT_EQ(f.aMap().getDefault(4, 9), 9);
  EXPECT_EQ(f.aHashSet().count(5), 1);
  EXPECT_EQ(f.aHashSet().count(6), 0);
  EXPECT_EQ(f.aHashMap().getDefault(6, 9), 7);
  EXPECT_EQ(f.aHashMap().getDefault(7, 9), 9);
  EXPECT_EQ(f.fbVector()[0], 8);
}

REGISTER_TYPED_TEST_CASE_P(FrozenStructsWithVectors, Freezable, Serializable);
using MyTypes = ::testing::Types<VectorTest>;
INSTANTIATE_TYPED_TEST_CASE_P(CppVerions, FrozenStructsWithVectors, MyTypes);
