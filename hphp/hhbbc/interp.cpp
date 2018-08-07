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
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
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

const StaticString s_Throwable("Throwable");
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

  env.flags.wasPEI          = false;
  env.flags.canConstProp    = true;
  env.flags.effectFree      = true;

  for (auto it = begin(bcs); it != end(bcs); ++it) {
    assert(env.flags.jmpDest == NoBlockId &&
           "you can't use impl with branching opcodes before last position");

    auto const wasPEI = env.flags.wasPEI;
    auto const canConstProp = env.flags.canConstProp;
    auto const effectFree = env.flags.effectFree;

    FTRACE(3, "    (impl {}\n", show(env.ctx.func, *it));
    env.flags.wasPEI          = true;
    env.flags.canConstProp    = false;
    env.flags.effectFree      = false;
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
      auto applyConstProp = [&] {
        if (env.flags.effectFree && !env.flags.wasPEI) return;
        auto stk = env.state.stack.end();
        for (auto i = it->numPush(); i--; ) {
          --stk;
          if (!is_scalar(stk->type)) return;
        }
        env.flags.effectFree = true;
        env.flags.wasPEI = false;
      };
      if (reduce) {
        auto added = false;
        if (env.flags.canConstProp) {
          if (env.collect.propagate_constants) {
            if (env.collect.propagate_constants(*it, env.state,
                                                currentReduction)) {
              added = true;
              env.flags.canConstProp = false;
              env.flags.wasPEI = false;
              env.flags.effectFree = true;
            }
          } else {
            applyConstProp();
          }
        }
        if (!added) currentReduction.push_back(std::move(*it));
      } else if (env.flags.canConstProp) {
        applyConstProp();
      }
    }

    // If any of the opcodes in the impl list said they could throw,
    // then the whole thing could throw.
    env.flags.wasPEI = env.flags.wasPEI || wasPEI;
    env.flags.canConstProp = env.flags.canConstProp && canConstProp;
    env.flags.effectFree = env.flags.effectFree && effectFree;
    if (env.state.unreachable) break;
  }

  if (reduce) {
    env.flags.strengthReduced = std::move(currentReduction);
  } else {
    env.flags.strengthReduced = folly::none;
  }
}

LocalId equivLocalRange(ISS& env, const LocalRange& range) {
  auto bestRange = range.first;
  auto equivFirst = findLocEquiv(env, range.first);
  if (equivFirst == NoLocalId) return bestRange;
  do {
    if (equivFirst < bestRange) {
      auto equivRange = [&] {
        // local equivalency includes differing by Uninit, so we need
        // to check the types.
        if (peekLocRaw(env, equivFirst) != peekLocRaw(env, range.first)) {
          return false;
        }

        for (uint32_t i = 1; i < range.count; ++i) {
          if (!locsAreEquiv(env, equivFirst + i, range.first + i) ||
              peekLocRaw(env, equivFirst + i) !=
              peekLocRaw(env, range.first + i)) {
            return false;
          }
        }

        return true;
      }();

      if (equivRange) {
        bestRange = equivFirst;
      }
    }
    equivFirst = findLocEquiv(env, equivFirst);
    assert(equivFirst != NoLocalId);
  } while (equivFirst != range.first);

  return bestRange;
}

namespace interp_step {

void in(ISS& env, const bc::Nop&)  { effect_free(env); }
void in(ISS& env, const bc::DiscardClsRef& op) {
  nothrow(env);
  takeClsRefSlot(env, op.slot);
}
void in(ISS& env, const bc::PopC&) {
  nothrow(env);
  if (!could_run_destructor(popC(env))) effect_free(env);
}
void in(ISS& env, const bc::PopU&) { effect_free(env); popU(env); }
void in(ISS& env, const bc::PopV&) { nothrow(env); popV(env); }
void in(ISS& env, const bc::PopR&) {
  auto t = topT(env, 0);
  if (t.subtypeOf(BCell)) {
    return reduce(env, bc::UnboxRNop {}, bc::PopC {});
  }
  nothrow(env);
  popR(env);
}

void in(ISS& env, const bc::EntryNop&) { effect_free(env); }

void in(ISS& env, const bc::Dup& /*op*/) {
  effect_free(env);
  auto equiv = topStkEquiv(env);
  auto val = popC(env);
  push(env, val, equiv);
  push(env, std::move(val), StackDupId);
}

void in(ISS& env, const bc::AssertRATL& op) {
  mayReadLocal(env, op.loc1);
  effect_free(env);
}

void in(ISS& env, const bc::AssertRATStk&) {
  effect_free(env);
}

void in(ISS& env, const bc::BreakTraceHint&) { effect_free(env); }

void in(ISS& env, const bc::Box&) {
  effect_free(env);
  popC(env);
  push(env, TRef);
}

void in(ISS& env, const bc::BoxR&) {
  effect_free(env);
  if (topR(env).subtypeOf(BRef)) {
    return reduce(env, bc::BoxRNop {});
  }
  popR(env);
  push(env, TRef);
}

void in(ISS& env, const bc::Unbox&) {
  effect_free(env);
  popV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::UnboxR&) {
  auto const t = topR(env);
  if (t.subtypeOf(BInitCell)) return reduce(env, bc::UnboxRNop {});
  nothrow(env);
  popT(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::RGetCNop&) { effect_free(env); }

void in(ISS& env, const bc::CGetCUNop&) {
  effect_free(env);
  auto const t = popCU(env);
  push(env, remove_uninit(t));
}

void in(ISS& env, const bc::UGetCUNop&) {
  effect_free(env);
  popCU(env);
  push(env, TUninit);
}

void in(ISS& env, const bc::UnboxRNop&) {
  effect_free(env);
  constprop(env);
  auto t = popR(env);
  if (!t.subtypeOf(BInitCell)) t = TInitCell;
  push(env, std::move(t));
}

void in(ISS& env, const bc::BoxRNop&) {
  effect_free(env);
  auto t = popR(env);
  if (!t.subtypeOf(BRef)) t = TRef;
  push(env, std::move(t));
}

void in(ISS& env, const bc::Null&) {
  effect_free(env);
  push(env, TInitNull);
}

void in(ISS& env, const bc::NullUninit&) {
  effect_free(env);
  push(env, TUninit);
}

void in(ISS& env, const bc::True&) {
  effect_free(env);
  push(env, TTrue);
}

void in(ISS& env, const bc::False&) {
  effect_free(env);
  push(env, TFalse);
}

void in(ISS& env, const bc::Int& op) {
  effect_free(env);
  push(env, ival(op.arg1));
}

void in(ISS& env, const bc::Double& op) {
  effect_free(env);
  push(env, dval(op.dbl1));
}

void in(ISS& env, const bc::String& op) {
  effect_free(env);
  push(env, sval(op.str1));
}

void in(ISS& env, const bc::Array& op) {
  assert(op.arr1->isPHPArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || op.arr1->isNotDVArray());
  effect_free(env);
  push(env, aval(op.arr1));
}

void in(ISS& env, const bc::Vec& op) {
  assert(op.arr1->isVecArray());
  effect_free(env);
  push(env, vec_val(op.arr1));
}

void in(ISS& env, const bc::Dict& op) {
  assert(op.arr1->isDict());
  effect_free(env);
  push(env, dict_val(op.arr1));
}

void in(ISS& env, const bc::Keyset& op) {
  assert(op.arr1->isKeyset());
  effect_free(env);
  push(env, keyset_val(op.arr1));
}

void in(ISS& env, const bc::NewArray& op) {
  push(env, op.arg1 == 0 ?
       effect_free(env), aempty() : some_aempty());
}

void in(ISS& env, const bc::NewDictArray& op) {
  push(env, op.arg1 == 0 ?
       effect_free(env), dict_empty() : some_dict_empty());
}

void in(ISS& env, const bc::NewMixedArray& op) {
  push(env, op.arg1 == 0 ?
       effect_free(env), aempty() : some_aempty());
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

void in(ISS& env, const bc::NewVArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  push(env, arr_packed_varray(std::move(elems)));
  constprop(env);
}

void in(ISS& env, const bc::NewDArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  push(env, op.arg1 == 0 ?
       effect_free(env), aempty_darray() : some_aempty_darray());
}

void in(ISS& env, const bc::NewStructArray& op) {
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, arr_map(std::move(map)));
  constprop(env);
}

void in(ISS& env, const bc::NewStructDArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, arr_map_darray(std::move(map)));
  constprop(env);
}

void in(ISS& env, const bc::NewStructDict& op) {
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, dict_map(std::move(map)));
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
  push(env, some_aempty());
}

void in(ISS& env, const bc::AddElemC& /*op*/) {
  auto const v = popC(env);
  auto const k = popC(env);

  auto const outTy = [&] (Type ty) ->
    folly::Optional<std::pair<Type,ThrowMode>> {
    if (ty.subtypeOf(BArr)) {
      return array_set(std::move(ty), k, v);
    }
    if (ty.subtypeOf(BDict)) {
      return dict_set(std::move(ty), k, v);
    }
    return folly::none;
  }(popC(env));

  if (!outTy) {
    return push(env, union_of(TArr, TDict));
  }

  if (outTy->first.subtypeOf(BBottom)) {
    unreachable(env);
  } else if (outTy->second == ThrowMode::None) {
    nothrow(env);
    if (any(env.collect.opts & CollectionOpts::TrackConstantArrays)) {
      constprop(env);
    }
  }
  push(env, std::move(outTy->first));
}

void in(ISS& env, const bc::AddElemV& /*op*/) {
  popV(env); popC(env);
  auto const ty = popC(env);
  auto const outTy =
    ty.subtypeOf(BArr) ? TArr
    : ty.subtypeOf(BDict) ? TDict
    : union_of(TArr, TDict);
  push(env, outTy);
}

void in(ISS& env, const bc::AddNewElemC&) {
  auto v = popC(env);

  auto const outTy = [&] (Type ty) -> folly::Optional<Type> {
    if (ty.subtypeOf(BArr)) {
      return array_newelem(std::move(ty), std::move(v)).first;
    }
    if (ty.subtypeOf(BVec)) {
      return vec_newelem(std::move(ty), std::move(v)).first;
    }
    if (ty.subtypeOf(BKeyset)) {
      return keyset_newelem(std::move(ty), std::move(v)).first;
    }
    return folly::none;
  }(popC(env));

  if (!outTy) {
    return push(env, TInitCell);
  }

  if (outTy->subtypeOf(BBottom)) {
    unreachable(env);
  } else {
    if (any(env.collect.opts & CollectionOpts::TrackConstantArrays)) {
      constprop(env);
    }
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

void doCns(ISS& env, SString str, SString fallback)  {
  if (!options.HardConstProp) return push(env, TInitCell);

  auto t = env.index.lookup_constant(env.ctx, str, fallback);
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
    // constprop will take care of nothrow *if* its a constant; and if
    // its not, we might trigger autoload.
    constprop(env);
  }
  push(env, std::move(*t));
}

void in(ISS& env, const bc::Cns& op)  { doCns(env, op.str1, nullptr); }
void in(ISS& env, const bc::CnsE& op) { doCns(env, op.str1, nullptr); }
void in(ISS& env, const bc::CnsU& op) { doCns(env, op.str1, op.str2); }

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

void in(ISS& env, const bc::File&)   { effect_free(env); push(env, TSStr); }
void in(ISS& env, const bc::Dir&)    { effect_free(env); push(env, TSStr); }
void in(ISS& env, const bc::Method&) { effect_free(env); push(env, TSStr); }

void in(ISS& env, const bc::ClsRefName& op) {
  nothrow(env);
  takeClsRefSlot(env, op.slot);
  push(env, TSStr);
}

void concatHelper(ISS& env, uint32_t n) {
  uint32_t i = 0;
  StringData* result = nullptr;
  while (i < n) {
    auto const t = topC(env, i);
    auto const v = tv(t);
    if (!v) break;
    if (!isStringType(v->m_type)   &&
        v->m_type != KindOfNull    &&
        v->m_type != KindOfBoolean &&
        v->m_type != KindOfInt64   &&
        v->m_type != KindOfDouble) {
      break;
    }
    auto const cell = eval_cell_value([&] {
        auto const s = makeStaticString(
          result ?
          StringData::Make(tvAsCVarRef(&*v).toString().get(), result) :
          tvAsCVarRef(&*v).toString().get());
        return make_tv<KindOfString>(s);
      });
    if (!cell) break;
    result = cell->m_data.pstr;
    i++;
  }
  if (result && i >= 2) {
    std::vector<Bytecode> bcs(i, bc::PopC {});
    bcs.push_back(gen_constant(make_tv<KindOfString>(result)));
    if (i < n) {
      bcs.push_back(bc::ConcatN { n - i + 1 });
    }
    return reduce(env, std::move(bcs));
  }
  discard(env, n);
  push(env, TStr);
}

void in(ISS& env, const bc::Concat& /*op*/) {
  concatHelper(env, 2);
}

void in(ISS& env, const bc::ConcatN& op) {
  if (op.arg1 == 2) return reduce(env, bc::Concat {});
  concatHelper(env, op.arg1);
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
  return t.couldBe(BVec | BDict | BKeyset);
}

template<bool NSame>
std::pair<Type,bool> resolveSame(ISS& env) {
  auto const l1 = topStkEquiv(env, 0);
  auto const t1 = topC(env, 0);
  auto const l2 = topStkEquiv(env, 1);
  auto const t2 = topC(env, 1);

  auto const mightWarn = [&] {
    // EvalHackArrCompatNotices will notice on === and !== between PHP arrays
    // and Hack arrays.
    if (RuntimeOption::EvalHackArrCompatNotices) {
      if (t1.couldBe(BArr) && couldBeHackArr(t2)) return true;
      if (couldBeHackArr(t1) && t2.couldBe(BArr)) return true;
    }
    if (RuntimeOption::EvalHackArrCompatDVCmpNotices) {
      if (!t1.couldBe(BArr) || !t2.couldBe(BArr)) return false;
      if (t1.subtypeOf(BPArr) && t2.subtypeOf(BPArr)) return false;
      if (t1.subtypeOf(BVArr) && t2.subtypeOf(BVArr)) return false;
      if (t1.subtypeOf(BDArr) && t2.subtypeOf(BDArr)) return false;
      return true;
    }
    return false;
  };

  auto const result = [&] {
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);

    if (l1 == StackDupId ||
        (l1 <= MaxLocalId && l2 <= MaxLocalId &&
         (l1 == l2 || locsAreEquiv(env, l1, l2)))) {
      if (!t1.couldBe(BDbl) || !t2.couldBe(BDbl) ||
          (v1 && (v1->m_type != KindOfDouble || !std::isnan(v1->m_data.dbl))) ||
          (v2 && (v2->m_type != KindOfDouble || !std::isnan(v2->m_data.dbl)))) {
        return NSame ? TFalse : TTrue;
      }
    }

    if (v1 && v2) {
      if (auto r = eval_cell_value([&]{ return cellSame(*v2, *v1); })) {
        return r != NSame ? TTrue : TFalse;
      }
    }

    return NSame ? typeNSame(t1, t2) : typeSame(t1, t2);
  };

  return { result(), mightWarn() };
}

