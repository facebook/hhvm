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
#include "hphp/hhbbc/abstract-interp.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <cmath>

#include <boost/dynamic_bitset.hpp>
#include <boost/variant.hpp>

#include "folly/Conv.h"
#include "folly/Optional.h"
#include "folly/String.h"

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"

#include "hphp/runtime/ext/ext_math.h" // f_abs

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/class-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_extract("extract");
const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");
const StaticString s_noreturn("static analysis error: function "
                              "returned that was inferred to be noreturn");

//////////////////////////////////////////////////////////////////////

std::string fpiKindStr(FPIKind k) {
  switch (k) {
  case FPIKind::Unknown: return "unk";
  case FPIKind::Func:    return "func";
  case FPIKind::Ctor:    return "ctor";
  case FPIKind::ObjMeth: return "objm";
  case FPIKind::ClsMeth: return "clsm";
  }
  not_reached();
}

std::string show(const ActRec& a) {
  return folly::to<std::string>(
    "ActRec { ",
    fpiKindStr(a.kind),
    a.func ? (": " + show(*a.func)) : std::string{},
    " }"
  );
}

std::string state_string(const php::Func& f, const State& st) {
  std::string ret;
  if (!st.initialized) return "state: uninitialized\n";
  ret = "state:\n";
  for (size_t i = 0; i < st.locals.size(); ++i) {
    ret += folly::format("${: <8} :: {}\n",
      f.locals[i]->name
        ? std::string(f.locals[i]->name->data())
        : folly::format("<unnamed{}>", i).str(),
      show(st.locals[i])
    ).str();
  }
  for (size_t i = 0; i < st.stack.size(); ++i) {
    ret += folly::format("stk[{:02}] :: {}\n",
      i,
      show(st.stack[i])
    ).str();
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

// Returns whether anything changed in the dst state.
bool merge_into(State& dst, const State& src) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  assert(src.initialized);
  assert(dst.locals.size() == src.locals.size());
  assert(dst.stack.size() == src.stack.size());
  assert(dst.fpiStack.size() == src.fpiStack.size());

  bool changed = false;

  for (size_t i = 0; i < dst.stack.size(); ++i) {
    auto const newT = union_of(dst.stack[i], src.stack[i]);
    if (dst.stack[i] != newT) changed = true;
    dst.stack[i] = newT;
  }
  for (size_t i = 0; i < dst.locals.size(); ++i) {
    auto const newT = union_of(dst.locals[i], src.locals[i]);
    if (dst.locals[i] != newT) changed = true;
    dst.locals[i] = newT;
  }
  for (size_t i = 0; i < dst.fpiStack.size(); ++i) {
    if (dst.fpiStack[i] != src.fpiStack[i]) {
      always_assert(dst.fpiStack[i].kind == src.fpiStack[i].kind);
      changed = true;
      dst.fpiStack[i] = ActRec { src.fpiStack[i].kind };
    }
  }

  return changed;
}

// Return a copy of a State without copying either the evaluation
// stack or FPI stack.
State without_stacks(State const& src) {
  State ret;
  ret.initialized = src.initialized;
  ret.locals = src.locals;
  return ret;
}

//////////////////////////////////////////////////////////////////////

/*
 * When constant-evaluating certain operations, it's possible they
 * will return non-static objects, or throw exceptions (e.g. cellAdd()
 * with an array and an int).  This routine converts these things back
 * to types.
 */
template<class Pred>
Type eval_cell(Pred p) {
  try {
    Cell c = p();
    if (IS_REFCOUNTED_TYPE(c.m_type)) {
      switch (c.m_type) {
      case KindOfString:
        {
          auto const sstr = makeStaticString(c.m_data.pstr);
          tvDecRef(&c);
          c = make_tv<KindOfStaticString>(sstr);
        }
        break;
      case KindOfArray:
        {
          auto const sarr = ArrayData::GetScalarArray(c.m_data.parr);
          tvDecRef(&c);
          c = make_tv<KindOfArray>(sarr);
        }
        break;
      default:
        always_assert(0 && "Impossible constant evaluation occured");
      }
    }
    return from_cell(c);
  } catch (const std::exception&) {
    /*
     * Not currently trying to set the nothrow() flag based on whether
     * or not this catch block is hit.  To do it correctly, it will
     * require checking whether the non-exceptioning cases above
     * possibly entered the error handler by running a raise_notice or
     * similar.
     */
    FTRACE(2, "    <constant eval_cell throws exception>\n");
    return TInitCell;
  }
}

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
   *
   * TODO_3: canConstProp should imply it can't throw when it is
   * actually a constant.
   */
  bool canConstProp = false;

  /*
   * If an instruction may read or write to locals, this flag is set.
   * It is only used to try to leave out unnecessary type assertions
   * on locals (for options.FilterAssertions).
   */
  bool mayReadLocals = false;

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

template<class Propagate, class PropagateThrow>
struct InterpStepper : boost::static_visitor<void> {
  explicit InterpStepper(const Index* index,
                         Context ctx,
                         State& st,
                         StepFlags& flags,
                         Propagate propagate,
                         PropagateThrow propagateThrow)
    : m_index(*index)
    , m_ctx(ctx)
    , m_state(st)
    , m_flags(flags)
    , m_propagate(propagate)
    , m_propagateThrow(propagateThrow)
  {}

  void operator()(const bc::Nop&)  { nothrow(); }
  void operator()(const bc::PopA&) { nothrow(); popA(); }
  void operator()(const bc::PopC&) { nothrow(); popC(); }
  void operator()(const bc::PopV&) { nothrow(); popV(); }
  void operator()(const bc::PopR&) { nothrow(); popR(); }

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

  void operator()(const bc::Box&)     { throw_oom_only(); popC(); push(TRef); }
  void operator()(const bc::BoxR&)    { throw_oom_only(); popR(); push(TRef); }
  void operator()(const bc::Unbox&)   { nothrow(); popV(); push(TInitCell); }

  void operator()(const bc::UnboxR&) {
    nothrow();
    auto const t = popR();
    if (t.subtypeOf(TInitCell)) return push(t);
    push(TInitCell);
  }

  void operator()(const bc::UnboxRNop&) {
    nothrow();
    auto const t = popR();
    push(t.subtypeOf(TInitCell) ? t : TInitCell);
  }

  void operator()(const bc::BoxRNop&) {
    throw_oom_only();
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

  void operator()(const bc::NewCol&)      { push(TObj); }
  void operator()(const bc::ColAddElemC&) {
    popC(); popC(); popC();
    push(TObj);
  }
  void operator()(const bc::ColAddNewElemC&) { popC(); popC(); push(TObj); }

  // Note: unlikely class constants, these can be dynamic system
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

  void operator()(const bc::File&) { nothrow(); push(TSStr); }
  void operator()(const bc::Dir&)  { nothrow(); push(TSStr); }

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
    auto const t1 = popC();
    auto const t2 = popC();
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);
    if (v1 && v2) {
      constprop();
      return push(eval_cell([&] { return fun(*v2, *v1); }));
    }
    // TODO_4: op-specific type manipulations; only doing constant
    // propagation right now.  (E.g. Shl always returns an int.)
    push(TInitCell);
  }

  template<class Op, class Fun>
  void divModImpl(const Op& op, Fun fun) {
    auto const t1 = popC();
    auto const t2 = popC();
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);
    if (v1 && v2 && cellToInt(*v1) != 0 && cellToDouble(*v1) != 0.0) {
      constprop();
      return push(eval_cell([&] { return fun(*v2, *v1); }));
    }
    push(TInitCell);
  }

  void operator()(const bc::Add& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellAdd(c1, c2); });
  }
  void operator()(const bc::Sub& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellSub(c1, c2); });
  }
  void operator()(const bc::Mul& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellMul(c1, c2); });
  }
  void operator()(const bc::Div& op) {
    divModImpl(op, [&] (Cell c1, Cell c2) { return cellDiv(c1, c2); });
  }
  void operator()(const bc::Mod& op) {
    divModImpl(op, [&] (Cell c1, Cell c2) { return cellMod(c1, c2); });
  }

  void operator()(const bc::BitAnd& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellBitAnd(c1, c2); });
  }
  void operator()(const bc::BitOr& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellBitOr(c1, c2); });
  }
  void operator()(const bc::BitXor& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) { return cellBitXor(c1, c2); });
  }

  void operator()(const bc::Shl& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) {
      return make_tv<KindOfInt64>(cellToInt(c1) << cellToInt(c2));
    });
  }

  void operator()(const bc::Shr& op) {
    arithImpl(op, [&] (Cell c1, Cell c2) {
      return make_tv<KindOfInt64>(cellToInt(c1) >> cellToInt(c2));
    });
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
    if (val.subtypeOf(TObj)) return push(val);
    return push(TObj);
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
    auto const converted_true = [&] {
      if (is_opt(loc)) return unopt(loc);
      if (loc.subtypeOf(TBool)) return TTrue;
      return loc;
    }();
    auto const converted_false = [&] {
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
    auto const rcls = m_index.resolve_class(m_ctx, inst.str1);
    if (!rcls) return impl(cgetl, inst, jmp);
    auto const instTy = subObj(*rcls);
    auto const loc = derefLoc(cgetl.loc1);
    if (loc.subtypeOf(instTy) || !loc.couldBe(instTy)) {
      return impl(cgetl, inst, jmp);
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
  void operator()(const bc::Catch&)      { push(TObj); }
  void operator()(const bc::NativeImpl&) { killLocals(); doRet(TInitGen); }

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
  void operator()(const bc::CGetS&) { popA(); popC(); push(TInitCell); }

  void operator()(const bc::VGetL& op) {
    throw_oom_only();
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
  void operator()(const bc::VGetS&) { popA(); popC(); push(TRef); }

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
    auto const t1 = popC();
    auto const t2 = popC();
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
  void operator()(const bc::EmptyS&) { popA(); popC(); push(TBool); }
  void operator()(const bc::IssetG&) { popC(); push(TBool); }
  void operator()(const bc::IssetS&) { popA(); popC(); push(TBool); }

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
      isTypeImpl(t1, subObj(*rcls));
      return;
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
    // This might try to allocate a VarEnv, although that probably
    // doesn't actually ever throw OOM ...
    throw_oom_only();

    // This isn't trivial to strength reduce, without a "flip two top
    // elements of stack" opcode.

    auto const t1 = popC();
    auto const t2 = popC();
    auto const v2 = tv(t2);

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
    auto const t1 = popC();
    popA();
    popC();
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
    popC(); popA(); popC();
    push(TInitCell);
  }

  void operator()(const bc::IncDecL& op) {
    auto const loc = locAsCell(op.loc1);
    auto const val = tv(loc);
    if (!val) return push(TInitCell); // Only constants for now

    auto const subop = op.subop;
    auto const pre = subop == IncDecOp::PreInc || subop == IncDecOp::PreDec;
    auto const inc = subop == IncDecOp::PreInc || subop == IncDecOp::PostInc;

    if (!pre) push(loc);

    // We can't constprop with this eval_cell, because of the effects
    // on locals.
    auto resultTy = eval_cell([&] {
      auto c = *val;
      if (inc) {
        cellInc(c);
      } else {
        cellDec(c);
      }
      return c;
    });

    // We may have inferred a TSStr or TSArr with a value here, but
    // at runtime it will not be static.  For now just throw that
    // away.
    if (resultTy.subtypeOf(TStr))      resultTy = TStr;
    else if (resultTy.subtypeOf(TArr)) resultTy = TArr;

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
  void operator()(const bc::IncDecS&) { popA(); popC(); push(TInitCell); }

  void operator()(const bc::BindL& op) {
    throw_oom_only();
    auto const t1 = popV();
    setLocRaw(op.loc1, t1);
    push(t1);
  }

  void operator()(const bc::BindN&) {
    // Could throw_oom_only (except for OOM) if t2 can't be a TObj or
    // TRes.
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
    auto const t1 = popV();
    popA();
    popC();
    push(t1);
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
    killLocals();
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
      return reduce(bc::PopC {},
                    bc::FPushFuncD { op.arg1, v1->m_data.pstr });
    }
    popC();
    fpiPush(ActRec { FPIKind::Func });
  }

  void operator()(const bc::FPushFuncU&) {
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
      return push(TGen);
    case PrepKind::Val: return impl(bc::CGetL { op.loc2 });
    case PrepKind::Ref: return impl(bc::VGetL { op.loc2 });
    }
  }

  void operator()(const bc::FPassN& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown:
      // This could change the type of any local.
      popC();
      killLocals();
      return push(TGen);
    case PrepKind::Val: return impl(bc::CGetN {});
    case PrepKind::Ref: return impl(bc::VGetN {});
    }
  }

  void operator()(const bc::FPassG& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: popC(); return push(TGen);
    case PrepKind::Val:     return impl(bc::CGetG {});
    case PrepKind::Ref:     return impl(bc::VGetG {});
    }
  }

  void operator()(const bc::FPassS& op) {
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: popA(); popC(); return push(TGen);
    case PrepKind::Val:     return impl(bc::CGetS {});
    case PrepKind::Ref:     return impl(bc::VGetS {});
    }
  }

  void operator()(const bc::FPassV& op) {
    nothrow();
    switch (prepKind(op.arg1)) {
    case PrepKind::Unknown: popV(); return push(TGen);
    case PrepKind::Val:     return impl(bc::Unbox {});
    case PrepKind::Ref:     assert(topT().subtypeOf(TRef)); return;
    }
  }

  void operator()(const bc::FPassVNop&) { nothrow(); push(popV()); }

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

  void operator()(const bc::FPassC& op)  { nothrow(); }
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
    case PrepKind::Unknown:  return conservative(op);
    case PrepKind::Val:      return reduce(bc::CGetM { op.mvec },
                                           bc::FPassC { op.arg1 });
    case PrepKind::Ref:      return reduce(bc::VGetM { op.mvec },
                                           bc::FPassVNop { op.arg1 });
    }
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
    // Any include/require (or eval) op kills all locals.
    popC();
    killLocals();
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

  // TODO_5: can we propagate our object type if unique?
  void operator()(const bc::This&)         { push(TObj); }
  void operator()(const bc::LateBoundCls&) { push(TCls); }
  void operator()(const bc::CheckThis&)    {}

  void operator()(const bc::BareThis& op) {
    switch (op.subop) {
    case BareThisOp::Notice:   break;
    case BareThisOp::NoNotice: nothrow(); break;
    }
    push(TOptObj);
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
    readLocals();
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
      setLoc(borrow(m_ctx.func->locals[op.arg1]),
             m_index.lookup_constraint(m_ctx, constraint));
    }
  }

  // These only occur in traits, so we don't need to do better than
  // this.
  void operator()(const bc::Self&)   { push(TCls); }
  void operator()(const bc::Parent&) { push(TCls); }

  void operator()(const bc::CreateCl& op) {
    auto const nargs = op.arg1;
    for (auto i = uint32_t{0}; i < nargs; ++i) popT();
    if (auto const rcls = m_index.resolve_class(m_ctx, op.str2)) {
      return push(objExact(*rcls));
    }
    push(TObj);
  }

  void operator()(const bc::CreateCont&)  { killLocals(); push(TObj); }
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

  void operator()(const bc::AsyncAwait& op) {
    popC();
    push(TInitCell);
    push(TBool);
  }
  void operator()(const bc::AsyncESuspend&) {
    killLocals();
    popC();
    push(TObj);
  }
  void operator()(const bc::AsyncWrapResult&) { popC(); push(TObj); }
  void operator()(const bc::AsyncWrapException&) { popC(); push(TObj); }

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

  void operator()(const bc::LowInvalid&)  { always_assert(!"LowInvalid"); }
  void operator()(const bc::HighInvalid&) { always_assert(!"HighInvalid"); }

