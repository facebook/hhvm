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

#include <thrift/compiler/whisker/dsl.h>
#include <thrift/compiler/whisker/object.h>

namespace w = whisker::make;

namespace whisker {

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
  EXPECT_NE(o, array::of({}));
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
  EXPECT_NE(o, map::of({}));
  EXPECT_NE(o, array::ptr());
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, map) {
  object o = w::map(
      {{"foo", w::i64(1)},
       {"bar", w::array({w::string("foo"), w::boolean(true)})}});
  EXPECT_TRUE(o.is_map());
  EXPECT_FALSE(o.is_array());
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
  EXPECT_NE(o, map::ptr());
  EXPECT_EQ(o, o);
}

TEST(ObjectTest, map_array_equality) {
  struct always_zero_map : map {
    explicit always_zero_map(std::set<std::string> keys)
        : keys_(std::move(keys)) {}

    std::optional<object> lookup_property(std::string_view) const override {
      return w::i64(0);
    }

    std::optional<std::set<std::string>> keys() const override { return keys_; }

    void print_to(
        tree_printer::scope& scope,
        const object_print_options& options) const override {
      default_print_to("always_zero_map", keys_, scope, options);
    }

   private:
    std::set<std::string> keys_;
  };

  struct always_zero_array : array {
    explicit always_zero_array(std::size_t sz) : size_(sz) {}

    std::size_t size() const override { return size_; }

    object at(std::size_t) const override { return w::i64(0); }

    void print_to(
        tree_printer::scope& scope,
        const object_print_options& options) const override {
      default_print_to("always_zero_array", scope, options);
    }

   private:
    std::size_t size_;
  };

  map::ptr m1 = std::make_shared<always_zero_map>(std::set<std::string>{"foo"});
  map::ptr m2 = std::make_shared<always_zero_map>(std::set<std::string>{"foo"});
  map::ptr m3 =
      std::make_shared<always_zero_map>(std::set<std::string>{"foo", "bar"});
  map::ptr raw_m3 = map::of({{"foo", w::i64(0)}, {"bar", w::i64(0)}});

  object o1{m1};
  EXPECT_TRUE(o1.is_map());
  EXPECT_FALSE(o1.is_i64());
  EXPECT_EQ(o1, m1);
  EXPECT_EQ(m1, o1);
  EXPECT_EQ(o1, m2);
  EXPECT_EQ(m2, o1);
  EXPECT_NE(o1, m3);
  EXPECT_NE(m3, o1);
  EXPECT_NE(o1, raw_m3);
  EXPECT_EQ(raw_m3, object{m3});
  EXPECT_EQ(object{m3}, raw_m3);

  array::ptr a1 = std::make_shared<always_zero_array>(2);
  array::ptr a2 = std::make_shared<always_zero_array>(2);
  array::ptr a3 = std::make_shared<always_zero_array>(3);
  array::ptr raw_a3 = array::of({w::i64(0), w::i64(0), w::i64(0)});

  object o2{a1};
  EXPECT_TRUE(o2.is_array());
  EXPECT_FALSE(o2.is_i64());
  EXPECT_EQ(o2, a1);
  EXPECT_EQ(a1, o2);
  EXPECT_EQ(o2, a2);
  EXPECT_EQ(a2, o2);
  EXPECT_NE(o2, a3);
  EXPECT_NE(a3, o2);
  EXPECT_NE(o2, raw_a3);
  EXPECT_EQ(raw_a3, object{a3});
  EXPECT_EQ(object{a3}, raw_a3);

  EXPECT_EQ(o1, o1);
  EXPECT_EQ(o2, o2);

  EXPECT_NE(o1, map::ptr());
  EXPECT_NE(map::ptr(), o1);
  EXPECT_NE(o2, array::ptr());
  EXPECT_NE(array::ptr(), o2);
}

