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
#ifndef incl_HHBBC_INTERP_H_
#define incl_HHBBC_INTERP_H_

#include <vector>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <bitset>

#include "folly/Optional.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/ext/ext_math.h" // f_abs

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/type-arith.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/analyze.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

const StaticString s_empty("");
const StaticString s_extract("extract");
const StaticString s_Exception("Exception");
const StaticString s_Continuation("Continuation");
const StaticString s_stdClass("stdClass");
const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");
const StaticString s_86pinit("86pinit"), s_86sinit("86sinit");

//////////////////////////////////////////////////////////////////////

/*
 * Each single-instruction step of the abstract interpreter sends
 * various information back to the caller in this structure.
 */
struct StepFlags {
  /*
   * Potentially Exception-throwing Instruction.
   *
   * Instructions are assumed to be PEIs unless the abstract
   * interpreter says they aren't.  A PEI must propagate the state
   * from before the instruction across all factored exit edges.
   *
   * Some instructions that can throw with mid-opcode states need to
   * handle those cases specially.
   */
  bool wasPEI = true;

  /*
   * If a conditional branch at the end of the BB was known to be
   * taken (e.g. because the condition was a constant), this flag
   * indicates the state doesn't need to be propagated to the
   * fallthrough block.
   */
  bool tookBranch = false;

  /*
   * If true, we made a call to a function that never returns.
   */
  bool calledNoReturn = false;

  /*
   * If an instruction sets this flag, it means that if it pushed a
   * type with a constant value, it had no side effects other than
   * computing the value which was pushed.  This means the instruction
   * can be replaced with pops of its inputs followed by a push of the
   * constant.
   */
  bool canConstProp = false;

  /*
   * If an instruction may read or write to locals, these flags
   * indicate which ones.  We don't track this information for local
   * ids past 64.
   *
   * This is currently only used to try to leave out unnecessary type
   * assertions on locals (for options.FilterAssertions).
   */
  std::bitset<64> mayReadLocalSet;

  /*
   * If the instruction on this step could've been replaced with
   * cheaper bytecode, this is the list of bytecode that can be used.
   */
  folly::Optional<std::vector<Bytecode>> strengthReduced;

  /*
   * If this is not none, the interpreter executed a return on this
   * step, with this type.
   */
  folly::Optional<Type> returned;
};

//////////////////////////////////////////////////////////////////////

/*
 * TODO: move to interp state?
 */

// Return a copy of a State without copying either the evaluation
// stack or FPI stack.
inline State without_stacks(State const& src) {
  auto ret = State{};
  ret.initialized       = src.initialized;
  ret.thisAvailable     = src.thisAvailable;
  ret.locals            = src.locals;
  return ret;
}

//////////////////////////////////////////////////////////////////////
// Some member instruction type-system predicates.

inline bool couldBeEmptyish(Type ty) {
  return ty.couldBe(TNull) ||
         ty.couldBe(sval(s_empty.get())) ||
         ty.couldBe(TFalse);
}

inline bool mustBeEmptyish(Type ty) {
  return ty.subtypeOf(TNull) ||
         ty.subtypeOf(sval(s_empty.get())) ||
         ty.subtypeOf(TFalse);
}

inline bool elemCouldPromoteToArr(Type ty) { return couldBeEmptyish(ty); }
inline bool propCouldPromoteToObj(Type ty) { return couldBeEmptyish(ty); }
inline bool elemMustPromoteToArr(Type ty)  { return mustBeEmptyish(ty); }
inline bool propMustPromoteToObj(Type ty)  { return mustBeEmptyish(ty); }

//////////////////////////////////////////////////////////////////////

template<class Propagate, class PropagateThrow>
struct InterpStepper : boost::static_visitor<void> {
  explicit InterpStepper(const Index* index,
                         Context ctx,
                         PropertiesInfo& props,
                         State& st,
                         StepFlags& flags,
                         Propagate propagate,
                         PropagateThrow propagateThrow)
    : m_index(*index)
    , m_ctx(ctx)
    , m_props(props)
    , m_state(st)
    , m_flags(flags)
    , m_propagate(propagate)
    , m_propagateThrow(propagateThrow)
  {}

  void operator()(const bc::Nop&)  { nothrow(); }
  void operator()(const bc::PopA&) { nothrow(); popA(); }
  void operator()(const bc::PopC&) { nothrow(); popC(); }
  void operator()(const bc::PopV&) { nothrow(); popV(); }
  void operator()(const bc::PopR&) {
    auto t = topT(0);
    if (t.subtypeOf(TCell)) {
      return reduce(bc::UnboxRNop {},
                    bc::PopC {});
    }
    nothrow(); popR();
  }

  void operator()(const bc::Dup& op) {
    nothrow();
    auto const val = popC();
    push(val);
    push(val);
  }

  void operator()(const bc::AssertTL&)       {}
  void operator()(const bc::AssertTStk&)     {}
  void operator()(const bc::AssertObjL&)     {}
  void operator()(const bc::AssertObjStk&)   {}
  void operator()(const bc::PredictTL&)      {}
  void operator()(const bc::PredictTStk&)    {}
  void operator()(const bc::BreakTraceHint&) {}

  void operator()(const bc::Box&)     { nothrow(); popC(); push(TRef); }
  void operator()(const bc::BoxR&)    { nothrow(); popR(); push(TRef); }
  void operator()(const bc::Unbox&)   { nothrow(); popV(); push(TInitCell); }

  void operator()(const bc::UnboxR&) {
    auto const t = topR();
    if (t.subtypeOf(TInitCell)) return reduce(bc::UnboxRNop {});
    nothrow();
    popT();
    push(TInitCell);
  }

  void operator()(const bc::UnboxRNop&) {
    nothrow();
    auto const t = popR();
    push(t.subtypeOf(TInitCell) ? t : TInitCell);
  }

  void operator()(const bc::BoxRNop&) {
    nothrow();
    auto const t = popR();
    push(t.subtypeOf(TRef) ? t : TRef);
  }

  void operator()(const bc::Null&)      { nothrow(); push(TInitNull); }
  void operator()(const bc::NullUninit&){ nothrow(); push(TUninit); }
  void operator()(const bc::True&)      { nothrow(); push(TTrue); }
  void operator()(const bc::False&)     { nothrow(); push(TFalse); }
  void operator()(const bc::Int&    op) { nothrow(); push(ival(op.arg1)); }
  void operator()(const bc::Double& op) { nothrow(); push(dval(op.dbl1)); }
  void operator()(const bc::String& op) { nothrow(); push(sval(op.str1)); }
  void operator()(const bc::Array&  op) { nothrow(); push(aval(op.arr1)); }

  void operator()(const bc::NewArray&)        { push(TArr); }
  void operator()(const bc::NewArrayReserve&) { push(TArr); }

  void operator()(const bc::NewPackedArray& op) {
    for (auto i = uint32_t{0}; i < op.arg1; ++i) popT();
    push(TArr);
  }

  void operator()(const bc::NewStructArray& op) {
    for (auto n = op.keys.size(); n > 0; n--) popC();
    push(TArr);
  }

  void operator()(const bc::AddElemC& op) {
    popC(); popC(); popC();
    push(TArr);
  }
  void operator()(const bc::AddElemV& op) {
    popV(); popC(); popC();
    push(TArr);
  }

  void operator()(const bc::AddNewElemC&) { popC(); popC(); push(TArr); }
  void operator()(const bc::AddNewElemV&) { popV(); popC(); push(TArr); }

  void operator()(const bc::NewCol& op) {
    auto const name = collectionTypeToString(op.arg1);
    push(objExact(m_index.builtin_class(name)));
  }

  void operator()(const bc::ColAddElemC&) {
    popC(); popC();
    auto const coll = popC();
    push(coll);
  }
  void operator()(const bc::ColAddNewElemC&) {
    popC();
    auto const coll = popC();
    push(coll);
  }

  // Note: unlike class constants, these can be dynamic system
  // constants, so this doesn't have to be TInitUnc.
  void operator()(const bc::Cns&)  { push(TInitCell); }
  void operator()(const bc::CnsE&) { push(TInitCell); }
  void operator()(const bc::CnsU&) { push(TInitCell); }

  void operator()(const bc::ClsCns& op) {
    auto const t1 = topA();
    if (t1.strictSubtypeOf(TCls)) {
      auto const dcls = dcls_of(t1);
      if (dcls.type == DCls::Exact) {
        return reduce(bc::PopA {},
                      bc::ClsCnsD { op.str1, dcls.cls.name() });
      }
    }
    popA();
    push(TInitUnc);
  }

  void operator()(const bc::ClsCnsD& op) {
    if (!options.HardConstProp) return push(TInitUnc);
    if (auto const rcls = m_index.resolve_class(m_ctx, op.str2)) {
      auto const t = m_index.lookup_class_constant(m_ctx, *rcls, op.str1);
      constprop();
      push(t);
      return;
    }
    push(TInitUnc);
  }

  void operator()(const bc::File&)  { nothrow(); push(TSStr); }
  void operator()(const bc::Dir&)   { nothrow(); push(TSStr); }
  void operator()(const bc::NameA&) { nothrow(); popA(); push(TSStr); }

