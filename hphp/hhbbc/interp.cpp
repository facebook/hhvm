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
#include "hphp/hhbbc/interp.h"

#include <algorithm>
#include <vector>
#include <string>
#include <iterator>

#include <folly/Optional.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-ops.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/analyze.h"

#include "hphp/hhbbc/interp-internal.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

#define MII(m, ...) void minstr(ISS&, const bc::m##M&);
MINSTRS
#undef MII

void builtin(ISS&, const bc::FCallBuiltin&);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Exception("Exception");
const StaticString s_empty("");
const StaticString s_construct("__construct");
const StaticString s_86ctor("86ctor");
const StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
const StaticString s_IMemoizeParam("HH\\IMemoizeParam");
const StaticString s_getInstanceKey("getInstanceKey");

//////////////////////////////////////////////////////////////////////

#define O(opcode, ...) void in(ISS&, const bc::opcode&);
OPCODES
#undef O

//////////////////////////////////////////////////////////////////////

/*
 * impl(...)
 *
 * Utility for chaining one bytecode implementation to a series of a
 * few others.  Use reduce() if you also want to enable strength
 * reduction (i.e. the bytecode can be replaced by some other
 * bytecode as an optimization).
 *
 * The chained-to bytecodes should not take branches.  Also, constprop with
 * impl() will only occur on the last thing in the impl list---earlier opcodes
 * may set the canConstProp flag, but it will have no effect.
 */

template<class... Ts>
void impl(ISS& env, Ts&&... ts) {
  std::vector<Bytecode> bcs = { std::forward<Ts>(ts)... };

  folly::Optional<std::vector<Bytecode>> currentReduction;

  for (auto it = begin(bcs); it != end(bcs); ++it) {
    assert(env.flags.jmpFlag == StepFlags::JmpFlags::Either &&
           "you can't use impl with branching opcodes before last position");

    auto const wasPEI = env.flags.wasPEI;

    FTRACE(3, "    (impl {}\n", show(*it));
    env.flags.wasPEI          = true;
    env.flags.canConstProp    = false;
    env.flags.strengthReduced = folly::none;
    default_dispatch(env, *it);

    if (env.flags.strengthReduced) {
      if (!currentReduction) {
        currentReduction = std::vector<Bytecode>{};
        currentReduction->assign(begin(bcs), it);
      }
      std::copy(begin(*env.flags.strengthReduced),
                end(*env.flags.strengthReduced),
                std::back_inserter(*currentReduction));
    } else if (currentReduction) {
      currentReduction->push_back(*it);
    }

    // If any of the opcodes in the impl list said they could throw,
    // then the whole thing could throw.
    env.flags.wasPEI = env.flags.wasPEI || wasPEI;
  }

  env.flags.strengthReduced = currentReduction;
}

/*
 * Reduce means that (given some situation in the execution state),
 * a given bytecode could be replaced by some other bytecode
 * sequence.  Ensure that if you call reduce(), it is before any
 * state-affecting operations (like popC()).
 */
template<class... Bytecodes>
void reduce(ISS& env, const Bytecodes&... hhbc) {
  impl(env, hhbc...);
  if (!env.flags.strengthReduced) {
    env.flags.strengthReduced = std::vector<Bytecode> { hhbc... };
  }
}

//////////////////////////////////////////////////////////////////////

void in(ISS& env, const bc::Nop&)  { nothrow(env); }
void in(ISS& env, const bc::PopA&) { nothrow(env); popA(env); }
void in(ISS& env, const bc::PopC&) { nothrow(env); popC(env); }
void in(ISS& env, const bc::PopV&) { nothrow(env); popV(env); }
void in(ISS& env, const bc::PopR&) {
  auto t = topT(env, 0);
  if (t.subtypeOf(TCell)) {
    return reduce(env, bc::UnboxRNop {}, bc::PopC {});
  }
  nothrow(env);
  popR(env);
}

void in(ISS& env, const bc::Dup& op) {
  nothrow(env);
  auto const val = popC(env);
  push(env, val);
  push(env, val);
}

void in(ISS& env, const bc::AssertRATL&)     { nothrow(env); }
void in(ISS& env, const bc::AssertRATStk&)   { nothrow(env); }
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

void in(ISS& env, const bc::UnboxRNop&) {
  nothrow(env);
  constprop(env);
  auto const t = popR(env);
  push(env, t.subtypeOf(TInitCell) ? t : TInitCell);
}

void in(ISS& env, const bc::BoxRNop&) {
  nothrow(env);
  auto const t = popR(env);
  push(env, t.subtypeOf(TRef) ? t : TRef);
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
  nothrow(env);
  push(env, aval(op.arr1));
}

void in(ISS& env, const bc::NewArray& op) {
  push(env, op.arg1 == 0 ? aempty() : counted_aempty());
}

void in(ISS& env, const bc::NewMixedArray& op) {
  push(env, op.arg1 == 0 ? aempty() : counted_aempty());
}

void in(ISS& env, const bc::NewPackedArray& op) {
  auto elems = std::vector<Type>{};
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(popC(env));
  }
  std::reverse(begin(elems), end(elems));
  push(env, carr_packed(std::move(elems)));
}

void in(ISS& env, const bc::NewStructArray& op) {
  auto map = StructMap{};
  for (auto rit = op.keys.rbegin(); rit != op.keys.rend(); ++rit) {
    map[*rit] = popC(env);
  }
  push(env, carr_struct(std::move(map)));
}

void in(ISS& env, const bc::NewLikeArrayL&) {
  push(env, counted_aempty());
}

void in(ISS& env, const bc::AddElemC& op) {
  popC(env); popC(env); popC(env);
  push(env, TArr);
}
void in(ISS& env, const bc::AddElemV& op) {
  popV(env); popC(env); popC(env);
  push(env, TArr);
}

void in(ISS& env, const bc::AddNewElemC&) {
  popC(env);
  popC(env);
  push(env, TArr);
}
void in(ISS& env, const bc::AddNewElemV&) {
  popV(env);
  popC(env);
  push(env, TArr);
}

