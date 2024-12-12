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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/whisker/object.h>

namespace w = whisker::make;

namespace whisker {

namespace {
class basic_native_object : public native_object {
  void print_to(
      tree_printer::scope scope, const object_print_options&) const override {
    scope.println("<basic_native_object>");
  }
};
} // namespace

TEST(ObjectTest, empty) {
  object o;
  EXPECT_TRUE(o.is_null());
  EXPECT_EQ(o, w::null);
  EXPECT_EQ(w::null, o);
  EXPECT_EQ(o, object());
  EXPECT_EQ(object(), o);
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, i64) {
  std::vector<object> objects = {
      w::i64(1),
      w::i64(0L),
      w::i64(short(-1)),
      w::i64(std::int32_t(4)),
      w::i64(std::int8_t(5)),
  };
  for (auto& o : objects) {
    EXPECT_TRUE(o.is_i64());
    EXPECT_FALSE(o.is_f64());
    EXPECT_EQ(o, o);
  }
  EXPECT_EQ(objects.front(), w::i64(1));
  EXPECT_EQ(i64(1), objects.front());
  EXPECT_NE(objects.front(), w::i64(2));
  EXPECT_NE(objects.front(), w::f64(2.));
}

TEST(ObjectTest, f64) {
  object o = w::f64(2.);
  EXPECT_TRUE(o.is_f64());
  EXPECT_FALSE(o.is_string());
  EXPECT_EQ(o, 2.);
  EXPECT_EQ(2., o);
  EXPECT_NE(o, 3.);
  EXPECT_NE(o, std::string("bar"));
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, string) {
  object o = w::string("foo");
  EXPECT_TRUE(o.is_string());
  EXPECT_FALSE(o.is_boolean());
  EXPECT_EQ(o, std::string("foo"));
  EXPECT_EQ(std::string("foo"), o);
  EXPECT_NE(o, std::string("bar"));
  EXPECT_NE(o, true);
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, boolean) {
  object o = w::boolean(true);
  EXPECT_TRUE(o.is_boolean());
  EXPECT_FALSE(o.is_null());
  EXPECT_EQ(o, true);
  EXPECT_EQ(true, o);
  EXPECT_NE(o, false);
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, null) {
  object o = null();
  EXPECT_TRUE(o.is_null());
  EXPECT_FALSE(o.is_array());
  EXPECT_EQ(o, null());
  EXPECT_EQ(null(), o);
  EXPECT_NE(o, array());
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, array) {
  object o =
      w::array({w::i64(1), w::array({w::string("foo")}), w::boolean(true)});
  EXPECT_TRUE(o.is_array());
  EXPECT_FALSE(o.is_map());
  EXPECT_EQ(
      o, w::array({w::i64(1), w::array({w::string("foo")}), w::boolean(true)}));
  EXPECT_EQ(
      w::array({w::i64(1), w::array({w::string("foo")}), w::boolean(true)}), o);
  EXPECT_NE(o, w::array({w::string("foo"), w::i64(1), w::boolean(true)}));
  EXPECT_NE(o, w::array({w::i64(0), w::string("foo"), w::boolean(true)}));
  EXPECT_NE(o, map());
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, map) {
  object o = w::map(
      {{"foo", w::i64(1)},
       {"bar", w::array({w::string("foo"), w::boolean(true)})}});
  EXPECT_TRUE(o.is_map());
  EXPECT_FALSE(o.is_native_object());
  EXPECT_EQ(
      o,
      w::map(
          {{"foo", w::i64(1)},
           {"bar", w::array({w::string("foo"), w::boolean(true)})}}));
  EXPECT_NE(
      o,
      w::map(
          {{"foo-bar", w::i64(1)},
           {"bar", w::array({w::string("foo"), w::boolean(true)})}}));
  EXPECT_NE(o, native_object::ptr(nullptr));
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, native_object) {
  native_object::ptr ptr = std::make_shared<basic_native_object>();

  object o = w::native_object(ptr);
  EXPECT_TRUE(o.is_native_object());
  EXPECT_FALSE(o.is_i64());
  EXPECT_EQ(o, ptr);
  EXPECT_EQ(ptr, o);
  EXPECT_NE(o, native_object::ptr(nullptr));
  EXPECT_NE(native_object::ptr(nullptr), o);
  EXPECT_NE(o, i64(1));
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, native_object_equality) {
  struct integer_native_object : basic_native_object {
    explicit integer_native_object(int value) : value(value) {}
    int value;

    bool operator==(const native_object& other) const override {
      if (auto* o = dynamic_cast<const integer_native_object*>(&other)) {
        return value == o->value;
      }
      return false;
    }
  };
  native_object::ptr o1 = std::make_shared<integer_native_object>(5);
  native_object::ptr o2 = std::make_shared<integer_native_object>(5);
  native_object::ptr o3 = std::make_shared<integer_native_object>(10);

  object o = w::native_object(o1);
  EXPECT_TRUE(o.is_native_object());
  EXPECT_FALSE(o.is_i64());

  EXPECT_EQ(o, o1);
  EXPECT_EQ(o1, o);
  EXPECT_EQ(o, o2);
  EXPECT_EQ(o2, o);
  EXPECT_NE(o, o3);
  EXPECT_NE(o3, o);
  EXPECT_NE(o, w::make_native_object<basic_native_object>());
  EXPECT_NE(w::make_native_object<basic_native_object>(), o);

  EXPECT_NE(o, native_object::ptr(nullptr));
  EXPECT_NE(native_object::ptr(nullptr), o);
  EXPECT_NE(o, i64(1));
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, copy) {
  native_object::ptr ptr = std::make_shared<basic_native_object>();

  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo"), w::f64(4.)}),
       w::map({{"bar", w::null}, {"baz", w::native_object(ptr)}})});
  object o2 = o1;

  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo"), w::f64(4.)}),
           w::map({{"bar", w::null}, {"baz", w::native_object(ptr)}})}));
  EXPECT_EQ(o1, o2);

  o1 = "foo";
  EXPECT_EQ(o1, "foo");
  EXPECT_NE(o1, o2);

  o1 = o2;
  EXPECT_EQ(o2, o1);
  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo"), w::f64(4.)}),
           w::map({{"bar", w::null}, {"baz", w::native_object(ptr)}})}));
}

