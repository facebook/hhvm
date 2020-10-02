/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <folly/Lazy.h>

#include "hphp/runtime/base/array-init.h"

#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/index.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP { namespace HHBBC {

void PrintTo(const Type& t, ::std::ostream* os) { *os << show(t); }

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
const StaticString s_UniqueRecBase("UniqueRecBase");
const StaticString s_UniqueRec("UniqueRec");
const StaticString s_UniqueRecA("UniqueRecA");
const StaticString s_Awaitable("HH\\Awaitable");

// A test program so we can actually test things involving object or
// class or record types.
void add_test_unit(php::Program& program) {
  assert(SystemLib::s_inited);
  std::string const hhas = R"(
    # Technically this should be provided by systemlib, but it's the
    # only one we have to make sure the type system can see for unit
    # test purposes, so we can just define it here.  We don't need to
    # give it any of its functions currently.
    .class [abstract unique builtin] HH\Awaitable {
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

    # Make sure BAA doesn't get AttrNoOverride:
    .class [unique] BAADeriver extends BAA {
      .default_ctor;
    }

    .class [unique] TestClass {
      .default_ctor;
    }

    # Make sure TestClass doesn't get AttrNoOverride:
    .class [unique] TestClassDeriver extends TestClass {
      .default_ctor;
    }

    .record [abstract persistent unique] UniqueRecBase {}
    .record [final unique] UniqueRec extends UniqueRecBase {}
    .record [final unique] UniqueRecA extends UniqueRecBase {}

    .function test() {
      Int 1
      RetC
    }
  )";
  std::unique_ptr<UnitEmitter> ue(assemble_string(
    hhas.c_str(), hhas.size(),
    "ignore.php",
    SHA1("1234543212345432123454321234543212345432"),
    Native::s_noNativeFuncs
  ));
  parse_unit(program, ue.get());
}

php::ProgramPtr make_test_program() {
  RuntimeOption::EvalHackRecords = true;
  auto program = make_program();
  add_test_unit(*program);
  return program;
}

//////////////////////////////////////////////////////////////////////

TypedValue tv(SString s) { return make_tv<KindOfPersistentString>(s); }
TypedValue tv(const StaticString& s) { return tv(s.get()); }
TypedValue tv(int64_t i) { return make_tv<KindOfInt64>(i); }

auto const test_empty_dict = folly::lazy([] {
  return staticEmptyDictArray();
});

auto const test_dict_map_value = folly::lazy([] {
  auto ar = make_dict_array(s_A.get(), s_B.get(), s_test.get(), 12);
  return ArrayData::GetScalarArray(std::move(ar));
});

// The following helpers produce "vector-like" arrays, in the sense that they
// have the keys 0...size-1, inserted in that order. They're actually backed
// by dicts in the runtime, but we can still test our array specialization
// logic with these types.

auto const test_dict_vector_value = folly::lazy([] {
  auto ar = make_dict_array(0, 42, 1, 23, 2, 12);
  return ArrayData::GetScalarArray(std::move(ar));
});

auto const test_dict_vector_value2 = folly::lazy([] {
  auto ar = make_dict_array(0, 42, 1, 23.0, 2, 12);
  return ArrayData::GetScalarArray(std::move(ar));
});

auto const test_dict_vector_value3 = folly::lazy([] {
  auto ar = make_dict_array(0, 1, 1, 2, 2, 3, 3, 4, 4, 5);
  return ArrayData::GetScalarArray(std::move(ar));
});

auto const with_data = folly::lazy([] {
  return std::vector<Type> {
    ival(2),
    dval(2.0),
    sval(s_test.get()),
    sval_nonstatic(s_test.get()),
    dict_val(test_dict_map_value()),
    dict_val(test_dict_vector_value())
  };
});

auto const specialized_array_examples = folly::lazy([] {
  auto ret = std::vector<Type>{};

  auto test_map_a          = MapElems{};
  test_map_a[tv(s_test)]   = ival(2);
  ret.emplace_back(sdict_map(test_map_a));

  auto test_map_b          = MapElems{};
  test_map_b[tv(s_test)]   = TInt;
  ret.emplace_back(sdict_map(test_map_b));

  auto test_map_c          = MapElems{};
  test_map_c[tv(s_A)]      = TInt;
  ret.emplace_back(sdict_map(test_map_c));

  auto test_map_d          = MapElems{};
  test_map_d[tv(s_A)]      = TInt;
  test_map_d[tv(s_test)]   = TDbl;
  ret.emplace_back(sdict_map(test_map_d));

  auto test_map_e          = MapElems{};
  test_map_e[tv(s_A)]      = TInt;
  test_map_e[tv(s_test)]   = TObj;
  ret.emplace_back(dict_map(test_map_e));

  auto test_map_f          = MapElems{};
  test_map_f[tv(s_A)]      = TInt;
  test_map_f[tv(s_test)]   = TDbl;
  ret.emplace_back(dict_map(test_map_f, TInt, TSStr));

  auto test_map_g          = MapElems{};
  test_map_g[tv(s_A)]      = TInt;
  ret.emplace_back(dict_map(test_map_g, sval(s_test.get()), TSStr));

  auto test_map_h          = MapElems{};
  test_map_h[tv(s_A)]      = TInt;
  ret.emplace_back(dict_map(test_map_h, TSStr, TDbl));

  auto test_map_i          = MapElems{};
  test_map_i[tv(s_A)]      = TInt;
  test_map_i[tv(s_test)]   = TDbl;
  ret.emplace_back(dict_map(test_map_i, TArrKey, TBool));

  auto test_map_j          = MapElems{};
  test_map_j[tv(s_A)]      = TInt;
  ret.emplace_back(dict_map(test_map_j));

  ret.emplace_back(dict_packedn(TInt));
  ret.emplace_back(dict_n(TSStr, dict_n(TInt, TSStr)));
  ret.emplace_back(dict_n(TSStr, TArr));
  ret.emplace_back(dict_n(TSStr, dict_packedn(TSStr)));
  ret.emplace_back(dict_n(TSStr, dict_n(TSStr, TSStr)));
  ret.emplace_back(dict_packed({TInt, TStr, TBool}));
  ret.emplace_back(dict_packedn(TObj));
  ret.emplace_back(dict_packed({TInt, TObj}));
  ret.emplace_back(dict_n(TSStr, TObj));
  ret.emplace_back(union_of(dict_packedn(TObj), TDictE));
  ret.emplace_back(union_of(dict_packed({TInt, TObj}), TDictE));
  ret.emplace_back(union_of(dict_n(TInt, TObj), TDictE));
  ret.emplace_back(union_of(dict_map(test_map_e), TDictE));
  ret.emplace_back(union_of(dict_map(test_map_i, TStr, TObj), TDictE));
  ret.emplace_back(opt(dict_packedn(TObj)));
  ret.emplace_back(opt(dict_packed({TInt, TObj})));
  ret.emplace_back(opt(dict_n(TInt, TObj)));
  ret.emplace_back(opt(dict_map(test_map_e)));
  ret.emplace_back(opt(dict_map(test_map_i, TStr, TObj)));
  ret.emplace_back(opt(union_of(dict_packedn(TObj), TDictE)));
  ret.emplace_back(opt(union_of(dict_packed({TInt, TObj}), TDictE)));
  ret.emplace_back(opt(union_of(dict_n(TInt, TObj), TDictE)));
  ret.emplace_back(opt(union_of(dict_map(test_map_e), TDictE)));
  ret.emplace_back(opt(union_of(dict_map(test_map_i, TSStr, TObj), TDictE)));

  return ret;
});

// In the sense of "non-union type", not the sense of TPrim.
auto const primitives = folly::lazy([] {
  return std::vector<Type> {
    TUninit,
    TInitNull,
    TFalse,
    TTrue,
    TInt,
    TDbl,
    TSStr,
    TSVArrE,
    TSVArrN,
    TSDArrE,
    TSDArrN,
    TSVecE,
    TSVecN,
    TSDictE,
    TSDictN,
    TSKeysetE,
    TSKeysetN,
    TObj,
    TRecord,
    TRes,
    TCls,
    TClsMeth,
    TRClsMeth,
    TFunc,
    TRFunc,
    TLazyCls
  };
});

auto const optionals = folly::lazy([] {
  return std::vector<Type> {
    TOptTrue,
    TOptFalse,
    TOptBool,
    TOptInt,
    TOptDbl,
    TOptNum,
    TOptUncArrKey,
    TOptUncArrKeyCompat,
    TOptArrKey,
    TOptArrKeyCompat,
    TOptSStr,
    TOptStr,
    TOptSVArrE,
    TOptSVArrN,
    TOptSVArr,
    TOptVArr,
    TOptSDArrE,
    TOptSDArrN,
    TOptSDArr,
    TOptDArr,
    TOptSArrE,
    TOptSArrN,
    TOptSArr,
    TOptArr,
    TOptSVecE,
    TOptSVecN,
    TOptSVec,
    TOptVec,
    TOptSDictE,
    TOptSDictN,
    TOptSDict,
    TOptDict,
    TOptSKeysetE,
    TOptSKeysetN,
    TOptSKeyset,
    TOptKeyset,
    TOptObj,
    TOptRecord,
    TOptRes,
    TOptClsMeth,
    TOptRClsMeth,
    TOptArrLikeE,
    TOptArrLikeN,
    TOptArrLike,
    TOptRFunc,
    TOptLazyCls
  };
});

auto const non_opt_unions = folly::lazy([] {
  return std::vector<Type> {
    TInitCell,
    TCell,
    TNull,
    TBool,
    TNum,
    TStr,
    TVArrE,
    TVArrN,
    TSVArr,
    TVArr,
    TDArrE,
    TDArrN,
    TSDArr,
    TDArr,
    TSArrE,
    TSArrN,
    TArrE,
    TArrN,
    TSArr,
    TArr,
    TVecE,
    TVecN,
    TSVec,
    TVec,
    TDictE,
    TDictN,
    TSDict,
    TDict,
    TKeysetE,
    TKeysetN,
    TSKeyset,
    TKeyset,
    TArrLikeE,
    TArrLikeN,
    TArrLike,
    TArrLikeCompat,
    TInitPrim,
    TPrim,
    TInitUnc,
    TUnc,
    TUncArrKey,
    TUncArrKeyCompat,
    TArrKey,
    TArrKeyCompat,
    TArrCompat,
    TArrCompatSA,
    TVArrCompat,
    TVArrCompatSA,
    TVecCompat,
    TVecCompatSA,
    TStrLike,
    TUncStrLike,
    TFuncLike,
    TClsMethLike,
    TTop
  };
});

auto const all_unions = folly::lazy([] {
  return boost::join(optionals(), non_opt_unions());
});

auto const all_no_data = folly::lazy([] {
  std::vector<Type> ret;
  ret.insert(end(ret), begin(primitives()), end(primitives()));
  ret.insert(end(ret), begin(all_unions()), end(all_unions()));
  return ret;
});

auto const all = folly::lazy([] {
  auto ret = all_no_data();
  ret.insert(end(ret), begin(with_data()), end(with_data()));
  ret.insert(end(ret),
             begin(specialized_array_examples()),
             end(specialized_array_examples()));
  return ret;
});

template<class Range>
std::vector<Type> wait_handles_of(const Index& index, const Range& r) {
  std::vector<Type> ret;
  for (auto& t : r) ret.push_back(wait_handle(index, t));
  return ret;
}

std::vector<Type> all_no_data_with_waithandles(const Index& index) {
  auto ret = wait_handles_of(index, all_no_data());
  for (auto& t : all_no_data()) ret.push_back(t);
  return ret;
}

std::vector<Type> all_with_waithandles(const Index& index) {
  auto ret = wait_handles_of(index, all());
  for (auto& t : all()) ret.push_back(t);
  return ret;
}

//////////////////////////////////////////////////////////////////////

}

TEST(Type, Top) {
  auto const program = make_test_program();
  Index index { program.get() };

  // Everything is a subtype of Top, couldBe Top, and the union of Top
  // with anything is Top.
  for (auto& t : all_with_waithandles(index)) {
    EXPECT_TRUE(t.subtypeOf(BTop));
    EXPECT_TRUE(t.couldBe(BTop));
    EXPECT_EQ(union_of(t, TTop), TTop);
    EXPECT_EQ(union_of(TTop, t), TTop);
    EXPECT_EQ(intersection_of(TTop, t), t);
    EXPECT_EQ(intersection_of(t, TTop), t);
  }
}

TEST(Type, Bottom) {
  auto const program = make_test_program();
  Index index { program.get() };

  // Bottom is a subtype of everything, nothing couldBe Bottom, and
  // the union_of anything with Bottom is itself.
  for (auto& t : all_with_waithandles(index)) {
    EXPECT_TRUE(TBottom.subtypeOf(t));
    EXPECT_TRUE(!TBottom.couldBe(t));
    EXPECT_EQ(union_of(t, TBottom), t);
    EXPECT_EQ(union_of(TBottom, t), t);
    EXPECT_EQ(intersection_of(TBottom, t), TBottom);
    EXPECT_EQ(intersection_of(t, TBottom), TBottom);
  }
}

