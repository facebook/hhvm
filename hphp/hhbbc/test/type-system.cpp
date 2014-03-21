/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/hhbbc/type-system.h"

#include <gtest/gtest.h>
#include <boost/range/join.hpp>
#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "folly/Lazy.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/array-init.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/index.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_test("test");
const StaticString s_TestClass("TestClass");
const StaticString s_Base("Base");
const StaticString s_A("A");
const StaticString s_AA("AA");
const StaticString s_AB("AB");
const StaticString s_B("B");
const StaticString s_BA("BA");
const StaticString s_BB("BB");
const StaticString s_BAA("BAA");
const StaticString s_IBase("IBase");
const StaticString s_IA("IA");
const StaticString s_IAA("IAA");
const StaticString s_IB("IB");
const StaticString s_NonUnique("NonUnique");
const StaticString s_NonUniqueA("NonUniqueA");
const StaticString s_WaitHandle("WaitHandle");

// A test program so we can actually test things involving object or
// class types.
std::unique_ptr<php::Unit> make_test_unit() {
  assert(SystemLib::s_inited);
  std::string const hhas = R"(
    .main {
      Int 1
      RetC
    }

    # Technically this should be provided by systemlib, but it's the
    # only one we have to make sure the type system can see for unit
    # test purposes, so we can just define it here.  We don't need to
    # give it any of its functions currently.
    .class [abstract unique builtin] WaitHandle {
    }

    .class [interface unique] IBase {
    }

    .class [interface unique] IA implements (IBase) {
    }

    .class [interface unique] IB implements (IBase) {
    }

    .class [interface unique] IAA implements (IA) {
    }

    .class [unique] Base {
      .default_ctor;
    }

    .class [unique] A extends Base implements (IA) {
      .default_ctor;
    }

    .class [no_override unique] AA extends A implements (IAA) {
      .default_ctor;
    }

    .class [no_override unique] AB extends A {
      .default_ctor;
    }

    .class [unique] B extends Base {
      .default_ctor;
    }

    .class [unique] BA extends B {
      .default_ctor;
    }

    .class [no_override unique] BB extends B {
      .default_ctor;
    }

    .class [unique] BAA extends BA {
      .default_ctor;
    }

    .class [unique] TestClass {
      .default_ctor;
    }

    .class NonUnique { .default_ctor; }
    .class NonUnique { .default_ctor; }

    .class NonUniqueA extends NonUnique {
      .default_ctor;
    }
    .class NonUniqueA extends NonUnique {
      .default_ctor;
    }

    .function test() {
      Int 1
      RetC
    }
  )";
  std::unique_ptr<UnitEmitter> ue(assemble_string(
    hhas.c_str(), hhas.size(),
    "ignore.php",
    MD5("12345432123454321234543212345432")
  ));
  return parse_unit(*ue);
}

std::unique_ptr<php::Program> make_program() {
  auto program = folly::make_unique<php::Program>();
  program->units.push_back(make_test_unit());
  return program;
}

//////////////////////////////////////////////////////////////////////

auto const test_empty_array = folly::lazy([&] {
  return HphpArray::GetStaticEmptyArray();
});

auto const test_array_map_value = folly::lazy([&] {
  auto ar = make_map_array(
    s_A.get(), s_B.get(),
    s_test.get(), 12
  );
  return ArrayData::GetScalarArray(ar.get());
});

auto const test_array_packed_value = folly::lazy([&] {
  auto ar = make_packed_array(
    42,
    23,
    12
  );
  return ArrayData::GetScalarArray(ar.get());
});

auto const test_array_packed_value2 = folly::lazy([&] {
  auto ar = make_packed_array(
    42,
    23.0,
    12
  );
  return ArrayData::GetScalarArray(ar.get());
});

auto const test_array_packed_value3 = folly::lazy([&] {
  auto ar = make_packed_array(
    1,
    2,
    3,
    4,
    5
  );
  return ArrayData::GetScalarArray(ar.get());
});

auto const with_data = folly::lazy([&] {
  return std::vector<Type> {
    ival(2),
    dval(2.0),
    sval(s_test.get()),
    aval(test_array_map_value()),
    aval(test_array_packed_value())
  };
});

// In the sense of "non-union type", not the sense of TPrim.
auto const primitives = {
  TUninit,
  TInitNull,
  TFalse,
  TTrue,
  TInt,
  TDbl,
  TSStr,
  TCStr,
  TSArrE,
  TCArrE,
  TSArrN,
  TCArrN,
  TObj,
  TRes,
  TCls,
  TRef
};

auto const optionals = {
  TOptTrue,
  TOptFalse,
  TOptBool,
  TOptInt,
  TOptDbl,
  TOptNum,
  TOptSStr,
  TOptCStr,
  TOptStr,
  TOptSArrE,
  TOptCArrE,
  TOptSArrN,
  TOptCArrN,
  TOptSArr,
  TOptCArr,
  TOptArr,
  TOptObj,
  TOptRes
};

auto const non_opt_unions = {
  TInitCell,
  TCell,
  TInitGen,
  TGen,
  TNull,
  TBool,
  TNum,
  TStr,
  TArrE,
  TArrN,
  TSArr,
  TCArr,
  TArr,
  TInitPrim,
  TPrim,
  TInitUnc,
  TUnc,
  TTop,
};

auto const all_unions = boost::join(optionals, non_opt_unions);

auto const all = folly::lazy([&] {
  std::vector<Type> ret;
  auto const wdata = with_data();
  ret.insert(end(ret), begin(primitives), end(primitives));
  ret.insert(end(ret), begin(all_unions), end(all_unions));
  ret.insert(end(ret), begin(wdata), end(wdata));
  return ret;
});

template<class Range>
std::vector<Type> wait_handles_of(const Index& index, const Range& r) {
  std::vector<Type> ret;
  for (auto& t : r) ret.push_back(wait_handle(index, t));
  return ret;
}

std::vector<Type> all_with_waithandles(const Index& index) {
  auto ret = wait_handles_of(index, all());
  for (auto& t : all()) ret.push_back(t);
  return ret;
}

auto const specialized_array_examples = folly::lazy([&] {
  auto ret = std::vector<Type>{};

  auto test_map_a          = StructMap{};
  test_map_a[s_test.get()] = ival(2);
  ret.emplace_back(sarr_struct(test_map_a));

  auto test_map_b          = StructMap{};
  test_map_b[s_test.get()] = TInt;
  ret.emplace_back(sarr_struct(test_map_b));

  auto test_map_c          = StructMap{};
  test_map_c[s_A.get()]    = TInt;
  ret.emplace_back(sarr_struct(test_map_c));

  auto test_map_d          = StructMap{};
  test_map_d[s_A.get()]    = TInt;
  test_map_d[s_test.get()] = TDbl;
  ret.emplace_back(sarr_struct(test_map_d));

  ret.emplace_back(arr_packedn(TInt));
  ret.emplace_back(arr_mapn(TSStr, arr_mapn(TInt, TSStr)));
  ret.emplace_back(arr_mapn(TSStr, TArr));
  ret.emplace_back(arr_mapn(TSStr, arr_packedn(TSStr)));
  ret.emplace_back(arr_mapn(TSStr, arr_mapn(TSStr, TSStr)));

  return ret;
});

//////////////////////////////////////////////////////////////////////

}

TEST(Type, Top) {
  auto const program = make_program();
  Index index { borrow(program) };

  // Everything is a subtype of Top, couldBe Top, and the union of Top
  // with anything is Top.
  for (auto& t : all_with_waithandles(index)) {
    EXPECT_TRUE(t.subtypeOf(TTop));
    EXPECT_TRUE(t.couldBe(TTop));
    EXPECT_TRUE(union_of(t, TTop) == TTop);
    EXPECT_TRUE(union_of(TTop, t) == TTop);
  }
}

TEST(Type, Bottom) {
  auto const program = make_program();
  Index index { borrow(program) };

  // Bottom is a subtype of everything, nothing couldBe Bottom, and
  // the union_of anything with Bottom is itself.
  for (auto& t : all_with_waithandles(index)) {
    EXPECT_TRUE(TBottom.subtypeOf(t));
    EXPECT_TRUE(!TBottom.couldBe(t));
    EXPECT_TRUE(union_of(t, TBottom) == t);
    EXPECT_TRUE(union_of(TBottom, t) == t);
  }
}

TEST(Type, Prims) {
  auto const program = make_program();
  Index index { borrow(program) };

  // All pairs of non-equivalent primitives are not related by either
  // subtypeOf or couldBe, including if you wrap them in wait handles.
  for (auto& t1 : primitives) {
    for (auto& t2 : primitives) {
      if (t1 != t2) {
        EXPECT_TRUE(!t1.subtypeOf(t2) && !t2.subtypeOf(t1));
        EXPECT_TRUE(!t1.couldBe(t2));
        EXPECT_TRUE(!t2.couldBe(t1));

        auto const w1 = wait_handle(index, t1);
        auto const w2 = wait_handle(index, t2);
        EXPECT_TRUE(!w1.subtypeOf(w2) && !w2.subtypeOf(w1));
        EXPECT_TRUE(!w1.couldBe(w2));
        EXPECT_TRUE(!w2.couldBe(w1));
      }
    }
  }
}

TEST(Type, Relations) {
  auto const program = make_program();
  Index index { borrow(program) };

  // couldBe is symmetric and reflexive
  for (auto& t1 : all_with_waithandles(index)) {
    for (auto& t2 : all()) {
      EXPECT_TRUE(t1.couldBe(t2) == t2.couldBe(t1));
    }
  }
  for (auto& t1 : all_with_waithandles(index)) {
    EXPECT_TRUE(t1.couldBe(t1));
  }

  // subtype is antisymmetric and reflexive
  for (auto& t1 : all_with_waithandles(index)) {
    for (auto& t2 : all_with_waithandles(index)) {
      if (t1 != t2) {
        EXPECT_TRUE(!(t1.subtypeOf(t2) && t2.subtypeOf(t1)));
      }
    }
  }
  for (auto& t1 : all_with_waithandles(index)) {
    EXPECT_TRUE(t1.subtypeOf(t1));
  }

  // union_of is commutative
  for (auto& t1 : all_with_waithandles(index)) {
    for (auto& t2 : all_with_waithandles(index)) {
      EXPECT_TRUE(union_of(t1, t2) == union_of(t2, t1))
        << "   " << show(t1) << ' ' << show(t2)
        << "\n   union_of(t1, t2): " << show(union_of(t1, t2))
        << "\n   union_of(t2, t1): " << show(union_of(t2, t1));
    }
  }
}