  void operator()(const bc::Concat& op) {
    auto const t1 = popC();
    auto const t2 = popC();
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);
    if (v1 && v2) {
      if (v1->m_type == KindOfStaticString &&
          v2->m_type == KindOfStaticString) {
        constprop();
        return push(eval_cell([&] {
          String s(StringData::Make(v2->m_data.pstr,
                                    v1->m_data.pstr->slice()));
          return make_tv<KindOfString>(s.detach());
        }));
      }
    }
    // Not nothrow even if both are strings: can throw for strings
    // that are too large.
    push(TStr);
  }

  template<class Op, class Fun>
  void arithImpl(const Op& op, Fun fun) {
    constprop();
    auto const t1 = popC();
    auto const t2 = popC();
    push(fun(t2, t1));
  }

  void operator()(const bc::Add& op)    { arithImpl(op, typeAdd); }
  void operator()(const bc::Sub& op)    { arithImpl(op, typeSub); }
  void operator()(const bc::Mul& op)    { arithImpl(op, typeMul); }
  void operator()(const bc::Div& op)    { arithImpl(op, typeDiv); }
  void operator()(const bc::Mod& op)    { arithImpl(op, typeMod); }
  void operator()(const bc::BitAnd& op) { arithImpl(op, typeBitAnd); }
  void operator()(const bc::BitOr& op)  { arithImpl(op, typeBitOr); }
  void operator()(const bc::BitXor& op) { arithImpl(op, typeBitXor); }

  template<class Op, class Fun>
  void shiftImpl(const Op& op, Fun fop) {
    constprop();
    auto const v1 = tv(typeToInt(popC()));
    auto const v2 = tv(typeToInt(popC()));
    if (v1 && v2) {
      return push(eval_cell([&] {
        return make_tv<KindOfInt64>(fop(cellToInt(*v2), cellToInt(*v1)));
      }));
    }
    push(TInt);
  }

  void operator()(const bc::Shl& op) {
    shiftImpl(op, [&] (int64_t a, int64_t b) { return a << b; });
  }

  void operator()(const bc::Shr& op) {
    shiftImpl(op, [&] (int64_t a, int64_t b) { return a >> b; });
  }

  void operator()(const bc::BitNot& op) {
    auto const t = popC();
    auto const v = tv(t);
    if (v) {
      constprop();
      return push(eval_cell([&] {
        auto c = *v;
        cellBitNot(c);
        return c;
      }));
    }
    push(TInitCell);
  }

  template<bool Negate>
  void sameImpl() {
    nothrow();

    auto const t1 = popC();
    auto const t2 = popC();
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);
    if (v1 && v2) {
      return push(cellSame(*v2, *v1) != Negate ? TTrue : TFalse);
    }
    if (!t1.couldBe(t2) && !t2.couldBe(t1)) {
      return push(false != Negate ? TTrue : TFalse);
    }
    push(TBool);
  }

  void operator()(const bc::Same&)  { sameImpl<false>(); }
  void operator()(const bc::NSame&) { sameImpl<true>(); }

  template<class Fun>
  void binOpBoolImpl(Fun fun) {
    auto const t1 = popC();
    auto const t2 = popC();
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);
    if (v1 && v2) {
      constprop();
      return push(fun(*v2, *v1) ? TTrue : TFalse);
    }
    // TODO_4: evaluate when these can throw, non-constant type stuff.
    push(TBool);
  }

  void operator()(const bc::Eq&) {
    binOpBoolImpl([&] (Cell c1, Cell c2) { return cellEqual(c1, c2); });
  }
  void operator()(const bc::Neq&) {
    binOpBoolImpl([&] (Cell c1, Cell c2) { return !cellEqual(c1, c2); });
  }
  void operator()(const bc::Lt&) {
    binOpBoolImpl([&] (Cell c1, Cell c2) { return cellLess(c1, c2); });
  }
  void operator()(const bc::Gt&) {
    binOpBoolImpl([&] (Cell c1, Cell c2) { return cellGreater(c1, c2); });
  }
  void operator()(const bc::Lte&) { binOpBoolImpl(cellLessOrEqual); }
  void operator()(const bc::Gte&) { binOpBoolImpl(cellGreaterOrEqual); }

  void operator()(const bc::Xor&) {
    binOpBoolImpl([&] (Cell c1, Cell c2) {
      return cellToBool(c1) ^ cellToBool(c2);
    });
  }

  void castBoolImpl(bool negate) {
    nothrow();
    auto const t = popC();
    auto const v = tv(t);
    if (v) {
      constprop();
      return push(eval_cell([&] {
        return make_tv<KindOfBoolean>(cellToBool(*v) != negate);
      }));
    }
    push(TBool);
  }

  void operator()(const bc::Not&)      { castBoolImpl(true); }
  void operator()(const bc::CastBool&) { castBoolImpl(false); }

  void operator()(const bc::CastInt&) {
    auto const t = popC();
    if (!t.couldBe(TObj) && !t.couldBe(TRes)) nothrow();
    auto const v = tv(t);
    if (v) {
      constprop();
      return push(eval_cell([&] {
        return make_tv<KindOfInt64>(cellToInt(*v));
      }));
    }
    push(TInt);
  }

  void operator()(const bc::CastDouble&) { popC(); push(TDbl); }
  void operator()(const bc::CastString&) { popC(); push(TStr); }
  void operator()(const bc::CastArray&)  { popC(); push(TArr); }
  void operator()(const bc::CastObject&) { popC(); push(TObj); }

  void operator()(const bc::Print& op) { popC(); push(ival(1)); }

  void operator()(const bc::Clone& op) {
    auto const val = popC();
    push(val.subtypeOf(TObj) ? val :
         is_opt(val)         ? unopt(val) :
         TObj);
  }

  void operator()(const bc::Exit&)  { popC(); push(TInitNull); }
  void operator()(const bc::Fatal&) { popC(); }

  void operator()(const bc::JmpNS&) {
    always_assert(0 && "blocks should not contain JmpNS instructions");
  }

  void operator()(const bc::Jmp&) {
    always_assert(0 && "blocks should not contain Jmp instructions");
  }

  template<bool Negate, class Op>
  void jmpImpl(const Op& op) {
    nothrow();
    auto const t1 = popC();
    auto const v1 = tv(t1);
    if (v1) {
      auto const taken = !cellToBool(*v1) != Negate;
      if (taken) {
        nofallthrough();
        m_propagate(*op.target, m_state);
      }
      return;
    }
    m_propagate(*op.target, m_state);
  }

  void operator()(const bc::JmpNZ& op) { jmpImpl<true>(op); }
  void operator()(const bc::JmpZ& op)  { jmpImpl<false>(op); }

  void group(const bc::AsyncAwait& await, const bc::JmpNZ& jmp) {
    auto const t = topC();
    if (!is_specialized_wait_handle(t) || is_opt(t)) {
      return impl(await, jmp);
    }
    auto const inner = wait_handle_inner(t);
    if (inner.subtypeOf(TBottom)) {
      // It's always going to throw.
      return impl(await, jmp);
    }

    popC();
    push(wait_handle_inner(t));
    m_propagate(*jmp.target, m_state);
    popC();
    push(t);
  }

  template<class JmpOp>
  void group(const bc::IsTypeL& istype, const JmpOp& jmp) {
    if (istype.subop == IsTypeOp::Scalar) return impl(istype, jmp);

    auto const loc = derefLoc(istype.loc1);
    auto const testTy = type_of_istype(istype.subop);
    if (loc.subtypeOf(testTy) || !loc.couldBe(testTy)) {
      return impl(istype, jmp);
    }

    if (!locCouldBeUninit(istype.loc1)) nothrow();

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

    setLoc(istype.loc1, negate ? was_true : was_false);
    m_propagate(*jmp.target, m_state);
    setLoc(istype.loc1, negate ? was_false : was_true);
  }

  template<class JmpOp>
  void group(const bc::CGetL& cgetl, const JmpOp& jmp) {
    auto const loc = derefLoc(cgetl.loc1);
    if (tv(loc)) return impl(cgetl, jmp);

    if (!locCouldBeUninit(cgetl.loc1)) nothrow();

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

    setLoc(cgetl.loc1, negate ? converted_true : converted_false);
    m_propagate(*jmp.target, m_state);
    setLoc(cgetl.loc1, negate ? converted_false : converted_true);
  }

  template<class JmpOp>
  void group(const bc::CGetL& cgetl,
             const bc::InstanceOfD& inst,
             const JmpOp& jmp) {
    auto bail = [&] { this->impl(cgetl, inst, jmp); };

    if (interface_supports_non_objects(inst.str1)) return bail();
    auto const rcls = m_index.resolve_class(m_ctx, inst.str1);
    if (!rcls) return bail();

    auto const instTy = subObj(*rcls);
    auto const loc = derefLoc(cgetl.loc1);
    if (loc.subtypeOf(instTy) || !loc.couldBe(instTy)) {
      return bail();
    }

    auto const negate    = jmp.op == Op::JmpNZ;
    auto const was_true  = instTy;
    auto const was_false = loc;
    setLoc(cgetl.loc1, negate ? was_true : was_false);
    m_propagate(*jmp.target, m_state);
    setLoc(cgetl.loc1, negate ? was_false : was_true);
  }

  void operator()(const bc::Switch& op) {
    popC();
    forEachTakenEdge(op, [&] (php::Block& blk) {
      m_propagate(blk, m_state);
    });
  }

  void operator()(const bc::SSwitch& op) {
    popC();
    forEachTakenEdge(op, [&] (php::Block& blk) {
      m_propagate(blk, m_state);
    });
  }

  void operator()(const bc::RetC& op)    { doRet(popC()); }
  void operator()(const bc::RetV& op)    { doRet(popV()); }
  void operator()(const bc::Unwind& op)  {}
  void operator()(const bc::Throw& op)   { popC(); }

  void operator()(const bc::Catch&) {
    nothrow();
    return push(subObj(m_index.builtin_class(s_Exception.get())));
  }

  void operator()(const bc::NativeImpl&) {
    killLocals();
    if (m_ctx.func->nativeInfo) {
      auto const dt = m_ctx.func->nativeInfo->returnType;
      if (dt != KindOfInvalid) {
        // TODO(#3568043): adding TInitNull, because HNI doesn't know
        // about nullability.
        return doRet(union_of(from_DataType(dt), TInitNull));
      }
    }
    doRet(TInitGen);
  }

  void operator()(const bc::CGetL& op) {
    if (!locCouldBeUninit(op.loc1)) { nothrow(); constprop(); }
    push(locAsCell(op.loc1));
  }

  void operator()(const bc::PushL& op) {
    impl(bc::CGetL { op.loc1 }, bc::UnsetL { op.loc1 });
  }

  void operator()(const bc::CGetL2& op) {
    // Can't constprop yet because of no INS_1 support in bc.h
    if (!locCouldBeUninit(op.loc1)) nothrow();
    auto const loc = locAsCell(op.loc1);
    auto const top = popT();
    push(loc);
    push(top);
  }

  void operator()(const bc::CGetL3& op) {
    // Can't constprop yet because of no INS_2 support in bc.h
    if (!locCouldBeUninit(op.loc1)) nothrow();
    auto const loc = locAsCell(op.loc1);
    auto const t1 = popT();
    auto const t2 = popT();
    push(loc);
    push(t2);
    push(t1);
  }

  void operator()(const bc::CGetN&) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      if (auto const loc = findLocal(v1->m_data.pstr)) {
        return reduce(bc::PopC {},
                      bc::CGetL { loc });
      }
    }
    readUnknownLocals();
    popC(); // conversion to string can throw
    push(TInitCell);
  }

  void operator()(const bc::CGetG&) { popC(); push(TInitCell); }

  void operator()(const bc::CGetS&) {
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();
    if (vname && vname->m_type == KindOfStaticString &&
        self && tcls.subtypeOf(*self)) {
      if (auto const ty = selfPropAsCell(vname->m_data.pstr)) {
        // Only nothrow when we know it's a private declared property
        // (and thus accessible here).
        nothrow();
        return push(*ty);
      }
    }
    push(TInitCell);
  }

  void operator()(const bc::VGetL& op) {
    nothrow();
    setLocRaw(op.loc1, TRef);
    push(TRef);
  }

  void operator()(const bc::VGetN&) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      if (auto const loc = findLocal(v1->m_data.pstr)) {
        return reduce(bc::PopC {},
                      bc::VGetL { loc });
      }
    }
    boxUnknownLocal();
    push(TRef);
  }

  void operator()(const bc::VGetG&) { popC(); push(TRef); }

  void operator()(const bc::VGetS&) {
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();
    if (!self || tcls.couldBe(*self)) {
      if (vname && vname->m_type == KindOfStaticString) {
        boxSelfProp(vname->m_data.pstr);
      } else {
        killSelfProps();
      }
    }
    push(TRef);
  }

  void aGetImpl(Type t1) {
    if (t1.subtypeOf(TObj)) {
      nothrow();
      return push(objcls(t1));
    }
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      if (auto const rcls = m_index.resolve_class(m_ctx, v1->m_data.pstr)) {
        return push(clsExact(*rcls));
      }
    }
    push(TCls);
  }

  void operator()(const bc::AGetL& op) { aGetImpl(locAsCell(op.loc1));}
  void operator()(const bc::AGetC& op) { aGetImpl(popC()); }

  void operator()(const bc::AKExists& op) {
    auto const t1   = popC();
    auto const t2   = popC();
    auto const t1Ok = t1.subtypeOf(TObj) || t1.subtypeOf(TArr);
    auto const t2Ok = t2.subtypeOf(TInt) || t2.subtypeOf(TNull) ||
                      t2.subtypeOf(TStr);
    if (t1Ok && t2Ok) nothrow();
    push(TBool);
  }

  void operator()(const bc::IssetL& op) {
    nothrow();
    constprop();
    auto const loc = locAsCell(op.loc1);
    if (loc.subtypeOf(TNull))  return push(TFalse);
    if (!loc.couldBe(TNull))   return push(TTrue);
    push(TBool);
  }

  void operator()(const bc::EmptyL& op) {
    nothrow();
    constprop();
    if (!locCouldBeUninit(op.loc1)) {
      return impl(bc::CGetL { op.loc1 },
                  bc::Not {});
    }
    locAsCell(op.loc1); // read the local
    push(TBool);
  }

  void operator()(const bc::EmptyS&) {
    popA();
    popC();
    push(TBool);
  }

  void operator()(const bc::IssetS&) {
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();
    if (self && tcls.subtypeOf(*self) &&
        vname && vname->m_type == KindOfStaticString) {
      if (auto const t = selfPropAsCell(vname->m_data.pstr)) {
        if (t->subtypeOf(TNull))  { constprop(); return push(TFalse); }
        if (!t->couldBe(TNull))   { constprop(); return push(TTrue); }
      }
    }
    push(TBool);
  }

  template<class ReduceOp>
  void issetEmptyNImpl() {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      if (auto const loc = findLocal(v1->m_data.pstr)) {
        return reduce(bc::PopC {},
                      ReduceOp { loc });
      }
      // Can't push true in the non findLocal case unless we know
      // whether this function can have a VarEnv.
    }
    readUnknownLocals();
    popC();
    push(TBool);
  }

  void operator()(const bc::IssetN&) { issetEmptyNImpl<bc::IssetL>(); }
  void operator()(const bc::EmptyN&) { issetEmptyNImpl<bc::EmptyL>(); }
  void operator()(const bc::EmptyG&) { popC(); push(TBool); }
  void operator()(const bc::IssetG&) { popC(); push(TBool); }

  void isTypeImpl(Type locOrCell, Type test) {
    constprop();
    if (locOrCell.subtypeOf(test))  return push(TTrue);
    if (!locOrCell.couldBe(test))   return push(TFalse);
    push(TBool);
  }

  template<class Op>
  void isTypeLImpl(const Op& op) {
    if (!locCouldBeUninit(op.loc1)) { nothrow(); constprop(); }
    auto const loc = locAsCell(op.loc1);
    if (op.subop == IsTypeOp::Scalar) {
      return push(TBool);
    }
    isTypeImpl(loc, type_of_istype(op.subop));
  }

  template<class Op>
  void isTypeCImpl(const Op& op) {
    nothrow();
    auto const t1 = popC();
    if (op.subop == IsTypeOp::Scalar) {
      return push(TBool);
    }
    isTypeImpl(t1, type_of_istype(op.subop));
  }

  void operator()(const bc::IsTypeC& op) { isTypeCImpl(op); }
  void operator()(const bc::IsTypeL& op) { isTypeLImpl(op); }

  void operator()(const bc::InstanceOfD& op) {
    auto const t1 = popC();
    // Note: InstanceOfD can do autoload if the type might be a type
    // alias, so it's not nothrow unless we know it's an object type.
    if (auto const rcls = m_index.resolve_class(m_ctx, op.str1)) {
      nothrow();
      if (!interface_supports_non_objects(rcls->name())) {
        isTypeImpl(t1, subObj(*rcls));
        return;
      }
    }
    push(TBool);
  }

  void operator()(const bc::InstanceOf& op) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      return reduce(bc::PopC {},
                    bc::InstanceOfD { v1->m_data.pstr });
    }
    // Ignoring t1-is-an-object case.
    popC();
    popC();
    push(TBool);
  }

  void operator()(const bc::SetL& op) {
    nothrow();
    auto const val = popC();
    setLoc(op.loc1, val);
    push(val);
  }

  void operator()(const bc::SetN&) {
    // This isn't trivial to strength reduce, without a "flip two top
    // elements of stack" opcode.
    auto const t1 = popC();
    auto const t2 = popC();
    auto const v2 = tv(t2);
    // TODO(#3653110): could nothrow if t2 can't be an Obj or Res

    auto const knownLoc = v2 && v2->m_type == KindOfStaticString
      ? findLocal(v2->m_data.pstr)
      : nullptr;
    if (knownLoc) {
      setLoc(knownLoc, t1);
    } else {
      // We could be changing the value of any local, but we won't
      // change whether or not they are boxed or initialized.
      loseNonRefLocalTypes();
    }
    push(t1);
  }

  void operator()(const bc::SetG&) {
    auto const t1 = popC();
    popC();
    push(t1);
  }

  void operator()(const bc::SetS&) {
    auto const t1    = popC();
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();
    if (!self || tcls.couldBe(*self)) {
      if (vname && vname->m_type == KindOfStaticString) {
        nothrow();
        mergeSelfProp(vname->m_data.pstr, t1);
      } else {
        mergeEachSelfPropRaw([&] (Type) { return t1; });
      }
    }
    push(t1);
  }

  void operator()(const bc::SetOpL& op) {
    auto const t1     = popC();
    auto const v1     = tv(t1);
    auto const loc    = locAsCell(op.loc1);
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

      // We may have inferred a TSStr or TSArr with a value here, but
      // at runtime it will not be static.  For now just throw that
      // away.
      if (resultTy.subtypeOf(TStr))      resultTy = TStr;
      else if (resultTy.subtypeOf(TArr)) resultTy = TArr;

      setLoc(op.loc1, resultTy);
      push(resultTy);
      return;
    }

    setLoc(op.loc1, TInitCell);
    push(TInitCell);
  }

  void operator()(const bc::SetOpN&) {
    popC();
    popC();
    loseNonRefLocalTypes();
    push(TInitCell);
  }

  void operator()(const bc::SetOpG&) {
    popC(); popC();
    push(TInitCell);
  }

  void operator()(const bc::SetOpS&) {
    popC();
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();

    if (!self || tcls.couldBe(*self)) {
      if (vname && vname->m_type == KindOfStaticString) {
        mergeSelfProp(vname->m_data.pstr, TInitCell);
      } else {
        loseNonRefSelfPropTypes();
      }
    }

    push(TInitCell);
  }

  void operator()(const bc::IncDecL& op) {
    auto const loc = locAsCell(op.loc1);
    auto const val = tv(loc);
    if (!val) {
      // Only tracking IncDecL for constants for now.
      setLoc(op.loc1, TInitCell);
      return push(TInitCell);
    }

    auto const subop = op.subop;
    auto const pre = subop == IncDecOp::PreInc || subop == IncDecOp::PreDec;
    auto const inc = subop == IncDecOp::PreInc || subop == IncDecOp::PostInc;

    if (!pre) push(loc);

    // We can't constprop with this eval_cell, because of the effects
    // on locals.
    auto resultTy = eval_cell([inc,val] {
      auto c = *val;
      if (inc) {
        cellInc(c);
      } else {
        cellDec(c);
      }
      return c;
    });

    // We may have inferred a TSStr or TSArr with a value here, but at
    // runtime it will not be static.
    resultTy = loosen_statics(resultTy);

    if (pre) push(resultTy);

    setLoc(op.loc1, resultTy);
  }

  void operator()(const bc::IncDecN& op) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    auto const knownLoc = v1 && v1->m_type == KindOfStaticString
      ? findLocal(v1->m_data.pstr)
      : nullptr;
    if (knownLoc) {
      return reduce(bc::PopC {},
                    bc::IncDecL { knownLoc, op.subop });
    }
    popC();
    loseNonRefLocalTypes();
    push(TInitCell);
  }

  void operator()(const bc::IncDecG&) { popC(); push(TInitCell); }

  void operator()(const bc::IncDecS&) {
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();

    if (!self || tcls.couldBe(*self)) {
      if (vname && vname->m_type == KindOfStaticString) {
        mergeSelfProp(vname->m_data.pstr, TInitCell);
      } else {
        loseNonRefSelfPropTypes();
      }
    }
    push(TInitCell);
  }

  void operator()(const bc::BindL& op) {
    nothrow();
    auto const t1 = popV();
    setLocRaw(op.loc1, t1);
    push(t1);
  }

  void operator()(const bc::BindN&) {
    // TODO(#3653110): could nothrow if t2 can't be an Obj or Res
    auto const t1 = popV();
    auto const t2 = popC();
    auto const v2 = tv(t2);
    auto const knownLoc = v2 && v2->m_type == KindOfStaticString
      ? findLocal(v2->m_data.pstr)
      : nullptr;
    if (knownLoc) {
      setLocRaw(knownLoc, t1);
    } else {
      boxUnknownLocal();
    }
    push(t1);
  }

  void operator()(const bc::BindG&) {
    auto const t1 = popV();
    popC();
    push(t1);
  }

  void operator()(const bc::BindS&) {
    popV();
    auto const tcls  = popA();
    auto const tname = popC();
    auto const vname = tv(tname);
    auto const self  = selfCls();

    if (!self || tcls.couldBe(*self)) {
      if (vname && vname->m_type == KindOfStaticString) {
        boxSelfProp(vname->m_data.pstr);
      } else {
        killSelfProps();
      }
    }

    push(TRef);
  }

  void operator()(const bc::EmptyM& op)       { minstr(op); }
  void operator()(const bc::IssetM& op)       { minstr(op); }
  void operator()(const bc::CGetM& op)        { minstr(op); }
  void operator()(const bc::VGetM& op)        { minstr(op); }
  void operator()(const bc::SetM& op)         { minstr(op); }
  void operator()(const bc::SetWithRefLM& op) { minstr(op); }
  void operator()(const bc::SetWithRefRM& op) { minstr(op); }
  void operator()(const bc::SetOpM& op)       { minstr(op); }
  void operator()(const bc::IncDecM& op)      { minstr(op); }
  void operator()(const bc::UnsetM& op)       { minstr(op); }
  void operator()(const bc::BindM& op)        { minstr(op); }

  void operator()(const bc::UnsetL& op) {
    nothrow();
    setLocRaw(op.loc1, TUninit);
  }

  void operator()(const bc::UnsetN& op) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      if (auto const loc = findLocal(v1->m_data.pstr)) {
        return reduce(bc::PopC {},
                      bc::UnsetL { loc });
      }
    }
    popC();
    if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow();
    unsetUnknownLocal();
  }

  void operator()(const bc::UnsetG& op) {
    auto const t1 = popC();
    if (!t1.couldBe(TObj) && !t1.couldBe(TRes)) nothrow();
  }

  void operator()(const bc::FPushFuncD& op) {
    auto const rfunc = m_index.resolve_func(m_ctx, op.str2);
    fpiPush(ActRec { FPIKind::Func, rfunc });
  }

  void operator()(const bc::FPushFunc& op) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      auto const name = normalizeNS(v1->m_data.pstr);
      if (isNSNormalized(name)) {
        return reduce(bc::PopC {},
                      bc::FPushFuncD { op.arg1, name });
      }
    }
    popC();
    if (t1.subtypeOf(TObj)) return fpiPush(ActRec { FPIKind::ObjInvoke });
    if (t1.subtypeOf(TArr)) return fpiPush(ActRec { FPIKind::CallableArr });
    if (t1.subtypeOf(TStr)) return fpiPush(ActRec { FPIKind::Func });
    fpiPush(ActRec { FPIKind::Unknown });
  }

  void operator()(const bc::FPushFuncU& op) {
    fpiPush(ActRec { FPIKind::Func });
  }

  void operator()(const bc::FPushObjMethodD& op) {
    auto const obj = popC();
    fpiPush(ActRec {
      FPIKind::ObjMeth,
      obj.subtypeOf(TObj) ? m_index.resolve_method(m_ctx, objcls(obj), op.str2)
                          : folly::none
    });
  }

  void operator()(const bc::FPushObjMethod& op) {
    auto const t1 = topC();
    auto const v1 = tv(t1);
    if (v1 && v1->m_type == KindOfStaticString) {
      return reduce(bc::PopC {},
                    bc::FPushObjMethodD { op.arg1, v1->m_data.pstr });
    }
    popC();
    popC();
    fpiPush(ActRec { FPIKind::ObjMeth });
  }

  void operator()(const bc::FPushClsMethodD& op) {
    auto const rcls = m_index.resolve_class(m_ctx, op.str3);
    auto const rfun =
      rcls ? m_index.resolve_method(m_ctx, clsExact(*rcls), op.str2)
           : folly::none;
    fpiPush(ActRec { FPIKind::ClsMeth, rfun });
  }

  void operator()(const bc::FPushClsMethod& op) {
    auto const t1 = popA();
    auto const t2 = popC();
    auto const v2 = tv(t2);
    auto const rfunc =
      v2 && v2->m_type == KindOfStaticString
        ? m_index.resolve_method(m_ctx, t1, v2->m_data.pstr)
        : folly::none;
    fpiPush(ActRec { FPIKind::ClsMeth, rfunc });
  }

  void operator()(const bc::FPushClsMethodF& op) {
    // The difference with FPushClsMethod is what ends up on the
    // ActRec (late-bound class), which we currently aren't tracking.
    impl(bc::FPushClsMethod { op.arg1 });
  }

  void operator()(const bc::FPushCtorD& op) {
    auto const rcls = m_index.resolve_class(m_ctx, op.str2);
    push(rcls ? objExact(*rcls) : TObj);
    auto const rfunc = rcls ? m_index.resolve_ctor(m_ctx, *rcls) : folly::none;
    fpiPush(ActRec { FPIKind::Ctor, rfunc });
  }

  void operator()(const bc::FPushCtor& op) {
    auto const t1 = topA();
    if (t1.strictSubtypeOf(TCls)) {
      auto const dcls = dcls_of(t1);
      if (dcls.type == DCls::Exact) {
        return reduce(bc::PopA {},
                      bc::FPushCtorD { op.arg1, dcls.cls.name() });
      }
    }
    popA();
    push(TObj);
    fpiPush(ActRec { FPIKind::Ctor });
  }

  void operator()(const bc::FPushCufIter&) {
    nothrow();
    fpiPush(ActRec { FPIKind::Unknown });
  }

  void operator()(const bc::FPushCuf&) {
    popC();
    fpiPush(ActRec { FPIKind::Unknown });
  }
  void operator()(const bc::FPushCufF&) {
    popC();
    fpiPush(ActRec { FPIKind::Unknown });
  }

  void operator()(const bc::FPushCufSafe&) {
    auto const t1 = popC();
    popC();
    push(t1);
    fpiPush(ActRec { FPIKind::Unknown });
    push(TBool);
  }

  void operator()(const bc::FPassL& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:
      if (!locCouldBeUninit(op.loc2)) nothrow();
      // This might box the local, we can't tell.  Note: if the local
      // is already TRef, we could try to leave it alone, but not for
      // now.
      setLocRaw(op.loc2, TGen);
      return push(TInitGen);
    case PrepKind::Val: return reduce(bc::CGetL { op.loc2 },
                                      bc::FPassC { op.arg1 });
    case PrepKind::Ref: return reduce(bc::VGetL { op.loc2 },
                                      bc::FPassVNop { op.arg1 });
    }
  }

  void operator()(const bc::FPassN& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:
      // This could change the type of any local.
      popC();
      killLocals();
      return push(TGen);
    case PrepKind::Val: return reduce(bc::CGetN {},
                                      bc::FPassC { op.arg1 });
    case PrepKind::Ref: return reduce(bc::VGetN {},
                                      bc::FPassVNop { op.arg1 });
    }
  }

  void operator()(const bc::FPassG& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: popC(); return push(TInitGen);
    case PrepKind::Val:     return reduce(bc::CGetG {},
                                          bc::FPassC { op.arg1 });
    case PrepKind::Ref:     return reduce(bc::VGetG {},
                                          bc::FPassVNop { op.arg1 });
    }
  }

  void operator()(const bc::FPassS& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:
      {
        auto const tcls  = popA();
        auto const self  = selfCls();
        auto const tname = popC();
        auto const vname = tv(tname);
        if (!self || tcls.couldBe(*self)) {
          if (vname && vname->m_type == KindOfStaticString) {
            // May or may not be boxing it, depending on the refiness.
            mergeSelfProp(vname->m_data.pstr, TInitGen);
          } else {
            killSelfProps();
          }
        }
      }
      return push(TGen);
    case PrepKind::Val:
      return reduce(bc::CGetS {}, bc::FPassC { op.arg1 });
    case PrepKind::Ref:
      return reduce(bc::VGetS {}, bc::FPassVNop { op.arg1 });
    }
  }

  void operator()(const bc::FPassV& op) {
    nothrow();
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: popV(); return push(TGen);
    case PrepKind::Val:     popV(); return push(TInitCell);
    case PrepKind::Ref:     assert(topT().subtypeOf(TRef)); return;
    }
  }

  void operator()(const bc::FPassR& op) {
    nothrow();
    if (topT().subtypeOf(TCell)) return reduce(bc::UnboxRNop {},
                                               bc::FPassC { op.arg1 });
    if (topT().subtypeOf(TRef))  return impl(bc::FPassV { op.arg1 });
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:      popR(); return push(TGen);
    case PrepKind::Val:          popR(); return push(TInitCell);
    case PrepKind::Ref:          popR(); return push(TRef);
    }
  }

  void operator()(const bc::FPassVNop&) { nothrow(); push(popV()); }
  void operator()(const bc::FPassC& op) { nothrow(); }

  void operator()(const bc::FPassCW& op) { impl(bc::FPassCE { op.arg1 }); }
  void operator()(const bc::FPassCE& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: return;
    case PrepKind::Val:     return reduce(bc::FPassC { op.arg1 });
    case PrepKind::Ref:     /* will warn/fatal at runtime */ return;
    }
  }

  void operator()(const bc::FPassM& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:
      break;
    case PrepKind::Val:
      return reduce(bc::CGetM { op.mvec }, bc::FPassC { op.arg1 });
    case PrepKind::Ref:
      return reduce(bc::VGetM { op.mvec }, bc::FPassVNop { op.arg1 });
    }

    /*
     * FPassM with an unknown PrepKind either has the effects of CGetM
     * or the effects of VGetM, but we don't know which statically.
     * These are complicated instructions, so the easiest way to
     * handle this is to run both and then merge their output states.
     */
    auto const start = m_state;
    (*this)(bc::CGetM { op.mvec });
    auto const cgetm = m_state;
    m_state = start;
    (*this)(bc::VGetM { op.mvec });
    merge_into(m_state, cgetm);
    assert(m_flags.wasPEI);
    assert(!m_flags.canConstProp);
  }

  void operator()(const bc::FCall& op) {
    for (auto i = uint32_t{0}; i < op.arg1; ++i) popF();
    auto const ar = fpiPop();
    specialFunctionEffects(ar);
    if (ar.func) {
      auto const ty = m_index.lookup_return_type(m_ctx, *ar.func);
      if (ty == TBottom) {
        // The callee function never returns.  It might throw, or loop
        // forever.
        calledNoReturn();
        // Right now we need to continue in some semi-sane state in
        // case we are in the middle of an FPI region or something
        // that must finish.  But we can't push a TBottom (there are
        // no values in that set), so we push something meaningless.
        return push(TInitGen);
      }
      return push(ty);
    }
    push(TInitGen);
  }

  void operator()(const bc::FCallArray& op) {
    popF();
    auto const ar = fpiPop();
    specialFunctionEffects(ar);
    if (ar.func) {
      return push(m_index.lookup_return_type(m_ctx, *ar.func));
    }
    push(TInitGen);
  }

  void operator()(const bc::FCallBuiltin& op) {
    for (auto i = uint32_t{0}; i < op.arg1; ++i) popT();
    specialFunctionEffects(op.str3);
    push(TInitGen);
  }

  void operator()(const bc::CufSafeArray&) {
    popR(); popC(); popC();
    push(TArr);
  }

  void operator()(const bc::CufSafeReturn&) {
    popR(); popC(); popC();
    push(TInitCell);
  }

  void operator()(const bc::DecodeCufIter&) { popC(); }

  void operator()(const bc::IterInit& op) {
    popC();
    // Take the branch before setting locals if the iter is already
    // empty, but after popping.  Similar for the other IterInits
    // below.
    m_propagate(*op.target, m_state);
    setLoc(op.loc3, TInitCell);
  }
  void operator()(const bc::MIterInit& op) {
    popV();
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TRef);
  }

  void operator()(const bc::IterInitK& op) {
    popC();
    m_propagate(*op.target, m_state);
    setLoc(op.loc3, TInitCell);
    setLoc(op.loc4, TInitCell);
  }
  void operator()(const bc::MIterInitK& op) {
    popV();
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TRef);
    setLoc(op.loc4, TInitCell);
  }

  void operator()(const bc::WIterInit& op) {
    popC();
    m_propagate(*op.target, m_state);
    // WIter* instructions may leave the value locals as either refs
    // or cells, depending whether the rhs of the assignment was a
    // ref.
    setLocRaw(op.loc3, TInitGen);
  }
  void operator()(const bc::WIterInitK& op) {
    popC();
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TInitGen);
    setLoc(op.loc4, TInitCell);
  }

  void operator()(const bc::IterNext& op) {
    m_propagate(*op.target, m_state);
    setLoc(op.loc3, TInitCell);
  }
  void operator()(const bc::MIterNext& op) {
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TRef);
  }

  void operator()(const bc::IterNextK& op) {
    m_propagate(*op.target, m_state);
    setLoc(op.loc3, TInitCell);
    setLoc(op.loc4, TInitCell);
  }
  void operator()(const bc::MIterNextK& op) {
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TRef);
    setLoc(op.loc4, TInitCell);
  }

  void operator()(const bc::WIterNext& op) {
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TInitGen);
  }
  void operator()(const bc::WIterNextK& op) {
    m_propagate(*op.target, m_state);
    setLocRaw(op.loc3, TInitGen);
    setLoc(op.loc4, TInitCell);
  }

  void operator()(const bc::IterFree&)  { nothrow(); }
  void operator()(const bc::MIterFree&) { nothrow(); }
  void operator()(const bc::CIterFree&) { nothrow(); }

  void operator()(const bc::IterBreak& op) {
    m_propagate(*op.target, m_state);
  }

  void inclOpImpl() {
    // Any include/require (or eval) op kills all locals, and private
    // properties.
    popC();
    killLocals();
    killThisProps();
    killSelfProps();
    push(TInitCell);
  }

  void operator()(const bc::Incl&)      { inclOpImpl(); }
  void operator()(const bc::InclOnce&)  { inclOpImpl(); }
  void operator()(const bc::Req&)       { inclOpImpl(); }
  void operator()(const bc::ReqOnce&)   { inclOpImpl(); }
  void operator()(const bc::ReqDoc&)    { inclOpImpl(); }
  void operator()(const bc::Eval&)      { inclOpImpl(); }

  void operator()(const bc::DefFunc&)      {}
  void operator()(const bc::DefCls&)       {}
  void operator()(const bc::NopDefCls&)    {}
  void operator()(const bc::DefCns&)       { popC(); push(TBool); }
  void operator()(const bc::DefTypeAlias&) {}

  void operator()(const bc::This&) {
    if (thisAvailable()) {
      return reduce(bc::BareThis { BareThisOp::NeverNull });
    }
    auto const ty = thisType();
    push(ty ? *ty : TObj);
    setThisAvailable();
  }

  void operator()(const bc::LateBoundCls&) {
    auto const ty = selfCls();
    push(ty ? *ty : TCls);
  }

  void operator()(const bc::CheckThis&) {
    if (thisAvailable()) {
      reduce(bc::Nop {});
    }
    setThisAvailable();
  }

  void operator()(const bc::BareThis& op) {
    if (thisAvailable()) {
      if (op.subop != BareThisOp::NeverNull) {
        return reduce(bc::BareThis { BareThisOp::NeverNull });
      }
    }

    auto const ty = thisType();
    switch (op.subop) {
    case BareThisOp::Notice:
      break;
    case BareThisOp::NoNotice:
      nothrow();
      break;
    case BareThisOp::NeverNull:
      nothrow();
      setThisAvailable();
      return push(ty ? *ty : TObj);
    }

    push(ty ? opt(*ty) : TOptObj);
  }

  void operator()(const bc::InitThisLoc& op) {
    setLocRaw(findLocalById(op.arg1), TCell);
  }

  void operator()(const bc::StaticLoc& op) {
    setLocRaw(findLocalById(op.arg1), TRef);
    push(TBool);
  }

  void operator()(const bc::StaticLocInit& op) {
    popC();
    setLocRaw(findLocalById(op.arg1), TRef);
  }

  /*
   * This can't trivially check that the class exists (e.g. via
   * resolve_class) without knowing either:
   *
   *    a) autoload is guaranteed to load it and t1 == true, or
   *    b) it's already defined in this unit.
   */
  void clsExistsImpl(Attr testAttr) { popC(); popC(); push(TBool); }

  void operator()(const bc::ClassExists&)     { clsExistsImpl(AttrNone); }
  void operator()(const bc::InterfaceExists&) { clsExistsImpl(AttrInterface); }
  void operator()(const bc::TraitExists&)     { clsExistsImpl(AttrTrait); }

  void operator()(const bc::VerifyParamType& op) {
    auto loc = findLocalById(op.arg1);
    locAsCell(loc);
    if (!options.HardTypeHints) return;

    /*
     * In HardTypeHints mode, we assume that if this opcode doesn't
     * throw, the parameter was of the specified type (although it may
     * have been a Ref if the parameter was by reference).
     *
     * The setLoc here handles dealing with a parameter that was
     * already known to be a reference.
     *
     * NB: VerifyParamType of a reference parameter can kill any
     * references if it re-enters, even if Option::HardTypeHints is
     * on.
     */
    auto const constraint = m_ctx.func->params[op.arg1].typeConstraint;
    if (constraint.hasConstraint() && !constraint.isTypeVar()) {
      FTRACE(2, "     {}\n", constraint.fullName());
      setLoc(loc, m_index.lookup_constraint(m_ctx, constraint));
    }
  }

  // These only occur in traits, so we don't need to do better than
  // this.
  void operator()(const bc::Self&)   { push(TCls); }
  void operator()(const bc::Parent&) { push(TCls); }

  void operator()(const bc::CreateCl& op) {
    auto const nargs = op.arg1;
    for (auto i = uint32_t{0}; i < nargs; ++i) {
      // TODO(#3599292): propagate these types into closure analysis
      // information.
      popT();
    }
    if (auto const rcls = m_index.resolve_class(m_ctx, op.str2)) {
      return push(objExact(*rcls));
    }
    push(TObj);
  }

  void operator()(const bc::CreateCont&) {
    killLocals();
    if (m_ctx.func->isClosureBody) {
      // Generator closures create functions *outside* the class that
      // the closure is in, and we haven't hooked that up to be part
      // of the class-at-a-time analysis.  So we need to kill
      // everything on $this/self.
      killThisProps();
      killSelfProps();
    }
    push(objExact(m_index.builtin_class(s_Continuation.get())));
  }

  void operator()(const bc::ContEnter&)   { popC(); }
  void operator()(const bc::UnpackCont&)  { readUnknownLocals();
                                            push(TInitCell); push(TInt); }
  void operator()(const bc::ContSuspend&) { readUnknownLocals();
                                            popC(); doRet(TInitGen); }
  void operator()(const bc::ContSuspendK&){ readUnknownLocals();
                                            popC(); popC(); doRet(TInitGen); }
  void operator()(const bc::ContRetC&)    { readUnknownLocals();
                                            popC(); doRet(TInitGen); }
  void operator()(const bc::ContCheck&)   {}
  void operator()(const bc::ContRaise&)   {}
  void operator()(const bc::ContValid&)   { push(TBool); }
  void operator()(const bc::ContKey&)     { push(TInitCell); }
  void operator()(const bc::ContCurrent&) { push(TInitCell); }
  void operator()(const bc::ContStopped&) {}
  void operator()(const bc::ContHandle&)  { popC(); }

  void operator()(const bc::AsyncAwait&) {
    // We handle this better if we manage to group the opcode.
    popC();
    push(TInitCell);
    push(TBool);
  }

  void operator()(const bc::AsyncWrapResult&) {
    auto const t = popC();
    push(wait_handle(m_index, t));
  }

  void operator()(const bc::AsyncESuspend&) {
    /*
     * A suspended async function WaitHandle must end up returning
     * whatever type we infer the eager function will return, so we
     * don't want it to influence that type.  Using WaitH<Bottom>
     * handles this, but note that it relies on the rule that the only
     * thing you can do with the output of this opcode is pass it to
     * RetC.
     */
    unsetNamedLocals();
    popC();
    push(wait_handle(m_index, TBottom));
  }

  void operator()(const bc::AsyncWrapException&) {
    // A wait handle which always throws when you join it is
    // represented as a WaitH<Bottom>.
    popC();
    push(wait_handle(m_index, TBottom));
  }

  void operator()(const bc::Strlen&) {
    auto const t1 = popC();
    auto const v1 = tv(t1);
    if (v1) {
      if (v1->m_type == KindOfStaticString) {
        constprop();
        return push(ival(v1->m_data.pstr->size()));
      }
      return push(TInitCell);
    }
    if (t1.subtypeOf(TStr)) { nothrow(); return push(TInt); }
    push(TInitCell);
  }

  void operator()(const bc::IncStat&) {}

  void operator()(const bc::Abs&) {
    auto const t1 = popC();
    auto const v1 = tv(t1);
    if (v1) {
      constprop();
      return push(eval_cell([&] {
        auto const cell = *v1;
        auto const ret = f_abs(tvAsCVarRef(&cell));
        assert(!IS_REFCOUNTED_TYPE(ret.asCell()->m_type));
        return *ret.asCell();
      }));
    }
    if (t1.subtypeOf(TInt)) return push(TInt);
    if (t1.subtypeOf(TDbl)) return push(TDbl);
    return push(TInitUnc);
  }

  void operator()(const bc::Idx&) {
    popC(); popC(); popC();
    push(TInitCell);
  }

  void operator()(const bc::ArrayIdx&) {
    popC(); popC(); popC();
    push(TInitCell);
  }

  template<class Op>
  void floatFnImpl(Op op, Type nonConstType) {
    auto const t1 = popC();
    auto const v1 = tv(t1);
    if (v1) {
      if (v1->m_type == KindOfDouble) {
        constprop();
        return push(dval(op(v1->m_data.dbl)));
      }
      if (v1->m_type == KindOfInt64) {
        constprop();
        return push(dval(op(static_cast<double>(v1->m_data.num))));
      }
    }
    push(nonConstType);
  }

  void operator()(const bc::Floor&) { floatFnImpl(floor, TDbl); }
  void operator()(const bc::Ceil&)  { floatFnImpl(ceil,  TDbl); }
  void operator()(const bc::Sqrt&)  { floatFnImpl(sqrt,  TInitUnc); }

  void operator()(const bc::CheckProp&) { push(TBool); }
  void operator()(const bc::InitProp& op) {
    auto const t = popC();
    switch (op.subop) {
      case InitPropOp::Static: {
        mergeSelfProp(op.str1, t);
      } break;
      case InitPropOp::NonStatic: {
        mergeThisProp(op.str1, t);
      } break;
    }
  }

  void operator()(const bc::LowInvalid&)  { always_assert(!"LowInvalid"); }
  void operator()(const bc::HighInvalid&) { always_assert(!"HighInvalid"); }