template<bool Negate>
void sameImpl(ISS& env) {
  auto pair = resolveSame<Negate>(env);
  discard(env, 2);

  if (!pair.second) {
    nothrow(env);
    constprop(env);
  }

  push(env, std::move(pair.first));
}

template<class Same, class JmpOp>
void sameJmpImpl(ISS& env, const Same& same, const JmpOp& jmp) {
  auto bail = [&] { impl(env, same, jmp); };

  constexpr auto NSame = Same::op == Op::NSame;

  if (resolveSame<NSame>(env).first != TBool) return bail();

  auto const loc0 = topStkEquiv(env, 0);
  auto const loc1 = topStkEquiv(env, 1);
  if (loc0 == NoLocalId && loc1 == NoLocalId) return bail();

  auto const ty0 = topC(env, 0);
  auto const ty1 = topC(env, 1);
  auto const val0 = tv(ty0);
  auto const val1 = tv(ty1);

  if ((val0 && val1) ||
      (loc0 == NoLocalId && !val0 && ty1.subtypeOf(ty0)) ||
      (loc1 == NoLocalId && !val1 && ty0.subtypeOf(ty1))) {
    return bail();
  }

  // We need to loosen away the d/varray bits here because array comparison does
  // not take into account the difference.
  auto isect = intersection_of(
    loosen_dvarrayness(ty0),
    loosen_dvarrayness(ty1)
  );
  discard(env, 2);

  auto handle_same = [&] {
    // Currently dce uses equivalency to prove that something isn't
    // the last reference - so we can only assert equivalency here if
    // we know that won't be affected. Its irrelevant for uncounted
    // things, and for TObj and TRes, $x === $y iff $x and $y refer to
    // the same thing.
    if (loc0 <= MaxLocalId && loc1 <= MaxLocalId &&
        (ty0.subtypeOfAny(TOptObj, TOptRes) ||
         ty1.subtypeOfAny(TOptObj, TOptRes) ||
         (ty0.subtypeOf(BUnc) && ty1.subtypeOf(BUnc)))) {
      if (loc1 == StackDupId) {
        setStkLocal(env, loc0);
      } else if (loc0 != loc1 && !locsAreEquiv(env, loc0, loc1)) {
        auto loc = loc0;
        while (true) {
          auto const other = findLocEquiv(env, loc);
          if (other == NoLocalId) break;
          killLocEquiv(env, loc);
          addLocEquiv(env, loc, loc1);
          loc = other;
        }
        addLocEquiv(env, loc, loc1);
      }
    }
    return refineLocation(env, loc1 != NoLocalId ? loc1 : loc0, [&] (Type ty) {
      if (!ty.couldBe(BUninit) || !isect.couldBe(BNull)) {
        auto ret = intersection_of(std::move(ty), isect);
        return ty.subtypeOf(BUnc) ? ret : loosen_staticness(ret);
      }

      if (isect.subtypeOf(BNull)) {
        return ty.couldBe(BInitNull) ? TNull : TUninit;
      }

      return ty;
    });
  };

  auto handle_differ_side = [&] (LocalId location, const Type& ty) {
    if (!ty.subtypeOf(BInitNull) && !ty.strictSubtypeOf(TBool)) return true;
    return refineLocation(env, location, [&] (Type t) {
      if (ty.subtypeOf(BNull)) {
        t = remove_uninit(std::move(t));
        if (is_opt(t)) t = unopt(std::move(t));
        return t;
      } else if (ty.strictSubtypeOf(TBool) && t.subtypeOf(BBool)) {
        return ty == TFalse ? TTrue : TFalse;
      }
      return t;
    });
  };

  auto handle_differ = [&] {
    return
      (loc0 == NoLocalId || handle_differ_side(loc0, ty1)) &&
      (loc1 == NoLocalId || handle_differ_side(loc1, ty0));
  };

  auto const sameIsJmpTarget =
    (Same::op == Op::Same) == (JmpOp::op == Op::JmpNZ);

  auto save = env.state;
  if (sameIsJmpTarget ? handle_same() : handle_differ()) {
    env.propagate(jmp.target, &env.state);
  } else {
    jmp_nevertaken(env);
  }
  env.state = std::move(save);
  if (!(sameIsJmpTarget ? handle_differ() : handle_same())) {
    jmp_setdest(env, jmp.target);
  }
}

bc::JmpNZ invertJmp(const bc::JmpZ& jmp) { return bc::JmpNZ { jmp.target }; }
bc::JmpZ invertJmp(const bc::JmpNZ& jmp) { return bc::JmpZ { jmp.target }; }

}

template<class Same, class JmpOp>
void group(ISS& env, const Same& same, const JmpOp& jmp) {
  sameJmpImpl(env, same, jmp);
}

template<class Same, class JmpOp>
void group(ISS& env, const Same& same, const bc::Not&, const JmpOp& jmp) {
  sameJmpImpl(env, same, invertJmp(jmp));
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
  auto rs = resolveSame<false>(env);
  if (rs.first == TTrue) {
    if (!rs.second) constprop(env);
    discard(env, 2);
    return push(env, TTrue);
  }
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellEqual(c1, c2); });
}
void in(ISS& env, const bc::Neq&) {
  auto rs = resolveSame<false>(env);
  if (rs.first == TTrue) {
    if (!rs.second) constprop(env);
    discard(env, 2);
    return push(env, TFalse);
  }
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

void castBoolImpl(ISS& env, const Type& t, bool negate) {
  nothrow(env);
  constprop(env);

  auto const e = emptiness(t);
  switch (e) {
    case Emptiness::Empty:
    case Emptiness::NonEmpty:
      return push(env, (e == Emptiness::Empty) == negate ? TTrue : TFalse);
    case Emptiness::Maybe:
      break;
  }

  push(env, TBool);
}

void in(ISS& env, const bc::Not&) {
  castBoolImpl(env, popC(env), true);
}

void in(ISS& env, const bc::CastBool&) {
  auto const t = topC(env);
  if (t.subtypeOf(BBool)) return reduce(env, bc::Nop {});
  castBoolImpl(env, popC(env), false);
}

void in(ISS& env, const bc::CastInt&) {
  constprop(env);
  auto const t = topC(env);
  if (t.subtypeOf(BInt)) return reduce(env, bc::Nop {});
  popC(env);
  // Objects can raise a warning about converting to int.
  if (!t.couldBe(BObj)) nothrow(env);
  if (auto const v = tv(t)) {
    auto cell = eval_cell([&] {
      return make_tv<KindOfInt64>(cellToInt(*v));
    });
    if (cell) return push(env, std::move(*cell));
  }
  push(env, TInt);
}

// Handle a casting operation, where "target" is the type being casted to. If
// "fn" is provided, it will be called to cast any constant inputs. If "elide"
// is set to true, if the source type is the same as the destination, the cast
// will be optimized away.
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
  castImpl(env, TPArr, tvCastToArrayInPlace);
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
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  castImpl(env, TVArr, tvCastToVArrayInPlace);
}

void in(ISS& env, const bc::CastDArray&)  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  castImpl(env, TDArr, tvCastToDArrayInPlace);
}

void in(ISS& env, const bc::DblAsBits&) {
  nothrow(env);
  constprop(env);

  auto const ty = popC(env);
  if (!ty.couldBe(BDbl)) return push(env, ival(0));

  if (auto val = tv(ty)) {
    assertx(isDoubleType(val->m_type));
    val->m_type = KindOfInt64;
    push(env, from_cell(*val));
    return;
  }

  push(env, TInt);
}

void in(ISS& env, const bc::Print& /*op*/) {
  popC(env);
  push(env, ival(1));
}