TEST(Type, Prim) {
  auto subtype_true = std::initializer_list<std::pair<Type,Type>> {
    { TInt,      TPrim },
    { TBool,     TPrim },
    { TNum,      TPrim },
    { TInitNull, TPrim },
    { TDbl,      TPrim },
    { dval(0.0), TPrim },
    { ival(0),   TPrim },
    { TNull,     TPrim },
    { TInt,      TInitPrim },
    { TBool,     TInitPrim },
    { TNum,      TInitPrim },
    { TInitNull, TInitPrim },
    { TDbl,      TInitPrim },
    { dval(0.0), TInitPrim },
    { ival(0),   TInitPrim },
  };

  auto subtype_false = std::initializer_list<std::pair<Type,Type>> {
    { sval(s_test.get()), TPrim },
    { TSStr, TPrim },
    { TSArr, TPrim },
    { TNull, TInitPrim }, // TNull could be uninit
    { TPrim, TBool },
    { TPrim, TInt },
    { TPrim, TNum },
    { TInitPrim, TNum },
    { TUnc, TPrim },
    { TUnc, TInitPrim },
    { TInitUnc, TPrim },
    { TSStr, TInitPrim },
    { TArr, TInitPrim },
    { TSArr, TPrim },
    { TPrim, dval(0.0) },
  };

  auto couldbe_true = std::initializer_list<std::pair<Type,Type>> {
    { TPrim, TInt },
    { TPrim, TBool },
    { TPrim, TNum },
    { TInitPrim, TNum },
    { TInitPrim, TFalse },
    { TPrim, TCell },
    { TPrim, TInt },
    { TPrim, TInt },
    { TPrim, TOptObj },
    { TPrim, TOptFalse },
  };

  auto couldbe_false = std::initializer_list<std::pair<Type,Type>> {
    { TPrim, TSStr },
    { TInitPrim, TSStr },
    { TInitPrim, sval(s_test.get()) },
    { TPrim, sval(s_test.get()) },
    { TInitPrim, TUninit },
    { TPrim, TRef },
    { TPrim, TObj },
  };

  for (auto kv : subtype_true) {
    EXPECT_TRUE(kv.first.subtypeOf(kv.second))
      << show(kv.first) << " subtypeOf " << show(kv.second);
  }
  for (auto kv : subtype_false) {
    EXPECT_FALSE(kv.first.subtypeOf(kv.second))
      << show(kv.first) << " !subtypeOf " << show(kv.second);
  }
  for (auto kv : couldbe_true) {
    EXPECT_TRUE(kv.first.couldBe(kv.second))
      << show(kv.first) << " couldbe " << show(kv.second);
    EXPECT_TRUE(kv.second.couldBe(kv.first))
      << show(kv.first) << " couldbe " << show(kv.second);
  }
  for (auto kv : couldbe_false) {
    EXPECT_TRUE(!kv.first.couldBe(kv.second))
      << show(kv.first) << " !couldbe " << show(kv.second);
    EXPECT_TRUE(!kv.second.couldBe(kv.first))
      << show(kv.first) << " !couldbe " << show(kv.second);
  }
}

TEST(Type, CouldBeValues) {
  EXPECT_FALSE(ival(2).couldBe(ival(3)));
  EXPECT_TRUE(ival(2).couldBe(ival(2)));
  EXPECT_FALSE(aval(test_array_packed_value()).couldBe(
    aval(test_array_map_value())));
  EXPECT_TRUE(aval(test_array_packed_value()).couldBe(
    aval(test_array_packed_value())));
  EXPECT_TRUE(dval(2.0).couldBe(dval(2.0)));
  EXPECT_FALSE(dval(2.0).couldBe(dval(3.0)));
  EXPECT_FALSE(sval(s_test.get()).couldBe(sval(s_A.get())));
  EXPECT_TRUE(sval(s_test.get()).couldBe(sval(s_test.get())));
}

TEST(Type, Unc) {
  EXPECT_TRUE(TInt.subtypeOf(TInitUnc));
  EXPECT_TRUE(TInt.subtypeOf(TUnc));
  EXPECT_TRUE(TDbl.subtypeOf(TInitUnc));
  EXPECT_TRUE(TDbl.subtypeOf(TUnc));
  EXPECT_TRUE(dval(3.0).subtypeOf(TInitUnc));

  auto pairs = std::initializer_list<std::pair<Type,Type>> {
    { TUnc, TInitUnc },
    { TUnc, TInitCell },
    { TUnc, TCell },
    { TInitUnc, TInt },
    { TInitUnc, TOptInt },
    { TInitUnc, opt(ival(2)) },
    { TUnc, TInt },
    { TUnc, TOptInt },
    { TUnc, opt(ival(2)) },
    { TNum, TUnc },
    { TNum, TInitUnc },
  };
  for (auto kv : pairs) {
    EXPECT_TRUE(kv.first.couldBe(kv.second))
      << show(kv.first) << " couldBe " << show(kv.second);
  }
}

TEST(Type, DblNan) {
  auto const qnan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_TRUE(dval(qnan).subtypeOf(dval(qnan)));
  EXPECT_TRUE(dval(qnan).couldBe(dval(qnan)));
  EXPECT_TRUE(dval(qnan) == dval(qnan));
}

TEST(Type, Option) {
  auto const program = make_program();
  Index index { borrow(program) };

  EXPECT_TRUE(TTrue.subtypeOf(TOptTrue));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptTrue));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptTrue));

  EXPECT_TRUE(TFalse.subtypeOf(TOptFalse));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptFalse));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptFalse));

  EXPECT_TRUE(TFalse.subtypeOf(TOptBool));
  EXPECT_TRUE(TTrue.subtypeOf(TOptBool));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptBool));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptBool));

  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(TInt.subtypeOf(TOptInt));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptInt));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptInt));

  EXPECT_TRUE(TDbl.subtypeOf(TOptDbl));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptDbl));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptDbl));
  EXPECT_TRUE(dval(3.0).subtypeOf(TOptDbl));

  EXPECT_TRUE(sval(s_test.get()).subtypeOf(TOptSStr));
  EXPECT_TRUE(TSStr.subtypeOf(TOptSStr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptSStr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptSStr));
  EXPECT_TRUE(!TStr.subtypeOf(TOptSStr));
  EXPECT_TRUE(TStr.couldBe(TOptSStr));

  EXPECT_TRUE(TCStr.subtypeOf(TOptStr));
  EXPECT_TRUE(TCArr.subtypeOf(TOptArr));

  EXPECT_TRUE(TStr.subtypeOf(TOptStr));
  EXPECT_TRUE(TSStr.subtypeOf(TOptStr));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(TOptStr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptStr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptStr));

  EXPECT_TRUE(TSArr.subtypeOf(TOptSArr));
  EXPECT_TRUE(!TArr.subtypeOf(TOptSArr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptSArr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptSArr));

  EXPECT_TRUE(TArr.subtypeOf(TOptArr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptArr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptArr));

  EXPECT_TRUE(TObj.subtypeOf(TOptObj));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptObj));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptObj));

  EXPECT_TRUE(TRes.subtypeOf(TOptRes));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptRes));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptRes));

  for (auto& t : optionals) EXPECT_EQ(t, opt(unopt(t)));
  for (auto& t : optionals) EXPECT_TRUE(is_opt(t));
  for (auto& t : all()) {
    auto const found =
      std::find(begin(optionals), end(optionals), t) != end(optionals);
    EXPECT_EQ(found, is_opt(t));
  }

  EXPECT_TRUE(is_opt(opt(sval(s_test.get()))));
  EXPECT_TRUE(is_opt(opt(ival(2))));
  EXPECT_TRUE(is_opt(opt(dval(2.0))));

  EXPECT_FALSE(is_opt(sval(s_test.get())));
  EXPECT_FALSE(is_opt(ival(2)));
  EXPECT_FALSE(is_opt(dval(2.0)));

  EXPECT_TRUE(wait_handle(index, opt(dval(2.0))).couldBe(
    wait_handle(index, dval(2.0))));
}

TEST(Type, Num) {
  EXPECT_EQ(union_of(TInt, TDbl), TNum);
  EXPECT_EQ(union_of(ival(2), dval(1.0)), TNum);
  EXPECT_EQ(union_of(TInt, dval(1.0)), TNum);
}

TEST(Type, OptUnionOf) {
  EXPECT_EQ(opt(ival(2)), union_of(ival(2), TInitNull));
  EXPECT_EQ(opt(dval(2.0)), union_of(TInitNull, dval(2.0)));
  EXPECT_EQ(opt(sval(s_test.get())), union_of(sval(s_test.get()), TInitNull));
  EXPECT_EQ(opt(sval(s_test.get())), union_of(TInitNull, sval(s_test.get())));

  EXPECT_EQ(TOptBool, union_of(TOptFalse, TOptTrue));
  EXPECT_EQ(TOptBool, union_of(TOptTrue, TOptFalse));

  EXPECT_EQ(TOptCArr, union_of(TCArr, TInitNull));
  EXPECT_EQ(TOptCArr, union_of(TInitNull, TCArr));
  EXPECT_EQ(TOptSArr, union_of(TInitNull, TOptSArr));
  EXPECT_EQ(TOptSArr, union_of(TOptSArr, TInitNull));
  EXPECT_EQ(TOptArr, union_of(TOptArr, TInitNull));
  EXPECT_EQ(TOptArr, union_of(TInitNull, TOptArr));

  EXPECT_EQ(TInitUnc, union_of(TOptSArr, TSStr));
  EXPECT_EQ(TInitUnc, union_of(TSStr, TOptSArr));

  EXPECT_EQ(TOptNum, union_of(TInitNull, TNum));
  EXPECT_EQ(TOptNum, union_of(TInitNull, union_of(dval(1), ival(0))));
}

TEST(Type, OptTV) {
  EXPECT_TRUE(!tv(opt(ival(2))));
  EXPECT_TRUE(!tv(opt(sval(s_test.get()))));
  EXPECT_TRUE(!tv(opt(dval(2.0))));
  EXPECT_TRUE(!tv(TOptFalse));
  EXPECT_TRUE(!tv(TOptTrue));
  for (auto& x : optionals) {
    EXPECT_TRUE(!tv(x));
  }
}

TEST(Type, OptCouldBe) {
  for (auto& x : optionals) EXPECT_TRUE(x.couldBe(unopt(x)));

  auto true_cases = std::initializer_list<std::pair<Type,Type>> {
    { opt(sval(s_test.get())), TStr },
    { opt(sval(s_test.get())), TInitNull },
    { opt(sval(s_test.get())), TSStr },
    { opt(sval(s_test.get())), sval(s_test.get()) },

    { opt(ival(2)), TInt },
    { opt(ival(2)), TInitNull },
    { opt(ival(2)), ival(2) },

    { opt(dval(2.0)), TDbl },
    { opt(dval(2.0)), TInitNull },
    { opt(dval(2.0)), dval(2) },

    { opt(TFalse), TBool },
    { opt(TFalse), TFalse },

    { opt(TTrue), TBool },
    { opt(TTrue), TTrue },

    { opt(TDbl), opt(TNum) },
    { TDbl, opt(TNum) },
    { TNum, opt(TDbl) },

    { opt(TInt), TNum },
    { TInt, opt(TNum) },
    { opt(TDbl), TNum },
  };

  auto false_cases = std::initializer_list<std::pair<Type,Type>> {
    { opt(sval(s_test.get())), TCStr },
    { opt(ival(2)), TDbl },
    { opt(dval(2.0)), TInt },
    { opt(TFalse), TTrue },
    { opt(TTrue), TFalse },
    { TFalse, opt(TNum) },
  };

  for (auto kv : true_cases) {
    EXPECT_TRUE(kv.first.couldBe(kv.second))
      << show(kv.first) << " couldBe " << show(kv.second)
      << " should be true";
  }
  for (auto kv : false_cases) {
    EXPECT_TRUE(!kv.first.couldBe(kv.second))
      << show(kv.first) << " couldBe " << show(kv.second)
      << " should be false";
  }
  for (auto kv : boost::join(true_cases, false_cases)) {
    EXPECT_EQ(kv.first.couldBe(kv.second), kv.second.couldBe(kv.first))
      << show(kv.first) << " couldBe " << show(kv.second)
      << " wasn't reflexive";
  }

  for (auto& x : optionals) {
    EXPECT_TRUE(x.couldBe(unopt(x)));
    EXPECT_TRUE(x.couldBe(TInitNull));
    EXPECT_TRUE(!x.couldBe(TUninit));
    for (auto& y : optionals) {
      EXPECT_TRUE(x.couldBe(y));
    }
  }
}

