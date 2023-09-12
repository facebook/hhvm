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

#include <folly/Traits.h>
#include <folly/container/Array.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/index.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/deprecated_terse_writes_types.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/simple_types.h>

constexpr int kIterationCount = folly::kIsDebug ? 50'000 : 500'000;
constexpr int kListMaxSize = 10;

namespace apache::thrift::test {

std::mt19937 rng;

std::vector<int32_t> randomField() {
  std::vector<int32_t> ret(rng() % kListMaxSize);
  std::generate(ret.begin(), ret.end(), std::ref(rng));
  return ret;
}

template <class Serializer, class Struct>
std::string randomSerializedStruct() {
  Struct s;
  s.field4_ref() = randomField();
  return Serializer::template serialize<std::string>(s);
}

template <class Struct, class LazyStruct>
void randomTestWithSeed(int seed) {
  rng.seed(seed);
  Struct foo;
  LazyStruct lazyFoo;

  constexpr bool kIsOptional = apache::thrift::detail::is_optional_field_ref_v<
      std::remove_reference_t<decltype(foo.field4_ref())>>;

  auto create = [](const std::vector<int32_t>& field4) {
    std::pair<Struct, LazyStruct> ret;
    ret.first.field4_ref() = field4;
    ret.second.field4_ref() = field4;
    return ret;
  };

  for (int i = 0; i < kIterationCount; i++) {
    auto arg = randomField();
    std::vector<std::function<void()>> methods = {
        [&] {
          EXPECT_EQ(
              foo.field4_ref().has_value(), lazyFoo.field4_ref().has_value());
        },
        [&] {
          EXPECT_EQ(
              foo.field4_ref().emplace(arg), lazyFoo.field4_ref().emplace(arg));
        },
        [&] { foo.field4_ref() = arg, lazyFoo.field4_ref() = arg; },
        [&] {
          if (foo.field4_ref().has_value() || !kIsOptional) {
            EXPECT_EQ(foo.field4_ref().value(), lazyFoo.field4_ref().value());
          } else {
            EXPECT_THROW(foo.field4_ref().value(), bad_field_access);
            EXPECT_THROW(lazyFoo.field4_ref().value(), bad_field_access);
          }
        },
        [&] {
          if (foo.field4_ref().has_value() || !kIsOptional) {
            EXPECT_EQ(*foo.field4_ref(), *lazyFoo.field4_ref());
          } else {
            EXPECT_THROW(*foo.field4_ref(), bad_field_access);
            EXPECT_THROW(*lazyFoo.field4_ref(), bad_field_access);
          }
        },
        [&] { EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref()); },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo < foo2, lazyFoo < lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo > foo2, lazyFoo > lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo <= foo2, lazyFoo <= lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo >= foo2, lazyFoo >= lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo == foo2, lazyFoo == lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          EXPECT_EQ(foo != foo2, lazyFoo != lazyFoo2);
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          foo = foo2;
          lazyFoo = lazyFoo2;
          EXPECT_EQ(foo, foo2);
          EXPECT_EQ(lazyFoo, lazyFoo2);
          EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
          EXPECT_EQ(foo2.field4_ref(), lazyFoo2.field4_ref());
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          foo2 = foo;
          lazyFoo2 = lazyFoo;
          EXPECT_EQ(foo, foo2);
          EXPECT_EQ(lazyFoo, lazyFoo2);
          EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
          EXPECT_EQ(foo2.field4_ref(), lazyFoo2.field4_ref());
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          foo = std::move(foo2);
          lazyFoo = std::move(lazyFoo2);
          EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
        },
        [&] {
          auto [foo2, lazyFoo2] = create(arg);
          foo2 = std::move(foo);
          lazyFoo2 = std::move(lazyFoo);
          EXPECT_EQ(foo2.field4_ref(), lazyFoo2.field4_ref());

          // Put `foo` and `lazyFoo` back to original state
          foo = std::move(foo2);
          lazyFoo = std::move(lazyFoo2);
        },
        [&] {
          auto foo2 = foo;
          auto lazyFoo2 = lazyFoo;
          EXPECT_EQ(foo, foo2);
          EXPECT_EQ(lazyFoo, lazyFoo2);
          EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
          EXPECT_EQ(foo2.field4_ref(), lazyFoo2.field4_ref());
        },
        [&] {
          auto foo2 = std::move(foo);
          auto lazyFoo2 = std::move(lazyFoo);
          EXPECT_EQ(foo2.field4_ref(), lazyFoo2.field4_ref());

          // Put `foo` and `lazyFoo` back to original state
          foo = std::move(foo2);
          lazyFoo = std::move(lazyFoo2);
        },
    };

    if constexpr (kIsOptional) {
      methods.push_back([&] {
        EXPECT_EQ(bool(foo.field4_ref()), bool(lazyFoo.field4_ref()));
      });
      methods.push_back([&] {
        EXPECT_EQ(
            foo.field4_ref().value_or(arg), lazyFoo.field4_ref().value_or(arg));
      });
      methods.push_back([&] {
        foo.field4_ref().reset();
        lazyFoo.field4_ref().reset();
        EXPECT_FALSE(foo.field4_ref().has_value());
        EXPECT_FALSE(lazyFoo.field4_ref().has_value());
      });
    }

    auto addSerializationMethods = [&](auto ser) {
      using Serializer = decltype(ser);
      methods.push_back([&] {
        auto s = randomSerializedStruct<Serializer, Struct>();
        Serializer::deserialize(s, foo);
        Serializer::deserialize(s, lazyFoo);
      });
      methods.push_back([&] {
        auto s = randomSerializedStruct<Serializer, LazyStruct>();
        Serializer::deserialize(s, foo);
        Serializer::deserialize(s, lazyFoo);
      });
      methods.push_back([&] {
        auto s = randomSerializedStruct<Serializer, Struct>();
        foo = Serializer::template deserialize<Struct>(s);
        lazyFoo = Serializer::template deserialize<LazyStruct>(s);
      });
      methods.push_back([&] {
        auto s = randomSerializedStruct<Serializer, LazyStruct>();
        foo = Serializer::template deserialize<Struct>(s);
        lazyFoo = Serializer::template deserialize<LazyStruct>(s);
      });
      methods.push_back([&] {
        foo = Serializer::template deserialize<Struct>(
            Serializer::template serialize<std::string>(foo));
        lazyFoo = Serializer::template deserialize<LazyStruct>(
            Serializer::template serialize<std::string>(lazyFoo));
      });
    };

    addSerializationMethods(CompactSerializer{});
    addSerializationMethods(BinarySerializer{});

    // Choose a random method and call it
    methods[rng() % methods.size()]();
  }
}

class RandomTestWithSeed : public testing::TestWithParam<int> {};
TEST_P(RandomTestWithSeed, test) {
  for (bool enable : {false, true}) {
    FLAGS_thrift_enable_lazy_deserialization = enable;
    randomTestWithSeed<Foo, LazyFoo>(GetParam());
    randomTestWithSeed<OptionalFoo, OptionalLazyFoo>(GetParam());
    randomTestWithSeed<TerseFoo, TerseLazyFoo>(GetParam());
    randomTestWithSeed<TerseOptionalFoo, TerseOptionalLazyFoo>(GetParam());
  }
}

INSTANTIATE_TEST_CASE_P(
    RandomTest,
    RandomTestWithSeed,
    testing::Range(0, folly::kIsDebug ? 10 : 1000));

} // namespace apache::thrift::test