void in(ISS& env, const bc::NewCol& op) {
  auto const type = static_cast<CollectionType>(op.arg1);
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
}

void in(ISS& env, const bc::ColFromArray& op) {
  popC(env);
  auto const type = static_cast<CollectionType>(op.arg1);
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
}

void in(ISS& env, const bc::MapAddElemC&) {
  popC(env); popC(env);
  auto const coll = popC(env);
  push(env, coll);
}
void in(ISS& env, const bc::ColAddNewElemC&) {
  popC(env);
  auto const coll = popC(env);
  push(env, coll);
}

// Note: unlike class constants, these can be dynamic system
// constants, so this doesn't have to be TInitUnc.
void in(ISS& env, const bc::Cns&)  { push(env, TInitCell); }
void in(ISS& env, const bc::CnsE&) { push(env, TInitCell); }
void in(ISS& env, const bc::CnsU&) { push(env, TInitCell); }

void in(ISS& env, const bc::ClsCns& op) {
  auto const t1 = topA(env);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    if (dcls.type == DCls::Exact) {
      return reduce(env, bc::PopA {},
                         bc::ClsCnsD { op.str1, dcls.cls.name() });
    }
  }
  popA(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ClsCnsD& op) {
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str2)) {
    auto const t = env.index.lookup_class_constant(env.ctx, *rcls, op.str1);
    if (options.HardConstProp) constprop(env);
    push(env, t);
    return;
  }
  push(env, TInitCell);
}

void in(ISS& env, const bc::File&)  { nothrow(env); push(env, TSStr); }
void in(ISS& env, const bc::Dir&)   { nothrow(env); push(env, TSStr); }

void in(ISS& env, const bc::NameA&) {
  nothrow(env);
  popA(env);
  push(env, TSStr);
}

void in(ISS& env, const bc::Concat& op) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    if (v1->m_type == KindOfStaticString &&
        v2->m_type == KindOfStaticString) {
      constprop(env);
      auto const cell = eval_cell([&] {
        auto s = StringData::Make(
          v2->m_data.pstr, v1->m_data.pstr->slice());
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

template<class Op, class Fun>
void arithImpl(ISS& env, const Op& op, Fun fun) {
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

void in(ISS& env, const bc::BitNot& op) {
  auto const t = popC(env);
  auto const v = tv(t);
  if (v) {
    constprop(env);
    auto const cell = eval_cell([&] {
      auto c = *v;
      cellBitNot(c);
      return c;
    });
    return push(env, cell ? *cell : TInitCell);
  }
  push(env, TInitCell);
}

template<bool Negate>
void sameImpl(ISS& env) {
  nothrow(env);
  constprop(env);
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    return push(env, cellSame(*v2, *v1) != Negate ? TTrue : TFalse);
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
    constprop(env);
    return push(env, fun(*v2, *v1) ? TTrue : TFalse);
  }
  // TODO_4: evaluate when these can throw, non-constant type stuff.
  push(env, TBool);
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
    auto const cell = eval_cell([&] {
      return make_tv<KindOfBoolean>(cellToBool(*v) != negate);
    });
    always_assert_flog(!!cell, "cellToBool should never throw");
    return push(env, *cell);
  }

  if (t.subtypeOf(TArrE)) return push(env, negate ? TTrue : TFalse);
  if (t.subtypeOf(TArrN)) return push(env, negate ? TFalse : TTrue);

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
    auto const cell = eval_cell([&] {
      return make_tv<KindOfInt64>(cellToInt(*v));
    });
    return push(env, cell ? *cell : TInitCell);
  }
  push(env, TInt);
}

void castImpl(ISS& env, Type target) {
  auto const t = topC(env);
  if (t.subtypeOf(target)) return reduce(env, bc::Nop {});
  constprop(env);
  // TODO(#3875556): constant evaluate conversions when we can.
  popC(env);
  push(env, target);
}

void in(ISS& env, const bc::CastDouble&) { castImpl(env, TDbl); }
void in(ISS& env, const bc::CastString&) { castImpl(env, TStr); }
void in(ISS& env, const bc::CastArray&)  { castImpl(env, TArr); }
void in(ISS& env, const bc::CastObject&) { castImpl(env, TObj); }

void in(ISS& env, const bc::Print& op) { popC(env); push(env, ival(1)); }

void in(ISS& env, const bc::Clone& op) {
  auto const val = popC(env);
  push(env, val.subtypeOf(TObj) ? val :
            is_opt(val)         ? unopt(val) :
            TObj);
}

void in(ISS& env, const bc::Exit&)  { popC(env); push(env, TInitNull); }
void in(ISS& env, const bc::Fatal&) { popC(env); }

void in(ISS& env, const bc::JmpNS&) {
  always_assert(0 && "blocks should not contain JmpNS instructions");
}

void in(ISS& env, const bc::Jmp&) {
  always_assert(0 && "blocks should not contain Jmp instructions");
}

template<bool Negate, class Op>
void jmpImpl(ISS& env, const Op& op) {
  nothrow(env);
  auto const t1 = popC(env);
  auto const v1 = tv(t1);
  if (v1) {
    auto const taken = !cellToBool(*v1) != Negate;
    if (taken) {
      jmp_nofallthrough(env);
      env.propagate(*op.target, env.state);
    } else {
      jmp_nevertaken(env);
    }
    return;
  }
  env.propagate(*op.target, env.state);
}

void in(ISS& env, const bc::JmpNZ& op) { jmpImpl<true>(env, op); }
void in(ISS& env, const bc::JmpZ& op)  { jmpImpl<false>(env, op); }

template<class JmpOp>
void group(ISS& env, const bc::IsTypeL& istype, const JmpOp& jmp) {
  if (istype.subop == IsTypeOp::Scalar) return impl(env, istype, jmp);

  auto const loc = derefLoc(env, istype.loc1);
  auto const testTy = type_of_istype(istype.subop);
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

  setLoc(env, istype.loc1, negate ? was_true : was_false);
  env.propagate(*jmp.target, env.state);
  setLoc(env, istype.loc1, negate ? was_false : was_true);
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

  setLoc(env, cgetl.loc1, negate ? converted_true : converted_false);
  env.propagate(*jmp.target, env.state);
  setLoc(env, cgetl.loc1, negate ? converted_false : converted_true);
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
  setLoc(env, cgetl.loc1, negate ? was_true : was_false);
  env.propagate(*jmp.target, env.state);
  setLoc(env, cgetl.loc1, negate ? was_false : was_true);
}

void group(ISS& env,
           const bc::CGetL& cgetl,
           const bc::FPushObjMethodD& fpush) {
  auto const obj = locAsCell(env, cgetl.loc1);
  impl(env, cgetl, fpush);
  if (!is_specialized_obj(obj)) {
    setLoc(env, cgetl.loc1,
           fpush.subop == ObjMethodOp::NullThrows ? TObj : TOptObj);
  } else if (is_opt(obj) && fpush.subop == ObjMethodOp::NullThrows) {
    setLoc(env, cgetl.loc1, unopt(obj));
  }
}

void in(ISS& env, const bc::Switch& op) {
  popC(env);
  forEachTakenEdge(op, [&] (php::Block& blk) {
    env.propagate(blk, env.state);
  });
}

void in(ISS& env, const bc::SSwitch& op) {
  popC(env);
  forEachTakenEdge(op, [&] (php::Block& blk) {
    env.propagate(blk, env.state);
  });
}

void in(ISS& env, const bc::RetC& op)    { doRet(env, popC(env)); }
void in(ISS& env, const bc::RetV& op)    { doRet(env, popV(env)); }
void in(ISS& env, const bc::Unwind& op)  {}
void in(ISS& env, const bc::Throw& op)   { popC(env); }

void in(ISS& env, const bc::Catch&) {
  nothrow(env);
  return push(env, subObj(env.index.builtin_class(s_Exception.get())));
}

void in(ISS& env, const bc::NativeImpl&) {
  killLocals(env);

  if (is_collection_method_returning_this(env.ctx.cls, env.ctx.func)) {
    auto const resCls = env.index.builtin_class(env.ctx.cls->name);
    return doRet(env, objExact(resCls));
  }

  if (env.ctx.func->nativeInfo) {
    return doRet(env, native_function_return_type(env.ctx.func));
  }
  doRet(env, TInitGen);
}

void in(ISS& env, const bc::CGetL& op) {
  if (!locCouldBeUninit(env, op.loc1)) { nothrow(env); constprop(env); }
  push(env, locAsCell(env, op.loc1));
}

void in(ISS& env, const bc::CUGetL& op) {
  auto const ty = locRaw(env, op.loc1);
  if (ty.subtypeOf(TUninit)) {
    return reduce(env, bc::NullUninit {});
  }
  nothrow(env);
  if (!ty.couldBe(TUninit)) constprop(env);
  push(env, ty.subtypeOf(TCell) ? ty : TCell);
}

void in(ISS& env, const bc::PushL& op) {
  impl(env, bc::CGetL { op.loc1 }, bc::UnsetL { op.loc1 });
}

void in(ISS& env, const bc::CGetL2& op) {
  // Can't constprop yet because of no INS_1 support in bc.h
  if (!locCouldBeUninit(env, op.loc1)) nothrow(env);
  auto const loc = locAsCell(env, op.loc1);
  auto const top = popT(env);
  push(env, loc);
  push(env, top);
}

void in(ISS& env, const bc::CGetL3& op) {
  // Can't constprop yet because of no INS_2 support in bc.h
  if (!locCouldBeUninit(env, op.loc1)) nothrow(env);
  auto const loc = locAsCell(env, op.loc1);
  auto const t1 = popT(env);
  auto const t2 = popT(env);
  push(env, loc);
  push(env, t2);
  push(env, t1);
}

void in(ISS& env, const bc::CGetN&) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfStaticString) {
    if (auto const loc = findLocal(env, v1->m_data.pstr)) {
      return reduce(env, bc::PopC {},
                         bc::CGetL { loc });
    }
  }
  readUnknownLocals(env);
  popC(env); // conversion to string can throw
  push(env, TInitCell);
}