TEST(Type, SpecificExamples) {
  // Random examples to stress option types, values, etc:

  EXPECT_TRUE(!TInt.subtypeOf(ival(1)));

  EXPECT_TRUE(TInitCell.couldBe(ival(1)));
  EXPECT_TRUE(TInitCell.subtypeOf(TGen));
  EXPECT_TRUE(ival(2).subtypeOf(TInt));
  EXPECT_TRUE(!ival(2).subtypeOf(TBool));
  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(TInt.subtypeOf(TOptInt));
  EXPECT_TRUE(!TBool.subtypeOf(TOptInt));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptInt));
  EXPECT_TRUE(!TNull.subtypeOf(TOptInt));
  EXPECT_TRUE(TNull.couldBe(TOptInt));
  EXPECT_TRUE(TNull.couldBe(TOptBool));

  EXPECT_TRUE(TInitNull.subtypeOf(TInitCell));
  EXPECT_TRUE(TInitNull.subtypeOf(TCell));
  EXPECT_TRUE(!TUninit.subtypeOf(TInitNull));

  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(ival(3).subtypeOf(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(TInt));
  EXPECT_TRUE(TInitNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TInitNull.subtypeOf(opt(ival(3))));
  EXPECT_TRUE(!TNull.subtypeOf(opt(ival(3))));

  EXPECT_EQ(TStr, union_of(sval(s_test.get()), TCStr));
  EXPECT_EQ(TStr, union_of(TCStr, sval(s_test.get())));

  EXPECT_EQ(TGen, union_of(TRef, TUninit));
}