TEST(Type, Prims) {
  auto const program = make_test_program();
  Index index { program.get() };

  // All pairs of non-equivalent primitives are not related by either
  // subtypeOf or couldBe, including if you wrap them in wait handles.
  for (auto& t1 : primitives()) {
    for (auto& t2 : primitives()) {
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
  auto const program = make_test_program();
  Index index { program.get() };
  // couldBe is symmetric and reflexive
  for (auto& t1 : all_with_waithandles(index)) {
    for (auto& t2 : all_with_waithandles(index)) {
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

  for (auto const& t1 : all_with_waithandles(index)) {
    for (auto const& t2 : all_with_waithandles(index)) {
      if (t1.subtypeOf(t2)) {
        EXPECT_TRUE(t1.couldBe(t2));
      }
      if (!t1.couldBe(t2)) {
        EXPECT_FALSE(t1.subtypeOf(t2));
      }
    }
  }

  for (auto const& t : all_with_waithandles(index)) {
    EXPECT_EQ(union_of(t, t), t);
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

  for (auto const& t1 : all_with_waithandles(index)) {
    for (auto const& t2 : all_with_waithandles(index)) {
      EXPECT_TRUE(t1.subtypeOf(union_of(t1, t2)));
      EXPECT_TRUE(t2.subtypeOf(union_of(t1, t2)));
      EXPECT_TRUE(t1.couldBe(union_of(t1, t2)));
      EXPECT_TRUE(t2.couldBe(union_of(t1, t2)));
    }
  }

  for (auto const& t1 : all_with_waithandles(index)) {
    for (auto const& t2 : all_with_waithandles(index)) {
      if (t1.subtypeOf(t2)) {
        EXPECT_EQ(union_of(t1, t2), t2);
      }
      if (t2.subtypeOf(t1)) {
        EXPECT_EQ(union_of(t1, t2), t1);
      }
    }
  }
  for (auto const& t1 : all_with_waithandles(index)) {
    for (auto const& t2 : all_with_waithandles(index)) {
      EXPECT_EQ(intersection_of(t1, t2), intersection_of(t2, t1));
      if (t1.subtypeOf(t2)) {
        EXPECT_EQ(intersection_of(t1, t2), t1);
      } else if (t2.subtypeOf(t1)) {
        EXPECT_EQ(intersection_of(t1, t2), t2);
      }

      if (t1.couldBe(t2)) {
        EXPECT_NE(intersection_of(t1, t2), TBottom);
      } else {
        EXPECT_EQ(intersection_of(t1, t2), TBottom);
      }

      EXPECT_EQ(intersection_of(union_of(t1, t2), t1), t1);
      EXPECT_EQ(intersection_of(union_of(t1, t2), t2), t2);
    }
  }

  for (auto const& t : all_with_waithandles(index)) {
    EXPECT_EQ(intersection_of(t, t), t);
  }
}

TEST(Type, Prim) {
  const std::initializer_list<std::pair<Type, Type>> subtype_true{
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

  const std::initializer_list<std::pair<Type, Type>> subtype_false{
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
    { TRes, TPrim },
    { TObj, TPrim },
    { TRFunc, TPrim },
    { TPrim, dval(0.0) },
    { TVArrCompat, TPrim },
    { TVecCompat, TPrim },
    { TArrCompat, TPrim },
    { TStrLike, TPrim },
    { TFuncLike, TPrim },
    { TCls, TInitPrim },
    { TFunc,    TInitPrim },
  };

  const std::initializer_list<std::pair<Type, Type>> couldbe_true{
    { TPrim, TInt },
    { TPrim, TBool },
    { TPrim, TNum },
    { TInitPrim, TNum },
    { TInitPrim, TFalse },
    { TPrim, TCell },
    { TPrim, TOptObj },
    { TPrim, TOptRecord },
    { TPrim, TOptFalse },
  };

  const std::initializer_list<std::pair<Type, Type>> couldbe_false{
    { TPrim, TSStr },
    { TInitPrim, TSStr },
    { TInitPrim, sval(s_test.get()) },
    { TPrim, sval(s_test.get()) },
    { TInitPrim, TUninit },
    { TPrim, TObj },
    { TPrim, TRecord },
    { TPrim, TRes },
    { TPrim, TRFunc },
    { TPrim, TFunc },
    { TPrim, TFuncLike },
    { TPrim, TStrLike },
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
    EXPECT_FALSE(kv.first.couldBe(kv.second))
      << show(kv.first) << " !couldbe " << show(kv.second);
    EXPECT_FALSE(kv.second.couldBe(kv.first))
      << show(kv.first) << " !couldbe " << show(kv.second);
  }

  EXPECT_FALSE(TClsMeth.subtypeOf(TInitPrim));
  EXPECT_FALSE(TPrim.couldBe(TClsMeth));
  EXPECT_FALSE(TPrim.couldBe(TVArrCompat));
  EXPECT_FALSE(TPrim.couldBe(TVecCompat));
  EXPECT_FALSE(TPrim.couldBe(TArrCompat));
}

TEST(Type, CouldBeValues) {
  EXPECT_FALSE(ival(2).couldBe(ival(3)));
  EXPECT_TRUE(ival(2).couldBe(ival(2)));
  EXPECT_FALSE(dict_val(test_dict_vector_value()).couldBe(
               dict_val(test_dict_map_value())));
  EXPECT_TRUE(dict_val(test_dict_vector_value()).couldBe(
              dict_val(test_dict_vector_value())));
  EXPECT_TRUE(dval(2.0).couldBe(dval(2.0)));
  EXPECT_FALSE(dval(2.0).couldBe(dval(3.0)));

  EXPECT_FALSE(sval(s_test.get()).couldBe(sval(s_A.get())));
  EXPECT_TRUE(sval(s_test.get()).couldBe(sval(s_test.get())));
  EXPECT_FALSE(
    sval_nonstatic(s_test.get()).couldBe(sval_nonstatic(s_A.get()))
  );
  EXPECT_TRUE(
    sval_nonstatic(s_test.get()).couldBe(sval_nonstatic(s_test.get()))
  );
  EXPECT_TRUE(sval(s_test.get()).couldBe(sval_nonstatic(s_test.get())));
  EXPECT_TRUE(sval_nonstatic(s_test.get()).couldBe(sval(s_test.get())));
  EXPECT_FALSE(sval(s_test.get()).couldBe(sval_nonstatic(s_A.get())));
  EXPECT_FALSE(sval_nonstatic(s_test.get()).couldBe(sval(s_A.get())));
}

TEST(Type, Unc) {
  EXPECT_TRUE(TInt.subtypeOf(BInitUnc));
  EXPECT_TRUE(TInt.subtypeOf(BUnc));
  EXPECT_TRUE(TDbl.subtypeOf(BInitUnc));
  EXPECT_TRUE(TDbl.subtypeOf(BUnc));
  EXPECT_TRUE(dval(3.0).subtypeOf(BInitUnc));
  EXPECT_TRUE(TUncStrLike.subtypeOf(BInitUnc));

  if (use_lowptr) {
    EXPECT_TRUE(TClsMeth.subtypeOf(BInitUnc));
    EXPECT_TRUE(TVArrCompatSA.subtypeOf(BInitUnc));
    EXPECT_TRUE(TVecCompatSA.subtypeOf(BInitUnc));
    EXPECT_TRUE(TArrCompatSA.subtypeOf(BInitUnc));
  } else {
    EXPECT_FALSE(TClsMeth.subtypeOf(BInitUnc));
    EXPECT_FALSE(TVArrCompatSA.subtypeOf(BInitUnc));
    EXPECT_FALSE(TVecCompatSA.subtypeOf(BInitUnc));
    EXPECT_FALSE(TArrCompatSA.subtypeOf(BInitUnc));
  }

  EXPECT_FALSE(TVArrCompat.subtypeOf(BInitUnc));
  EXPECT_FALSE(TVecCompat.subtypeOf(BInitUnc));
  EXPECT_FALSE(TArrCompat.subtypeOf(BInitUnc));

  const std::initializer_list<std::tuple<Type, Type, bool>> tests{
    { TUnc, TInitUnc, true },
    { TUnc, TInitCell, true },
    { TUnc, TCell, true },
    { TInitUnc, TInt, true },
    { TInitUnc, TOptInt, true },
    { TInitUnc, opt(ival(2)), true },
    { TUnc, TInt, true },
    { TUnc, TOptInt, true },
    { TUnc, opt(ival(2)), true },
    { TNum, TUnc, true },
    { TNum, TInitUnc, true },
    { TUncArrKey, TInitUnc, true },
    { TUncArrKeyCompat, TUncArrKey, true },
    { TStrLike, TInitUnc, true },
    { TUncStrLike, TInitUnc, true },
    { TClsMeth, TInitUnc, use_lowptr },
    { TVArrCompat, TInitUnc, true },
    { TVecCompat, TInitUnc, true },
    { TArrCompat, TInitUnc, true },
    { TVArrCompatSA, TInitUnc, true },
    { TVecCompatSA, TInitUnc, true },
    { TArrCompatSA, TInitUnc, true },
  };
  for (auto const& t : tests) {
    auto const& ty1 = std::get<0>(t);
    auto const& ty2 = std::get<1>(t);
    if (std::get<2>(t)) {
      EXPECT_TRUE(ty1.couldBe(ty2))
        << show(ty1) << " couldBe " << show(ty2);
    } else {
      EXPECT_FALSE(ty1.couldBe(ty2))
        << show(ty1) << " !couldBe " << show(ty2);
    }
  }
}

TEST(Type, DblNan) {
  auto const qnan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_TRUE(dval(qnan).subtypeOf(dval(qnan)));
  EXPECT_TRUE(dval(qnan).couldBe(dval(qnan)));
  EXPECT_TRUE(dval(qnan) == dval(qnan));
}

TEST(Type, Option) {
  auto const program = make_test_program();
  Index index { program.get() };

  EXPECT_TRUE(TTrue.subtypeOf(BOptTrue));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptTrue));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptTrue));

  EXPECT_TRUE(TFalse.subtypeOf(BOptFalse));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptFalse));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptFalse));

  EXPECT_TRUE(TFalse.subtypeOf(BOptBool));
  EXPECT_TRUE(TTrue.subtypeOf(BOptBool));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptBool));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptBool));

  EXPECT_TRUE(ival(3).subtypeOf(BOptInt));
  EXPECT_TRUE(TInt.subtypeOf(BOptInt));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptInt));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptInt));

  EXPECT_TRUE(TDbl.subtypeOf(BOptDbl));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptDbl));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptDbl));
  EXPECT_TRUE(dval(3.0).subtypeOf(BOptDbl));

  EXPECT_TRUE(sval(s_test.get()).subtypeOf(BOptSStr));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(BOptStr));
  EXPECT_TRUE(sval_nonstatic(s_test.get()).subtypeOf(BOptStr));
  EXPECT_TRUE(TSStr.subtypeOf(BOptSStr));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptSStr));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptSStr));
  EXPECT_TRUE(!TStr.subtypeOf(BOptSStr));
  EXPECT_TRUE(TStr.couldBe(BOptSStr));

  EXPECT_TRUE(TStr.subtypeOf(BOptStr));
  EXPECT_TRUE(TSStr.subtypeOf(BOptStr));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(BOptStr));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptStr));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptStr));

  EXPECT_TRUE(TSArr.subtypeOf(BOptSArr));
  EXPECT_TRUE(!TArr.subtypeOf(BOptSArr));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptSArr));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptSArr));

  EXPECT_TRUE(TArr.subtypeOf(BOptArr));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptArr));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptArr));

  EXPECT_TRUE(TObj.subtypeOf(BOptObj));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptObj));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptObj));

  EXPECT_TRUE(TRecord.subtypeOf(BOptRecord));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptRecord));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptRecord));

  EXPECT_TRUE(TRes.subtypeOf(BOptRes));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptRes));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptRes));

  EXPECT_TRUE(TClsMeth.subtypeOf(BOptClsMeth));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptClsMeth));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptClsMeth));

  EXPECT_TRUE(TRClsMeth.subtypeOf(BOptRClsMeth));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptRClsMeth));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptRClsMeth));

  EXPECT_TRUE(TLazyCls.subtypeOf(BOptLazyCls));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptLazyCls));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptLazyCls));

  EXPECT_TRUE(TArrKey.subtypeOf(BOptArrKey));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptArrKey));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptArrKey));

  EXPECT_TRUE(TArrKeyCompat.subtypeOf(BOptArrKeyCompat));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptArrKeyCompat));
  EXPECT_TRUE(!TUninit.subtypeOf(BOptArrKeyCompat));

  for (auto& t : optionals()) EXPECT_EQ(t, opt(unopt(t)));
  for (auto& t : optionals()) EXPECT_TRUE(is_opt(t));
  for (auto& t : all()) {
    if (!is_specialized_dict(t)) {
      auto const found =
        std::find(begin(optionals()), end(optionals()), t) != end(optionals());
      EXPECT_EQ(found, is_opt(t));
    }
  }

  EXPECT_TRUE(is_opt(opt(sval(s_test.get()))));
  EXPECT_TRUE(is_opt(opt(sval_nonstatic(s_test.get()))));
  EXPECT_TRUE(is_opt(opt(ival(2))));
  EXPECT_TRUE(is_opt(opt(dval(2.0))));

  EXPECT_FALSE(is_opt(sval(s_test.get())));
  EXPECT_FALSE(is_opt(sval_nonstatic(s_test.get())));
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
  EXPECT_EQ(opt(sval_nonstatic(s_test.get())),
            union_of(sval_nonstatic(s_test.get()), TInitNull));
  EXPECT_EQ(opt(sval(s_test.get())), union_of(TInitNull, sval(s_test.get())));
  EXPECT_EQ(opt(sval_nonstatic(s_test.get())),
            union_of(TInitNull, sval_nonstatic(s_test.get())));

  EXPECT_EQ(TOptBool, union_of(TOptFalse, TOptTrue));
  EXPECT_EQ(TOptBool, union_of(TOptTrue, TOptFalse));

  EXPECT_EQ(TOptSArr, union_of(TInitNull, TOptSArr));
  EXPECT_EQ(TOptSArr, union_of(TOptSArr, TInitNull));
  EXPECT_EQ(TOptArr, union_of(TOptArr, TInitNull));
  EXPECT_EQ(TOptArr, union_of(TInitNull, TOptArr));

  EXPECT_EQ(TInitUnc, union_of(TOptSArr, TSStr));
  EXPECT_EQ(TInitUnc, union_of(TSStr, TOptSArr));

  EXPECT_EQ(TOptSStr,
            union_of(opt(sval(s_test.get())), opt(sval(s_TestClass.get()))));
  EXPECT_EQ(TOptStr,
            union_of(opt(sval_nonstatic(s_test.get())),
                     opt(sval_nonstatic(s_TestClass.get()))));

  EXPECT_EQ(TOptInt, union_of(opt(ival(2)), opt(ival(3))));
  EXPECT_EQ(TOptDbl, union_of(opt(dval(2.0)), opt(dval(3.0))));
  EXPECT_EQ(TOptNum, union_of(TInitNull, TNum));
  EXPECT_EQ(TOptNum, union_of(TInitNull, union_of(dval(1), ival(0))));

  EXPECT_EQ(TOptTrue, union_of(TInitNull, TTrue));
  EXPECT_EQ(TOptFalse, union_of(TInitNull, TFalse));
  EXPECT_EQ(TOptRes, union_of(TInitNull, TRes));

  EXPECT_EQ(TOptTrue, union_of(TOptTrue, TTrue));
  EXPECT_EQ(TOptFalse, union_of(TOptFalse, TFalse));
  EXPECT_EQ(TOptBool, union_of(TOptTrue, TFalse));

  EXPECT_EQ(TOptClsMeth, union_of(TInitNull, TClsMeth));
  EXPECT_EQ(TOptRClsMeth, union_of(TInitNull, TRClsMeth));
  EXPECT_EQ(TOptClsMethLike, union_of(TInitNull, TClsMethLike));

  EXPECT_EQ(TOptFuncLike, union_of(TInitNull, TFuncLike));
  EXPECT_EQ(TOptFuncLike, union_of(TFunc, TOptRFunc));
  EXPECT_EQ(TOptFuncLike, union_of(TRFunc, TOptFunc));


  EXPECT_EQ(TOptStrLike, union_of(TOptCls, TStr));
  EXPECT_EQ(TOptUncStrLike, union_of(TOptCls, TSStr));

  EXPECT_EQ(TOptVArrCompat, union_of(TOptClsMeth, TVArr));
  EXPECT_EQ(TOptVArrCompatSA, union_of(TOptClsMeth, TSVArr));

  EXPECT_EQ(TOptVecCompat, union_of(TOptClsMeth, TVec));
  EXPECT_EQ(TOptVecCompatSA, union_of(TOptClsMeth, TSVec));

  auto const program = make_test_program();
  Index index { program.get() };
  auto const rcls = index.builtin_class(s_Awaitable.get());

  EXPECT_TRUE(union_of(TObj, opt(objExact(rcls))) == TOptObj);

  auto wh1 = wait_handle(index, TInt);
  auto wh2 = wait_handle(index, ival(2));
  auto wh3 = wait_handle(index, ival(3));

  EXPECT_TRUE(union_of(wh1, wh2) == wh1);
  auto owh1 = opt(wh1);
  auto owh2 = opt(wh2);
  auto owh3 = opt(wh3);

  EXPECT_TRUE(union_of(owh1, owh2) == owh1);
  EXPECT_TRUE(union_of(owh1, wh2) == owh1);
  EXPECT_TRUE(union_of(owh2, wh1) == owh1);

  EXPECT_TRUE(union_of(wh1, owh2) == owh1);
  EXPECT_TRUE(union_of(wh2, owh1) == owh1);

  EXPECT_TRUE(union_of(wh2, owh3) == owh1);
  EXPECT_TRUE(union_of(owh2, wh3) == owh1);
}

TEST(Type, OptTV) {
  EXPECT_TRUE(!tv(opt(ival(2))));
  EXPECT_TRUE(!tv(opt(sval(s_test.get()))));
  EXPECT_TRUE(!tv(opt(sval_nonstatic(s_test.get()))));
  EXPECT_TRUE(!tv(opt(dval(2.0))));
  EXPECT_TRUE(!tv(TOptFalse));
  EXPECT_TRUE(!tv(TOptTrue));
  for (auto& x : optionals()) {
    EXPECT_TRUE(!tv(x));
  }
}

