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
#include "hphp/hhbbc/interp.h"

#include <algorithm>
#include <vector>
#include <string>
#include <iterator>

#include <folly/Optional.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/ext/hh/ext_hh.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-ops.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

#include "hphp/hhbbc/interp-internal.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_Throwable("__SystemLib\\Throwable");
const StaticString s_empty("");
const StaticString s_construct("__construct");
const StaticString s_86ctor("86ctor");
const StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
const StaticString s_IMemoizeParam("HH\\IMemoizeParam");
const StaticString s_getInstanceKey("getInstanceKey");
const StaticString s_Closure("Closure");
const StaticString s_byRefWarn("Only variables should be passed by reference");
const StaticString s_byRefError("Only variables can be passed by reference");
const StaticString s_trigger_error("trigger_error");
}

//////////////////////////////////////////////////////////////////////

void impl_vec(ISS& env, bool reduce, std::vector<Bytecode>&& bcs) {
  std::vector<Bytecode> currentReduction;
  if (!options.StrengthReduce) reduce = false;

  for (auto it = begin(bcs); it != end(bcs); ++it) {
    assert(env.flags.jmpFlag == StepFlags::JmpFlags::Either &&
           "you can't use impl with branching opcodes before last position");

    auto const wasPEI = env.flags.wasPEI;
    auto const canConstProp = env.flags.canConstProp;

    FTRACE(3, "    (impl {}\n", show(env.ctx.func, *it));
    env.flags.wasPEI          = true;
    env.flags.canConstProp    = false;
    env.flags.strengthReduced = folly::none;
    default_dispatch(env, *it);

    if (env.flags.strengthReduced) {
      if (instrFlags(env.flags.strengthReduced->back().op) & TF) {
        unreachable(env);
      }
      if (reduce) {
        std::move(begin(*env.flags.strengthReduced),
                  end(*env.flags.strengthReduced),
                  std::back_inserter(currentReduction));
      }
    } else {
      if (instrFlags(it->op) & TF) {
        unreachable(env);
      }
      if (reduce) {
        if (env.flags.canConstProp &&
            env.collect.propagate_constants &&
            env.collect.propagate_constants(*it, env.state, currentReduction)) {
          env.flags.canConstProp = false;
          env.flags.wasPEI = false;
        } else {
          currentReduction.push_back(std::move(*it));
        }
      }
    }

    // If any of the opcodes in the impl list said they could throw,
    // then the whole thing could throw.
    env.flags.wasPEI = env.flags.wasPEI || wasPEI;
    env.flags.canConstProp = env.flags.canConstProp && canConstProp;
    if (env.state.unreachable) break;
  }

  if (reduce) {
    env.flags.strengthReduced = std::move(currentReduction);
  } else {
    env.flags.strengthReduced = folly::none;
  }
}

namespace interp_step {

void in(ISS& env, const bc::Nop&)  { nothrow(env); }
void in(ISS& env, const bc::DiscardClsRef& op) {
  nothrow(env);
  takeClsRefSlot(env, op.slot);
}
void in(ISS& env, const bc::PopC&) { nothrow(env); popC(env); }
void in(ISS& env, const bc::PopV&) { nothrow(env); popV(env); }
void in(ISS& env, const bc::PopU&) { nothrow(env); popU(env); }
void in(ISS& env, const bc::PopR&) {
  auto t = topT(env, 0);
  if (t.subtypeOf(TCell)) {
    return reduce(env, bc::UnboxRNop {}, bc::PopC {});
  }
  nothrow(env);
  popR(env);
}

void in(ISS& env, const bc::EntryNop&) { nothrow(env); }

void in(ISS& env, const bc::Dup& /*op*/) {
  nothrow(env);
  auto val = popC(env);
  push(env, val);
  push(env, std::move(val));
}

void in(ISS& env, const bc::AssertRATL& op) {
  mayReadLocal(env, op.loc1);
  nothrow(env);
}

void in(ISS& env, const bc::AssertRATStk&) {
  nothrow(env);
}

void in(ISS& env, const bc::BreakTraceHint&) { nothrow(env); }

void in(ISS& env, const bc::Box&) {
  nothrow(env);
  popC(env);
  push(env, TRef);
}

void in(ISS& env, const bc::BoxR&) {
  nothrow(env);
  if (topR(env).subtypeOf(TRef)) {
    return reduce(env, bc::BoxRNop {});
  }
  popR(env);
  push(env, TRef);
}

void in(ISS& env, const bc::Unbox&) {
  nothrow(env);
  popV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::UnboxR&) {
  auto const t = topR(env);
  if (t.subtypeOf(TInitCell)) return reduce(env, bc::UnboxRNop {});
  nothrow(env);
  popT(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::RGetCNop&) { nothrow(env); }

void in(ISS& env, const bc::CGetCUNop&) {
  nothrow(env);
  auto const t = popCU(env);
  push(env, remove_uninit(t));
}

void in(ISS& env, const bc::UGetCUNop&) {
  nothrow(env);
  popCU(env);
  push(env, TUninit);
}

void in(ISS& env, const bc::UnboxRNop&) {
  nothrow(env);
  constprop(env);
  auto t = popR(env);
  if (!t.subtypeOf(TInitCell)) t = TInitCell;
  push(env, std::move(t));
}

void in(ISS& env, const bc::BoxRNop&) {
  nothrow(env);
  auto t = popR(env);
  if (!t.subtypeOf(TRef)) t = TRef;
  push(env, std::move(t));
}

void in(ISS& env, const bc::Null&)      { nothrow(env); push(env, TInitNull); }
void in(ISS& env, const bc::NullUninit&){ nothrow(env); push(env, TUninit); }
void in(ISS& env, const bc::True&)      { nothrow(env); push(env, TTrue); }
void in(ISS& env, const bc::False&)     { nothrow(env); push(env, TFalse); }

void in(ISS& env, const bc::Int& op) {
  nothrow(env);
  push(env, ival(op.arg1));
}

void in(ISS& env, const bc::Double& op) {
  nothrow(env);
  push(env, dval(op.dbl1));
}

void in(ISS& env, const bc::String& op) {
  nothrow(env);
  push(env, sval(op.str1));
}

void in(ISS& env, const bc::Array& op) {
  assert(op.arr1->isPHPArray());
  nothrow(env);
  push(env, aval(op.arr1));
}

void in(ISS& env, const bc::Vec& op) {
  assert(op.arr1->isVecArray());
  nothrow(env);
  push(env, vec_val(op.arr1));
}

void in(ISS& env, const bc::Dict& op) {
  assert(op.arr1->isDict());
  nothrow(env);
  push(env, dict_val(op.arr1));
}

void in(ISS& env, const bc::Keyset& op) {
  assert(op.arr1->isKeyset());
  nothrow(env);
  push(env, keyset_val(op.arr1));
}

void in(ISS& env, const bc::NewArray& op) {
  push(env, op.arg1 == 0 ? aempty() : counted_aempty());
}

void in(ISS& env, const bc::NewDictArray& op) {
  push(env, op.arg1 == 0 ? dict_empty() : counted_dict_empty());
}

void in(ISS& env, const bc::NewMixedArray& op) {
  push(env, op.arg1 == 0 ? aempty() : counted_aempty());
}

void in(ISS& env, const bc::NewPackedArray& op) {
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  push(env, arr_packed(std::move(elems)));
  constprop(env);
}

void in(ISS& env, const bc::NewStructArray& op) {
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, arr_map(std::move(map)));
  constprop(env);
}

void in(ISS& env, const bc::NewVecArray& op) {
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  constprop(env);
  push(env, vec(std::move(elems)));
}

void in(ISS& env, const bc::NewKeysetArray& op) {
  assert(op.arg1 > 0);
  auto map = MapElems{};
  auto ty = TBottom;
  auto useMap = true;
  auto bad = false;
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    auto k = disect_strict_key(popC(env));
    if (k.type == TBottom) {
      bad = true;
      useMap = false;
    }
    if (useMap) {
      if (auto const v = k.tv()) {
        map.emplace_front(*v, k.type);
      } else {
        useMap = false;
      }
    }
    ty |= std::move(k.type);
  }
  if (useMap) {
    push(env, keyset_map(std::move(map)));
    constprop(env);
  } else if (!bad) {
    push(env, keyset_n(ty));
  } else {
    unreachable(env);
    push(env, TBottom);
  }
}

void in(ISS& env, const bc::NewLikeArrayL& op) {
  locAsCell(env, op.loc1);
  push(env, counted_aempty());
}

void in(ISS& env, const bc::AddElemC& /*op*/) {
  auto const v = popC(env);
  auto const k = popC(env);

  auto const outTy = [&] (Type ty) -> folly::Optional<std::pair<Type,bool>> {
    if (ty.subtypeOf(TArr)) {
      return array_set(std::move(ty), k, v);
    }
    if (ty.subtypeOf(TDict)) {
      return dict_set(std::move(ty), k, v);
    }
    return folly::none;
  }(popC(env));

  if (!outTy) {
    return push(env, union_of(TArr, TDict));
  }

  if (outTy->first.subtypeOf(TBottom)) {
    unreachable(env);
  } else if (outTy->second) {
    nothrow(env);
    if (env.collect.trackConstantArrays) constprop(env);
  }
  push(env, std::move(outTy->first));
}

void in(ISS& env, const bc::AddElemV& /*op*/) {
  popV(env); popC(env);
  auto const ty = popC(env);
  auto const outTy =
    ty.subtypeOf(TArr) ? TArr
    : ty.subtypeOf(TDict) ? TDict
    : union_of(TArr, TDict);
  push(env, outTy);
}

void in(ISS& env, const bc::AddNewElemC&) {
  auto v = popC(env);

  auto const outTy = [&] (Type ty) -> folly::Optional<Type> {
    if (ty.subtypeOf(TArr)) {
      return array_newelem(std::move(ty), std::move(v));
    }
    return folly::none;
  }(popC(env));

  if (!outTy) {
    return push(env, TInitCell);
  }

  if (outTy->subtypeOf(TBottom)) {
    unreachable(env);
  } else {
    if (env.collect.trackConstantArrays) constprop(env);
  }
  push(env, std::move(*outTy));
}

void in(ISS& env, const bc::AddNewElemV&) {
  popV(env);
  popC(env);
  push(env, TArr);
}

void in(ISS& env, const bc::NewCol& op) {
  auto const type = static_cast<CollectionType>(op.subop1);
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
}

void in(ISS& env, const bc::NewPair& /*op*/) {
  popC(env); popC(env);
  auto const name = collections::typeToString(CollectionType::Pair);
  push(env, objExact(env.index.builtin_class(name)));
}

void in(ISS& env, const bc::ColFromArray& op) {
  popC(env);
  auto const type = static_cast<CollectionType>(op.subop1);
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
}

void doCns(ISS& env, SString str)  {
  auto t = env.index.lookup_constant(env.ctx, str);
  if (!t) {
    // There's no entry for this constant in the index. It must be
    // the first iteration, so we'll add a dummy entry to make sure
    // there /is/ something next time around.
    Cell val;
    val.m_type = kReadOnlyConstant;
    env.collect.cnsMap.emplace(str, val);
    t = TInitCell;
    // make sure we're re-analyzed
    env.collect.readsUntrackedConstants = true;
  } else if (t->strictSubtypeOf(TInitCell)) {
    nothrow(env);
    constprop(env);
  }
  push(env, std::move(*t));
}

void in(ISS& env, const bc::Cns& op)  { doCns(env, op.str1); }
void in(ISS& env, const bc::CnsE& op) { doCns(env, op.str1); }
void in(ISS& env, const bc::CnsU&)    { push(env, TInitCell); }

void in(ISS& env, const bc::ClsCns& op) {
  auto const& t1 = peekClsRefSlot(env, op.slot);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    if (dcls.type == DCls::Exact) {
      return reduce(env, bc::DiscardClsRef { op.slot },
                         bc::ClsCnsD { op.str1, dcls.cls.name() });
    }
  }
  takeClsRefSlot(env, op.slot);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ClsCnsD& op) {
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str2)) {
    auto t = env.index.lookup_class_constant(env.ctx, *rcls, op.str1);
    if (options.HardConstProp) constprop(env);
    push(env, std::move(t));
    return;
  }
  push(env, TInitCell);
}

void in(ISS& env, const bc::File&)   { nothrow(env); push(env, TSStr); }
void in(ISS& env, const bc::Dir&)    { nothrow(env); push(env, TSStr); }
void in(ISS& env, const bc::Method&) { nothrow(env); push(env, TSStr); }

void in(ISS& env, const bc::ClsRefName& op) {
  nothrow(env);
  takeClsRefSlot(env, op.slot);
  push(env, TSStr);
}