void in(ISS& env, const bc::Clone& /*op*/) {
  auto val = popC(env);
  if (!val.subtypeOf(BObj)) {
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
  auto const location = topStkEquiv(env);
  auto const e = emptiness(popC(env));
  if (e == (Negate ? Emptiness::NonEmpty : Emptiness::Empty)) {
    jmp_setdest(env, op.target);
    env.propagate(op.target, &env.state);
    return;
  }

  if (e == (Negate ? Emptiness::Empty : Emptiness::NonEmpty)) {
    jmp_nevertaken(env);
    return;
  }

  if (next_real_block(*env.ctx.func, env.blk.fallthrough) ==
      next_real_block(*env.ctx.func, op.target)) {
    jmp_nevertaken(env);
    return;
  }

  if (location == NoLocalId) return env.propagate(op.target, &env.state);

  auto val = peekLocation(env, location);
  assertx(!val.couldBe(BRef)); // we shouldn't have an equivLoc if it was

  refineLocation(env, location,
                 Negate ? assert_nonemptiness : assert_emptiness,
                 op.target,
                 Negate ? assert_emptiness : assert_nonemptiness);
}

void in(ISS& env, const bc::JmpNZ& op) { jmpImpl<true>(env, op); }
void in(ISS& env, const bc::JmpZ& op)  { jmpImpl<false>(env, op); }

void in(ISS& env, const bc::Select& op) {
  auto const cond = topC(env);
  auto const t = topC(env, 1);
  auto const f = topC(env, 2);

  nothrow(env);
  constprop(env);

  switch (emptiness(cond)) {
    case Emptiness::Maybe:
      discard(env, 3);
      push(env, union_of(t, f));
      return;
    case Emptiness::NonEmpty:
      discard(env, 3);
      push(env, t);
      return;
    case Emptiness::Empty:
      return reduce(env, bc::PopC {}, bc::PopC {});
  }
  not_reached();
}

namespace {

template<class IsType, class JmpOp>
void isTypeHelper(ISS& env,
                  IsTypeOp typeOp, LocalId location,
                  const IsType& istype, const JmpOp& jmp) {

  if (typeOp == IsTypeOp::Scalar || typeOp == IsTypeOp::ArrLike) {
    return impl(env, istype, jmp);
  }

  auto const val = istype.op == Op::IsTypeC ?
    topT(env) : locRaw(env, location);
  auto const testTy = type_of_istype(typeOp);
  if (!val.subtypeOf(BCell) || val.subtypeOf(testTy) || !val.couldBe(testTy)) {
    return impl(env, istype, jmp);
  }

  if (istype.op == Op::IsTypeC) {
    if (!is_type_might_raise(testTy, val)) nothrow(env);
    popT(env);
  } else if (!locCouldBeUninit(env, location) &&
             !is_type_might_raise(testTy, val)) {
    nothrow(env);
  }

  auto const negate = jmp.op == Op::JmpNZ;
  auto const was_true = [&] (Type t) {
    if (testTy.subtypeOf(BNull)) return intersection_of(t, TNull);
    assertx(!testTy.couldBe(BNull));
    return intersection_of(t, testTy);
  };
  auto const was_false = [&] (Type t) {
    auto tinit = remove_uninit(t);
    if (testTy.subtypeOf(BNull)) {
      return is_opt(tinit) ? unopt(tinit) : tinit;
    }
    if (is_opt(tinit)) {
      assertx(!testTy.couldBe(BNull));
      if (unopt(tinit).subtypeOf(testTy)) return TNull;
    }
    return t;
  };

  auto const pre = [&] (Type t) {
    return negate ? was_true(std::move(t)) : was_false(std::move(t));
  };

  auto const post = [&] (Type t) {
    return negate ? was_false(std::move(t)) : was_true(std::move(t));
  };

  refineLocation(env, location, pre, jmp.target, post);
}

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
// of the function being wrapped along with if the wrapped function is effect
// free.
std::pair<Type, bool> memoizeImplRetType(ISS& env) {
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

  // Determine the context the wrapped function will be called on.
  auto const ctxType = [&]() -> Type {
    if (env.ctx.func->cls) {
      if (env.ctx.func->attrs & AttrStatic) {
        // The class context for static methods is the method's class,
        // if LSB is not specified.
        auto const clsTy =
          env.ctx.func->isMemoizeWrapperLSB ?
          selfCls(env) :
          selfClsExact(env);
        return clsTy ? *clsTy : TCls;
      } else {
        auto const s = thisType(env);
        return s ? *s : TObj;
      }
    }
    return TBottom;
  }();

  auto retTy = env.index.lookup_return_type(
    CallContext { env.ctx, args, ctxType },
    memo_impl_func
  );
  auto const effectFree = env.index.is_effect_free(memo_impl_func);
  // Regardless of anything we know the return type will be an InitCell (this is
  // a requirement of memoize functions).
  if (!retTy.subtypeOf(BInitCell)) return { TInitCell, effectFree };
  return { retTy, effectFree };
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

  if (valTy.subtypeOf(testTy) || failTy.subtypeOf(BBottom)) {
    push(env, std::move(valTy));
    if (takenOnSuccess) {
      jmp_setdest(env, jmp.target);
      env.propagate(jmp.target, &env.state);
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
      jmp_setdest(env, jmp.target);
      env.propagate(jmp.target, &env.state);
    }
    return;
  }

  push(env, std::move(takenOnSuccess ? testTy : failTy));
  env.propagate(jmp.target, &env.state);
  discard(env, 1);
  push(env, std::move(takenOnSuccess ? failTy : testTy));
}

// After a StaticLocCheck, we know the local is bound on the true path,
// and not changed on the false path.
template<class JmpOp>
void staticLocCheckJmpImpl(ISS& env,
                           const bc::StaticLocCheck& slc,
                           const JmpOp& jmp) {
  auto const takenOnInit = jmp.op == Op::JmpNZ;
  auto save = env.state;

  if (auto const v = staticLocHelper(env, slc.loc1, TBottom)) {
    return impl(env, slc, jmp);
  }

  if (env.collect.localStaticTypes.size() > slc.loc1 &&
      env.collect.localStaticTypes[slc.loc1].subtypeOf(BBottom)) {
    if (takenOnInit) {
      env.state = std::move(save);
      jmp_nevertaken(env);
    } else {
      env.propagate(jmp.target, &save);
      jmp_setdest(env, jmp.target);
    }
    return;
  }

  if (takenOnInit) {
    env.propagate(jmp.target, &env.state);
    env.state = std::move(save);
  } else {
    env.propagate(jmp.target, &save);
  }
}

}

template<class JmpOp>
void group(ISS& env, const bc::StaticLocCheck& slc, const JmpOp& jmp) {
  staticLocCheckJmpImpl(env, slc, jmp);
}

template<class JmpOp>
void group(ISS& env, const bc::StaticLocCheck& slc,
           const bc::Not&, const JmpOp& jmp) {
  staticLocCheckJmpImpl(env, slc, invertJmp(jmp));
}

template<class JmpOp>
void group(ISS& env, const bc::IsTypeL& istype, const JmpOp& jmp) {
  isTypeHelper(env, istype.subop2, istype.loc1, istype, jmp);
}

template<class JmpOp>
void group(ISS& env, const bc::IsTypeL& istype,
           const bc::Not&, const JmpOp& jmp) {
  isTypeHelper(env, istype.subop2, istype.loc1, istype, invertJmp(jmp));
}

// If we duplicate a value, and then test its type and Jmp based on that result,
// we can narrow the type of the top of the stack. Only do this for null checks
// right now (because its useful in memoize wrappers).
template<class JmpOp>
void group(ISS& env, const bc::IsTypeC& istype, const JmpOp& jmp) {
  auto const location = topStkEquiv(env);
  if (location == NoLocalId) return impl(env, istype, jmp);
  isTypeHelper(env, istype.subop1, location, istype, jmp);
}

template<class JmpOp>
void group(ISS& env, const bc::IsTypeC& istype,
           const bc::Not& negate, const JmpOp& jmp) {
  auto const location = topStkEquiv(env);
  if (location == NoLocalId) return impl(env, istype, negate, jmp);
  isTypeHelper(env, istype.subop1, location, istype, invertJmp(jmp));
}

namespace {

template<class JmpOp>
void instanceOfJmpImpl(ISS& env,
                       const bc::InstanceOfD& inst,
                       const JmpOp& jmp) {
  auto bail = [&] { impl(env, inst, jmp); };

  auto const locId = topStkEquiv(env);
  if (locId == NoLocalId || interface_supports_non_objects(inst.str1)) {
    return bail();
  }
  auto const rcls = env.index.resolve_class(env.ctx, inst.str1);
  if (!rcls) return bail();

  auto const val = topC(env);
  auto const instTy = subObj(*rcls);
  if (val.subtypeOf(instTy) || !val.couldBe(instTy)) {
    return bail();
  }

  // If we have an optional type, whose unopt is guaranteed to pass
  // the instanceof check, then failing to pass implies it was null.
  auto const fail_implies_null = is_opt(val) && unopt(val).subtypeOf(instTy);

  popC(env);
  auto const negate = jmp.op == Op::JmpNZ;
  auto const result = [&] (Type t, bool pass) {
    return pass ? instTy : fail_implies_null ? TNull : t;
  };
  auto const pre  = [&] (Type t) { return result(t, negate); };
  auto const post = [&] (Type t) { return result(t, !negate); };
  refineLocation(env, locId, pre, jmp.target, post);
}

}

template<class JmpOp>
void group(ISS& env,
           const bc::InstanceOfD& inst,
           const JmpOp& jmp) {
  instanceOfJmpImpl(env, inst, jmp);
}

template<class JmpOp>
void group(ISS& env,
           const bc::InstanceOfD& inst,
           const bc::Not&,
           const JmpOp& jmp) {
  instanceOfJmpImpl(env, inst, invertJmp(jmp));
}

namespace {

template<class JmpOp>
void isTypeStructJmpImpl(ISS& env,
                       const bc::IsTypeStruct& inst,
                       const JmpOp& jmp) {
  auto bail = [&] { impl(env, inst, jmp); };

  auto const locId = topStkEquiv(env);
  if (locId == NoLocalId) return bail();

  auto ts_type = type_of_type_structure(inst.arr1);
  if (!ts_type) return bail();

  // TODO(T26859386): refine if ($x is nonnull) case

  popC(env);
  auto const negate = jmp.op == Op::JmpNZ;
  auto const result = [&] (Type t, bool pass) {
    if (!pass) {
      auto tinit = remove_uninit(t);
      if (tinit.subtypeOf(*ts_type)) return TBottom;
      if (t.couldBe(BNull)) {
        auto tnonnull = is_opt(tinit) ? unopt(tinit) : tinit;
        if (ts_type->couldBe(BNull)) {
          return tnonnull;
        }
        if (tnonnull.subtypeOf(*ts_type)) {
          return TNull;
        }
      }

      return t;
    }
    if (t.couldBe(BUninit) && ts_type->couldBe(BNull)) {
      return union_of(intersection_of(std::move(t),
                                      std::move(ts_type.value())),
                      TUninit);
    }
    return intersection_of(std::move(t), std::move(ts_type.value()));
  };
  auto const pre  = [&] (Type t) { return result(std::move(t), negate); };
  auto const post = [&] (Type t) { return result(std::move(t), !negate); };
  refineLocation(env, locId, pre, jmp.target, post);
}

}

template<class JmpOp>
void group(ISS& env,
           const bc::IsTypeStruct& inst,
           const JmpOp& jmp) {
  isTypeStructJmpImpl(env, inst, jmp);
}

template<class JmpOp>
void group(ISS& env,
           const bc::IsTypeStruct& inst,
           const bc::Not&,
           const JmpOp& jmp) {
  isTypeStructJmpImpl(env, inst, invertJmp(jmp));
}

void in(ISS& env, const bc::Switch& op) {
  auto v = tv(popC(env));

  if (v) {
    auto go = [&] (BlockId blk) {
      effect_free(env);
      env.propagate(blk, &env.state);
      jmp_setdest(env, blk);
    };
    auto num_elems = op.targets.size();
    if (op.subop1 == SwitchKind::Bounded) {
      if (v->m_type == KindOfInt64 &&
          v->m_data.num >= 0 && v->m_data.num < num_elems) {
        return go(op.targets[v->m_data.num]);
      }
    } else {
      assertx(num_elems > 2);
      num_elems -= 2;
      for (auto i = size_t{}; ; i++) {
        if (i == num_elems) {
          return go(op.targets.back());
        }
        auto match = eval_cell_value([&] {
            return cellEqual(*v, static_cast<int64_t>(op.arg2 + i));
        });
        if (!match) break;
        if (*match) {
          return go(op.targets[i]);
        }
      }
    }
  }

  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, &env.state);
  });
}

void in(ISS& env, const bc::SSwitch& op) {
  auto v = tv(popC(env));

  if (v) {
    for (auto& kv : op.targets) {
      auto match = eval_cell_value([&] {
        return !kv.first || cellEqual(*v, kv.first);
      });
      if (!match) break;
      if (*match) {
        effect_free(env);
        env.propagate(kv.second, &env.state);
        jmp_setdest(env, kv.second);
        return;
      }
    }
  }

  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, &env.state);
  });
}

void in(ISS& env, const bc::RetC& /*op*/) {
  auto const locEquiv = topStkLocal(env);
  doRet(env, popC(env), false);
  if (locEquiv != NoLocalId && locEquiv < env.ctx.func->params.size()) {
    env.flags.retParam = locEquiv;
  }
}
void in(ISS& env, const bc::RetV& /*op*/) {
  doRet(env, popV(env), false);
}
void in(ISS& env, const bc::RetM& op) {
  std::vector<Type> ret(op.arg1);
  for (int i = 0; i < op.arg1; i++) {
    ret[op.arg1 - i - 1] = popC(env);
  }
  doRet(env, vec(std::move(ret)), false);
}

void in(ISS& env, const bc::Unwind&) {
  nothrow(env); // Don't propagate to throw edges
  for (auto exit : env.blk.unwindExits) {
    auto const stackLess = without_stacks(env.state);
    env.propagate(exit, &stackLess);
  }
}

void in(ISS& env, const bc::Throw& /*op*/) {
  popC(env);
}

void in(ISS& env, const bc::Catch&) {
  nothrow(env);
  return push(env, subObj(env.index.builtin_class(s_Throwable.get())));
}

void in(ISS& env, const bc::ChainFaults&) {
  popC(env);
}

void in(ISS& env, const bc::NativeImpl&) {
  killLocals(env);
  mayUseVV(env);

  if (is_collection_method_returning_this(env.ctx.cls, env.ctx.func)) {
    assert(env.ctx.func->attrs & AttrParamCoerceModeNull);
    assert(!(env.ctx.func->attrs & AttrReference));
    auto const resCls = env.index.builtin_class(env.ctx.cls->name);
    // Can still return null if parameter coercion fails
    return doRet(env, union_of(objExact(resCls), TInitNull), true);
  }

  if (env.ctx.func->nativeInfo) {
    return doRet(env, native_function_return_type(env.ctx.func), true);
  }
  doRet(env, TInitGen, true);
}

void in(ISS& env, const bc::CGetL& op) {
  if (op.loc1 == env.state.thisLocToKill) {
    return reduce(env, bc::BareThis { BareThisOp::Notice });
  }
  if (!locCouldBeUninit(env, op.loc1)) {
    nothrow(env);
    constprop(env);
  }
  push(env, locAsCell(env, op.loc1), op.loc1);
}

void in(ISS& env, const bc::CGetQuietL& op) {
  if (op.loc1 == env.state.thisLocToKill) {
    return reduce(env, bc::BareThis { BareThisOp::NoNotice });
  }
  nothrow(env);
  constprop(env);
  push(env, locAsCell(env, op.loc1), op.loc1);
}

void in(ISS& env, const bc::CUGetL& op) {
  auto ty = locRaw(env, op.loc1);
  if (ty.subtypeOf(BUninit)) {
    return reduce(env, bc::NullUninit {});
  }
  nothrow(env);
  if (!ty.couldBe(BUninit)) constprop(env);
  if (!ty.subtypeOf(BCell)) ty = TCell;
  push(env, std::move(ty), op.loc1);
}

void in(ISS& env, const bc::PushL& op) {
  if (auto val = tv(locRaw(env, op.loc1))) {
    return reduce(env, gen_constant(*val), bc::UnsetL { op.loc1 });
  }
  impl(env, bc::CGetL { op.loc1 }, bc::UnsetL { op.loc1 });
}