private:
  // This implements any opcode conservatively (throws away everything
  // we know about locals).  You can forward to this for bytecodes
  // that aren't really supported yet.
  template<class T>
  void conservative(const T& t) {
    FTRACE(2, "    *** unknown op\n");

    for (uint32_t i = 0; i < t.numPop(); ++i)  popT();
    if (isFCallStar(T::op))                    fpiPop();
    for (uint32_t i = 0; i < t.numPush(); ++i) push(t.pushType(i));

    if (isFPush(T::op)) {
      fpiPush(ActRec { FPIKind::Unknown });
    }

    killLocals();

    // If this instruction has taken edges, we need to propagate the
    // state to them.
    forEachTakenEdge(t, [&] (php::Block& blk) {
      m_propagate(blk, m_state);
    });
  }

private: // member instructions
  struct UnkBase   {};
  struct ThisBase  {};
  struct SPropBase { Type cls; Type prop; };
  typedef boost::variant<UnkBase,ThisBase,SPropBase> Base;

  struct MInstrState {
    explicit MInstrState(const MInstrInfo* info,
                         const MVector& mvec)
      : info(*info)
      , base(UnkBase{})
      , stackIdx(info->valCount() + numVecPops(mvec))
    {}

    const MInstrInfo& info;
    Base base;

    // One above next stack slot to read, going forward in the mvec
    // (deeper to higher on stack).
    int32_t stackIdx;
  };

  /*
   * Local bases are a mild pain, because they can change type
   * depending on the mvector.  The current behavior here is very
   * conservative.
   *
   * Basically for now, if we're about to do property dims and it's
   * not an Obj, we give up, and if we're about to do elem dims and
   * it's not an Arr or Obj, we give up.
   *
   * Also, return UnkBase no matter what---tracking bases isn't
   * implemented yet.
   */
  Base miBaseLoc(const MInstrInfo& info, const MVector& mvec) {
    auto const isDefine = info.getAttr(mvec.lcode) & MIA_define;

    if (isDefine) ensureInit(mvec.locBase);

    auto const locTy = locAsCell(mvec.locBase);

    // Unsetting can turn static strings and arrays non-static.
    if (info.m_instr == MI_UnsetM) {
      if (locTy.strictSubtypeOf(TArr)) {
        setLoc(mvec.locBase, TArr);
      } else if (locTy.strictSubtypeOf(TStr)) {
        setLoc(mvec.locBase, TStr);
      }
      return UnkBase {};
    }

    if (!isDefine) return UnkBase {};

    auto const firstDim = mvec.mcodes[0].mcode;
    if (mcodeMaybePropName(firstDim)) {
      if (!locTy.subtypeOf(TObj)) {
        setLoc(mvec.locBase, TInitCell);
      }
      return UnkBase {};
    }

    if (mcodeMaybeArrayOrMapKey(firstDim) || firstDim == MW) {
      if (locTy.strictSubtypeOf(TArr)) {
        // We're potentially about to mutate any constant or static
        // array, so raise it to TArr for now.
        setLoc(mvec.locBase, TArr);
      } else if (!locTy.subtypeOf(TArr) && !locTy.subtypeOf(TObj)) {
        setLoc(mvec.locBase, TInitCell);
      }
      return UnkBase {};
    }

    return UnkBase {};
  }

  Base miBase(MInstrState& state, const MVector& mvec) {
    switch (mvec.lcode) {
    case LL:  return miBaseLoc(state.info, mvec);
    case LC:
      topC(--state.stackIdx);
      return UnkBase {};
    case LR:
      topR(--state.stackIdx);
      return UnkBase {};
    case LH:
      return ThisBase {};
    case LGL:
      return UnkBase {};
    case LGC:
      topC(--state.stackIdx);
      return UnkBase {};

    case LNL:
      killLocals();
      return UnkBase {};
    case LNC:
      killLocals();
      topC(--state.stackIdx);
      return UnkBase {};

    case LSL:
      {
        auto const cls = topA(state.info.valCount());
        return SPropBase { cls, locAsCell(mvec.locBase) };
      }
    case LSC:
      {
        auto const cls  = topA(state.info.valCount());
        auto const prop = topC(--state.stackIdx);
        return SPropBase { cls, prop };
      }

    case NumLocationCodes:
      break;
    }
    not_reached();
  }

  void miProp(MInstrState& state, Type propKey) {
    state.base = UnkBase {};
  }

  void miElem(MInstrState& state, Type elemKey) {
    state.base = UnkBase {};
  }

  void miNewElem(MInstrState& state) {
    if (!state.info.newElem()) {
      // We're about to fatal ...
      state.base = UnkBase {};
      return;
    }
    state.base = UnkBase {};
  }

  void miIntermediate(MInstrState& state, const MElem& melem) {
    switch (melem.mcode) {
    case MPC:  return miProp(state, topC(--state.stackIdx));
    case MPL:  return miProp(state, locAsCell(melem.immLoc));
    case MPT:  return miProp(state, sval(melem.immStr));

    case MEC:  return miElem(state, topC(--state.stackIdx));
    case MET:  return miElem(state, sval(melem.immStr));
    case MEL:  return miElem(state, locAsCell(melem.immLoc));
    case MEI:  return miElem(state, ival(melem.immInt));

    case MW:   return miNewElem(state);

    case NumMemberCodes:
      break;
    }
    not_reached();
  }

  void miPop(const MInstrState& state, const MVector& mvec) {
    if (mvec.lcode == LSL || mvec.lcode == LSC) {
      assert(state.stackIdx == state.info.valCount() + 1 /* clsref */);
    } else {
      assert(state.stackIdx == state.info.valCount());
    }
    for (auto i = uint32_t{0}; i < numVecPops(mvec); ++i) popT();
  }

  // "Do" an intermediate or final op in the dim but throw away
  // anything we know so far.  This is used to allow most vector
  // instructions to not really be supported for much yet ...
  void miDiscard(MInstrState& state, const MElem& melem) {
    state.base = UnkBase {};
    switch (melem.mcode) {
    case MPC:  topC(--state.stackIdx);   break;
    case MPL:  locAsCell(melem.immLoc);  break;
    case MPT:  sval(melem.immStr);       break;

    case MEC:  topC(--state.stackIdx);   break;
    case MET:  sval(melem.immStr);       break;
    case MEL:  locAsCell(melem.immLoc);  break;
    case MEI:  ival(melem.immInt);       break;

    case MW:                             break;

    case NumMemberCodes:                 break;
    }
  }

  void miFinal(const bc::EmptyM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
    push(TBool);
  }

  void miFinal(const bc::IssetM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
    push(TBool);
  }

  void miFinal(const bc::CGetM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
    push(TInitCell);
  }

  void miFinal(const bc::VGetM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
    push(TRef);
  }

  void miFinal(const bc::SetM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    popC();
    miPop(state, op.mvec);
    push(TInitCell);  // SetM is weird ...
  }

  void miFinal(const bc::SetOpM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    popC();
    miPop(state, op.mvec);
    push(TInitCell);
  }

  void miFinal(const bc::IncDecM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
    push(TInitCell);
  }

  void miFinal(const bc::BindM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    popV();
    miPop(state, op.mvec);
    push(TRef);
  }

  void miFinal(const bc::UnsetM& op, MInstrState& state, const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
  }

  void miFinal(const bc::SetWithRefLM& op,
               MInstrState& state,
               const MElem& melem) {
    miDiscard(state, melem);
    miPop(state, op.mvec);
  }

  void miFinal(const bc::SetWithRefRM& op,
               MInstrState& state,
               const MElem& melem) {
    miDiscard(state, melem);
    popR();
    miPop(state, op.mvec);
  }

  template<class Op>
  void miImpl(const Op& op, const MInstrInfo& info, const MVector& mvec) {
    auto state = MInstrState { &info, mvec };
    // The state before miBase is propagated because wasPEI will be
    // true.
    state.base = miBase(state, mvec);
    miThrow();
    for (auto mInd = size_t{0}; mInd < mvec.mcodes.size() - 1; ++mInd) {
      miIntermediate(state, mvec.mcodes[mInd]);
      // Note: this one might not be necessary: review whether member
      // instructions can ever modify local types on itermediate dims.
      miThrow();
    }
    miFinal(op, state, mvec.mcodes[mvec.mcodes.size() - 1]);
  }

  // MInstrs can throw in between each op, so the states of locals
  // need to be propagated across factored exit edges.
  void miThrow() { m_propagateThrow(m_state); }

  template<class Op>
  void minstr(const Op& op) {
    miImpl(op, getMInstrInfo(Op::op), op.mvec);
  }