TEST(Type, OptCouldBe) {
  for (auto& x : optionals()) EXPECT_TRUE(x.couldBe(unopt(x)));

  const std::initializer_list<std::pair<Type, Type>> true_cases{
    { opt(sval(s_test.get())), TStr },
    { opt(sval(s_test.get())), TInitNull },
    { opt(sval(s_test.get())), TSStr },
    { opt(sval(s_test.get())), sval(s_test.get()) },
    { opt(sval(s_test.get())), sval_nonstatic(s_test.get()) },

    { opt(sval_nonstatic(s_test.get())), TStr },
    { opt(sval_nonstatic(s_test.get())), TInitNull },
    { opt(sval_nonstatic(s_test.get())), TSStr },
    { opt(sval_nonstatic(s_test.get())), sval_nonstatic(s_test.get()) },
    { opt(sval_nonstatic(s_test.get())), sval(s_test.get()) },

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

  for (auto kv : true_cases) {
    EXPECT_TRUE(kv.first.couldBe(kv.second))
      << show(kv.first) << " couldBe " << show(kv.second)
      << " should be true";
  }

  const std::initializer_list<std::pair<Type, Type>> false_cases{
    { opt(ival(2)), TDbl },
    { opt(dval(2.0)), TInt },
    { opt(TFalse), TTrue },
    { opt(TTrue), TFalse },
    { TFalse, opt(TNum) },
  };

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

  for (auto& x : optionals()) {
    EXPECT_TRUE(x.couldBe(unopt(x)));
    EXPECT_TRUE(x.couldBe(BInitNull));
    EXPECT_TRUE(!x.couldBe(BUninit));
    for (auto& y : optionals()) {
      EXPECT_TRUE(x.couldBe(y));
    }
  }
}

TEST(Type, SpecificExamples) {
  // Random examples to stress option types, values, etc:

  EXPECT_TRUE(!TInt.subtypeOf(ival(1)));

  EXPECT_TRUE(TInitCell.couldBe(ival(1)));
  EXPECT_TRUE(ival(2).subtypeOf(BInt));
  EXPECT_TRUE(!ival(2).subtypeOf(BBool));
  EXPECT_TRUE(ival(3).subtypeOrNull(BInt));
  EXPECT_TRUE(TInt.subtypeOrNull(BInt));
  EXPECT_TRUE(!TBool.subtypeOrNull(BInt));
  EXPECT_TRUE(TInitNull.subtypeOf(BOptInt));
  EXPECT_TRUE(!TNull.subtypeOf(BOptInt));
  EXPECT_TRUE(TNull.couldBe(BOptInt));
  EXPECT_TRUE(TNull.couldBe(BOptBool));

  EXPECT_TRUE(TInitNull.subtypeOf(BInitCell));
  EXPECT_TRUE(TInitNull.subtypeOf(BCell));
  EXPECT_TRUE(!TUninit.subtypeOf(BInitNull));

  EXPECT_TRUE(ival(3).subtypeOrNull(BInt));
  EXPECT_TRUE(ival(3).subtypeOf(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(BInt));
  EXPECT_TRUE(TInitNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TInitNull.subtypeOf(opt(ival(3))));
  EXPECT_TRUE(!TNull.subtypeOf(opt(ival(3))));

  EXPECT_EQ(intersection_of(TClsMeth, TInitUnc),
            use_lowptr ? TClsMeth : TBottom);
  EXPECT_EQ(intersection_of(TVecCompat, TInitUnc),
            use_lowptr ? TVecCompatSA : TSVec);
  EXPECT_EQ(intersection_of(TVArrCompat, TInitUnc),
            use_lowptr ? TVArrCompatSA : TSVArr);
  EXPECT_EQ(intersection_of(TArrCompat, TInitUnc),
            use_lowptr ? TArrCompatSA : TSArr);

  auto test_map_a = MapElems{};
  test_map_a[tv(s_A)] = TDbl;
  test_map_a[tv(s_B)] = TBool;

  auto test_map_b = MapElems{};
  test_map_b[tv(s_A)] = TObj;
  test_map_b[tv(s_B)] = TRes;

  auto const disjointArrSpecs = std::vector<Type>{
    dict_packedn(TInt),
    dict_packedn(TStr),
    dict_packed({TDbl}),
    dict_packed({TBool}),
    dict_n(TStr, TStr),
    dict_n(TStr, TInt),
    dict_map(test_map_a),
    dict_map(test_map_b)
  };
  for (auto const& t1 : disjointArrSpecs) {
    for (auto const& t2 : disjointArrSpecs) {
      if (t1 == t2) continue;
      EXPECT_FALSE(t1.couldBe(t2));
      EXPECT_FALSE(t2.couldBe(t1));

      auto const t3 = union_of(t1, TDictE);
      auto const t4 = union_of(t2, TDictE);
      EXPECT_TRUE(t3.couldBe(t4));
      EXPECT_TRUE(t4.couldBe(t3));
      EXPECT_FALSE(t3.subtypeOf(t4));
      EXPECT_FALSE(t4.subtypeOf(t3));
      EXPECT_EQ(intersection_of(t3, t4), TDictE);
      EXPECT_EQ(intersection_of(t4, t3), TDictE);

      auto const t5 = opt(t1);
      auto const t6 = opt(t2);
      EXPECT_TRUE(t5.couldBe(t6));
      EXPECT_TRUE(t6.couldBe(t5));
      EXPECT_FALSE(t5.subtypeOf(t6));
      EXPECT_FALSE(t6.subtypeOf(t5));
      EXPECT_EQ(intersection_of(t5, t6), TInitNull);
      EXPECT_EQ(intersection_of(t6, t5), TInitNull);

      auto const t7 = opt(t3);
      auto const t8 = opt(t4);
      EXPECT_TRUE(t7.couldBe(t8));
      EXPECT_TRUE(t8.couldBe(t7));
      EXPECT_FALSE(t7.subtypeOf(t8));
      EXPECT_FALSE(t8.subtypeOf(t7));
      EXPECT_EQ(intersection_of(t7, t8), opt(TDictE));
      EXPECT_EQ(intersection_of(t8, t7), opt(TDictE));
    }
  }
}

TEST(Type, IndexBased) {
  auto const program = make_test_program();
  auto const unit = program->units.back().get();
  auto const func = [&]() -> php::Func* {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return f.get();
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{program.get()};

  auto const cls = idx.resolve_class(ctx, s_TestClass.get());
  if (!cls) ADD_FAILURE();
  auto const clsBase = idx.resolve_class(ctx, s_Base.get());
  if (!clsBase) ADD_FAILURE();

  // Need to use base because final records are always exact.
  auto const rec = idx.resolve_record(s_UniqueRecBase.get());
  if (!rec) ADD_FAILURE();

  auto const objExactTy = objExact(*cls);
  auto const subObjTy   = subObj(*cls);
  auto const clsExactTy = clsExact(*cls);
  auto const subClsTy   = subCls(*cls);
  auto const objExactBaseTy = objExact(*clsBase);
  auto const subObjBaseTy   = subObj(*clsBase);

  auto const exactRecTy = exactRecord(*rec);
  auto const subRecTy   = subRecord(*rec);

  // Basic relationship between the class types and object types.
  EXPECT_EQ(objcls(objExactTy), clsExactTy);
  EXPECT_EQ(objcls(subObjTy), subClsTy);

  // =TestClass <: <=TestClass, and not vice versa. Same for records.
  EXPECT_TRUE(objExactTy.subtypeOf(subObjTy));
  EXPECT_TRUE(!subObjTy.subtypeOf(objExactTy));
  EXPECT_TRUE(exactRecTy.subtypeOf(subRecTy));
  EXPECT_TRUE(!subRecTy.subtypeOf(exactRecTy));
  // =TestClass <: <=TestClass, and not vice versa.
  EXPECT_TRUE(clsExactTy.subtypeOf(subClsTy));
  EXPECT_TRUE(!subClsTy.subtypeOf(clsExactTy));

  // =TestClass couldBe <= TestClass, and vice versa. Same for records.
  EXPECT_TRUE(objExactTy.couldBe(subObjTy));
  EXPECT_TRUE(subObjTy.couldBe(objExactTy));
  EXPECT_TRUE(exactRecTy.couldBe(subRecTy));
  EXPECT_TRUE(subRecTy.couldBe(exactRecTy));
  EXPECT_TRUE(clsExactTy.couldBe(subClsTy));
  EXPECT_TRUE(subClsTy.couldBe(clsExactTy));

  // Foo= and Foo<= are both subtypes of Foo, and couldBe Foo.
  EXPECT_TRUE(objExactTy.subtypeOf(BObj));
  EXPECT_TRUE(subObjTy.subtypeOf(BObj));
  EXPECT_TRUE(objExactTy.couldBe(BObj));
  EXPECT_TRUE(subObjTy.couldBe(BObj));
  EXPECT_TRUE(TObj.couldBe(objExactTy));
  EXPECT_TRUE(TObj.couldBe(subObjTy));
  EXPECT_TRUE(clsExactTy.subtypeOf(BCls));
  EXPECT_TRUE(subClsTy.subtypeOf(BCls));
  EXPECT_TRUE(clsExactTy.couldBe(BCls));
  EXPECT_TRUE(subClsTy.couldBe(BCls));
  EXPECT_TRUE(TCls.couldBe(clsExactTy));
  EXPECT_TRUE(TCls.couldBe(subClsTy));
  EXPECT_TRUE(exactRecTy.subtypeOf(BRecord));
  EXPECT_TRUE(subRecTy.subtypeOf(BRecord));
  EXPECT_TRUE(exactRecTy.couldBe(BRecord));
  EXPECT_TRUE(subRecTy.couldBe(BRecord));
  EXPECT_TRUE(TRecord.couldBe(exactRecTy));
  EXPECT_TRUE(TRecord.couldBe(subRecTy));

  // These checks are relevant for class to key conversions
  EXPECT_TRUE(clsExactTy.subtypeOf(BOptCls | BOptLazyCls));
  EXPECT_TRUE(subClsTy.subtypeOf(BOptCls | BOptLazyCls));
  EXPECT_TRUE(TCls.subtypeOf(BOptCls | BOptLazyCls));
  EXPECT_TRUE(TLazyCls.subtypeOf(BOptCls | BOptLazyCls));
  EXPECT_TRUE(clsExactTy.couldBe(BOptCls | BOptLazyCls));
  EXPECT_TRUE(subClsTy.couldBe(BOptCls | BOptLazyCls));
  auto keyTy1 = union_of(clsExactTy, sval(s_TestClass.get()));
  EXPECT_TRUE(keyTy1.couldBe(BOptCls | BOptLazyCls));
  auto keyTy2 = union_of(TLazyCls, sval(s_TestClass.get()));
  EXPECT_TRUE(keyTy2.couldBe(BOptCls | BOptLazyCls));
  EXPECT_FALSE(TSStr.couldBe(BOptCls | BOptLazyCls));
  EXPECT_FALSE(TStr.couldBe(BOptCls | BOptLazyCls));


  // Obj= and Obj<= both couldBe ?Obj, and vice versa.
  EXPECT_TRUE(objExactTy.couldBe(BOptObj));
  EXPECT_TRUE(subObjTy.couldBe(BOptObj));
  EXPECT_TRUE(TOptObj.couldBe(objExactTy));
  EXPECT_TRUE(TOptObj.couldBe(subObjTy));
  EXPECT_TRUE(exactRecTy.couldBe(BOptRecord));
  EXPECT_TRUE(subRecTy.couldBe(BOptRecord));
  EXPECT_TRUE(TOptRecord.couldBe(exactRecTy));
  EXPECT_TRUE(TOptRecord.couldBe(subRecTy));

  // Obj= and Obj<= are subtypes of ?Obj.
  EXPECT_TRUE(objExactTy.subtypeOrNull(BObj));
  EXPECT_TRUE(subObjTy.subtypeOrNull(BObj));
  EXPECT_TRUE(exactRecTy.subtypeOrNull(BRecord));
  EXPECT_TRUE(subRecTy.subtypeOrNull(BRecord));

  // Obj= is a subtype of ?Obj=, and also ?Obj<=.
  EXPECT_TRUE(objExactTy.subtypeOf(opt(objExactTy)));
  EXPECT_TRUE(objExactTy.subtypeOf(opt(subObjTy)));
  EXPECT_TRUE(!opt(objExactTy).subtypeOf(objExactTy));
  EXPECT_TRUE(!opt(subObjTy).subtypeOf(objExactTy));
  EXPECT_TRUE(exactRecTy.subtypeOf(opt(exactRecTy)));
  EXPECT_TRUE(exactRecTy.subtypeOf(opt(subRecTy)));
  EXPECT_TRUE(!opt(exactRecTy).subtypeOf(exactRecTy));
  EXPECT_TRUE(!opt(subRecTy).subtypeOf(exactRecTy));

  // Obj= couldBe ?Obj= and ?Obj<=, and vice versa.
  EXPECT_TRUE(objExactTy.couldBe(opt(objExactTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(objExactTy));
  EXPECT_TRUE(objExactTy.couldBe(opt(subObjTy)));
  EXPECT_TRUE(opt(subObjTy).couldBe(objExactTy));
  EXPECT_TRUE(exactRecTy.couldBe(opt(exactRecTy)));
  EXPECT_TRUE(opt(exactRecTy).couldBe(exactRecTy));
  EXPECT_TRUE(exactRecTy.couldBe(opt(subRecTy)));
  EXPECT_TRUE(opt(subRecTy).couldBe(exactRecTy));

  // Obj<= is not a subtype of ?Obj=, it is overlapping but
  // potentially contains other types.  (We might eventually check
  // whether objects are final as part of this, but not right now.)
  EXPECT_TRUE(!subObjTy.subtypeOf(opt(objExactTy)));
  EXPECT_TRUE(!opt(objExactTy).subtypeOf(subObjTy));
  EXPECT_TRUE(!subRecTy.subtypeOf(opt(exactRecTy)));
  EXPECT_TRUE(!opt(exactRecTy).subtypeOf(subRecTy));

  // Obj<= couldBe ?Obj= and vice versa.
  EXPECT_TRUE(subObjTy.couldBe(opt(objExactTy)));
  EXPECT_TRUE(opt(objExactTy).couldBe(subObjTy));
  EXPECT_TRUE(subRecTy.couldBe(opt(exactRecTy)));
  EXPECT_TRUE(opt(exactRecTy).couldBe(subRecTy));

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
  auto const program = make_test_program();
  auto const unit = program->units.back().get();
  auto const func = [&]() -> php::Func* {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return f.get();
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{program.get()};

  // load classes in hierarchy
  auto const clsBase = idx.resolve_class(ctx, s_Base.get());
  if (!clsBase) ADD_FAILURE();
  auto const clsA = idx.resolve_class(ctx, s_A.get());
  if (!clsA) ADD_FAILURE();
  auto const clsB = idx.resolve_class(ctx, s_B.get());
  if (!clsB) ADD_FAILURE();
  auto const clsAA = idx.resolve_class(ctx, s_AA.get());
  if (!clsAA) ADD_FAILURE();
  auto const clsAB = idx.resolve_class(ctx, s_AB.get());
  if (!clsAB) ADD_FAILURE();
  auto const clsBA = idx.resolve_class(ctx, s_BA.get());
  if (!clsBA) ADD_FAILURE();
  auto const clsBB = idx.resolve_class(ctx, s_BB.get());
  if (!clsBB) ADD_FAILURE();
  auto const clsBAA = idx.resolve_class(ctx, s_BAA.get());
  if (!clsBAA) ADD_FAILURE();
  auto const clsTestClass = idx.resolve_class(ctx, s_TestClass.get());
  if (!clsTestClass) ADD_FAILURE();

  auto const recBaseUnique = idx.resolve_record(s_UniqueRecBase.get());
  if (!recBaseUnique) ADD_FAILURE();
  auto const recUnique = idx.resolve_record(s_UniqueRec.get());
  if (!recUnique) ADD_FAILURE();
  auto const recUniqueA = idx.resolve_record(s_UniqueRecA.get());
  if (!recUniqueA) ADD_FAILURE();

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

  auto const exactRecUniqueBaseTy = exactRecord(*recBaseUnique);
  auto const subRecUniqueBaseTy = subRecord(*recBaseUnique);
  auto const exactRecUniqueTy = exactRecord(*recUnique);
  auto const subRecUniqueTy   = subRecord(*recUnique);
  auto const exactRecUniqueATy = exactRecord(*recUniqueA);
  auto const subRecUniqueATy   = subRecord(*recUniqueA);

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

  // check union_of
  // union of subCls
  EXPECT_EQ(union_of(subClsATy, subClsBTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, subClsABTy), subClsATy);
  EXPECT_EQ(union_of(subClsATy, subClsBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsBAATy, subClsBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsBAATy, subClsBBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsAATy, subClsBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, subClsTestClassTy), TCls);
  // union of subCls and clsExact mixed
  EXPECT_EQ(union_of(clsExactATy, subClsBTy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsAATy, clsExactABTy), subClsATy);
  EXPECT_EQ(union_of(clsExactATy, subClsBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(subClsBAATy, clsExactBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactBAATy, subClsBBTy), subClsBTy);
  EXPECT_EQ(union_of(subClsAATy, clsExactBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, subClsTestClassTy), TCls);
  // union of clsExact
  EXPECT_EQ(union_of(clsExactATy, clsExactBTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, clsExactABTy), subClsATy);
  EXPECT_EQ(union_of(clsExactATy, clsExactBAATy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactBAATy, clsExactBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactBAATy, clsExactBBTy), subClsBTy);
  EXPECT_EQ(union_of(clsExactAATy, clsExactBaseTy), subClsBaseTy);
  EXPECT_EQ(union_of(clsExactAATy, subClsTestClassTy), TCls);
  // union of subObj
  EXPECT_EQ(union_of(subObjATy, subObjBTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, subObjABTy), subObjATy);
  EXPECT_EQ(union_of(subObjATy, subObjBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjBAATy, subObjBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjBAATy, subObjBBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjAATy, subObjBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, subObjTestClassTy), TObj);
  EXPECT_EQ(union_of(subRecUniqueTy, subRecUniqueATy), subRecUniqueBaseTy);
  // union of subObj and objExact mixed
  EXPECT_EQ(union_of(objExactATy, subObjBTy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjAATy, objExactABTy), subObjATy);
  EXPECT_EQ(union_of(objExactATy, subObjBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(subObjBAATy, objExactBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactBAATy, subObjBBTy), subObjBTy);
  EXPECT_EQ(union_of(subObjAATy, objExactBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, subObjTestClassTy), TObj);
  EXPECT_EQ(union_of(subRecUniqueATy, exactRecUniqueTy), subRecUniqueBaseTy);
  // union of objExact
  EXPECT_EQ(union_of(objExactATy, objExactBTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, objExactABTy), subObjATy);
  EXPECT_EQ(union_of(objExactATy, objExactBAATy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactBAATy, objExactBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactBAATy, objExactBBTy), subObjBTy);
  EXPECT_EQ(union_of(objExactAATy, objExactBaseTy), subObjBaseTy);
  EXPECT_EQ(union_of(objExactAATy, objExactTestClassTy), TObj);
  EXPECT_EQ(union_of(subRecUniqueATy, exactRecUniqueTy), subRecUniqueBaseTy);
  // optional sub obj
  EXPECT_EQ(union_of(opt(subObjATy), opt(subObjBTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(subObjABTy)), opt(subObjATy));
  EXPECT_EQ(union_of(opt(subObjATy), subObjBAATy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), opt(subObjBTy)), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), subObjBBTy), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjAATy), opt(subObjBaseTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(subObjTestClassTy)), opt(TObj));
  EXPECT_EQ(union_of(opt(subRecUniqueATy), exactRecUniqueTy),
            opt(subRecUniqueBaseTy));
  EXPECT_EQ(union_of(opt(subRecUniqueATy), opt(exactRecUniqueTy)),
            opt(subRecUniqueBaseTy));
  // optional sub and exact obj mixed
  EXPECT_EQ(union_of(opt(objExactATy), subObjBTy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjAATy, opt(objExactABTy)), opt(subObjATy));
  EXPECT_EQ(union_of(opt(objExactATy), objExactBAATy), opt(subObjBaseTy));
  EXPECT_EQ(union_of(subObjBAATy, opt(objExactBTy)), opt(subObjBTy));
  EXPECT_EQ(union_of(opt(subObjBAATy), objExactBBTy), opt(subObjBTy));
  EXPECT_EQ(union_of(objExactAATy, opt(objExactBaseTy)), opt(subObjBaseTy));
  EXPECT_EQ(union_of(opt(subObjAATy), objExactTestClassTy), opt(TObj));
}

TEST(Type, Interface) {
  auto const program = make_test_program();
  auto const unit = program->units.back().get();
  auto const func = [&]() -> php::Func* {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return f.get();
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{program.get()};

  // load classes in hierarchy
  auto const clsIA = idx.resolve_class(ctx, s_IA.get());
  if (!clsIA) ADD_FAILURE();
  auto const clsIB = idx.resolve_class(ctx, s_IB.get());
  if (!clsIB) ADD_FAILURE();
  auto const clsIAA = idx.resolve_class(ctx, s_IAA.get());
  if (!clsIAA) ADD_FAILURE();
  auto const clsA = idx.resolve_class(ctx, s_A.get());
  if (!clsA) ADD_FAILURE();
  auto const clsAA = idx.resolve_class(ctx, s_AA.get());
  if (!clsAA) ADD_FAILURE();

  // make sometypes and objects
  auto const subObjIATy   = subObj(*clsIA);
  auto const subClsIATy   = subCls(*clsIA);
  auto const subObjIAATy  = subObj(*clsIAA);
  auto const subClsIAATy  = subCls(*clsIAA);
  auto const subObjIBTy   = subObj(*clsIB);
  auto const subObjATy    = subObj(*clsA);
  auto const clsExactATy  = clsExact(*clsA);
  auto const subClsATy    = subCls(*clsA);
  auto const subObjAATy   = subObj(*clsAA);
  auto const subClsAATy   = subCls(*clsAA);
  auto const exactObjATy  = objExact(*clsA);
  auto const exactObjAATy = objExact(*clsAA);

  EXPECT_TRUE(subClsATy.subtypeOf(objcls(subObjIATy)));
  EXPECT_TRUE(subClsATy.couldBe(objcls(subObjIATy)));
  EXPECT_TRUE(objcls(subObjATy).strictSubtypeOf(subClsIATy));
  EXPECT_TRUE(subClsAATy.subtypeOf(objcls(subObjIAATy)));
  EXPECT_TRUE(subClsAATy.couldBe(objcls(subObjIAATy)));
  EXPECT_TRUE(objcls(subObjAATy).strictSubtypeOf(objcls(subObjIAATy)));

  EXPECT_FALSE(subClsATy.subtypeOf(objcls(subObjIAATy)));
  EXPECT_FALSE(objcls(subObjATy).strictSubtypeOf(objcls(subObjIAATy)));

  EXPECT_EQ(intersection_of(subObjIAATy, subObjAATy), subObjAATy);
  EXPECT_EQ(intersection_of(subObjIAATy, exactObjAATy), exactObjAATy);
  EXPECT_EQ(intersection_of(subObjIAATy, exactObjATy), TBottom);
  EXPECT_EQ(intersection_of(subObjIAATy, subObjATy), subObjATy);
  EXPECT_EQ(intersection_of(subObjIAATy, subObjIBTy), TObj);

  // We don't support couldBe or intersection intelligently for
  // interfaces quite yet, so here are some tests that may start
  // failing if we ever do:
  EXPECT_TRUE(clsExactATy.couldBe(objcls(subObjIAATy)));

  EXPECT_TRUE(union_of(opt(exactObjATy), opt(subObjIATy)) == opt(subObjIATy));
  // Since we have invariants in the index that types only improve, it is
  // important that the below union is more or equally refined than the
  // above union.
  EXPECT_TRUE(union_of(opt(exactObjATy), subObjIATy) == opt(subObjIATy));
}

TEST(Type, WaitH) {
  auto const program = make_test_program();
  Index index { program.get() };

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

  auto const rcls   = index.builtin_class(s_Awaitable.get());
  auto const twhobj = subObj(rcls);
  EXPECT_TRUE(wait_handle(index, TTop).subtypeOf(twhobj));

  // Some test cases with optional wait handles.
  auto const optWH = opt(wait_handle(index, ival(2)));
  EXPECT_TRUE(is_opt(optWH));
  EXPECT_TRUE(TInitNull.subtypeOf(optWH));
  EXPECT_TRUE(optWH.subtypeOrNull(BObj));
  EXPECT_TRUE(optWH.subtypeOf(opt(twhobj)));
  EXPECT_TRUE(wait_handle(index, ival(2)).subtypeOf(optWH));
  EXPECT_FALSE(optWH.subtypeOf(wait_handle(index, ival(2))));
  EXPECT_TRUE(optWH.couldBe(wait_handle(index, ival(2))));

  // union_of(WaitH<T>, Obj<=Awaitable) == Obj<=Awaitable
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
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\mixed")), TInitCell);
  EXPECT_EQ(from_hni_constraint(makeStaticString("HH\\arraykey")), TArrKey);
  EXPECT_EQ(from_hni_constraint(makeStaticString("?HH\\arraykey")), TOptArrKey);

  // These are conservative, but we're testing them that way.  If we
  // make the function better later we'll remove the tests.
  EXPECT_EQ(from_hni_constraint(makeStaticString("stdClass")), TCell);
  EXPECT_EQ(from_hni_constraint(makeStaticString("?stdClass")), TCell);
  EXPECT_EQ(from_hni_constraint(makeStaticString("fooooo")), TCell);
  EXPECT_EQ(from_hni_constraint(makeStaticString("")), TCell);
}

TEST(Type, DictPacked1) {
  auto const a1 = dict_packed({ival(2), TSStr, TInt});
  auto const a2 = dict_packed({TInt,    TStr,  TInitCell});
  auto const s1 = sdict_packed({ival(2), TSStr, TInt});
  auto const s2 = sdict_packed({TInt,    TStr,  TInitCell});

  for (auto& a : { a1, s1, a2, s2 }) {
    EXPECT_TRUE(a.subtypeOf(BDict));
    EXPECT_TRUE(a.subtypeOf(a));
    EXPECT_EQ(a, a);
  }

  // Subtype stuff.

  EXPECT_TRUE(a1.subtypeOf(BDict));
  EXPECT_FALSE(a1.subtypeOf(BSDict));

  EXPECT_TRUE(s1.subtypeOf(BDict));
  EXPECT_TRUE(s1.subtypeOf(BSDict));

  EXPECT_TRUE(a1.subtypeOf(a2));
  EXPECT_TRUE(s1.subtypeOf(s2));
  EXPECT_TRUE(s1.subtypeOf(a1));

  // Could be stuff.

  EXPECT_TRUE(s1.couldBe(a1));
  EXPECT_TRUE(s2.couldBe(a2));

  EXPECT_TRUE(a1.couldBe(a2));
  EXPECT_TRUE(a2.couldBe(a1));
  EXPECT_TRUE(s1.couldBe(a2));
  EXPECT_TRUE(s2.couldBe(a1));

  EXPECT_TRUE(s1.couldBe(s2));
  EXPECT_TRUE(s2.couldBe(s1));
}

TEST(Type, OptDictPacked1) {
  auto const a1 = opt(dict_packed({ival(2), TSStr, TInt}));
  auto const a2 = opt(dict_packed({TInt,    TStr,  TInitCell}));
  auto const s1 = opt(sdict_packed({ival(2), TSStr, TInt}));
  auto const s2 = opt(sdict_packed({TInt,    TStr,  TInitCell}));

  for (auto& a : { a1, s1, a2, s2 }) {
    EXPECT_TRUE(a.subtypeOrNull(BDict));
    EXPECT_TRUE(a.subtypeOf(a));
    EXPECT_EQ(a, a);
  }

  // Subtype stuff.

  EXPECT_TRUE(a1.subtypeOrNull(BDict));
  EXPECT_FALSE(a1.subtypeOrNull(BSDict));

  EXPECT_TRUE(s1.subtypeOrNull(BDict));
  EXPECT_TRUE(s1.subtypeOrNull(BSDict));

  EXPECT_TRUE(a1.subtypeOf(a2));
  EXPECT_TRUE(s1.subtypeOf(s2));
  EXPECT_TRUE(s1.subtypeOf(a1));

  // Could be stuff.

  EXPECT_TRUE(s1.couldBe(a1));
  EXPECT_TRUE(s2.couldBe(a2));

  EXPECT_TRUE(a1.couldBe(a2));
  EXPECT_TRUE(a2.couldBe(a1));
  EXPECT_TRUE(s1.couldBe(a2));
  EXPECT_TRUE(s2.couldBe(a1));

  EXPECT_TRUE(s1.couldBe(s2));
  EXPECT_TRUE(s2.couldBe(s1));
}

TEST(Type, DictPacked2) {
  {
    auto const a1 = dict_packed({TInt, TInt, TDbl});
    auto const a2 = dict_packed({TInt, TInt});
    EXPECT_FALSE(a1.subtypeOf(a2));
    EXPECT_FALSE(a1.couldBe(a2));
  }

  {
    auto const a1 = dict_packed({TInitCell, TInt});
    auto const a2 = dict_packed({TInt, TInt});
    EXPECT_TRUE(a1.couldBe(a2));
    EXPECT_TRUE(a2.subtypeOf(a1));
  }

  {
    auto const a1 = dict_packed({TInt, TInt, TInt});
    auto const s1 = sdict_packed({TInt, TInt, TInt});
    auto const s2 = dict_val(test_dict_vector_value());
    EXPECT_TRUE(s2.subtypeOf(a1));
    EXPECT_TRUE(s2.subtypeOf(s1));
    EXPECT_TRUE(s2.couldBe(a1));
    EXPECT_TRUE(s2.couldBe(s1));
  }

  {
    auto const s1 = sdict_packed({ival(42), ival(23), ival(12)});
    auto const s2 = dict_val(test_dict_vector_value());
    auto const s3 = sdict_packed({TInt});
    auto const a4 = sdict_packed({TInt});
    auto const a5 = dict_packed({ival(42), ival(23), ival(12)});
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
  }
}

TEST(Type, DictPackedUnion) {
  {
    auto const a1 = dict_packed({TInt, TDbl});
    auto const a2 = dict_packed({TDbl, TInt});
    EXPECT_EQ(union_of(a1, a2), dict_packed({TNum, TNum}));
  }

  {
    auto const s1 = sdict_packed({TInt, TDbl});
    auto const s2 = sdict_packed({TDbl, TInt});
    EXPECT_EQ(union_of(s1, s1), s1);
    EXPECT_EQ(union_of(s1, s2), sdict_packed({TNum, TNum}));
  }

  {
    auto const s1 = sdict_packed({TInt});
    auto const s2 = sdict_packed({TDbl, TDbl});
    EXPECT_EQ(union_of(s1, s2), sdict_packedn(TNum));
  }

  {
    auto const s1 = dict_val(test_dict_vector_value());
    auto const s2 = sdict_packed({TInt, TInt, TInt});
    auto const s3 = sdict_packed({TInt, TNum, TInt});
    EXPECT_EQ(union_of(s1, s2), s2);
    EXPECT_EQ(union_of(s1, s3), s3);
  }

  {
    auto const s1  = sdict_packed({TInt});
    auto const os1 = opt(s1);
    EXPECT_EQ(union_of(s1, TInitNull), os1);
    EXPECT_EQ(union_of(os1, s1), os1);
    EXPECT_EQ(union_of(TInitNull, s1), os1);
    EXPECT_EQ(union_of(os1, os1), os1);
  }

  {
    auto const s1 = sdict_packed({TInt});
    EXPECT_EQ(union_of(s1, TObj), TInitCell);
    EXPECT_EQ(union_of(s1, TSDict), TSDict);
  }

  {
    auto const s1 = dict_val(test_dict_vector_value());
    auto const s2 = dict_val(test_dict_vector_value2());
    EXPECT_EQ(union_of(s1, s2), sdict_packed({ival(42), TNum, ival(12)}));
  }
}

TEST(Type, DictPackedN) {
  auto const a1 = dict_val(test_dict_vector_value());
  auto const a2 = sdict_packed({TInt, TInt});
  EXPECT_EQ(union_of(a1, a2), sdict_packedn(TInt));

  auto const s2 = sdict_packed({TInt, TInt});
  EXPECT_TRUE(s2.subtypeOf(sdict_packedn(TInt)));
  EXPECT_FALSE(s2.subtypeOf(sdict_packedn(TDbl)));
  EXPECT_TRUE(s2.subtypeOf(sdict_packedn(TNum)));
  EXPECT_TRUE(s2.subtypeOf(dict_packedn(TInt)));
  EXPECT_TRUE(s2.subtypeOf(opt(dict_packedn(TInt))));

  EXPECT_TRUE(s2.couldBe(dict_packedn(TInt)));
  EXPECT_TRUE(s2.couldBe(dict_packedn(TInitCell)));

  auto const sn1 = sdict_packedn(TInt);
  auto const sn2 = sdict_packedn(TInitNull);

  EXPECT_EQ(union_of(sn1, sn2), sdict_packedn(TOptInt));
  EXPECT_EQ(union_of(sn1, TInitNull), opt(sn1));
  EXPECT_EQ(union_of(TInitNull, sn1), opt(sn1));
  EXPECT_FALSE(sn2.couldBe(sn1));
  EXPECT_FALSE(sn1.couldBe(sn2));

  auto const sn3 = sdict_packedn(TInitCell);
  EXPECT_TRUE(sn1.couldBe(sn3));
  EXPECT_TRUE(sn2.couldBe(sn3));
  EXPECT_TRUE(sn3.couldBe(sn1));
  EXPECT_TRUE(sn3.couldBe(sn2));

  EXPECT_TRUE(s2.couldBe(sn3));
  EXPECT_TRUE(s2.couldBe(sn1));
  EXPECT_FALSE(s2.couldBe(sn2));
}

TEST(Type, DictStruct) {
  auto test_map_a          = MapElems{};
  test_map_a[tv(s_test)]   = ival(2);

  auto test_map_b          = MapElems{};
  test_map_b[tv(s_test)]   = TInt;

  auto test_map_c          = MapElems{};
  test_map_c[tv(s_test)]   = ival(2);
  test_map_c[tv(s_A)]      = TInt;
  test_map_c[tv(s_B)]      = TDbl;

  auto const ta = dict_map(test_map_a);
  auto const tb = dict_map(test_map_b);
  auto const tc = dict_map(test_map_c);

  EXPECT_FALSE(ta.subtypeOf(tc));
  EXPECT_FALSE(tc.subtypeOf(ta));
  EXPECT_TRUE(ta.subtypeOf(tb));
  EXPECT_FALSE(tb.subtypeOf(ta));
  EXPECT_TRUE(ta.couldBe(tb));
  EXPECT_TRUE(tb.couldBe(ta));
  EXPECT_FALSE(tc.couldBe(ta));
  EXPECT_FALSE(tc.couldBe(tb));

  EXPECT_TRUE(ta.subtypeOf(BDict));
  EXPECT_TRUE(tb.subtypeOf(BDict));
  EXPECT_TRUE(tc.subtypeOf(BDict));

  auto const sa = sdict_map(test_map_a);
  auto const sb = sdict_map(test_map_b);
  auto const sc = sdict_map(test_map_c);

  EXPECT_FALSE(sa.subtypeOf(sc));
  EXPECT_FALSE(sc.subtypeOf(sa));
  EXPECT_TRUE(sa.subtypeOf(sb));
  EXPECT_FALSE(sb.subtypeOf(sa));
  EXPECT_TRUE(sa.couldBe(sb));
  EXPECT_TRUE(sb.couldBe(sa));
  EXPECT_FALSE(sc.couldBe(sa));
  EXPECT_FALSE(sc.couldBe(sb));

  EXPECT_TRUE(sa.subtypeOf(BSDict));
  EXPECT_TRUE(sb.subtypeOf(BSDict));
  EXPECT_TRUE(sc.subtypeOf(BSDict));

  auto test_map_d          = MapElems{};
  test_map_d[tv(s_A)]      = sval(s_B.get());
  test_map_d[tv(s_test)]   = ival(12);
  auto const sd = sdict_map(test_map_d);
  EXPECT_EQ(sd, dict_val(test_dict_map_value()));

  auto test_map_e          = MapElems{};
  test_map_e[tv(s_A)]      = TSStr;
  test_map_e[tv(s_test)]   = TNum;
  auto const se = sdict_map(test_map_e);
  EXPECT_TRUE(dict_val(test_dict_map_value()).subtypeOf(se));
  EXPECT_TRUE(se.couldBe(dict_val(test_dict_map_value())));
}

TEST(Type, DictMapN) {
  auto const test_map = dict_val(test_dict_map_value());
  EXPECT_TRUE(test_map != dict_n(TSStr, TInitUnc));
  EXPECT_TRUE(test_map.subtypeOf(dict_n(TSStr, TInitUnc)));
  EXPECT_TRUE(test_map.subtypeOf(sdict_n(TSStr, TInitUnc)));
  EXPECT_TRUE(sdict_packedn({TInt}).subtypeOf(dict_n(TInt, TInt)));
  EXPECT_TRUE(sdict_packed({TInt}).subtypeOf(dict_n(TInt, TInt)));

  auto test_map_a          = MapElems{};
  test_map_a[tv(s_test)]   = ival(2);
  auto const tstruct       = sdict_map(test_map_a);

  EXPECT_TRUE(tstruct.subtypeOf(dict_n(TSStr, ival(2))));
  EXPECT_TRUE(tstruct.subtypeOf(dict_n(TSStr, TInt)));
  EXPECT_TRUE(tstruct.subtypeOf(sdict_n(TSStr, TInt)));
  EXPECT_TRUE(tstruct.subtypeOf(dict_n(TStr, TInt)));

  EXPECT_TRUE(test_map.couldBe(dict_n(TSStr, TInitCell)));
  EXPECT_FALSE(test_map.couldBe(dict_n(TSStr, TStr)));
  EXPECT_FALSE(test_map.couldBe(dict_n(TSStr, TObj)));

  EXPECT_FALSE(test_map.couldBe(dict_val(test_empty_dict())));
  EXPECT_FALSE(dict_n(TSStr, TInt).couldBe(dict_val(test_empty_dict())));

  EXPECT_TRUE(sdict_packedn(TInt).couldBe(sdict_n(TInt, TInt)));
  EXPECT_FALSE(sdict_packedn(TInt).couldBe(sdict_n(TInt, TObj)));

  EXPECT_TRUE(tstruct.couldBe(sdict_n(TSStr, TInt)));
  EXPECT_FALSE(tstruct.couldBe(sdict_n(TSStr, TObj)));
}

TEST(Type, DictEquivalentRepresentations) {
  {
    auto const simple = dict_val(test_dict_vector_value());
    auto const bulky  = sdict_packed({ival(42), ival(23), ival(12)});
    EXPECT_EQ(simple, bulky);
  }

  {
    auto const simple = dict_val(test_dict_map_value());

    auto map          = MapElems{};
    map[tv(s_A)]      = sval(s_B.get());
    map[tv(s_test)]   = ival(12);
    auto const bulky  = sdict_map(map);

    EXPECT_EQ(simple, bulky);
  }
}

TEST(Type, DictUnions) {
  auto test_map_a          = MapElems{};
  test_map_a[tv(s_test)]   = ival(2);
  auto const tstruct       = sdict_map(test_map_a);

  auto test_map_b          = MapElems{};
  test_map_b[tv(s_test)]   = TInt;
  auto const tstruct2      = sdict_map(test_map_b);

  auto test_map_c          = MapElems{};
  test_map_c[tv(s_A)]      = TInt;
  auto const tstruct3      = sdict_map(test_map_c);

  auto test_map_d          = MapElems{};
  test_map_d[tv(s_A)]      = TInt;
  test_map_d[tv(s_test)]   = TDbl;
  auto const tstruct4      = sdict_map(test_map_d);

  auto const packed_int = dict_packedn(TInt);

  EXPECT_EQ(union_of(tstruct, packed_int),
            dict_n(union_of(TSStr, TInt), TInt));
  EXPECT_EQ(union_of(tstruct, tstruct2), tstruct2);
  EXPECT_EQ(union_of(tstruct, tstruct3), sdict_n(TSStr, TInt));
  EXPECT_EQ(union_of(tstruct, tstruct4), sdict_n(TSStr, TNum));

  EXPECT_EQ(union_of(sdict_packed({TInt, TDbl, TDbl}), sdict_packedn(TDbl)),
            sdict_packedn(TNum));
  EXPECT_EQ(union_of(sdict_packed({TInt, TDbl}), tstruct),
            sdict_n(union_of(TSStr, TInt), TNum));

  EXPECT_EQ(union_of(dict_n(TInt, TTrue), dict_n(TStr, TFalse)),
            dict_n(TArrKey, TBool));

  auto const dict_val1 = dict_val(test_dict_vector_value());
  auto const dict_val2 = dict_val(test_dict_vector_value3());
  EXPECT_EQ(union_of(dict_val1, dict_val2), sdict_packedn(TInt));
}

TEST(Type, DictIntersections) {
  auto test_map_a          = MapElems{};
  test_map_a[tv(s_test)]   = ival(2);
  auto const tstruct       = sdict_map(test_map_a);

  auto test_map_b          = MapElems{};
  test_map_b[tv(s_test)]   = TInt;
  auto const tstruct2      = sdict_map(test_map_b);

  auto test_map_c          = MapElems{};
  test_map_c[tv(s_A)]      = TInt;
  auto const tstruct3      = sdict_map(test_map_c);

  auto test_map_d          = MapElems{};
  test_map_d[tv(s_A)]      = TInt;
  test_map_d[tv(s_test)]   = TDbl;
  auto const tstruct4      = sdict_map(test_map_d);

  auto test_map_e          = MapElems{};
  test_map_e[tv(s_A)]      = TInt;
  test_map_e[tv(s_B)]      = TDbl;
  auto const tstruct5      = sdict_map(test_map_e);

  auto test_map_f          = MapElems{};
  test_map_f[tv(s_A)]      = TArrKey;
  test_map_f[tv(s_B)]      = TInt;
  auto const tstruct6      = sdict_map(test_map_f);

  auto test_map_g          = MapElems{};
  test_map_g[tv(s_A)]      = TStr;
  test_map_g[tv(s_B)]      = TArrKey;
  auto const tstruct7      = sdict_map(test_map_g);

  auto test_map_h          = MapElems{};
  test_map_h[tv(s_A)]      = TStr;
  test_map_h[tv(s_B)]      = TInt;
  auto const tstruct8      = sdict_map(test_map_h);

  auto test_map_i          = MapElems{};
  test_map_i[tv(s_A)]      = TStr;
  test_map_i[tv(s_B)]      = TInt;
  test_map_i[tv(s_BB)]     = TVec;
  auto const tstruct9      = dict_map(test_map_i);

  auto test_map_j          = MapElems{};
  test_map_j[tv(s_A)]      = TSStr;
  test_map_j[tv(s_B)]      = TInt;
  test_map_j[tv(s_BB)]     = TSVec;
  auto const tstruct10     = sdict_map(test_map_j);

  auto test_map_k          = MapElems{};
  test_map_k[tv(s_A)]      = TSStr;
  test_map_k[tv(s_B)]      = TInt;
  test_map_k[tv(s_BB)]     = TObj;
  auto const tstruct11     = dict_map(test_map_k);

  auto const mapn_str_int = dict_n(TStr, TInt);

  EXPECT_EQ(intersection_of(tstruct,  mapn_str_int), tstruct);
  EXPECT_EQ(intersection_of(tstruct2, mapn_str_int), tstruct2);
  EXPECT_EQ(intersection_of(tstruct3, mapn_str_int), tstruct3);
  EXPECT_EQ(intersection_of(tstruct4, mapn_str_int), TBottom);
  EXPECT_EQ(intersection_of(tstruct,  tstruct2),     tstruct);
  EXPECT_EQ(intersection_of(tstruct,  tstruct3),     TBottom);
  EXPECT_EQ(intersection_of(tstruct4, tstruct5),     TBottom);
  EXPECT_EQ(intersection_of(tstruct6, tstruct7),     tstruct8);
  EXPECT_EQ(intersection_of(tstruct8, tstruct),      TBottom);

  EXPECT_EQ(intersection_of(sdict_packed({TNum, TDbl, TNum}),
                            sdict_packedn(TDbl)),
            sdict_packed({TDbl, TDbl, TDbl}));
  EXPECT_EQ(intersection_of(sdict_packed({TNum, TDbl, TNum}),
                            sdict_packed({TDbl, TNum, TInt})),
            sdict_packed({TDbl, TDbl, TInt}));
  EXPECT_EQ(intersection_of(TSDictN, dict_n(TArrKey, TInitCell)),
            sdict_n(TUncArrKey, TInitUnc));
  EXPECT_EQ(intersection_of(TSDictN, dict_n(TArrKey, TInitCell)),
            sdict_n(TUncArrKey, TInitUnc));
  EXPECT_EQ(intersection_of(tstruct9, TSDictN), tstruct10);
  EXPECT_EQ(intersection_of(TSDictN, dict_packed({TStr, TVec, TInt, TInitCell})),
                            sdict_packed({TSStr, TSVec, TInt, TInitUnc}));
  EXPECT_EQ(intersection_of(dict_packedn(TStr), TSDictN), sdict_packedn(TSStr));
  EXPECT_EQ(intersection_of(TSDictN, dict_packedn(TObj)), TBottom);
  EXPECT_EQ(intersection_of(dict_packed({TStr, TInt, TObj}), TSDictN), TBottom);
  EXPECT_EQ(intersection_of(TSDictN, tstruct11), TBottom);
  EXPECT_EQ(
    intersection_of(dict_n(TArrKey, TObj), TSDictN),
    TBottom
  );
  EXPECT_EQ(
    intersection_of(union_of(dict_n(TInt, TObj), TDictE),
                    union_of(dict_packed({TInt, TObj}), TDictE)),
    TDictE
  );
  EXPECT_EQ(
    intersection_of(opt(dict_n(TInt, TObj)), TUnc),
    TInitNull
  );
  EXPECT_EQ(
    intersection_of(opt(dict_packedn(TObj)), TInitUnc),
    TInitNull
  );
  EXPECT_EQ(
    intersection_of(union_of(dict_packed({TInt, TObj}), TDictE), TUnc),
    TSDictE
  );
  EXPECT_EQ(
    intersection_of(opt(union_of(dict_packed({TInt, TObj}), TDictE)), TUnc),
    opt(TSDictE)
  );
}

TEST(Type, DictOfDict) {
  auto const t1 = dict_n(TSStr, dict_n(TInt, TSStr));
  auto const t2 = dict_n(TSStr, TDict);
  auto const t3 = dict_n(TSStr, dict_packedn(TSStr));
  auto const t4 = dict_n(TSStr, dict_n(TSStr, TSStr));
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

TEST(Type, EmptyDict) {
  {
    auto const possible_e = union_of(dict_packedn(TInt), dict_empty());
    EXPECT_TRUE(possible_e.couldBe(dict_empty()));
    EXPECT_TRUE(possible_e.couldBe(dict_packedn(TInt)));
    EXPECT_EQ(dict_elem(possible_e, ival(0)).first, TInt);
  }

  {
    auto const possible_e = union_of(dict_packed({TInt, TInt}), dict_empty());
    EXPECT_TRUE(possible_e.couldBe(dict_empty()));
    EXPECT_TRUE(possible_e.couldBe(dict_packed({TInt, TInt})));
    EXPECT_FALSE(possible_e.couldBe(dict_packed({TInt, TInt, TInt})));
    EXPECT_FALSE(possible_e.subtypeOf(dict_packedn(TInt)));
    EXPECT_EQ(dict_elem(possible_e, ival(0)).first, TInt);
    EXPECT_EQ(dict_elem(possible_e, ival(1)).first, TInt);
  }

  {
    auto const estat = union_of(sdict_packedn(TInt), dict_empty());
    EXPECT_TRUE(estat.couldBe(dict_empty()));
    EXPECT_TRUE(estat.couldBe(sdict_packedn(TInt)));
    EXPECT_FALSE(estat.subtypeOf(sdict_packedn(TInt)));
    EXPECT_FALSE(estat.subtypeOf(BSDictE));
    EXPECT_TRUE(estat.couldBe(BSDictE));
  }

  EXPECT_EQ(dict_newelem(dict_empty(), ival(142)).first,
            dict_packed({ival(142)}));
}

TEST(Type, BasicArrays) {
  EXPECT_TRUE(TSArr.subtypeOf(BArr));
  EXPECT_TRUE(TArrE.subtypeOf(BArr));
  EXPECT_TRUE(TArrN.subtypeOf(BArr));
  EXPECT_TRUE(TSArrE.subtypeOf(BArr));
  EXPECT_TRUE(TSArrN.subtypeOf(BArr));

  EXPECT_EQ(union_of(TArrN, TArrE), TArr);

  EXPECT_EQ(union_of(TSArrN, TArrE), TArr);
  EXPECT_EQ(union_of(TSArrE, TArrN), TArr);
  EXPECT_EQ(union_of(TOptArrN, TSArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArr, TArr), TOptArr);
  EXPECT_EQ(union_of(TOptSArrE, TArrE), TOptArrE);
  EXPECT_EQ(union_of(TOptSArrN, TArrN), TOptArrN);
  EXPECT_EQ(union_of(TOptArrN, TArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArrN, TOptArrE), TOptArr);
  EXPECT_EQ(union_of(TOptSArrN, TOptArrE), TOptArr);

  EXPECT_EQ(union_of(TOptSArr, TOptArr), TOptArr);
  EXPECT_EQ(union_of(TOptSArrE, TOptArrE), TOptArrE);
  EXPECT_EQ(union_of(TOptSArrN, TOptArrN), TOptArrN);
  EXPECT_EQ(union_of(TOptArrN, TOptArrE), TOptArr);

  EXPECT_EQ(union_of(TSArr, TInitNull), TOptSArr);
  EXPECT_EQ(union_of(TSArrE, TInitNull), TOptSArrE);
  EXPECT_EQ(union_of(TSArrN, TInitNull), TOptSArrN);
  EXPECT_EQ(union_of(TArr, TInitNull), TOptArr);
  EXPECT_EQ(union_of(TArrE, TInitNull), TOptArrE);
  EXPECT_EQ(union_of(TArrN, TInitNull), TOptArrN);

  EXPECT_EQ(union_of(TSVArrE, TSDArrE), TSArrE);
  EXPECT_EQ(union_of(TSVArrN, TSDArrN), TSArrN);
  EXPECT_EQ(union_of(TVArrN, TDArrN), TArrN);
  EXPECT_EQ(union_of(TVArrE, TDArrE), TArrE);
  EXPECT_EQ(union_of(TSVArr, TSDArrN), TSArr);
  EXPECT_EQ(union_of(TVArr, TDArr), TArr);
}

/*
 * These are tests for some unrepresentable bit combos.  If we ever
 * add predefined bits for things like TSArrE|TArrN these will fail
 * and need to be revisted.
 */
TEST(Type, DictBitCombos) {
  auto const u1 = union_of(sdict_packedn(TInt), TDictE);
  EXPECT_TRUE(u1.couldBe(BDictE));
  EXPECT_TRUE(u1.couldBe(BSDictE));
  EXPECT_TRUE(u1.couldBe(sdict_packedn(TInt)));
  EXPECT_EQ(dict_elem(u1, ival(0)).first, TInt);

  auto const u2 = union_of(TSDictE, dict_packedn(TInt));
  EXPECT_TRUE(u2.couldBe(BDictE));
  EXPECT_TRUE(u2.couldBe(BSDictE));
  EXPECT_TRUE(u2.couldBe(dict_packedn(TInt)));
  EXPECT_EQ(dict_elem(u2, ival(0)).first, TInt);
}

TEST(Type, ArrKey) {
  EXPECT_TRUE(TInt.subtypeOf(BArrKey));
  EXPECT_TRUE(TStr.subtypeOf(BArrKey));
  EXPECT_TRUE(ival(0).subtypeOf(BArrKey));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(BArrKey));
  EXPECT_TRUE(sval_nonstatic(s_test.get()).subtypeOf(BArrKey));

  EXPECT_TRUE(TInt.subtypeOrNull(BArrKey));
  EXPECT_TRUE(TStr.subtypeOrNull(BArrKey));
  EXPECT_TRUE(ival(0).subtypeOrNull(BArrKey));
  EXPECT_TRUE(sval(s_test.get()).subtypeOrNull(BArrKey));
  EXPECT_TRUE(TInitNull.subtypeOrNull(BArrKey));

  EXPECT_TRUE(TInt.subtypeOf(BUncArrKey));
  EXPECT_TRUE(TSStr.subtypeOf(BUncArrKey));
  EXPECT_TRUE(ival(0).subtypeOf(BUncArrKey));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(BUncArrKey));

  EXPECT_TRUE(TInt.subtypeOrNull(BUncArrKey));
  EXPECT_TRUE(TSStr.subtypeOrNull(BUncArrKey));
  EXPECT_TRUE(ival(0).subtypeOrNull(BUncArrKey));
  EXPECT_TRUE(sval(s_test.get()).subtypeOrNull(BUncArrKey));
  EXPECT_TRUE(TInitNull.subtypeOrNull(BUncArrKey));

  EXPECT_TRUE(TArrKey.subtypeOrNull(BArrKey));
  EXPECT_TRUE(TUncArrKey.subtypeOrNull(BUncArrKey));
  EXPECT_TRUE(TUncArrKey.subtypeOf(BArrKey));
  EXPECT_TRUE(TOptUncArrKey.subtypeOrNull(BArrKey));

  EXPECT_TRUE(TArrKey.subtypeOf(BInitCell));
  EXPECT_TRUE(TUncArrKey.subtypeOf(BInitCell));
  EXPECT_TRUE(TOptArrKey.subtypeOf(BInitCell));
  EXPECT_TRUE(TOptUncArrKey.subtypeOf(BInitCell));

  EXPECT_TRUE(TUncArrKey.subtypeOf(BInitUnc));
  EXPECT_TRUE(TOptUncArrKey.subtypeOf(BInitUnc));

  EXPECT_TRUE(union_of(TInt, TStr) == TArrKey);
  EXPECT_TRUE(union_of(TInt, TSStr) == TUncArrKey);
  EXPECT_TRUE(union_of(ival(1), TStr) == TArrKey);
  EXPECT_TRUE(union_of(ival(1), sval(s_test.get())) == TUncArrKey);
  EXPECT_TRUE(union_of(ival(1), sval_nonstatic(s_test.get())) == TArrKey);
  EXPECT_TRUE(union_of(TArrKey, TInitNull) == TOptArrKey);
  EXPECT_TRUE(union_of(TUncArrKey, TInitNull) == TOptUncArrKey);

  EXPECT_TRUE(opt(TArrKey) == TOptArrKey);
  EXPECT_TRUE(opt(TUncArrKey) == TOptUncArrKey);
  EXPECT_TRUE(unopt(TOptArrKey) == TArrKey);
  EXPECT_TRUE(unopt(TOptUncArrKey) == TUncArrKey);

  EXPECT_TRUE(union_of(union_of(TArrKey, TCls), TLazyCls) == TArrKeyCompat);
  EXPECT_TRUE(union_of(union_of(TUncArrKey, TCls), TLazyCls) == TUncArrKeyCompat);
  EXPECT_TRUE(union_of(TArrKeyCompat, TInitNull) == TOptArrKeyCompat);
  EXPECT_TRUE(union_of(TUncArrKeyCompat, TInitNull) == TOptUncArrKeyCompat);
}

TEST(Type, LoosenStaticness) {
  auto const program = make_test_program();
  Index index{ program.get() };

  for (auto const& t : all()) {
    if (t == TUncArrKey || t == TOptUncArrKey ||
        t == TUncArrKeyCompat || t == TOptUncArrKeyCompat ||
        t == TInitUnc || t == TUnc ||
        (t.subtypeOfAny(TOptArr,
                        TOptVec,
                        TOptDict,
                        TOptKeyset,
                        TOptSStr,
                        TOptUncStrLike,
                        TOptArrCompatSA,
                        TOptVArrCompatSA,
                        TOptVecCompatSA) &&
         t != TInitNull)) continue;
    EXPECT_EQ(loosen_staticness(t), t);
  }

  for (auto const& t : all()) {
    EXPECT_EQ(loosen_staticness(wait_handle(index, t)),
              wait_handle(index, loosen_staticness(t)));
    if (t.subtypeOf(TInitCell)) {
      EXPECT_EQ(loosen_staticness(dict_packedn(t)),
                dict_packedn(loosen_staticness(t)));
      EXPECT_EQ(loosen_staticness(sdict_packedn(t)),
                dict_packedn(loosen_staticness(t)));
    }
  }

  auto test_map          = MapElems{};
  test_map[tv(s_A)]      = TInt;
  std::vector<std::pair<Type, Type>> tests = {
    { TSStr, TStr },
    { TSArrE, TArrE },
    { TSArrN, TArrN },
    { TSArr, TArr },
    { TSVArrE, TVArrE },
    { TSVArrN, TVArrN },
    { TSVArr, TVArr },
    { TSDArrE, TDArrE },
    { TSDArrN, TDArrN },
    { TSDArr, TDArr },
    { TSVecE, TVecE },
    { TSVecN, TVecN },
    { TSVec, TVec },
    { TSDictE, TDictE },
    { TSDictN, TDictN },
    { TSDict, TDict },
    { TSKeysetE, TKeysetE },
    { TSKeysetN, TKeysetN },
    { TSKeyset, TKeyset },
    { TUncArrKey, TArrKey },
    { TUncArrKeyCompat, TArrKeyCompat },
    { TUnc, TCell },
    { TInitUnc, TInitCell },
    { sval(s_test.get()), sval_nonstatic(s_test.get()) },
    { sdict_packedn(TInt), dict_packedn(TInt) },
    { sdict_packed({TInt, TBool}), dict_packed({TInt, TBool}) },
    { sdict_n(TSStr, TInt), dict_n(TStr, TInt) },
    { sdict_n(TInt, TSDictN), dict_n(TInt, TDictN) },
    { sdict_map(test_map), dict_map(test_map) },
    { TClsMeth, TClsMeth },
    { TFuncLike, TFuncLike },
    { TStrLike, TStrLike },
    { TUncStrLike, TStrLike },
    { TVArrCompat, TVArrCompat },
    { TVArrCompatSA, TVArrCompat },
    { TVecCompat, TVecCompat },
    { TVecCompatSA, TVecCompat },
    { TArrCompat, TArrCompat },
    { TArrCompatSA, TArrCompat },
  };
  for (auto const& p : tests) {
    EXPECT_EQ(loosen_staticness(p.first), p.second);
    if (p.first == TUnc || p.first == TInitUnc) continue;
    EXPECT_EQ(loosen_staticness(opt(p.first)), opt(p.second));
    EXPECT_EQ(loosen_staticness(wait_handle(index, p.first)),
              wait_handle(index, p.second));
    EXPECT_EQ(loosen_staticness(dict_packedn(p.first)),
              dict_packedn(p.second));
    EXPECT_EQ(loosen_staticness(sdict_packedn(p.first)),
              dict_packedn(p.second));
  }
}

TEST(Type, LoosenEmptiness) {
  auto const program = make_test_program();
  Index index{ program.get() };

  for (auto const& t : all_with_waithandles(index)) {
    if (t.subtypeOfAny(TOptArrLike) && t != TInitNull) continue;
    EXPECT_EQ(loosen_emptiness(t), t);
  }

  auto test_map          = MapElems{};
  test_map[tv(s_A)]      = TInt;
  std::vector<std::pair<Type, Type>> tests = {
    { TSArrE, TSArr },
    { TSArrN, TSArr },
    { TArrE, TArr },
    { TArrN, TArr },
    { TSVArrE, TSVArr },
    { TSVArrN, TSVArr },
    { TVArrE, TVArr },
    { TVArrN, TVArr },
    { TSDArrE, TSDArr },
    { TSDArrN, TSDArr },
    { TDArrE, TDArr },
    { TDArrN, TDArr },
    { TSVecE, TSVec },
    { TSVecN, TSVec },
    { TVecE, TVec },
    { TVecN, TVec },
    { TSDictE, TSDict },
    { TSDictN, TSDict },
    { TDictE, TDict },
    { TDictN, TDict },
    { TSKeysetE, TSKeyset },
    { TSKeysetN, TSKeyset },
    { TKeysetE, TKeyset },
    { TKeysetN, TKeyset },
    { dict_packedn(TInt), union_of(TDictE, dict_packedn(TInt)) },
    { dict_packed({TInt, TBool}), union_of(TDictE, dict_packed({TInt, TBool})) },
    { dict_n(TStr, TInt), union_of(TDictE, dict_n(TStr, TInt)) },
    { dict_map(test_map), union_of(TDictE, dict_map(test_map)) },
    { TArrLikeE, TArrLike },
    { TArrLikeE, TArrLike },
  };
  for (auto const& p : tests) {
    EXPECT_EQ(loosen_emptiness(p.first), p.second);
    EXPECT_EQ(loosen_emptiness(opt(p.first)), opt(p.second));
  }
}

TEST(Type, LoosenValues) {
  auto const program = make_test_program();
  auto const unit = program->units.back().get();
    auto const func = [&]() -> php::Func* {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return f.get();
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index index{ program.get() };

  for (auto const& t : all_no_data_with_waithandles(index)) {
    if (t == TTrue || t == TFalse) continue;
    if (t == TOptTrue || t == TOptFalse) continue;
    EXPECT_EQ(loosen_values(t), t);
  }

  EXPECT_TRUE(loosen_values(TTrue) == TBool);
  EXPECT_TRUE(loosen_values(TFalse) == TBool);
  EXPECT_TRUE(loosen_values(TOptTrue) == TOptBool);
  EXPECT_TRUE(loosen_values(TOptFalse) == TOptBool);

  auto test_map          = MapElems{};
  test_map[tv(s_A)]      = TInt;
  std::vector<std::pair<Type, Type>> tests = {
    { ival(123), TInt },
    { dval(3.14), TDbl },
    { sval(s_test.get()), TSStr },
    { sval_nonstatic(s_test.get()), TStr },
    { dict_val(test_dict_vector_value()), TSDictN },
    { dict_packedn(TInt), TDictN },
    { dict_packed({TInt, TBool}), TDictN },
    { dict_n(TStr, TInt), TDictN },
    { dict_map(test_map), TDictN }
  };
  for (auto const& p : tests) {
    EXPECT_EQ(loosen_values(p.first), p.second);
    EXPECT_EQ(loosen_values(opt(p.first)), opt(p.second));
  }

  auto const cls = index.resolve_class(ctx, s_TestClass.get());
  EXPECT_TRUE(!!cls);
  auto const rec = index.resolve_record(s_UniqueRec.get());
  EXPECT_TRUE(rec);

  EXPECT_TRUE(loosen_values(objExact(*cls)) == objExact(*cls));
  EXPECT_TRUE(loosen_values(subObj(*cls)) == subObj(*cls));
  EXPECT_TRUE(loosen_values(clsExact(*cls)) == clsExact(*cls));
  EXPECT_TRUE(loosen_values(subCls(*cls)) == subCls(*cls));
  EXPECT_TRUE(loosen_values(exactRecord(*rec)) == exactRecord(*rec));
  EXPECT_TRUE(loosen_values(subRecord(*rec)) == subRecord(*rec));

  EXPECT_TRUE(loosen_values(opt(objExact(*cls))) == opt(objExact(*cls)));
  EXPECT_TRUE(loosen_values(opt(subObj(*cls))) == opt(subObj(*cls)));
}

TEST(Type, AddNonEmptiness) {
  auto const program = make_test_program();
  Index index{ program.get() };

  for (auto const& t : all_with_waithandles(index)) {
    if (t.subtypeOfAny(TOptArrLikeE) && t != TInitNull) continue;
    EXPECT_EQ(add_nonemptiness(t), t);
  }

  std::vector<std::pair<Type, Type>> tests = {
    { TArrE, TArr },
    { TSArrE, TSArr },
    { TVArrE, TVArr },
    { TSVArrE, TSVArr },
    { TDArrE, TDArr },
    { TSDArrE, TSDArr },
    { TVecE, TVec },
    { TSVecE, TSVec },
    { TDictE, TDict },
    { TSDictE, TSDict },
    { TKeysetE, TKeyset },
    { TSKeysetE, TSKeyset },
    { TSArrLikeE, TSArrLike },
    { TArrLikeE, TArrLike },
  };
  for (auto const& p : tests) {
    EXPECT_EQ(add_nonemptiness(p.first), p.second);
    EXPECT_EQ(add_nonemptiness(opt(p.first)), opt(p.second));
  }
}

TEST(Type, LoosenDVArrayness) {
  auto const program = make_test_program();
  Index index{ program.get() };

  for (auto const& t : all_with_waithandles(index)) {
    if (t.subtypeOfAny(TOptVArr, TOptDArr, TOptVArrCompat) && t != TInitNull) {
      continue;
    }
    EXPECT_EQ(loosen_dvarrayness(t), t);
  }

  std::vector<std::pair<Type, Type>> tests = {
    { TSVArrE, TSArrE },
    { TSVArrN, TSArrN },
    { TSVArr,  TSArr },
    { TVArrE,  TArrE },
    { TVArrN,  TArrN },
    { TVArr,   TArr },

    { TSDArrE, TSArrE },
    { TSDArrN, TSArrN },
    { TSDArr,  TSArr },
    { TDArrE,  TArrE },
    { TDArrN,  TArrN },
    { TDArr,   TArr }
  };
  for (auto const& p : tests) {
    EXPECT_EQ(loosen_dvarrayness(p.first), p.second);
    EXPECT_EQ(loosen_dvarrayness(opt(p.first)), opt(p.second));
  }
}

TEST(Type, StrValues) {
  auto const t1 = sval(s_test.get());
  auto const t2 = sval_nonstatic(s_test.get());
  auto const t3 = sval(s_A.get());
  auto const t4 = sval_nonstatic(s_test.get());
  auto const t5 = sval_nonstatic(s_A.get());

  EXPECT_TRUE(t1.subtypeOf(t2));
  EXPECT_TRUE(t1.subtypeOf(TSStr));
  EXPECT_TRUE(t1.subtypeOf(TStr));
  EXPECT_FALSE(t1.subtypeOf(t3));

  EXPECT_FALSE(t2.subtypeOf(t1));
  EXPECT_FALSE(t2.subtypeOf(TSStr));
  EXPECT_TRUE(t2.subtypeOf(TStr));
  EXPECT_FALSE(t2.subtypeOf(t3));
  EXPECT_TRUE(t2.subtypeOf(t4));
  EXPECT_FALSE(t2.subtypeOf(t5));

  EXPECT_FALSE(TStr.subtypeOf(t1));
  EXPECT_FALSE(TSStr.subtypeOf(t2));
  EXPECT_FALSE(TStr.subtypeOf(t2));
  EXPECT_FALSE(TSStr.subtypeOf(t2));
  EXPECT_FALSE(t2.subtypeOf(t1));
  EXPECT_FALSE(t3.subtypeOf(t2));
  EXPECT_TRUE(t4.subtypeOf(t2));
  EXPECT_FALSE(t5.subtypeOf(t2));

  EXPECT_TRUE(t1.couldBe(t2));
  EXPECT_FALSE(t1.couldBe(t3));
  EXPECT_TRUE(t1.couldBe(TStr));
  EXPECT_TRUE(t1.couldBe(TSStr));

  EXPECT_TRUE(t2.couldBe(t1));
  EXPECT_FALSE(t2.couldBe(t3));
  EXPECT_TRUE(t2.couldBe(t4));
  EXPECT_FALSE(t2.couldBe(t5));
  EXPECT_TRUE(t2.couldBe(TStr));
  EXPECT_TRUE(t2.couldBe(TSStr));

  EXPECT_TRUE(TSStr.couldBe(t1));
  EXPECT_TRUE(TStr.couldBe(t1));
  EXPECT_TRUE(TSStr.couldBe(t2));
  EXPECT_TRUE(TStr.couldBe(t2));
  EXPECT_FALSE(t3.couldBe(t1));
  EXPECT_FALSE(t3.couldBe(t2));
  EXPECT_TRUE(t4.couldBe(t2));
  EXPECT_FALSE(t5.couldBe(t2));

  EXPECT_TRUE(union_of(t1, t1) == t1);
  EXPECT_TRUE(union_of(t2, t2) == t2);
  EXPECT_TRUE(union_of(t1, t2) == t2);
  EXPECT_TRUE(union_of(t2, t1) == t2);
  EXPECT_TRUE(union_of(t1, t3) == TSStr);
  EXPECT_TRUE(union_of(t3, t1) == TSStr);
  EXPECT_TRUE(union_of(t2, t3) == TStr);
  EXPECT_TRUE(union_of(t3, t2) == TStr);
  EXPECT_TRUE(union_of(t2, t4) == t2);
  EXPECT_TRUE(union_of(t4, t2) == t2);
  EXPECT_TRUE(union_of(t2, t5) == TStr);
  EXPECT_TRUE(union_of(t5, t2) == TStr);
}

TEST(Type, RemoveCounted) {
  for (auto const& t1 : all()) {
    auto const t2 = remove_counted(t1);
    EXPECT_TRUE(t2.subtypeOf(TUnc));
    if (t1.subtypeOf(TUnc)) {
      EXPECT_EQ(t2, t1);
    }
    if (!t1.couldBe(TUnc)) {
      EXPECT_EQ(t2, TBottom);
    }
    EXPECT_EQ(t2, intersection_of(t1, TUnc));
  }

  auto test_map_a = MapElems{};
  test_map_a[tv(s_test)] = TInt;
  test_map_a[tv(s_A)] = TDbl;

  auto test_map_b = MapElems{};
  test_map_b[tv(s_A)] = TStr;
  test_map_b[tv(s_B)] = TArrKey;

  auto test_map_c = MapElems{};
  test_map_c[tv(s_A)] = TSStr;
  test_map_c[tv(s_B)] = TUncArrKey;

  auto test_map_d = MapElems{};
  test_map_d[tv(s_A)] = TInt;
  test_map_d[tv(s_B)] = TObj;

  auto test_map_e = MapElems{};
  test_map_e[tv(s_A)] = ival(123);
  test_map_e[tv(s_B)] = sval_nonstatic(s_test.get());

  auto test_map_f = MapElems{};
  test_map_f[tv(s_A)] = ival(123);
  test_map_f[tv(s_B)] = sval(s_test.get());

  auto test_map_g = MapElems{};
  test_map_g[tv(s_A)] = TSStr;
  test_map_g[tv(s_B)] = sdict_map(test_map_f);

  auto test_map_h = MapElems{};
  test_map_h[tv(s_A)] = TStr;
  test_map_h[tv(s_B)] = dict_map(test_map_e);

  auto test_map_i = MapElems{};
  test_map_i[tv(s_A)] = TSStr;
  test_map_i[tv(s_B)] = sdict_map(test_map_f);

  auto test_map_j = MapElems{};
  test_map_j[tv(s_A)] = TSStr;
  test_map_j[tv(s_B)] = dict_map(test_map_d);

  auto const cases = std::vector<std::pair<Type, Type>>{
    { TInt, TInt },
    { TUninit, TUninit },
    { TInitNull, TInitNull },
    { TFalse, TFalse },
    { TTrue, TTrue },
    { TInt, TInt },
    { TDbl, TDbl },
    { TSStr, TSStr },
    { TFunc, TFunc },
    { TSArr, TSArr },
    { TSDict, TSDict },
    { TSVec, TSVec },
    { TSKeyset, TSKeyset },
    { TCls, TCls },
    { TPrim, TPrim },
    { TUnc, TUnc },
    { TInitUnc, TInitUnc },
    { TSDictN, TSDictN },
    { TSVecE, TSVecE },
    { TUncArrKey, TUncArrKey },
    { TUncStrLike, TUncStrLike },
    { TObj, TBottom },
    { TRes, TBottom },
    { TStr, TSStr },
    { TArr, TSArr },
    { TVec, TSVec },
    { TDict, TSDict },
    { TKeyset, TSKeyset },
    { TVecN, TSVecN },
    { TKeysetN, TSKeysetN },
    { TDArr, TSDArr },
    { TDArrN, TSDArrN },
    { TArrKey, TUncArrKey },
    { TStrLike, TUncStrLike },
    { TInitCell, TInitUnc },
    { TCell, TUnc },
    { some_dict_empty(), dict_empty() },
    { ival(1), ival(1) },
    { dval(2.0), dval(2.0) },
    { sval(s_test.get()), sval(s_test.get()) },
    { sval_nonstatic(s_test.get()), sval(s_test.get()) },
    { dict_val(test_dict_map_value()), dict_val(test_dict_map_value()) },
    { dict_val(test_dict_vector_value()), dict_val(test_dict_vector_value()) },
    { sdict_packedn(TInt), sdict_packedn(TInt) },
    { dict_packedn(TInitCell), sdict_packedn(TInitUnc) },
    { dict_packedn(TObj), TBottom },
    { dict_packedn(ival(123)), sdict_packedn(ival(123)) },
    {
      dict_packedn(sval_nonstatic(s_test.get())),
      sdict_packedn(sval(s_test.get()))
    },
    { sdict_packed({TInt, TSStr}), sdict_packed({TInt, TSStr}) },
    { dict_packed({TInitCell, TStr}), sdict_packed({TInitUnc, TSStr}) },
    { dict_packed({TInt, TObj, TInt}), TBottom },
    { sdict_n(TInt, TSStr), sdict_n(TInt, TSStr) },
    { sdict_n(TSStr, TInt), sdict_n(TSStr, TInt) },
    { dict_n(TArrKey, TArrKey), sdict_n(TUncArrKey, TUncArrKey) },
    { dict_n(TStr, TStr), sdict_n(TSStr, TSStr) },
    { dict_n(TInt, TObj), TBottom },
    { dict_n(TStr, dval(3.14)), sdict_n(TSStr, dval(3.14)) },
    { sdict_map(test_map_a), sdict_map(test_map_a) },
    { dict_map(test_map_a), sdict_map(test_map_a) },
    { dict_map(test_map_b), sdict_map(test_map_c) },
    { dict_map(test_map_d), TBottom },
    { dict_map(test_map_e), sdict_map(test_map_f) },
    { sdict_map(test_map_f), sdict_map(test_map_f) },
    { sdict_packedn(sdict_packedn(TInt)), sdict_packedn(sdict_packedn(TInt)) },
    { dict_packedn(dict_packedn(TStr)), sdict_packedn(sdict_packedn(TSStr)) },
    { dict_packedn(dict_packedn(TObj)), TBottom },
    {
      sdict_packed({sdict_packed({TInt, TDbl}), TInt}),
      sdict_packed({sdict_packed({TInt, TDbl}), TInt})
    },
    {
      dict_packed({dict_packed({TArrKey, TStr}), TInitCell}),
      sdict_packed({sdict_packed({TUncArrKey, TSStr}), TInitUnc})
    },
    { dict_packed({dict_packed({TObj, TObj}), TInt}), TBottom },
    {
      sdict_n(TSStr, sdict_n(TSStr, TInitUnc)),
      sdict_n(TSStr, sdict_n(TSStr, TInitUnc))
    },
    {
      dict_n(TStr, dict_n(TStr, TArrKey)),
      sdict_n(TSStr, sdict_n(TSStr, TUncArrKey))
    },
    { dict_n(TInt, dict_n(TInt, TObj)), TBottom },
    { sdict_map(test_map_g), sdict_map(test_map_g) },
    { dict_map(test_map_h), sdict_map(test_map_i) },
    { dict_map(test_map_j), TBottom },
    { opt(sdict_n(TInt, TInt)), opt(sdict_n(TInt, TInt)) },
    {
      union_of(sdict_n(TInt, TInt), dict_empty()),
      union_of(sdict_n(TInt, TInt), dict_empty())
    },
    { opt(dict_n(TStr, TObj)), TInitNull },
    { opt(dict_packedn(TObj)), TInitNull },
    { opt(dict_packed({TObj, TInt})), TInitNull },
    { opt(dict_map(test_map_j)), TInitNull },
    {
      opt(dict_packedn(opt(dict_n(TStr, TObj)))),
      opt(sdict_packedn(TInitNull))
    },
    { union_of(dict_n(TStr, TObj), some_dict_empty()), TSDictE },
    { union_of(dict_packedn(TObj), some_dict_empty()), TSDictE },
    { union_of(dict_packed({TObj, TInt}), some_dict_empty()), TSDictE },
    { union_of(dict_map(test_map_j), some_dict_empty()), TSDictE },
    {
      dict_packedn(union_of(dict_n(TStr, TObj), some_dict_empty())),
      sdict_packedn(TSDictE)
    },
    { opt(union_of(dict_n(TStr, TObj), some_dict_empty())), opt(TSDictE) },
    { opt(union_of(dict_packedn(TObj), some_dict_empty())), opt(TSDictE) },
    { opt(union_of(dict_packed({TObj, TInt}), some_dict_empty())), opt(TSDictE) },
    { opt(union_of(dict_map(test_map_j), some_dict_empty())), opt(TSDictE) },
    { dict_map(test_map_d, TInt, TInt), TBottom },
    { dict_map(test_map_d, TStr, TObj), TBottom },
    { dict_map(test_map_a, TInt, TStr), sdict_map(test_map_a, TInt, TSStr) },
    {
      dict_map(test_map_a, TStr, TInitCell),
      sdict_map(test_map_a, TSStr, TInitUnc)
    },
    { dict_map(test_map_a, TInt, TObj), sdict_map(test_map_a) },
    {
      dict_map(test_map_a, TStr, opt(TObj)),
      sdict_map(test_map_a, TSStr, TInitNull)
    },
    {
      dict_map(test_map_a, TStr, union_of(dict_packedn(TObj), some_dict_empty())),
      sdict_map(test_map_a, TSStr, TSDictE)
    },
    { TClsMeth, use_lowptr ? TClsMeth : TBottom },
    { TVArrCompat, use_lowptr ? TVArrCompatSA : TSVArr },
    { TVArrCompatSA, use_lowptr ? TVArrCompatSA : TSVArr },
    { TVecCompat, use_lowptr ? TVecCompatSA : TSVec },
    { TVecCompatSA, use_lowptr ? TVecCompatSA : TSVec },
    { TArrCompat, use_lowptr ? TArrCompatSA : TSArr },
    { TArrCompatSA, use_lowptr ? TArrCompatSA : TSArr },
  };
  for (auto const& p : cases) {
    EXPECT_EQ(remove_counted(p.first), p.second);
    if (!p.first.couldBe(TNull) && p.second != TBottom) {
      EXPECT_EQ(remove_counted(opt(p.first)), opt(p.second));
    }
  }
}

TEST(Type, MustBeCounted) {
  for (auto const& t1 : all()) {
    if (t1.subtypeOf(TUnc)) {
      EXPECT_FALSE(must_be_counted(t1));
    }
    EXPECT_EQ(!t1.couldBe(TUnc), must_be_counted(t1));
    if (is_opt(t1)) {
      EXPECT_FALSE(must_be_counted(t1));
    }
    EXPECT_FALSE(must_be_counted(union_of(t1, TUnc)));
    if (!t1.couldBe(TNull)) {
      EXPECT_FALSE(must_be_counted(opt(t1)));
    }
    EXPECT_FALSE(must_be_counted(union_of(t1, some_dict_empty())));
  }

  auto test_map_a = MapElems{};
  test_map_a[tv(s_A)] = TSStr;
  test_map_a[tv(s_B)] = TInt;

  auto test_map_b = MapElems{};
  test_map_b[tv(s_A)] = TSStr;
  test_map_b[tv(s_B)] = TObj;

  auto const cases = std::vector<std::pair<Type, bool>>{
    { TInt, false },
    { TUninit, false },
    { TInitNull, false },
    { TFalse, false },
    { TTrue, false },
    { TInt, false },
    { TDbl, false },
    { TFunc, false },
    { TRFunc, true },
    { TSArr, false },
    { TCls, false },
    { TPrim, false },
    { TUnc, false },
    { TInitUnc, false },
    { TSDictN, false },
    { TArrKey, false },
    { TUncArrKey, false },
    { TArrKeyCompat, false },
    { TUncArrKeyCompat, false },
    { TStrLike, false },
    { TUncStrLike, false },
    { TInitCell, false },
    { TObj, true },
    { TRes, true },
    { TRecord, true },
    { ival(1), false },
    { dval(2.0), false },
    { sval(s_test.get()), false },
    { dict_val(test_dict_map_value()), false },
    { dict_val(test_dict_vector_value()), false },
    { dict_packedn(TInt), false },
    { dict_packedn(TObj), true },
    { dict_packed({TInt, TStr}), false },
    { dict_packed({TInt, TObj}), true },
    { dict_n(TInt, TInt), false },
    { dict_n(TInt, TObj), true },
    { dict_packed({TInt, dict_packedn(TObj)}), true },
    { dict_map(test_map_a), false },
    { dict_map(test_map_b), true },
    { dict_map(test_map_b, TStr, TObj), true },
    { dict_map(test_map_a, TStr, TObj), false },
    { dict_map(test_map_b, TSStr, TInt), true },
    { dict_map(test_map_a, TSStr, TInt), false },
    { TClsMeth, !use_lowptr },
    { TVArrCompat, false },
    { TVArrCompatSA, false },
    { TVecCompat, false },
    { TVecCompatSA, false },
    { TArrCompat, false },
    { TArrCompatSA, false },
  };
  for (auto const& p : cases) {
    if (p.second) {
      EXPECT_TRUE(must_be_counted(p.first));
    } else {
      EXPECT_FALSE(must_be_counted(p.first));
    }

    if (!p.first.couldBe(TNull)) {
      EXPECT_FALSE(must_be_counted(opt(p.first)));
    }
    EXPECT_FALSE(must_be_counted(union_of(p.first, some_dict_empty())));
  }
}

TEST(Type, DictMapOptValues) {
  auto test_map_a = MapElems{};
  test_map_a[tv(s_A)] = TInt;
  test_map_a[tv(s_B)] = TDbl;

  auto test_map_b = MapElems{};
  test_map_b[tv(s_A)] = TInt;

  auto test_map_c = MapElems{};
  test_map_c[tv(s_A)] = TInt;
  test_map_c[tv(s_test)] = TInt;

  auto test_map_d = MapElems{};
  test_map_d[tv(s_test)] = TInt;
  test_map_d[tv(s_A)] = TInt;

  auto test_map_e = MapElems{};
  test_map_e[tv(s_A)] = TInt;
  test_map_e[tv(s_B)] = TObj;

  auto test_map_f = MapElems{};
  test_map_f[tv(10)] = TInt;
  test_map_f[tv(11)] = TDbl;

  auto test_map_g = MapElems{};
  test_map_g[tv(s_A)] = TArrKey;

  auto test_map_h = MapElems{};
  test_map_h[tv(s_A)] = TInt;
  test_map_h[tv(s_B)] = TStr;

  auto test_map_i = MapElems{};
  test_map_i[tv(s_A)] = TInt;
  test_map_i[tv(s_B)] = TDbl;
  test_map_i[tv(s_test)] = TStr;

  auto test_map_j = MapElems{};
  test_map_j[tv(s_A)] = TInt;
  test_map_j[tv(s_B)] = TDbl;
  test_map_j[tv(s_test)] = TArrKey;

  EXPECT_EQ(dict_map(test_map_a, TInt, TSStr), dict_map(test_map_a, TInt, TSStr));
  EXPECT_NE(dict_map(test_map_a, TInt, TSStr), dict_map(test_map_a, TInt, TStr));

  EXPECT_FALSE(
    dict_map(test_map_c, TSStr, TInt).subtypeOf(dict_map(test_map_d, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_map(test_map_e, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_b, TSStr, TInt).subtypeOf(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_map(test_map_b, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_map(test_map_b, TSStr, TNum))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_map(test_map_b, TInt, TNum))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_map(test_map_a, TStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TStr, TInt).subtypeOf(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TNum).subtypeOf(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_n(TStr, TNum))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).subtypeOf(dict_n(TStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_f, TSStr, TInt).subtypeOf(dict_n(TInt, TNum))
  );
  EXPECT_FALSE(dict_map(test_map_a).subtypeOf(dict_n(TInt, TNum)));
  EXPECT_FALSE(
    dict_n(TSStr, TInt).subtypeOf(dict_map(test_map_a, TSStr, TInt))
  );

  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_a, TSStr, TNum))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TNum).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TArrKey, TInt).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_a, TArrKey, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_a, TInt, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TInt, TInt).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a, TSStr, TDbl).couldBe(dict_map(test_map_a, TSStr, TObj))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_c, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_c, TSStr, TInt).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_a, TSStr, TInt).couldBe(dict_map(test_map_e, TSStr, TInt))
  );
  EXPECT_FALSE(
    dict_map(test_map_e, TSStr, TInt).couldBe(dict_map(test_map_a, TSStr, TInt))
  );
  EXPECT_TRUE(
    dict_map(test_map_a).couldBe(dict_map(test_map_b, TSStr, TDbl))
  );
  EXPECT_TRUE(
    dict_map(test_map_b, TSStr, TDbl).couldBe(dict_map(test_map_a))
  );
  EXPECT_FALSE(
    dict_map(test_map_a).couldBe(dict_map(test_map_b, TSStr, TObj))
  );
  EXPECT_FALSE(
    dict_map(test_map_b, TSStr, TObj).couldBe(dict_map(test_map_a))
  );

  EXPECT_EQ(
    union_of(dict_map(test_map_a), dict_map(test_map_b)),
    dict_map(test_map_b, sval(s_B.get()), TDbl)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_a), dict_map(test_map_c)),
    dict_map(test_map_b, TSStr, TNum)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_a, TInt, TStr), dict_map(test_map_a, TStr, TInt)),
    dict_map(test_map_a, TArrKey, TArrKey)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_c), dict_map(test_map_d)),
    dict_n(TSStr, TInt)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_c, TInt, TInt), dict_map(test_map_d, TInt, TInt)),
    dict_n(TUncArrKey, TInt)
  );
  EXPECT_EQ(
    union_of(
      dict_map(test_map_c, TSStr, TDbl),
      dict_map(test_map_d, TSStr, TDbl)
    ),
    dict_n(TSStr, TNum)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_c, TSStr, TDbl), dict_packed({TInt})),
    dict_n(TUncArrKey, TNum)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_c, TSStr, TDbl), dict_packedn(TInt)),
    dict_n(TUncArrKey, TNum)
  );
  EXPECT_EQ(
    union_of(dict_map(test_map_c, TInt, TDbl), dict_n(TSStr, TInt)),
    dict_n(TUncArrKey, TNum)
  );

  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TSStr, TInt),
      dict_map(test_map_a, TSStr, TInt)
    ),
    dict_map(test_map_a, TSStr, TInt)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TSStr, TArrKey),
      dict_map(test_map_a, TSStr, TInt)
    ),
    dict_map(test_map_a, TSStr, TInt)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TSStr, TInt),
      dict_map(test_map_a, TArrKey, TInt)
    ),
    dict_map(test_map_a, TSStr, TInt)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TSStr, TInt),
      dict_map(test_map_a, TInt, TInt)
    ),
    dict_map(test_map_a)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TInt, TStr),
      dict_map(test_map_a, TInt, TInt)
    ),
    dict_map(test_map_a)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TInt, TInt),
      dict_map(test_map_e, TInt, TInt)
    ),
    TBottom
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_a), dict_map(test_map_b, TSStr, TNum)),
    dict_map(test_map_a)
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_b, TSStr, TNum), dict_map(test_map_a)),
    dict_map(test_map_a)
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_a), dict_map(test_map_b, TSStr, TObj)),
    TBottom
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_b, TSStr, TObj), dict_map(test_map_a)),
    TBottom
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_a, TSStr, TObj), dict_n(TSStr, TObj)),
    TBottom
  );
  EXPECT_EQ(
    intersection_of(dict_map(test_map_a, TSStr, TObj), dict_n(TSStr, TNum)),
    dict_map(test_map_a)
  );
  EXPECT_EQ(
    intersection_of(
      dict_map(test_map_a, TSStr, TInitCell),
      dict_n(TSStr, TNum)
    ),
    dict_map(test_map_a, TSStr, TNum)
  );

  EXPECT_EQ(
    dict_set(dict_map(test_map_b), TSStr, TStr).first,
    dict_map(test_map_g, TSStr, TStr)
  );
  EXPECT_EQ(
    dict_set(dict_map(test_map_a), sval(s_B.get()), TStr).first,
    dict_map(test_map_h)
  );
  EXPECT_EQ(
    dict_set(dict_map(test_map_a), sval(s_test.get()), TStr).first,
    dict_map(test_map_i)
  );
  EXPECT_EQ(
    dict_set(dict_map(test_map_a, TSStr, TInt), sval(s_test.get()), TStr).first,
    dict_map(test_map_a, TSStr, TArrKey)
  );
  EXPECT_EQ(
    dict_set(
      dict_map(test_map_a, sval(s_test.get()), TInt),
      sval(s_test.get()),
      TStr
    ).first,
    dict_map(test_map_j)
  );
}