TEST(Type, IndexBased) {
  auto const program = make_program();
  auto const unit = borrow(program->units.back());
  auto const func = [&]() -> borrowed_ptr<php::Func> {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return borrow(f);
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{borrow(program)};

  auto const cls = idx.resolve_class(ctx, s_TestClass.get());
  if (!cls) EXPECT_TRUE(false);
  auto const clsBase = idx.resolve_class(ctx, s_Base.get());
  if (!clsBase) EXPECT_TRUE(false);

  auto const objExactTy = objExact(*cls);
  auto const subObjTy   = subObj(*cls);
  auto const clsExactTy = clsExact(*cls);
  auto const subClsTy   = subCls(*cls);
  auto const objExactBaseTy = objExact(*clsBase);
  auto const subObjBaseTy   = subObj(*clsBase);

  // Basic relationship between the class types and object types.
  EXPECT_EQ(objcls(objExactTy), clsExactTy);
  EXPECT_EQ(objcls(subObjTy), subClsTy);

  // =TestClass <: <=TestClass, and not vice versa.
  EXPECT_TRUE(objExactTy.subtypeOf(subObjTy));
  EXPECT_TRUE(!subObjTy.subtypeOf(objExactTy));
  // =TestClass <: <=TestClass, and not vice versa.
  EXPECT_TRUE(clsExactTy.subtypeOf(subClsTy));
  EXPECT_TRUE(!subClsTy.subtypeOf(clsExactTy));

  // =TestClass couldBe <= TestClass, and vice versa.
  EXPECT_TRUE(objExactTy.couldBe(subObjTy));
  EXPECT_TRUE(subObjTy.couldBe(objExactTy));
  EXPECT_TRUE(clsExactTy.couldBe(subClsTy));
  EXPECT_TRUE(subClsTy.couldBe(clsExactTy));

  // Foo= and Foo<= are both subtypes of Foo, and couldBe Foo.
  EXPECT_TRUE(objExactTy.subtypeOf(TObj));
  EXPECT_TRUE(subObjTy.subtypeOf(TObj));
  EXPECT_TRUE(objExactTy.couldBe(TObj));
  EXPECT_TRUE(subObjTy.couldBe(TObj));
  EXPECT_TRUE(TObj.couldBe(objExactTy));
  EXPECT_TRUE(TObj.couldBe(subObjTy));
  EXPECT_TRUE(clsExactTy.subtypeOf(TCls));
  EXPECT_TRUE(subClsTy.subtypeOf(TCls));
  EXPECT_TRUE(clsExactTy.couldBe(TCls));
  EXPECT_TRUE(subClsTy.couldBe(TCls));
  EXPECT_TRUE(TCls.couldBe(clsExactTy));
  EXPECT_TRUE(TCls.couldBe(subClsTy));

  // Obj= and Obj<= both couldBe ?Obj, and vice versa.
  EXPECT_TRUE(objExactTy.couldBe(TOptObj));
  EXPECT_TRUE(subObjTy.couldBe(TOptObj));
  EXPECT_TRUE(TOptObj.couldBe(objExactTy));
  EXPECT_TRUE(TOptObj.couldBe(subObjTy));

  // Obj= and Obj<= are subtypes of ?Obj.
  EXPECT_TRUE(objExactTy.subtypeOf(TOptObj));
  EXPECT_TRUE(subObjTy.subtypeOf(TOptObj));

  // Obj= is a subtype of ?Obj=, and also ?Obj<=.
  EXPECT_TRUE(objExactTy.subtypeOf(opt(objExactTy)));
  EXPECT_TRUE(objExactTy.subtypeOf(opt(subObjTy)));
  EXPECT_TRUE(!opt(objExactTy).subtypeOf(objExactTy));
  EXPECT_TRUE(!opt(subObjTy).subtypeOf(objExactTy));

  // Obj= couldBe ?Obj= and ?Obj<=, and vice versa.
  EXPECT_TRUE(objExactTy.couldBe(opt(objExactTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(objExactTy));
  EXPECT_TRUE(objExactTy.couldBe(opt(subObjTy)));
  EXPECT_TRUE(opt(subObjTy).couldBe(objExactTy));

  // Obj<= is not a subtype of ?Obj=, it is overlapping but
  // potentially contains other types.  (We might eventually check
  // whether objects are final as part of this, but not right now.)
  EXPECT_TRUE(!subObjTy.subtypeOf(opt(objExactTy)));
  EXPECT_TRUE(!opt(objExactTy).subtypeOf(subObjTy));

  // Obj<= couldBe ?Obj= and vice versa.
  EXPECT_TRUE(subObjTy.couldBe(opt(objExactTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(subObjTy));

  // ?Obj<=, ?Obj=, ?Foo<= and ?Foo= couldBe each other
  EXPECT_TRUE(opt(subObjTy).couldBe(opt(objExactBaseTy)));
  EXPECT_TRUE(opt(objExactBaseTy).couldBe(opt(subObjTy)));
  EXPECT_TRUE(opt(subObjTy).couldBe(opt(subObjBaseTy)));
  EXPECT_TRUE(opt(subObjBaseTy).couldBe(opt(subObjTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(opt(objExactBaseTy)));
  EXPECT_TRUE(opt(objExactBaseTy).couldBe(opt(objExactTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(opt(subObjBaseTy)));
  EXPECT_TRUE(opt(subObjBaseTy).couldBe(opt(objExactTy)));
}

TEST(Type, Hierarchies) {
  auto const program = make_program();
  auto const unit = borrow(program->units.back());
  auto const func = [&]() -> borrowed_ptr<php::Func> {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return borrow(f);
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{borrow(program)};

  // load classes in hierarchy
  auto const clsBase = idx.resolve_class(ctx, s_Base.get());
  if (!clsBase) EXPECT_TRUE(false);
  auto const clsA = idx.resolve_class(ctx, s_A.get());
  if (!clsA) EXPECT_TRUE(false);
  auto const clsB = idx.resolve_class(ctx, s_B.get());
  if (!clsB) EXPECT_TRUE(false);
  auto const clsAA = idx.resolve_class(ctx, s_AA.get());
  if (!clsAA) EXPECT_TRUE(false);
  auto const clsAB = idx.resolve_class(ctx, s_AB.get());
  if (!clsAB) EXPECT_TRUE(false);
  auto const clsBA = idx.resolve_class(ctx, s_BA.get());
  if (!clsBA) EXPECT_TRUE(false);
  auto const clsBB = idx.resolve_class(ctx, s_BB.get());
  if (!clsBB) EXPECT_TRUE(false);
  auto const clsBAA = idx.resolve_class(ctx, s_BAA.get());
  if (!clsBAA) EXPECT_TRUE(false);
  auto const clsTestClass = idx.resolve_class(ctx, s_TestClass.get());
  if (!clsTestClass) EXPECT_TRUE(false);
  auto const clsNonUnique = idx.resolve_class(ctx, s_NonUnique.get());
  if (!clsNonUnique) EXPECT_TRUE(false);

  // make *exact type* and *sub type* types and objects for all loaded classes
  auto const objExactBaseTy = objExact(*clsBase);
  auto const subObjBaseTy   = subObj(*clsBase);
  auto const clsExactBaseTy = clsExact(*clsBase);
  auto const subClsBaseTy   = subCls(*clsBase);

  auto const objExactATy    = objExact(*clsA);
  auto const subObjATy      = subObj(*clsA);
  auto const clsExactATy    = clsExact(*clsA);
  auto const subClsATy      = subCls(*clsA);

  auto const objExactAATy    = objExact(*clsAA);
  auto const subObjAATy      = subObj(*clsAA);
  auto const clsExactAATy    = clsExact(*clsAA);
  auto const subClsAATy      = subCls(*clsAA);

  auto const objExactABTy    = objExact(*clsAB);
  auto const subObjABTy      = subObj(*clsAB);
  auto const clsExactABTy    = clsExact(*clsAB);
  auto const subClsABTy      = subCls(*clsAB);

  auto const objExactBTy    = objExact(*clsB);
  auto const subObjBTy      = subObj(*clsB);
  auto const clsExactBTy    = clsExact(*clsB);
  auto const subClsBTy      = subCls(*clsB);

  auto const objExactBATy    = objExact(*clsBA);
  auto const subObjBATy      = subObj(*clsBA);
  auto const clsExactBATy    = clsExact(*clsBA);
  auto const subClsBATy      = subCls(*clsBA);

  auto const objExactBBTy    = objExact(*clsBB);
  auto const subObjBBTy      = subObj(*clsBB);
  auto const clsExactBBTy    = clsExact(*clsBB);
  auto const subClsBBTy      = subCls(*clsBB);

  auto const objExactBAATy    = objExact(*clsBAA);
  auto const subObjBAATy      = subObj(*clsBAA);
  auto const clsExactBAATy    = clsExact(*clsBAA);
  auto const subClsBAATy      = subCls(*clsBAA);

  auto const objExactTestClassTy = objExact(*clsTestClass);
  auto const subObjTestClassTy   = subObj(*clsTestClass);
  auto const clsExactTestClassTy = clsExact(*clsTestClass);
  auto const subClsTestClassTy   = subCls(*clsTestClass);

  auto const objExactNonUniqueTy = objExact(*clsNonUnique);
  auto const subObjNonUniqueTy   = subObj(*clsNonUnique);
  auto const clsExactNonUniqueTy = clsExact(*clsNonUnique);
  auto const subClsNonUniqueTy   = subCls(*clsNonUnique);

  // check that type from object and type are the same (obnoxious test)
  EXPECT_EQ(objcls(objExactBaseTy), clsExactBaseTy);
  EXPECT_EQ(objcls(subObjBaseTy), subClsBaseTy);
  EXPECT_EQ(objcls(objExactATy), clsExactATy);
  EXPECT_EQ(objcls(subObjATy), subClsATy);
  EXPECT_EQ(objcls(objExactAATy), clsExactAATy);
  EXPECT_EQ(objcls(subObjAATy), subClsAATy);
  EXPECT_EQ(objcls(objExactABTy), clsExactABTy);
  EXPECT_EQ(objcls(subObjABTy), subClsABTy);
  EXPECT_EQ(objcls(objExactBTy), clsExactBTy);
  EXPECT_EQ(objcls(subObjBTy), subClsBTy);
  EXPECT_EQ(objcls(objExactBATy), clsExactBATy);
  EXPECT_EQ(objcls(subObjBATy), subClsBATy);
  EXPECT_EQ(objcls(objExactBBTy), clsExactBBTy);
  EXPECT_EQ(objcls(subObjBBTy), subClsBBTy);
  EXPECT_EQ(objcls(objExactBAATy), clsExactBAATy);
  EXPECT_EQ(objcls(subObjBAATy), subClsBAATy);

  // both subobj(A) and subcls(A) of no_override class A change to exact types
  EXPECT_EQ(objcls(objExactABTy), subClsABTy);
  EXPECT_EQ(objcls(subObjABTy), clsExactABTy);

  // a T= is a subtype of itself but not a strict subtype
  // also a T= is in a "could be" relationship with itself.
  EXPECT_TRUE(objcls(objExactBaseTy).subtypeOf(clsExactBaseTy));
  EXPECT_FALSE(objcls(objExactBaseTy).strictSubtypeOf(objcls(objExactBaseTy)));
  EXPECT_TRUE(objcls(objExactBAATy).subtypeOf(clsExactBAATy));
  EXPECT_FALSE(clsExactBAATy.strictSubtypeOf(objcls(objExactBAATy)));
  EXPECT_TRUE(clsExactBAATy.couldBe(clsExactBAATy));

  // Given the hierarchy A <- B <- C where A is the base then:
  // B= is not in any subtype relationshipt with a A= or C=.
  // Neither they are in "could be" relationships.
  // Overall T= sets are always disjoint.
  EXPECT_FALSE(objcls(objExactBATy).subtypeOf(clsExactBaseTy));
  EXPECT_FALSE(objcls(objExactBATy).subtypeOf(clsExactBTy));
  EXPECT_FALSE(objcls(objExactBATy).subtypeOf(clsExactBAATy));
  EXPECT_FALSE(clsExactBATy.strictSubtypeOf(objcls(objExactBaseTy)));
  EXPECT_FALSE(clsExactBATy.strictSubtypeOf(objcls(objExactBTy)));
  EXPECT_FALSE(clsExactBATy.strictSubtypeOf(objcls(objExactBAATy)));
  EXPECT_FALSE(clsExactBATy.couldBe(objcls(objExactBaseTy)));
  EXPECT_FALSE(objcls(objExactBATy).couldBe(clsExactBTy));
  EXPECT_FALSE(clsExactBATy.couldBe(objcls(objExactBAATy)));

  // any T= is both a subtype and strict subtype of T<=.
  // Given the hierarchy A <- B <- C where A is the base then:
  // C= is a subtype and a strict subtype of B<=, ?B<=, A<= and ?A<=.
  // The "could be" relationship also holds.
  EXPECT_TRUE(objcls(objExactATy).subtypeOf(subClsATy));
  EXPECT_TRUE(objcls(objExactBAATy).subtypeOf(subClsBaseTy));
  EXPECT_TRUE(objExactBAATy.subtypeOf(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(objExactBAATy).subtypeOf(subClsBTy));
  EXPECT_TRUE(objExactBAATy.subtypeOf(opt(subObjBTy)));
  EXPECT_TRUE(clsExactBAATy.subtypeOf(objcls(subObjBATy)));
  EXPECT_TRUE(objExactBAATy.subtypeOf(opt(subObjBATy)));
  EXPECT_TRUE(clsExactBAATy.subtypeOf(objcls(subObjBAATy)));
  EXPECT_TRUE(objExactBAATy.subtypeOf(opt(subObjBAATy)));
  EXPECT_TRUE(objcls(objExactATy).strictSubtypeOf(subClsATy));
  EXPECT_TRUE(objcls(objExactBAATy).strictSubtypeOf(subClsBaseTy));
  EXPECT_TRUE(objExactBAATy.strictSubtypeOf(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(objExactBAATy).strictSubtypeOf(subClsBTy));
  EXPECT_TRUE(objExactBAATy.strictSubtypeOf(opt(subObjBTy)));
  EXPECT_TRUE(clsExactBAATy.strictSubtypeOf(objcls(subObjBATy)));
  EXPECT_TRUE(objExactBAATy.strictSubtypeOf(opt(subObjBATy)));
  EXPECT_TRUE(clsExactBAATy.strictSubtypeOf(objcls(subObjBAATy)));
  EXPECT_TRUE(objExactBAATy.strictSubtypeOf(opt(subObjBAATy)));
  EXPECT_TRUE(objcls(objExactATy).couldBe(subClsATy));
  EXPECT_TRUE(objcls(objExactBAATy).couldBe(subClsBaseTy));
  EXPECT_TRUE(objExactBAATy.couldBe(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(objExactBAATy).couldBe(subClsBTy));
  EXPECT_TRUE(objExactBAATy.couldBe(opt(subObjBTy)));
  EXPECT_TRUE(clsExactBAATy.couldBe(objcls(subObjBATy)));
  EXPECT_TRUE(objExactBAATy.couldBe(opt(subObjBATy)));
  EXPECT_TRUE(clsExactBAATy.couldBe(objcls(subObjBAATy)));
  EXPECT_TRUE(objExactBAATy.couldBe(opt(subObjBAATy)));

  // a T<= is a subtype of itself but not a strict subtype
  // also a T<= is in a "could be" relationship with itself
  EXPECT_TRUE(objcls(subObjBaseTy).subtypeOf(subClsBaseTy));
  EXPECT_FALSE(objcls(subObjBaseTy).strictSubtypeOf(objcls(subObjBaseTy)));
  EXPECT_TRUE(objcls(subObjBAATy).subtypeOf(subClsBAATy));
  EXPECT_FALSE(subClsBAATy.strictSubtypeOf(objcls(subObjBAATy)));
  EXPECT_TRUE(subClsBAATy.couldBe(subClsBAATy));

  // a T<= type is in no subtype relationship with T=.
  // However a T<= is in a "could be" relationship with T=.
  EXPECT_FALSE(objcls(subObjATy).subtypeOf(clsExactATy));
  EXPECT_FALSE(objcls(subObjATy).strictSubtypeOf(clsExactATy));
  EXPECT_TRUE(clsExactATy.couldBe(objcls(subObjATy)));

  // Given 2 types A and B in no inheritance relationship then
  // A<= and B<= are in no subtype or "could be" relationship.
  // Same if one of the 2 types is an optional type
  EXPECT_FALSE(objcls(subObjATy).subtypeOf(clsExactBTy));
  EXPECT_FALSE(objcls(subObjATy).strictSubtypeOf(clsExactBTy));
  EXPECT_FALSE(subObjATy.subtypeOf(opt(objExactBTy)));
  EXPECT_FALSE(subObjATy.strictSubtypeOf(opt(objExactBTy)));
  EXPECT_FALSE(clsExactATy.couldBe(objcls(subObjBTy)));
  EXPECT_FALSE(objExactATy.couldBe(opt(subObjBTy)));
  EXPECT_FALSE(objcls(subObjBTy).subtypeOf(clsExactATy));
  EXPECT_FALSE(subObjBTy.subtypeOf(opt(objExactATy)));
  EXPECT_FALSE(objcls(subObjBTy).strictSubtypeOf(clsExactATy));
  EXPECT_FALSE(subObjBTy.strictSubtypeOf(opt(objExactATy)));
  EXPECT_FALSE(clsExactBTy.couldBe(objcls(subObjATy)));
  EXPECT_FALSE(objExactBTy.couldBe(opt(subObjATy)));

  // Given the hierarchy A <- B <- C where A is the base then:
  // C<= is a subtype and a strict subtype of B<=, ?B<=, A<= and ?A<=.
  // It is also in a "could be" relationship with all its ancestors
  // (including optional)
  EXPECT_TRUE(objcls(subObjBAATy).subtypeOf(subClsBaseTy));
  EXPECT_TRUE(subObjBAATy.subtypeOf(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(subObjBAATy).subtypeOf(subClsBTy));
  EXPECT_TRUE(subObjBAATy.subtypeOf(opt(subObjBTy)));
  EXPECT_TRUE(subClsBAATy.subtypeOf(objcls(subObjBATy)));
  EXPECT_TRUE(subObjBAATy.subtypeOf(opt(subObjBATy)));
  EXPECT_TRUE(objcls(subObjBAATy).strictSubtypeOf(subClsBaseTy));
  EXPECT_TRUE(subObjBAATy.strictSubtypeOf(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(subObjBAATy).strictSubtypeOf(subClsBTy));
  EXPECT_TRUE(subObjBAATy.strictSubtypeOf(opt(subObjBTy)));
  EXPECT_TRUE(subClsBAATy.strictSubtypeOf(objcls(subObjBATy)));
  EXPECT_TRUE(subObjBAATy.strictSubtypeOf(opt(subObjBATy)));
  EXPECT_TRUE(objcls(subObjBAATy).couldBe(subClsBaseTy));
  EXPECT_TRUE(subObjBAATy.couldBe(opt(subObjBaseTy)));
  EXPECT_TRUE(objcls(subObjBAATy).couldBe(subClsBTy));
  EXPECT_TRUE(subObjBAATy.couldBe(opt(subObjBTy)));
  EXPECT_TRUE(subClsBAATy.couldBe(objcls(subObjBATy)));
  EXPECT_TRUE(subObjBAATy.couldBe(opt(subObjBATy)));

  // Given the hierarchy A <- B <- C where A is the base then:
  // A<= is not in a subtype neither a strict subtype with B<=, ?B<=, A<=
  // ?A<=. However A<= is in a "could be" relationship with all its
  // children (including optional)
  EXPECT_FALSE(objcls(subObjBaseTy).subtypeOf(subClsATy));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjATy)));
  EXPECT_FALSE(objcls(subObjBaseTy).subtypeOf(subClsBTy));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjBTy)));
  EXPECT_FALSE(subClsBaseTy.subtypeOf(objcls(subObjAATy)));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjAATy)));
  EXPECT_FALSE(subClsBaseTy.subtypeOf(objcls(subObjABTy)));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjABTy)));
  EXPECT_FALSE(objcls(subObjBaseTy).subtypeOf(subClsBATy));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjBATy)));
  EXPECT_FALSE(subClsBaseTy.subtypeOf(objcls(subObjBBTy)));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjBBTy)));
  EXPECT_FALSE(subClsBaseTy.subtypeOf(objcls(subObjBAATy)));
  EXPECT_FALSE(subObjBaseTy.subtypeOf(opt(subObjBAATy)));
  EXPECT_FALSE(objcls(subObjBaseTy).strictSubtypeOf(subClsATy));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjATy)));
  EXPECT_FALSE(objcls(subObjBaseTy).strictSubtypeOf(subClsBTy));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjBTy)));
  EXPECT_FALSE(subClsBaseTy.strictSubtypeOf(objcls(subObjAATy)));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjAATy)));
  EXPECT_FALSE(subClsBaseTy.strictSubtypeOf(objcls(subObjABTy)));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjABTy)));
  EXPECT_FALSE(objcls(subObjBaseTy).strictSubtypeOf(subClsBATy));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjBATy)));
  EXPECT_FALSE(subClsBaseTy.strictSubtypeOf(objcls(subObjBBTy)));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjBBTy)));
  EXPECT_FALSE(subClsBaseTy.strictSubtypeOf(objcls(subObjBAATy)));
  EXPECT_FALSE(subObjBaseTy.strictSubtypeOf(opt(subObjBAATy)));
  EXPECT_TRUE(objcls(subObjBaseTy).couldBe(subClsATy));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjATy)));
  EXPECT_TRUE(objcls(subObjBaseTy).couldBe(subClsBTy));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjBTy)));
  EXPECT_TRUE(subClsBaseTy.couldBe(objcls(subObjAATy)));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjAATy)));
  EXPECT_TRUE(subClsBaseTy.couldBe(objcls(subObjABTy)));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjABTy)));
  EXPECT_TRUE(objcls(subObjBaseTy).couldBe(subClsBATy));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjBATy)));
  EXPECT_TRUE(subClsBaseTy.couldBe(objcls(subObjBBTy)));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjBBTy)));
  EXPECT_TRUE(subClsBaseTy.couldBe(objcls(subObjBAATy)));
  EXPECT_TRUE(subObjBaseTy.couldBe(opt(subObjBAATy)));

  // check union_of and commonAncestor API
  EXPECT_TRUE((*(*clsA).commonAncestor(*clsB)).same(*clsBase));
  EXPECT_TRUE((*(*clsB).commonAncestor(*clsA)).same(*clsBase));
  EXPECT_TRUE((*(*clsAA).commonAncestor(*clsAB)).same(*clsA));
  EXPECT_TRUE((*(*clsAB).commonAncestor(*clsAA)).same(*clsA));
  EXPECT_TRUE((*(*clsA).commonAncestor(*clsBAA)).same(*clsBase));
  EXPECT_TRUE((*(*clsBAA).commonAncestor(*clsA)).same(*clsBase));
  EXPECT_TRUE((*(*clsBAA).commonAncestor(*clsB)).same(*clsB));
  EXPECT_TRUE((*(*clsB).commonAncestor(*clsBAA)).same(*clsB));
  EXPECT_TRUE((*(*clsBAA).commonAncestor(*clsBB)).same(*clsB));
  EXPECT_TRUE((*(*clsBB).commonAncestor(*clsBAA)).same(*clsB));
  EXPECT_TRUE((*(*clsAA).commonAncestor(*clsBase)).same(*clsBase));
  EXPECT_TRUE((*(*clsBase).commonAncestor(*clsAA)).same(*clsBase));
  EXPECT_FALSE((*clsAA).commonAncestor(*clsTestClass));
  EXPECT_FALSE((*clsTestClass).commonAncestor(*clsAA));
  EXPECT_FALSE((*clsBAA).commonAncestor(*clsNonUnique));
  EXPECT_FALSE((*clsNonUnique).commonAncestor(*clsBAA));

  // check union_of
  // union of subCls
  EXPECT_EQ(union_of(subClsATy, subClsBTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, subClsABTy), subClsATy);
  EXPECT_EQ(union_of(subClsATy, subClsBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsBAATy, subClsBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsBAATy, subClsBBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsAATy, subClsBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, subClsTestClassTy), TCls);
  EXPECT_EQ(union_of(subClsBAATy, subClsNonUniqueTy), TCls);
  // union of subCls and clsExact mixed
  EXPECT_EQ(union_of(clsExactATy, subClsBTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, clsExactABTy), subClsATy);
  EXPECT_EQ(union_of(clsExactATy, subClsBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsBAATy, clsExactBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactBAATy, subClsBBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsAATy, clsExactBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, subClsTestClassTy), TCls);
  EXPECT_EQ(union_of(subClsBAATy, clsExactNonUniqueTy), TCls);
  // union of clsExact
  EXPECT_EQ(union_of(clsExactATy, clsExactBTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, clsExactABTy), subClsATy);
  EXPECT_EQ(union_of(clsExactATy, clsExactBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactBAATy, clsExactBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactBAATy, clsExactBBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactAATy, clsExactBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, subClsTestClassTy), TCls);
  EXPECT_EQ(union_of(clsExactBAATy, clsExactNonUniqueTy), TCls);
  // union of subObj
  EXPECT_EQ(union_of(subObjATy, subObjBTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, subObjABTy), subObjATy);
  EXPECT_EQ(union_of(subObjATy, subObjBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjBAATy, subObjBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjBAATy, subObjBBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjAATy, subObjBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, subObjTestClassTy), TObj);
  EXPECT_EQ(union_of(subObjBAATy, subObjNonUniqueTy), TObj);
  // union of subObj and objExact mixed
  EXPECT_EQ(union_of(objExactATy, subObjBTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, objExactABTy), subObjATy);
  EXPECT_EQ(union_of(objExactATy, subObjBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjBAATy, objExactBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactBAATy, subObjBBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjAATy, objExactBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, subObjTestClassTy), TObj);
  EXPECT_EQ(union_of(subObjBAATy, objExactNonUniqueTy), TObj);
  // union of objExact
  EXPECT_EQ(union_of(objExactATy, objExactBTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, objExactABTy), subObjATy);
  EXPECT_EQ(union_of(objExactATy, objExactBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactBAATy, objExactBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactBAATy, objExactBBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactAATy, objExactBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, objExactTestClassTy), TObj);
  EXPECT_EQ(union_of(objExactBAATy, objExactNonUniqueTy), TObj);
  // optional sub obj
  EXPECT_EQ(union_of(opt(subObjATy), opt(subObjBTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(subObjABTy)), opt(subObjATy));
  EXPECT_EQ(union_of(opt(subObjATy), subObjBAATy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), opt(subObjBTy)), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), subObjBBTy), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjAATy), opt(subObjBaseTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(subObjTestClassTy)), opt(TObj));
  EXPECT_EQ(union_of(subObjBAATy, opt(subObjNonUniqueTy)), opt(TObj));
  // optional sub and exact obj mixed
  EXPECT_EQ(union_of(opt(objExactATy), subObjBTy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(objExactABTy)), opt(subObjATy));
  EXPECT_EQ(union_of(opt(objExactATy), objExactBAATy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjBAATy, opt(objExactBTy)), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), objExactBBTy), opt(subObjBTy));
  EXPECT_EQ(union_of(objExactAATy, opt(objExactBaseTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(opt(subObjAATy), objExactTestClassTy), opt(TObj));
  EXPECT_EQ(union_of(subObjBAATy, opt(objExactNonUniqueTy)), opt(TObj));
}

TEST(Type, Interface) {
  auto const program = make_program();
  auto const unit = borrow(program->units.back());
  auto const func = [&]() -> borrowed_ptr<php::Func> {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return borrow(f);
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{borrow(program)};

  // load classes in hierarchy
  auto const clsIA = idx.resolve_class(ctx, s_IA.get());
  if (!clsIA) EXPECT_TRUE(false);
  auto const clsIAA = idx.resolve_class(ctx, s_IAA.get());
  if (!clsIAA) EXPECT_TRUE(false);
  auto const clsA = idx.resolve_class(ctx, s_A.get());
  if (!clsA) EXPECT_TRUE(false);
  auto const clsAA = idx.resolve_class(ctx, s_AA.get());
  if (!clsAA) EXPECT_TRUE(false);

  // make sometypes and objects
  auto const subObjIATy  = subObj(*clsIA);
  auto const subClsIATy  = subCls(*clsIA);
  auto const subObjIAATy = subObj(*clsIAA);
  auto const subClsIAATy = subCls(*clsIAA);
  auto const subObjATy   = subObj(*clsA);
  auto const clsExactATy = clsExact(*clsA);
  auto const subClsATy   = subCls(*clsA);
  auto const subObjAATy  = subObj(*clsAA);
  auto const subClsAATy  = subCls(*clsAA);

  // we don't support interfaces quite yet so let's put few tests
  // that will fail once interfaces are supported

  // first 2 are "not precise" - should be true
  EXPECT_FALSE(subClsATy.subtypeOf(objcls(subObjIATy)));
  EXPECT_FALSE(objcls(subObjATy).strictSubtypeOf(subClsIATy));
  EXPECT_TRUE(subClsATy.couldBe(objcls(subObjIATy)));

  // first 2 are "not precise" - should be true
  EXPECT_FALSE(subClsAATy.subtypeOf(objcls(subObjIAATy)));
  EXPECT_FALSE(objcls(subObjAATy).strictSubtypeOf(objcls(subObjIAATy)));
  EXPECT_TRUE(subClsAATy.couldBe(objcls(subObjIAATy)));

  // 3rd one is not precise - should be false
  EXPECT_FALSE(subClsATy.subtypeOf(objcls(subObjIAATy)));
  EXPECT_FALSE(objcls(subObjATy).strictSubtypeOf(objcls(subObjIAATy)));
  EXPECT_TRUE(clsExactATy.couldBe(objcls(subObjIAATy)));
}

TEST(Type, NonUnique) {
  auto const program = make_program();
  auto const unit = borrow(program->units.back());
  auto const func = [&]() -> borrowed_ptr<php::Func> {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return borrow(f);
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{borrow(program)};

  auto const clsA = idx.resolve_class(ctx, s_A.get());
  if (!clsA) EXPECT_TRUE(false);
  auto const clssNonUnique = idx.resolve_class(ctx, s_NonUnique.get());
  if (!clssNonUnique) EXPECT_TRUE(false);
  auto const clssNonUniqueA = idx.resolve_class(ctx, s_NonUniqueA.get());
  if (!clssNonUniqueA) EXPECT_TRUE(false);

  // non unique types are funny because we cannot really make any conclusion
  // about them so they resolve to "non precise" subtype relationship
  auto const subObjATy          = subObj(*clsA);
  auto const subClsATy          = subCls(*clsA);
  auto const subObjNonUniqueTy  = subObj(*clssNonUnique);
  auto const subClsNonUniqueTy  = subCls(*clssNonUnique);
  auto const subObjNonUniqueATy = subObj(*clssNonUniqueA);
  auto const subClsNonUniqueATy = subCls(*clssNonUniqueA);

  // all are obviously "non precise" but what can you do?....
  EXPECT_FALSE(subClsNonUniqueATy.subtypeOf(objcls(subObjNonUniqueTy)));
  EXPECT_FALSE(objcls(subObjNonUniqueATy).strictSubtypeOf(subClsNonUniqueTy));
  EXPECT_TRUE(subClsATy.couldBe(objcls(subObjNonUniqueTy)));
}

TEST(Type, WaitH) {
  auto const program = make_program();
  Index index { borrow(program) };

  for (auto& t : wait_handles_of(index, all())) {
    EXPECT_TRUE(is_specialized_wait_handle(t));
    EXPECT_TRUE(t.subtypeOf(wait_handle(index, TTop)));
  }

  // union_of(WaitH<A>, WaitH<B>) == WaitH<union_of(A, B)>
  for (auto& t1 : all()) {
    for (auto& t2 : all()) {
      auto const u1 = union_of(t1, t2);
      auto const u2 = union_of(wait_handle(index, t1), wait_handle(index, t2));
      EXPECT_TRUE(is_specialized_wait_handle(u2));
      EXPECT_EQ(wait_handle_inner(u2), u1);
      EXPECT_EQ(wait_handle(index, u1), u2);
    }
  }

  // union_of(?WaitH<A>, ?WaitH<B>) == ?WaitH<union_of(A, B)>
  for (auto& t1 : all()) {
    for (auto& t2 : all()) {
      auto const w1 = opt(wait_handle(index, t1));
      auto const w2 = opt(wait_handle(index, t2));
      auto const u1 = union_of(w1, w2);
      auto const u2 = opt(wait_handle(index, union_of(t1, t2)));
      EXPECT_EQ(u1, u2);
    }
  }

  auto const rcls   = index.builtin_class(s_WaitHandle.get());
  auto const twhobj = subObj(rcls);
  EXPECT_TRUE(wait_handle(index, TTop).subtypeOf(twhobj));

  // Some test cases with optional wait handles.
  auto const optWH = opt(wait_handle(index, ival(2)));
  EXPECT_TRUE(is_opt(optWH));
  EXPECT_TRUE(TInitNull.subtypeOf(optWH));
  EXPECT_TRUE(optWH.subtypeOf(TOptObj));
  EXPECT_TRUE(optWH.subtypeOf(opt(twhobj)));
  EXPECT_TRUE(wait_handle(index, ival(2)).subtypeOf(optWH));
  EXPECT_FALSE(optWH.subtypeOf(wait_handle(index, ival(2))));
  EXPECT_TRUE(optWH.couldBe(wait_handle(index, ival(2))));

  // union_of(WaitH<T>, Obj<=WaitHandle) == Obj<=WaitHandle
  for (auto& t : all()) {
    auto const u = union_of(wait_handle(index, t), twhobj);
    EXPECT_EQ(u, twhobj);
  }

  for (auto& t : all()) {
    auto const u1 = union_of(wait_handle(index, t), TInitNull);
    EXPECT_TRUE(is_opt(u1));
    EXPECT_TRUE(is_specialized_wait_handle(u1));
    auto const u2 = union_of(TInitNull, wait_handle(index, t));
    EXPECT_TRUE(is_opt(u2));
    EXPECT_TRUE(is_specialized_wait_handle(u2));
    EXPECT_EQ(u1, u2);
  }

  // You can have WaitH<WaitH<T>>.  And stuff.
  for (auto& w : wait_handles_of(index, all())) {
    auto const ww  = wait_handle(index, w);
    auto const www = wait_handle(index, ww);

    EXPECT_EQ(wait_handle_inner(www), ww);
    EXPECT_EQ(wait_handle_inner(ww), w);

    // Skip the following in cases like WaitH<WaitH<Obj>>, which
    // actually *is* a subtype of WaitH<Obj>, since a WaitH<Obj> is
    // also an Obj.  Similar for Top, or InitCell, etc.
    auto const inner = wait_handle_inner(w);
    if (twhobj.subtypeOf(inner)) continue;

    EXPECT_FALSE(w.subtypeOf(ww));
    EXPECT_FALSE(ww.subtypeOf(w));
    EXPECT_FALSE(w.couldBe(ww));
    EXPECT_TRUE(ww.subtypeOf(twhobj));
    EXPECT_TRUE(www.subtypeOf(twhobj));
    EXPECT_FALSE(ww.subtypeOf(www));
    EXPECT_FALSE(www.subtypeOf(ww));
    EXPECT_FALSE(ww.couldBe(www));
  }
}

TEST(Type, FromHNIConstraint) {
  EXPECT_EQ(from_hni_constraint(makeStaticString("?HH\\resource")), TOptRes);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\resource")), TRes);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\bool")), TBool);
  EXPECT_EQ(from_hni_constraint(makeStaticString("?HH\\bool")), TOptBool);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\int")), TInt);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\float")), TDbl);
  EXPECT_EQ(from_hni_constraint(makeStaticString("?HH\\float")), TOptDbl);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\mixed")), TInitGen);

  // These are conservative, but we're testing them that way.  If we
  // make the function better later we'll remove the tests.
  EXPECT_EQ(from_hni_constraint(makeStaticString("stdClass")), TGen);
  EXPECT_EQ(from_hni_constraint(makeStaticString("?stdClass")), TGen);
  EXPECT_EQ(from_hni_constraint(makeStaticString("fooooo")), TGen);
  EXPECT_EQ(from_hni_constraint(makeStaticString("")), TGen);
}