private: // member instructions
  /*
   * Tag indicating what sort of thing contains the current base.
   *
   * The base is always the unboxed version of the type, and its
   * location could be inside of a Ref.  So, for example, a base with
   * BaseLoc::Frame could be located inside of a Ref that is pointed
   * to by the Frame.  (We may want to distinguish these two cases at
   * some point if we start trying to track real information about
   * Refs, but not yet.)
   */
  enum class BaseLoc {
    /*
     * Base is in a number of possible places after an Elem op.  It
     * cannot possibly be in an object property (although it certainly
     * may alias one).  See miElem for details.
     *
     * If it is definitely in an array, the locTy in the Base will be
     * a subtype of TArr.
     */
    PostElem,

    /*
     * Base is in possible locations after a Prop op.  This means it
     * possibly lives in a property on an object, but possibly not
     * (e.g. it could be a null in tvScratch).  See miProp for
     * details.
     *
     * If it is definitely known to be a property in an object, the
     * locTy in the Base will be a subtype of TObj.
     */
    PostProp,

    /*
     * Known to be a static property on an object.  This is only
     * possible as an initial base.
     */
    StaticObjProp,

    /*
     * Known to be contained in the current frame as a local, as the
     * frame $this, by the evaluation stack, or inside $GLOBALS.  Only
     * possible as initial bases.
     */
    Frame,
    FrameThis,
    EvalStack,
    Global,

    /*
     * If we've execute an operation that's known to fatal, we use
     * this BaseLoc.
     */
    Fataled,
  };

  struct Base {
    Type type;
    BaseLoc loc;

    /*
     * We also need to track effects of intermediate dims on the type
     * of the base.  So we have a type and name information about the
     * base's container..
     *
     * For StaticObjProp, locName this is the name of the property if
     * known, or nullptr, and locTy is the type of the class
     * containing the static property.
     *
     * Similarly, if loc is PostProp, locName is the name of the
     * property if it was known, and locTy gives as much information
     * about the object type it is in.  (If we actually *know* it is
     * in an object, locTy will be a subtype of TObj.)
     */
    Type locTy;
    SString locName;
  };

  static std::string base_string(const Base& b) {
    auto const locStr = [&]() -> const char* {
      switch (b.loc) {
      case BaseLoc::PostElem:      return "PostElem";
      case BaseLoc::PostProp:      return "PostProp";
      case BaseLoc::StaticObjProp: return "StaticObjProp";
      case BaseLoc::Frame:         return "Frame";
      case BaseLoc::FrameThis:     return "FrameThis";
      case BaseLoc::EvalStack:     return "EvalStack";
      case BaseLoc::Global:        return "Global";
      case BaseLoc::Fataled:       return "Fataled";
      }
      not_reached();
    }();
    return folly::format(
      "{: <8}  ({: <14} {: <8} @ {})",
      show(b.type),
      locStr,
      show(b.locTy),
      b.locName ? b.locName->data() : "?"
    ).str();
  }

  struct MInstrState {
    explicit MInstrState(const MInstrInfo* info,
                         const MVector& mvec)
      : info(*info)
      , mvec(mvec)
      , stackIdx(info->valCount() + numVecPops(mvec))
    {}

    // Return the current MElem.  Only valid after the first base has
    // been processed.
    const MElem& melem() const {
      assert(mInd < mvec.mcodes.size());
      return mvec.mcodes[mInd];
    }

    // Return the current MemberCode.  Only valid after the first base
    // has been processed.
    MemberCode mcode() const { return melem().mcode; }

    const MInstrInfo& info;
    const MVector& mvec;

    // The current base.  Updated as we move through the vector
    // instruction.
    Base base;

    // Current index in mcodes vector (or 0 if we still haven't done
    // the base).
    uint32_t mInd = 0;

    // One above next stack slot to read, going forward in the mvec
    // (deeper to higher on stack).
    int32_t stackIdx;
  };

  //////////////////////////////////////////////////////////////////////

  /*
   * A note about bases.
   *
   * Generally inference needs to know two kinds of things about the
   * base to handle effects on tracked locations:
   *
   *   - Could the base be a location we're tracking deeper structure
   *     on, so the next operation actually affects something inside
   *     of it.  (For example, could the base be an object with the
   *     same type as $this.)
   *
   *   - Could the base be something (regardless of type) that is
   *     inside one of the things we're tracking.  I.e., the base
   *     might be whatever (an array or a bool or something), but
   *     living inside a property inside an object with the same type
   *     as $this.
   *
   * The first cases apply because final operations are going to
   * directly affect the type of these elements.  The second case is
   * because vector operations may change the base at each step if it
   * is a defining instruction.
   *
   * Note that both of these cases can apply to the same base: you
   * might have an object property on $this that could be an object of
   * the type of $this.
   *
   * The functions below with names "couldBeIn*" detect the second
   * case.  The effects on the tracked location in the second case are
   * handled in the functions with names "handleIn*{Prop,Elem,..}".
   * The effects for the first case are generally handled in the
   * miFinal op functions.
   */

  bool couldBeThisObj(const Base& b) const {
    if (b.loc == BaseLoc::Fataled) return false;
    auto const thisTy = thisType();
    return b.type.couldBe(thisTy ? *thisTy : TObj);
  }

  bool mustBeThisObj(const Base& b) const {
    if (b.loc == BaseLoc::FrameThis) return true;
    if (auto const ty = thisType())  return b.type.subtypeOf(*ty);
    return false;
  }

  bool couldBeInThis(const Base& b) const {
    if (b.loc != BaseLoc::PostProp) return false;
    auto const thisTy = thisType();
    if (!thisTy) return true;
    if (!b.locTy.couldBe(*thisTy)) return false;
    if (b.locName) {
      return isTrackedThisProp(b.locName);
    }
    return true;
  }

  bool couldBeInSelf(const Base& b) const {
    if (b.loc != BaseLoc::StaticObjProp) return false;
    auto const selfTy = selfCls();
    return !selfTy || b.locTy.couldBe(*selfTy);
  }

  //////////////////////////////////////////////////////////////////////

  void handleInThisPropD(MInstrState& state) {
    if (!couldBeInThis(state.base)) return;

    if (auto const name = state.base.locName) {
      auto const ty = thisPropAsCell(name);
      if (ty && propCouldPromoteToObj(*ty)) {
        mergeThisProp(name,
          objExact(m_index.builtin_class(s_stdClass.get())));
      }
      return;
    }

    mergeEachThisPropRaw([&] (Type t) {
      return propCouldPromoteToObj(t) ? TObj : TBottom;
    });
  }

  void handleInSelfPropD(MInstrState& state) {
    if (!couldBeInSelf(state.base)) return;

    if (auto const name = state.base.locName) {
      auto const ty = thisPropAsCell(name);
      if (ty && propCouldPromoteToObj(*ty)) {
        mergeSelfProp(name,
          objExact(m_index.builtin_class(s_stdClass.get())));
      }
      return;
    }

    loseNonRefSelfPropTypes();
  }

  void handleInThisElemD(MInstrState& state) {
    if (!couldBeInThis(state.base)) return;

    if (auto const name = state.base.locName) {
      auto const ty = thisPropAsCell(name);
      if (ty && elemCouldPromoteToArr(*ty)) {
        mergeThisProp(name, TArr);
      }
      return;
    }

    mergeEachThisPropRaw([&] (Type t) {
      return elemCouldPromoteToArr(t) ? TArr : TBottom;
    });
  }

  void handleInSelfElemD(MInstrState& state) {
    if (!couldBeInSelf(state.base)) return;

    if (auto const name = state.base.locName) {
      if (auto const ty = selfPropAsCell(name)) {
        if (elemCouldPromoteToArr(*ty)) {
          mergeSelfProp(name, TArr);
        }
        mergeSelfProp(name, loosen_statics(*ty));
      }
      return;
    }
    loseNonRefSelfPropTypes();
  }

  // Currently NewElem and Elem InFoo effects don't need to do
  // anything different from each other.
  void handleInThisNewElem(MInstrState& state) { handleInThisElemD(state); }
  void handleInSelfNewElem(MInstrState& state) { handleInSelfElemD(state); }

  void handleInSelfElemU(MInstrState& state) {
    if (!couldBeInSelf(state.base)) return;

    if (auto const name = state.base.locName) {
      auto const ty = selfPropAsCell(name);
      if (ty) mergeSelfProp(name, loosen_statics(*ty));
    } else {
      mergeEachSelfPropRaw(loosen_statics);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // base ops

  void handleLocBasePropD(const MInstrState& state) {
    auto const locTy = locAsCell(state.mvec.locBase);
    if (propMustPromoteToObj(locTy)) {
      auto const ty = objExact(m_index.builtin_class(s_stdClass.get()));
      setLoc(state.mvec.locBase, ty);
      return;
    }
    if (propCouldPromoteToObj(locTy)) {
      setLoc(state.mvec.locBase, union_of(locTy, TObj));
    }
  }

  void handleLocBaseElemD(const MInstrState& state) {
    auto const locTy = locAsCell(state.mvec.locBase);
    if (locTy.subtypeOf(TArr) || elemMustPromoteToArr(locTy)) {
      // We need to do this even if it was already an array, because
      // we may modify it if it was an SArr or SArr=.
      setLoc(state.mvec.locBase, TArr);
      return;
    }
    if (elemCouldPromoteToArr(locTy)) {
      setLoc(state.mvec.locBase, union_of(locTy, TArr));
    }
  }

  /*
   * Local bases can change the type of the local depending on the
   * mvector, and the next dim.  This function updates the types as
   * well as calling the appropriate handler to compute effects on
   * local types.
   */
  Base miBaseLoc(const MInstrState& state) {
    auto& info = state.info;
    auto& mvec = state.mvec;
    bool const isDefine = info.getAttr(mvec.lcode) & MIA_define;

    if (info.m_instr == MI_UnsetM) {
      // Unsetting can turn static strings and arrays non-static.
      auto const loose = loosen_statics(derefLoc(mvec.locBase));
      setLoc(mvec.locBase, loose);
      return Base { loose, BaseLoc::Frame };
    }

    if (!isDefine) return Base { derefLoc(mvec.locBase), BaseLoc::Frame };

    ensureInit(mvec.locBase);

    auto const firstDim = mvec.mcodes[0].mcode;
    if (mcodeIsProp(firstDim)) {
      handleLocBasePropD(state);
    } else if (mcodeIsElem(firstDim) || firstDim == MemberCode::MW) {
      handleLocBaseElemD(state);
    }

    return Base { locAsCell(mvec.locBase), BaseLoc::Frame };
  }

  Base miBaseSProp(Type cls, Type tprop) {
    auto const self = selfCls();
    auto const prop = tv(tprop);
    auto const name = prop && prop->m_type == KindOfStaticString
                        ? prop->m_data.pstr : nullptr;
    if (self && cls.subtypeOf(*self) && name) {
      if (auto const ty = selfPropAsCell(prop->m_data.pstr)) {
        return Base { *ty, BaseLoc::StaticObjProp, cls, name };
      }
    }
    return Base { TInitCell, BaseLoc::StaticObjProp, cls, name };
  }

  Base miBase(MInstrState& state) {
    auto& mvec = state.mvec;
    switch (mvec.lcode) {
    case LL:
      return miBaseLoc(state);
    case LC:
      return Base { topC(--state.stackIdx), BaseLoc::EvalStack };
    case LR:
      {
        auto const t = topR(--state.stackIdx);
        return Base { t.subtypeOf(TInitCell) ? t : TInitCell,
                      BaseLoc::EvalStack };
      }
    case LH:
      {
        auto const ty = thisType();
        return Base { ty ? *ty : TObj, BaseLoc::FrameThis };
      }
    case LGL:
      locAsCell(state.mvec.locBase);
      return Base { TInitCell, BaseLoc::Global };
    case LGC:
      topC(--state.stackIdx);
      return Base { TInitCell, BaseLoc::Global };

    case LNL:
      loseNonRefLocalTypes();
      return Base { TInitCell, BaseLoc::Frame };
    case LNC:
      loseNonRefLocalTypes();
      topC(--state.stackIdx);
      return Base { TInitCell, BaseLoc::Frame };

    case LSL:
      {
        auto const cls  = topA(state.info.valCount());
        auto const prop = locAsCell(state.mvec.locBase);
        return miBaseSProp(cls, prop);
      }
    case LSC:
      {
        auto const cls  = topA(state.info.valCount());
        auto const prop = topC(--state.stackIdx);
        return miBaseSProp(cls, prop);
      }

    case NumLocationCodes:
      break;
    }
    not_reached();
  }

  Type mcodeKey(MInstrState& state) {
    auto const melem = state.mvec.mcodes[state.mInd];
    switch (melem.mcode) {
    case MPC:  return topC(--state.stackIdx);
    case MPL:  return locAsCell(melem.immLoc);
    case MPT:  return sval(melem.immStr);

    case MEC:  return topC(--state.stackIdx);
    case MET:  return sval(melem.immStr);
    case MEL:  return locAsCell(melem.immLoc);
    case MEI:  return ival(melem.immInt);

    case MW:
    case NumMemberCodes:
      break;
    }
    not_reached();
  }

  // Returns nullptr if it's an unknown key or not a string.
  SString mcodeStringKey(MInstrState& state) {
    auto const v = tv(mcodeKey(state));
    return v && v->m_type == KindOfStaticString ? v->m_data.pstr : nullptr;
  }

  void miPop(const MInstrState& state) {
    auto& mvec = state.mvec;
    if (mvec.lcode == LSL || mvec.lcode == LSC) {
      assert(state.stackIdx == state.info.valCount() + 1 /* clsref */);
    } else {
      assert(state.stackIdx == state.info.valCount());
    }
    for (auto i = uint32_t{0}; i < numVecPops(mvec); ++i) popT();
  }

  // MInstrs can throw in between each op, so the states of locals
  // need to be propagated across factored exit edges.
  void miThrow() { m_propagateThrow(m_state); }

  //////////////////////////////////////////////////////////////////////
  // intermediate ops

  void miProp(MInstrState& state) {
    auto const name     = mcodeStringKey(state);
    bool const isDefine = state.info.getAttr(state.mcode()) & MIA_define;
    bool const isUnset  = state.info.getAttr(state.mcode()) & MIA_unset;

    /*
     * MIA_unset Props doesn't promote "emptyish" things to stdClass,
     * or affect arrays, however it can define a property on an object
     * base.  This means we don't need any couldBeInFoo logic, but if
     * the base could actually be $this, and a declared property could
     * be Uninit, we need to merge InitNull.
     *
     * We're trying to handle this case correctly as far as the type
     * inference here is concerned, but the runtime doesn't actually
     * behave this way right now for declared properties.  Note that
     * it never hurts to merge more types than a thisProp could
     * actually be, so this is fine.
     *
     * See TODO(#3602740): unset with intermediate dims on previously
     * declared properties doesn't define them to null.
     */
    if (isUnset && couldBeThisObj(state.base)) {
      if (name) {
        auto const ty = thisPropRaw(name);
        if (ty && ty->couldBe(TUninit)) {
          mergeThisProp(name, TInitNull);
        }
      } else {
        mergeEachThisPropRaw([&] (Type ty) {
          return ty.couldBe(TUninit) ? TInitNull : TBottom;
        });
      }
    }

    if (isDefine) {
      handleInThisPropD(state);
      handleInSelfPropD(state);
    }

    if (mustBeThisObj(state.base)) {
      auto const optThisTy = thisType();
      auto const thisTy    = optThisTy ? *optThisTy : TObj;
      if (name) {
        auto const propTy = thisPropAsCell(name);
        state.base = Base { propTy ? *propTy : TInitCell,
                            BaseLoc::PostProp,
                            thisTy,
                            name };
      } else {
        state.base = Base { TInitCell, BaseLoc::PostProp, thisTy };
      }
      return;
    }

    // We know for sure we're going to be in an object property.
    if (state.base.type.subtypeOf(TObj)) {
      state.base = Base { TInitCell,
                          BaseLoc::PostProp,
                          state.base.type,
                          name };
      return;
    }

    /*
     * Otherwise, intermediate props with define can promote a null,
     * false, or "" to stdClass.  Those cases, and others, if it's not
     * MIA_define, will set the base to a null value in tvScratch.
     * The base may also legitimately be an object and our next base
     * is in an object property.
     *
     * If we know for sure we're promoting to stdClass, we can put the
     * locType pointing at that.  Otherwise we conservatively treat
     * all these cases as "possibly" being inside of an object
     * property with "PostProp" with locType TTop.
     */
    auto const newBaseLocTy =
      propMustPromoteToObj(state.base.type)
        ? objExact(m_index.builtin_class(s_stdClass.get()))
        : TTop;

    state.base = Base { TInitCell, BaseLoc::PostProp, newBaseLocTy, name };
  }

  void miElem(MInstrState& state) {
    mcodeKey(state);
    bool const isDefine = state.info.getAttr(state.mcode()) & MIA_define;
    bool const isUnset  = state.info.getAttr(state.mcode()) & MIA_unset;

    /*
     * Elem dims with MIA_unset can change a base from a static array
     * into a reference counted array.  It never promotes emptyish
     * types, however.
     *
     * We only need to handle this for self props, because we don't
     * track static-ness on this props.  The similar effect on local
     * bases is handled in miBase.
     */
    if (isUnset) {
      handleInSelfElemU(state);
    }
    if (isDefine) {
      handleInThisElemD(state);
      handleInSelfElemD(state);
    }

    if (state.base.type.subtypeOf(TArr)) {
      state.base = Base { TInitCell, BaseLoc::PostElem, state.base.type };
      return;
    }
    if (state.base.type.subtypeOf(TStr)) {
      state.base = Base { TStr, BaseLoc::PostElem };
      return;
    }

    /*
     * Other cases could leave the base as anything (if nothing else,
     * via ArrayAccess on an object).
     *
     * The resulting BaseLoc is either inside an array, is the global
     * init_null_variant, or inside tvScratch.  We represent this with
     * the PostElem base location with locType TTop.
     */
    state.base = Base { TInitCell, BaseLoc::PostElem, TTop };
  }

  void miNewElem(MInstrState& state) {
    if (!state.info.newElem()) {
      state.base = Base { TInitCell, BaseLoc::Fataled };
      return;
    }

    handleInThisNewElem(state);
    handleInSelfNewElem(state);

    if (state.base.type.subtypeOf(TArr)) {
      /*
       * Inside of an array, this appears to create a TUninit and let
       * the next operation turn it into a real null (or stdClass or
       * whatever).  The lvalBlackhole case explicitly unsets the
       * blackhole before making it the base.
       *
       * We're representing it as TNull because it doesn't really seem
       * like we should be using Uninit in these cases (it would be
       * nice if Elem dims didn't have to have KindOfUninit in their
       * switches), and a wider type is never wrong.
       */
      state.base = Base { TNull, BaseLoc::PostElem, state.base.type };
      return;
    }

    state.base = Base { TInitCell, BaseLoc::PostElem, TTop };
  }

  void miIntermediate(MInstrState& state) {
    if (mcodeIsProp(state.mcode())) return miProp(state);
    if (mcodeIsElem(state.mcode())) return miElem(state);
    return miNewElem(state);
  }

  //////////////////////////////////////////////////////////////////////
  // final prop ops

  void miFinalIssetProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    miPop(state);
    if (name && mustBeThisObj(state.base)) {
      if (auto const pt = thisPropAsCell(name)) {
        if (pt->subtypeOf(TNull))  return push(TFalse);
        if (!pt->couldBe(TNull))   return push(TTrue);
      }
    }
    push(TBool);
  }

  void miFinalCGetProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    miPop(state);
    if (name && mustBeThisObj(state.base)) {
      if (auto const t = thisPropAsCell(name)) return push(*t);
    }
    push(TInitCell);
  }

  void miFinalVGetProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    miPop(state);
    handleInThisPropD(state);
    handleInSelfPropD(state);
    if (couldBeThisObj(state.base)) {
      if (name) {
        boxThisProp(name);
      } else {
        killThisProps();
      }
    }
    push(TRef);
  }

  void miFinalSetProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    auto const t1 = popC();
    miPop(state);
    handleInThisPropD(state);
    handleInSelfPropD(state);
    if (couldBeThisObj(state.base)) {
      if (!name) {
        // We could just merge t1 into every thisProp, but not for
        // now.
        loseNonRefThisPropTypes();
        push(TInitCell);
        return;
      }
      mergeThisProp(name, t1);
      // TODO(#3343813): I think we can push t1 if it's known to be
      // any TObj (even ArrayAccess doesn't mess that up), but not for
      // now.  (Need some additional testing first.)
      push(mustBeThisObj(state.base) ? t1 : TInitCell);
      return;
    }
    push(TInitCell);
  }

  void miFinalSetOpProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    popC();
    miPop(state);
    handleInThisPropD(state);
    handleInSelfPropD(state);
    if (couldBeThisObj(state.base)) {
      if (name) {
        mergeThisProp(name, TInitCell);
      } else {
        loseNonRefThisPropTypes();
      }
    }
    push(TInitCell);
  }

  void miFinalIncDecProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    miPop(state);
    handleInThisPropD(state);
    handleInSelfPropD(state);
    if (couldBeThisObj(state.base)) {
      if (name) {
        mergeThisProp(name, TInitCell);
      } else {
        loseNonRefThisPropTypes();
      }
    }
    push(TInitCell);
  }

  void miFinalBindProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    popV();
    miPop(state);
    handleInThisPropD(state);
    handleInSelfPropD(state);
    if (couldBeThisObj(state.base)) {
      if (name) {
        boxThisProp(name);
      } else {
        killThisProps();
      }
    }
    push(TRef);
  }

  void miFinalUnsetProp(MInstrState& state) {
    auto const name = mcodeStringKey(state);
    miPop(state);

    /*
     * Unset does define intermediate dims but with slightly different
     * rules than sets.  It only applies to object properties.
     *
     * Note that this can't affect self props, because static
     * properties can never be unset.
     */
    handleInThisPropD(state);

    if (couldBeThisObj(state.base)) {
      if (name) {
        unsetThisProp(name);
      } else {
        unsetUnknownThisProp();
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Final elem ops

  void miFinalSetElem(MInstrState& state) {
    // TODO(#3343813): we should push the type of the rhs when we can;
    // SetM has some weird cases where it pushes null instead to
    // handle.
    mcodeKey(state);
    auto const t1 = popC();
    miPop(state);
    handleInThisElemD(state);
    handleInSelfElemD(state);
    // ArrayAccess on $this will always push the rhs.
    push(mustBeThisObj(state.base) ? t1 : TInitCell);
  }

  void miFinalSetOpElem(MInstrState& state) {
    mcodeKey(state);
    popC();
    miPop(state);
    handleInThisElemD(state);
    handleInSelfElemD(state);
    push(TInitCell);
  }

  void miFinalIncDecElem(MInstrState& state) {
    mcodeKey(state);
    miPop(state);
    handleInThisElemD(state);
    handleInSelfElemD(state);
    push(TInitCell);
  }

  void miFinalBindElem(MInstrState& state) {
    mcodeKey(state);
    popV();
    miPop(state);
    handleInThisElemD(state);
    handleInSelfElemD(state);
    push(TRef);
  }

  //////////////////////////////////////////////////////////////////////
  // Final new elem ops

  void miFinalSetNewElem(MInstrState& state) {
    auto const t1 = popC();
    miPop(state);
    handleInThisNewElem(state);
    handleInSelfNewElem(state);
    // ArrayAccess on $this will always push the rhs.
    push(mustBeThisObj(state.base) ? t1 : TInitCell);
  }

  void miFinalSetOpNewElem(MInstrState& state) {
    popC();
    miPop(state);
    handleInThisNewElem(state);
    handleInSelfNewElem(state);
    push(TInitCell);
  }

  void miFinalIncDecNewElem(MInstrState& state) {
    miPop(state);
    handleInThisNewElem(state);
    handleInSelfNewElem(state);
    push(TInitCell);
  }

  void miFinalBindNewElem(MInstrState& state) {
    popV();
    miPop(state);
    handleInThisNewElem(state);
    handleInSelfNewElem(state);
    push(TRef);
  }

  //////////////////////////////////////////////////////////////////////

  void miFinal(MInstrState& state, const bc::EmptyM& op) {
    // Same thing for now for props and elems, regardless of the base.
    // MW would be a fatal.
    mcodeKey(state);
    miPop(state);
    push(TBool);
  }

  void miFinal(MInstrState& state, const bc::IssetM& op) {
    if (mcodeIsProp(state.mcode())) return miFinalIssetProp(state);
    // Elem case (MW would be a fatal):
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    miPop(state);
    push(TBool);
  }

  void miFinal(MInstrState& state, const bc::CGetM& op) {
    if (mcodeIsProp(state.mcode())) return miFinalCGetProp(state);
    // Elem case (MW would be a fatal):
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    miPop(state);
    push(TInitCell);
  }

  void miFinal(MInstrState& state, const bc::VGetM& op) {
    if (mcodeIsProp(state.mcode())) return miFinalVGetProp(state);
    // Elem and MW case:
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    miPop(state);
    if (state.mcode() != MemberCode::MW) {
      handleInThisElemD(state);
      handleInSelfElemD(state);
    } else {
      handleInThisNewElem(state);
      handleInSelfNewElem(state);
    }
    push(TRef);
  }

  void miFinal(MInstrState& state, const bc::SetM& op) {
    if (mcodeIsElem(state.mcode())) return miFinalSetElem(state);
    if (mcodeIsProp(state.mcode())) return miFinalSetProp(state);
    return miFinalSetNewElem(state);
  }

  void miFinal(MInstrState& state, const bc::SetOpM& op) {
    if (mcodeIsElem(state.mcode())) return miFinalSetOpElem(state);
    if (mcodeIsProp(state.mcode())) return miFinalSetOpProp(state);
    return miFinalSetOpNewElem(state);
  }

  void miFinal(MInstrState& state, const bc::IncDecM& op) {
    if (mcodeIsElem(state.mcode())) return miFinalIncDecElem(state);
    if (mcodeIsProp(state.mcode())) return miFinalIncDecProp(state);
    return miFinalIncDecNewElem(state);
  }

  void miFinal(MInstrState& state, const bc::BindM& op) {
    if (mcodeIsElem(state.mcode())) return miFinalBindElem(state);
    if (mcodeIsProp(state.mcode())) return miFinalBindProp(state);
    return miFinalBindNewElem(state);
  }

  void miFinal(MInstrState& state, const bc::UnsetM& op) {
    if (mcodeIsProp(state.mcode())) return miFinalUnsetProp(state);
    // Elem and MW case:
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    handleInSelfElemU(state);
    miPop(state);
  }

  void miFinal(MInstrState& state, const bc::SetWithRefLM& op) {
    killThisProps();
    killSelfProps();
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    miPop(state);
  }

  void miFinal(MInstrState& state, const bc::SetWithRefRM& op) {
    killThisProps();
    killSelfProps();
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
    popR();
    miPop(state);
  }

  //////////////////////////////////////////////////////////////////////

  template<class Op>
  void miImpl(const Op& op, const MInstrInfo& info, const MVector& mvec) {
    auto state = MInstrState { &info, mvec };
    // The m_state before miBase is propagated across factored edges
    // because wasPEI will be true---no need for miThrow.
    state.base = miBase(state);
    FTRACE(4, "   base: {}\n", base_string(state.base));
    miThrow();
    for (; state.mInd < mvec.mcodes.size() - 1; ++state.mInd) {
      miIntermediate(state);
      FTRACE(4, "   base: {}\n", base_string(state.base));
      // Note: this one might not be necessary: review whether member
      // instructions can ever modify local types on itermediate dims.
      miThrow();
    }
    miFinal(state, op);
  }

  template<class Op>
  void minstr(const Op& op) {
    miImpl(op, getMInstrInfo(Op::op), op.mvec);
  }

private:
  void nothrow()       { FTRACE(2, "    nothrow\n"); m_flags.wasPEI = false; }
  void calledNoReturn(){ m_flags.calledNoReturn = true; }
  void constprop()     { m_flags.canConstProp = true; }
  void nofallthrough() { m_flags.tookBranch = true; }

  void doRet(Type t) {
    readAllLocals();
    assert(m_state.stack.empty());
    m_flags.returned = t;
  }

  /*
   * impl(...)
   *
   * Utility for chaining one bytecode implementation to a series of a
   * few others.  Use reduce() if you also want to enable strength
   * reduction (i.e. the bytecode can be replaced by some other
   * bytecode as an optimization).
   *
   * The chained-to bytecodes should not take branches, or do strength
   * reduction.  If they use impl themselves, the outer impl() should
   * only be chaining to a single bytecode (or flag effects can be
   * hard to reason about), and shouldn't set any flags prior to that.
   *
   * Note: nested reduce would probably be nice to have later for FPassR.
   *
   * constprop with impl() should only occur on the last thing in the
   * impl list.  This isn't checked, but we'll ignore a canConstProp
   * flag anywhere earlier.
   */

  template<class T> void impl(const T& t) {
    FTRACE(3, "    (impl {}\n", show(Bytecode { t }));
    m_flags.wasPEI       = true;
    m_flags.canConstProp = false;
    // Keep whatever mayReadLocals was set to.
    (*this)(t);
  }

  template<class T, class... Ts>
  void impl(const T& t, Ts&&... ts) {
    impl(t);

    assert(!m_flags.tookBranch &&
           "you can't use impl with branching opcodes before last position");
    assert(!m_flags.strengthReduced);

    auto const wasPEI = m_flags.wasPEI;

    impl(std::forward<Ts>(ts)...);

    // If any of the opcodes in the impl list said they could throw,
    // then the whole thing could throw.
    m_flags.wasPEI = m_flags.wasPEI || wasPEI;
  }

  /*
   * Reduce means that (given some situation in the execution state),
   * a given bytecode could be replaced by some other bytecode
   * sequence.  Ensure that if you call reduce(), it is before any
   * state-affecting operations (like popC()).
   */
  template<class... Bytecodes>
  void reduce(const Bytecodes&... hhbc) {
    impl(hhbc...);
    m_flags.strengthReduced = std::vector<Bytecode> { hhbc... };
  }

private:
  void readUnknownLocals() { m_flags.mayReadLocalSet.set(); }
  void readAllLocals()     { m_flags.mayReadLocalSet.set(); }

  void killLocals() {
    FTRACE(2, "    killLocals\n");
    readUnknownLocals();
    for (auto& l : m_state.locals) l = TGen;
  }

  // Force non-ref locals to TCell.  Used when something modifies an
  // unknown local's value, without changing reffiness.
  void loseNonRefLocalTypes() {
    readUnknownLocals();
    FTRACE(2, "    loseNonRefLocalTypes\n");
    for (auto& l : m_state.locals) {
      if (l.subtypeOf(TCell)) l = TCell;
    }
  }

  void boxUnknownLocal() {
    readUnknownLocals();
    FTRACE(2, "   boxUnknownLocal\n");
    for (auto& l : m_state.locals) {
      if (!l.subtypeOf(TRef)) l = TGen;
    }
  }

  void unsetUnknownLocal() {
    readUnknownLocals();
    FTRACE(2, "  unsetUnknownLocal\n");
    for (auto& l : m_state.locals) l = union_of(l, TUninit);
  }

  void unsetNamedLocals() {
    for (auto i = size_t{0}; i < m_state.locals.size(); ++i) {
      if (m_ctx.func->locals[i]->name) {
        m_state.locals[i] = TUninit;
      }
    }
  }

private:
  void specialFunctionEffects(SString name) {
    // extract() trashes the local variable environment.
    if (name->isame(s_extract.get())) {
      readUnknownLocals();
      killLocals();
    }
  }

  void specialFunctionEffects(ActRec ar) {
    switch (ar.kind) {
    case FPIKind::Unknown:
      // fallthrough
    case FPIKind::Func:
      // Could be a dynamic call to extract:
      if (!ar.func) {
        readUnknownLocals();
        killLocals();
        return;
      }
      specialFunctionEffects(ar.func->name());
      break;
    case FPIKind::Ctor:
    case FPIKind::ObjMeth:
    case FPIKind::ClsMeth:
    case FPIKind::ObjInvoke:
    case FPIKind::CallableArr:
      break;
    }
  }

private: // eval stack
  Type popC() {
    auto const v = popT();
    assert(v.subtypeOf(TInitCell)); // or it would be popU, which doesn't exist
    return v;
  }

  Type popV() {
    auto const v = popT();
    assert(v.subtypeOf(TRef));
    return v;
  }

  Type popA() {
    auto const v = popT();
    assert(v.subtypeOf(TCls));
    return v;
  }

  Type popR()  { return popT(); }
  Type popF()  { return popT(); }
  Type popCV() { return popT(); }
  Type popU()  { return popT(); }

  Type popT() {
    assert(!m_state.stack.empty());
    auto const ret = m_state.stack.back();
    FTRACE(2, "    pop:  {}\n", show(ret));
    m_state.stack.pop_back();
    return ret;
  }

  void popFlav(Flavor flav) {
    switch (flav) {
    case Flavor::C: popC(); break;
    case Flavor::V: popV(); break;
    case Flavor::U: popU(); break;
    case Flavor::F: popF(); break;
    case Flavor::R: popR(); break;
    case Flavor::A: popA(); break;
    }
  }

  Type topT(uint32_t idx = 0) {
    assert(idx < m_state.stack.size());
    return m_state.stack[m_state.stack.size() - idx - 1];
  }

  Type topA(uint32_t i = 0) {
    assert(topT(i).subtypeOf(TCls));
    return topT(i);
  }

  Type topC(uint32_t i = 0) {
    assert(topT(i).subtypeOf(TInitCell));
    return topT(i);
  }

  Type topR(uint32_t i = 0) { return topT(i); }

  Type topV(uint32_t i = 0) {
    assert(topT(i).subtypeOf(TRef));
    return topT(i);
  }

  void push(Type t) {
    FTRACE(2, "    push: {}\n", show(t));
    m_state.stack.push_back(t);
  }

private: // fpi
  void fpiPush(ActRec ar) {
    FTRACE(2, "    fpi+: {}\n", show(ar));
    m_state.fpiStack.push_back(ar);
  }

  ActRec fpiPop() {
    assert(!m_state.fpiStack.empty());
    auto const ret = m_state.fpiStack.back();
    FTRACE(2, "    fpi-: {}\n", show(ret));
    m_state.fpiStack.pop_back();
    return ret;
  }

  const ActRec& fpiTop() const {
    assert(!m_state.fpiStack.empty());
    return m_state.fpiStack.back();
  }

  PrepKind prepKind(uint32_t paramId) const {
    auto& ar = fpiTop();
    if (ar.func) return m_index.lookup_param_prep(m_ctx, *ar.func, paramId);
    return PrepKind::Unknown;
  }

private: // locals
  void mayReadLocal(uint32_t id) {
    if (id < m_flags.mayReadLocalSet.size()) {
      m_flags.mayReadLocalSet.set(id);
    }
  }

  Type locRaw(borrowed_ptr<const php::Local> l) {
    mayReadLocal(l->id);
    return m_state.locals[l->id];
  }

  void setLocRaw(borrowed_ptr<const php::Local> l, Type t) {
    mayReadLocal(l->id);
    m_state.locals[l->id] = t;
  }

  // Read a local type in the sense of CGetL.  (TUninits turn into
  // TInitNull, and potentially reffy types return the "inner" type,
  // which is always a subtype of InitCell.)
  Type locAsCell(borrowed_ptr<const php::Local> l) {
    auto t = locRaw(l);
    return !t.subtypeOf(TCell) ? TInitCell :
            t.subtypeOf(TUninit) ? TInitNull :
            remove_uninit(t);
  }

  // Read a local type, dereferencing refs, but without converting
  // potential TUninits to TInitNull.
  Type derefLoc(borrowed_ptr<const php::Local> l) {
    auto v = locRaw(l);
    if (v.subtypeOf(TCell)) return v;
    return v.couldBe(TUninit) ? TCell : TInitCell;
  }

  void ensureInit(borrowed_ptr<const php::Local> l) {
    auto t = locRaw(l);
    if (t.couldBe(TUninit)) {
      if (t.subtypeOf(TUninit)) return setLocRaw(l, TInitNull);
      if (t.subtypeOf(TCell))   return setLocRaw(l, remove_uninit(t));
      setLocRaw(l, TInitGen);
    }
  }

  bool locCouldBeUninit(borrowed_ptr<const php::Local> l) {
    return locRaw(l).couldBe(TUninit);
  }

  /*
   * Set a local type in the sense of tvSet.  If the local is boxed or
   * not known to be not boxed, we can't change the type.  May be used
   * to set locals to types that include Uninit.
   */
  void setLoc(borrowed_ptr<const php::Local> l, Type t) {
    auto v = locRaw(l);
    if (v.subtypeOf(TCell)) m_state.locals[l->id] = t;
  }

  borrowed_ptr<php::Local> findLocal(SString name) {
    for (auto& l : m_ctx.func->locals) {
      if (l->name->same(name)) {
        mayReadLocal(l->id);
        return borrow(l);
      }
    }
    return nullptr;
  }

  borrowed_ptr<php::Local> findLocalById(int32_t id) {
    assert(id < m_ctx.func->locals.size());
    mayReadLocal(id);
    return borrow(m_ctx.func->locals[id]);
  }

private: // $this
  void setThisAvailable() {
    FTRACE(2, "    setThisAvailable\n");
    m_state.thisAvailable = true;
  }

  bool thisAvailable() const { return m_state.thisAvailable; }

  // Returns the type $this would have if it's not null.  Generally
  // you have to check thisIsAvailable() before assuming it can't be
  // null.
  folly::Optional<Type> thisType() const {
    if (!m_ctx.cls) return folly::none;
    if (auto const rcls = m_index.resolve_class(m_ctx, m_ctx.cls->name)) {
      return subObj(*rcls);
    }
    return folly::none;
  }

  folly::Optional<Type> selfCls() const {
    if (!m_ctx.cls) return folly::none;
    if (auto const rcls = m_index.resolve_class(m_ctx, m_ctx.cls->name)) {
      return subCls(*rcls);
    }
    return folly::none;
  }

private: // properties on $this
  /*
   * Note: we are only tracking control-flow insensitive types for
   * object properties, because it can be pretty rough to try to track
   * all cases that could re-enter the VM, run arbitrary code, and
   * potentially change the type of a property.
   *
   * Because of this, the various "setter" functions for thisProps
   * here actually just union the new type into what we already had.
   */

  Type* thisPropRaw(SString name) const {
    auto& privateProperties = m_props.privateProperties();
    auto const it = privateProperties.find(name);
    if (it != end(privateProperties)) {
      return &it->second;
    }
    return nullptr;
  }

  bool isTrackedThisProp(SString name) const { return thisPropRaw(name); }

  void killThisProps() {
    FTRACE(2, "    killThisProps\n");
    for (auto& kv : m_props.privateProperties()) kv.second = TGen;
  }

  /*
   * This function returns a type that includes all the possible types
   * that could result from reading a property $this->name.
   *
   * Note that this may include types that the property itself cannot
   * actually contain, due to the effects of a possible __get
   * function.  For now we handle that case by just returning
   * InitCell, rather than detecting if $this could have a magic
   * getter.  TODO(#3669480).
   */
  folly::Optional<Type> thisPropAsCell(SString name) const {
    auto const t = thisPropRaw(name);
    if (!t) return folly::none;

    if (t->couldBe(TUninit)) {
      // Could come out of __get.
      return TInitCell;
    }
    if (t->subtypeOf(TCell)) return *t;
    return TInitCell;
  }

  /*
   * Merge a type into the track property types on $this, in the sense
   * of tvSet (i.e. setting the inner type on possible refs).
   *
   * Note that all types we see that could go into an object property
   * have to loosen_statics and loosen_values.  This is because the
   * object could be serialized and then deserialized, losing the
   * static-ness of a string or array member, and we don't guarantee
   * deserialization would preserve a constant value object property
   * type.
   */
  void mergeThisProp(SString name, Type type) {
    auto const t = thisPropRaw(name);
    if (!t) return;
    *t = union_of(*t, loosen_statics(loosen_values(type)));
  }

  /*
   * Merge something into each this prop.  Usually MapFn will be a
   * predicate that returns TBottom when some condition doesn't hold.
   *
   * The types given to the map function are the raw tracked types
   * (i.e. could be TRef or TUninit).
   */
  template<class MapFn>
  void mergeEachThisPropRaw(MapFn fn) {
    for (auto& kv : m_props.privateProperties()) {
      mergeThisProp(kv.first, fn(kv.second));
    }
  }

  void unsetThisProp(SString name) { mergeThisProp(name, TUninit); }
  void unsetUnknownThisProp() {
    for (auto& kv : m_props.privateProperties()) {
      mergeThisProp(kv.first, TUninit);
    }
  }

  void boxThisProp(SString name) {
    auto const t = thisPropRaw(name);
    if (!t) return;
    *t = union_of(*t, TRef);
  }

  /*
   * Forces non-ref property types up to TCell.  This is used when an
   * operation affects an unknown property on $this, but can't change
   * its reffiness.  This could only do TInitCell, but we're just
   * going to gradually get rid of the callsites of this.
   */
  void loseNonRefThisPropTypes() {
    FTRACE(2, "    loseNonRefThisPropTypes\n");
    for (auto& kv : m_props.privateProperties()) {
      if (kv.second.subtypeOf(TCell)) kv.second = TCell;
    }
  }

private: // properties on self::
  // Similar to $this properties above, we only track control-flow
  // insensitive types for these.

  Type* selfPropRaw(SString name) const {
    auto& privateStatics = m_props.privateStatics();
    auto it = privateStatics.find(name);
    if (it != end(privateStatics)) {
      return &it->second;
    }
    return nullptr;
  }

  void killSelfProps() {
    FTRACE(2, "    killSelfProps\n");
    for (auto& kv : m_props.privateStatics()) kv.second = TGen;
  }

  void killSelfProp(SString name) const {
    FTRACE(2, "    killSelfProp {}\n", name->data());
    if (auto t = selfPropRaw(name)) *t = TGen;
  }

  // TODO(#3684136): self::$foo can't actually ever be uninit.  Right
  // now uninits may find their way into here though.
  folly::Optional<Type> selfPropAsCell(SString name) const {
    auto const t = selfPropRaw(name);
    if (!t) return folly::none;
    return !t->subtypeOf(TCell) ? TInitCell :
            t->subtypeOf(TUninit) ? TInitNull :
            remove_uninit(*t);
  }

  /*
   * Merges a type into tracked static properties on self, in the
   * sense of tvSet (i.e. setting the inner type on possible refs).
   */
  void mergeSelfProp(SString name, Type type) {
    auto const t = selfPropRaw(name);
    if (!t) return;
    *t = union_of(*t, type);
  }

  /*
   * Similar to mergeEachThisPropRaw, but for self props.
   */
  template<class MapFn>
  void mergeEachSelfPropRaw(MapFn fn) {
    for (auto& kv : m_props.privateStatics()) {
      mergeSelfProp(kv.first, fn(kv.second));
    }
  }

  void boxSelfProp(SString name) { mergeSelfProp(name, TRef); }

  /*
   * Forces non-ref static properties up to TCell.  This is used when
   * an operation affects an unknown static property on self::, but
   * can't change its reffiness.
   *
   * This could only do TInitCell because static properties can never
   * be unset.  We're just going to get rid of the callers of this
   * function over a few more changes, though.
   */
  void loseNonRefSelfPropTypes() {
    FTRACE(2, "    loseNonRefSelfPropTypes\n");
    for (auto& kv : m_props.privateStatics()) {
      if (kv.second.subtypeOf(TInitCell)) kv.second = TCell;
    }
  }

private:
  TRACE_SET_MOD(hhbbc);

private:
  const Index& m_index;
  const Context m_ctx;
  PropertiesInfo& m_props;
  State& m_state;
  StepFlags& m_flags;
  Propagate m_propagate;
  PropagateThrow m_propagateThrow;
};

//////////////////////////////////////////////////////////////////////

/*
 * Block-at-a-time interpreter usable for both analysis and
 * optimization passes.
 */
struct Interpreter {
  Interpreter(const Index* index,
              Context ctx,
              PropertiesInfo& props,
              const php::Block* blk,
              State& state)
    : m_index(*index)
    , m_ctx(ctx)
    , m_props(props)
    , m_blk(*blk)
    , m_state(state)
  {}

  /*
   * Run the interpreter on a whole block, propagating states using
   * the propagate function.
   */
  template<class Propagate, class MergeReturn>
  void run(Propagate propagate,
           MergeReturn mergeReturn) {
    SCOPE_EXIT {
      FTRACE(2, "out {}\n", state_string(*m_ctx.func, m_state));
    };

    auto const stop = end(m_blk.hhbcs);
    auto iter       = begin(m_blk.hhbcs);
    while (iter != stop) {
      auto const flags = interpOps(iter, stop, propagate);
      if (flags.calledNoReturn) {
        FTRACE(2, "  <called function that never returns>\n");
        continue;
      }
      if (flags.tookBranch) {
        FTRACE(2, "  <took branch; no fallthrough>\n");
        return;
      }
      if (flags.returned) {
        FTRACE(2, "  returned {}\n", show(*flags.returned));
        mergeReturn(*flags.returned);
      }
    }

    FTRACE(2, "  <end block>\n");
    if (m_blk.fallthrough) propagate(*m_blk.fallthrough, m_state);
  }

  /*
   * Run a single opcode in the interpreter.  This entry point is used
   * to propagate block entry states to mid-block positions after the
   * global analysis has already finished.
   */
  StepFlags step(const Bytecode& op) {
    auto propagate      = [] (php::Block&, const State&) {};
    auto propagateThrow = [] (const State&) {};
    auto flags          = StepFlags{};

    InterpStepper<decltype(propagate),decltype(propagateThrow)> stepper(
      &m_index,
      m_ctx,
      m_props,
      m_state,
      flags,
      propagate,
      propagateThrow
    );
    visit(op, stepper);
    return flags;
  }

private:
  template<class Stepper, class FirstOp>
  void doJmpType(Stepper& stepper, const FirstOp& op, const Bytecode& jmp) {
    switch (jmp.op) {
    case Op::JmpZ:    return stepper.group(op, jmp.JmpZ);
    case Op::JmpNZ:   return stepper.group(op, jmp.JmpNZ);
    default:          not_reached();
    }
  }

  template<class Stepper, class Iterator, class... Args>
  void group(Stepper& st, Iterator& it, Args&&... args) {
    FTRACE(2, " {}\n", [&]() -> std::string {
      auto ret = std::string{};
      for (auto i = size_t{0}; i < sizeof...(Args); ++i) {
        ret += " " + show(it[i]);
        if (i != sizeof...(Args) - 1) ret += ';';
      }
      return ret;
    }());
    it += sizeof...(Args);
    return st.group(std::forward<Args>(args)...);
  }

  template<class Stepper, class Iterator>
  void interpStep(Stepper& st, Iterator& it, Iterator stop) {
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
      case Op::JmpZ:   return group(st, it, it[0].CGetL, it[1].JmpZ);
      case Op::JmpNZ:  return group(st, it, it[0].CGetL, it[1].JmpNZ);
      case Op::InstanceOfD:
        switch (o3) {
        case Op::JmpZ:
          return group(st, it, it[0].CGetL, it[1].InstanceOfD, it[2].JmpZ);
        case Op::JmpNZ:
          return group(st, it, it[0].CGetL, it[1].InstanceOfD, it[2].JmpNZ);
        default: break;
        }
      default: break;
      }
      break;
    case Op::IsTypeL:
      switch (o2) {
      case Op::JmpZ:   return group(st, it, it[0].IsTypeL, it[1].JmpZ);
      case Op::JmpNZ:  return group(st, it, it[0].IsTypeL, it[1].JmpNZ);
      default: break;
      }
      break;
    case Op::AsyncAwait:
      switch (o2) {
      /*
       * Note: AsyncAwait is in practice always followed by JmpNZ with
       * the current async function implementation.  We could support
       * JmpZ, but this is not easy to test, and we'd rather hit an
       * assert here if we change emission to start doing this.
       */
      case Op::JmpNZ:  return group(st, it, it[0].AsyncAwait, it[1].JmpNZ);
      case Op::JmpZ:   always_assert(!"who is generating this code?");
      default: break;
      }
    default: break;
    }

    FTRACE(2, "  {}\n", show(*it));
    visit(*it++, st);
  }

  template<class Iterator, class Propagate>
  StepFlags interpOps(Iterator& iter, Iterator stop, Propagate propagate) {
    auto propagateThrow = [&] (const State& state) {
      for (auto& factored : m_blk.factoredExits) {
        propagate(*factored, without_stacks(state));
      }
    };

    auto flags = StepFlags{};
    InterpStepper<Propagate,decltype(propagateThrow)> stepper(
      &m_index,
      m_ctx,
      m_props,
      m_state,
      flags,
      propagate,
      propagateThrow
    );

    // Make a copy of the state (except stacks) in case we need to
    // propagate across factored exits (if it's a PEI).
    auto const stateBefore = without_stacks(m_state);
    auto const numPushed   = iter->numPush();
    interpStep(stepper, iter, stop);
    if (flags.wasPEI) {
      auto outputs_constant = [&] {
        auto const size = m_state.stack.size();
        for (auto i = size_t{0}; i < numPushed; ++i) {
          if (!tv(m_state.stack[size - i - 1])) return false;
        }
        return true;
      };

      if (flags.canConstProp && outputs_constant()) {
        FTRACE(2, "   nothrow (due to constprop)\n");
      } else {
        FTRACE(2, "   PEI.\n");
        propagateThrow(stateBefore);
      }
    }
    return flags;
  }

private:
  TRACE_SET_MOD(hhbbc);

private:
  const Index& m_index;
  const Context m_ctx;
  PropertiesInfo& m_props;
  const php::Block& m_blk;
  State& m_state;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