void in(ISS& env, const bc::Concat& /*op*/) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    auto to_string_is_safe = [] (const Cell& cell) {
      return
        isStringType(cell.m_type)    ||
        cell.m_type == KindOfNull    ||
        cell.m_type == KindOfBoolean ||
        cell.m_type == KindOfInt64   ||
        cell.m_type == KindOfDouble;
    };
    if (to_string_is_safe(*v1) && to_string_is_safe(*v2)) {
      constprop(env);
      auto const cell = eval_cell([&] {
        auto s = StringData::Make(
          tvAsCVarRef(&*v2).toString().get(),
          tvAsCVarRef(&*v1).toString().get());
        return make_tv<KindOfString>(s);
      });
      return push(env, cell ? *cell : TInitCell);
    }
  }
  // Not nothrow even if both are strings: can throw for strings
  // that are too large.
  push(env, TStr);
}

void in(ISS& env, const bc::ConcatN& op) {
  auto n = op.arg1;
  assert(n > 1);
  assert(n < 5);

  for (auto i = 0; i < n; ++i) {
    popC(env);
  }
  push(env, TStr);
}

template <class Op, class Fun>
void arithImpl(ISS& env, const Op& /*op*/, Fun fun) {
  constprop(env);
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  push(env, fun(t2, t1));
}

void in(ISS& env, const bc::Add& op)    { arithImpl(env, op, typeAdd); }
void in(ISS& env, const bc::Sub& op)    { arithImpl(env, op, typeSub); }
void in(ISS& env, const bc::Mul& op)    { arithImpl(env, op, typeMul); }
void in(ISS& env, const bc::Div& op)    { arithImpl(env, op, typeDiv); }
void in(ISS& env, const bc::Mod& op)    { arithImpl(env, op, typeMod); }
void in(ISS& env, const bc::Pow& op)    { arithImpl(env, op, typePow); }
void in(ISS& env, const bc::BitAnd& op) { arithImpl(env, op, typeBitAnd); }
void in(ISS& env, const bc::BitOr& op)  { arithImpl(env, op, typeBitOr); }
void in(ISS& env, const bc::BitXor& op) { arithImpl(env, op, typeBitXor); }
void in(ISS& env, const bc::AddO& op)   { arithImpl(env, op, typeAddO); }
void in(ISS& env, const bc::SubO& op)   { arithImpl(env, op, typeSubO); }
void in(ISS& env, const bc::MulO& op)   { arithImpl(env, op, typeMulO); }
void in(ISS& env, const bc::Shl& op)    { arithImpl(env, op, typeShl); }
void in(ISS& env, const bc::Shr& op)    { arithImpl(env, op, typeShr); }

void in(ISS& env, const bc::BitNot& /*op*/) {
  auto const t = popC(env);
  auto const v = tv(t);
  if (v) {
    constprop(env);
    auto cell = eval_cell([&] {
      auto c = *v;
      cellBitNot(c);
      return c;
    });
    if (cell) return push(env, std::move(*cell));
  }
  push(env, TInitCell);
}

namespace {

bool couldBeHackArr(Type t) {
  return t.couldBe(TVec) || t.couldBe(TDict) || t.couldBe(TKeyset);
}

}

template<bool Negate>
void sameImpl(ISS& env) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);

  auto const mightWarn = [&]{
    // EvalHackArrCompatNotices will notice on === and !== between PHP arrays
    // and Hack arrays.
    if (!RuntimeOption::EvalHackArrCompatNotices) return false;
    if (t1.couldBe(TArr) && couldBeHackArr(t2)) return true;
    if (couldBeHackArr(t1) && t2.couldBe(TArr)) return true;
    return false;
  }();
  if (!mightWarn) {
    nothrow(env);
    constprop(env);
  }

  if (v1 && v2) {
    if (auto r = eval_cell_value([&]{ return cellSame(*v2, *v1); })) {
      return push(env, r != Negate ? TTrue : TFalse);
    }
  }
  push(env, Negate ? typeNSame(t1, t2) : typeSame(t1, t2));
}

void in(ISS& env, const bc::Same&)  { sameImpl<false>(env); }
void in(ISS& env, const bc::NSame&) { sameImpl<true>(env); }

template<class Fun>
void binOpBoolImpl(ISS& env, Fun fun) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    if (auto r = eval_cell_value([&]{ return fun(*v2, *v1); })) {
      constprop(env);
      return push(env, *r ? TTrue : TFalse);
    }
  }
  // TODO_4: evaluate when these can throw, non-constant type stuff.
  push(env, TBool);
}

template<class Fun>
void binOpInt64Impl(ISS& env, Fun fun) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    if (auto r = eval_cell_value([&]{ return ival(fun(*v2, *v1)); })) {
      constprop(env);
      return push(env, std::move(*r));
    }
  }
  // TODO_4: evaluate when these can throw, non-constant type stuff.
  push(env, TInt);
}

void in(ISS& env, const bc::Eq&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellEqual(c1, c2); });
}
void in(ISS& env, const bc::Neq&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return !cellEqual(c1, c2); });
}
void in(ISS& env, const bc::Lt&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellLess(c1, c2); });
}
void in(ISS& env, const bc::Gt&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellGreater(c1, c2); });
}
void in(ISS& env, const bc::Lte&) { binOpBoolImpl(env, cellLessOrEqual); }
void in(ISS& env, const bc::Gte&) { binOpBoolImpl(env, cellGreaterOrEqual); }

void in(ISS& env, const bc::Cmp&) {
  binOpInt64Impl(env, [&] (Cell c1, Cell c2) { return cellCompare(c1, c2); });
}