void in(ISS& env, const bc::CGetG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::CGetS&) {
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (vname && vname->m_type == KindOfStaticString &&
      self && tcls.subtypeOf(*self)) {
    if (auto const ty = selfPropAsCell(env, vname->m_data.pstr)) {
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

      return push(env, *ty);
    }
  }

  auto const indexTy = env.index.lookup_public_static(tcls, tname);
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
    return push(env, indexTy);
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
  if (v1 && v1->m_type == KindOfStaticString) {
    if (auto const loc = findLocal(env, v1->m_data.pstr)) {
      return reduce(env, bc::PopC {},
                         bc::VGetL { loc });
    }
  }
  popC(env);
  boxUnknownLocal(env);
  push(env, TRef);
}

void in(ISS& env, const bc::VGetG&) { popC(env); push(env, TRef); }

void in(ISS& env, const bc::VGetS&) {
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfStaticString) {
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

void aGetImpl(ISS& env, Type t1) {
  if (t1.subtypeOf(TObj)) {
    nothrow(env);
    return push(env, objcls(t1));
  }
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfStaticString) {
    if (auto const rcls = env.index.resolve_class(env.ctx, v1->m_data.pstr)) {
      return push(env, clsExact(*rcls));
    }
  }
  push(env, TCls);
}

void in(ISS& env, const bc::AGetL& op) {
  aGetImpl(env, locAsCell(env, op.loc1));
}
void in(ISS& env, const bc::AGetC& op) {
  aGetImpl(env, popC(env));
}

void in(ISS& env, const bc::AKExists& op) {
  auto const t1   = popC(env);
  auto const t2   = popC(env);
  auto const t1Ok = t1.subtypeOf(TObj) || t1.subtypeOf(TArr);
  auto const t2Ok = t2.subtypeOf(TInt) || t2.subtypeOf(TNull) ||
                    t2.subtypeOf(TStr);
  if (t1Ok && t2Ok) nothrow(env);
  push(env, TBool);
}