TEST(ObjectTest, prototype_find_descriptor) {
  auto parent = prototype<>::from(
      {{"parent_prop", w::true_value}, {"overridden", w::string("base")}},
      nullptr,
      "parent");
  auto child = prototype<>::from(
      {{"child_prop", w::true_value}, {"overridden", w::string("inherited")}},
      parent,
      "child");

  // Unwrap a descriptor to a fixed value, or w::null
  const auto unwrap_descriptor =
      [](const prototype<>::descriptor* descriptor) -> object {
    return descriptor == nullptr
        ? w::null
        : detail::variant_match(
              *descriptor,
              [](const prototype<>::fixed_object& fixed) -> object {
                return fixed.value;
              },
              [](const auto&) -> object { return w::null; });
  };

  // Parent
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(parent->find_descriptor("", "parent_prop")))
      << "parent_prop should be accessible with no name qualifier";
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(parent->find_descriptor("parent", "parent_prop")))
      << "parent_prop should be accessible with parent's name qualifier";

  EXPECT_EQ(
      w::string("base"),
      unwrap_descriptor(parent->find_descriptor("", "overridden")))
      << "overridden should be accessible with no name qualifier";
  EXPECT_EQ(
      w::string("base"),
      unwrap_descriptor(parent->find_descriptor("parent", "overridden")))
      << "overridden should be accessible with parent's name qualifier";

  EXPECT_EQ(
      w::null, unwrap_descriptor(parent->find_descriptor("", "child_prop")))
      << "Non-existent property child_prop should not be accessible on parent";
  EXPECT_EQ(
      w::null,
      unwrap_descriptor(parent->find_descriptor("child", "parent_prop")))
      << "Child's name qualifier should not be accessible directly on parent";

  // Child
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(child->find_descriptor("", "parent_prop")))
      << "parent_prop should be accessible on child without name qualifier";
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(child->find_descriptor("child", "parent_prop")))
      << "parent_prop should be accessible with child's name qualifier";
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(child->find_descriptor("parent", "parent_prop")))
      << "parent_prop should be accessible with parent's name qualifier";

  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(child->find_descriptor("", "child_prop")))
      << "child_prop should be accessible on child";
  EXPECT_EQ(
      w::true_value,
      unwrap_descriptor(child->find_descriptor("child", "child_prop")))
      << "child_prop should be accessible with child's name qualifier";

  EXPECT_EQ(w::null, unwrap_descriptor(child->find_descriptor("", "bar")))
      << "Non-existent property bar should return a null descriptor";

  // By using "parent" as a name qualifier, child is being up-cast to parent, so
  // "child_prop" is not accessible
  EXPECT_EQ(
      w::null,
      unwrap_descriptor(child->find_descriptor("parent", "child_prop")))
      << "child_prop should not be accessible with parent name qualifier";

  EXPECT_EQ(
      w::string("inherited"),
      unwrap_descriptor(child->find_descriptor("", "overridden")))
      << "overridden with no name qualifier should return the child's value";
  EXPECT_EQ(
      w::string("inherited"),
      unwrap_descriptor(child->find_descriptor("child", "overridden")))
      << "overridden with child's name qualifier should return the child's value";
  EXPECT_EQ(
      w::string("base"),
      unwrap_descriptor(child->find_descriptor("parent", "overridden")))
      << "overridden with parent's name qualifier should return the parent's value";
}

TEST(ObjectTest, copy) {
  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo"), w::f64(4.)}),
       w::map({{"bar", w::null}, {"baz", w::string("xyz")}})});
  object o2 = o1;

  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo"), w::f64(4.)}),
           w::map({{"bar", w::null}, {"baz", w::string("xyz")}})}));
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
           w::map({{"bar", w::null}, {"baz", w::string("xyz")}})}));
}

TEST(ObjectTest, move) {
  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo")}),
       w::map({{"bar", w::boolean(true)}, {"baz", w::string("xyz")}})});
  object o2 = std::move(o1);

  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo")}),
           w::map({{"bar", w::boolean(true)}, {"baz", w::string("xyz")}})}));
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
           w::map({{"bar", w::boolean(true)}, {"baz", w::string("xyz")}})}));
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(o2, null());
}