private:
  void nothrow()       { FTRACE(2, "    nothrow\n"); m_flags.wasPEI = false; }
  void calledNoReturn(){ m_flags.calledNoReturn = true; }
  void constprop()     { m_flags.canConstProp = true; }
  void nofallthrough() { m_flags.tookBranch = true; }
  void readLocals()    { m_flags.mayReadLocals = true; }
  void doRet(Type t)   { assert(m_state.stack.empty()); m_flags.returned = t; }

  /*
   * It's not entirely clear whether we need to propagate state for a
   * potential OOM, so for now we're annotating the instructions that
   * can only throw OOM separately.
   */
  void throw_oom_only() {
    FTRACE(2, "    throw_oom_only\n");
    m_flags.wasPEI = false;
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
  void readUnknownLocals() { readLocals(); }
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
  Type locRaw(borrowed_ptr<const php::Local> l) {
    readLocals();
    return m_state.locals[l->id];
  }

  void setLocRaw(borrowed_ptr<const php::Local> l, Type t) {
    readLocals();
    m_state.locals[l->id] = t;
  }

  // Read a local type in the sense of CGetL.  (TUninits turn into
  // TInitNull, and potentially reffy types return the "inner" type,
  // which is always a subtype of InitCell.)
  Type locAsCell(borrowed_ptr<const php::Local> l) {
    readLocals();
    auto v = locRaw(l);
    if (v.subtypeOf(TInitCell)) return v;
    if (v.subtypeOf(TUninit))   return TInitNull;
    return TInitCell;
  }

  // Read a local type, dereferencing refs, but without converting
  // potential TUninits to TInitNull.
  Type derefLoc(borrowed_ptr<const php::Local> l) {
    readLocals();
    auto v = locRaw(l);
    if (v.subtypeOf(TCell)) return v;
    return v.couldBe(TUninit) ? TCell : TInitCell;
  }

  void ensureInit(borrowed_ptr<const php::Local> l) {
    readLocals();
    auto v = locRaw(l);
    if (v.couldBe(TUninit)) {
      if (v.subtypeOf(TNull))    return setLocRaw(l, TInitNull);
      if (v.subtypeOf(TUnc))     return setLocRaw(l, TUnc);
      if (v.subtypeOf(TCell))    return setLocRaw(l, TInitCell);
      if (v.subtypeOf(TGen))     return setLocRaw(l, TInitGen);
    }
  }

  bool locCouldBeUninit(borrowed_ptr<const php::Local> l) {
    readLocals();
    return locRaw(l).couldBe(TUninit);
  }

  /*
   * Set a local type in the sense of tvSet.  If the local is boxed or
   * not known to be not boxed, we can't change the type.  May be used
   * to set locals to types that include Uninit.
   */
  void setLoc(borrowed_ptr<const php::Local> l, Type t) {
    readLocals();
    auto v = locRaw(l);
    if (v.subtypeOf(TCell)) m_state.locals[l->id] = t;
  }

  borrowed_ptr<php::Local> findLocal(SString name) {
    readLocals();
    for (auto& l : m_ctx.func->locals) {
      if (l->name->same(name)) return borrow(l);
    }
    return nullptr;
  }

  borrowed_ptr<php::Local> findLocalById(int32_t id) {
    readLocals();
    assert(id < m_ctx.func->locals.size());
    return borrow(m_ctx.func->locals[id]);
  }

private:
  const Index& m_index;
  const Context m_ctx;
  State& m_state;
  StepFlags& m_flags;
  Propagate m_propagate;
  PropagateThrow m_propagateThrow;
};