void in(ISS& env, const bc::GetMemoKey& op) {
  auto const tyIMemoizeParam =
    subObj(env.index.builtin_class(s_IMemoizeParam.get()));
  auto const t = topC(env);

  if (t.subtypeOf(TInt) || t.subtypeOf(TStr) || t.subtypeOf(TOptInt)) {
    return reduce(env, bc::Nop {});
  }
  if (t.subtypeOf(TBool)) {
    return reduce(env, bc::CastInt {});
  }
  if (t.subtypeOf(tyIMemoizeParam)) {
    return reduce(
      env,
      bc::FPushObjMethodD { 0, s_getInstanceKey.get(),
                            ObjMethodOp::NullThrows },
      bc::FCall { 0 },
      bc::UnboxR {}
    );
  }

  popC(env);
  push(env, TInitCell);
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
    return impl(env, bc::CGetL { op.loc1 },
                     bc::Not {});
  }
  locAsCell(env, op.loc1); // read the local
  push(env, TBool);
}

void in(ISS& env, const bc::EmptyS&) {
  popA(env);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::IssetS&) {
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (self && tcls.subtypeOf(*self) &&
      vname && vname->m_type == KindOfStaticString) {
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
  if (v1 && v1->m_type == KindOfStaticString) {
    if (auto const loc = findLocal(env, v1->m_data.pstr)) {
      return reduce(env, bc::PopC {}, ReduceOp { loc });
    }
    // Can't push true in the non env.findLocal case unless we know
    // whether this function can have a VarEnv.
  }
  readUnknownLocals(env);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::IssetN&) { issetEmptyNImpl<bc::IssetL>(env); }
void in(ISS& env, const bc::EmptyN&) { issetEmptyNImpl<bc::EmptyL>(env); }
void in(ISS& env, const bc::EmptyG&) { popC(env); push(env, TBool); }
void in(ISS& env, const bc::IssetG&) { popC(env); push(env, TBool); }

void isTypeImpl(ISS& env, Type locOrCell, Type test) {
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
  switch (op.subop) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, loc);
  default: return isTypeImpl(env, loc, type_of_istype(op.subop));
  }
}

template<class Op>
void isTypeCImpl(ISS& env, const Op& op) {
  nothrow(env);
  auto const t1 = popC(env);
  switch (op.subop) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, t1);
  default: return isTypeImpl(env, t1, type_of_istype(op.subop));
  }
}

void in(ISS& env, const bc::IsTypeC& op) { isTypeCImpl(env, op); }
void in(ISS& env, const bc::IsTypeL& op) { isTypeLImpl(env, op); }

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

void in(ISS& env, const bc::InstanceOf& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfStaticString) {
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
  auto const val = popC(env);
  setLoc(env, op.loc1, val);
  push(env, val);
}

void in(ISS& env, const bc::SetN&) {
  // This isn't trivial to strength reduce, without a "flip two top
  // elements of stack" opcode.
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);
  // TODO(#3653110): could nothrow if t2 can't be an Obj or Res

  auto const knownLoc = v2 && v2->m_type == KindOfStaticString
    ? findLocal(env, v2->m_data.pstr)
    : nullptr;
  if (knownLoc) {
    setLoc(env, knownLoc, t1);
  } else {
    // We could be changing the value of any local, but we won't
    // change whether or not they are boxed or initialized.
    loseNonRefLocalTypes(env);
  }
  push(env, t1);
}

void in(ISS& env, const bc::SetG&) {
  auto const t1 = popC(env);
  popC(env);
  push(env, t1);
}

void in(ISS& env, const bc::SetS&) {
  auto const t1    = popC(env);
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfStaticString) {
      nothrow(env);
      mergeSelfProp(env, vname->m_data.pstr, t1);
    } else {
      mergeEachSelfPropRaw(env, [&] (Type) { return t1; });
    }
  }

  if (auto c = env.collect.publicStatics) {
    c->merge(env.ctx, tcls, tname, t1);
  }

  push(env, t1);
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
      SETOP_BODY_CELL(&c, op.subop, &rhs);
      return c;
    });
    if (!resultTy) resultTy = TInitCell;

    // We may have inferred a TSStr or TSArr with a value here, but
    // at runtime it will not be static.  For now just throw that
    // away.  TODO(#3696042): should be able to loosen_statics here.
    if (resultTy->subtypeOf(TStr))      resultTy = TStr;
    else if (resultTy->subtypeOf(TArr)) resultTy = TArr;

    setLoc(env, op.loc1, *resultTy);
    push(env, *resultTy);
    return;
  }

  auto const resultTy = typeSetOp(op.subop, loc, t1);
  setLoc(env, op.loc1, resultTy);
  push(env, resultTy);
}

