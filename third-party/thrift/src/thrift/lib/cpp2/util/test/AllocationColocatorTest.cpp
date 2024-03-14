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

#include <string_view>

#include <thrift/lib/cpp2/util/AllocationColocator.h>

using apache::thrift::util::AllocationColocator;

TEST(AllocationColocatorTest, Basic) {
  constexpr std::string_view kStr = "hello world";
  struct Foo {
    struct Bar {
      int first, second;
      explicit Bar(int first, int second) : first(first), second(second) {}
      explicit Bar(int first) : Bar(first, 42) {}
      bool operator==(const Bar&) const = default;
    };

    int a;
    Bar* b;
    int* c;
    std::string_view d;
    int* e;

    explicit Foo(int&& a) : a(a) {}
  };

  AllocationColocator<Foo> alloc;
  auto b = alloc.object<Foo::Bar>();
  auto c = alloc.array<int>(2);
  auto d = alloc.string(kStr.length());
  auto e = alloc.object<int>();

  {
    AllocationColocator<Foo>::Ptr foo = alloc.allocate([&](auto make) mutable {
      Foo foo(10);
      foo.b = make(std::move(b), 12);
      foo.c = make(std::move(c));
      foo.c[0] = 50;
      foo.c[1] = 60;
      foo.e = make(std::move(e), 18);
      foo.d = make(std::move(d), kStr);
      return foo;
    });

    {
      auto offset = sizeof(Foo);
      EXPECT_EQ(b.offset, offset);

      offset += sizeof(Foo::Bar);
      EXPECT_EQ(c.offset, offset);

      offset += sizeof(int) * 2;
      EXPECT_EQ(d.offset, offset);

      offset += sizeof(char) * kStr.length() + 1;
      EXPECT_EQ(e.offset, offset);

      offset += sizeof(int);
      EXPECT_EQ(
          apache::thrift::util::detail::AllocationColocatorInternals::
              getNumBytesForAllocation(alloc),
          offset);
    }

    EXPECT_EQ(foo->a, 10);
    EXPECT_EQ(*foo->b, Foo::Bar(12, 42));
    EXPECT_EQ(foo->c[0], 50);
    EXPECT_EQ(foo->c[1], 60);
    EXPECT_EQ(foo->d, kStr);
    EXPECT_EQ(*foo->e, 18);

    auto cursor = AllocationColocator<Foo>::unsafeCursor(foo);
    EXPECT_EQ(*cursor.object<Foo::Bar>(), Foo::Bar(12, 42));
    auto arr = cursor.array<int>(2);
    EXPECT_EQ(arr[0], 50);
    EXPECT_EQ(arr[1], 60);
    EXPECT_EQ(cursor.string(kStr.length()), kStr);
    EXPECT_EQ(*cursor.object<int>(), 18);
  }
}

TEST(AllocationColocatorTest, StringLengthDeathTest) {
  constexpr std::string_view kStr = "hello world";
  struct Foo {
    std::string_view str;
  };

  AllocationColocator<Foo> alloc;
  EXPECT_DEATH(
      ({
        alloc.allocate(
            [&, str = alloc.string(kStr.length())](auto make) mutable {
              Foo foo;
              foo.str = make.string(std::move(str), std::string(kStr) + ".");
              return foo;
            });
      }),
      "String value length exceeds requested buffer length");
}

TEST(AllocationColocatorTest, NoExcept) {
  constexpr std::string_view kStr = "hello world";
  struct Foo {
    int* a;
    int* b;
    std::string_view c;
  };

  AllocationColocator<Foo> alloc;
  auto build =
      [&, a = alloc.object<int>(), b = alloc.array<int>(2), c = alloc.string(kStr.length())](auto make) mutable noexcept(
          noexcept(
              make(
                  std::declval<AllocationColocator<>::ObjectLocator<int>>(), 1),
              make(std::declval<AllocationColocator<>::ArrayLocator<int>>()),
              make(std::declval<AllocationColocator<>::StringLocator>(), kStr)))
      -> Foo {
    Foo foo;
    foo.a = make(std::move(a), 1);
    foo.b = make(std::move(b));
    foo.c = make(std::move(c), kStr);
    return foo;
  };
  EXPECT_TRUE(noexcept(build(std::declval<decltype(alloc)::Builder>())));

  auto foo = alloc.allocate(build);
  foo->b[0] = 2;
  foo->b[1] = 3;
  EXPECT_EQ(*foo->a, 1);
  EXPECT_EQ(foo->c, kStr);
}

TEST(AllocationColocatorTest, Alignment) {
  constexpr auto kAlign = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
  struct alignas(kAlign) FatInt {
    int value;
  };
  struct Foo {
    FatInt* ptr;
  };

  AllocationColocator<Foo> alloc;
  auto ptr = alloc.object<FatInt>();
  EXPECT_EQ(ptr.offset, kAlign);

  auto foo = alloc.allocate([ptr = std::move(ptr)](auto make) mutable {
    Foo foo;
    foo.ptr = make(std::move(ptr), FatInt{1});
    return foo;
  });

  EXPECT_EQ(foo->ptr->value, 1);
  EXPECT_EQ(
      apache::thrift::util::detail::AllocationColocatorInternals::
          getNumBytesForAllocation(alloc),
      kAlign + sizeof(FatInt));

  auto cursor = AllocationColocator<Foo>::unsafeCursor(foo);
  EXPECT_EQ(cursor.object<FatInt>()->value, 1);
}