TEST(Type, ArrPacked1) {
  auto const a1 = arr_packed({ival(2), TSStr, TInt});
  auto const a2 = arr_packed({TInt,    TStr,  TInitCell});
  auto const s1 = sarr_packed({ival(2), TSStr, TInt});
  auto const s2 = sarr_packed({TInt,    TStr,  TInitCell});
  auto const c1 = carr_packed({ival(2), TSStr, TInt});
  auto const c2 = carr_packed({TInt,    TStr,  TInitCell});

  for (auto& a : { a1, s1, c1, a2, s2, c2 }) {
    EXPECT_TRUE(a.subtypeOf(TArr));
    EXPECT_TRUE(a.subtypeOf(a));
    EXPECT_EQ(a, a);
  }

  // Subtype stuff.

  EXPECT_TRUE(a1.subtypeOf(TArr));
  EXPECT_FALSE(a1.subtypeOf(TSArr));
  EXPECT_FALSE(a1.subtypeOf(TCArr));

  EXPECT_TRUE(s1.subtypeOf(TArr));
  EXPECT_TRUE(s1.subtypeOf(TSArr));
  EXPECT_FALSE(s1.subtypeOf(TCArr));

  EXPECT_TRUE(c1.subtypeOf(TArr));
  EXPECT_TRUE(c1.subtypeOf(TCArr));
  EXPECT_FALSE(c1.subtypeOf(TSArr));

  EXPECT_TRUE(a1.subtypeOf(a2));
  EXPECT_TRUE(s1.subtypeOf(s2));
  EXPECT_TRUE(c1.subtypeOf(c2));
  EXPECT_TRUE(s1.subtypeOf(a1));
  EXPECT_TRUE(c1.subtypeOf(a1));

  EXPECT_FALSE(a1.subtypeOf(c1));
  EXPECT_FALSE(s1.subtypeOf(c1));
  EXPECT_FALSE(c1.subtypeOf(s1));
  EXPECT_FALSE(a1.subtypeOf(c1));

  // Could be stuff.

  EXPECT_TRUE(c1.couldBe(a1));
  EXPECT_TRUE(c2.couldBe(a2));
  EXPECT_TRUE(s1.couldBe(a1));
  EXPECT_TRUE(s2.couldBe(a2));

  EXPECT_TRUE(a1.couldBe(a2));
  EXPECT_TRUE(a2.couldBe(a1));
  EXPECT_TRUE(s1.couldBe(a2));
  EXPECT_TRUE(s2.couldBe(a1));
  EXPECT_TRUE(c1.couldBe(a2));
  EXPECT_TRUE(c2.couldBe(a1));

  EXPECT_TRUE(s1.couldBe(s2));
  EXPECT_TRUE(s2.couldBe(s1));
  EXPECT_TRUE(c1.couldBe(c2));
  EXPECT_TRUE(c2.couldBe(c1));

  EXPECT_FALSE(c1.couldBe(s1));
  EXPECT_FALSE(c2.couldBe(s1));
  EXPECT_FALSE(c1.couldBe(s2));
  EXPECT_FALSE(c2.couldBe(s2));

  EXPECT_FALSE(s1.couldBe(c1));
  EXPECT_FALSE(s2.couldBe(c1));
  EXPECT_FALSE(s1.couldBe(c2));
  EXPECT_FALSE(s2.couldBe(c2));
}