void in(ISS& env, const bc::SetOpN&) {
  popC(env);
  popC(env);
  loseNonRefLocalTypes(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::SetOpG&) {
  popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::SetOpS&) {
  popC(env);
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfStaticString) {
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
  auto const loc = locAsCell(env, op.loc1);
  auto const newT = typeIncDec(op.subop, loc);
  auto const pre = isPre(op.subop);

  // If it's a non-numeric string, this may cause it to exceed the max length.
  if (!locCouldBeUninit(env, op.loc1) &&
      !loc.couldBe(TStr)) {
    nothrow(env);
  }

  if (!pre) push(env, loc);
  setLoc(env, op.loc1, newT);
  if (pre)  push(env, newT);
}

void in(ISS& env, const bc::IncDecN& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  auto const knownLoc = v1 && v1->m_type == KindOfStaticString
    ? findLocal(env, v1->m_data.pstr)
    : nullptr;
  if (knownLoc) {
    return reduce(env, bc::PopC {},
                       bc::IncDecL { knownLoc, op.subop });
  }
  popC(env);
  loseNonRefLocalTypes(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::IncDecG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::IncDecS&) {
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfStaticString) {
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
  nothrow(env);
  auto const t1 = popV(env);
  setLocRaw(env, op.loc1, t1);
  push(env, t1);
}

void in(ISS& env, const bc::BindN&) {
  // TODO(#3653110): could nothrow if t2 can't be an Obj or Res
  auto const t1 = popV(env);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);
  auto const knownLoc = v2 && v2->m_type == KindOfStaticString
    ? findLocal(env, v2->m_data.pstr)
    : nullptr;
  if (knownLoc) {
    setLocRaw(env, knownLoc, t1);
  } else {
    boxUnknownLocal(env);
  }
  push(env, t1);
}

void in(ISS& env, const bc::BindG&) {
  auto const t1 = popV(env);
  popC(env);
  push(env, t1);
}

void in(ISS& env, const bc::BindS&) {
  popV(env);
  auto const tcls  = popA(env);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfStaticString) {
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

void in(ISS& env, const bc::EmptyM& op)       { minstr(env, op); }
void in(ISS& env, const bc::IssetM& op)       { minstr(env, op); }
void in(ISS& env, const bc::CGetM& op)        { minstr(env, op); }
void in(ISS& env, const bc::VGetM& op)        { minstr(env, op); }
void in(ISS& env, const bc::SetM& op)         { minstr(env, op); }
void in(ISS& env, const bc::SetWithRefLM& op) { minstr(env, op); }
void in(ISS& env, const bc::SetWithRefRM& op) { minstr(env, op); }
void in(ISS& env, const bc::SetOpM& op)       { minstr(env, op); }
void in(ISS& env, const bc::IncDecM& op)      { minstr(env, op); }
void in(ISS& env, const bc::UnsetM& op)       { minstr(env, op); }
void in(ISS& env, const bc::BindM& op)        { minstr(env, op); }

void in(ISS& env, const bc::UnsetL& op) {
  nothrow(env);
  setLocRaw(env, op.loc1, TUninit);
}

void in(ISS& env, const bc::UnsetN& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfStaticString) {
    if (auto const loc = findLocal(env, v1->m_data.pstr)) {
      return reduce(env, bc::PopC {},
                         bc::UnsetL { loc });
    }
  }
  popC(env);
  if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow(env);
  unsetUnknownLocal(env);
}

void in(ISS& env, const bc::UnsetG& op) {
  auto const t1 = popC(env);
  if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow(env);
}

void in(ISS& env, const bc::FPushFuncD& op) {
  auto const rfunc = env.index.resolve_func(env.ctx, op.str2);
  fpiPush(env, ActRec { FPIKind::Func, folly::none, rfunc });
}

void in(ISS& env, const bc::FPushFunc& op) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfStaticString) {
    auto const name = normalizeNS(v1->m_data.pstr);
    if (isNSNormalized(name)) {
      return reduce(env, bc::PopC {},
                         bc::FPushFuncD { op.arg1, name });
    }
  }
  popC(env);
  if (t1.subtypeOf(TObj)) return fpiPush(env, ActRec { FPIKind::ObjInvoke });
  if (t1.subtypeOf(TArr)) return fpiPush(env, ActRec { FPIKind::CallableArr });
  if (t1.subtypeOf(TStr)) return fpiPush(env, ActRec { FPIKind::Func });
  fpiPush(env, ActRec { FPIKind::Unknown });
}

void in(ISS& env, const bc::FPushFuncU& op) {
  auto const rfunc =
    env.index.resolve_func_fallback(env.ctx, op.str2, op.str3);
  fpiPush(env, ActRec { FPIKind::Func, folly::none, rfunc });
}

void in(ISS& env, const bc::FPushObjMethodD& op) {
  auto t1 = popC(env);
  if (is_opt(t1) && op.subop == ObjMethodOp::NullThrows) {
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
  if (v1 && v1->m_type == KindOfStaticString) {
    return reduce(
      env,
      bc::PopC {},
      bc::FPushObjMethodD { op.arg1, v1->m_data.pstr, op.subop }
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
  auto const t1 = popA(env);
  auto const t2 = popC(env);
  auto const v2 = tv(t2);

  folly::Optional<res::Func> rfunc;
  if (v2 && v2->m_type == KindOfStaticString) {
    rfunc = env.index.resolve_method(env.ctx, t1, v2->m_data.pstr);
  }
  folly::Optional<res::Class> rcls;
  if (is_specialized_cls(t1)) rcls = dcls_of(t1).cls;
  fpiPush(env, ActRec { FPIKind::ClsMeth, rcls, rfunc });
}

void in(ISS& env, const bc::FPushClsMethodF& op) {
  // The difference with FPushClsMethod is what ends up on the
  // ActRec (late-bound class), which we currently aren't tracking.
  impl(env, bc::FPushClsMethod { op.arg1 });
}

void in(ISS& env, const bc::FPushCtorD& op) {
  auto const rcls = env.index.resolve_class(env.ctx, op.str2);
  push(env, rcls ? objExact(*rcls) : TObj);
  auto const rfunc =
    rcls ? env.index.resolve_ctor(env.ctx, *rcls) : folly::none;
  fpiPush(env, ActRec { FPIKind::Ctor, rcls, rfunc });
}

void in(ISS& env, const bc::FPushCtor& op) {
  auto const t1 = topA(env);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    if (dcls.type == DCls::Exact) {
      return reduce(env, bc::PopA {},
                         bc::FPushCtorD { op.arg1, dcls.cls.name() });
    }
  }
  popA(env);
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
  auto const t1 = popC(env);
  popC(env);
  push(env, t1);
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
  case PrepKind::Val: return reduce(env, bc::CGetL { op.loc2 },
                                         bc::FPassC { op.arg1 });
  case PrepKind::Ref: return reduce(env, bc::VGetL { op.loc2 },
                                         bc::FPassVNop { op.arg1 });
  }
}

void in(ISS& env, const bc::FPassN& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    // This could change the type of any local.
    popC(env);
    killLocals(env);
    return push(env, TInitGen);
  case PrepKind::Val: return reduce(env, bc::CGetN {},
                                         bc::FPassC { op.arg1 });
  case PrepKind::Ref: return reduce(env, bc::VGetN {},
                                         bc::FPassVNop { op.arg1 });
  }
}