TEST(AllocationColocatorTest, NonTrivialDestructor) {
  struct Foo {
    struct NonTrivial {
      int& ref;
      explicit NonTrivial(int& ref) : ref(ref) {}
      ~NonTrivial() noexcept { ref = 42; }
    };

    int value;
    AllocationColocator<>::Ptr<NonTrivial> nonTrivial;
  };

  int ref = 100;
  AllocationColocator<Foo> alloc;
  auto foo = alloc.allocate(
      [&, nonTrivial = alloc.object<Foo::NonTrivial>()](auto make) mutable {
        Foo foo;
        foo.value = 24;
        foo.nonTrivial = make(std::move(nonTrivial), ref);
        return foo;
      });

  EXPECT_EQ(foo->value, 24);
  EXPECT_EQ(foo->nonTrivial->ref, 100);

  auto cursor = AllocationColocator<Foo>::unsafeCursor(
      static_cast<const Foo*>(foo.get()));
  EXPECT_EQ(cursor.object<Foo::NonTrivial>()->ref, 100);

  foo.reset();
  EXPECT_EQ(ref, 42);
}

TEST(AllocationColocatorTest, NonTrivialDestructorArray) {
  struct Foo {
    struct NonTrivial {
      int& destructorCount;

      NonTrivial(int& constructorCount, int& destructorCount)
          : destructorCount(destructorCount) {
        constructorCount++;
      }
      ~NonTrivial() noexcept { ++destructorCount; }
    };
    AllocationColocator<>::ArrayPtr<NonTrivial> nonTrivials;
  };

  int constructorCount = 0;
  int destructorCount = 0;
  AllocationColocator<Foo> alloc;
  auto foo = alloc.allocate(
      [&, nonTrivial = alloc.array<Foo::NonTrivial>(2)](auto make) mutable {
        Foo foo;
        foo.nonTrivials = make(std::move(nonTrivial), [&] {
          return Foo::NonTrivial(constructorCount, destructorCount);
        });
        return foo;
      });

  EXPECT_EQ(constructorCount, 2);
  EXPECT_EQ(destructorCount, 0);
  EXPECT_EQ(&destructorCount, &foo->nonTrivials[0].destructorCount);
  EXPECT_EQ(&destructorCount, &foo->nonTrivials[1].destructorCount);

  auto cursor = AllocationColocator<Foo>::unsafeCursor(
      static_cast<const Foo*>(foo.get()));
  EXPECT_EQ(
      &cursor.object<Foo::NonTrivial>()->destructorCount, &destructorCount);
  EXPECT_EQ(
      &cursor.object<Foo::NonTrivial>()->destructorCount, &destructorCount);

  foo.reset();
  EXPECT_EQ(constructorCount, 2);
  EXPECT_EQ(destructorCount, 2);
}

TEST(AllocationColocatorTest, NonTrivialDestructorArrayWithException) {
  struct Foo {
    struct NonTrivial {
      int& destructorCount;

      NonTrivial(int& constructorCount, int& destructorCount)
          : destructorCount(destructorCount) {
        constructorCount++;
        if (constructorCount == 3) {
          throw std::runtime_error("this is the third object!");
        }
      }
      ~NonTrivial() noexcept { ++destructorCount; }
    };
    AllocationColocator<>::ArrayPtr<NonTrivial> nonTrivials;
  };

  int constructorCount = 0;
  int destructorCount = 0;
  AllocationColocator<Foo> alloc;
  EXPECT_THROW(
      {
        auto foo =
            alloc.allocate([&, nonTrivial = alloc.array<Foo::NonTrivial>(3)](
                               auto make) mutable {
              Foo foo;
              foo.nonTrivials = make(std::move(nonTrivial), [&] {
                return Foo::NonTrivial(constructorCount, destructorCount);
              });
              return foo;
            });
      },
      std::runtime_error);

  EXPECT_EQ(constructorCount, 3);
  EXPECT_EQ(destructorCount, 2);
}

TEST(AllocationColocatorTest, ArrayZeroSize) {
  struct Foo {
    AllocationColocator<>::ArrayPtr<std::string> array1;
    AllocationColocator<>::ArrayPtr<std::string> array2;
  };

  AllocationColocator<Foo> alloc;
  auto foo =
      alloc.allocate([&,
                      array1 = alloc.array<std::string>(0),
                      array2 = alloc.array<std::string>(0)](auto make) mutable {
        Foo foo;
        auto generator = []() -> std::string {
          throw std::runtime_error("Should never be called!");
        };
        foo.array1 = make(std::move(array1), generator);
        foo.array2 = make(std::move(array2), generator);
        return foo;
      });

  EXPECT_NE(foo->array1.get(), nullptr);
  EXPECT_EQ(foo->array1.get(), foo->array2.get());
}
