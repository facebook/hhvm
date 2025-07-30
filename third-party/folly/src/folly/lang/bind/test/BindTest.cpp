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

#include <folly/lang/bind/Bind.h>
#include <folly/portability/GTest.h>

namespace folly::bind::detail {

using namespace ::folly::bind::ext;

namespace detail {
using by_ref_bind_info = decltype([](auto bi) {
  bi.category = ext::category_t::ref;
  return bi;
});
} // namespace detail

// This isn't in `Bind.h` only because it's unclear if users need something a
// const-defaultable "by reference" verb.
template <typename... Ts>
struct by_ref : ext::merge_update_args<detail::by_ref_bind_info, Ts...> {
  using ext::merge_update_args<detail::by_ref_bind_info, Ts...>::
      merge_update_args;
};
template <typename... Ts>
by_ref(Ts&&...) -> by_ref<ext::deduce_args_t<Ts>...>;

struct Foo : folly::NonCopyableNonMovable {
  constexpr explicit Foo(bool* made, int n) : n_(n) {
    if (made) {
      *made = true;
    }
  }
  int n_;
};

// This is here so that test "runs" show up in CI history
TEST(BindTest, all_tests_run_at_build_time) {
  // This is a manually-enabled example of the `lifetimebound` annotation on
  // `in_place_args::unsafe_tuple_to_bind()`.  With `lifetimebound` it
  // won't compile, without it would hit an ASAN failure.  It has to be a
  // runtime test because `constexpr` evaluation detects usage of dangling
  // references regardless of `lifetimebound`..
#if 0
  int n = 1337;
  bool made = false;
  auto fooMaker = in_place<Foo>(&made, n).unsafe_tuple_to_bind();
  // UNSAFE: `fooMaker` contains a ref to the prvalue `&made`, which became
  // invalid at the `;` of the previous line.
  lite_tuple::tuple<Foo> foo = std::move(fooMaker);
  EXPECT_TRUE(made);
  EXPECT_EQ(1337, std::get<0>(foo).n_);
  FAIL() << "Should not compile, or at least fail under ASAN.";
#endif
}

// Better UX than `assert()` in constexpr tests.
constexpr void test(bool ok) {
  if (!ok) {
    throw std::exception(); // Throwing in constexpr code is a compile error
  }
}

constexpr auto check_ref_args() {
  int y = 5;
  static_assert(std::is_same_v<decltype(args{5}), args<int&&>>);
  {
    auto lval = args(y);
    static_assert(std::is_same_v<decltype(lval), args<int&>>);
    static_assert(
        std::is_same_v<
            decltype(lval)::binding_list_t,
            tag_t<binding_t<bind_info_t{}, int&>>>);
    y += 20;
    static_assert(
        std::is_same_v<
            decltype(std::move(lval).unsafe_tuple_to_bind()),
            lite_tuple::tuple<int&>>);
    test(25 == lite_tuple::get<0>(std::move(lval).unsafe_tuple_to_bind()));
  }
  {
    auto rval = args(std::move(y));
    static_assert(std::is_same_v<decltype(rval), args<int&&>>);
    static_assert(
        std::is_same_v<
            decltype(rval)::binding_list_t,
            tag_t<binding_t<bind_info_t{}, int&&>>>);
    y -= 10;
    static_assert(
        std::is_same_v<
            decltype(std::move(rval).unsafe_tuple_to_bind()),
            lite_tuple::tuple<int&&>>);
    test(15 == lite_tuple::get<0>(std::move(rval).unsafe_tuple_to_bind()));
  }
  return true;
}

static_assert(check_ref_args());

constexpr auto check_nested_args() {
  int b = 2, d = 4;
  using FlatT = decltype(args{1.2, args{b, 'x'}, d, args{}, "abc"});
  static_assert(
      std::is_same_v<
          FlatT,
          args<double&&, args<int&, char&&>, int&, args<>, const char(&)[4]>>);
  constexpr auto BI = bind_info_t{};
  static_assert(
      std::is_same_v<
          FlatT::binding_list_t,
          tag_t<
              binding_t<BI, double&&>,
              binding_t<BI, int&>,
              binding_t<BI, char&&>,
              binding_t<BI, int&>,
              binding_t<BI, const char(&)[4]>>>);
  return true;
}

static_assert(check_nested_args());

constexpr auto check_const_and_non_const() {
  double b = 2.3;

  using non_const_bas = decltype(mut(1, args(b, 'c')));
  static_assert(
      std::is_same_v<non_const_bas, mut<int&&, args<double&, char&&>>>);
  constexpr bind_info_t def_non_const_bi{category_t{}, constness_t::mut};
  static_assert(
      std::is_same_v<
          non_const_bas::binding_list_t,
          tag_t<
              binding_t<def_non_const_bi, int&&>,
              binding_t<def_non_const_bi, double&>,
              binding_t<def_non_const_bi, char&&>>>);

  using const_bas = decltype(constant{mut{1, args{b, 'c'}}});
  static_assert(
      std::is_same_v<const_bas, constant<mut<int&&, args<double&, char&&>>>>);
  constexpr bind_info_t def_const_bi{category_t{}, constness_t::constant};
  static_assert(
      std::is_same_v<
          const_bas::binding_list_t,
          tag_t<
              binding_t<def_const_bi, int&&>,
              binding_t<def_const_bi, double&>,
              binding_t<def_const_bi, char&&>>>);

  static_assert(
      std::is_same_v<
          decltype(constant(b))::binding_list_t,
          tag_t<binding_t<def_const_bi, double&>>>);

  static_assert(
      std::is_same_v<
          decltype(constant(mut(b)))::binding_list_t,
          tag_t<binding_t<def_const_bi, double&>>>);

  static_assert(
      std::is_same_v<
          decltype(mut(b))::binding_list_t,
          tag_t<binding_t<def_non_const_bi, double&>>>);

  static_assert(
      std::is_same_v<
          decltype(mut(constant(b)))::binding_list_t,
          tag_t<binding_t<def_non_const_bi, double&>>>);

  return true;
}

static_assert(check_const_and_non_const());

constexpr auto check_by_ref() {
  double b = 2.3;

  using ref = decltype(by_ref{1, args{b, 'c'}});
  static_assert(std::is_same_v<ref, by_ref<int&&, args<double&, char&&>>>);
  constexpr bind_info_t ref_def_bi{category_t::ref, constness_t{}};
  static_assert(
      std::is_same_v<
          ref::binding_list_t,
          tag_t<
              binding_t<ref_def_bi, int&&>,
              binding_t<ref_def_bi, double&>,
              binding_t<ref_def_bi, char&&>>>);

  using constant_ref = decltype(const_ref{1, args{b, 'c'}});
  static_assert(
      std::is_same_v<constant_ref, const_ref<int&&, args<double&, char&&>>>);
  constexpr bind_info_t ref_const_bi{category_t::ref, constness_t::constant};
  static_assert(
      std::is_same_v<
          constant_ref::binding_list_t,
          tag_t<
              binding_t<ref_const_bi, int&&>,
              binding_t<ref_const_bi, double&>,
              binding_t<ref_const_bi, char&&>>>);

  using mutable_ref = decltype(mut{const_ref{1, args{b, 'c'}}});
  static_assert(
      std::
          is_same_v<mutable_ref, mut<const_ref<int&&, args<double&, char&&>>>>);
  constexpr bind_info_t ref_non_const_bi{category_t::ref, constness_t::mut};
  using non_const_bindings = tag_t<
      binding_t<ref_non_const_bi, int&&>,
      binding_t<ref_non_const_bi, double&>,
      binding_t<ref_non_const_bi, char&&>>;
  static_assert(
      std::is_same_v<mutable_ref::binding_list_t, non_const_bindings>);

  using non_const_ref = decltype(mut_ref{1, args{b, 'c'}});
  static_assert(
      std::is_same_v<non_const_ref, mut_ref<int&&, args<double&, char&&>>>);
  static_assert(
      std::is_same_v<non_const_ref::binding_list_t, non_const_bindings>);

  static_assert(
      std::is_same_v<
          decltype(constant(const_ref(b))),
          constant<const_ref<double&>>>);
  static_assert(
      std::is_same_v<
          decltype(constant(const_ref(b)))::binding_list_t,
          tag_t<binding_t<ref_const_bi, double&>>>);
  static_assert(
      std::is_same_v<
          decltype(const_ref(constant(b))),
          const_ref<constant<double&>>>);
  static_assert(
      std::is_same_v<
          decltype(const_ref(constant(b)))::binding_list_t,
          tag_t<binding_t<ref_const_bi, double&>>>);

  using bind_ref_non_const = tag_t<binding_t<ref_non_const_bi, double&>>;
  static_assert(
      std::is_same_v<decltype(mut_ref(b))::binding_list_t, bind_ref_non_const>);
  static_assert(
      std::is_same_v<
          decltype(mut(const_ref(b)))::binding_list_t,
          bind_ref_non_const>);
  static_assert(
      std::is_same_v<
          decltype(by_ref{mut{b}})::binding_list_t,
          bind_ref_non_const>);
  static_assert(
      std::is_same_v<
          decltype(const_ref{mut{b}})::binding_list_t,
          tag_t<binding_t<ref_const_bi, double&>>>);

  return true;
}

static_assert(check_by_ref());

constexpr auto check_move_and_copy() {
  double b = 2.3;

  // Test copy/move modifier by itself
  using move_args = decltype(move{1, args{b, 'c'}});
  static_assert(std::is_same_v<move_args, move<int&&, args<double&, char&&>>>);
  constexpr bind_info_t move_bi{category_t::move, constness_t{}};
  static_assert(
      std::is_same_v<
          move_args::binding_list_t,
          tag_t<
              binding_t<move_bi, int&&>,
              binding_t<move_bi, double&>,
              binding_t<move_bi, char&&>>>);

  using copy_args = decltype(copy{1, args{b, 'c'}});
  static_assert(std::is_same_v<copy_args, copy<int&&, args<double&, char&&>>>);
  constexpr bind_info_t copy_bi{category_t::copy, constness_t{}};
  static_assert(
      std::is_same_v<
          copy_args::binding_list_t,
          tag_t<
              binding_t<copy_bi, int&&>,
              binding_t<copy_bi, double&>,
              binding_t<copy_bi, char&&>>>);

  // Lightly test composition with `constness` modifiers -- order doesn't matter
  using const_move = decltype(constant{move{b}});
  static_assert(std::is_same_v<const_move, constant<move<double&>>>);
  constexpr bind_info_t const_move_bi{category_t::move, constness_t::constant};
  static_assert(
      std::is_same_v<
          const_move::binding_list_t,
          tag_t<binding_t<const_move_bi, double&>>>);

  using copy_const = decltype(copy{constant{b}});
  static_assert(std::is_same_v<copy_const, copy<constant<double&>>>);
  constexpr bind_info_t const_copy_bi{category_t::copy, constness_t::constant};
  static_assert(
      std::is_same_v<
          copy_const::binding_list_t,
          tag_t<binding_t<const_copy_bi, double&>>>);

  // Composing `move{const_ref{}}` replaces `category` with `move` but
  // preserves `constness` from the inner `const_ref`.
  using move_ref = decltype(move{const_ref{b}});
  static_assert(std::is_same_v<move_ref, move<const_ref<double&>>>);
  constexpr bind_info_t move_ref_bi{category_t::move, constness_t::constant};
  static_assert(
      std::is_same_v<
          move_ref::binding_list_t,
          tag_t<binding_t<move_ref_bi, double&>>>);

  // Light `in_place` test -- it's independent of the `category` change
  using move_in_place = decltype(move{in_place<int>(42)});
  static_assert(std::is_same_v<move_in_place, move<in_place_args<int, int>>>);
  static_assert(
      std::is_same_v<
          move_in_place::binding_list_t,
          tag_t<binding_t<move_bi, int>>>);

  return true;
}

static_assert(check_move_and_copy());

constexpr auto check_in_place_args_one_line() {
  bool made = false;

  static_assert(
      1 ==
      std::tuple_size_v<
          decltype(in_place<Foo>(&made, 37).unsafe_tuple_to_bind())>);

  // Binding prvalues is ok since `Foo` is constructed in the same statement.
  Foo foo = lite_tuple::get<0>(in_place<Foo>(&made, 37).unsafe_tuple_to_bind());
  test(made);
  test(foo.n_ == 37);

  int n = 3;
  Foo f2 = lite_tuple::get<0>(in_place<Foo>(nullptr, n).unsafe_tuple_to_bind());
  ++n;
  test(3 == f2.n_);
  test(4 == n);

  return true;
}

static_assert(check_in_place_args_one_line());
constexpr auto check_in_place_args_step_by_step() {
  bool made = false;

  // These vars can't be prvalues since the `Foo` ctor is delayed.
  bool* made_ptr = &made;
  int n = 37;

  // Not a prvalue due to [[clang::lifetimebound]] on `what_to_bind()`.
  auto b = in_place<Foo>(made_ptr, n);
  static_assert(std::is_same_v<decltype(b), in_place_args<Foo, bool*&, int&>>);
  auto [fooMaker] = std::move(b).unsafe_tuple_to_bind();
  test(!made);

  Foo foo = std::move(fooMaker);
  test(made);
  test(foo.n_ == n);

  return true;
}

static_assert(check_in_place_args_step_by_step());

// NB: These signatures are NOT meant to be user-visible.
constexpr auto check_in_place_args_type_sig() {
  static_assert(
      std::is_same_v<
          decltype(in_place<Foo>(nullptr, 7)),
          in_place_args<Foo, std::nullptr_t, int>>);

  int n = 7;
  static_assert(
      std::is_same_v<
          decltype(in_place<Foo>(nullptr, n)),
          in_place_args<Foo, std::nullptr_t, int&>>);

  // Composes with projection modifiers as expected
  using const_in_place = decltype(constant(in_place<Foo>(nullptr, 7)));
  static_assert(
      std::is_same_v<
          const_in_place,
          constant<in_place_args<Foo, std::nullptr_t, int>>>);
  constexpr bind_info_t const_bi{category_t{}, constness_t::constant};
  static_assert(
      std::is_same_v<
          const_in_place::binding_list_t,
          tag_t<binding_t<const_bi, Foo>>>);

  return true;
}

static_assert(check_in_place_args_type_sig());

constexpr auto check_in_place_args_via_fn() {
  // Test for issues with prvalue lambdas
  Foo f1 = lite_tuple::get<0>(
      in_place_with([]() { return Foo{nullptr, 17}; }).unsafe_tuple_to_bind());
  test(17 == f1.n_);

  auto fn = []() { return Foo{nullptr, 37}; };
  auto b2 = in_place_with(fn);
  static_assert(
      std::is_same_v<decltype(b2), in_place_fn_args<Foo, decltype(fn)>>);
  static_assert(
      std::is_same_v<
          decltype(b2)::binding_list_t,
          tag_t<binding_t<bind_info_t{}, Foo>>>);
  static_assert(
      1 == std::tuple_size_v<decltype(std::move(b2).unsafe_tuple_to_bind())>);
  Foo f2 = lite_tuple::get<0>(std::move(b2).unsafe_tuple_to_bind());
  test(37 == f2.n_);

  struct MoveN : MoveOnly {
    int n_;
  };

  int n1 = 1000, n2 = 300, n3 = 30, n4 = 7;
  auto fn2 = [mn1 = MoveN{.n_ = n1}](int&& i2, int& i3, const int& i4) {
    return mn1.n_ + i2 + i3 + i4;
  };
  auto b3 = in_place_with(
      std::move(fn2), // the contained `MoveN` is noncopyable
      std::move(n2),
      n3,
      std::as_const(n4));
  static_assert(
      std::is_same_v<
          decltype(b3),
          in_place_fn_args<int, decltype(fn2), int, int&, const int&>>);
  static_assert(
      std::is_same_v<
          decltype(b3)::binding_list_t,
          tag_t<binding_t<bind_info_t{}, int>>>);

  return true;
}

static_assert(check_in_place_args_via_fn());

constexpr auto check_in_place_args_modifier_distributive_property() {
  constexpr bind_info_t def_non_const_bi{category_t{}, constness_t::mut};
  constexpr bind_info_t ref_non_const_bi{category_t::ref, constness_t::mut};
  using expected_binding_list = tag_t<
      binding_t<def_non_const_bi, bool&&>,
      binding_t<ref_non_const_bi, double&>,
      binding_t<def_non_const_bi, int>,
      binding_t<ref_non_const_bi, char&&>>;

  double b = 2;
  static_assert(
      std::is_same_v<
          expected_binding_list,
          decltype(mut(true, const_ref(b), in_place<int>(3), by_ref('c')))::
              binding_list_t>);
  static_assert(
      std::is_same_v<
          expected_binding_list,
          decltype(mut(
              mut(true), mut_ref(b), mut(in_place<int>(3)), mut_ref('c')))::
              binding_list_t>);

  return true;
}

static_assert(check_in_place_args_modifier_distributive_property());

constexpr auto check_unsafe_move() {
  int y = 5;
  args one_ref{y};
  args wrapped{unsafe_move_args::from(std::move(one_ref))};

  bool made = false;
  bool* made_ptr = &made;
  auto foo = in_place<Foo>(made_ptr, y);

  args merged1{
      0xdeadbeef,
      unsafe_move_args::from(std::move(wrapped)),
      unsafe_move_args::from(std::move(foo))};
  auto merged2 = unsafe_move_args::from(std::move(merged1));

  test(!made);

  static_assert(
      std::is_same_v<
          decltype(merged2),
          args<
              unsigned int&&, // the now-destroyed ephemeral 0xdeadbeef
              args<int&>, // wrapped ref to `y`
              in_place_args<Foo, bool*&, int&>>>);

  return true;
}

static_assert(check_unsafe_move());

} // namespace folly::bind::detail