void in(ISS& env, const bc::FPassG& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown: popC(env); return push(env, TInitGen);
  case PrepKind::Val:     return reduce(env, bc::CGetG {},
                                             bc::FPassC { op.arg1 });
  case PrepKind::Ref:     return reduce(env, bc::VGetG {},
                                             bc::FPassVNop { op.arg1 });
  }
}

void in(ISS& env, const bc::FPassS& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    {
      auto const tcls  = popA(env);
      auto const self  = selfCls(env);
      auto const tname = popC(env);
      auto const vname = tv(tname);
      if (!self || tcls.couldBe(*self)) {
        if (vname && vname->m_type == KindOfStaticString) {
          // May or may not be boxing it, depending on the refiness.
          mergeSelfProp(env, vname->m_data.pstr, TInitGen);
        } else {
          killSelfProps(env);
        }
      }
      if (auto c = env.collect.publicStatics) {
        c->merge(env.ctx, tcls, tname, TInitGen);
      }
    }
    return push(env, TInitGen);
  case PrepKind::Val:
    return reduce(env, bc::CGetS {}, bc::FPassC { op.arg1 });
  case PrepKind::Ref:
    return reduce(env, bc::VGetS {}, bc::FPassVNop { op.arg1 });
  }
}

void in(ISS& env, const bc::FPassV& op) {
  nothrow(env);
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    popV(env);
    return push(env, TInitGen);
  case PrepKind::Val:
    return reduce(env, bc::Unbox {}, bc::FPassC { op.arg1 });
  case PrepKind::Ref:
    return reduce(env, bc::FPassVNop { op.arg1 });
  }
}