TEST(ObjectTest, move) {
  native_object::ptr ptr = std::make_shared<basic_native_object>();

  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo")}),
       w::map({{"bar", w::boolean(true)}, {"baz", w::native_object(ptr)}})});
  object o2 = std::move(o1);

  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo")}),
           w::map(
               {{"bar", w::boolean(true)}, {"baz", w::native_object(ptr)}})}));
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(o1, null());

  // @lint-ignore CLANGTIDY bugprone-use-after-move
  o1 = std::move(o2);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(
      o1,
      w::array(
          {w::i64(1),
           w::array({w::string("foo")}),
           w::map(
               {{"bar", w::boolean(true)}, {"baz", w::native_object(ptr)}})}));
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(o2, null());
}

TEST(ObjectTest, swap) {
  native_object::ptr ptr1 = std::make_shared<basic_native_object>();
  native_object::ptr ptr2 = std::make_shared<basic_native_object>();

  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo")}),
       w::map({{"bar", w::boolean(true)}, {"baz", w::native_object(ptr1)}})});
  object o2 = w::array(
      {w::array({w::boolean(true), w::string("foo"), w::f64(4.)}),
       w::map({{"bar", w::string("xyz")}, {"baz", w::native_object(ptr1)}})});
  EXPECT_NE(o1, o2);

  o1.swap(o2);
  EXPECT_EQ(
      o1,
      w::array(
          {w::array({w::boolean(true), w::string("foo"), w::f64(4.)}),
           w::map(
               {{"bar", w::string("xyz")}, {"baz", w::native_object(ptr1)}})}));
  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo")}),
           w::map(
               {{"bar", w::boolean(true)}, {"baz", w::native_object(ptr1)}})}));
}

TEST(ObjectTest, assign_copy_alternatives) {
  object o;
  EXPECT_EQ(o, null());
  {
    i64 i = 1;
    o = i;
    EXPECT_EQ(i64(1), o);
  }
  {
    f64 f = 2.;
    o = f;
    EXPECT_EQ(f64(2.), o);
  }
  {
    string str = "foobar";
    o = str;
    EXPECT_EQ("foobar", o);
    EXPECT_EQ("foobar", str);
  }
  {
    boolean b = true;
    o = b;
    EXPECT_EQ(true, o);
  }
  {
    null n = null{};
    o = n;
    EXPECT_EQ(null(), o);
  }
  {
    array a = {w::i64(1), w::string("2"), w::i64(3)};
    o = a;
    EXPECT_EQ(w::array({w::i64(1), w::string("2"), w::i64(3)}), o);
    EXPECT_EQ(a, w::array({w::i64(1), w::string("2"), w::i64(3)}));
  }
  {
    map m = {{"foo", w::i64(3)}, {"bar", w::string("baz")}};
    o = m;
    EXPECT_EQ((w::map({{"foo", w::i64(3)}, {"bar", w::string("baz")}})), o);
    EXPECT_EQ(m, (w::map({{"foo", w::i64(3)}, {"bar", w::string("baz")}})));
  }
  {
    native_object::ptr ptr = std::make_shared<basic_native_object>();
    o = ptr;
    EXPECT_EQ(ptr, o);
    EXPECT_NE(ptr, nullptr);
  }
}