void in(ISS& env, const bc::Xor&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

void castBoolImpl(ISS& env, bool negate) {
  nothrow(env);
  constprop(env);

  auto const t = popC(env);
  auto const v = tv(t);
  if (v) {
    auto cell = eval_cell([&] {
      return make_tv<KindOfBoolean>(cellToBool(*v) != negate);
    });
    always_assert_flog(!!cell, "cellToBool should never throw");
    return push(env, std::move(*cell));
  }

  if (t.subtypeOfAny(TArrE, TVecE, TDictE, TKeysetE)) {
    return push(env, negate ? TTrue : TFalse);
  }
  if (t.subtypeOfAny(TArrN, TVecN, TDictN, TKeysetN)) {
    return push(env, negate ? TFalse : TTrue);
  }

  push(env, TBool);
}

void in(ISS& env, const bc::Not&) {
  castBoolImpl(env, true);
}

void in(ISS& env, const bc::CastBool&) {
  auto const t = topC(env);
  if (t.subtypeOf(TBool)) return reduce(env, bc::Nop {});
  castBoolImpl(env, false);
}

void in(ISS& env, const bc::CastInt&) {
  constprop(env);
  auto const t = topC(env);
  if (t.subtypeOf(TInt)) return reduce(env, bc::Nop {});
  popC(env);
  // Objects can raise a warning about converting to int.
  if (!t.couldBe(TObj)) nothrow(env);
  if (auto const v = tv(t)) {
    auto cell = eval_cell([&] {
      return make_tv<KindOfInt64>(cellToInt(*v));
    });
    if (cell) return push(env, std::move(*cell));
  }
  push(env, TInt);
}

void castImpl(ISS& env, Type target, void(*fn)(TypedValue*)) {
  auto const t = topC(env);
  if (t.subtypeOf(target)) return reduce(env, bc::Nop {});
  popC(env);
  if (fn) {
    if (auto val = tv(t)) {
      if (auto result = eval_cell([&] { fn(&*val); return *val; })) {
        constprop(env);
        target = *result;
      }
    }
  }
  push(env, std::move(target));
}

void in(ISS& env, const bc::CastDouble&) {
  castImpl(env, TDbl, tvCastToDoubleInPlace);
}

void in(ISS& env, const bc::CastString&) {
  castImpl(env, TStr, tvCastToStringInPlace);
}

void in(ISS& env, const bc::CastArray&)  {
  castImpl(env, TArr, tvCastToArrayInPlace);
}

void in(ISS& env, const bc::CastObject&) { castImpl(env, TObj, nullptr); }

void in(ISS& env, const bc::CastDict&)   {
  castImpl(env, TDict, tvCastToDictInPlace);
}

void in(ISS& env, const bc::CastVec&)    {
  castImpl(env, TVec, tvCastToVecInPlace);
}

void in(ISS& env, const bc::CastKeyset&) {
  castImpl(env, TKeyset, tvCastToKeysetInPlace);
}

void in(ISS& env, const bc::CastVArray&)  {
  auto const t = popC(env);
  if (auto val = tv(t)) {
    auto result = eval_cell(
      [&] {
        tvCastToVArrayInPlace(&*val);
        return *val;
      }
    );
    if (result) {
      constprop(env);
      return push(env, std::move(*result));
    }
  }
  push(env, TArr);
}

void in(ISS& env, const bc::CastDArray&)  {
  castImpl(env, TArr, tvCastToDArrayInPlace);
}

void in(ISS& env, const bc::Print& /*op*/) {
  popC(env);
  push(env, ival(1));
}

void in(ISS& env, const bc::Clone& /*op*/) {
  auto val = popC(env);
  if (!val.subtypeOf(TObj)) {
    val = is_opt(val) ? unopt(std::move(val)) : TObj;
  }
  push(env, std::move(val));
}

void in(ISS& env, const bc::Exit&)  { popC(env); push(env, TInitNull); }
void in(ISS& env, const bc::Fatal&) { popC(env); }

void in(ISS& /*env*/, const bc::JmpNS&) {
  always_assert(0 && "blocks should not contain JmpNS instructions");
}

void in(ISS& /*env*/, const bc::Jmp&) {
  always_assert(0 && "blocks should not contain Jmp instructions");
}

template<bool Negate, class JmpOp>
void jmpImpl(ISS& env, const JmpOp& op) {
  nothrow(env);
  auto const t1 = popC(env);
  auto const v1 = tv(t1);
  if (v1) {
    auto const taken = !cellToBool(*v1) != Negate;
    if (taken) {
      jmp_nofallthrough(env);
      env.propagate(op.target, env.state);
    } else {
      jmp_nevertaken(env);
    }
    return;
  }
  if (next_real_block(*env.ctx.func, env.blk.fallthrough) ==
      next_real_block(*env.ctx.func, op.target)) {
    jmp_nevertaken(env);
    return;
  }
  env.propagate(op.target, env.state);
}

void in(ISS& env, const bc::JmpNZ& op) { jmpImpl<true>(env, op); }
void in(ISS& env, const bc::JmpZ& op)  { jmpImpl<false>(env, op); }

template<class JmpOp>
void group(ISS& env, const bc::IsTypeL& istype, const JmpOp& jmp) {
  if (istype.subop2 == IsTypeOp::Scalar) return impl(env, istype, jmp);

  auto const loc = derefLoc(env, istype.loc1);
  auto const testTy = type_of_istype(istype.subop2);
  if (loc.subtypeOf(testTy) || !loc.couldBe(testTy)) {
    return impl(env, istype, jmp);
  }

  if (!locCouldBeUninit(env, istype.loc1)) nothrow(env);

  auto const negate = jmp.op == Op::JmpNZ;
  auto const was_true = [&] {
    if (is_opt(loc)) {
      if (testTy.subtypeOf(TNull)) return TInitNull;
      auto const unopted = unopt(loc);
      if (unopted.subtypeOf(testTy)) return unopted;
    }
    return testTy;
  }();
  auto const was_false = [&] {
    if (is_opt(loc)) {
      auto const unopted = unopt(loc);
      if (testTy.subtypeOf(TNull))   return unopted;
      if (unopted.subtypeOf(testTy)) return TInitNull;
    }
    return loc;
  }();

  refineLoc(env, istype.loc1, negate ? was_true : was_false);
  env.propagate(jmp.target, env.state);
  refineLoc(env, istype.loc1, negate ? was_false : was_true);
}

namespace {

folly::Optional<Cell> staticLocHelper(ISS& env, LocalId l, Type init) {
  if (is_volatile_local(env.ctx.func, l)) return folly::none;
  unbindLocalStatic(env, l);
  setLocRaw(env, l, TRef);
  bindLocalStatic(env, l, std::move(init));
  if (!env.ctx.func->isMemoizeWrapper &&
      !env.ctx.func->isClosureBody &&
      env.collect.localStaticTypes.size() > l) {
    auto t = env.collect.localStaticTypes[l];
    if (auto v = tv(t)) {
      useLocalStatic(env, l);
      setLocRaw(env, l, t);
      return v;
    }
  }
  useLocalStatic(env, l);
  return folly::none;
}

// If the current function is a memoize wrapper, return the inferred return type
// of the function being wrapped.
Type memoizeImplRetType(ISS& env) {
  always_assert(env.ctx.func->isMemoizeWrapper);

  // Lookup the wrapped function. This should always resolve to a precise
  // function but we don't rely on it.
  auto const memo_impl_func = [&]{
    if (env.ctx.func->cls) {
      auto const clsTy = selfClsExact(env);
      return env.index.resolve_method(
        env.ctx,
        clsTy ? *clsTy : TCls,
        memoize_impl_name(env.ctx.func)
      );
    }
    return env.index.resolve_func(env.ctx, memoize_impl_name(env.ctx.func));
  }();

  // Infer the return type of the wrapped function, taking into account the
  // types of the parameters for context sensitive types.
  auto const numArgs = env.ctx.func->params.size();
  std::vector<Type> args{numArgs};
  for (auto i = LocalId{0}; i < numArgs; ++i) {
    args[i] = locAsCell(env, i);
  }

  auto retTy = env.index.lookup_return_type(
    CallContext { env.ctx, args },
    memo_impl_func
  );
  // Regardless of anything we know the return type will be an InitCell (this is
  // a requirement of memoize functions).
  if (!retTy.subtypeOf(TInitCell)) return TInitCell;
  return retTy;
}

/*
 * Propagate a more specific type to the taken/fall-through branches of a jmp
 * operation when the jmp is done because of a type test. Given a type `valTy`,
 * being tested against the type `testTy`, propagate `failTy` to the branch
 * representing test failure, and `testTy` to the branch representing test
 * success.
 */
template<class JmpOp>
void typeTestPropagate(ISS& env, Type valTy, Type testTy,
                       Type failTy, const JmpOp& jmp) {
  nothrow(env);
  auto const takenOnSuccess = jmp.op == Op::JmpNZ;

  if (valTy.subtypeOf(testTy) || failTy.subtypeOf(TBottom)) {
    push(env, std::move(valTy));
    if (takenOnSuccess) {
      jmp_nofallthrough(env);
      env.propagate(jmp.target, env.state);
    } else {
      jmp_nevertaken(env);
    }
    return;
  }
  if (!valTy.couldBe(testTy)) {
    push(env, failTy);
    if (takenOnSuccess) {
      jmp_nevertaken(env);
    } else {
      jmp_nofallthrough(env);
      env.propagate(jmp.target, env.state);
    }
    return;
  }

  push(env, std::move(takenOnSuccess ? testTy : failTy));
  env.propagate(jmp.target, env.state);
  discard(env, 1);
  push(env, std::move(takenOnSuccess ? failTy : testTy));
}

}

// After a StaticLocCheck, we know the local is bound on the true path,
// and not changed on the false path.
template<class JmpOp>
void group(ISS& env, const bc::StaticLocCheck& slc, const JmpOp& jmp) {
  auto const takenOnInit = jmp.op == Op::JmpNZ;
  auto save = env.state;

  if (auto const v = staticLocHelper(env, slc.loc1, TBottom)) {
    return impl(env, slc, jmp);
  }

  if (env.collect.localStaticTypes.size() > slc.loc1 &&
      env.collect.localStaticTypes[slc.loc1].subtypeOf(TBottom)) {
    if (takenOnInit) {
      env.state = std::move(save);
      jmp_nevertaken(env);
    } else {
      env.propagate(jmp.target, save);
      jmp_nofallthrough(env);
    }
    return;
  }

  if (takenOnInit) {
    env.propagate(jmp.target, env.state);
    env.state = std::move(save);
  } else {
    env.propagate(jmp.target, save);
  }
}

// If we duplicate a value, and then test its type and Jmp based on that result,
// we can narrow the type of the top of the stack. Only do this for null checks
// right now (because its useful in memoize wrappers).
template<class JmpOp>
void group(ISS& env, const bc::Dup& dup,
           const bc::IsTypeC& istype, const JmpOp& jmp) {
  if (istype.subop1 != IsTypeOp::Scalar) {
    auto const testTy = type_of_istype(istype.subop1);
    if (testTy.subtypeOf(TNull)) {
      auto const valTy = popC(env);
      typeTestPropagate(
        env, valTy, TInitNull, is_opt(valTy) ? unopt(valTy) : valTy, jmp
      );
      return;
    }
  }
  impl(env, dup, istype, jmp);
}

// If we do an IsUninit check and then Jmp based on the check, one branch will
// be the original type minus the Uninit, and the other will be
// Uninit. (IsUninit does not pop the value).
template<class JmpOp>
void group(ISS& env, const bc::IsUninit&, const JmpOp& jmp) {
  auto const valTy = popCU(env);
  typeTestPropagate(env, valTy, TUninit, remove_uninit(valTy), jmp);
}

// A MemoGet, followed by an IsUninit, followed by a Jmp, can have the type of
// the stack inferred very well. The IsUninit success path will be Uninit and
// the failure path will be the inferred return type of the wrapped
// function. This has to be done as a group and not via individual interp()
// calls is because of limitations in HHBBC's type-system. The type that MemoGet
// pushes is the inferred return type of the wrapper function with Uninit added
// in. Unfortunately HHBBC's type-system cannot exactly represent this
// combination, so it gets forced to Cell. By analyzing this triplet as a group,
// we can avoid this loss of type precision.
template <class JmpOp>
void group(ISS& env, const bc::MemoGet& get, const bc::IsUninit& /*isuninit*/,
           const JmpOp& jmp) {
  impl(env, get);
  typeTestPropagate(env, popCU(env), TUninit, memoizeImplRetType(env), jmp);
}

template<class JmpOp>
void group(ISS& env, const bc::CGetL& cgetl, const JmpOp& jmp) {
  auto const loc = derefLoc(env, cgetl.loc1);
  if (tv(loc)) return impl(env, cgetl, jmp);

  if (!locCouldBeUninit(env, cgetl.loc1)) nothrow(env);

  auto const negate = jmp.op == Op::JmpNZ;
  auto const converted_true = [&]() -> const Type {
    if (is_opt(loc)) return unopt(loc);
    if (loc.subtypeOf(TBool)) return TTrue;
    return loc;
  }();
  auto const converted_false = [&]() -> const Type {
    if (!could_have_magic_bool_conversion(loc) && loc.subtypeOf(TOptObj)) {
      return TInitNull;
    }
    if (loc.subtypeOf(TInt))  return ival(0);
    if (loc.subtypeOf(TBool)) return TFalse;
    if (loc.subtypeOf(TDbl))  return dval(0);
    // Can't tell if any of the other ?primitives are going to be
    // null based on this, so leave those types alone.  E.g. a Str
    // might contain "" and be falsey, or an array or collection
    // could be empty.
    return loc;
  }();

  refineLoc(env, cgetl.loc1, negate ? converted_true : converted_false);
  env.propagate(jmp.target, env.state);
  refineLoc(env, cgetl.loc1, negate ? converted_false : converted_true);
}

template<class JmpOp>
void group(ISS& env,
           const bc::CGetL& cgetl,
           const bc::InstanceOfD& inst,
           const JmpOp& jmp) {
  auto bail = [&] { impl(env, cgetl, inst, jmp); };

  if (interface_supports_non_objects(inst.str1)) return bail();
  auto const rcls = env.index.resolve_class(env.ctx, inst.str1);
  if (!rcls) return bail();

  auto const instTy = subObj(*rcls);
  auto const loc = derefLoc(env, cgetl.loc1);
  if (loc.subtypeOf(instTy) || !loc.couldBe(instTy)) {
    return bail();
  }

  auto const negate    = jmp.op == Op::JmpNZ;
  auto const was_true  = instTy;
  auto const was_false = loc;
  refineLoc(env, cgetl.loc1, negate ? was_true : was_false);
  env.propagate(jmp.target, env.state);
  refineLoc(env, cgetl.loc1, negate ? was_false : was_true);
}

void group(ISS& env,
           const bc::CGetL& cgetl,
           const bc::FPushObjMethodD& fpush) {
  auto const obj = locAsCell(env, cgetl.loc1);
  impl(env, cgetl, fpush);
  if (!is_specialized_obj(obj)) {
    refineLoc(env, cgetl.loc1,
              fpush.subop3 == ObjMethodOp::NullThrows ? TObj : TOptObj);
  } else if (is_opt(obj) && fpush.subop3 == ObjMethodOp::NullThrows) {
    refineLoc(env, cgetl.loc1, unopt(obj));
  }
}

void in(ISS& env, const bc::Switch& op) {
  popC(env);
  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, env.state);
  });
}

void in(ISS& env, const bc::SSwitch& op) {
  popC(env);
  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, env.state);
  });
}

void in(ISS& env, const bc::RetC& /*op*/) {
  doRet(env, popC(env));
}
void in(ISS& env, const bc::RetV& /*op*/) {
  doRet(env, popV(env));
}
void in(ISS& /*env*/, const bc::Unwind& /*op*/) {}
void in(ISS& env, const bc::Throw& /*op*/) {
  popC(env);
}

void in(ISS& env, const bc::Catch&) {
  nothrow(env);
  return push(env, subObj(env.index.builtin_class(s_Throwable.get())));
}

void in(ISS& env, const bc::NativeImpl&) {
  killLocals(env);
  mayUseVV(env);

  if (is_collection_method_returning_this(env.ctx.cls, env.ctx.func)) {
    assert(env.ctx.func->attrs & AttrParamCoerceModeNull);
    assert(!(env.ctx.func->attrs & AttrReference));
    auto const resCls = env.index.builtin_class(env.ctx.cls->name);
    // Can still return null if parameter coercion fails
    return doRet(env, union_of(objExact(resCls), TInitNull));
  }

  if (env.ctx.func->nativeInfo) {
    return doRet(env, native_function_return_type(env.ctx.func));
  }
  doRet(env, TInitGen);
}

void in(ISS& env, const bc::CGetL& op) {
  LocalId equivLocal = NoLocalId;
  // If the local could be Uninit or a Ref, don't record equality because the
  // value on the stack won't the same as in the local.
  if (!locCouldBeUninit(env, op.loc1)) {
    nothrow(env);
    constprop(env);
    if (!locCouldBeRef(env, op.loc1) &&
        !is_volatile_local(env.ctx.func, op.loc1)) {
      equivLocal = op.loc1;
    }
  }
  push(env, locAsCell(env, op.loc1), equivLocal);
}

void in(ISS& env, const bc::CGetQuietL& op) {
  nothrow(env);
  constprop(env);
  push(env, locAsCell(env, op.loc1));
}

void in(ISS& env, const bc::CUGetL& op) {
  auto ty = locRaw(env, op.loc1);
  if (ty.subtypeOf(TUninit)) {
    return reduce(env, bc::NullUninit {});
  }
  nothrow(env);
  if (!ty.couldBe(TUninit)) constprop(env);
  if (!ty.subtypeOf(TCell)) ty = TCell;
  push(env, std::move(ty));
}

void in(ISS& env, const bc::PushL& op) {
  impl(env, bc::CGetL { op.loc1 }, bc::UnsetL { op.loc1 });
}

void in(ISS& env, const bc::CGetL2& op) {
  // Can't constprop yet because of no INS_1 support in bc.h
  if (!locCouldBeUninit(env, op.loc1)) nothrow(env);
  auto loc = locAsCell(env, op.loc1);
  auto top = popT(env);
  push(env, std::move(loc));
  push(env, std::move(top));
}

namespace {

template <typename Op> void common_cgetn(ISS& env) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    auto const loc = findLocal(env, v1->m_data.pstr);
    if (loc != NoLocalId) {
      return reduce(env, bc::PopC {}, Op { loc });
    }
  }
  readUnknownLocals(env);
  mayUseVV(env);
  popC(env); // conversion to string can throw
  push(env, TInitCell);
}

}

void in(ISS& env, const bc::CGetN&) { common_cgetn<bc::CGetL>(env); }
void in(ISS& env, const bc::CGetQuietN&) { common_cgetn<bc::CGetQuietL>(env); }

void in(ISS& env, const bc::CGetG&) { popC(env); push(env, TInitCell); }
void in(ISS& env, const bc::CGetQuietG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::CGetS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (vname && vname->m_type == KindOfPersistentString &&
      self && tcls.subtypeOf(*self)) {
    if (auto ty = selfPropAsCell(env, vname->m_data.pstr)) {
      // Only nothrow when we know it's a private declared property
      // (and thus accessible here).
      nothrow(env);

      // We can only constprop here if we know for sure this is exactly the
      // correct class.  The reason for this is that you could have a LSB class
      // attempting to access a private static in a derived class with the same
      // name as a private static in this class, which is supposed to fatal at
      // runtime (for an example see test/quick/static_sprop2.php).
      auto const selfExact = selfClsExact(env);
      if (selfExact && tcls.subtypeOf(*selfExact)) {
        constprop(env);
      }

      return push(env, std::move(*ty));
    }
  }

  auto indexTy = env.index.lookup_public_static(tcls, tname);
  if (indexTy.subtypeOf(TInitCell)) {
    /*
     * Constant propagation here can change when we invoke autoload, so it's
     * considered HardConstProp.  It's safe not to check anything about private
     * or protected static properties, because you can't override a public
     * static property with a private or protected one---if the index gave us
     * back a constant type, it's because it found a public static and it must
     * be the property this would have read dynamically.
     */
    if (options.HardConstProp) constprop(env);
    return push(env, std::move(indexTy));
  }

  push(env, TInitCell);
}