//////////////////////////////////////////////////////////////////////

struct Interpreter {
  Interpreter(const Index* index,
              Context ctx,
              const php::Block* blk,
              State& state)
    : m_index(*index)
    , m_ctx(ctx)
    , m_blk(*blk)
    , m_state(state)
  {}

  /*
   * Run the interpreter on a whole block, propagating states using
   * the propagate function.
   */
  template<class Propagate, class MergeReturn>
  void run(Propagate propagate, MergeReturn mergeReturn) {
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
    auto propagate       = [&] (php::Block&, const State&) {};
    auto propagate_throw = [&] (const State& state) {};
    auto flags           = StepFlags{};

    InterpStepper<decltype(propagate),decltype(propagate_throw)> stepper(
      &m_index,
      m_ctx,
      m_state,
      flags,
      propagate,
      propagate_throw
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
    default: break;
    }

    FTRACE(2, "  {}\n", show(*it));
    visit(*it++, st);
  }

  template<class Iterator, class Propagate>
  StepFlags interpOps(Iterator& iter, Iterator stop, Propagate propagate) {
    auto propagate_throw = [&] (const State& state) {
      for (auto& factored : m_blk.factoredExits) {
        propagate(*factored, without_stacks(state));
      }
    };

    auto flags = StepFlags{};
    InterpStepper<Propagate,decltype(propagate_throw)> stepper(
      &m_index,
      m_ctx,
      m_state,
      flags,
      propagate,
      propagate_throw
    );

    // Make a copy of the state (except stacks) in case we need to
    // propagate across factored exits (if it's a PEI).
    auto const stateBefore = without_stacks(m_state);

    interpStep(stepper, iter, stop);

    if (flags.wasPEI) {
      FTRACE(2, "   PEI.\n");
      propagate_throw(stateBefore);
    }

    return flags;
  }

private:
  const Index& m_index;
  const Context m_ctx;
  const php::Block& m_blk;
  State& m_state;
};