void in(ISS& env, const bc::FPassR& op) {
  nothrow(env);
  auto const t1 = topT(env);
  if (t1.subtypeOf(TCell)) {
    return reduce(env, bc::UnboxRNop {},
                       bc::FPassC { op.arg1 });
  }

  // If it's known to be a ref, this behaves like FPassV, except we need to do
  // it slightly differently to keep stack flavors correct.
  if (t1.subtypeOf(TRef)) {
    switch (prepKind(env, op.arg1)) {
    case PrepKind::Unknown:
      popV(env);
      return push(env, TInitGen);
    case PrepKind::Val:
      return reduce(env, bc::UnboxR {}, bc::FPassC { op.arg1 });
    case PrepKind::Ref:
      return reduce(env, bc::BoxRNop {}, bc::FPassVNop { op.arg1 });
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

void in(ISS& env, const bc::FPassVNop&) { nothrow(env); push(env, popV(env)); }
void in(ISS& env, const bc::FPassC& op) { nothrow(env); }

void in(ISS& env, const bc::FPassCW& op) {
  impl(env, bc::FPassCE { op.arg1 });
}

void in(ISS& env, const bc::FPassCE& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown: return;
  case PrepKind::Val:     return reduce(env, bc::FPassC { op.arg1 });
  case PrepKind::Ref:     /* will warn/fatal at runtime */ return;
  }
}

void in(ISS& env, const bc::FPassM& op) {
  switch (prepKind(env, op.arg1)) {
  case PrepKind::Unknown:
    break;
  case PrepKind::Val:
    return reduce(env, bc::CGetM { op.mvec }, bc::FPassC { op.arg1 });
  case PrepKind::Ref:
    return reduce(env, bc::VGetM { op.mvec }, bc::FPassVNop { op.arg1 });
  }

  /*
   * FPassM with an unknown PrepKind either has the effects of CGetM
   * or the effects of VGetM, but we don't know which statically.
   * These are complicated instructions, so the easiest way to
   * handle this is to run both and then merge their output states.
   */
  auto const start = env.state;
  in(env, bc::CGetM { op.mvec });
  auto const cgetm = env.state;
  env.state = start;
  in(env, bc::VGetM { op.mvec });
  merge_into(env.state, cgetm);
  assert(env.flags.wasPEI);
  assert(!env.flags.canConstProp);
}

void pushCallReturnType(ISS& env, const Type& ty) {
  if (ty == TBottom) {
    // The callee function never returns.  It might throw, or loop forever.
    unreachable(env);
  }
  return push(env, ty);
}

void fcallKnownImpl(ISS& env, uint32_t numArgs) {
  auto const ar = fpiPop(env);
  always_assert(ar.func.hasValue());
  specialFunctionEffects(env, ar);

  std::vector<Type> args(numArgs);
  for (auto i = uint32_t{0}; i < numArgs; ++i) {
    args[numArgs - i - 1] = popF(env);
  }
  auto const ty = env.index.lookup_return_type(
    CallContext { env.ctx, args },
    *ar.func
  );
  pushCallReturnType(env, ty);
}

void in(ISS& env, const bc::FCall& op) {
  auto const ar = fpiTop(env);
  if (ar.func) {
    switch (ar.kind) {
    case FPIKind::Unknown:
    case FPIKind::CallableArr:
    case FPIKind::ObjInvoke:
      not_reached();
    case FPIKind::Func:
      return reduce(
        env,
        bc::FCallD { op.arg1, s_empty.get(), ar.func->name() }
      );
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
  if (ar.func) return fcallKnownImpl(env, op.arg1);
  specialFunctionEffects(env, ar);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popF(env);
  push(env, TInitGen);
}

void fcallArrayImpl(ISS& env) {
  auto const ar = fpiPop(env);
  specialFunctionEffects(env, ar);
  if (ar.func) {
    auto const ty = env.index.lookup_return_type(env.ctx, *ar.func);
    pushCallReturnType(env, ty);
    return;
  }
  return push(env, TInitGen);
}

void in(ISS& env, const bc::FCallArray& op) {
  popF(env);
  fcallArrayImpl(env);
}

void in(ISS& env, const bc::FCallUnpack& op) {
  for (auto i = uint32_t{0}; i < op.arg1; ++i) { popF(env); }
  fcallArrayImpl(env);
}

void in(ISS& env, const bc::FCallBuiltin& op) { builtin(env, op); }

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
  env.propagate(*op.target, env.state); // before iter is modifed
}

void in(ISS& env, const bc::IterInit& op) {
  auto const t1 = popC(env);
  // Take the branch before setting locals if the iter is already
  // empty, but after popping.  Similar for the other IterInits
  // below.
  freeIter(env, op.iter1);
  env.propagate(*op.target, env.state);
  if (t1.subtypeOf(TArrE)) {
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
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::IterInitK& op) {
  auto const t1 = popC(env);
  freeIter(env, op.iter1);
  env.propagate(*op.target, env.state);
  if (t1.subtypeOf(TArrE)) {
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
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterInit& op) {
  popC(env);
  env.propagate(*op.target, env.state);
  // WIter* instructions may leave the value locals as either refs
  // or cells, depending whether the rhs of the assignment was a
  // ref.
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterInitK& op) {
  popC(env);
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TInitGen);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::IterNext& op) {
  auto const curLoc3 = locRaw(env, op.loc3);

  match<void>(
    env.state.iters[op.iter1->id],
    [&] (UnknownIter)           { setLoc(env, op.loc3, TInitCell); },
    [&] (const TrackedIter& ti) { setLoc(env, op.loc3, ti.kv.second); }
  );
  env.propagate(*op.target, env.state);

  freeIter(env, op.iter1);
  setLocRaw(env, op.loc3, curLoc3);
}

void in(ISS& env, const bc::MIterNext& op) {
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TRef);
}

void in(ISS& env, const bc::IterNextK& op) {
  auto const curLoc3 = locRaw(env, op.loc3);
  auto const curLoc4 = locRaw(env, op.loc4);

  match<void>(
    env.state.iters[op.iter1->id],
    [&] (UnknownIter) {
      setLoc(env, op.loc3, TInitCell);
      setLoc(env, op.loc4, TInitCell);
    },
    [&] (const TrackedIter& ti) {
      setLoc(env, op.loc3, ti.kv.second);
      setLoc(env, op.loc4, ti.kv.first);
    }
  );
  env.propagate(*op.target, env.state);

  freeIter(env, op.iter1);
  setLocRaw(env, op.loc3, curLoc3);
  setLocRaw(env, op.loc4, curLoc4);
}

void in(ISS& env, const bc::MIterNextK& op) {
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TRef);
  setLoc(env, op.loc4, TInitCell);
}

void in(ISS& env, const bc::WIterNext& op) {
  env.propagate(*op.target, env.state);
  setLocRaw(env, op.loc3, TInitGen);
}

void in(ISS& env, const bc::WIterNextK& op) {
  env.propagate(*op.target, env.state);
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
  env.propagate(*op.target, env.state);
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
  push(env, TInitCell);
}

void in(ISS& env, const bc::Incl&)      { inclOpImpl(env); }
void in(ISS& env, const bc::InclOnce&)  { inclOpImpl(env); }
void in(ISS& env, const bc::Req&)       { inclOpImpl(env); }
void in(ISS& env, const bc::ReqOnce&)   { inclOpImpl(env); }
void in(ISS& env, const bc::ReqDoc&)    { inclOpImpl(env); }
void in(ISS& env, const bc::Eval&)      { inclOpImpl(env); }

void in(ISS& env, const bc::DefFunc&)      {}
void in(ISS& env, const bc::DefCls&)       {}
void in(ISS& env, const bc::DefClsNop&)    {}
void in(ISS& env, const bc::DefCns&)       { popC(env); push(env, TBool); }
void in(ISS& env, const bc::DefTypeAlias&) {}

void in(ISS& env, const bc::This&) {
  if (thisAvailable(env)) {
    return reduce(env, bc::BareThis { BareThisOp::NeverNull });
  }
  auto const ty = thisType(env);
  push(env, ty ? *ty : TObj);
  setThisAvailable(env);
}

void in(ISS& env, const bc::LateBoundCls&) {
  auto const ty = selfCls(env);
  push(env, ty ? *ty : TCls);
}

void in(ISS& env, const bc::CheckThis&) {
  if (thisAvailable(env)) {
    reduce(env, bc::Nop {});
  }
  setThisAvailable(env);
}

void in(ISS& env, const bc::BareThis& op) {
  if (thisAvailable(env)) {
    if (op.subop != BareThisOp::NeverNull) {
      return reduce(env, bc::BareThis { BareThisOp::NeverNull });
    }
  }

  auto const ty = thisType(env);
  switch (op.subop) {
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

void in(ISS& env, const bc::StaticLoc& op) {
  setLocRaw(env, op.loc1, TRef);
  push(env, TBool);
}

void in(ISS& env, const bc::StaticLocInit& op) {
  popC(env);
  setLocRaw(env, op.loc1, TRef);
}

/*
 * This can't trivially check that the class/trait/interface exists
 * (e.g. via resolve_class) without knowing either:
 *
 *  a) autoload is guaranteed to load it and t1 == true, or
 *  b) it's already defined in this unit.
 *
 * op.subop (OODeclExistsOp) would be useful with resolution of the class
 */
void in(ISS& env, const bc::OODeclExists& op) {
  popC(env);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::VerifyParamType& op) {
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
  auto const constraint = env.ctx.func->params[op.loc1->id].typeConstraint;
  if (constraint.hasConstraint() && !constraint.isTypeVar() &&
      !constraint.isTypeConstant()) {
    FTRACE(2, "     {}\n", constraint.fullName());
    setLoc(env, op.loc1, env.index.lookup_constraint(env.ctx, constraint));
  }
}

void in(ISS& env, const bc::VerifyRetTypeV& op) {}

void in(ISS& env, const bc::VerifyRetTypeC& op) {
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
  if (!options.HardReturnTypeHints || constraint.isSoft()) {
    return;
  }

  // If we reach here, then HardReturnTypeHints is true AND the constraint
  // is not soft.  We can safely assume that either VerifyRetTypeC will
  // throw or it will produce a value whose type is compatible with the
  // return type constraint.
  auto tcT =
    remove_uninit(env.index.lookup_constraint(env.ctx, constraint));

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
  auto const retT = stackT.subtypeOf(tcT) ? stackT :
                    is_opt(tcT) && !stackT.couldBe(TInitNull) ? unopt(tcT) :
                    tcT;

  // Update the top of stack with the rough conservative approximate of the
  // intersection of stackT and tcT
  popC(env);
  push(env, retT);
}

// These only occur in traits, so we don't need to do better than
// this.
void in(ISS& env, const bc::Self&)   { push(env, TCls); }
void in(ISS& env, const bc::Parent&) { push(env, TCls); }

void in(ISS& env, const bc::CreateCl& op) {
  auto const nargs   = op.arg1;
  auto const clsPair = env.index.resolve_closure_class(env.ctx, op.str2);

  /*
   * Every closure should have a unique allocation site, but we may
   * see it multiple times in a given round of analyzing this
   * function.  Each time we may have more information about the used
   * variables; the types should only possibly grow.  If it's already
   * there we need to merge the used vars in with what we saw last
   * time.
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

  return push(env, objExact(clsPair.first));
}

void in(ISS& env, const bc::CreateCont& op) {
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

void in(ISS& env, const bc::ContCheck&)   {}
void in(ISS& env, const bc::ContValid&)   { push(env, TBool); }
void in(ISS& env, const bc::ContKey&)     { push(env, TInitCell); }
void in(ISS& env, const bc::ContCurrent&) { push(env, TInitCell); }

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

  auto const inner = wait_handle_inner(t);
  if (inner.subtypeOf(TBottom)) {
    // If it's a WaitH<Bottom>, we know it's going to throw an exception, and
    // the fallthrough code is not reachable.
    push(env, TBottom);
    unreachable(env);
    return;
  }

  push(env, inner);
}

void in(ISS& env, const bc::WHResult&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& env, const bc::Await&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& env, const bc::Strlen&) {
  auto const t1 = popC(env);
  auto const v1 = tv(t1);
  if (v1) {
    if (v1->m_type == KindOfStaticString) {
      constprop(env);
      return push(env, ival(v1->m_data.pstr->size()));
    }
    return push(env, TInitCell);
  }
  if (t1.subtypeOf(TStr)) { nothrow(env); return push(env, TInt); }
  push(env, TInitCell);
}

void in(ISS& env, const bc::IncStat&) {}

void in(ISS& env, const bc::Idx&) {
  popC(env); popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ArrayIdx&) {
  popC(env); popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::CheckProp&) { push(env, TBool); }

void in(ISS& env, const bc::InitProp& op) {
  auto const t = popC(env);
  switch (op.subop) {
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
}

void in(ISS& env, const bc::Silence& op) {
  nothrow(env);
  switch (op.subop) {
    case SilenceOp::Start:
      setLoc(env, op.loc1, TInt);
      break;
    case SilenceOp::End:
      break;
  }
}

void in(ISS& env, const bc::LowInvalid&)  { always_assert(!"LowInvalid"); }
void in(ISS& env, const bc::HighInvalid&) { always_assert(!"HighInvalid"); }

//////////////////////////////////////////////////////////////////////

void dispatch(ISS& env, const Bytecode& op) {
#define O(opcode, ...) case Op::opcode: in(env, op.opcode); return;
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
      ret += " " + show(it[i]);
      if (i != sizeof...(Args) - 1) ret += ';';
    }
    return ret;
  }());
  it += sizeof...(Args);
  return group(env, std::forward<Args>(args)...);
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
  default: break;
  }

  FTRACE(2, "  {}\n", show(*it));
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
  if (flags.wasPEI) {
    auto outputs_constant = [&] {
      auto const size = interp.state.stack.size();
      for (auto i = size_t{0}; i < numPushed; ++i) {
        if (!tv(interp.state.stack[size - i - 1])) return false;
      }
      return true;
    };

    if (flags.canConstProp && outputs_constant()) {
      FTRACE(2, "   nothrow (due to constprop)\n");
    } else {
      FTRACE(2, "   PEI.\n");
      for (auto& factored : interp.blk->factoredExits) {
        propagate(*factored, stateBefore);
      }
    }
  }
  return flags;
}

//////////////////////////////////////////////////////////////////////

}

RunFlags run(Interp& interp, PropagateFn propagate) {
  SCOPE_EXIT {
    FTRACE(2, "out {}\n", state_string(*interp.ctx.func, interp.state));
  };

  auto const stop = end(interp.blk->hhbcs);
  auto iter       = begin(interp.blk->hhbcs);
  while (iter != stop) {
    auto const flags = interpOps(interp, iter, stop, propagate);
    if (interp.state.unreachable) {
      FTRACE(2, "  <bytecode fallthrough is unreachable>\n");
      if (interp.state.fpiStack.empty()) {
        // We have no reason to continue running the interpreter if there's no
        // FPI region active.
        return RunFlags {};
      }
      continue;
    }

    switch (flags.jmpFlag) {
    case StepFlags::JmpFlags::Taken:
      FTRACE(2, "  <took branch; no fallthrough>\n");
      return RunFlags {};
    case StepFlags::JmpFlags::Fallthrough:
    case StepFlags::JmpFlags::Either:
      break;
    }
    if (flags.returned) {
      FTRACE(2, "  returned {}\n", show(*flags.returned));
      always_assert(iter == stop);
      always_assert(!interp.blk->fallthrough);
      return RunFlags { *flags.returned };
    }
  }

  FTRACE(2, "  <end block>\n");
  if (interp.blk->fallthrough) {
    propagate(*interp.blk->fallthrough, interp.state);
  }
  return RunFlags {};
}

StepFlags step(Interp& interp, const Bytecode& op) {
  auto flags   = StepFlags{};
  auto noop    = [] (php::Block&, const State&) {};
  ISS env { interp, flags, noop };
  dispatch(env, op);
  return flags;
}

void default_dispatch(ISS& env, const Bytecode& op) {
  dispatch(env, op);
}

//////////////////////////////////////////////////////////////////////

}}
