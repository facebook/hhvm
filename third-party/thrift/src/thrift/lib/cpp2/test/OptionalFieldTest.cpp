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
#include <thrift/lib/cpp2/FieldRefHash.h>

#include <type_traits>

namespace apache::thrift {
template <class T, class U>
void heterogeneousComparisonsTest(T opt1, U opt2) {
  EXPECT_TRUE(opt1(4) == int64_t(4));
  EXPECT_FALSE(opt1(8) == int64_t(4));
  EXPECT_FALSE(opt1() == int64_t(4));

  EXPECT_TRUE(int64_t(4) == opt1(4));
  EXPECT_FALSE(int64_t(4) == opt1(8));
  EXPECT_FALSE(int64_t(4) == opt1());

  EXPECT_FALSE(opt1(4) != int64_t(4));
  EXPECT_TRUE(opt1(8) != int64_t(4));
  EXPECT_TRUE(opt1() != int64_t(4));

  EXPECT_FALSE(int64_t(4) != opt1(4));
  EXPECT_TRUE(int64_t(4) != opt1(8));
  EXPECT_TRUE(int64_t(4) != opt1());

  EXPECT_TRUE(opt1() < int64_t(4));
  EXPECT_TRUE(opt1(4) < int64_t(8));
  EXPECT_FALSE(opt1(4) < int64_t(4));
  EXPECT_FALSE(opt1(8) < int64_t(4));

  EXPECT_FALSE(opt1() > int64_t(4));
  EXPECT_FALSE(opt1(4) > int64_t(8));
  EXPECT_FALSE(opt1(4) > int64_t(4));
  EXPECT_TRUE(opt1(8) > int64_t(4));

  EXPECT_TRUE(opt1() <= int64_t(4));
  EXPECT_TRUE(opt1(4) <= int64_t(8));
  EXPECT_TRUE(opt1(4) <= int64_t(4));
  EXPECT_FALSE(opt1(8) <= int64_t(4));

  EXPECT_FALSE(opt1() >= int64_t(4));
  EXPECT_FALSE(opt1(4) >= int64_t(8));
  EXPECT_TRUE(opt1(4) >= int64_t(4));
  EXPECT_TRUE(opt1(8) >= int64_t(4));

  EXPECT_TRUE(int64_t(4) < opt1(8));
  EXPECT_FALSE(int64_t(4) < opt1(4));
  EXPECT_FALSE(int64_t(8) < opt1(4));
  EXPECT_FALSE(int64_t(4) < opt1());

  EXPECT_FALSE(int64_t(4) > opt1(8));
  EXPECT_FALSE(int64_t(4) > opt1(4));
  EXPECT_TRUE(int64_t(8) > opt1(4));
  EXPECT_TRUE(int64_t(4) > opt1());

  EXPECT_TRUE(int64_t(4) <= opt1(8));
  EXPECT_TRUE(int64_t(4) <= opt1(4));
  EXPECT_FALSE(int64_t(8) <= opt1(4));
  EXPECT_FALSE(int64_t(4) <= opt1());

  EXPECT_FALSE(int64_t(4) >= opt1(8));
  EXPECT_TRUE(int64_t(4) >= opt1(4));
  EXPECT_TRUE(int64_t(8) >= opt1(4));
  EXPECT_TRUE(int64_t(4) >= opt1());

  EXPECT_TRUE(opt1() == opt2());
  EXPECT_TRUE(opt1(4) == opt2(4));
  EXPECT_FALSE(opt1(8) == opt2(4));
  EXPECT_FALSE(opt1() == opt2(4));
  EXPECT_FALSE(opt1(4) == opt2());

  EXPECT_FALSE(opt1() != opt2());
  EXPECT_FALSE(opt1(4) != opt2(4));
  EXPECT_TRUE(opt1(8) != opt2(4));
  EXPECT_TRUE(opt1() != opt2(4));
  EXPECT_TRUE(opt1(4) != opt2());

  EXPECT_TRUE(opt1() < opt2(4));
  EXPECT_TRUE(opt1(4) < opt2(8));
  EXPECT_FALSE(opt1() < opt2());
  EXPECT_FALSE(opt1(4) < opt2(4));
  EXPECT_FALSE(opt1(8) < opt2(4));
  EXPECT_FALSE(opt1(4) < opt2());

  EXPECT_FALSE(opt1() > opt2(4));
  EXPECT_FALSE(opt1(4) > opt2(8));
  EXPECT_FALSE(opt1() > opt2());
  EXPECT_FALSE(opt1(4) > opt2(4));
  EXPECT_TRUE(opt1(8) > opt2(4));
  EXPECT_TRUE(opt1(4) > opt2());

  EXPECT_TRUE(opt1() <= opt2(4));
  EXPECT_TRUE(opt1(4) <= opt2(8));
  EXPECT_TRUE(opt1() <= opt2());
  EXPECT_TRUE(opt1(4) <= opt2(4));
  EXPECT_FALSE(opt1(8) <= opt2(4));
  EXPECT_FALSE(opt1(4) <= opt2());

  EXPECT_FALSE(opt1() >= opt2(4));
  EXPECT_FALSE(opt1(4) >= opt2(8));
  EXPECT_TRUE(opt1() >= opt2());
  EXPECT_TRUE(opt1(4) >= opt2(4));
  EXPECT_TRUE(opt1(8) >= opt2(4));
  EXPECT_TRUE(opt1(4) >= opt2());
}

TEST(OptionalFieldRefTest, HeterogeneousComparisons) {
  auto genOptionalFieldRef = [i_ = int8_t(0),
                              b_ = uint8_t(0)](auto... i) mutable {
    return detail::make_optional_field_ref(i_ = int(i...), b_ = sizeof...(i));
  };
  heterogeneousComparisonsTest(genOptionalFieldRef, genOptionalFieldRef);
}

TEST(FieldRefTest, HeterogeneousComparisons) {
  auto genFieldRef = [i_ = int8_t(0), b_ = uint8_t(0)](auto... i) mutable {
    return detail::make_field_ref(i_ = int(i...), b_ = sizeof...(i));
  };
  heterogeneousComparisonsTest(genFieldRef, genFieldRef);
}

TEST(RequiredFieldRefTest, HeterogeneousComparisons) {
  auto genRequiredFieldRef = [i_ = int8_t(0)](auto... i) mutable {
    return required_field_ref<const int8_t&>(i_ = int(i...));
  };
  heterogeneousComparisonsTest(genRequiredFieldRef, genRequiredFieldRef);
}
} // namespace apache::thrift