void in(ISS& env, const bc::VGetL& op) {
  nothrow(env);
  setLocRaw(env, op.loc1, TRef);
  push(env, TRef);
}

void in(ISS& env, const bc::VGetN&) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    auto const loc = findLocal(env, v1->m_data.pstr);
    if (loc != NoLocalId) {
      return reduce(env, bc::PopC {},
                         bc::VGetL { loc });
    }
  }
  modifyLocalStatic(env, NoLocalId, TRef);
  popC(env);
  boxUnknownLocal(env);
  mayUseVV(env);
  push(env, TRef);
}

void in(ISS& env, const bc::VGetG&) { popC(env); push(env, TRef); }

void in(ISS& env, const bc::VGetS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      boxSelfProp(env, vname->m_data.pstr);
    } else {
      killSelfProps(env);
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, TRef);
  }

  push(env, TRef);
}

void clsRefGetImpl(ISS& env, Type t1, ClsRefSlotId slot) {
  auto cls = [&]{
    if (t1.subtypeOf(TObj)) {
      nothrow(env);
      return objcls(t1);
    }
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfPersistentString) {
      if (auto const rcls = env.index.resolve_class(env.ctx, v1->m_data.pstr)) {
        return clsExact(*rcls);
      }
    }
    return TCls;
  }();
  putClsRefSlot(env, slot, std::move(cls));
}

void in(ISS& env, const bc::ClsRefGetL& op) {
  clsRefGetImpl(env, locAsCell(env, op.loc1), op.slot);
}
void in(ISS& env, const bc::ClsRefGetC& op) {
  clsRefGetImpl(env, popC(env), op.slot);
}

void in(ISS& env, const bc::AKExists& /*op*/) {
  auto const t1   = popC(env);
  auto const t2   = popC(env);

  auto const mayThrow = [&]{
    if (!t1.subtypeOfAny(TObj, TArr, TVec, TDict, TKeyset)) return true;
    if (t2.subtypeOfAny(TStr, TNull)) {
      return t1.subtypeOfAny(TObj, TArr) &&
        RuntimeOption::EvalHackArrCompatNotices;
    }
    if (t2.subtypeOf(TInt)) return false;
    return true;
  }();

  if (!mayThrow) nothrow(env);
  push(env, TBool);
}

void in(ISS& env, const bc::GetMemoKeyL& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);

  auto const tyIMemoizeParam =
    subObj(env.index.builtin_class(s_IMemoizeParam.get()));

  auto const inTy = locAsCell(env, op.loc1);

  // If the local could be uninit, we might raise a warning (as
  // usual). Converting an object to a memo key might invoke PHP code if it has
  // the IMemoizeParam interface, and if it doesn't, we'll throw.
  if (!locCouldBeUninit(env, op.loc1) && !inTy.couldBe(TObj)) {
    nothrow(env); constprop(env);
  }

  // If type constraints are being enforced and the local being turned into a
  // memo key is a parameter, then we can possibly using the type constraint to
  // perform a more efficient memoization scheme. Note that this all needs to
  // stay in sync with the interpreter and JIT.
  using MK = MemoKeyConstraint;
  auto const mkc = [&] {
    if (!options.HardTypeHints) return MK::None;
    if (op.loc1 >= env.ctx.func->params.size()) return MK::None;
    auto tc = env.ctx.func->params[op.loc1].typeConstraint;
    if (tc.type() == AnnotType::Object) {
      auto res = env.index.resolve_type_name(tc.typeName());
      if (res.type != AnnotType::Object) {
        tc.resolveType(res.type, res.nullable || tc.isNullable());
      }
    }
    return memoKeyConstraintFromTC(tc);
  }();

  switch (mkc) {
    case MK::Null:
      // Always null, so the key can always just be 0
      always_assert(inTy.subtypeOf(TNull));
      return push(env, ival(0));
    case MK::Int:
      // Always an int, so the key is always an identity mapping
      always_assert(inTy.subtypeOf(TInt));
      return reduce(env, bc::CGetL { op.loc1 });
    case MK::Bool:
      // Always a bool, so the key is the bool cast to an int
      always_assert(inTy.subtypeOf(TBool));
      return reduce(env, bc::CGetL { op.loc1 }, bc::CastInt {});
    case MK::Str:
      // Always a string, so the key is always an identity mapping
      always_assert(inTy.subtypeOf(TStr));
      return reduce(env, bc::CGetL { op.loc1 });
    case MK::IntOrStr:
      // Either an int or string, so the key can be an identity mapping
      return reduce(env, bc::CGetL { op.loc1 });
    case MK::StrOrNull:
    case MK::IntOrNull:
      // A nullable string or int. For strings the key will always be 0 or the
      // string. For ints the key will be the int or a static string. We can't
      // reduce either without introducing control flow.
      return push(env, union_of(TInt, TStr));
    case MK::BoolOrNull:
      // A nullable bool. The key will always be an int (null will be 2), but we
      // can't reduce that without introducing control flow.
      return push(env, TInt);
    case MK::None:
      break;
  }

  // No type constraint, or one that isn't usuable. Use the generic memoization
  // scheme which can handle any type:

  // Integer keys are always mapped to themselves
  if (inTy.subtypeOf(TInt)) return reduce(env, bc::CGetL { op.loc1 });

  if (inTy.subtypeOf(tyIMemoizeParam)) {
    return reduce(
      env,
      bc::CGetL { op.loc1 },
      bc::FPushObjMethodD {
        0,
        s_getInstanceKey.get(),
        ObjMethodOp::NullThrows,
        false
      },
      bc::FCall { 0 },
      bc::UnboxR {}
    );
  }

  // A memo key can be an integer if the input might be an integer, and is a
  // string otherwise. Booleans are always static strings.
  auto keyTy = [&]{
    if (auto const val = tv(inTy)) {
      auto const key = eval_cell(
        [&]{ return HHVM_FN(serialize_memoize_param)(*val); }
      );
      if (key) return *key;
    }
    if (inTy.subtypeOf(TBool)) return TSStr;
    if (inTy.couldBe(TInt)) return union_of(TInt, TStr);
    return TStr;
  }();
  push(env, std::move(keyTy));
}

void in(ISS& env, const bc::IssetL& op) {
  nothrow(env);
  constprop(env);
  auto const loc = locAsCell(env, op.loc1);
  if (loc.subtypeOf(TNull))  return push(env, TFalse);
  if (!loc.couldBe(TNull))   return push(env, TTrue);
  push(env, TBool);
}

void in(ISS& env, const bc::EmptyL& op) {
  nothrow(env);
  constprop(env);
  if (!locCouldBeUninit(env, op.loc1)) {
    return impl(env, bc::CGetL { op.loc1 }, bc::Not {});
  }
  locAsCell(env, op.loc1); // read the local
  push(env, TBool);
}

void in(ISS& env, const bc::EmptyS& op) {
  takeClsRefSlot(env, op.slot);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::IssetS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (self && tcls.subtypeOf(*self) &&
      vname && vname->m_type == KindOfPersistentString) {
    if (auto const t = selfPropAsCell(env, vname->m_data.pstr)) {
      if (t->subtypeOf(TNull))  { constprop(env); return push(env, TFalse); }
      if (!t->couldBe(TNull))   { constprop(env); return push(env, TTrue); }
    }
  }

  auto const indexTy = env.index.lookup_public_static(tcls, tname);
  if (indexTy.subtypeOf(TInitCell)) {
    // See the comments in CGetS about constprop for public statics.
    if (options.HardConstProp) constprop(env);
    if (indexTy.subtypeOf(TNull))  { return push(env, TFalse); }
    if (!indexTy.couldBe(TNull))   { return push(env, TTrue); }
  }

  push(env, TBool);
}

template<class ReduceOp>
void issetEmptyNImpl(ISS& env) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    auto const loc = findLocal(env, v1->m_data.pstr);
    if (loc != NoLocalId) {
      return reduce(env, bc::PopC {}, ReduceOp { loc });
    }
    // Can't push true in the non env.findLocal case unless we know
    // whether this function can have a VarEnv.
  }
  readUnknownLocals(env);
  mayUseVV(env);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::IssetN&) { issetEmptyNImpl<bc::IssetL>(env); }
void in(ISS& env, const bc::EmptyN&) { issetEmptyNImpl<bc::EmptyL>(env); }
void in(ISS& env, const bc::EmptyG&) { popC(env); push(env, TBool); }
void in(ISS& env, const bc::IssetG&) { popC(env); push(env, TBool); }

void isTypeImpl(ISS& env, const Type& locOrCell, const Type& test) {
  constprop(env);
  if (locOrCell.subtypeOf(test))  return push(env, TTrue);
  if (!locOrCell.couldBe(test))   return push(env, TFalse);
  push(env, TBool);
}

void isTypeObj(ISS& env, const Type& ty) {
  if (!ty.couldBe(TObj)) return push(env, TFalse);
  if (ty.subtypeOf(TObj)) {
    auto const incompl = objExact(
      env.index.builtin_class(s_PHP_Incomplete_Class.get()));
    if (!ty.couldBe(incompl))  return push(env, TTrue);
    if (ty.subtypeOf(incompl)) return push(env, TFalse);
  }
  push(env, TBool);
}

template<class Op>
void isTypeLImpl(ISS& env, const Op& op) {
  if (!locCouldBeUninit(env, op.loc1)) { nothrow(env); constprop(env); }
  auto const loc = locAsCell(env, op.loc1);
  switch (op.subop2) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, loc);
  default: return isTypeImpl(env, loc, type_of_istype(op.subop2));
  }
}

template<class Op>
void isTypeCImpl(ISS& env, const Op& op) {
  nothrow(env);
  auto const t1 = popC(env);
  switch (op.subop1) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, t1);
  default: return isTypeImpl(env, t1, type_of_istype(op.subop1));
  }
}

void in(ISS& env, const bc::IsTypeC& op) { isTypeCImpl(env, op); }
void in(ISS& env, const bc::IsTypeL& op) { isTypeLImpl(env, op); }

void in(ISS& env, const bc::IsUninit& /*op*/) {
  nothrow(env);
  push(env, popCU(env));
  isTypeImpl(env, topT(env), TUninit);
}

void in(ISS& env, const bc::MaybeMemoType& /*op*/) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  nothrow(env);
  constprop(env);
  auto const memoTy = memoizeImplRetType(env);
  auto const ty = popC(env);
  push(env, ty.couldBe(memoTy) ? TTrue : TFalse);
}

void in(ISS& env, const bc::IsMemoType& /*op*/) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  nothrow(env);
  constprop(env);
  auto const memoTy = memoizeImplRetType(env);
  auto const ty = popC(env);
  push(env, memoTy.subtypeOf(ty) ? TTrue : TFalse);
}

void in(ISS& env, const bc::InstanceOfD& op) {
  auto const t1 = popC(env);
  // Note: InstanceOfD can do autoload if the type might be a type
  // alias, so it's not nothrow unless we know it's an object type.
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str1)) {
    nothrow(env);
    if (!interface_supports_non_objects(rcls->name())) {
      isTypeImpl(env, t1, subObj(*rcls));
      return;
    }
  }
  push(env, TBool);
}

void in(ISS& env, const bc::InstanceOf& /*op*/) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    return reduce(env, bc::PopC {},
                       bc::InstanceOfD { v1->m_data.pstr });
  }

  if (t1.subtypeOf(TObj) && is_specialized_obj(t1)) {
    auto const dobj = dobj_of(t1);
    switch (dobj.type) {
    case DObj::Sub:
      break;
    case DObj::Exact:
      return reduce(env, bc::PopC {},
                         bc::InstanceOfD { dobj.cls.name() });
    }
  }

  popC(env);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::SetL& op) {
  nothrow(env);
  auto equivLoc = topStkEquiv(env);
  // If the local could be a Ref, don't record equality because the stack
  // element and the local won't actually have the same type.
  if (!locCouldBeRef(env, op.loc1) &&
      !is_volatile_local(env.ctx.func, op.loc1)) {
    if (equivLoc != NoLocalId) {
      if (equivLoc == op.loc1 ||
          locsAreEquiv(env, equivLoc, op.loc1)) {
        return reduce(env, bc::Nop {});
      }
    } else {
      equivLoc = op.loc1;
    }
  }
  auto val = popC(env);
  setLoc(env, op.loc1, val);
  if (equivLoc != op.loc1 && equivLoc != NoLocalId) {
    addLocEquiv(env, op.loc1, equivLoc);
  }
  push(env, std::move(val), equivLoc);
}