TEST(Type, OptArrPacked1) {
  auto const a1 = opt(arr_packed({ival(2), TSStr, TInt}));
  auto const a2 = opt(arr_packed({TInt,    TStr,  TInitCell}));
  auto const s1 = opt(sarr_packed({ival(2), TSStr, TInt}));
  auto const s2 = opt(sarr_packed({TInt,    TStr,  TInitCell}));
  auto const c1 = opt(carr_packed({ival(2), TSStr, TInt}));
  auto const c2 = opt(carr_packed({TInt,    TStr,  TInitCell}));

  for (auto& a : { a1, s1, c1, a2, s2, c2 }) {
    EXPECT_TRUE(a.subtypeOf(TOptArr));
    EXPECT_TRUE(a.subtypeOf(a));
    EXPECT_EQ(a, a);
  }

  // Subtype stuff.

  EXPECT_TRUE(a1.subtypeOf(TOptArr));
  EXPECT_FALSE(a1.subtypeOf(TOptSArr));
  EXPECT_FALSE(a1.subtypeOf(TOptCArr));

  EXPECT_TRUE(s1.subtypeOf(TOptArr));
  EXPECT_TRUE(s1.subtypeOf(TOptSArr));
  EXPECT_FALSE(s1.subtypeOf(TOptCArr));

  EXPECT_TRUE(c1.subtypeOf(TOptArr));
  EXPECT_TRUE(c1.subtypeOf(TOptCArr));
  EXPECT_FALSE(c1.subtypeOf(TOptSArr));

  EXPECT_TRUE(a1.subtypeOf(a2));
  EXPECT_TRUE(s1.subtypeOf(s2));
  EXPECT_TRUE(c1.subtypeOf(c2));
  EXPECT_TRUE(s1.subtypeOf(a1));
  EXPECT_TRUE(c1.subtypeOf(a1));

  EXPECT_FALSE(a1.subtypeOf(c1));
  EXPECT_FALSE(s1.subtypeOf(c1));
  EXPECT_FALSE(c1.subtypeOf(s1));
  EXPECT_FALSE(a1.subtypeOf(c1));

  // Could be stuff.

  EXPECT_TRUE(c1.couldBe(a1));
  EXPECT_TRUE(c2.couldBe(a2));
  EXPECT_TRUE(s1.couldBe(a1));
  EXPECT_TRUE(s2.couldBe(a2));

  EXPECT_TRUE(a1.couldBe(a2));
  EXPECT_TRUE(a2.couldBe(a1));
  EXPECT_TRUE(s1.couldBe(a2));
  EXPECT_TRUE(s2.couldBe(a1));
  EXPECT_TRUE(c1.couldBe(a2));
  EXPECT_TRUE(c2.couldBe(a1));

  EXPECT_TRUE(s1.couldBe(s2));
  EXPECT_TRUE(s2.couldBe(s1));
  EXPECT_TRUE(c1.couldBe(c2));
  EXPECT_TRUE(c2.couldBe(c1));

  EXPECT_TRUE(c1.couldBe(s1));
  EXPECT_TRUE(c2.couldBe(s1));
  EXPECT_TRUE(c1.couldBe(s2));
  EXPECT_TRUE(c2.couldBe(s2));

  EXPECT_TRUE(s1.couldBe(c1));
  EXPECT_TRUE(s2.couldBe(c1));
  EXPECT_TRUE(s1.couldBe(c2));
  EXPECT_TRUE(s2.couldBe(c2));
}

