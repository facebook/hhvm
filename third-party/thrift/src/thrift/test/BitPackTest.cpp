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

#include <random>
#include <gtest/gtest.h>
#include <thrift/test/gen-cpp2/Bitpack_for_each_field.h>
#include <thrift/test/gen-cpp2/Bitpack_types.h>

namespace apache::thrift::test {

std::mt19937 rng;

template <typename Type>
Type makeStructExtra() {
  auto obj = Type();
  obj.extraInt32Def_ref() = 4000;
  obj.extraInt32Req_ref() = 5000;
  obj.extraInt32Opt_ref() = 6000;
  return obj;
}

template <typename Type>
Type makeStructBasic() {
  auto obj = Type();
  obj.int32Req_ref() = 2000;
  obj.int32Opt_ref() = 3000;

  obj.stringReq_ref() = "required";
  obj.stringOpt_ref() = "optional";

  obj.setReq_ref() = std::set{1, 2, 3};
  obj.setOpt_ref() = std::set{4, 5, 6};

  obj.listReq_ref() = {111, 222};
  obj.listOpt_ref() = {333, 444};

  obj.structOpt_ref() = makeStructExtra<cpp2::Extra_unbitpack>();
  obj.structPackedOpt_ref() = makeStructExtra<cpp2::Extra_bitpack>();
  obj.structAtomicPackedOpt_ref() =
      makeStructExtra<cpp2::AtomicExtra_bitpack>();
  return obj;
}

template <typename Type>
void randomTestWithSeed(int seed) {
  rng.seed(seed);
  auto obj1 = makeStructBasic<cpp2::Unbitpack>();
  auto obj2 = makeStructBasic<Type>();
  std::vector<std::function<void()>> methods = {
      [&] {
        obj1.int32Req() = 2;
        obj2.int32Req_ref() = 2;
        EXPECT_EQ(obj1.int32Req(), obj2.int32Req_ref());
      },
      [&] {
        obj1.int32Opt() = 3;
        obj2.int32Opt_ref() = 3;
        EXPECT_EQ(obj1.int32Opt(), obj2.int32Opt_ref());
      },
      [&] {
        obj1.int32Opt().reset();
        obj2.int32Opt_ref().reset();
        EXPECT_EQ(obj1.int32Opt(), obj2.int32Opt_ref());
      },
      [&] {
        obj1.stringReq() = "a";
        obj2.stringReq_ref() = "a";
        EXPECT_EQ(obj1.stringReq(), obj2.stringReq_ref());
      },
      [&] {
        obj1.stringOpt() = "b";
        obj2.stringOpt_ref() = "b";
        EXPECT_EQ(obj1.stringOpt(), obj2.stringOpt_ref());
      },
      [&] {
        obj1.stringOpt().reset();
        obj2.stringOpt_ref().reset();
        EXPECT_EQ(obj1.stringOpt(), obj2.stringOpt_ref());
      },
      [&] {
        obj1.setReq() = {7, 8, 9};
        obj2.setReq_ref() = {7, 8, 9};
        EXPECT_EQ(obj1.setReq(), obj2.setReq_ref());
      },
      [&] {
        obj1.setOpt() = {7, 8, 9};
        obj2.setOpt_ref() = {7, 8, 9};
        EXPECT_EQ(obj1.setOpt(), obj2.setOpt_ref());
      },
      [&] {
        obj1.setOpt().reset();
        obj2.setOpt_ref().reset();
        EXPECT_EQ(obj1.setOpt(), obj2.setOpt_ref());
      },
      [&] {
        obj1.listReq() = {555};
        obj2.listReq_ref() = {555};
        EXPECT_EQ(obj1.listReq(), obj2.listReq_ref());
      },
      [&] {
        obj1.structOpt()->extraInt32Def() = 10;
        obj2.structOpt_ref()->extraInt32Def_ref() = 10;
        EXPECT_EQ(obj1.structOpt(), obj2.structOpt_ref());
      },
      [&] {
        obj1.structOpt()->extraInt32Req() = 20;
        obj2.structOpt_ref()->extraInt32Req_ref() = 20;
        EXPECT_EQ(obj1.structOpt(), obj2.structOpt_ref());
      },
      [&] {
        obj1.structOpt()->extraInt32Opt() = 30;
        obj2.structOpt_ref()->extraInt32Opt_ref() = 30;
        EXPECT_EQ(obj1.structOpt(), obj2.structOpt_ref());
      },
      [&] {
        obj1.structOpt()->extraInt32Opt().reset();
        obj2.structOpt_ref()->extraInt32Opt_ref().reset();
        EXPECT_EQ(obj1.structOpt(), obj2.structOpt_ref());
      },
      [&] {
        obj1.structPackedOpt()->extraInt32Def() = 40;
        obj2.structPackedOpt_ref()->extraInt32Def_ref() = 40;
        EXPECT_EQ(obj1.structPackedOpt(), obj2.structPackedOpt_ref());
      },
      [&] {
        obj1.structPackedOpt()->extraInt32Req() = 50;
        obj2.structPackedOpt_ref()->extraInt32Req_ref() = 50;
        EXPECT_EQ(obj1.structPackedOpt(), obj2.structPackedOpt_ref());
      },
      [&] {
        obj1.structPackedOpt()->extraInt32Opt() = 60;
        obj2.structPackedOpt_ref()->extraInt32Opt_ref() = 60;
        EXPECT_EQ(obj1.structPackedOpt(), obj2.structPackedOpt_ref());
      },
      [&] {
        obj1.structPackedOpt()->extraInt32Opt().reset();
        obj2.structPackedOpt_ref()->extraInt32Opt_ref().reset();
        EXPECT_EQ(obj1.structPackedOpt(), obj2.structPackedOpt_ref());
      },
      [&] {
        obj1.structAtomicPackedOpt()->extraInt32Def() = 40;
        obj2.structAtomicPackedOpt_ref()->extraInt32Def_ref() = 40;
        EXPECT_EQ(
            obj1.structAtomicPackedOpt(), obj2.structAtomicPackedOpt_ref());
      },
      [&] {
        obj1.structAtomicPackedOpt()->extraInt32Req() = 50;
        obj2.structAtomicPackedOpt_ref()->extraInt32Req_ref() = 50;
        EXPECT_EQ(
            obj1.structAtomicPackedOpt(), obj2.structAtomicPackedOpt_ref());
      },
      [&] {
        obj1.structAtomicPackedOpt()->extraInt32Opt() = 60;
        obj2.structAtomicPackedOpt_ref()->extraInt32Opt_ref() = 60;
        EXPECT_EQ(
            obj1.structAtomicPackedOpt(), obj2.structAtomicPackedOpt_ref());
      },
      [&] {
        obj1.structAtomicPackedOpt()->extraInt32Opt().reset();
        obj2.structAtomicPackedOpt_ref()->extraInt32Opt_ref().reset();
        EXPECT_EQ(
            obj1.structAtomicPackedOpt(), obj2.structAtomicPackedOpt_ref());
      },
  };
  methods[rng() % methods.size()]();
}

TEST(BitPackTest, compare_size) {
  cpp2::A obj1;
  cpp2::A_bitpack obj2;
  cpp2::A_atomic_bitpack obj3;
  // size comparasion: 16 vs 9 -> 44% memory decreased after bitpacking
  static_assert(sizeof(cpp2::A) == 16);
  static_assert(sizeof(cpp2::A_bitpack) == 9);
  static_assert(sizeof(cpp2::A_atomic_bitpack) == 9);
}

template <class>
struct BitpacksTest : public ::testing::Test {};

using Types = ::testing::Types<cpp2::Bitpack, cpp2::AtomicBitpack>;

TYPED_TEST_SUITE(BitpacksTest, Types);

TYPED_TEST(BitpacksTest, compare_basic) {
  auto obj1 = makeStructBasic<cpp2::Unbitpack>();
  auto obj2 = makeStructBasic<TypeParam>();

  EXPECT_EQ(obj1.int32Req(), obj2.int32Req_ref());
  EXPECT_EQ(obj1.int32Opt(), obj2.int32Opt_ref());
  EXPECT_EQ(obj1.stringReq(), obj2.stringReq_ref());
  EXPECT_EQ(obj1.stringOpt(), obj2.stringOpt_ref());
  EXPECT_EQ(obj1.setReq(), obj2.setReq_ref());
  EXPECT_EQ(obj1.setOpt(), obj2.setOpt_ref());
  EXPECT_EQ(obj1.listReq(), obj2.listReq_ref());
  EXPECT_EQ(obj1.listOpt(), obj2.listOpt_ref());
  EXPECT_EQ(obj1.structOpt(), obj2.structOpt_ref());
  EXPECT_EQ(obj1.structPackedOpt(), obj2.structPackedOpt_ref());
}

class RandomTestWithSeed : public testing::TestWithParam<int> {};

TEST_P(RandomTestWithSeed, test) {
  randomTestWithSeed<cpp2::Bitpack>(GetParam());
  randomTestWithSeed<cpp2::AtomicBitpack>(GetParam());
}

INSTANTIATE_TEST_CASE_P(
    RandomTest,
    RandomTestWithSeed,
    testing::Range(0, folly::kIsDebug ? 100 : 10000));

TEST(AtomicBitpack, multithread) {
  cpp2::AtomicBitpack obj;
  std::vector<std::thread> v;
  v.emplace_back([&] { obj.int32Opt().emplace(); });
  v.emplace_back([&] { obj.stringOpt().emplace(); });
  v.emplace_back([&] { obj.setOpt().emplace(); });
  v.emplace_back([&] { obj.listOpt().emplace(); });
  v.emplace_back([&] { obj.structOpt().emplace(); });
  v.emplace_back([&] { obj.structPackedOpt().emplace(); });
  v.emplace_back([&] { obj.structAtomicPackedOpt().emplace(); });

  for (auto&& t : v) {
    t.join();
  }

  apache::thrift::for_each_field(
      obj, [](auto&&, auto&& ref) { EXPECT_TRUE(ref.has_value()); });
}

} // namespace apache::thrift::test