void in(ISS& env, const bc::SetN&) {
  // This isn't trivial to strength reduce, without a "flip two top
  // elements of stack" opcode.
  auto t1 = popC(env);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);
  // TODO(#3653110): could nothrow if t2 can't be an Obj or Res

  auto const knownLoc = v2 && v2->m_type == KindOfPersistentString
    ? findLocal(env, v2->m_data.pstr)
    : NoLocalId;
  if (knownLoc != NoLocalId) {
    setLoc(env, knownLoc, t1);
  } else {
    // We could be changing the value of any local, but we won't
    // change whether or not they are boxed or initialized.
    loseNonRefLocalTypes(env);
  }
  mayUseVV(env);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::SetG&) {
  auto t1 = popC(env);
  popC(env);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::SetS& op) {
  auto const t1    = popC(env);
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      nothrow(env);
      mergeSelfProp(env, vname->m_data.pstr, t1);
    } else {
      mergeEachSelfPropRaw(env, [&] (Type) { return t1; });
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, t1);
  }

  push(env, std::move(t1));
}

void in(ISS& env, const bc::SetOpL& op) {
  auto const t1     = popC(env);
  auto const v1     = tv(t1);
  auto const loc    = locAsCell(env, op.loc1);
  auto const locVal = tv(loc);
  if (v1 && locVal) {
    // Can't constprop at this eval_cell, because of the effects on
    // locals.
    auto resultTy = eval_cell([&] {
      Cell c = *locVal;
      Cell rhs = *v1;
      setopBody(&c, op.subop2, &rhs);
      return c;
    });
    if (!resultTy) resultTy = TInitCell;

    // We may have inferred a TSStr or TSArr with a value here, but
    // at runtime it will not be static.  For now just throw that
    // away.  TODO(#3696042): should be able to loosen_staticness here.
    if (resultTy->subtypeOf(TStr)) resultTy = TStr;
    else if (resultTy->subtypeOf(TArr)) resultTy = TArr;
    else if (resultTy->subtypeOf(TVec)) resultTy = TVec;
    else if (resultTy->subtypeOf(TDict)) resultTy = TDict;
    else if (resultTy->subtypeOf(TKeyset)) resultTy = TKeyset;

    setLoc(env, op.loc1, *resultTy);
    push(env, *resultTy);
    return;
  }

  auto resultTy = typeSetOp(op.subop2, loc, t1);
  setLoc(env, op.loc1, resultTy);
  push(env, std::move(resultTy));
}

void in(ISS& env, const bc::SetOpN&) {
  popC(env);
  popC(env);
  loseNonRefLocalTypes(env);
  mayUseVV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::SetOpG&) {
  popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::SetOpS& op) {
  popC(env);
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      mergeSelfProp(env, vname->m_data.pstr, TInitCell);
    } else {
      loseNonRefSelfPropTypes(env);
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, TInitCell);
  }

  push(env, TInitCell);
}

void in(ISS& env, const bc::IncDecL& op) {
  auto loc = locAsCell(env, op.loc1);
  auto newT = typeIncDec(op.subop2, loc);
  auto const pre = isPre(op.subop2);

  // If it's a non-numeric string, this may cause it to exceed the max length.
  if (!locCouldBeUninit(env, op.loc1) &&
      !loc.couldBe(TStr)) {
    nothrow(env);
  }

  if (!pre) push(env, std::move(loc));
  setLoc(env, op.loc1, newT);
  if (pre)  push(env, std::move(newT));
}

void in(ISS& env, const bc::IncDecN& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  auto const knownLoc = v1 && v1->m_type == KindOfPersistentString
    ? findLocal(env, v1->m_data.pstr)
    : NoLocalId;
  if (knownLoc != NoLocalId) {
    return reduce(env, bc::PopC {},
                       bc::IncDecL { knownLoc, op.subop1 });
  }
  popC(env);
  loseNonRefLocalTypes(env);
  mayUseVV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::IncDecG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::IncDecS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      mergeSelfProp(env, vname->m_data.pstr, TInitCell);
    } else {
      loseNonRefSelfPropTypes(env);
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, TInitCell);
  }

  push(env, TInitCell);
}

void in(ISS& env, const bc::BindL& op) {
  // If the op.loc1 was bound to a local static, its going to be
  // unbound from it. If the thing its being bound /to/ is a local
  // static, we've already marked it as modified via the VGetL, so
  // there's nothing more to track.
  // Unbind it before any updates.
  modifyLocalStatic(env, op.loc1, TUninit);
  nothrow(env);
  auto t1 = popV(env);
  setLocRaw(env, op.loc1, t1);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::BindN&) {
  // TODO(#3653110): could nothrow if t2 can't be an Obj or Res
  auto t1 = popV(env);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);
  auto const knownLoc = v2 && v2->m_type == KindOfPersistentString
    ? findLocal(env, v2->m_data.pstr)
    : NoLocalId;
  unbindLocalStatic(env, knownLoc);
  if (knownLoc != NoLocalId) {
    setLocRaw(env, knownLoc, t1);
  } else {
    boxUnknownLocal(env);
  }
  mayUseVV(env);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::BindG&) {
  auto t1 = popV(env);
  popC(env);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::BindS& op) {
  popV(env);
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      boxSelfProp(env, vname->m_data.pstr);
    } else {
      killSelfProps(env);
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, TRef);
  }

  push(env, TRef);
}

void in(ISS& env, const bc::UnsetL& op) {
  nothrow(env);
  setLocRaw(env, op.loc1, TUninit);
}

void in(ISS& env, const bc::UnsetN& /*op*/) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    auto const loc = findLocal(env, v1->m_data.pstr);
    if (loc != NoLocalId) {
      return reduce(env, bc::PopC {},
                         bc::UnsetL { loc });
    }
  }
  popC(env);
  if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow(env);
  unsetUnknownLocal(env);
  mayUseVV(env);
}

void in(ISS& env, const bc::UnsetG& /*op*/) {
  auto const t1 = popC(env);
  if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow(env);
}

void in(ISS& env, const bc::FPushFuncD& op) {
  auto const rfunc = env.index.resolve_func(env.ctx, op.str2);
  if (auto const func = rfunc.exactFunc()) {
    if (can_emit_builtin(func, op.arg1, op.has_unpack)) {
      fpiPush(env, ActRec { FPIKind::Builtin, folly::none, rfunc });
      return reduce(env, bc::Nop {});
    }
  }
  fpiPush(env, ActRec { FPIKind::Func, folly::none, rfunc });
}

void in(ISS& env, const bc::FPushFunc& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    auto const name = normalizeNS(v1->m_data.pstr);
    // FPushFuncD doesn't support class-method pair strings yet.
    if (isNSNormalized(name) && notClassMethodPair(name)) {
      auto const rfunc = env.index.resolve_func(env.ctx, name);
      // Don't turn dynamic calls to caller frame affecting functions into
      // static calls, because they might fatal (whereas the static one won't).
      if (!rfunc.mightAccessCallerFrame()) {
        return reduce(env, bc::PopC {},
                      bc::FPushFuncD { op.arg1, name, op.has_unpack });
      }
    }
  }
  popC(env);
  if (t1.subtypeOf(TObj)) return fpiPush(env, ActRec { FPIKind::ObjInvoke });
  if (t1.subtypeOf(TArr)) return fpiPush(env, ActRec { FPIKind::CallableArr });
  if (t1.subtypeOf(TStr)) return fpiPush(env, ActRec { FPIKind::Func });
  fpiPush(env, ActRec { FPIKind::Unknown });
}

void in(ISS& env, const bc::FPushFuncU& op) {
  auto const rfuncPair =
    env.index.resolve_func_fallback(env.ctx, op.str2, op.str3);
  if (options.ElideAutoloadInvokes && !rfuncPair.second) {
    return reduce(
      env,
      bc::FPushFuncD { op.arg1, rfuncPair.first.name(), op.has_unpack }
    );
  }
  fpiPush(
    env,
    ActRec { FPIKind::Func, folly::none, rfuncPair.first, rfuncPair.second }
  );
}

void in(ISS& env, const bc::FPushObjMethodD& op) {
  auto t1 = popC(env);
  if (is_opt(t1) && op.subop3 == ObjMethodOp::NullThrows) {
    t1 = unopt(t1);
  }
  auto const clsTy = objcls(t1);
  auto const rcls = [&]() -> folly::Optional<res::Class> {
    if (is_specialized_cls(clsTy)) return dcls_of(clsTy).cls;
    return folly::none;
  }();

  fpiPush(env, ActRec {
    FPIKind::ObjMeth,
    rcls,
    env.index.resolve_method(env.ctx, clsTy, op.str2)
  });
}

void in(ISS& env, const bc::FPushObjMethod& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    return reduce(
      env,
      bc::PopC {},
      bc::FPushObjMethodD { op.arg1, v1->m_data.pstr, op.subop2, op.has_unpack }
    );
  }
  popC(env);
  popC(env);
  fpiPush(env, ActRec { FPIKind::ObjMeth });
}

void in(ISS& env, const bc::FPushClsMethodD& op) {
  auto const rcls = env.index.resolve_class(env.ctx, op.str3);
  auto const rfun = env.index.resolve_method(
    env.ctx,
    rcls ? clsExact(*rcls) : TCls,
    op.str2
  );
  fpiPush(env, ActRec { FPIKind::ClsMeth, rcls, rfun });
}

void in(ISS& env, const bc::FPushClsMethod& op) {
  auto const t1 = takeClsRefSlot(env, op.slot);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);

  folly::Optional<res::Func> rfunc;
  if (v2 && v2->m_type == KindOfPersistentString) {
    rfunc = env.index.resolve_method(env.ctx, t1, v2->m_data.pstr);
  }
  folly::Optional<res::Class> rcls;
  if (is_specialized_cls(t1)) rcls = dcls_of(t1).cls;
  fpiPush(env, ActRec { FPIKind::ClsMeth, rcls, rfunc });
}

void in(ISS& env, const bc::FPushClsMethodF& op) {
  // The difference with FPushClsMethod is what ends up on the
  // ActRec (late-bound class), which we currently aren't tracking.
  impl(env, bc::FPushClsMethod { op.arg1, op.slot, op.has_unpack });
}

void ctorHelper(ISS& env, SString name) {
  auto const rcls = env.index.resolve_class(env.ctx, name);
  push(env, rcls ? objExact(*rcls) : TObj);
  auto const rfunc =
    rcls ? env.index.resolve_ctor(env.ctx, *rcls) : folly::none;
  fpiPush(env, ActRec { FPIKind::Ctor, rcls, rfunc });
}

void in(ISS& env, const bc::FPushCtorD& op) {
  ctorHelper(env, op.str2);
}

void in(ISS& env, const bc::FPushCtorI& op) {
  auto const name = env.ctx.unit->classes[op.arg2]->name;
  ctorHelper(env, name);
}

void in(ISS& env, const bc::FPushCtor& op) {
  auto const& t1 = peekClsRefSlot(env, op.slot);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    if (dcls.type == DCls::Exact) {
      return reduce(env, bc::DiscardClsRef { op.slot },
                    bc::FPushCtorD { op.arg1, dcls.cls.name(), op.has_unpack });
    }
  }
  takeClsRefSlot(env, op.slot);
  push(env, TObj);
  fpiPush(env, ActRec { FPIKind::Ctor });
}

void in(ISS& env, const bc::FPushCufIter&) {
  nothrow(env);
  fpiPush(env, ActRec { FPIKind::Unknown });
}

void in(ISS& env, const bc::FPushCuf&) {
  popC(env);
  fpiPush(env, ActRec { FPIKind::Unknown });
}
void in(ISS& env, const bc::FPushCufF&) {
  popC(env);
  fpiPush(env, ActRec { FPIKind::Unknown });
}

void in(ISS& env, const bc::FPushCufSafe&) {
  auto t1 = popC(env);
  popC(env);
  push(env, std::move(t1));
  fpiPush(env, ActRec { FPIKind::Unknown });
  push(env, TBool);
}

void in(ISS& env, const bc::FPassL& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    if (!locCouldBeUninit(env, op.loc2)) nothrow(env);
    // This might box the local, we can't tell.  Note: if the local
    // is already TRef, we could try to leave it alone, but not for
    // now.
    setLocRaw(env, op.loc2, TGen);
    return push(env, TInitGen);
  case PrepKind::Val:
    return reduce_fpass_arg(env, bc::CGetL { op.loc2 }, op.arg1, false);
  case PrepKind::Ref:
    return reduce_fpass_arg(env, bc::VGetL { op.loc2 }, op.arg1, true);
  }
}