TEST(Type, ArrPacked2) {
  {
    auto const a1 = arr_packed({TInt, TInt, TDbl});
    auto const a2 = arr_packed({TInt, TInt});
    EXPECT_FALSE(a1.subtypeOf(a2));
    EXPECT_FALSE(a1.couldBe(a2));
  }

  {
    auto const a1 = arr_packed({TInitCell, TInt});
    auto const a2 = arr_packed({TInt, TInt});
    EXPECT_TRUE(a1.couldBe(a2));
    EXPECT_TRUE(a2.subtypeOf(a1));
  }

  {
    auto const a1 = arr_packed({TInt, TInt, TInt});
    auto const s1 = sarr_packed({TInt, TInt, TInt});
    auto const c1 = carr_packed({TInt, TInt, TInt});
    auto const s2 = aval(test_array_packed_value());
    EXPECT_TRUE(s2.subtypeOf(a1));
    EXPECT_TRUE(s2.subtypeOf(s1));
    EXPECT_FALSE(s2.subtypeOf(c1));
    EXPECT_TRUE(s2.couldBe(a1));
    EXPECT_TRUE(s2.couldBe(s1));
    EXPECT_FALSE(s2.couldBe(c1));
  }

  {
    auto const s1 = sarr_packed({ival(42), ival(23), ival(12)});
    auto const s2 = aval(test_array_packed_value());
    auto const s3 = sarr_packed({TInt});
    auto const a4 = sarr_packed({TInt});
    auto const a5 = arr_packed({ival(42), ival(23), ival(12)});
    auto const c6 = carr_packed({ival(42), ival(23), ival(12)});
    EXPECT_TRUE(s1.subtypeOf(s2));
    EXPECT_EQ(s1, s2);
    EXPECT_FALSE(s2.subtypeOf(s3));
    EXPECT_FALSE(s2.couldBe(s3));
    EXPECT_FALSE(s2.subtypeOf(s3));
    EXPECT_FALSE(s2.couldBe(s3));
    EXPECT_TRUE(s2.couldBe(s1));
    EXPECT_TRUE(s2.couldBe(a5));
    EXPECT_TRUE(s2.subtypeOf(a5));
    EXPECT_FALSE(a5.subtypeOf(s2));
    EXPECT_FALSE(s2.subtypeOf(c6));
    EXPECT_FALSE(c6.subtypeOf(s2));
  }
}

TEST(Type, ArrPackedUnion) {
  {
    auto const a1 = arr_packed({TInt, TDbl});
    auto const a2 = arr_packed({TDbl, TInt});
    EXPECT_EQ(union_of(a1, a2), arr_packed({TNum, TNum}));
  }

  {
    auto const s1 = sarr_packed({TInt, TDbl});
    auto const s2 = sarr_packed({TDbl, TInt});
    auto const c2 = carr_packed({TDbl, TInt});
    EXPECT_EQ(union_of(s1, c2), arr_packed({TNum, TNum}));
    EXPECT_EQ(union_of(s1, s1), s1);
    EXPECT_EQ(union_of(s1, s2), sarr_packed({TNum, TNum}));
  }

  {
    auto const s1 = sarr_packed({TInt});
    auto const s2 = sarr_packed({TDbl, TDbl});
    EXPECT_EQ(union_of(s1, s2), sarr_packedn(TNum));
  }

  {
    auto const s1 = aval(test_array_packed_value());
    auto const s2 = sarr_packed({TInt, TInt, TInt});
    auto const s3 = sarr_packed({TInt, TNum, TInt});
    auto const s4 = carr_packed({TInt, TObj, TInt});
    EXPECT_EQ(union_of(s1, s2), s2);
    EXPECT_EQ(union_of(s1, s3), s3);
    EXPECT_EQ(union_of(s1, s4), arr_packed({TInt, TInitCell, TInt}));
  }

  {
    auto const s1  = sarr_packed({TInt});
    auto const os1 = opt(s1);
    EXPECT_EQ(union_of(s1, TInitNull), os1);
    EXPECT_EQ(union_of(os1, s1), os1);
    EXPECT_EQ(union_of(TInitNull, s1), os1);
    EXPECT_EQ(union_of(os1, os1), os1);
  }

  {
    auto const s1 = sarr_packed({TInt});
    EXPECT_EQ(union_of(s1, TObj), TInitCell);
    EXPECT_EQ(union_of(s1, TCArr), TArr);
  }

  {
    auto const s1 = aval(test_array_packed_value());
    auto const s2 = aval(test_array_packed_value2());
    EXPECT_EQ(union_of(s1, s2), sarr_packed({ival(42), TNum, ival(12)}));
  }
}

TEST(Type, ArrPackedN) {
  auto const s1 = aval(test_array_packed_value());
  auto const s2 = sarr_packed({TInt, TInt});
  EXPECT_EQ(union_of(s1, s2), sarr_packedn(TInt));

  EXPECT_TRUE(s2.subtypeOf(sarr_packedn(TInt)));
  EXPECT_FALSE(s2.subtypeOf(sarr_packedn(TDbl)));
  EXPECT_TRUE(s2.subtypeOf(sarr_packedn(TNum)));
  EXPECT_TRUE(s2.subtypeOf(arr_packedn(TInt)));
  EXPECT_TRUE(s2.subtypeOf(opt(arr_packedn(TInt))));

  EXPECT_TRUE(s2.couldBe(arr_packedn(TInt)));
  EXPECT_TRUE(s2.couldBe(arr_packedn(TInitGen)));

  auto const sn1 = sarr_packedn(TInt);
  auto const sn2 = sarr_packedn(TInitNull);

  EXPECT_EQ(union_of(sn1, sn2), sarr_packedn(TOptInt));
  EXPECT_EQ(union_of(sn1, TInitNull), opt(sn1));
  EXPECT_EQ(union_of(TInitNull, sn1), opt(sn1));
  EXPECT_FALSE(sn2.couldBe(sn1));
  EXPECT_FALSE(sn1.couldBe(sn2));

  auto const sn3 = sarr_packedn(TInitCell);
  EXPECT_TRUE(sn1.couldBe(sn3));
  EXPECT_TRUE(sn2.couldBe(sn3));
  EXPECT_TRUE(sn3.couldBe(sn1));
  EXPECT_TRUE(sn3.couldBe(sn2));

  EXPECT_TRUE(s2.couldBe(sn3));
  EXPECT_TRUE(s2.couldBe(sn1));
  EXPECT_FALSE(s2.couldBe(sn2));

  EXPECT_EQ(union_of(carr_packedn(TInt), sarr_packedn(TInt)),
            arr_packedn(TInt));
}

TEST(Type, ArrStruct) {
  auto test_map_a          = StructMap{};
  test_map_a[s_test.get()] = ival(2);

  auto test_map_b          = StructMap{};
  test_map_b[s_test.get()] = TInt;

  auto test_map_c          = StructMap{};
  test_map_c[s_test.get()] = ival(2);
  test_map_c[s_A.get()]    = TInt;
  test_map_c[s_B.get()]    = TDbl;

  auto const ta = arr_struct(test_map_a);
  auto const tb = arr_struct(test_map_b);
  auto const tc = arr_struct(test_map_c);

  EXPECT_FALSE(ta.subtypeOf(tc));
  EXPECT_FALSE(tc.subtypeOf(ta));
  EXPECT_TRUE(ta.subtypeOf(tb));
  EXPECT_FALSE(tb.subtypeOf(ta));
  EXPECT_TRUE(ta.couldBe(tb));
  EXPECT_TRUE(tb.couldBe(ta));
  EXPECT_FALSE(tc.couldBe(ta));
  EXPECT_FALSE(tc.couldBe(tb));

  EXPECT_TRUE(ta.subtypeOf(TArr));
  EXPECT_TRUE(tb.subtypeOf(TArr));
  EXPECT_TRUE(tc.subtypeOf(TArr));

  auto const sa = sarr_struct(test_map_a);
  auto const sb = sarr_struct(test_map_b);
  auto const sc = sarr_struct(test_map_c);

  EXPECT_FALSE(sa.subtypeOf(sc));
  EXPECT_FALSE(sc.subtypeOf(sa));
  EXPECT_TRUE(sa.subtypeOf(sb));
  EXPECT_FALSE(sb.subtypeOf(sa));
  EXPECT_TRUE(sa.couldBe(sb));
  EXPECT_TRUE(sb.couldBe(sa));
  EXPECT_FALSE(sc.couldBe(sa));
  EXPECT_FALSE(sc.couldBe(sb));

  EXPECT_TRUE(sa.subtypeOf(TSArr));
  EXPECT_TRUE(sb.subtypeOf(TSArr));
  EXPECT_TRUE(sc.subtypeOf(TSArr));

  auto test_map_d          = StructMap{};
  test_map_d[s_A.get()]    = sval(s_B.get());
  test_map_d[s_test.get()] = ival(12);
  auto const sd = sarr_struct(test_map_d);
  EXPECT_EQ(sd, aval(test_array_map_value()));

  auto test_map_e          = StructMap{};
  test_map_e[s_A.get()]    = TSStr;
  test_map_e[s_test.get()] = TNum;
  auto const se = sarr_struct(test_map_e);
  EXPECT_TRUE(aval(test_array_map_value()).subtypeOf(se));
  EXPECT_TRUE(se.couldBe(aval(test_array_map_value())));
}

