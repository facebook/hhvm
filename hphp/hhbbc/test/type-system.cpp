/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/index.h"
#include "hphp/runtime/vm/as.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_test("test");
const StaticString s_TestClass("TestClass");

// A test program so we can actually test things involving object or
// class types.
std::unique_ptr<php::Unit> make_test_unit() {
  assert(SystemLib::s_inited);
  std::string const hhas = R"(
    .main {
      Int 1
      RetC
    }

    .class [unique] TestClass {
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

//////////////////////////////////////////////////////////////////////

auto const with_data = {
  ival(2),
  dval(2.0),
  sval(s_test.get())
};

auto const primitives = {
  TUninit,
  TInitNull,
  TFalse,
  TTrue,
  TInt,
  TDbl,
  TSStr,
  TCStr,
  TSArr,
  TCArr,
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
  TOptSStr,
  TOptCStr,
  TOptStr,
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
  TStr,
  TArr,
  TInitUnc,
  TUnc,
  TTop,
};

auto const all_unions = boost::join(optionals, non_opt_unions);
auto const all = boost::join(boost::join(with_data, primitives), all_unions);

//////////////////////////////////////////////////////////////////////

}

TEST(Type, Top) {
  // Everything is a subtype of Top, couldBe Top, and the union of Top
  // with anything is Top.
  for (auto& t : all) {
    EXPECT_TRUE(t.subtypeOf(TTop));
    EXPECT_TRUE(t.couldBe(TTop));
    EXPECT_TRUE(union_of(t, TTop) == TTop);
    EXPECT_TRUE(union_of(TTop, t) == TTop);
  }
}

TEST(Type, Bottom) {
  // Bottom is a subtype of everything, nothing couldBe Bottom, and
  // the union_of anything with Bottom is itself.
  for (auto& t : all) {
    EXPECT_TRUE(TBottom.subtypeOf(t));
    EXPECT_TRUE(!TBottom.couldBe(t));
    EXPECT_TRUE(union_of(t, TBottom) == t);
    EXPECT_TRUE(union_of(TBottom, t) == t);
  }
}

TEST(Type, Prims) {
  // All pairs of non-equivalent primitives are not related by either
  // subtypeOf or couldBe.
  for (auto& t1 : primitives) {
    for (auto& t2 : primitives) {
      if (t1 != t2) {
        EXPECT_TRUE(!t1.subtypeOf(t2) && !t2.subtypeOf(t1));
        EXPECT_TRUE(!t1.couldBe(t2));
        EXPECT_TRUE(!t2.couldBe(t1));
      }
    }
  }
}

TEST(Type, Relations) {
  // couldBe is symmetric and reflexive
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      EXPECT_TRUE(t1.couldBe(t2) == t2.couldBe(t1));
    }
  }
  for (auto& t1 : all) EXPECT_TRUE(t1.couldBe(t1));

  // subtype is antisymmetric and reflexive
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      if (t1 != t2) {
        EXPECT_TRUE(!(t1.subtypeOf(t2) && t2.subtypeOf(t1)));
      }
    }
  }
  for (auto& t1 : all) EXPECT_TRUE(t1.subtypeOf(t1));

  // union_of is commutative
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      EXPECT_TRUE(union_of(t1, t2) == union_of(t2, t1));
    }
  }
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
  for (auto& t : all) {
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
  };

  auto false_cases = std::initializer_list<std::pair<Type,Type>> {
    { opt(sval(s_test.get())), TCStr },
    { opt(ival(2)), TDbl },
    { opt(dval(2.0)), TInt },
    { opt(TFalse), TTrue },
    { opt(TTrue), TFalse },
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
  auto const program = folly::make_unique<php::Program>();
  program->units.push_back(make_test_unit());
  auto const unit = borrow(program->units.back());
  auto const func = [&]() -> borrowed_ptr<php::Func> {
    for (auto& f : unit->funcs) {
      if (f->name->isame(s_test.get())) return borrow(f);
    }
    return nullptr;
  }();
  EXPECT_TRUE(func != nullptr);

  auto const ctx = Context { unit, func };
  Index idx{borrow(program), Options{}};
  auto const cls = idx.resolve_class(ctx, s_TestClass.get());
  if (!cls) EXPECT_TRUE(false);

  auto const objExactTy = objExact(*cls);
  auto const subObjTy   = subObj(*cls);
  auto const clsExactTy = clsExact(*cls);
  auto const subClsTy   = subCls(*cls);

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
}

//////////////////////////////////////////////////////////////////////

}}