void in(ISS& env, const bc::FPassN& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    // This could change the type of any local.
    popC(env);
    killLocals(env);
    mayUseVV(env);
    return push(env, TInitGen);
  case PrepKind::Val: return reduce_fpass_arg(env,
                                              bc::CGetN {},
                                              op.arg1,
                                              false);
  case PrepKind::Ref: return reduce_fpass_arg(env,
                                              bc::VGetN {},
                                              op.arg1,
                                              true);
  }
}

void in(ISS& env, const bc::FPassG& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown: popC(env); return push(env, TInitGen);
  case PrepKind::Val:     return reduce_fpass_arg(env,
                                                  bc::CGetG {},
                                                  op.arg1,
                                                  false);
  case PrepKind::Ref:     return reduce_fpass_arg(env,
                                                  bc::VGetG {},
                                                  op.arg1,
                                                  true);
  }
}

void in(ISS& env, const bc::FPassS& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    {
      auto tcls        = takeClsRefSlot(env, op.slot);
      auto const self  = selfCls(env);
      auto const tname = popC(env);
      auto const vname = tv(tname);
      if (!self || tcls.couldBe(*self)) {
        if (vname && vname->m_type == KindOfPersistentString) {
          // May or may not be boxing it, depending on the refiness.
          mergeSelfProp(env, vname->m_data.pstr, TInitGen);
        } else {
          killSelfProps(env);
        }
      }
      if (auto c = env.collect.publicStatics) {
        c->merge(env.ctx, std::move(tcls), tname, TInitGen);
      }
    }
    return push(env, TInitGen);
  case PrepKind::Val:
    return reduce_fpass_arg(env, bc::CGetS { op.slot }, op.arg1, false);
  case PrepKind::Ref:
    return reduce_fpass_arg(env, bc::VGetS { op.slot }, op.arg1, true);
  }
}

void in(ISS& env, const bc::FPassV& op) {
  nothrow(env);
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    popV(env);
    return push(env, TInitGen);
  case PrepKind::Val:
    return reduce_fpass_arg(env, bc::Unbox {}, op.arg1, false);
  case PrepKind::Ref:
    return reduce_fpass_arg(env, bc::Nop {}, op.arg1, true);
  }
}

void in(ISS& env, const bc::FPassR& op) {
  nothrow(env);
  if (fpiTop(env).kind == FPIKind::Builtin) {
    switch (prepKind(env, op.arg1)) {
    case PrepKind::Unknown:
      not_reached();
    case PrepKind::Val:
      return reduce(env, bc::UnboxR {});
    case PrepKind::Ref:
      return reduce(env, bc::BoxR {});
    }
  }

  auto const t1 = topT(env);
  if (t1.subtypeOf(TCell)) {
    return reduce_fpass_arg(env, bc::UnboxRNop {}, op.arg1, false);
  }

  // If it's known to be a ref, this behaves like FPassV, except we need to do
  // it slightly differently to keep stack flavors correct.
  if (t1.subtypeOf(TRef)) {
    switch (prepKind(env, op.arg1)) {
    case PrepKind::Unknown:
      popV(env);
      return push(env, TInitGen);
    case PrepKind::Val:
      return reduce_fpass_arg(env, bc::UnboxR {}, op.arg1, false);
    case PrepKind::Ref:
      return reduce_fpass_arg(env, bc::BoxRNop {}, op.arg1, true);
    }
    not_reached();
  }

  // Here we don't know if it is going to be a cell or a ref.
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:      popR(env); return push(env, TInitGen);
  case PrepKind::Val:          popR(env); return push(env, TInitCell);
  case PrepKind::Ref:          popR(env); return push(env, TRef);
  }
}

void in(ISS& env, const bc::FPassVNop&) {
  push(env, popV(env));
  if (fpiTop(env).kind == FPIKind::Builtin) {
    return reduce(env, bc::Nop {});
  }
  nothrow(env);
}

void in(ISS& env, const bc::FPassC& /*op*/) {
  if (fpiTop(env).kind == FPIKind::Builtin) {
    return reduce(env, bc::Nop {});
  }
  nothrow(env);
}

void fpassCXHelper(ISS& env, uint32_t param, bool error) {
  auto const& fpi = fpiTop(env);
  if (fpi.kind == FPIKind::Builtin) {
    switch (prepKind(env, param)) {
      case PrepKind::Unknown:
        not_reached();
      case PrepKind::Ref:
      {
        auto const& params = fpi.func->exactFunc()->params;
        if (param >= params.size() || params[param].mustBeRef) {
          if (error) {
            return reduce(env,
                          bc::String { s_byRefError.get() },
                          bc::Fatal { FatalOp::Runtime });
          } else {
            return reduce(env,
                          bc::String { s_byRefWarn.get() },
                          bc::Int { (int)ErrorMode::STRICT },
                          bc::FCallBuiltin { 2, 2, s_trigger_error.get() },
                          bc::PopC {});
          }
        }
        // fall through
      }
      case PrepKind::Val:
        return reduce(env, bc::Nop {});
    }
    not_reached();
  }
  switch (prepKind(env, param)) {
    case PrepKind::Unknown: return;
    case PrepKind::Val:     return reduce(env, bc::FPassC { param });
    case PrepKind::Ref:     /* will warn/fatal at runtime */ return;
  }
}

void in(ISS& env, const bc::FPassCW& op) {
  fpassCXHelper(env, op.arg1, false);
}

void in(ISS& env, const bc::FPassCE& op) {
  fpassCXHelper(env, op.arg1, true);
}

void pushCallReturnType(ISS& env, Type&& ty) {
  if (ty == TBottom) {
    // The callee function never returns.  It might throw, or loop forever.
    unreachable(env);
  }
  return push(env, std::move(ty));
}

const StaticString s_defined { "defined" };
const StaticString s_function_exists { "function_exists" };

void fcallKnownImpl(ISS& env, uint32_t numArgs) {
  auto const ar = fpiPop(env);
  always_assert(ar.func.hasValue());
  specialFunctionEffects(env, ar);

  if (ar.func->name()->isame(s_function_exists.get())) {
    handle_function_exists(env, numArgs, false);
  }

  std::vector<Type> args(numArgs);
  for (auto i = uint32_t{0}; i < numArgs; ++i) {
    args[numArgs - i - 1] = popF(env);
  }

  if (options.HardConstProp &&
      numArgs == 1 &&
      ar.func->name()->isame(s_defined.get())) {
    // If someone calls defined('foo') they probably want foo to be
    // defined normally; ie not a persistent constant.
    if (auto const v = tv(args[0])) {
      if (isStringType(v->m_type) &&
          !env.index.lookup_constant(env.ctx, v->m_data.pstr)) {
        env.collect.cnsMap[v->m_data.pstr].m_type = kDynamicConstant;
      }
    }
  }

  auto ty = env.index.lookup_return_type(
    CallContext { env.ctx, args },
    *ar.func
  );
  if (!ar.fallbackFunc) {
    pushCallReturnType(env, std::move(ty));
    return;
  }
  auto ty2 = env.index.lookup_return_type(
    CallContext { env.ctx, args },
    *ar.fallbackFunc
  );
  pushCallReturnType(env, union_of(std::move(ty), std::move(ty2)));
}

void in(ISS& env, const bc::FCall& op) {
  auto const ar = fpiTop(env);
  if (ar.func && !ar.fallbackFunc) {
    switch (ar.kind) {
    case FPIKind::Unknown:
    case FPIKind::CallableArr:
    case FPIKind::ObjInvoke:
      not_reached();
    case FPIKind::Func:
      // Don't turn dynamic calls into static calls with functions that can
      // potentially touch the caller's frame. Such functions will fatal if
      // called dynamically and we want to preserve that behavior.
      if (!ar.func->mightAccessCallerFrame()) {
        return reduce(
          env,
          bc::FCallD { op.arg1, s_empty.get(), ar.func->name() }
        );
      }
      break;
    case FPIKind::Builtin:
      return finish_builtin(env, ar.func->exactFunc(), op.arg1, false);
    case FPIKind::Ctor:
      /*
       * Need to be wary of old-style ctors. We could get into the situation
       * where we're constructing class D extends B, and B has an old-style
       * ctor but D::B also exists.  (So in this case we'll skip the
       * fcallKnownImpl stuff.)
       */
      if (!ar.func->name()->isame(s_construct.get()) &&
          !ar.func->name()->isame(s_86ctor.get())) {
        break;
      }
      // fallthrough
    case FPIKind::ObjMeth:
    case FPIKind::ClsMeth:
      if (ar.cls.hasValue() && ar.func->cantBeMagicCall()) {
        return reduce(
          env,
          bc::FCallD { op.arg1, ar.cls->name(), ar.func->name() }
        );
      }

      // If we didn't return a reduce above, we still can compute a
      // partially-known FCall effect with our res::Func.
      return fcallKnownImpl(env, op.arg1);
    }
  }

  for (auto i = uint32_t{0}; i < op.arg1; ++i) popF(env);
  fpiPop(env);
  specialFunctionEffects(env, ar);
  push(env, TInitGen);
}

void in(ISS& env, const bc::FCallD& op) {
  auto const ar = fpiTop(env);
  if (ar.kind == FPIKind::Builtin) {
    return finish_builtin(env, ar.func->exactFunc(), op.arg1, false);
  }
  if (ar.func) return fcallKnownImpl(env, op.arg1);
  specialFunctionEffects(env, ar);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popF(env);
  push(env, TInitGen);
}

void in(ISS& env, const bc::FCallAwait& op) {
  in(env, bc::FCallD { op.arg1, op.str2, op.str3 });
  in(env, bc::UnboxRNop { });
  in(env, bc::Await { });

  env.flags.wasPEI = true;
  env.flags.canConstProp = false;
}

void fcallArrayImpl(ISS& env, int arg) {
  auto const ar = fpiTop(env);
  if (ar.kind == FPIKind::Builtin) {
    return finish_builtin(env, ar.func->exactFunc(), arg, true);
  }

  for (auto i = uint32_t{0}; i < arg; ++i) { popF(env); }
  fpiPop(env);
  specialFunctionEffects(env, ar);
  if (ar.func) {
    auto ty = env.index.lookup_return_type(env.ctx, *ar.func);
    if (!ar.fallbackFunc) {
      pushCallReturnType(env, std::move(ty));
      return;
    }
    auto ty2 = env.index.lookup_return_type(env.ctx, *ar.fallbackFunc);
    pushCallReturnType(env, union_of(std::move(ty), std::move(ty2)));
    return;
  }
  return push(env, TInitGen);
}

void in(ISS& env, const bc::FCallArray& /*op*/) {
  fcallArrayImpl(env, 1);
}

void in(ISS& env, const bc::FCallUnpack& op) {
  fcallArrayImpl(env, op.arg1);
}

void in(ISS& env, const bc::CufSafeArray&) {
  popR(env); popC(env); popC(env);
  push(env, TArr);
}

void in(ISS& env, const bc::CufSafeReturn&) {
  popR(env); popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::DecodeCufIter& op) {
  popC(env); // func
  env.propagate(op.target, env.state); // before iter is modifed
}

void in(ISS& env, const bc::IterInit& op) {
  auto const t1 = popC(env);
  // Take the branch before setting locals if the iter is already
  // empty, but after popping.  Similar for the other IterInits
  // below.
  freeIter(env, op.iter1);
  env.propagate(op.target, env.state);
  if (t1.subtypeOfAny(TArrE, TVecE, TDictE, TKeysetE)) {
    nothrow(env);
    jmp_nofallthrough(env);
    return;
  }
  auto ity = iter_types(t1);
  setLoc(env, op.loc3, ity.second);
  setIter(env, op.iter1, TrackedIter { std::move(ity) });
}