TEST(Type, ArrMapN) {
  auto const test_map = aval(test_array_map_value());
  EXPECT_TRUE(test_map != arr_mapn(TSStr, TInitUnc));
  EXPECT_TRUE(test_map.subtypeOf(arr_mapn(TSStr, TInitUnc)));
  EXPECT_TRUE(test_map.subtypeOf(sarr_mapn(TSStr, TInitUnc)));
  EXPECT_TRUE(!test_map.subtypeOf(carr_mapn(TSStr, TInitUnc)));
  EXPECT_TRUE(sarr_packedn({TInt}).subtypeOf(arr_mapn(TInt, TInt)));
  EXPECT_TRUE(sarr_packed({TInt}).subtypeOf(arr_mapn(TInt, TInt)));

  auto test_map_a          = StructMap{};
  test_map_a[s_test.get()] = ival(2);
  auto const tstruct       = sarr_struct(test_map_a);

  EXPECT_TRUE(tstruct.subtypeOf(arr_mapn(TSStr, ival(2))));
  EXPECT_TRUE(tstruct.subtypeOf(arr_mapn(TSStr, TInt)));
  EXPECT_TRUE(tstruct.subtypeOf(sarr_mapn(TSStr, TInt)));
  EXPECT_TRUE(tstruct.subtypeOf(arr_mapn(TStr, TInt)));
  EXPECT_TRUE(!tstruct.subtypeOf(carr_mapn(TStr, TInt)));

  EXPECT_TRUE(test_map.couldBe(arr_mapn(TSStr, TInitCell)));
  EXPECT_FALSE(test_map.couldBe(arr_mapn(TSStr, TCStr)));
  EXPECT_FALSE(test_map.couldBe(arr_mapn(TSStr, TObj)));

  EXPECT_FALSE(test_map.couldBe(aval(test_empty_array())));
  EXPECT_FALSE(arr_mapn(TSStr, TInt).couldBe(aval(test_empty_array())));

  EXPECT_TRUE(sarr_packedn(TInt).couldBe(sarr_mapn(TInt, TInt)));
  EXPECT_FALSE(sarr_packedn(TInt).couldBe(sarr_mapn(TInt, TObj)));

  EXPECT_TRUE(tstruct.couldBe(sarr_mapn(TSStr, TInt)));
  EXPECT_FALSE(tstruct.couldBe(sarr_mapn(TSStr, TObj)));
  EXPECT_FALSE(tstruct.couldBe(carr_mapn(TSStr, TObj)));
  EXPECT_FALSE(tstruct.couldBe(carr_mapn(TSStr, TInt)));
}

TEST(Type, ArrEquivalentRepresentations) {
  {
    auto const simple = aval(test_array_packed_value());
    auto const bulky  = sarr_packed({ival(42), ival(23), ival(12)});
    EXPECT_EQ(simple, bulky);
  }

  {
    auto const simple = aval(test_array_map_value());

    auto map          = StructMap{};
    map[s_A.get()]    = sval(s_B.get());
    map[s_test.get()] = ival(12);
    auto const bulky  = sarr_struct(map);

    EXPECT_EQ(simple, bulky);
  }
}

TEST(Type, ArrUnions) {
  auto test_map_a          = StructMap{};
  test_map_a[s_test.get()] = ival(2);
  auto const tstruct       = sarr_struct(test_map_a);

  auto test_map_b          = StructMap{};
  test_map_b[s_test.get()] = TInt;
  auto const tstruct2      = sarr_struct(test_map_b);

  auto test_map_c          = StructMap{};
  test_map_c[s_A.get()]    = TInt;
  auto const tstruct3      = sarr_struct(test_map_c);

  auto test_map_d          = StructMap{};
  test_map_d[s_A.get()]    = TInt;
  test_map_d[s_test.get()] = TDbl;
  auto const tstruct4      = sarr_struct(test_map_d);

  auto const packed_int = arr_packedn(TInt);

  EXPECT_EQ(union_of(tstruct, packed_int),
            arr_mapn(union_of(TSStr, TInt), TInt));
  EXPECT_EQ(union_of(tstruct, tstruct2), tstruct2);
  EXPECT_EQ(union_of(tstruct, tstruct3), sarr_mapn(TSStr, TInt));
  EXPECT_EQ(union_of(tstruct, tstruct4), sarr_mapn(TSStr, TNum));

  EXPECT_EQ(union_of(sarr_packed({TInt, TDbl, TDbl}), sarr_packedn(TDbl)),
            sarr_packedn(TNum));
  EXPECT_EQ(union_of(sarr_packed({TInt, TDbl}), tstruct),
            sarr_mapn(union_of(TSStr, TInt), TNum));

  EXPECT_EQ(union_of(arr_mapn(TInt, TTrue), arr_mapn(TDbl, TFalse)),
            arr_mapn(TNum, TBool));

  auto const aval1 = aval(test_array_packed_value());
  auto const aval2 = aval(test_array_packed_value3());
  EXPECT_EQ(union_of(aval1, aval2), sarr_packedn(TInt));
}

TEST(Type, ArrOfArr) {
  auto const t1 = arr_mapn(TSStr, arr_mapn(TInt, TSStr));
  auto const t2 = arr_mapn(TSStr, TArr);
  auto const t3 = arr_mapn(TSStr, arr_packedn(TSStr));
  auto const t4 = arr_mapn(TSStr, arr_mapn(TSStr, TSStr));
  EXPECT_TRUE(t1.subtypeOf(t2));
  EXPECT_TRUE(t1.couldBe(t3));
  EXPECT_FALSE(t1.subtypeOf(t3));
  EXPECT_TRUE(t3.subtypeOf(t1));
  EXPECT_TRUE(t3.subtypeOf(t2));
  EXPECT_FALSE(t1.couldBe(t4));
  EXPECT_FALSE(t4.couldBe(t3));
  EXPECT_TRUE(t4.subtypeOf(t2));
}

TEST(Type, WideningAlreadyStable) {
  // A widening union on types that are already stable should not move
  // the type anywhere.
  for (auto& t : all()) {
    EXPECT_EQ(widening_union(t, t), t);
  }
  for (auto& t : specialized_array_examples()) {
    EXPECT_EQ(widening_union(t, t), t);
  }
}

TEST(Type, EmptyArray) {
  {
    auto const possible_e = union_of(arr_packedn(TInt), aempty());
    EXPECT_TRUE(possible_e.couldBe(aempty()));
    EXPECT_TRUE(possible_e.couldBe(arr_packedn(TInt)));
    EXPECT_EQ(array_elem(possible_e, ival(0)), opt(TInt));
  }

  {
    auto const possible_e = union_of(arr_packed({TInt, TInt}), aempty());
    EXPECT_TRUE(possible_e.couldBe(aempty()));
    EXPECT_TRUE(possible_e.couldBe(arr_packed({TInt, TInt})));
    EXPECT_FALSE(possible_e.couldBe(arr_packed({TInt, TInt, TInt})));
    EXPECT_FALSE(possible_e.subtypeOf(arr_packedn(TInt)));
    EXPECT_EQ(array_elem(possible_e, ival(0)), opt(TInt));
    EXPECT_EQ(array_elem(possible_e, ival(1)), opt(TInt));
  }

  {
    auto const estat = union_of(sarr_packedn(TInt), aempty());
    EXPECT_TRUE(estat.couldBe(aempty()));
    EXPECT_TRUE(estat.couldBe(sarr_packedn(TInt)));
    EXPECT_FALSE(estat.subtypeOf(sarr_packedn(TInt)));
    EXPECT_FALSE(estat.subtypeOf(TCArr));
    EXPECT_FALSE(estat.couldBe(TCArr));
    EXPECT_FALSE(estat.subtypeOf(TSArrE));
    EXPECT_TRUE(estat.couldBe(TSArrE));
  }

  EXPECT_EQ(array_newelem(aempty(), ival(142)), arr_packed({ival(142)}));
}

TEST(Type, BasicArrays) {
  EXPECT_TRUE(TSArr.subtypeOf(TArr));
  EXPECT_TRUE(TCArr.subtypeOf(TArr));
  EXPECT_TRUE(TArrE.subtypeOf(TArr));
  EXPECT_TRUE(TArrN.subtypeOf(TArr));
  EXPECT_TRUE(TSArrE.subtypeOf(TArr));
  EXPECT_TRUE(TSArrN.subtypeOf(TArr));
  EXPECT_TRUE(TCArrE.subtypeOf(TArr));
  EXPECT_TRUE(TCArrN.subtypeOf(TArr));

  EXPECT_EQ(union_of(TSArr, TCArr), TArr);
  EXPECT_EQ(union_of(TSArrE, TCArrE), TArrE);
  EXPECT_EQ(union_of(TSArrN, TCArrN), TArrN);
  EXPECT_EQ(union_of(TArrN, TArrE), TArr);

  EXPECT_EQ(union_of(TSArrN, TCArrE), TArr);
  EXPECT_EQ(union_of(TSArrE, TCArrN), TArr);
  EXPECT_EQ(union_of(TOptCArrN, TSArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArr, TCArr), TOptArr);
  EXPECT_EQ(union_of(TOptSArrE, TCArrE), TOptArrE);
  EXPECT_EQ(union_of(TOptSArrN, TCArrN), TOptArrN);
  EXPECT_EQ(union_of(TOptArrN, TArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArrN, TOptCArrE), TOptArr);
  EXPECT_EQ(union_of(TOptSArrN, TOptCArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArr, TOptCArr), TOptArr);
  EXPECT_EQ(union_of(TOptSArrE, TOptCArrE), TOptArrE);
  EXPECT_EQ(union_of(TOptSArrN, TOptCArrN), TOptArrN);
  EXPECT_EQ(union_of(TOptArrN, TOptArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArrN, TOptCArrE), TOptArr);
  EXPECT_EQ(union_of(TOptSArrN, TOptCArrE), TOptArr);

  EXPECT_EQ(union_of(TSArr, TInitNull), TOptSArr);
  EXPECT_EQ(union_of(TSArrE, TInitNull), TOptSArrE);
  EXPECT_EQ(union_of(TSArrN, TInitNull), TOptSArrN);
  EXPECT_EQ(union_of(TCArr, TInitNull), TOptCArr);
  EXPECT_EQ(union_of(TCArrE, TInitNull), TOptCArrE);
  EXPECT_EQ(union_of(TCArrN, TInitNull), TOptCArrN);
  EXPECT_EQ(union_of(TArr, TInitNull), TOptArr);
  EXPECT_EQ(union_of(TArrE, TInitNull), TOptArrE);
  EXPECT_EQ(union_of(TArrN, TInitNull), TOptArrN);
}

/*
 * These are tests for some unrepresentable bit combos.  If we ever
 * add predefined bits for things like TSArrE|TCArrN these will fail
 * and need to be revisted.
 */
TEST(Type, ArrBitCombos) {
  auto const u1 = union_of(sarr_packedn(TInt), TCArrE);
  EXPECT_TRUE(u1.couldBe(TArrE));
  EXPECT_TRUE(u1.couldBe(TSArrE));
  EXPECT_TRUE(u1.couldBe(TCArrE));
  EXPECT_TRUE(u1.couldBe(sarr_packedn(TInt)));
  EXPECT_EQ(array_elem(u1, ival(0)), TOptInt);

  auto const u2 = union_of(TSArrE, carr_packedn(TInt));
  EXPECT_TRUE(u2.couldBe(TArrE));
  EXPECT_TRUE(u2.couldBe(TSArrE));
  EXPECT_TRUE(u2.couldBe(TCArrE));
  EXPECT_TRUE(u2.couldBe(arr_packedn(TInt)));
  EXPECT_EQ(array_elem(u2, ival(0)), TOptInt);
}

//////////////////////////////////////////////////////////////////////

}}