struct Tag {};

namespace std {
// @lint-ignore CLANGTIDY
template <>
struct hash<Tag> {
  explicit hash(size_t i) : i(i) {}
  size_t operator()(Tag) const { return i; }
  size_t i;
};
} // namespace std

namespace folly {
// @lint-ignore CLANGTIDY
template <>
struct hasher<Tag> {
  explicit hasher(size_t i) : i(i) {}
  size_t operator()(Tag) const { return i; }
  size_t i;
};
} // namespace folly

namespace apache::thrift {
template <template <class...> class Hasher, template <class> class Optional>
void StatelessHashTest() {
  Hasher<Optional<int&>> hash;
  int x = 0;
  uint8_t y = 0;
  Optional<int&> f(x, y);
  if (std::is_same_v<Optional<int&>, apache::thrift::field_ref<int&>>) {
    EXPECT_EQ(hash(f), Hasher<int>()(0));
  } else {
    EXPECT_EQ(
        hash(f), apache::thrift::detail::kHashValueForNonExistsOptionalField);
  }
  y = true;
  for (x = 0; x < 1000; x++) {
    EXPECT_EQ(hash(f), Hasher<int>()(x));
  }
}

template <template <class...> class Hasher, template <class> class Optional>
void StatefulHashTest() {
  Hasher<Optional<Tag&>> hash(42);
  Tag x;
  uint8_t y = 0;
  Optional<Tag&> f(x, y);
  if (std::is_same_v<Optional<int&>, apache::thrift::field_ref<int&>>) {
    EXPECT_EQ(hash(f), 42);
  } else {
    EXPECT_EQ(
        hash(f), apache::thrift::detail::kHashValueForNonExistsOptionalField);
  }
  y = true;
  EXPECT_EQ(hash(f), 42);
}

TEST(optionalFieldRefTest, Hash) {
  StatelessHashTest<std::hash, field_ref>();
  StatelessHashTest<std::hash, optional_field_ref>();
  StatelessHashTest<folly::hasher, field_ref>();
  StatelessHashTest<folly::hasher, optional_field_ref>();
  StatefulHashTest<std::hash, field_ref>();
  StatefulHashTest<std::hash, optional_field_ref>();
  StatefulHashTest<folly::hasher, field_ref>();
  StatefulHashTest<folly::hasher, optional_field_ref>();
}
} // namespace apache::thrift