TEST(ObjectTest, swap) {
  object o1 = w::array(
      {w::i64(1),
       w::array({w::string("foo")}),
       w::map({{"bar", w::boolean(true)}, {"baz", w::string("xyz")}})});
  object o2 = w::array(
      {w::array({w::boolean(true), w::string("foo"), w::f64(4.)}),
       w::map({{"bar", w::string("xyz")}, {"baz", w::string("xyz")}})});
  EXPECT_NE(o1, o2);

  o1.swap(o2);
  EXPECT_EQ(
      o1,
      w::array(
          {w::array({w::boolean(true), w::string("foo"), w::f64(4.)}),
           w::map({{"bar", w::string("xyz")}, {"baz", w::string("xyz")}})}));
  EXPECT_EQ(
      o2,
      w::array(
          {w::i64(1),
           w::array({w::string("foo")}),
           w::map({{"bar", w::boolean(true)}, {"baz", w::string("xyz")}})}));
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
    array::ptr a = array::of({w::i64(1), w::string("2"), w::i64(3)});
    o = a;
    EXPECT_EQ(w::array({w::i64(1), w::string("2"), w::i64(3)}), o);
    EXPECT_EQ(a, w::array({w::i64(1), w::string("2"), w::i64(3)}));
  }
  {
    map::ptr m = map::of({{"foo", w::i64(3)}, {"bar", w::string("baz")}});
    o = m;
    EXPECT_EQ((w::map({{"foo", w::i64(3)}, {"bar", w::string("baz")}})), o);
    EXPECT_EQ(m, (w::map({{"foo", w::i64(3)}, {"bar", w::string("baz")}})));
  }
}

TEST(ObjectTest, native_handle_equality) {
  {
    native_handle handle1{manage_owned<int>(42), nullptr /* prototype */};
    native_handle handle2{manage_owned<int>(42), nullptr /* prototype */};
    // Points to different objects, even though they have the same value.
    EXPECT_NE(handle1, handle2);
  }

  {
    auto integer = manage_owned<int>(42);
    native_handle handle1{integer, nullptr /* prototype */};
    native_handle handle2{integer, nullptr /* prototype */};
    EXPECT_EQ(handle1, handle2);

    // The underlying object is the same, even though the handles are of
    // different types.
    native_handle<void> untyped_handle{integer};
    EXPECT_EQ(handle1, untyped_handle);
    EXPECT_EQ(untyped_handle, handle2);
  }
}

// External linkage to avoid noise in demangled type name.
struct ObjectTestSomeNativeType {};