TEST(Type, ContextDependent) {
  // This only covers basic cases involving objects.  More testing should
  // be added for non object types, and nested types.
  auto const program = make_test_program();
  auto const unit = program->units.back().get();
  auto const func = [&]() -> php::Func* {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return f.get();
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{program.get()};

  // load classes in hierarchy  Base -> B -> BB
  auto const clsBase = idx.resolve_class(ctx, s_Base.get());
  if (!clsBase) ADD_FAILURE();
  auto const clsB = idx.resolve_class(ctx, s_B.get());
  if (!clsB) ADD_FAILURE();
  auto const clsBB = idx.resolve_class(ctx, s_BB.get());
  if (!clsBB) ADD_FAILURE();
  // Unrelated class.
  auto const clsUn = idx.resolve_class(ctx, s_TestClass.get());
  if (!clsUn) ADD_FAILURE();

  auto const objExactBaseTy     = objExact(*clsBase);
  auto const thisObjExactBaseTy = setctx(objExact(*clsBase));
  auto const subObjBaseTy       = subObj(*clsBase);
  auto const thisSubObjBaseTy   = setctx(subObj(*clsBase));

  auto const objExactBTy        = objExact(*clsB);
  auto const thisObjExactBTy    = setctx(objExact(*clsB));
  auto const subObjBTy          = subObj(*clsB);
  auto const thisSubObjBTy      = setctx(subObj(*clsB));
  auto const clsExactBTy        = clsExact(*clsB);
  auto const thisClsExactBTy    = setctx(clsExact(*clsB));
  auto const subClsBTy          = subCls(*clsB);
  auto const thisSubClsBTy      = setctx(subCls(*clsB));

  auto const objExactBBTy       = objExact(*clsBB);
  auto const thisObjExactBBTy   = setctx(objExact(*clsBB));
  auto const subObjBBTy         = subObj(*clsBB);
  auto const thisSubObjBBTy     = setctx(subObj(*clsBB));
  auto const clsExactBBTy       = clsExact(*clsBB);
  auto const thisClsExactBBTy   = setctx(clsExact(*clsBB));
  auto const subClsBBTy         = subCls(*clsBB);
  auto const thisSubClsBBTy     = setctx(subCls(*clsBB));

  auto const objExactUnTy       = objExact(*clsUn);
  auto const thisObjExactUnTy   = setctx(objExact(*clsUn));
  auto const subObjUnTy         = subObj(*clsUn);
  auto const thisSubObjUnTy     = setctx(subObj(*clsUn));

#define REFINE_EQ(A, B) \
  EXPECT_TRUE((A).equivalentlyRefined((B)))
#define REFINE_NEQ(A, B) \
  EXPECT_FALSE((A).equivalentlyRefined((B)))

  // check that improving any non context dependent type does not change the
  // type whether or not the context is related.
  REFINE_EQ(return_with_context(objExactBaseTy, objExactBTy),
            objExactBaseTy);
  REFINE_EQ(return_with_context(subObjBaseTy, objExactBTy),
            subObjBaseTy);
  REFINE_EQ(return_with_context(objExactBTy, objExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(subObjBTy, objExactBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(objExactBBTy, objExactBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(subObjBBTy, objExactBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(objExactUnTy, objExactBTy),
            objExactUnTy);
  REFINE_EQ(return_with_context(subObjUnTy, objExactBTy),
            subObjUnTy);
  REFINE_EQ(return_with_context(objExactBaseTy, clsExactBTy),
            objExactBaseTy);
  REFINE_EQ(return_with_context(subObjBaseTy, clsExactBTy),
            subObjBaseTy);
  REFINE_EQ(return_with_context(objExactBTy, clsExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(subObjBTy, clsExactBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(objExactBBTy, clsExactBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(subObjBBTy, clsExactBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(objExactUnTy, clsExactBTy),
            objExactUnTy);
  REFINE_EQ(return_with_context(subObjUnTy, clsExactBTy),
            subObjUnTy);

  // With sub.
  REFINE_EQ(return_with_context(objExactBaseTy, subObjBTy),
            objExactBaseTy);
  REFINE_EQ(return_with_context(subObjBaseTy, subObjBTy),
            subObjBaseTy);
  REFINE_EQ(return_with_context(objExactBTy, subObjBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(subObjBTy, subObjBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(objExactBBTy, subObjBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(subObjBBTy, subObjBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(objExactUnTy, subObjBTy),
            objExactUnTy);
  REFINE_EQ(return_with_context(subObjUnTy, subObjBTy),
            subObjUnTy);
  REFINE_EQ(return_with_context(objExactBaseTy, subClsBTy),
            objExactBaseTy);
  REFINE_EQ(return_with_context(subObjBaseTy, subClsBTy),
            subObjBaseTy);
  REFINE_EQ(return_with_context(objExactBTy, subClsBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(subObjBTy, subClsBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(objExactBBTy, subClsBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(subObjBBTy, subClsBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(objExactUnTy, subClsBTy),
            objExactUnTy);
  REFINE_EQ(return_with_context(subObjUnTy, subClsBTy),
            subObjUnTy);

  // Improvements (exact)
  REFINE_EQ(return_with_context(thisObjExactBaseTy, objExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, objExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, objExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, objExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, objExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBBTy, objExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactUnTy, objExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, objExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, clsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, clsExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, clsExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, clsExactBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, clsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBBTy, clsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactUnTy, clsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, clsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, thisObjExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, thisObjExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, thisObjExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, thisObjExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, thisObjExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBBTy, thisObjExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactUnTy, thisObjExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, thisObjExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, thisClsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, thisClsExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, thisClsExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, thisClsExactBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, thisClsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBBTy, thisClsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactUnTy, thisClsExactBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, thisClsExactBTy),
            TBottom);

  // Improvements (sub)
  REFINE_EQ(return_with_context(thisObjExactBaseTy, subObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, subObjBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, subObjBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, subObjBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, subObjBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(thisSubObjBBTy, subObjBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(thisObjExactUnTy, subObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, subObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, subClsBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, subClsBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, subClsBTy),
            objExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, subClsBTy),
            subObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, subClsBTy),
            objExactBBTy);
  REFINE_EQ(return_with_context(thisSubObjBBTy, subClsBTy),
            subObjBBTy);
  REFINE_EQ(return_with_context(thisObjExactUnTy, subClsBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, subClsBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, thisSubObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, thisSubObjBTy),
            thisSubObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, thisSubObjBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, thisSubObjBTy),
            thisSubObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, thisSubObjBTy),
            thisObjExactBBTy);
  REFINE_EQ(return_with_context(thisSubObjBBTy, thisSubObjBTy),
            thisSubObjBBTy);
  REFINE_EQ(return_with_context(thisObjExactUnTy, thisSubObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, thisSubObjBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisObjExactBaseTy, thisSubClsBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjBaseTy, thisSubClsBTy),
            thisSubObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBTy, thisSubClsBTy),
            thisObjExactBTy);
  REFINE_EQ(return_with_context(thisSubObjBTy, thisSubClsBTy),
            thisSubObjBTy);
  REFINE_EQ(return_with_context(thisObjExactBBTy, thisSubClsBTy),
            thisObjExactBBTy);
  REFINE_EQ(return_with_context(thisSubObjBBTy, thisSubClsBTy),
            thisSubObjBBTy);
  REFINE_EQ(return_with_context(thisObjExactUnTy, thisSubClsBTy),
            TBottom);
  REFINE_EQ(return_with_context(thisSubObjUnTy, thisSubClsBTy),
            TBottom);

  // Optional type preservation.
  REFINE_EQ(return_with_context(opt(subObjBaseTy), objExactBTy),
            opt(subObjBaseTy));
  REFINE_EQ(return_with_context(opt(subObjBaseTy), clsExactBTy),
            opt(subObjBaseTy));
  REFINE_EQ(return_with_context(opt(subObjBaseTy), subObjBTy),
            opt(subObjBaseTy));
  REFINE_EQ(return_with_context(opt(subObjBaseTy), subClsBTy),
            opt(subObjBaseTy));
  REFINE_EQ(return_with_context(opt(thisSubObjBaseTy), objExactBTy),
            opt(objExactBTy));
  REFINE_EQ(return_with_context(opt(thisSubObjBaseTy), clsExactBTy),
            opt(objExactBTy));
  REFINE_EQ(return_with_context(opt(thisSubObjBaseTy), thisObjExactBTy),
            opt(thisObjExactBTy));
  REFINE_EQ(return_with_context(opt(thisSubObjBaseTy), thisClsExactBTy),
            opt(thisObjExactBTy));


  // Refinedness operators.
  REFINE_EQ(objExactBTy, objExactBTy);
  REFINE_EQ(subObjBTy, subObjBTy);
  REFINE_EQ(clsExactBTy, clsExactBTy);
  REFINE_EQ(subClsBTy, subClsBTy);
  REFINE_EQ(thisObjExactBTy, thisObjExactBTy);
  REFINE_EQ(thisSubObjBTy, thisSubObjBTy);
  REFINE_EQ(thisClsExactBTy, thisClsExactBTy);
  REFINE_EQ(thisSubClsBTy, thisSubClsBTy);

  REFINE_NEQ(objExactBTy, thisObjExactBTy);
  REFINE_NEQ(subObjBTy, thisSubObjBTy);
  REFINE_NEQ(clsExactBTy, thisClsExactBTy);
  REFINE_NEQ(subClsBTy, thisSubClsBTy);
  REFINE_NEQ(thisObjExactBTy, objExactBTy);
  REFINE_NEQ(thisSubObjBTy, subObjBTy);
  REFINE_NEQ(thisClsExactBTy, clsExactBTy);
  REFINE_NEQ(thisSubClsBTy, subClsBTy);

  EXPECT_FALSE(objExactBTy.moreRefined(thisObjExactBTy));
  EXPECT_FALSE(subObjBTy.moreRefined(thisSubObjBTy));
  EXPECT_FALSE(clsExactBTy.moreRefined(thisClsExactBTy));
  EXPECT_FALSE(subClsBTy.moreRefined(thisSubClsBTy));

  EXPECT_TRUE(thisObjExactBTy.moreRefined(objExactBTy));
  EXPECT_TRUE(thisSubObjBTy.moreRefined(subObjBTy));
  EXPECT_TRUE(thisClsExactBTy.moreRefined(clsExactBTy));
  EXPECT_TRUE(thisSubClsBTy.moreRefined(subClsBTy));

  EXPECT_TRUE(thisObjExactBTy.moreRefined(thisObjExactBTy));
  EXPECT_TRUE(thisSubObjBTy.moreRefined(thisSubObjBTy));
  EXPECT_TRUE(thisClsExactBTy.moreRefined(thisClsExactBTy));
  EXPECT_TRUE(thisSubClsBTy.moreRefined(thisSubClsBTy));

  EXPECT_FALSE(thisObjExactBTy.strictlyMoreRefined(thisObjExactBTy));
  EXPECT_FALSE(thisSubObjBTy.strictlyMoreRefined(thisSubObjBTy));
  EXPECT_FALSE(thisClsExactBTy.strictlyMoreRefined(thisClsExactBTy));
  EXPECT_FALSE(thisSubClsBTy.strictlyMoreRefined(thisSubClsBTy));

  EXPECT_FALSE(thisObjExactBBTy.strictlyMoreRefined(thisObjExactBTy));
  EXPECT_TRUE(thisSubObjBBTy.strictlyMoreRefined(thisSubObjBTy));
  EXPECT_FALSE(thisClsExactBBTy.strictlyMoreRefined(thisClsExactBTy));
  EXPECT_TRUE(thisSubClsBBTy.strictlyMoreRefined(thisSubClsBTy));

  EXPECT_FALSE(thisObjExactBTy.strictlyMoreRefined(thisObjExactBBTy));
  EXPECT_FALSE(thisSubObjBTy.strictlyMoreRefined(thisSubObjBBTy));
  EXPECT_FALSE(thisClsExactBTy.strictlyMoreRefined(thisClsExactBBTy));
  EXPECT_FALSE(thisSubClsBTy.strictlyMoreRefined(thisSubClsBBTy));

  EXPECT_FALSE(objExactBBTy.strictlyMoreRefined(thisObjExactBTy));
  EXPECT_FALSE(subObjBBTy.strictlyMoreRefined(thisSubObjBTy));
  EXPECT_FALSE(clsExactBBTy.strictlyMoreRefined(thisClsExactBTy));
  EXPECT_FALSE(subClsBBTy.strictlyMoreRefined(thisSubClsBTy));

  // Normal equality should still hold.
  EXPECT_EQ(objExactBTy, thisObjExactBTy);
  EXPECT_EQ(subObjBTy, thisSubObjBTy);
  EXPECT_EQ(clsExactBTy, thisClsExactBTy);
  EXPECT_EQ(subClsBTy, thisSubClsBTy);
  EXPECT_EQ(thisObjExactBTy, objExactBTy);
  EXPECT_EQ(thisSubObjBTy, subObjBTy);
  EXPECT_EQ(thisClsExactBTy, clsExactBTy);
  EXPECT_EQ(thisSubClsBTy, subClsBTy);

#undef REFINE_NEQ
#undef REFINE_EQ
}

TEST(Type, ArrLike) {
  const std::initializer_list<std::pair<Type, Type>> subtype_true{
    // Expect all static arrays to be subtypes
    { TSArr,    TArrLike },
    { TSKeyset, TArrLike },
    { TSDict,   TArrLike },
    { TSVec,    TArrLike },
    // Expect other arrays to be subtypes
    { TArr,     TArrLike },
    { TKeyset,  TArrLike },
    { TDict,    TArrLike },
    { TVec,     TArrLike },
    // Expect VArray and DArray to be subtypes
    { TDArr,    TArrLike },
    { TVArr,    TArrLike },
    // Expect ClsMeth to be included in ArrLikeCompat
    { TClsMeth, TArrLikeCompat },
  };

  const std::initializer_list<std::pair<Type, Type>> subtype_false{
    // ClsMeth is not an array
    { TClsMeth, TArrLike },
    // Ints are not arrays
    { TInt,     TArrLike },
    // ArrLike doesn't contain null
    { TOptVec,  TArrLike },
  };

  const std::initializer_list<std::pair<Type, Type>> couldbe_true{
    { TArrLike, TVecCompat },
    { TArrLike, TOptKeysetE },
    { TArrLike, TArrCompatSA },
  };

  const std::initializer_list<std::pair<Type, Type>> couldbe_false{
    { TArrLike, TPrim },
    { TArrLike, TNull },
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
    EXPECT_FALSE(kv.first.couldBe(kv.second))
      << show(kv.first) << " !couldbe " << show(kv.second);
    EXPECT_FALSE(kv.second.couldBe(kv.first))
      << show(kv.first) << " !couldbe " << show(kv.second);
  }

}
//////////////////////////////////////////////////////////////////////

}}