void in(ISS& env, const bc::CGetL2& op) {
  // Can't constprop yet because of no INS_1 support in bc.h
  if (!locCouldBeUninit(env, op.loc1)) effect_free(env);
  auto loc = locAsCell(env, op.loc1);
  auto topEquiv = topStkLocal(env);
  auto top = popT(env);
  push(env, std::move(loc), op.loc1);
  push(env, std::move(top), topEquiv);
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
      auto const initMightRaise = classInitMightRaise(env, tcls);
      if (!initMightRaise) nothrow(env);

      // We can only constprop here if we know for sure this is exactly the
      // correct class.  The reason for this is that you could have a LSB class
      // attempting to access a private static in a derived class with the same
      // name as a private static in this class, which is supposed to fatal at
      // runtime (for an example see test/quick/static_sprop2.php).
      auto const selfExact = selfClsExact(env);
      if (selfExact && tcls.subtypeOf(*selfExact) && !initMightRaise) {
        constprop(env);
      }

      return push(env, std::move(*ty));
    }
  }

  auto indexTy = env.index.lookup_public_static(tcls, tname);
  if (indexTy.subtypeOf(BInitCell)) {
    /*
     * Constant propagation here can change when we invoke autoload, so it's
     * considered HardConstProp.  It's safe not to check anything about private
     * or protected static properties, because you can't override a public
     * static property with a private or protected one---if the index gave us
     * back a constant type, it's because it found a public static and it must
     * be the property this would have read dynamically.
     */
    if (options.HardConstProp && !classInitMightRaise(env, tcls)) {
      constprop(env);
    }
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
    if (t1.subtypeOf(BObj)) {
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
  if (op.loc1 == env.state.thisLocToKill) {
    return reduce(env,
                  bc::BareThis { BareThisOp::Notice },
                  bc::ClsRefGetC { op.slot });
  }
  clsRefGetImpl(env, locAsCell(env, op.loc1), op.slot);
}
void in(ISS& env, const bc::ClsRefGetC& op) {
  clsRefGetImpl(env, popC(env), op.slot);
}

void in(ISS& env, const bc::AKExists& /*op*/) {
  auto const base = popC(env);
  auto const key  = popC(env);

  // Bases other than array-like or object will raise a warning and return
  // false.
  if (!base.couldBeAny(TArr, TVec, TDict, TKeyset, TObj)) {
    return push(env, TFalse);
  }

  // Push the returned type and annotate effects appropriately, taking into
  // account if the base might be null. Allowing for a possibly null base lets
  // us capture more cases.
  auto const finish = [&] (const Type& t, bool mayThrow) {
    if (base.couldBe(BInitNull)) return push(env, union_of(t, TFalse));
    if (!mayThrow) {
      constprop(env);
      effect_free(env);
    }
    if (base.subtypeOf(BBottom)) unreachable(env);
    return push(env, t);
  };

  // Helper for Hack arrays. "validKey" is the set of key types which can return
  // a value from AKExists. "silentKey" is the set of key types which will
  // silently return false (anything else throws). The Hack array elem functions
  // will treat values of "silentKey" as throwing, so we must identify those
  // cases and deal with them.
  auto const hackArr = [&] (std::pair<Type, ThrowMode> elem,
                            const Type& validKey,
                            const Type& silentKey) {
    switch (elem.second) {
      case ThrowMode::None:
        assertx(key.subtypeOf(validKey));
        return finish(TTrue, false);
      case ThrowMode::MaybeMissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(TBool, false);
      case ThrowMode::MissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(TFalse, false);
      case ThrowMode::MaybeBadKey:
        assertx(key.couldBe(validKey));
        return finish(
          elem.first.subtypeOf(BBottom) ? TFalse : TBool,
          !key.subtypeOf(BOptArrKey)
        );
      case ThrowMode::BadOperation:
        assertx(!key.couldBe(validKey));
        return finish(key.couldBe(silentKey) ? TFalse : TBottom, true);
    }
  };

  // Vecs will throw for any key other than Int, Str, or Null, and will silently
  // return false for the latter two.
  if (base.subtypeOrNull(BVec)) {
    if (key.subtypeOrNull(BStr)) return finish(TFalse, false);
    return hackArr(vec_elem(base, key, TBottom), TInt, TOptStr);
  }

  // Dicts and keysets will throw for any key other than Int, Str, or Null,
  // and will silently return false for Null.
  if (base.subtypeOfAny(TOptDict, TOptKeyset)) {
    if (key.subtypeOf(BInitNull)) return finish(TFalse, false);
    auto const elem = base.subtypeOrNull(BDict)
      ? dict_elem(base, key, TBottom)
      : keyset_elem(base, key, TBottom);
    return hackArr(elem, TArrKey, TInitNull);
  }

  if (base.subtypeOrNull(BArr)) {
    // Unlike Idx, AKExists will transform a null key on arrays into the static
    // empty string, so we don't need to do any fixups here.
    auto const elem = array_elem(base, key, TBottom);
    switch (elem.second) {
      case ThrowMode::None:                return finish(TTrue, false);
      case ThrowMode::MaybeMissingElement: return finish(TBool, false);
      case ThrowMode::MissingElement:      return finish(TFalse, false);
      case ThrowMode::MaybeBadKey:
        return finish(elem.first.subtypeOf(BBottom) ? TFalse : TBool, true);
      case ThrowMode::BadOperation:        always_assert(false);
    }
  }

  // Objects or other unions of possible bases
  push(env, TBool);
}

void in(ISS& env, const bc::GetMemoKeyL& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);

  auto const rclsIMemoizeParam = env.index.builtin_class(s_IMemoizeParam.get());
  auto const tyIMemoizeParam = subObj(rclsIMemoizeParam);

  auto const inTy = locAsCell(env, op.loc1);

  // If the local could be uninit, we might raise a warning (as
  // usual). Converting an object to a memo key might invoke PHP code if it has
  // the IMemoizeParam interface, and if it doesn't, we'll throw.
  if (!locCouldBeUninit(env, op.loc1) &&
      !inTy.couldBeAny(TObj, TArr, TVec, TDict)) {
    nothrow(env); constprop(env);
  }

  // If type constraints are being enforced and the local being turned into a
  // memo key is a parameter, then we can possibly using the type constraint to
  // infer a more efficient memo key mode.
  using MK = MemoKeyConstraint;
  folly::Optional<res::Class> resolvedCls;
  auto const mkc = [&] {
    if (!RuntimeOption::EvalHardTypeHints) return MK::None;
    if (op.loc1 >= env.ctx.func->params.size()) return MK::None;
    auto tc = env.ctx.func->params[op.loc1].typeConstraint;
    if (tc.type() == AnnotType::Object) {
      auto res = env.index.resolve_type_name(tc.typeName());
      if (res.type != AnnotType::Object) {
        tc.resolveType(res.type, res.nullable || tc.isNullable());
      } else {
        resolvedCls = env.index.resolve_class(env.ctx, tc.typeName());
      }
    }
    return memoKeyConstraintFromTC(tc);
  }();

  // Use the type-constraint to reduce this operation to a more efficient memo
  // mode. Some of the modes can be reduced to simple bytecode operations
  // inline. Even with the type-constraints, we still need to check the inferred
  // type of the local. Something may have possibly clobbered the local between
  // the type-check and this op.
  switch (mkc) {
    case MK::Int:
      // Always an int, so the key is always an identity mapping
      if (inTy.subtypeOf(BInt)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::Bool:
      // Always a bool, so the key is the bool cast to an int
      if (inTy.subtypeOf(BBool)) {
        return reduce(env, bc::CGetL { op.loc1 }, bc::CastInt {});
      }
      break;
    case MK::Str:
      // Always a string, so the key is always an identity mapping
      if (inTy.subtypeOf(BStr)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::IntOrStr:
      // Either an int or string, so the key can be an identity mapping
      if (inTy.subtypeOf(BArrKey)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::StrOrNull:
      // A nullable string. The key will either be the string or the integer
      // zero.
      if (inTy.subtypeOrNull(BStr)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::Int { 0 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::IntOrNull:
      // A nullable int. The key will either be the integer, or the static empty
      // string.
      if (inTy.subtypeOrNull(BInt)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::String { staticEmptyString() },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::BoolOrNull:
      // A nullable bool. The key will either be 0, 1, or 2.
      if (inTy.subtypeOrNull(BBool)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::CastInt {},
          bc::Int { 2 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::Dbl:
      // The double will be converted (losslessly) to an integer.
      if (inTy.subtypeOf(BDbl)) {
        return reduce(env, bc::CGetL { op.loc1 }, bc::DblAsBits {});
      }
      break;
    case MK::DblOrNull:
      // A nullable double. The key will be an integer, or the static empty
      // string.
      if (inTy.subtypeOrNull(BDbl)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::DblAsBits {},
          bc::String { staticEmptyString() },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::Object:
      // An object. If the object is definitely known to implement IMemoizeParam
      // we can simply call that method, casting the output to ensure its always
      // a string (which is what the generic mode does). If not, it will use the
      // generic mode, which can handle collections or classes which don't
      // implement getInstanceKey.
      if (resolvedCls &&
          resolvedCls->subtypeOf(rclsIMemoizeParam) &&
          inTy.subtypeOf(tyIMemoizeParam)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::FPushObjMethodD {
            0,
            s_getInstanceKey.get(),
            ObjMethodOp::NullThrows,
            false
          },
          bc::FCall { 0, 0, 1, staticEmptyString(), staticEmptyString() },
          bc::UnboxR {},
          bc::CastString {}
        );
      }
      break;
    case MK::ObjectOrNull:
      // An object or null. We can use the null safe version of a function call
      // when invoking getInstanceKey and then select from the result of that,
      // or the integer 0. This might seem wasteful, but the JIT does a good job
      // inlining away the call in the null case.
      if (resolvedCls &&
          resolvedCls->subtypeOf(rclsIMemoizeParam) &&
          inTy.subtypeOf(opt(tyIMemoizeParam))) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::FPushObjMethodD {
            0,
            s_getInstanceKey.get(),
            ObjMethodOp::NullSafe,
            false
          },
          bc::FCall { 0, 0, 1, staticEmptyString(), staticEmptyString() },
          bc::UnboxR {},
          bc::CastString {},
          bc::Int { 0 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::None:
      break;
  }

  // No type constraint, or one that isn't usuable. Use the generic memoization
  // scheme which can handle any type:

  if (auto const val = tv(inTy)) {
    auto const key = eval_cell(
      [&]{ return HHVM_FN(serialize_memoize_param)(*val); }
    );
    if (key) return push(env, *key);
  }

  // Integer keys are always mapped to themselves
  if (inTy.subtypeOf(BInt)) return reduce(env, bc::CGetL { op.loc1 });
  if (inTy.subtypeOrNull(BInt)) {
    return reduce(
      env,
      bc::CGetL { op.loc1 },
      bc::String { s_nullMemoKey.get() },
      bc::IsTypeL { op.loc1, IsTypeOp::Null },
      bc::Select {}
    );
  }
  if (inTy.subtypeOf(BBool)) {
    return reduce(
      env,
      bc::String { s_falseMemoKey.get() },
      bc::String { s_trueMemoKey.get() },
      bc::CGetL { op.loc1 },
      bc::Select {}
    );
  }

  // A memo key can be an integer if the input might be an integer, and is a
  // string otherwise. Booleans and nulls are always static strings.
  auto keyTy = [&]{
    if (inTy.subtypeOrNull(BBool)) return TSStr;
    if (inTy.couldBe(BInt))        return union_of(TInt, TStr);
    return TStr;
  }();
  push(env, std::move(keyTy));
}

void in(ISS& env, const bc::IssetL& op) {
  if (op.loc1 == env.state.thisLocToKill) {
    return reduce(env,
                  bc::BareThis { BareThisOp::NoNotice },
                  bc::IsTypeC { IsTypeOp::Null },
                  bc::Not {});
  }
  nothrow(env);
  constprop(env);
  auto const loc = locAsCell(env, op.loc1);
  if (loc.subtypeOf(BNull))  return push(env, TFalse);
  if (!loc.couldBe(BNull))   return push(env, TTrue);
  push(env, TBool);
}

void in(ISS& env, const bc::EmptyL& op) {
  nothrow(env);
  constprop(env);
  castBoolImpl(env, locAsCell(env, op.loc1), true);
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
      if (t->subtypeOf(BNull)) {
        if (!classInitMightRaise(env, tcls)) constprop(env);
        return push(env, TFalse);
      }
      if (!t->couldBe(BNull)) {
        if (!classInitMightRaise(env, tcls)) constprop(env);
        return push(env, TTrue);
      }
    }
  }

  auto const indexTy = env.index.lookup_public_static(tcls, tname);
  if (indexTy.subtypeOf(BInitCell)) {
    // See the comments in CGetS about constprop for public statics.
    if (options.HardConstProp && !classInitMightRaise(env, tcls)) {
      constprop(env);
    }
    if (indexTy.subtypeOf(BNull))  { return push(env, TFalse); }
    if (!indexTy.couldBe(BNull))   { return push(env, TTrue); }
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
  if (!is_type_might_raise(test, locOrCell)) constprop(env);
  if (locOrCell.subtypeOf(test))  return push(env, TTrue);
  if (!locOrCell.couldBe(test))   return push(env, TFalse);
  push(env, TBool);
}

void isTypeObj(ISS& env, const Type& ty) {
  if (!ty.couldBe(BObj)) return push(env, TFalse);
  if (ty.subtypeOf(BObj)) {
    auto const incompl = objExact(
      env.index.builtin_class(s_PHP_Incomplete_Class.get()));
    if (!ty.couldBe(incompl))  return push(env, TTrue);
    if (ty.subtypeOf(incompl)) return push(env, TFalse);
  }
  push(env, TBool);
}

void isTypeArrLike(ISS& env, const Type& ty) {
  if (ty.subtypeOfAny(TArr, TVec, TDict, TKeyset)) return push(env, TTrue);
  if (!ty.couldBeAny(TArr, TVec, TDict, TKeyset)) return push(env, TFalse);
  push(env, TBool);
}

template<class Op>
void isTypeLImpl(ISS& env, const Op& op) {
  if (!locCouldBeUninit(env, op.loc1)) { nothrow(env); constprop(env); }
  auto const loc = locAsCell(env, op.loc1);
  switch (op.subop2) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, loc);
  case IsTypeOp::ArrLike: return isTypeArrLike(env, loc);
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
  case IsTypeOp::ArrLike: return isTypeArrLike(env, t1);
  default: return isTypeImpl(env, t1, type_of_istype(op.subop1));
  }
}

void in(ISS& env, const bc::IsTypeC& op) { isTypeCImpl(env, op); }
void in(ISS& env, const bc::IsTypeL& op) { isTypeLImpl(env, op); }

void in(ISS& env, const bc::InstanceOfD& op) {
  auto t1 = topC(env);
  // Note: InstanceOfD can do autoload if the type might be a type
  // alias, so it's not nothrow unless we know it's an object type.
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str1)) {
    auto result = [&] (const Type& r) {
      nothrow(env);
      if (r != TBool) constprop(env);
      popC(env);
      push(env, r);
    };
    if (!interface_supports_non_objects(rcls->name())) {
      auto testTy = subObj(*rcls);
      if (t1.subtypeOf(testTy)) return result(TTrue);
      if (!t1.couldBe(testTy)) return result(TFalse);
      if (is_opt(t1)) {
        t1 = unopt(std::move(t1));
        if (t1.subtypeOf(testTy)) {
          return reduce(env, bc::IsTypeC { IsTypeOp::Null }, bc::Not {});
        }
      }
      return result(TBool);
    }
  }
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::InstanceOf& /*op*/) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    return reduce(env, bc::PopC {},
                       bc::InstanceOfD { v1->m_data.pstr });
  }

  if (t1.subtypeOf(BObj) && is_specialized_obj(t1)) {
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

namespace {

bool isValidTypeOpForIsAs(const IsTypeOp& op) {
  switch (op) {
    case IsTypeOp::Null:
    case IsTypeOp::Bool:
    case IsTypeOp::Int:
    case IsTypeOp::Dbl:
    case IsTypeOp::Str:
    case IsTypeOp::Obj:
      return true;
    case IsTypeOp::Res:
    case IsTypeOp::Arr:
    case IsTypeOp::Vec:
    case IsTypeOp::Dict:
    case IsTypeOp::Keyset:
    case IsTypeOp::VArray:
    case IsTypeOp::DArray:
    case IsTypeOp::ArrLike:
    case IsTypeOp::Scalar:
      return false;
  }
  not_reached();
}

template<bool asExpression>
void isAsTypeStructImpl(ISS& env, SArray ts) {
  auto const t = topC(env);

  auto result = [&] (
    const Type& out,
    const folly::Optional<Type>& test = folly::none
  ) {
    auto const location = topStkEquiv(env);
    popC(env);
    if (!asExpression) {
      constprop(env);
      return push(env, out);
    }
    if (out.subtypeOf(BTrue)) {
      constprop(env);
      push(env, t);
      return reduce(env, bc::Nop {});
    }
    if (out.subtypeOf(BFalse)) {
      push(env, t);
      return unreachable(env);
    }

    assertx(out == TBool);
    if (!test) return push(env, t);
    auto const newT = intersection_of(*test, t);
    if (newT == TBottom || !refineLocation(env, location, [&] (Type t) {
          auto ret = intersection_of(*test, t);
          if (test->couldBe(BInitNull) && t.couldBe(BUninit)) {
            ret |= TUninit;
          }
          return ret;
        })) {
      unreachable(env);
    }
    return push(env, newT);
  };

  auto check = [&] (
    const folly::Optional<Type> type,
    const folly::Optional<Type> deopt = folly::none
  ) {
    if (!type) return result(TBool);
    auto const test = type.value();
    if (t.subtypeOf(test)) return result(TTrue);
    if (!t.couldBe(test) && (!deopt || !t.couldBe(deopt.value()))) {
      return result(TFalse);
    }
    auto const op = type_to_istypeop(test);
    if (asExpression || !op || !isValidTypeOpForIsAs(op.value())) {
      return result(TBool, test);
    }
    return reduce(env, bc::IsTypeC { *op });
  };

  auto const is_nullable_ts = is_ts_nullable(ts);
  auto const is_definitely_null = t.subtypeOf(BNull);
  auto const is_definitely_not_null = !t.couldBe(BNull);

  if (is_nullable_ts && is_definitely_null) return result(TTrue);

  auto const ts_type = type_of_type_structure(ts);

  if (is_nullable_ts && !is_definitely_not_null && ts_type == folly::none) {
    // Ts is nullable and we know that t could be null but we dont know for sure
    // Also we didn't get a type out of the type structure
    return result(TBool);
  }

  if (!asExpression) {
    if (ts_type && !is_type_might_raise(*ts_type, t)) nothrow(env);
    constprop(env);
  }
  switch (get_ts_kind(ts)) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_void:
      return check(ts_type);
    case TypeStructure::Kind::T_tuple:
      return RuntimeOption::EvalHackArrCompatIsArrayNotices
        ? check(ts_type, TDArr)
        : check(ts_type);
    case TypeStructure::Kind::T_shape:
      return RuntimeOption::EvalHackArrCompatIsArrayNotices
        ? check(ts_type, TVArr)
        : check(ts_type);
    case TypeStructure::Kind::T_dict:
      return check(ts_type, TDArr);
    case TypeStructure::Kind::T_vec:
      return check(ts_type, TVArr);
    case TypeStructure::Kind::T_noreturn:
      return result(TFalse);
    case TypeStructure::Kind::T_mixed:
      return result(TTrue);
    case TypeStructure::Kind::T_nonnull:
      if (is_definitely_null) return result(TFalse);
      if (is_definitely_not_null) return result(TTrue);
      if (!asExpression) {
        return reduce(env, bc::IsTypeC { IsTypeOp::Null }, bc::Not {});
      }
      return result(TBool);
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp:
      if (asExpression) return result(TBool);
      return reduce(env, bc::InstanceOfD { get_ts_classname(ts) });
    case TypeStructure::Kind::T_unresolved: {
      if (asExpression) return result(TBool);
      auto const rcls = env.index.resolve_class(env.ctx, get_ts_classname(ts));
      // We can only reduce to instance of if we know for sure that this class
      // can be resolved since instanceof undefined class does not throw
      if (!rcls || !rcls->resolved() || rcls->cls()->attrs & AttrEnum) {
        return result(TBool);
      }
      return reduce(env, bc::InstanceOfD { rcls->name() });
    }
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
      // TODO(T29232862): implement
      return result(TBool);
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
      return result(TBool);
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
      // We will error on these at the JIT
      return result(TBool);
  }

  not_reached();
}

}

void in(ISS& env, const bc::IsTypeStruct& op) {
  assertx(op.arr1->isDictOrDArray());
  isAsTypeStructImpl<false>(env, op.arr1);
}

void in(ISS& env, const bc::AsTypeStruct& op) {
  assertx(op.arr1->isDictOrDArray());
  isAsTypeStructImpl<true>(env, op.arr1);
}

namespace {

/*
 * If the value on the top of the stack is known to be equivalent to the local
 * its being moved/copied to, return folly::none without modifying any
 * state. Otherwise, pop the stack value, perform the set, and return a pair
 * giving the value's type, and any other local its known to be equivalent to.
 */
template <typename Op>
folly::Optional<std::pair<Type, LocalId>> moveToLocImpl(ISS& env,
                                                        const Op& op) {
  nothrow(env);
  auto equivLoc = topStkLocal(env);
  // If the local could be a Ref, don't record equality because the stack
  // element and the local won't actually have the same type.
  if (!locCouldBeRef(env, op.loc1)) {
    assertx(!is_volatile_local(env.ctx.func, op.loc1));
    if (equivLoc != NoLocalId) {
      if (equivLoc == op.loc1 ||
          locsAreEquiv(env, equivLoc, op.loc1)) {
        // We allow equivalency to ignore Uninit, so we need to check
        // the types here.
        if (peekLocRaw(env, op.loc1) == topC(env)) {
          return folly::none;
        }
      }
    } else {
      equivLoc = op.loc1;
    }
  } else {
    equivLoc = NoLocalId;
  }
  auto val = popC(env);
  setLoc(env, op.loc1, val);
  if (equivLoc != op.loc1 && equivLoc != NoLocalId) {
    addLocEquiv(env, op.loc1, equivLoc);
  }
  return { std::make_pair(std::move(val), equivLoc) };
}

}

void in(ISS& env, const bc::PopL& op) {
  // If the same value is already in the local, do nothing but pop
  // it. Otherwise, the set has been done by moveToLocImpl.
  if (!moveToLocImpl(env, op)) return reduce(env, bc::PopC {});
}

void in(ISS& env, const bc::SetL& op) {
  // If the same value is already in the local, do nothing because SetL keeps
  // the value on the stack. If it isn't, we need to push it back onto the stack
  // because moveToLocImpl popped it.
  if (auto p = moveToLocImpl(env, op)) {
    push(env, std::move(p->first), p->second);
  } else {
    reduce(env, bc::Nop {});
  }
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
      if (!classInitMightRaise(env, tcls)) nothrow(env);
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
    if (resultTy->subtypeOf(BStr)) resultTy = TStr;
    else if (resultTy->subtypeOf(BArr)) resultTy = TArr;
    else if (resultTy->subtypeOf(BVec)) resultTy = TVec;
    else if (resultTy->subtypeOf(BDict)) resultTy = TDict;
    else if (resultTy->subtypeOf(BKeyset)) resultTy = TKeyset;

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
      !loc.couldBe(BStr)) {
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
  if (!t1.couldBe(BObj | BRes)) nothrow(env);
  unsetUnknownLocal(env);
  mayUseVV(env);
}

void in(ISS& env, const bc::UnsetG& /*op*/) {
  auto const t1 = popC(env);
  if (!t1.couldBe(BObj | BRes)) nothrow(env);
}

void in(ISS& env, const bc::FPushFuncD& op) {
  auto const rfunc = env.index.resolve_func(env.ctx, op.str2);
  if (auto const func = rfunc.exactFunc()) {
    if (can_emit_builtin(func, op.arg1, op.has_unpack)) {
      fpiPush(
        env,
        ActRec { FPIKind::Builtin, TBottom, folly::none, rfunc },
        op.arg1,
        false
      );
      return reduce(env, bc::Nop {});
    }
  }
  if (fpiPush(env, ActRec { FPIKind::Func, TBottom, folly::none, rfunc },
              op.arg1, false)) {
    return reduce(env, bc::Nop {});
  }
}

void in(ISS& env, const bc::FPushFunc& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  folly::Optional<res::Func> rfunc;
  // FPushFuncD and FPushFuncU require that the names of inout functions be
  // mangled, so skip those for now.
  if (v1 && v1->m_type == KindOfPersistentString && op.argv.size() == 0) {
    auto const name = normalizeNS(v1->m_data.pstr);
    // FPushFuncD doesn't support class-method pair strings yet.
    if (isNSNormalized(name) && notClassMethodPair(name)) {
      rfunc = env.index.resolve_func(env.ctx, name);
      // If the function might distinguish being called dynamically from not,
      // don't turn a dynamic call into a static one.
      if (!rfunc->mightCareAboutDynCalls()) {
        return reduce(env, bc::PopC {},
                      bc::FPushFuncD { op.arg1, name, op.has_unpack });
      }
    }
  }
  popC(env);
  if (t1.subtypeOf(BObj)) {
    return fpiPushNoFold(env, ActRec { FPIKind::ObjInvoke, t1 });
  }
  if (t1.subtypeOf(BArr)) {
    return fpiPushNoFold(env, ActRec { FPIKind::CallableArr, TTop });
  }
  if (t1.subtypeOf(BStr)) {
    fpiPush(
      env,
      ActRec { FPIKind::Func, TTop, folly::none, rfunc },
      op.arg1,
      true);
    return;
  }
  fpiPushNoFold(env, ActRec { FPIKind::Unknown, TTop });
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
  fpiPushNoFold(
    env,
    ActRec {
      FPIKind::Func,
      TBottom,
      folly::none,
      rfuncPair.first,
      rfuncPair.second
    }
  );
}

void in(ISS& env, const bc::ResolveFunc& op) {
  // TODO (T29639296)
  push(env, TFunc);
}

void in(ISS& env, const bc::ResolveObjMethod& op) {
  // TODO (T29639296)
  popC(env);
  popC(env);
  push(env, TVArr);
}

void in(ISS& env, const bc::ResolveClsMethod& op) {
  // TODO (T29639296)
  popC(env);
  popC(env);
  push(env, TVArr);
}

const StaticString s_nullFunc { "__SystemLib\\__86null" };

void in(ISS& env, const bc::FPushObjMethodD& op) {
  auto const nullThrows = op.subop3 == ObjMethodOp::NullThrows;
  auto const input = topC(env);
  auto const mayCallMethod = input.couldBe(BObj);
  auto const mayCallNullsafe = !nullThrows && input.couldBe(BNull);
  auto const mayThrowNonObj = !input.subtypeOf(nullThrows ? TObj : TOptObj);

  if (!mayCallMethod && !mayCallNullsafe) {
    // This FPush may only throw, make sure it's not optimized away.
    fpiPushNoFold(env, ActRec { FPIKind::ObjMeth, TBottom });
    popC(env);
    return unreachable(env);
  }

  if (!mayCallMethod && !mayThrowNonObj) {
    // Null input, this may only call the nullsafe helper, so do that.
    return reduce(
      env,
      bc::PopC {},
      bc::FPushFuncD { op.arg1, s_nullFunc.get(), op.has_unpack }
    );
  }

  auto const ar = [&] {
    assertx(mayCallMethod);
    auto const kind = mayCallNullsafe ? FPIKind::ObjMethNS : FPIKind::ObjMeth;
    auto const ctxTy = intersection_of(input, TObj);
    auto const clsTy = objcls(ctxTy);
    auto const rcls = is_specialized_cls(clsTy)
      ? folly::Optional<res::Class>(dcls_of(clsTy).cls)
      : folly::none;
    auto const func = env.index.resolve_method(env.ctx, clsTy, op.str2);
    return ActRec { kind, ctxTy, rcls, func };
  };

  if (!mayCallMethod) {
    // Calls nullsafe helper, but can't fold as we may still throw.
    assertx(mayCallNullsafe && mayThrowNonObj);
    auto const func = env.index.resolve_func(env.ctx, s_nullFunc.get());
    assertx(func.exactFunc());
    fpiPushNoFold(env, ActRec { FPIKind::Func, TBottom, folly::none, func });
  } else if (mayCallNullsafe || mayThrowNonObj) {
    // Can't optimize away as FCall may push null instead of the folded value
    // or FCall may throw.
    fpiPushNoFold(env, ar());
  } else if (fpiPush(env, ar(), op.arg1, false)) {
    return reduce(env, bc::PopC {});
  }

  auto const location = topStkEquiv(env);
  if (location != NoLocalId) {
    if (!refineLocation(env, location, [&] (Type t) {
      if (nullThrows) return intersection_of(t, TObj);
      if (!t.couldBe(BUninit)) return intersection_of(t, TOptObj);
      if (!t.couldBe(BObj)) return intersection_of(t, TNull);
      return t;
    })) {
      unreachable(env);
    }
  }

  popC(env);
}

void in(ISS& env, const bc::FPushObjMethod& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  auto const clsTy = objcls(t1);
  folly::Optional<res::Func> rfunc;
  if (v1 && v1->m_type == KindOfPersistentString && op.argv.size() == 0) {
    rfunc = env.index.resolve_method(env.ctx, clsTy, v1->m_data.pstr);
    if (!rfunc->mightCareAboutDynCalls()) {
      return reduce(
        env,
        bc::PopC {},
        bc::FPushObjMethodD {
          op.arg1, v1->m_data.pstr, op.subop2, op.has_unpack
        }
      );
    }
  }
  popC(env);
  fpiPush(
    env,
    ActRec {
      FPIKind::ObjMeth,
      popC(env),
      is_specialized_cls(clsTy)
        ? folly::Optional<res::Class>(dcls_of(clsTy).cls)
        : folly::none,
      rfunc
    },
    op.arg1,
    true
  );
}

void in(ISS& env, const bc::FPushClsMethodD& op) {
  auto const rcls = env.index.resolve_class(env.ctx, op.str3);
  auto clsType = rcls ? clsExact(*rcls) : TCls;
  auto const rfun = env.index.resolve_method(
    env.ctx,
    clsType,
    op.str2
  );
  if (fpiPush(env, ActRec { FPIKind::ClsMeth, clsType, rcls, rfun }, op.arg1,
              false)) {
    return reduce(env, bc::Nop {});
  }
}

namespace {

Type ctxCls(ISS& env) {
  auto const s = selfCls(env);
  return setctx(s ? *s : TCls);
}

Type specialClsRefToCls(ISS& env, SpecialClsRef ref) {
  if (!env.ctx.cls) return TCls;
  auto const op = [&]()-> folly::Optional<Type> {
    switch (ref) {
      case SpecialClsRef::Static: return ctxCls(env);
      case SpecialClsRef::Self:   return selfClsExact(env);
      case SpecialClsRef::Parent: return parentClsExact(env);
    }
    always_assert(false);
  }();
  return op ? *op : TCls;
}

}

void in(ISS& env, const bc::FPushClsMethod& op) {
  auto const t1 = peekClsRefSlot(env, op.slot);
  auto const t2 = topC(env);
  auto const v2 = tv(t2);

  folly::Optional<res::Class> rcls;
  auto exactCls = false;
  if (is_specialized_cls(t1)) {
    auto dcls = dcls_of(t1);
    rcls = dcls.cls;
    exactCls = dcls.type == DCls::Exact;
  }
  folly::Optional<res::Func> rfunc;
  if (v2 && v2->m_type == KindOfPersistentString && op.argv.size() == 0) {
    rfunc = env.index.resolve_method(env.ctx, t1, v2->m_data.pstr);
    if (exactCls && rcls && !rfunc->mightCareAboutDynCalls()) {
      return reduce(
        env,
        bc::DiscardClsRef { op.slot },
        bc::PopC {},
        bc::FPushClsMethodD {
          op.arg1, v2->m_data.pstr, rcls->name(), op.has_unpack
        }
      );
    }
  }
  if (fpiPush(env, ActRec { FPIKind::ClsMeth, t1, rcls, rfunc }, op.arg1,
              true)) {
    return reduce(env,
                  bc::DiscardClsRef { op.slot },
                  bc::PopC {});
  }
  takeClsRefSlot(env, op.slot);
  popC(env);
}

void in(ISS& env, const bc::FPushClsMethodS& op) {
  auto const name  = topC(env);
  auto const namev = tv(name);
  auto const cls = specialClsRefToCls(env, op.subop2);
  folly::Optional<res::Func> rfunc;
  if (namev && namev->m_type == KindOfPersistentString && op.argv.size() == 0) {
    rfunc = env.index.resolve_method(env.ctx, cls, namev->m_data.pstr);
    if (!rfunc->mightCareAboutDynCalls()) {
      return reduce(
        env,
        bc::PopC {},
        bc::FPushClsMethodSD {
          op.arg1, op.subop2, namev->m_data.pstr, op.has_unpack
        }
      );
    }
  }
  auto const rcls = is_specialized_cls(cls)
    ? folly::Optional<res::Class>{dcls_of(cls).cls}
    : folly::none;
  if (fpiPush(env, ActRec {
                FPIKind::ClsMeth,
                ctxCls(env),
                rcls,
                rfunc
              }, op.arg1, true)) {
    return reduce(env, bc::PopC {});
  }
  popC(env);
}

void in(ISS& env, const bc::FPushClsMethodSD& op) {
  auto const cls = specialClsRefToCls(env, op.subop2);

  folly::Optional<res::Class> rcls;
  auto exactCls = false;
  if (is_specialized_cls(cls)) {
    auto dcls = dcls_of(cls);
    rcls = dcls.cls;
    exactCls = dcls.type == DCls::Exact;
  }

  if (op.subop2 == SpecialClsRef::Static && rcls && exactCls) {
    return reduce(
      env,
      bc::FPushClsMethodD {
        op.arg1, op.str3, rcls->name(), op.has_unpack
      }
    );
  }

  auto const rfun = env.index.resolve_method(env.ctx, cls, op.str3);
  if (fpiPush(env, ActRec {
                FPIKind::ClsMeth,
                ctxCls(env),
                rcls,
                rfun
              }, op.arg1, false)) {
    return reduce(env, bc::Nop {});
  }
}

void ctorHelper(ISS& env, SString name, int32_t nargs) {
  auto const rcls = env.index.resolve_class(env.ctx, name);
  auto const rfunc = rcls ?
    env.index.resolve_ctor(env.ctx, *rcls, true) : folly::none;
  auto ctxType = false;
  if (rcls && env.ctx.cls && rcls->same(env.index.resolve_class(env.ctx.cls)) &&
      !rcls->couldBeOverriden()) {
    ctxType = true;
  }
  fpiPush(
    env,
    ActRec {
      FPIKind::Ctor,
      setctx(rcls ? clsExact(*rcls) : TCls, ctxType),
      rcls,
      rfunc
    },
    nargs,
    false
  );
  push(env, setctx(rcls ? objExact(*rcls) : TObj, ctxType));
}

void in(ISS& env, const bc::FPushCtorD& op) {
  ctorHelper(env, op.str2, op.arg1);
}

void in(ISS& env, const bc::FPushCtorI& op) {
  auto const name = env.ctx.unit->classes[op.arg2]->name;
  ctorHelper(env, name, op.arg1);
}

void in(ISS& env, const bc::FPushCtorS& op) {
  auto const cls = specialClsRefToCls(env, op.subop2);
  if (is_specialized_cls(cls)) {
    auto const dcls = dcls_of(cls);
    if (dcls.type == DCls::Exact
        && (!dcls.cls.couldBeOverriden()
            || equivalently_refined(cls, unctx(cls)))) {
      return reduce(
        env,
        bc::FPushCtorD { op.arg1, dcls.cls.name(), op.has_unpack }
      );
    }
    auto const rfunc = env.index.resolve_ctor(env.ctx, dcls.cls, false);
    push(env, toobj(cls));
    // PHP doesn't forward the context to constructors.
    fpiPush(env, ActRec { FPIKind::Ctor, unctx(cls), dcls.cls, rfunc },
            op.arg1,
            false);
    return;
  }
  push(env, TObj);
  fpiPush(env, ActRec { FPIKind::Ctor, TCls }, op.arg1, false);
}

void in(ISS& env, const bc::FPushCtor& op) {
  auto const& t1 = peekClsRefSlot(env, op.slot);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    auto const rfunc = env.index.resolve_ctor(env.ctx, dcls.cls, false);
    if (dcls.type == DCls::Exact && rfunc && !rfunc->mightCareAboutDynCalls()) {
      return reduce(env, bc::DiscardClsRef { op.slot },
                    bc::FPushCtorD { op.arg1, dcls.cls.name(), op.has_unpack });
    }

    auto const& t2 = takeClsRefSlot(env, op.slot);
    push(env, toobj(t2));
    fpiPushNoFold(env, ActRec { FPIKind::Ctor, t2, dcls.cls, rfunc });
    return;
  }
  takeClsRefSlot(env, op.slot);
  push(env, TObj);
  fpiPushNoFold(env, ActRec { FPIKind::Ctor, TCls });
}

void in(ISS& env, const bc::FPushCufIter&) {
  nothrow(env);
  fpiPushNoFold(env, ActRec { FPIKind::Unknown, TTop });
}

void in(ISS& env, const bc::FIsParamByRef& op) {
  auto& ar = fpiTop(env);
  auto const kind = ar.func && !ar.fallbackFunc
    ? env.index.lookup_param_prep(env.ctx, *ar.func, op.arg1)
    : PrepKind::Unknown;

  auto makeFuncName = [&]() -> SString {
    if (!ar.cls) return ar.func->name();
    return makeStaticString(
      folly::sformat("{}::{}", ar.cls->name()->data(), ar.func->name()->data())
    );
  };

  switch (kind) {
  case PrepKind::Unknown:
    if (ar.foldable) {
      fpiNotFoldable(env);
    }
    if (op.subop2 == FPassHint::Any) {
      nothrow(env);
    }
    return push(env, TBool);
  case PrepKind::Val:
    if (op.subop2 != FPassHint::Ref) {
      return reduce(env, bc::False {});
    } else {
      auto const funcName = makeFuncName();
      auto const hrm = bc::FHandleRefMismatch { op.arg1, op.subop2, funcName };
      return reduce(env, hrm, bc::False {});
    }
  case PrepKind::Ref:
    if (op.subop2 != FPassHint::Cell) {
      return reduce(env, bc::True {});
    } else {
      auto const funcName = makeFuncName();
      auto const hrm = bc::FHandleRefMismatch { op.arg1, op.subop2, funcName };
      return reduce(env, hrm, bc::True {});
    }
  }
}

void in(ISS& env, const bc::FIsParamByRefCufIter& op) {
  if (op.subop2 == FPassHint::Any) {
    nothrow(env);
  }
  return push(env, TBool);
}

void in(ISS& env, const bc::FThrowOnRefMismatch& op) {
  auto& ar = fpiTop(env);
  if (!ar.func || ar.fallbackFunc) return;

  for (auto i = 0; i < op.argv.size(); ++i) {
    auto const kind = env.index.lookup_param_prep(env.ctx, *ar.func, i);
    if (ar.foldable && kind != PrepKind::Val) {
      fpiNotFoldable(env);
      return;
    }
    if (kind == PrepKind::Unknown) return;

    if (kind != (op.argv[i] ? PrepKind::Ref : PrepKind::Val)) {
      auto const exCls = makeStaticString("InvalidArgumentException");
      auto const err = makeStaticString(formatParamRefMismatch(
        ar.func->name()->data(), i, !op.argv[i]));

      return reduce(
        env,
        bc::FPushCtorD { 1, exCls, false },
        bc::String { err },
        bc::FCall { 1, 0, 1, staticEmptyString(), staticEmptyString() },
        bc::UnboxRNop {},
        bc::PopC {},
        bc::Throw {}
      );
    }
  }

  reduce(env, bc::Nop {});
}

void in(ISS& /*env*/, const bc::FHandleRefMismatch& /*op*/) {}

void pushCallReturnType(ISS& env, Type&& ty, uint32_t numRets) {
  if (ty == TBottom) {
    // The callee function never returns.  It might throw, or loop forever.
    unreachable(env);
  }
  if (numRets != 1) {
    for (auto i = uint32_t{0}; i < numRets - 1; ++i) popU(env);
    if (is_specialized_vec(ty)) {
      for (int32_t i = 1; i < numRets; i++) {
        push(env, vec_elem(ty, ival(i)).first);
      }
      push(env, vec_elem(ty, ival(0)).first);
    } else {
      for (int32_t i = 0; i < numRets; i++) push(env, TInitCell);
    }
    return;
  }
  return push(env, std::move(ty));
}

const StaticString s_defined { "defined" };
const StaticString s_function_exists { "function_exists" };

void fcallKnownImpl(ISS& env, uint32_t numArgs, bool unpack, uint32_t numRets) {
  auto const ar = fpiTop(env);
  always_assert(ar.func.hasValue());

  if (options.ConstantFoldBuiltins && ar.foldable) {
    if (!unpack && numRets == 1) {
      auto ty = [&] () {
        auto const func = ar.func->exactFunc();
        assertx(func);
        if (func->attrs & AttrBuiltin && func->attrs & AttrIsFoldable) {
          auto ret = const_fold(env, numArgs, *ar.func);
          return ret ? *ret : TBottom;
        }
        std::vector<Type> args(numArgs);
        for (auto i = uint32_t{0}; i < numArgs; ++i) {
          auto const arg = topT(env, i);
          if (!is_scalar(arg)) return TBottom;
          args[numArgs - i - 1] = scalarize(arg);
        }

        return env.index.lookup_foldable_return_type(
          env.ctx, func, std::move(args));
      }();
      if (auto v = tv(ty)) {
        std::vector<Bytecode> repl { numArgs, bc::PopC {} };
        repl.push_back(gen_constant(*v));
        repl.push_back(bc::RGetCNop {});
        fpiPop(env);
        return reduce(env, std::move(repl));
      }
    }
    fpiNotFoldable(env);
    fpiPop(env);
    discard(env, numArgs + (unpack ? 1 : 0));
    while (numRets--) push(env, TBottom);
    return;
  }

  fpiPop(env);
  specialFunctionEffects(env, ar);

  if (!unpack && ar.func->name()->isame(s_function_exists.get())) {
    handle_function_exists(env, numArgs, false);
  }

  if (unpack) popC(env);
  std::vector<Type> args(numArgs);
  for (auto i = uint32_t{0}; i < numArgs; ++i) {
    args[numArgs - i - 1] = popF(env);
  }

  if (options.HardConstProp &&
      numArgs == 1 &&
      !unpack &&
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

  auto ty = unpack
    ? env.index.lookup_return_type(env.ctx, *ar.func)
    : env.index.lookup_return_type(CallContext { env.ctx, args, ar.context },
                                   *ar.func);
  if (ar.kind == FPIKind::ObjMethNS) {
    ty = union_of(std::move(ty), TInitNull);
  }
  if (!ar.fallbackFunc) {
    pushCallReturnType(env, std::move(ty), numRets);
    return;
  }
  auto ty2 = unpack
    ? env.index.lookup_return_type(env.ctx, *ar.fallbackFunc)
    : env.index.lookup_return_type(CallContext { env.ctx, args, ar.context },
                                   *ar.fallbackFunc);
  pushCallReturnType(env, union_of(std::move(ty), std::move(ty2)), numRets);
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
      assertx(op.str4->empty());
      if (ar.func->name() != op.str5) {
        // We've found a more precise type for the call, so update it
        return reduce(env, bc::FCall {
          op.arg1, op.arg2, op.arg3, staticEmptyString(), ar.func->name() });
      }
      return fcallKnownImpl(env, op.arg1, op.arg2 != 0, op.arg3);
    case FPIKind::Builtin:
      assertx(op.arg3 == 1);
      return finish_builtin(env, ar.func->exactFunc(), op.arg1, op.arg2 != 0);
    case FPIKind::Ctor:
      assertx(op.arg3 == 1);
      /*
       * Need to be wary of old-style ctors. We could get into the situation
       * where we're constructing class D extends B, and B has an old-style
       * ctor but D::B also exists.  (So in this case we'll skip the
       * fcallKnownImpl stuff.)
       */
      if (!ar.func->name()->isame(s_construct.get())) {
        break;
      }
      // fallthrough
    case FPIKind::ObjMeth:
    case FPIKind::ClsMeth:
      assertx(op.str4->empty() == op.str5->empty());
      if (ar.cls.hasValue() && ar.func->cantBeMagicCall() &&
          (ar.cls->name() != op.str4 || ar.func->name() != op.str5)) {
        // We've found a more precise type for the call, so update it
        return reduce(env, bc::FCall {
          op.arg1, op.arg2, op.arg3, ar.cls->name(), ar.func->name() });
      }
      // fallthrough
    case FPIKind::ObjMethNS:
      // If we didn't return a reduce above, we still can compute a
      // partially-known FCall effect with our res::Func.
      return fcallKnownImpl(env, op.arg1, op.arg2 != 0, op.arg3);
    }
  }

  if (op.arg2) popC(env);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popF(env);
  fpiPop(env);
  specialFunctionEffects(env, ar);
  for (auto i = uint32_t{0}; i < op.arg3 - 1; ++i) popU(env);
  for (auto i = uint32_t{0}; i < op.arg3; ++i) {
    push(env, op.arg3 == 1 ? TInitGen : TInitCell);
  }
}

void in(ISS& env, const bc::FCallAwait& op) {
  auto const ar = fpiTop(env);
  if (ar.foldable) {
    discard(env, op.arg1);
    fpiNotFoldable(env);
    fpiPop(env);
    return push(env, TBottom);
  }
  if ((ar.func && ar.func->name() != op.str3) ||
      (ar.cls && ar.cls->name() != op.str2)) {
    // We've found a more precise type for the call, so update it
    return reduce(
      env,
      bc::FCallAwait {
        op.arg1, ar.cls ? ar.cls->name() : staticEmptyString(), ar.func->name()
      }
    );
  }
  impl(env,
       bc::FCall { op.arg1, 0, 1, op.str2, op.str3 },
       bc::UnboxRNop {},
       bc::Await {});
}

void in(ISS& env, const bc::DecodeCufIter& op) {
  popC(env); // func
  env.propagate(op.target, &env.state); // before iter is modifed
}

namespace {

void iterInitImpl(ISS& env, IterId iter, LocalId valueLoc,
                  BlockId target, const Type& base, LocalId baseLoc) {
  assert(iterIsDead(env, iter));

  auto ity = iter_types(base);
  if (!ity.mayThrowOnInit) nothrow(env);

  auto const taken = [&]{
    // Take the branch before setting locals if the iter is already
    // empty, but after popping.  Similar for the other IterInits
    // below.
    freeIter(env, iter);
    env.propagate(target, &env.state);
  };

  auto const fallthrough = [&]{
    setIter(env, iter, LiveIter { ity, baseLoc, NoLocalId, env.blk.id });
    // Do this after setting the iterator, in case it clobbers the base local
    // equivalency.
    setLoc(env, valueLoc, std::move(ity.value));
  };

  switch (ity.count) {
    case IterTypes::Count::Empty:
      taken();
      mayReadLocal(env, valueLoc);
      jmp_setdest(env, target);
      break;
    case IterTypes::Count::Single:
    case IterTypes::Count::NonEmpty:
      fallthrough();
      jmp_nevertaken(env);
      break;
    case IterTypes::Count::ZeroOrOne:
    case IterTypes::Count::Any:
      taken();
      fallthrough();
      break;
  }
}

void iterInitKImpl(ISS& env, IterId iter, LocalId valueLoc, LocalId keyLoc,
                   BlockId target, const Type& base, LocalId baseLoc) {
  assert(iterIsDead(env, iter));

  auto ity = iter_types(base);
  if (!ity.mayThrowOnInit) nothrow(env);

  auto const taken = [&]{
    freeIter(env, iter);
    env.propagate(target, &env.state);
  };

  auto const fallthrough = [&]{
    setIter(env, iter, LiveIter { ity, baseLoc, NoLocalId, env.blk.id });
    // Do this after setting the iterator, in case it clobbers the base local
    // equivalency.
    setLoc(env, valueLoc, std::move(ity.value));
    setLoc(env, keyLoc, std::move(ity.key));
    if (!locCouldBeRef(env, keyLoc)) setIterKey(env, iter, keyLoc);
  };

  switch (ity.count) {
    case IterTypes::Count::Empty:
      taken();
      mayReadLocal(env, valueLoc);
      mayReadLocal(env, keyLoc);
      jmp_setdest(env, target);
      break;
    case IterTypes::Count::Single:
    case IterTypes::Count::NonEmpty:
      fallthrough();
      jmp_nevertaken(env);
      break;
    case IterTypes::Count::ZeroOrOne:
    case IterTypes::Count::Any:
      taken();
      fallthrough();
      break;
  }
}

void iterNextImpl(ISS& env, IterId iter, LocalId valueLoc, BlockId target) {
  auto const curLoc = locRaw(env, valueLoc);

  auto const noTaken = match<bool>(
    env.state.iters[iter],
    [&] (DeadIter)           {
      always_assert(false && "IterNext on dead iter");
      return false;
    },
    [&] (const LiveIter& ti) {
      if (!ti.types.mayThrowOnNext) nothrow(env);
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
      switch (ti.types.count) {
        case IterTypes::Count::Single:
        case IterTypes::Count::ZeroOrOne:
          return true;
        case IterTypes::Count::NonEmpty:
        case IterTypes::Count::Any:
          setLoc(env, valueLoc, ti.types.value);
          return false;
        case IterTypes::Count::Empty:
          always_assert(false);
      }
      not_reached();
    }
  );
  if (noTaken) {
    jmp_nevertaken(env);
    freeIter(env, iter);
    return;
  }

  env.propagate(target, &env.state);

  freeIter(env, iter);
  setLocRaw(env, valueLoc, curLoc);
}

void iterNextKImpl(ISS& env, IterId iter, LocalId valueLoc,
                   LocalId keyLoc, BlockId target) {
  auto const curValue = locRaw(env, valueLoc);
  auto const curKey = locRaw(env, keyLoc);

  auto const noTaken = match<bool>(
    env.state.iters[iter],
    [&] (DeadIter)           {
      always_assert(false && "IterNextK on dead iter");
      return false;
    },
    [&] (const LiveIter& ti) {
      if (!ti.types.mayThrowOnNext) nothrow(env);
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
      switch (ti.types.count) {
        case IterTypes::Count::Single:
        case IterTypes::Count::ZeroOrOne:
          return true;
        case IterTypes::Count::NonEmpty:
        case IterTypes::Count::Any:
          setLoc(env, valueLoc, ti.types.value);
          setLoc(env, keyLoc, ti.types.key);
          if (!locCouldBeRef(env, keyLoc)) setIterKey(env, iter, keyLoc);
          return false;
        case IterTypes::Count::Empty:
          always_assert(false);
      }
      not_reached();
    }
  );
  if (noTaken) {
    jmp_nevertaken(env);
    freeIter(env, iter);
    return;
  }

  env.propagate(target, &env.state);

  freeIter(env, iter);
  setLocRaw(env, valueLoc, curValue);
  setLocRaw(env, keyLoc, curKey);
}

}

void in(ISS& env, const bc::IterInit& op) {
  auto const baseLoc = topStkLocal(env);
  auto base = popC(env);
  iterInitImpl(env, op.iter1, op.loc3, op.target, std::move(base), baseLoc);
}

void in(ISS& env, const bc::LIterInit& op) {
  iterInitImpl(
    env,
    op.iter1,
    op.loc4,
    op.target,
    locAsCell(env, op.loc2),
    op.loc2
  );
}

void in(ISS& env, const bc::IterInitK& op) {
  auto const baseLoc = topStkLocal(env);
  auto base = popC(env);
  iterInitKImpl(
    env,
    op.iter1,
    op.loc3,
    op.loc4,
    op.target,
    std::move(base),
    baseLoc
  );
}

void in(ISS& env, const bc::LIterInitK& op) {
  iterInitKImpl(
    env,
    op.iter1,
    op.loc4,
    op.loc5,
    op.target,
    locAsCell(env, op.loc2),
    op.loc2
  );
}

void in(ISS& env, const bc::MIterInit& op) {
  popV(env);
  env.propagate(op.target, &env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::MIterInitK& op) {
  popV(env);
  env.propagate(op.target, &env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterInit& op) {
  popC(env);
  env.propagate(op.target, &env.state);
  // WIter* instructions may leave the value locals as either refs
  // or cells, depending whether the rhs of the assignment was a
  // ref.
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterInitK& op) {
  popC(env);
  env.propagate(op.target, &env.state);
  setLocRaw(env, op.loc3, TInitGen);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::IterNext& op) {
  iterNextImpl(env, op.iter1, op.loc3, op.target);
}

void in(ISS& env, const bc::LIterNext& op) {
  mayReadLocal(env, op.loc2);
  iterNextImpl(env, op.iter1, op.loc4, op.target);
}

void in(ISS& env, const bc::IterNextK& op) {
  iterNextKImpl(env, op.iter1, op.loc3, op.loc4, op.target);
}

void in(ISS& env, const bc::LIterNextK& op) {
  mayReadLocal(env, op.loc2);
  iterNextKImpl(env, op.iter1, op.loc4, op.loc5, op.target);
}

void in(ISS& env, const bc::MIterNext& op) {
  env.propagate(op.target, &env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::MIterNextK& op) {
  env.propagate(op.target, &env.state);
  unbindLocalStatic(env, op.loc3);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterNext& op) {
  env.propagate(op.target, &env.state);
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterNextK& op) {
  env.propagate(op.target, &env.state);
  setLocRaw(env, op.loc3, TInitGen);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::IterFree& op) {
  // IterFree is used for weak iterators too, so we can't assert !iterIsDead.
  nothrow(env);

  match<void>(
    env.state.iters[op.iter1],
    []  (DeadIter) {},
    [&] (const LiveIter& ti) {
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
    }
  );

  freeIter(env, op.iter1);
}
void in(ISS& env, const bc::LIterFree& op) {
  nothrow(env);
  mayReadLocal(env, op.loc2);
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
  nothrow(env);

  for (auto const& it : op.iterTab) {
    if (it.kind == KindOfIter || it.kind == KindOfLIter) {
      match<void>(
        env.state.iters[it.id],
        []  (DeadIter) {},
        [&] (const LiveIter& ti) {
          if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
        }
      );
    }
    if (it.kind == KindOfLIter) mayReadLocal(env, it.local);
    freeIter(env, it.id);
  }

  env.propagate(op.target, &env.state);
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
  putClsRefSlot(env, op.slot, setctx(ty ? *ty : TCls));
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
  env.state.thisLocToKill = op.loc1;
}

void in(ISS& env, const bc::StaticLocDef& op) {
  if (staticLocHelper(env, op.loc1, topC(env))) {
    return reduce(env, bc::SetL { op.loc1 }, bc::PopC {});
  }
  popC(env);
}

void in(ISS& env, const bc::StaticLocCheck& op) {
  auto const l = op.loc1;
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
  setLocRaw(env, l, TGen);
  maybeBindLocalStatic(env, l);
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
      if (!any(env.collect.opts & CollectionOpts::Inlining)) {
        unit->persistent.store(false, std::memory_order_relaxed);
      }
      // At this point, if it mayExist, we still don't know that it
      // *does* exist, but if not we know that it either doesn't
      // exist, or it doesn't have the right type.
      return mayExist ? TBool : TFalse;
    } ());
}

namespace {
bool couldBeMocked(const Type& t) {
  if (is_specialized_cls(t)) {
    return dcls_of(t).cls.couldBeMocked();
  } else if (is_specialized_obj(t)) {
    return dobj_of(t).cls.couldBeMocked();
  }
  // In practice this should not occur since this is used mostly on the result
  // of looked up type constraints.
  return true;
}
}

void in(ISS& env, const bc::VerifyParamType& op) {
  if (env.ctx.func->isMemoizeImpl &&
      !locCouldBeRef(env, op.loc1) &&
      RuntimeOption::EvalHardTypeHints) {
    // a MemoizeImpl's params have already been checked by the wrapper
    return reduce(env, bc::Nop {});
  }

  // Generally we won't know anything about the params, but
  // analyze_func_inline does - and this can help with effect-free analysis
  auto const constraint = env.ctx.func->params[op.loc1].typeConstraint;
  if (env.index.satisfies_constraint(env.ctx,
                                     locAsCell(env, op.loc1),
                                     constraint)) {
    reduce(env, bc::Nop {});
    return;
  }

  if (!RuntimeOption::EvalHardTypeHints) return;

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
  if (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis()) {
    return;
  }
  if (constraint.hasConstraint() && !constraint.isTypeVar() &&
      !constraint.isTypeConstant()) {
    auto t =
      loosen_dvarrayness(env.index.lookup_constraint(env.ctx, constraint));
    if (constraint.isThis() && couldBeMocked(t)) {
      t = unctx(std::move(t));
    }
    if (t.subtypeOf(BBottom)) unreachable(env);
    FTRACE(2, "     {} ({})\n", constraint.fullName(), show(t));
    setLoc(env, op.loc1, std::move(t));
  }
}

void verifyRetImpl(ISS& env, TypeConstraint& constraint, bool reduce_this) {
  auto stackT = topC(env);

  // If there is no return type constraint, or if the return type
  // constraint is a typevar, or if the top of stack is the same
  // or a subtype of the type constraint, then this is a no-op.
  if (env.index.satisfies_constraint(env.ctx, stackT, constraint)) {
    reduce(env, bc::Nop {});
    return;
  }

  // If CheckReturnTypeHints < 3 OR if the constraint is soft,
  // then there are no optimizations we can safely do here, so
  // just leave the top of stack as is.
  if (RuntimeOption::EvalCheckReturnTypeHints < 3 || constraint.isSoft()
      || (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis())) {
    return;
  }

  // In cases where we have a `this` hint where stackT is an TOptObj known to
  // be this, we can replace the check with a non null check.  These cases are
  // likely from a BareThis that could return Null.  Since the runtime will
  // split these translations, it will rarely in practice return null.
  if (constraint.isThis() && !constraint.isNullable() && is_opt(stackT) &&
      env.index.satisfies_constraint(env.ctx, unopt(stackT), constraint)) {
    if (reduce_this) {
      reduce(env, bc::VerifyRetNonNullC {});
      return;
    }
  }

  // If we reach here, then CheckReturnTypeHints >= 3 AND the constraint
  // is not soft.  We can safely assume that either VerifyRetTypeC will
  // throw or it will produce a value whose type is compatible with the
  // return type constraint.
  auto tcT =
    remove_uninit(
      loosen_dvarrayness(env.index.lookup_constraint(env.ctx, constraint))
    );

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

  auto retT = intersection_of(std::move(tcT), std::move(stackT));
  if (retT.subtypeOf(BBottom)) {
    unreachable(env);
    return;
  }

  popC(env);
  push(env, std::move(retT));
}

void in(ISS& /*env*/, const bc::VerifyRetTypeV& /*op*/) {}
void in(ISS& env, const bc::VerifyOutType& op) {
  verifyRetImpl(env, env.ctx.func->params[op.arg1].typeConstraint, false);
}

void in(ISS& env, const bc::VerifyRetTypeC& /*op*/) {
  verifyRetImpl(env, env.ctx.func->retTypeConstraint, true);
}

void in(ISS& env, const bc::VerifyRetNonNullC& /*op*/) {
  auto const constraint = env.ctx.func->retTypeConstraint;
  if (RuntimeOption::EvalCheckReturnTypeHints < 3 || constraint.isSoft()
      || (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis())) {
    return;
  }

  auto stackT = topC(env);

  if (!stackT.couldBe(BInitNull)) {
    reduce(env, bc::Nop {});
    return;
  }

  if (stackT.subtypeOf(BNull)) return unreachable(env);

  auto const equiv = topStkEquiv(env);

  if (is_opt(stackT)) stackT = unopt(std::move(stackT));

  popC(env);
  push(env, stackT, equiv);
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
      usedVars[nargs - i - 1] = unctx(popT(env));
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
  auto const closure = env.index.builtin_class(s_Closure.get());

  return push(env, subObj(closure));
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

void pushTypeFromWH(ISS& env, Type t) {
  if (!t.couldBe(BObj)) {
    // These opcodes require an object descending from WaitHandle.
    // Exceptions will be thrown for any non-object.
    push(env, TBottom);
    unreachable(env);
    return;
  }

  // Throw away non-obj component.
  t &= TObj;

  // If we aren't even sure this is a wait handle, there's nothing we can
  // infer here.
  if (!is_specialized_wait_handle(t)) {
    return push(env, TInitCell);
  }

  auto inner = wait_handle_inner(t);
  if (inner.subtypeOf(BBottom)) {
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

void in(ISS& env, const bc::AwaitAll& op) {
  auto const equiv = equivLocalRange(env, op.locrange);
  if (equiv != op.locrange.first) {
    return reduce(
      env,
      bc::AwaitAll {LocalRange {equiv, op.locrange.count}}
    );
  }

  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    mayReadLocal(env, op.locrange.first + i);
  }

  push(env, TInitNull);
}

namespace {

void idxImpl(ISS& env, bool arraysOnly) {
  auto const def  = popC(env);
  auto const key  = popC(env);
  auto const base = popC(env);

  if (key.subtypeOf(BInitNull)) {
    // A null key, regardless of whether we're ArrayIdx or Idx will always
    // silently return the default value, regardless of the base type.
    constprop(env);
    effect_free(env);
    return push(env, def);
  }

  // Push the returned type and annotate effects appropriately, taking into
  // account if the base might be null. Allowing for a possibly null base lets
  // us capture more cases.
  auto const finish = [&] (const Type& t, bool canThrow) {
    // A null base will raise if we're ArrayIdx. For Idx, it will silently
    // return the default value.
    auto const baseMaybeNull = base.couldBe(BInitNull);
    if (!canThrow && (!arraysOnly || !baseMaybeNull)) {
      constprop(env);
      effect_free(env);
    }
    if (!arraysOnly && baseMaybeNull) return push(env, union_of(t, def));
    if (t.subtypeOf(BBottom)) unreachable(env);
    return push(env, t);
  };

  if (arraysOnly) {
    // If ArrayIdx, we'll raise an error for anything other than array-like and
    // null. This op is only terminal if null isn't possible.
    if (!base.couldBeAny(TArr, TVec, TDict, TKeyset)) {
      return finish(key.couldBe(BInitNull) ? def : TBottom, true);
    }
  } else if (!base.couldBeAny(TArr, TVec, TDict, TKeyset, TStr, TObj)) {
    // Otherwise, any strange bases for Idx will just return the default value
    // without raising.
    return finish(def, false);
  }

  // Helper for Hack arrays. "validKey" is the set key types which can return a
  // value from Idx. "silentKey" is the set of key types which will silently
  // return null (anything else throws). The Hack array elem functions will
  // treat values of "silentKey" as throwing, so we must identify those cases
  // and deal with them.
  auto const hackArr = [&] (std::pair<Type, ThrowMode> elem,
                            const Type& validKey,
                            const Type& silentKey) {
    switch (elem.second) {
      case ThrowMode::None:
      case ThrowMode::MaybeMissingElement:
      case ThrowMode::MissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(elem.first, false);
      case ThrowMode::MaybeBadKey:
        assertx(key.couldBe(validKey));
        if (key.couldBe(silentKey)) elem.first |= def;
        return finish(elem.first, !key.subtypeOf(BOptArrKey));
      case ThrowMode::BadOperation:
        assertx(!key.couldBe(validKey));
        return finish(key.couldBe(silentKey) ? def : TBottom, true);
    }
  };

  if (base.subtypeOrNull(BVec)) {
    // Vecs will throw for any key other than Int, Str, or Null, and will
    // silently return the default value for the latter two.
    if (key.subtypeOrNull(BStr)) return finish(def, false);
    return hackArr(vec_elem(base, key, def), TInt, TOptStr);
  }

  if (base.subtypeOfAny(TOptDict, TOptKeyset)) {
    // Dicts and keysets will throw for any key other than Int, Str, or Null,
    // and will silently return the default value for Null.
    auto const elem = base.subtypeOrNull(BDict)
      ? dict_elem(base, key, def)
      : keyset_elem(base, key, def);
    return hackArr(elem, TArrKey, TInitNull);
  }

  if (base.subtypeOrNull(BArr)) {
    // A possibly null key is more complicated for arrays. array_elem() will
    // transform a null key into an empty string (matching the semantics of
    // array access), but that's not what Idx does. So, attempt to remove
    // nullish from the key first. If we can't, it just means we'll get a more
    // conservative value.
    auto maybeNull = false;
    auto const fixedKey = [&]{
      if (key.couldBe(TInitNull)) {
        maybeNull = true;
        if (is_nullish(key)) return unnullish(key);
      }
      return key;
    }();

    auto elem = array_elem(base, fixedKey, def);
    // If the key was null, Idx will return the default value, so add to the
    // return type.
    if (maybeNull) elem.first |= def;

    switch (elem.second) {
      case ThrowMode::None:
      case ThrowMode::MaybeMissingElement:
      case ThrowMode::MissingElement:
        return finish(elem.first, false);
      case ThrowMode::MaybeBadKey:
        return finish(elem.first, true);
      case ThrowMode::BadOperation:
        always_assert(false);
    }
  }

  if (!arraysOnly && base.subtypeOrNull(BStr)) {
    // Idx on a string always produces a string or the default value (without
    // ever raising).
    return finish(union_of(TStr, def), false);
  }

  // Objects or other unions of possible bases
  push(env, TInitCell);
}

}

void in(ISS& env, const bc::Idx&)      { idxImpl(env, false); }
void in(ISS& env, const bc::ArrayIdx&) { idxImpl(env, true);  }

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
        c->merge(env.ctx, *env.ctx.cls, sval(op.str1), t);
      }
      break;
    case InitPropOp::NonStatic:
      mergeThisProp(env, op.str1, t);
      break;
  }

  for (auto& prop : env.ctx.func->cls->properties) {
    if (prop.name != op.str1) continue;

    ITRACE(1, "InitProp: {} = {}\n", op.str1, show(t));

    if (env.index.satisfies_constraint(env.ctx, t, prop.typeConstraint)) {
      prop.attrs |= AttrInitialSatisfiesTC;
    } else {
      badPropInitialValue(env);
      prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
    }

    auto const v = tv(t);
    if (v || !could_contain_objects(t)) {
      prop.attrs = (Attr)(prop.attrs & ~AttrDeepInit);
      if (!v) break;
      prop.val = *v;
      if (op.subop2 == InitPropOp::Static &&
          !env.collect.publicStatics &&
          !env.index.frozen()) {
        env.index.fixup_public_static(env.ctx.func->cls, prop.name, t);
      }
      return reduce(env, bc::PopC {});
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


void in(ISS& env, const bc::MemoGet& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  always_assert(op.locrange.first + op.locrange.count
                <= env.ctx.func->locals.size());

  // If we can use an equivalent, earlier range, then use that instead.
  auto const equiv = equivLocalRange(env, op.locrange);
  if (equiv != op.locrange.first) {
    return reduce(
      env,
      bc::MemoGet { op.target, LocalRange { equiv, op.locrange.count } }
    );
  }

  auto retTy = memoizeImplRetType(env);
  if (retTy.second) constprop(env);

  // MemoGet can raise if we give a non arr-key local, or if we're in a method
  // and $this isn't available.
  auto allArrKey = true;
  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    allArrKey &= locRaw(env, op.locrange.first + i).subtypeOf(BArrKey);
  }
  if (allArrKey &&
      (!env.ctx.func->cls ||
       (env.ctx.func->attrs & AttrStatic) ||
       thisAvailable(env))) {
    nothrow(env);
  }

  env.propagate(op.target, &env.state);
  if (retTy.first == TBottom) jmp_setdest(env, op.target);
  push(env, std::move(retTy.first));
}

void in(ISS& env, const bc::MemoSet& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  always_assert(op.locrange.first + op.locrange.count
                <= env.ctx.func->locals.size());

  // If we can use an equivalent, earlier range, then use that instead.
  auto const equiv = equivLocalRange(env, op.locrange);
  if (equiv != op.locrange.first) {
    return reduce(
      env,
      bc::MemoSet { LocalRange { equiv, op.locrange.count } }
    );
  }

  // MemoSet can raise if we give a non arr-key local, or if we're in a method
  // and $this isn't available.
  auto allArrKey = true;
  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    allArrKey &= locRaw(env, op.locrange.first + i).subtypeOf(BArrKey);
  }
  if (allArrKey &&
      (!env.ctx.func->cls ||
       (env.ctx.func->attrs & AttrStatic) ||
       thisAvailable(env))) {
    nothrow(env);
  }
  push(env, popC(env));
}

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

#define X(y)                                                             \
  case Op::y:                                                            \
    switch (o2) {                                                        \
    case Op::Not:                                                        \
      switch (o3) {                                                      \
      case Op::JmpZ:                                                     \
        return group(env, it, it[0].y, it[1].Not, it[2].JmpZ);           \
      case Op::JmpNZ:                                                    \
        return group(env, it, it[0].y, it[1].Not, it[2].JmpNZ);          \
      default: break;                                                    \
      }                                                                  \
      break;                                                             \
    case Op::JmpZ:                                                       \
      return group(env, it, it[0].y, it[1].JmpZ);                        \
    case Op::JmpNZ:                                                      \
      return group(env, it, it[0].y, it[1].JmpNZ);                       \
    default: break;                                                      \
    }                                                                    \
    break;

  switch (o1) {
  X(InstanceOfD)
  X(IsTypeStruct)
  X(IsTypeL)
  X(IsTypeC)
  X(StaticLocCheck)
  X(Same)
  X(NSame)
  default: break;
  }
#undef X

  FTRACE(2, "  {}\n", show(env.ctx.func, *it));
  dispatch(env, *it++);
}

template<class Iterator>
StepFlags interpOps(Interp& interp,
                    Iterator& iter, Iterator stop,
                    PropagateFn propagate) {
  auto flags = StepFlags{};
  ISS env { interp, flags, propagate };

  // If there are throw exit edges, make a copy of the state (except
  // stacks) in case we need to propagate across throw exits (if
  // it's a PEI).
  auto const stateBefore = interp.blk->throwExits.empty()
    ? State{}
    : without_stacks(interp.state);

  auto const numPushed   = iter->numPush();
  interpStep(env, iter, stop);

  auto fix_const_outputs = [&] {
    auto elems = &interp.state.stack.back();
    constexpr auto numCells = 4;
    Cell cells[numCells];

    auto i = size_t{0};
    while (i < numPushed) {
      if (i < numCells) {
        auto const v = tv(elems->type);
        if (!v) return false;
        cells[i] = *v;
      } else if (!is_scalar(elems->type)) {
        return false;
      }
      ++i;
      --elems;
    }
    while (++elems, i--) {
      elems->type = from_cell(i < numCells ?
                              cells[i] : *tv(elems->type));
    }
    return true;
  };

  if (options.ConstantProp && flags.canConstProp && fix_const_outputs()) {
    if (flags.wasPEI) {
      FTRACE(2, "   nothrow (due to constprop)\n");
      flags.wasPEI = false;
    }
    if (!flags.effectFree) {
      FTRACE(2, "   effect_free (due to constprop)\n");
      flags.effectFree = true;
    }
  }

  assertx(!flags.effectFree || !flags.wasPEI);
  if (flags.wasPEI) {
    FTRACE(2, "   PEI.\n");
    for (auto exit : interp.blk->throwExits) {
      propagate(exit, &stateBefore);
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
    if (interp.collect.effectFree && !flags.effectFree) {
      interp.collect.effectFree = false;
      if (any(interp.collect.opts & CollectionOpts::EffectFreeOnly)) {
        FTRACE(2, "  Bailing because not effect free\n");
        return ret;
      }
    }

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

    if (flags.jmpDest != NoBlockId &&
        flags.jmpDest != interp.blk->fallthrough) {
      FTRACE(2, "  <took branch; no fallthrough>\n");
      return ret;
    }

    if (flags.returned) {
      FTRACE(2, "  returned {}\n", show(*flags.returned));
      always_assert(iter == stop);
      always_assert(interp.blk->fallthrough == NoBlockId);
      if (!ret.returned) {
        ret.retParam = flags.retParam;
      } else if (ret.retParam != flags.retParam) {
        ret.retParam = NoLocalId;
      }
      ret.returned = flags.returned;
      return ret;
    }
  }

  FTRACE(2, "  <end block>\n");
  if (interp.blk->fallthrough != NoBlockId) {
    propagate(interp.blk->fallthrough, &interp.state);
  }
  return ret;
}

StepFlags step(Interp& interp, const Bytecode& op) {
  auto flags   = StepFlags{};
  auto noop    = [] (BlockId, const State*) {};
  ISS env { interp, flags, noop };
  dispatch(env, op);
  return flags;
}

void default_dispatch(ISS& env, const Bytecode& op) {
  dispatch(env, op);
}

folly::Optional<Type> thisType(const Interp& interp) {
  return thisTypeHelper(interp.index, interp.ctx);
}

//////////////////////////////////////////////////////////////////////

}}