TEST(ObjectTest, to_string) {
  object o = w::map({
      {"foo", w::i64(1)},
      {"baz",
       w::array(
           {w::string("foo"),
            w::boolean(true),
            w::make_native_object<basic_native_object>()})},
      {"abc", w::null},
      {"fun",
       w::array(
           {w::f64(2.f),
            w::array({w::string("foo")}),
            w::map({{"bar", w::i64(1)}, {"baz", w::array({w::null})}})})},
  });

  constexpr std::string_view full_output =
      "map (size=4)\n"
      "`-'abc'\n"
      "  |-null\n"
      "`-'baz'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-'foo'\n"
      "  | `-[1]\n"
      "  |   |-true\n"
      "  | `-[2]\n"
      "  |   |-<basic_native_object>\n"
      "`-'foo'\n"
      "  |-i64(1)\n"
      "`-'fun'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-f64(2)\n"
      "  | `-[1]\n"
      "  |   |-array (size=1)\n"
      "  |   | `-[0]\n"
      "  |   |   |-'foo'\n"
      "  | `-[2]\n"
      "  |   |-map (size=2)\n"
      "  |   | `-'bar'\n"
      "  |   |   |-i64(1)\n"
      "  |   | `-'baz'\n"
      "  |   |   |-array (size=1)\n"
      "  |   |   | `-[0]\n"
      "  |   |   |   |-null\n";

  EXPECT_EQ(to_string(o), full_output);

  const auto with_depth = [](unsigned depth) {
    return object_print_options{depth};
  };

  // Trying to print with depth 0 is not meaningful but it should not crash.
  EXPECT_EQ(to_string(o, with_depth(0)), "...\n");

  EXPECT_EQ(
      to_string(o, with_depth(1)),
      "map (size=4)\n"
      "`-'abc'\n"
      "  |-null\n"
      "`-'baz'\n"
      "  |-...\n"
      "`-'foo'\n"
      "  |-i64(1)\n"
      "`-'fun'\n"
      "  |-...\n");

  EXPECT_EQ(
      to_string(o, with_depth(2)),
      "map (size=4)\n"
      "`-'abc'\n"
      "  |-null\n"
      "`-'baz'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-'foo'\n"
      "  | `-[1]\n"
      "  |   |-true\n"
      "  | `-[2]\n"
      "  |   |-...\n"
      "`-'foo'\n"
      "  |-i64(1)\n"
      "`-'fun'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-f64(2)\n"
      "  | `-[1]\n"
      "  |   |-...\n"
      "  | `-[2]\n"
      "  |   |-...\n");

  EXPECT_EQ(
      to_string(o, with_depth(3)),
      "map (size=4)\n"
      "`-'abc'\n"
      "  |-null\n"
      "`-'baz'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-'foo'\n"
      "  | `-[1]\n"
      "  |   |-true\n"
      "  | `-[2]\n"
      "  |   |-<basic_native_object>\n"
      "`-'foo'\n"
      "  |-i64(1)\n"
      "`-'fun'\n"
      "  |-array (size=3)\n"
      "  | `-[0]\n"
      "  |   |-f64(2)\n"
      "  | `-[1]\n"
      "  |   |-array (size=1)\n"
      "  |   | `-[0]\n"
      "  |   |   |-'foo'\n"
      "  | `-[2]\n"
      "  |   |-map (size=2)\n"
      "  |   | `-'bar'\n"
      "  |   |   |-i64(1)\n"
      "  |   | `-'baz'\n"
      "  |   |   |-...\n");

  EXPECT_EQ(to_string(o, with_depth(4)), full_output);
  EXPECT_EQ(to_string(o, with_depth(5)), full_output);
}

} // namespace whisker