TEST(ObjectTest, to_string) {
  const auto constant = [](auto x) {
    return dsl::make_function([x](auto&&) { return x; });
  };
  ObjectTestSomeNativeType cpp_object;

  class custom_array final : public array {
   public:
    std::size_t size() const final { return 0; }
    [[noreturn]] object at(std::size_t) const final {
      throw std::runtime_error("Should never be called");
    }
  };

  class custom_map final : public map {
   public:
    std::optional<object> lookup_property(std::string_view) const final {
      return std::nullopt;
    }
    std::optional<std::set<std::string>> keys() const final {
      return std::set<std::string>{};
    }
  };

  object o = w::map({
      {"foo", w::i64(1)},
      {"baz",
       w::array(
           {w::string("foo"),
            w::boolean(true),
            w::make_array<custom_array>(),
            w::make_map<custom_map>()})},
      {"abc", w::null},
      {"fun",
       w::array(
           {w::f64(2.f),
            w::array({w::string("foo")}),
            w::map({{"bar", w::i64(1)}, {"baz", w::array({w::null})}})})},
      {"native",
       w::native_handle(
           manage_as_static(cpp_object),
           prototype<>::from(
               {{"foo", constant(w::string("bar"))}},
               prototype<>::from({{"abc", constant(w::string("xyz"))}})))},
  });

  constexpr std::string_view full_output =
      "map (size=5)\n"
      "├─ 'abc' → null\n"
      "├─ 'baz' → array (size=4)\n"
      "│  ├─ [0] 'foo'\n"
      "│  ├─ [1] true\n"
      "│  ├─ [2] array [custom] (size=0)\n"
      "│  ╰─ [3] map [custom] (size=0)\n"
      "├─ 'foo' → i64(1)\n"
      "├─ 'fun' → array (size=3)\n"
      "│  ├─ [0] f64(2)\n"
      "│  ├─ [1] array (size=1)\n"
      "│  │  ╰─ [0] 'foo'\n"
      "│  ╰─ [2] map (size=2)\n"
      "│     ├─ 'bar' → i64(1)\n"
      "│     ╰─ 'baz' → array (size=1)\n"
      "│        ╰─ [0] null\n"
      "╰─ 'native' → <native_handle type='whisker::ObjectTestSomeNativeType'>\n"
      "   ╰─ <prototype (size=1)>\n"
      "      ├─ 'foo'\n"
      "      ╰─ <prototype (size=1)>\n"
      "         ╰─ 'abc'\n";

  EXPECT_EQ(to_string(o), full_output);

  const auto with_depth = [](unsigned depth) {
    return object_print_options{depth};
  };

  // Trying to print with depth 0 is not meaningful but it should not crash.
  EXPECT_EQ(to_string(o, with_depth(0)), "...\n");

  EXPECT_EQ(
      to_string(o, with_depth(1)),
      "map (size=5)\n"
      "├─ 'abc' → null\n"
      "├─ 'baz' → ...\n"
      "├─ 'foo' → i64(1)\n"
      "├─ 'fun' → ...\n"
      "╰─ 'native' → ...\n");

  EXPECT_EQ(
      to_string(o, with_depth(2)),
      "map (size=5)\n"
      "├─ 'abc' → null\n"
      "├─ 'baz' → array (size=4)\n"
      "│  ├─ [0] 'foo'\n"
      "│  ├─ [1] true\n"
      "│  ├─ [2] ...\n"
      "│  ╰─ [3] ...\n"
      "├─ 'foo' → i64(1)\n"
      "├─ 'fun' → array (size=3)\n"
      "│  ├─ [0] f64(2)\n"
      "│  ├─ [1] ...\n"
      "│  ╰─ [2] ...\n"
      "╰─ 'native' → <native_handle type='whisker::ObjectTestSomeNativeType'>\n"
      "   ╰─ ...\n");

  EXPECT_EQ(
      to_string(o, with_depth(3)),
      "map (size=5)\n"
      "├─ 'abc' → null\n"
      "├─ 'baz' → array (size=4)\n"
      "│  ├─ [0] 'foo'\n"
      "│  ├─ [1] true\n"
      "│  ├─ [2] array [custom] (size=0)\n"
      "│  ╰─ [3] map [custom] (size=0)\n"
      "├─ 'foo' → i64(1)\n"
      "├─ 'fun' → array (size=3)\n"
      "│  ├─ [0] f64(2)\n"
      "│  ├─ [1] array (size=1)\n"
      "│  │  ╰─ [0] 'foo'\n"
      "│  ╰─ [2] map (size=2)\n"
      "│     ├─ 'bar' → i64(1)\n"
      "│     ╰─ 'baz' → ...\n"
      "╰─ 'native' → <native_handle type='whisker::ObjectTestSomeNativeType'>\n"
      "   ╰─ <prototype (size=1)>\n"
      "      ├─ 'foo'\n"
      "      ╰─ ...\n");

  EXPECT_EQ(
      // Proxied objects should be printed the same as the underlying object.
      to_string(object(manage_as_static(o)), with_depth(3)),
      "map (size=5)\n"
      "├─ 'abc' → null\n"
      "├─ 'baz' → array (size=4)\n"
      "│  ├─ [0] 'foo'\n"
      "│  ├─ [1] true\n"
      "│  ├─ [2] array [custom] (size=0)\n"
      "│  ╰─ [3] map [custom] (size=0)\n"
      "├─ 'foo' → i64(1)\n"
      "├─ 'fun' → array (size=3)\n"
      "│  ├─ [0] f64(2)\n"
      "│  ├─ [1] array (size=1)\n"
      "│  │  ╰─ [0] 'foo'\n"
      "│  ╰─ [2] map (size=2)\n"
      "│     ├─ 'bar' → i64(1)\n"
      "│     ╰─ 'baz' → ...\n"
      "╰─ 'native' → <native_handle type='whisker::ObjectTestSomeNativeType'>\n"
      "   ╰─ <prototype (size=1)>\n"
      "      ├─ 'foo'\n"
      "      ╰─ ...\n");
  // Proxied objects should compare equal to the underlying object.
  EXPECT_EQ(object(manage_as_static(o)), o);

  EXPECT_EQ(to_string(o, with_depth(4)), full_output);
  EXPECT_EQ(to_string(o, with_depth(5)), full_output);
}

} // namespace whisker