void in(ISS& env, const bc::MIterInit& op) {
  popV(env);
  env.propagate(op.target, env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::IterInitK& op) {
  auto const t1 = popC(env);
  freeIter(env, op.iter1);
  env.propagate(op.target, env.state);
  if (t1.subtypeOfAny(TArrE, TVecE, TDictE, TKeysetE)) {
    nothrow(env);
    jmp_nofallthrough(env);
    return;
  }
  auto ity = iter_types(t1);
  setLoc(env, op.loc3, ity.second);
  setLoc(env, op.loc4, ity.first);
  setIter(env, op.iter1, TrackedIter { std::move(ity) });
}

void in(ISS& env, const bc::MIterInitK& op) {
  popV(env);
  env.propagate(op.target, env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterInit& op) {
  popC(env);
  env.propagate(op.target, env.state);
  // WIter* instructions may leave the value locals as either refs
  // or cells, depending whether the rhs of the assignment was a
  // ref.
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterInitK& op) {
  popC(env);
  env.propagate(op.target, env.state);
  setLocRaw(env, op.loc3, TInitGen);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::IterNext& op) {
  auto const curLoc3 = locRaw(env, op.loc3);

  match<void>(
    env.state.iters[op.iter1],
    [&] (UnknownIter)           {
      setLoc(env, op.loc3, TInitCell);
    },
    [&] (const TrackedIter& ti) {
      setLoc(env, op.loc3, ti.kv.second);
    }
  );
  env.propagate(op.target, env.state);

  freeIter(env, op.iter1);
  if (curLoc3.subtypeOf(TInitCell)) setLocRaw(env, op.loc3, curLoc3);
}

void in(ISS& env, const bc::MIterNext& op) {
  env.propagate(op.target, env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::IterNextK& op) {
  auto const curLoc3 = locRaw(env, op.loc3);
  auto const curLoc4 = locRaw(env, op.loc4);

  match<void>(
    env.state.iters[op.iter1],
    [&] (UnknownIter) {
      setLoc(env, op.loc3, TInitCell);
      setLoc(env, op.loc4, TInitCell);
    },
    [&] (const TrackedIter& ti) {
      setLoc(env, op.loc3, ti.kv.second);
      setLoc(env, op.loc4, ti.kv.first);
    }
  );
  env.propagate(op.target, env.state);

  freeIter(env, op.iter1);
  if (curLoc3.subtypeOf(TInitCell)) setLocRaw(env, op.loc3, curLoc3);
  if (curLoc4.subtypeOf(TInitCell)) setLocRaw(env, op.loc4, curLoc4);
}

void in(ISS& env, const bc::MIterNextK& op) {
  env.propagate(op.target, env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterNext& op) {
  env.propagate(op.target, env.state);
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterNextK& op) {
  env.propagate(op.target, env.state);
  setLocRaw(env, op.loc3, TInitGen);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::IterFree& op) {
  nothrow(env);
  freeIter(env, op.iter1);
}
void in(ISS& env, const bc::MIterFree& op) {
  nothrow(env);
  freeIter(env, op.iter1);
}
void in(ISS& env, const bc::CIterFree& op) {
  nothrow(env);
  freeIter(env, op.iter1);
}

void in(ISS& env, const bc::IterBreak& op) {
  for (auto& kv : op.iterTab) freeIter(env, kv.second);
  env.propagate(op.target, env.state);
}

/*
 * Any include/require (or eval) op kills all locals, and private properties.
 *
 * We don't need to do anything for collect.publicStatics because we'll analyze
 * the included pseudo-main separately and see any effects it may have on
 * public statics.
 */
void inclOpImpl(ISS& env) {
  popC(env);
  killLocals(env);
  killThisProps(env);
  killSelfProps(env);
  mayUseVV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::Incl&)      { inclOpImpl(env); }
void in(ISS& env, const bc::InclOnce&)  { inclOpImpl(env); }
void in(ISS& env, const bc::Req&)       { inclOpImpl(env); }
void in(ISS& env, const bc::ReqOnce&)   { inclOpImpl(env); }
void in(ISS& env, const bc::ReqDoc&)    { inclOpImpl(env); }
void in(ISS& env, const bc::Eval&)      { inclOpImpl(env); }

void in(ISS& /*env*/, const bc::DefFunc&) {}
void in(ISS& /*env*/, const bc::DefCls&) {}
void in(ISS& /*env*/, const bc::DefClsNop&) {}
void in(ISS& env, const bc::AliasCls&) {
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::DefCns& op) {
  auto const t = popC(env);
  if (options.HardConstProp) {
    auto const v = tv(t);
    auto const val = v && tvAsCVarRef(&*v).isAllowedAsConstantValue() ?
      *v : make_tv<KindOfUninit>();
    auto const res = env.collect.cnsMap.emplace(op.str1, val);
    if (!res.second) {
      if (res.first->second.m_type == kReadOnlyConstant) {
        // we only saw a read of this constant
        res.first->second = val;
      } else {
        // more than one definition in this function
        res.first->second.m_type = kDynamicConstant;
      }
    }
  }
  push(env, TBool);
}

void in(ISS& /*env*/, const bc::DefTypeAlias&) {}

void in(ISS& env, const bc::This&) {
  if (thisAvailable(env)) {
    return reduce(env, bc::BareThis { BareThisOp::NeverNull });
  }
  auto const ty = thisType(env);
  push(env, ty ? *ty : TObj);
  setThisAvailable(env);
}

void in(ISS& env, const bc::LateBoundCls& op) {
  auto const ty = selfCls(env);
  putClsRefSlot(env, op.slot, ty ? *ty : TCls);
}

void in(ISS& env, const bc::CheckThis&) {
  if (thisAvailable(env)) {
    reduce(env, bc::Nop {});
  }
  setThisAvailable(env);
}

void in(ISS& env, const bc::BareThis& op) {
  if (thisAvailable(env)) {
    if (op.subop1 != BareThisOp::NeverNull) {
      return reduce(env, bc::BareThis { BareThisOp::NeverNull });
    }
  }

  auto const ty = thisType(env);
  switch (op.subop1) {
  case BareThisOp::Notice:
    break;
  case BareThisOp::NoNotice:
    nothrow(env);
    break;
  case BareThisOp::NeverNull:
    nothrow(env);
    setThisAvailable(env);
    return push(env, ty ? *ty : TObj);
  }

  push(env, ty ? opt(*ty) : TOptObj);
}

void in(ISS& env, const bc::InitThisLoc& op) {
  setLocRaw(env, op.loc1, TCell);
}

void in(ISS& env, const bc::StaticLocDef& op) {
  if (staticLocHelper(env, op.loc1, topC(env))) {
    return reduce(env, bc::SetL { op.loc1 }, bc::PopC {});
  }
  popC(env);
}

void in(ISS& env, const bc::StaticLocCheck& op) {
  auto const l = op.loc1;
  setLocRaw(env, l, TGen);
  maybeBindLocalStatic(env, l);
  if (!env.ctx.func->isMemoizeWrapper &&
      !env.ctx.func->isClosureBody &&
      env.collect.localStaticTypes.size() > l) {
    auto t = env.collect.localStaticTypes[l];
    if (auto v = tv(t)) {
      useLocalStatic(env, l);
      setLocRaw(env, l, t);
      return reduce(env,
                    gen_constant(*v),
                    bc::SetL { op.loc1 }, bc::PopC {},
                    bc::True {});
    }
  }
  push(env, TBool);
}

void in(ISS& env, const bc::StaticLocInit& op) {
  if (staticLocHelper(env, op.loc1, topC(env))) {
    return reduce(env, bc::SetL { op.loc1 }, bc::PopC {});
  }
  popC(env);
}

/*
 * Amongst other things, we use this to mark units non-persistent.
 */
void in(ISS& env, const bc::OODeclExists& op) {
  auto flag = popC(env);
  auto name = popC(env);
  push(env, [&] {
      if (!name.strictSubtypeOf(TStr)) return TBool;
      auto const v = tv(name);
      if (!v) return TBool;
      auto rcls = env.index.resolve_class(env.ctx, v->m_data.pstr);
      if (!rcls || !rcls->cls()) return TBool;
      auto const mayExist = [&] () -> bool {
        switch (op.subop1) {
          case OODeclExistsOp::Class:
            return !(rcls->cls()->attrs & (AttrInterface | AttrTrait));
          case OODeclExistsOp::Interface:
            return rcls->cls()->attrs & AttrInterface;
          case OODeclExistsOp::Trait:
            return rcls->cls()->attrs & AttrTrait;
        }
        not_reached();
      }();
      auto unit = rcls->cls()->unit;
      auto canConstProp = [&] {
        // Its generally not safe to constprop this, because of
        // autoload. We're safe if its part of systemlib, or a
        // superclass of the current context.
        if (is_systemlib_part(*unit)) return true;
        if (!env.ctx.cls) return false;
        auto thisClass = env.index.resolve_class(env.ctx.cls);
        return thisClass.subtypeOf(*rcls);
      };
      if (canConstProp()) {
        constprop(env);
        return mayExist ? TTrue : TFalse;
      }
      unit->persistent.store(false, std::memory_order_relaxed);
      // At this point, if it mayExist, we still don't know that it
      // *does* exist, but if not we know that it either doesn't
      // exist, or it doesn't have the right type.
      return mayExist ? TBool : TFalse;
    } ());
}

void in(ISS& env, const bc::VerifyParamType& op) {
  if (env.ctx.func->isMemoizeImpl &&
      !locCouldBeRef(env, op.loc1) &&
      options.HardTypeHints) {
    // a MemoizeImpl's params have already been checked by the wrapper
    return reduce(env, bc::Nop {});
  }

  locAsCell(env, op.loc1);
  if (!options.HardTypeHints) return;

  /*
   * In HardTypeHints mode, we assume that if this opcode doesn't
   * throw, the parameter was of the specified type (although it may
   * have been a Ref if the parameter was by reference).
   *
   * The env.setLoc here handles dealing with a parameter that was
   * already known to be a reference.
   *
   * NB: VerifyParamType of a reference parameter can kill any
   * references if it re-enters, even if Option::HardTypeHints is
   * on.
   */
  auto const constraint = env.ctx.func->params[op.loc1].typeConstraint;
  if (!options.CheckThisTypeHints && constraint.isThis()) {
    return;
  }
  if (constraint.hasConstraint() && !constraint.isTypeVar() &&
      !constraint.isTypeConstant()) {
    auto t = env.index.lookup_constraint(env.ctx, constraint);
    if (t.subtypeOf(TBottom)) unreachable(env);
    FTRACE(2, "     {} ({})\n", constraint.fullName(), show(t));
    setLoc(env, op.loc1, std::move(t));
  }
}

void in(ISS& /*env*/, const bc::VerifyRetTypeV& /*op*/) {}

void in(ISS& env, const bc::VerifyRetTypeC& /*op*/) {
  auto const constraint = env.ctx.func->retTypeConstraint;
  auto const stackT = topC(env);

  // If there is no return type constraint, or if the return type
  // constraint is a typevar, or if the top of stack is the same
  // or a subtype of the type constraint, then this is a no-op.
  if (env.index.satisfies_constraint(env.ctx, stackT, constraint)) {
    reduce(env, bc::Nop {});
    return;
  }

  // If HardReturnTypeHints is false OR if the constraint is soft,
  // then there are no optimizations we can safely do here, so
  // just leave the top of stack as is.
  if (!options.HardReturnTypeHints || constraint.isSoft()
      || (!options.CheckThisTypeHints && constraint.isThis())) {
    return;
  }

  // If we reach here, then HardReturnTypeHints is true AND the constraint
  // is not soft.  We can safely assume that either VerifyRetTypeC will
  // throw or it will produce a value whose type is compatible with the
  // return type constraint.
  auto tcT =
    remove_uninit(env.index.lookup_constraint(env.ctx, constraint));

  if (tcT.subtypeOf(TBottom)) {
    unreachable(env);
    return;
  }

  // Below we compute retT, which is a rough conservative approximate of the
  // intersection of stackT and tcT.
  // TODO(4441939): We could do better if we had an intersect_of() function
  // that provided a formal way to compute the intersection of two Types.

  // If tcT could be an interface or trait, we upcast it to TObj/TOptObj.
  // Why?  Because we want uphold the invariant that we only refine return
  // types and never widen them, and if we allow tcT to be an interface then
  // it's possible for violations of this invariant to arise.  For an example,
  // see "hphp/test/slow/hhbbc/return-type-opt-bug.php".
  // Note: It's safe to use TObj/TOptObj because lookup_constraint() only
  // returns classes or interfaces or traits (it never returns something that
  // could be an enum or type alias) and it never returns anything that could
  // be a "magic" interface that supports non-objects.  (For traits the return
  // typehint will always throw at run time, so it's safe to use TObj/TOptObj.)
  if (is_specialized_obj(tcT) && dobj_of(tcT).cls.couldBeInterfaceOrTrait()) {
    tcT = is_opt(tcT) ? TOptObj : TObj;
  }
  // If stackT is a subtype of tcT, use stackT.  Otherwise, if tc is an opt
  // type and stackT cannot be InitNull, then we can safely use unopt(tcT).
  // In all other cases, use tcT.
  auto retT = stackT.subtypeOf(tcT) ? stackT :
                    is_opt(tcT) && !stackT.couldBe(TInitNull) ? unopt(tcT) :
                    tcT;

  // Update the top of stack with the rough conservative approximate of the
  // intersection of stackT and tcT
  popC(env);
  push(env, std::move(retT));
}

void in(ISS& env, const bc::Self& op) {
  auto self = selfClsExact(env);
  putClsRefSlot(env, op.slot, self ? *self : TCls);
}

void in(ISS& env, const bc::Parent& op) {
  auto parent = parentClsExact(env);
  putClsRefSlot(env, op.slot, parent ? *parent : TCls);
}

void in(ISS& env, const bc::CreateCl& op) {
  auto const nargs   = op.arg1;
  auto const clsPair = env.index.resolve_closure_class(env.ctx, op.arg2);

  /*
   * Every closure should have a unique allocation site, but we may see it
   * multiple times in a given round of analyzing this function.  Each time we
   * may have more information about the used variables; the types should only
   * possibly grow.  If it's already there we need to merge the used vars in
   * with what we saw last time.
   */
  if (nargs) {
    std::vector<Type> usedVars(nargs);
    for (auto i = uint32_t{0}; i < nargs; ++i) {
      usedVars[nargs - i - 1] = popT(env);
    }
    merge_closure_use_vars_into(
      env.collect.closureUseTypes,
      clsPair.second,
      usedVars
    );
  }

  // Closure classes can be cloned and rescoped at runtime, so it's not safe to
  // assert the exact type of closure objects. The best we can do is assert
  // that it's a subclass of Closure.
  auto const closure = env.index.resolve_class(env.ctx, s_Closure.get());
  always_assert(closure.hasValue());

  return push(env, subObj(*closure));
}

void in(ISS& env, const bc::CreateCont& /*op*/) {
  // First resume is always next() which pushes null.
  push(env, TInitNull);
}

void in(ISS& env, const bc::ContEnter&) { popC(env); push(env, TInitCell); }
void in(ISS& env, const bc::ContRaise&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::Yield&) {
  popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::YieldK&) {
  popC(env);
  popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ContAssignDelegate&) {
  popC(env);
}

void in(ISS& env, const bc::ContEnterDelegate&) {
  popC(env);
}

void in(ISS& env, const bc::YieldFromDelegate&) {
  push(env, TInitCell);
}

void in(ISS& /*env*/, const bc::ContUnsetDelegate&) {}

void in(ISS& /*env*/, const bc::ContCheck&) {}
void in(ISS& env, const bc::ContValid&)   { push(env, TBool); }
void in(ISS& env, const bc::ContStarted&) { push(env, TBool); }
void in(ISS& env, const bc::ContKey&)     { push(env, TInitCell); }
void in(ISS& env, const bc::ContCurrent&) { push(env, TInitCell); }
void in(ISS& env, const bc::ContGetReturn&) { push(env, TInitCell); }

void pushTypeFromWH(ISS& env, const Type t) {
  if (!t.couldBe(TObj)) {
    // These opcodes require an object descending from WaitHandle.
    // Exceptions will be thrown for any non-object.
    push(env, TBottom);
    unreachable(env);
    return;
  }
  // If we aren't even sure this is a wait handle, there's nothing we can
  // infer here.  (This can happen if a user declares a class with a
  // getWaitHandle method that returns non-WaitHandle garbage.)
  if (!t.subtypeOf(TObj) || !is_specialized_wait_handle(t)) {
    return push(env, TInitCell);
  }

  auto inner = wait_handle_inner(t);
  if (inner.subtypeOf(TBottom)) {
    // If it's a WaitH<Bottom>, we know it's going to throw an exception, and
    // the fallthrough code is not reachable.
    push(env, TBottom);
    unreachable(env);
    return;
  }

  push(env, std::move(inner));
}

void in(ISS& env, const bc::WHResult&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& env, const bc::Await&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& /*env*/, const bc::IncStat&) {}

void in(ISS& env, const bc::Idx&) {
  popC(env); popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ArrayIdx&) {
  popC(env); popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::CheckProp&) {
  if (env.ctx.cls->attrs & AttrNoOverride) {
    return reduce(env, bc::False {});
  }
  nothrow(env);
  push(env, TBool);
}

void in(ISS& env, const bc::InitProp& op) {
  auto const t = topC(env);
  switch (op.subop2) {
    case InitPropOp::Static:
      mergeSelfProp(env, op.str1, t);
      if (auto c = env.collect.publicStatics) {
        auto const cls = selfClsExact(env);
        always_assert(!!cls);
        c->merge(env.ctx, *cls, sval(op.str1), t);
      }
      break;
    case InitPropOp::NonStatic:
      mergeThisProp(env, op.str1, t);
      break;
  }
  if (auto const v = tv(t)) {
    for (auto& prop : env.ctx.func->cls->properties) {
      if (prop.name == op.str1) {
        ITRACE(1, "InitProp: {} = {}\n", op.str1, show(t));
        prop.val = *v;
        if (op.subop2 == InitPropOp::Static &&
            !env.collect.publicStatics &&
            !env.index.frozen()) {
          env.index.fixup_public_static(env.ctx.func->cls, prop.name, t);
        }
        return reduce(env, bc::PopC {});
      }
    }
  }
  popC(env);
}

void in(ISS& env, const bc::Silence& op) {
  nothrow(env);
  switch (op.subop2) {
    case SilenceOp::Start:
      setLoc(env, op.loc1, TInt);
      break;
    case SilenceOp::End:
      break;
  }
}

void in(ISS& /*emv*/, const bc::VarEnvDynCall&) {}
}

//////////////////////////////////////////////////////////////////////

void dispatch(ISS& env, const Bytecode& op) {
#define O(opcode, ...) case Op::opcode: interp_step::in(env, op.opcode); return;
  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<class Iterator, class... Args>
void group(ISS& env, Iterator& it, Args&&... args) {
  FTRACE(2, " {}\n", [&]() -> std::string {
    auto ret = std::string{};
    for (auto i = size_t{0}; i < sizeof...(Args); ++i) {
      ret += " " + show(env.ctx.func, it[i]);
      if (i != sizeof...(Args) - 1) ret += ';';
    }
    return ret;
  }());
  it += sizeof...(Args);
  return interp_step::group(env, std::forward<Args>(args)...);
}

template<class Iterator>
void interpStep(ISS& env, Iterator& it, Iterator stop) {
  /*
   * During the analysis phase, we analyze some common bytecode
   * patterns involving conditional jumps as groups to be able to
   * add additional information to the type environment depending on
   * whether the branch is taken or not.
   */
  auto const o1 = it->op;
  auto const o2 = it + 1 != stop ? it[1].op : Op::Nop;
  auto const o3 = it + 1 != stop &&
                  it + 2 != stop ? it[2].op : Op::Nop;

  switch (o1) {
  case Op::CGetL:
    switch (o2) {
    case Op::JmpZ:   return group(env, it, it[0].CGetL, it[1].JmpZ);
    case Op::JmpNZ:  return group(env, it, it[0].CGetL, it[1].JmpNZ);
    case Op::InstanceOfD:
      switch (o3) {
      case Op::JmpZ:
        return group(env, it, it[0].CGetL, it[1].InstanceOfD, it[2].JmpZ);
      case Op::JmpNZ:
        return group(env, it, it[0].CGetL, it[1].InstanceOfD, it[2].JmpNZ);
      default: break;
      }
      break;
    case Op::FPushObjMethodD:
      return group(env, it, it[0].CGetL, it[1].FPushObjMethodD);
    default: break;
    }
    break;
  case Op::IsTypeL:
    switch (o2) {
    case Op::JmpZ:   return group(env, it, it[0].IsTypeL, it[1].JmpZ);
    case Op::JmpNZ:  return group(env, it, it[0].IsTypeL, it[1].JmpNZ);
    default: break;
    }
    break;
  case Op::IsUninit:
    switch (o2) {
    case Op::JmpZ:   return group(env, it, it[0].IsUninit, it[1].JmpZ);
    case Op::JmpNZ:  return group(env, it, it[0].IsUninit, it[1].JmpNZ);
    default: break;
    }
    break;
  case Op::Dup:
    switch (o2) {
    case Op::IsTypeC:
      switch (o3) {
      case Op::JmpZ:
        return group(env, it, it[0].Dup, it[1].IsTypeC, it[2].JmpZ);
      case Op::JmpNZ:
        return group(env, it, it[0].Dup, it[1].IsTypeC, it[2].JmpNZ);
      default: break;
      }
      break;
    default: break;
    }
    break;
  case Op::MemoGet:
    switch (o2) {
    case Op::IsUninit:
      switch (o3) {
      case Op::JmpZ:
        return group(env, it, it[0].MemoGet, it[1].IsUninit, it[2].JmpZ);
      case Op::JmpNZ:
        return group(env, it, it[0].MemoGet, it[1].IsUninit, it[2].JmpNZ);
      default: break;
      }
      break;
    default: break;
    }
    break;
  case Op::StaticLocCheck:
    switch (o2) {
    case Op::JmpZ:
      return group(env, it, it[0].StaticLocCheck, it[1].JmpZ);
    case Op::JmpNZ:
      return group(env, it, it[0].StaticLocCheck, it[1].JmpNZ);
    default: break;
    }
    break;
  default: break;
  }

  FTRACE(2, "  {}\n", show(env.ctx.func, *it));
  dispatch(env, *it++);
}

template<class Iterator>
StepFlags interpOps(Interp& interp,
                    Iterator& iter, Iterator stop,
                    PropagateFn propagate) {
  auto flags = StepFlags{};
  ISS env { interp, flags, propagate };

  // If there are factored edges, make a copy of the state (except
  // stacks) in case we need to propagate across factored exits (if
  // it's a PEI).
  auto const stateBefore = interp.blk->factoredExits.empty()
    ? State{}
    : without_stacks(interp.state);

  auto const numPushed   = iter->numPush();
  interpStep(env, iter, stop);

  auto outputs_constant = [&] {
    auto elems = &interp.state.stack.back();
    for (auto i = size_t{0}; i < numPushed; ++i, --elems) {
      if (!tv(elems->type)) return false;
    }
    return true;
  };

  if (options.ConstantProp && flags.canConstProp && outputs_constant()) {
    auto elems = &interp.state.stack.back();
    for (auto i = size_t{0}; i < numPushed; ++i, --elems) {
      auto& ty = elems->type;
      ty = from_cell(*tv(ty));
    }
    if (flags.wasPEI) {
      FTRACE(2, "   nothrow (due to constprop)\n");
      flags.wasPEI = false;
    }
  }

  if (flags.wasPEI) {
    FTRACE(2, "   PEI.\n");
    for (auto factored : interp.blk->factoredExits) {
      propagate(factored, stateBefore);
    }
  }
  return flags;
}

//////////////////////////////////////////////////////////////////////

RunFlags run(Interp& interp, PropagateFn propagate) {
  SCOPE_EXIT {
    FTRACE(2, "out {}{}\n",
           state_string(*interp.ctx.func, interp.state, interp.collect),
           property_state_string(interp.collect.props));
  };

  auto ret = RunFlags {};
  auto const stop = end(interp.blk->hhbcs);
  auto iter       = begin(interp.blk->hhbcs);
  while (iter != stop) {
    auto const flags = interpOps(interp, iter, stop, propagate);
    if (flags.usedLocalStatics) {
      if (!ret.usedLocalStatics) {
        ret.usedLocalStatics = std::move(flags.usedLocalStatics);
      } else {
        for (auto& elm : *flags.usedLocalStatics) {
          ret.usedLocalStatics->insert(std::move(elm));
        }
      }
    }

    if (interp.state.unreachable) {
      FTRACE(2, "  <bytecode fallthrough is unreachable>\n");
      return ret;
    }

    switch (flags.jmpFlag) {
    case StepFlags::JmpFlags::Taken:
      FTRACE(2, "  <took branch; no fallthrough>\n");
      return ret;
    case StepFlags::JmpFlags::Fallthrough:
    case StepFlags::JmpFlags::Either:
      break;
    }
    if (flags.returned) {
      FTRACE(2, "  returned {}\n", show(*flags.returned));
      always_assert(iter == stop);
      always_assert(interp.blk->fallthrough == NoBlockId);
      ret.returned = flags.returned;
      return ret;
    }
  }

  FTRACE(2, "  <end block>\n");
  if (interp.blk->fallthrough != NoBlockId) {
    propagate(interp.blk->fallthrough, interp.state);
  }
  return ret;
}

StepFlags step(Interp& interp, const Bytecode& op) {
  auto flags   = StepFlags{};
  auto noop    = [] (BlockId, const State&) {};
  ISS env { interp, flags, noop };
  dispatch(env, op);
  return flags;
}

void default_dispatch(ISS& env, const Bytecode& op) {
  dispatch(env, op);
}

//////////////////////////////////////////////////////////////////////

}}