//////////////////////////////////////////////////////////////////////

folly::Optional<AssertTOp> assertTOpFor(Type t) {
#define ASSERTT_OP(y) \
  if (t.subtypeOf(T##y)) return AssertTOp::y;
  ASSERTT_OPS
#undef ASSERTT_OP
  return folly::none;
}

template<class ObjBC, class TyBC, class ArgType>
folly::Optional<Bytecode> makeAssert(ArgType arg, Type t) {
  if (t.strictSubtypeOf(TObj)) {
    auto const dobj = dobj_of(t);
    auto const exact = dobj.type == DObj::Exact;
    return Bytecode { ObjBC { arg, exact, dobj.cls.name() } };
  }

  if (auto const op = assertTOpFor(t)) {
    return Bytecode { TyBC { arg, *op } };
  }
  return folly::none;
}

template<class Gen>
void insert_assertions(const php::Func& func,
                       const Bytecode& bcode,
                       const State& state,
                       bool mayReadLocals,
                       Gen gen) {
  if (!options.FilterAssertions || mayReadLocals) {
    for (size_t i = 0; i < state.locals.size(); ++i) {
      auto const realT = state.locals[i];
      auto const op = makeAssert<bc::AssertObjL,bc::AssertTL>(
        borrow(func.locals[i]), realT
      );
      if (op) gen(*op);
    }
  }

  if (!options.InsertStackAssertions) return;

  /*
   * This doesn't need to account for ActRecs on the fpiStack, because
   * no instruction in an FPI region can ever consume a stack value
   * from above the pre-live ActRec.
   */
  assert(state.stack.size() >= bcode.numPop());
  auto stackIdx = state.stack.size() - 1;
  for (auto i = size_t{0}; i < bcode.numPop(); ++i, --stackIdx) {
    auto const realT = state.stack[stackIdx];

    if (options.FilterAssertions &&
        !realT.strictSubtypeOf(stack_flav(realT))) {
      continue;
    }

    auto const op = makeAssert<bc::AssertObjStk,bc::AssertTStk>(
      static_cast<int32_t>(i), realT
    );
    if (op) gen(*op);
  }
}

//////////////////////////////////////////////////////////////////////

template<class Gen>
bool propagate_constants(const Bytecode& op, const State& state, Gen gen) {
  auto const numPop  = op.numPop();
  auto const numPush = op.numPush();
  auto const stkSize = state.stack.size();

  // All outputs of the instruction must have constant types for this
  // to be allowed.
  for (auto i = size_t{0}; i < numPush; ++i) {
    if (!tv(state.stack[stkSize - i - 1])) return false;
  }

  // Pop the inputs, and push the constants.
  for (auto i = size_t{0}; i < numPop; ++i) {
    switch (op.popFlavor(i)) {
    case Flavor::C:  gen(bc::PopC {}); break;
    case Flavor::V:  gen(bc::PopV {}); break;
    case Flavor::A:  gen(bc::PopA {}); break;
    case Flavor::R:  not_reached();    break;
    case Flavor::F:  not_reached();    break;
    case Flavor::U:  not_reached();    break;
    }
  }

  for (auto i = size_t{0}; i < numPush; ++i) {
    auto const v = tv(state.stack[stkSize - i - 1]);
    switch (v->m_type) {
    case KindOfUninit:        not_reached();          break;
    case KindOfNull:          gen(bc::Null {});       break;
    case KindOfBoolean:
      if (v->m_data.num) {
        gen(bc::True {});
      } else {
        gen(bc::False {});
      }
      break;
    case KindOfInt64:
      gen(bc::Int { v->m_data.num });
      break;
    case KindOfDouble:
      gen(bc::Double { v->m_data.dbl });
      break;
    case KindOfStaticString:
      gen(bc::String { v->m_data.pstr });
      break;
    case KindOfArray:
      gen(bc::Array { v->m_data.parr });
      break;

    case KindOfRef:
    case KindOfResource:
    case KindOfString:
    default:
      always_assert(0 && "invalid constant in propagate_constants");
    }

    // Special case for FPass* instructions.  We just put a C on the
    // stack, so we need to get it to be an F.
    if (isFPassStar(op.op)) {
      // We should only ever const prop for FPassL right now.
      always_assert(numPush == 1 && op.op == Op::FPassL);
      gen(bc::FPassC { op.FPassL.arg1 });
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

std::vector<Bytecode> optimize_block(const Index& index,
                                     const Context ctx,
                                     php::Block* const blk,
                                     State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  Interpreter interp { &index, ctx, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(newBCs.back()));
    };

    auto const preState = state;
    auto const flags    = interp.step(op);

    if (flags.calledNoReturn) {
      gen(op);
      gen(bc::BreakTraceHint {}); // The rest of this code is going to
                                  // be unreachable.
      // It would be nice to put a fatal here, but we can't because it
      // will mess up the bytecode invariant about blocks
      // not-reachable via fallthrough if the stack depth is non-zero.
      // It can also mess up FPI regions.
      continue;
    }

    if (options.InsertAssertions) {
      insert_assertions(*ctx.func, op, preState, flags.mayReadLocals, gen);
    }

    if (options.RemoveDeadBlocks && flags.tookBranch) {
      switch (op.op) {
      case Op::JmpNZ:  blk->fallthrough = op.JmpNZ.target; break;
      case Op::JmpZ:   blk->fallthrough = op.JmpZ.target;  break;
      default:
        // No switch, etc support.
        always_assert(0 && "unsupported tookBranch case");
      }
      continue;
    }

    if (options.ConstantProp && flags.canConstProp) {
      if (propagate_constants(op, state, gen)) continue;
    }

    if (options.StrengthReduceBC && flags.strengthReduced) {
      for (auto& hh : *flags.strengthReduced) gen(hh);
      continue;
    }

    gen(op);
  }

  return newBCs;
}

void do_optimize(const Index& index, const FuncAnalysis& ainfo) {
  FTRACE(2, "{:-^70}\n", "Optimize Func");

  for (auto& blk : ainfo.rpoBlocks) {
    FTRACE(2, "block #{}\n", blk->id);

    auto const& state = ainfo.bdata[blk->id].stateIn;
    if (!state.initialized) {
      FTRACE(2, "   unreachable\n");
      if (!options.InsertAssertions) continue;
      auto const srcLoc = blk->hhbcs.front().srcLoc;
      blk->hhbcs = {
        bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
        bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
      };
      blk->fallthrough = nullptr;
      continue;
    }

    blk->hhbcs = optimize_block(index, ainfo.ctx, blk, state);
  }
}

//////////////////////////////////////////////////////////////////////

State entry_state(const php::Func& func) {
  State ret;
  ret.initialized = true;
  ret.locals.resize(func.locals.size());

  uint32_t locId = 0;
  for (; locId < func.params.size(); ++locId) {
    // Parameters may be Uninit (i.e. no InitCell).  Also note that if
    // a function takes a param by ref, it might come in as a Cell
    // still if FPassC was used.
    ret.locals[locId] = func.params[locId].byRef ? TGen : TCell;
  }

  /*
   * Closures have a hidden local that's always the first local, which
   * stores the closure itself.
   *
   * TODO_4: make this a TObj=ClosureType.
   */
  if (func.isClosureBody) {
    assert(locId < ret.locals.size());
    ret.locals[locId++] = TObj;
  }

  for (; locId < func.locals.size(); ++locId) {
    /*
     * Generators and closures don't (necessarily) start with the
     * frame locals uninitialized.
     *
     * Ideas:
     *
     *  - maybe we can do better for generators by adding edges from
     *    the yields to the top of the generator
     *
     *  - for closures, since they are all unique to their creation
     *    sites and in the same unit, looking at the CreateCl could
     *    tell the types of used vars, even in single unit mode.
     */
    ret.locals[locId] =
      func.isGeneratorBody || func.isClosureBody ? TGen : TUninit;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool operator==(const ActRec& a, const ActRec& b) {
  auto const fsame =
    a.func.hasValue() != b.func.hasValue() ? false :
    a.func.hasValue() ? a.func->same(*b.func) :
    true;
  return a.kind == b.kind && fsame;
}

bool operator==(const State& a, const State& b) {
  return a.initialized == b.initialized &&
    a.locals == b.locals &&
    a.stack == b.stack &&
    a.fpiStack == b.fpiStack;
}

bool operator!=(const ActRec& a, const ActRec& b) { return !(a == b); }
bool operator!=(const State& a, const State& b) { return !(a == b); }

FuncAnalysis::FuncAnalysis(Context ctx)
  : ctx(ctx)
  , rpoBlocks(rpoSortAddDVs(*ctx.func))
  , bdata(ctx.func->blocks.size())
  , inferredReturn(TBottom)
{
  for (auto rpoId = size_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    bdata[rpoBlocks[rpoId]->id].rpoId = rpoId;
  }
}

//////////////////////////////////////////////////////////////////////

FuncAnalysis analyze_func(const Index& index, Context const ctx) {
  assert(ctx.func != ctx.unit->pseudomain.get() &&
         "pseudomains not supported");
  FTRACE(2, "{:-^70}\n", "Analyze");

  FuncAnalysis ai(ctx);

  auto rpoId = [&] (borrowed_ptr<php::Block> blk) {
    return ai.bdata[blk->id].rpoId;
  };

  /*
   * Set of RPO ids that still need to be visited.
   *
   * Initially, we need each entry block in this list.  As we visit
   * blocks, we propagate states to their successors and across their
   * back edges---when state merges cause a change to the block
   * stateIn, we will add it to this queue so it gets visited again.
   */
  std::set<uint32_t> incompleteQ;

  /*
   * We need to initialize the states for all function entries
   * (i.e. each dv init and the main entry), and all of them count as
   * places the function could be entered, so they all must be visited
   * at least once (add them to incompleteQ).
   */
  {
    auto const entryState = entry_state(*ctx.func);
    for (auto& param : ctx.func->params) {
      if (auto const dv = param.dvEntryPoint) {
        ai.bdata[dv->id].stateIn = entry_state(*ctx.func);
        incompleteQ.insert(rpoId(dv));
      }
    }
    ai.bdata[ctx.func->mainEntry->id].stateIn = entryState;
    incompleteQ.insert(rpoId(ctx.func->mainEntry));
  }

  // For debugging, count how many times basic blocks get interpreted.
  auto interp_counter = uint32_t{0};

  /*
   * Iterate until a fixed point.
   *
   * We know a fixed point must occur because types increase
   * monotonically.
   *
   * We may visit a block up to as many times as there are state
   * variables coming into the block (locals and eval stack slots),
   * times the height of the type lattice.
   *
   * Each time a stateIn for a block changes, we re-insert the block's
   * rpo ID in incompleteQ.  Since incompleteQ is ordered, we'll
   * always visit blocks with earlier RPO ids first, which hopefully
   * means less iterations.
   */
  while (!incompleteQ.empty()) {
    auto blk = ai.rpoBlocks[*begin(incompleteQ)];
    incompleteQ.erase(begin(incompleteQ));

    FTRACE(2, "block #{}\nin {}", blk->id,
      state_string(*ctx.func, ai.bdata[blk->id].stateIn));

    ++interp_counter;

    auto propagate = [&] (php::Block& target, const State& st) {
      FTRACE(2, "     -> {}\n", target.id);
      FTRACE(4, "target old {}\n",
        state_string(*ctx.func, ai.bdata[target.id].stateIn));

      if (merge_into(ai.bdata[target.id].stateIn, st)) {
        incompleteQ.insert(rpoId(&target));
      }
      FTRACE(4, "target new {}\n",
        state_string(*ctx.func, ai.bdata[target.id].stateIn));
    };

    auto mergeReturn = [&] (Type type) {
      ai.inferredReturn = union_of(ai.inferredReturn, type);
    };

    auto stateOut = ai.bdata[blk->id].stateIn;
    Interpreter interp(&index, ctx, blk, stateOut);
    interp.run(propagate, mergeReturn);
  }

  /*
   * If inferredReturn is TBottom, the callee didn't execute a return
   * at all.  (E.g. it unconditionally throws, or is an abstract
   * function body.)
   *
   * In this case, we leave the return type as TBottom, to indicate
   * the same to callers.
   */
  assert(ai.inferredReturn.subtypeOf(TGen));

  // For debugging, print the final input states for each block.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '=') + "\n";
    auto const sep = std::string(60, '-') + "\n";
    auto ret = folly::format(
      "{}{}{}\nAnalysis results ({} block interps):\n{}",
      bsep,
      ctx.cls ? folly::format("{}::", ctx.cls->name->data()).str()
              : std::string(),
      ctx.func->name->data(),
      interp_counter,
      bsep
    ).str();
    for (auto& bd : ai.bdata) {
      ret += folly::format(
        "{}block {}:\nin {}",
        sep,
        ai.rpoBlocks[bd.rpoId]->id,
        state_string(*ctx.func, bd.stateIn)
      ).str();
    }
    ret += sep + bsep;
    ret += folly::format(
      "Inferred return type: {}\n", show(ai.inferredReturn)).str();
    ret += bsep;
    return ret;
  }());

  return ai;
}

void optimize_func(const Index& index, const FuncAnalysis& ainfo) {
  do_optimize(index, ainfo);
}

void analyze_and_optimize_func(const Index& index, Context ctx) {
  optimize_func(index, analyze_func(index, ctx));
}

//////////////////////////////////////////////////////////////////////

}}
