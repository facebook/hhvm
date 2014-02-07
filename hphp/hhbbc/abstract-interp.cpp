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

#include "folly/Conv.h"
#include "folly/Optional.h"
#include "folly/String.h"

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/ext/ext_math.h" // f_abs

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/type-arith.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_empty("");
const StaticString s_extract("extract");
const StaticString s_Exception("Exception");
const StaticString s_Continuation("Continuation");
const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");

//////////////////////////////////////////////////////////////////////

std::string fpiKindStr(FPIKind k) {
  switch (k) {
  case FPIKind::Unknown:     return "unk";
  case FPIKind::CallableArr: return "arr";
  case FPIKind::Func:        return "func";
  case FPIKind::Ctor:        return "ctor";
  case FPIKind::ObjMeth:     return "objm";
  case FPIKind::ClsMeth:     return "clsm";
  case FPIKind::ObjInvoke:   return "invoke";
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

  if (!st.initialized) {
    ret = "state: uninitialized\n";
    return ret;
  }

  ret = "state:\n";
  for (auto i = size_t{0}; i < st.locals.size(); ++i) {
    ret += folly::format("${: <8} :: {}\n",
      f.locals[i]->name
        ? std::string(f.locals[i]->name->data())
        : folly::format("<unnamed{}>", i).str(),
      show(st.locals[i])
    ).str();
  }

  for (auto i = size_t{0}; i < st.stack.size(); ++i) {
    ret += folly::format("stk[{:02}] :: {}\n",
      i,
      show(st.stack[i])
    ).str();
  }

  if (st.thisAvailable) { ret += "$this is not null\n"; }
  for (auto& kv : st.privateProperties) {
    ret += folly::format("$this->{: <14} :: {}\n",
      kv.first->data(),
      show(kv.second)
    ).str();
  }
  for (auto& kv : st.privateStatics) {
    ret += folly::format("self::${: <14} :: {}\n",
      kv.first->data(),
      show(kv.second)
    ).str();
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

// Returns whether anything changed in the dst state.
bool merge_into(PropState& dst, const PropState& src) {
  assert(dst.size() == src.size());

  auto changed = false;

  auto dstIt = begin(dst);
  auto srcIt = begin(src);
  for (; dstIt != end(dst); ++dstIt, ++srcIt) {
    assert(srcIt != end(src));
    assert(srcIt->first == dstIt->first);
    auto const newT = union_of(dstIt->second, srcIt->second);
    if (newT != dstIt->second) {
      changed = true;
      dstIt->second = newT;
    }
  }

  return changed;
}

bool merge_into(ActRec& dst, const ActRec& src) {
  if (dst.kind != src.kind) {
    dst = ActRec { FPIKind::Unknown };
    return true;
  }
  if (dst != src) {
    dst = ActRec { src.kind };
    return true;
  }
  return false;
}

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

  auto changed = false;

  auto const available = dst.thisAvailable && src.thisAvailable;
  if (available != dst.thisAvailable) {
    changed = true;
    dst.thisAvailable = available;
  }

  for (auto i = size_t{0}; i < dst.stack.size(); ++i) {
    auto const newT = union_of(dst.stack[i], src.stack[i]);
    if (dst.stack[i] != newT) {
      changed = true;
      dst.stack[i] = newT;
    }
  }

  for (auto i = size_t{0}; i < dst.locals.size(); ++i) {
    auto const newT = union_of(dst.locals[i], src.locals[i]);
    if (dst.locals[i] != newT) {
      changed = true;
      dst.locals[i] = newT;
    }
  }

  for (auto i = size_t{0}; i < dst.fpiStack.size(); ++i) {
    if (merge_into(dst.fpiStack[i], src.fpiStack[i])) {
      changed = true;
    }
  }

  if (merge_into(dst.privateProperties, src.privateProperties)) {
    changed = true;
  }
  if (merge_into(dst.privateStatics, src.privateStatics)) {
    changed = true;
  }

  return changed;
}

// Return a copy of a State without copying either the evaluation
// stack or FPI stack.
State without_stacks(State const& src) {
  auto ret = State{};
  ret.initialized       = src.initialized;
  ret.thisAvailable     = src.thisAvailable;
  ret.locals            = src.locals;
  ret.privateProperties = src.privateProperties;
  ret.privateStatics    = src.privateStatics;
  return ret;
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

// Some member instruction type-system predicates.

bool couldBeEmptyish(Type ty) {
  return ty.couldBe(TNull) ||
         ty.couldBe(sval(s_empty.get())) ||
         ty.couldBe(TFalse);
}

bool elemCouldPromoteToArr(Type ty) { return couldBeEmptyish(ty); }
bool propCouldPromoteToObj(Type ty) { return couldBeEmptyish(ty); }

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

  void operator()(const bc::NewCol& op) {
    auto const name = collectionTypeToString(op.arg1);
    push(objExact(m_index.builtin_class(m_ctx, name)));
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
    return push(subObj(m_index.builtin_class(m_ctx, s_Exception.get())));
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
    push(objExact(m_index.builtin_class(m_ctx, s_Continuation.get())));
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
  // we know in the state).  You can forward to this for bytecodes
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
    killThisProps();
    killSelfProps();

    // If this instruction has taken edges, we need to propagate the
    // state to them.
    forEachTakenEdge(t, [&] (php::Block& blk) {
      m_propagate(blk, m_state);
    });
  }

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
     * Known to be contained in the current frame as a local, as
     * $this, or known to be contained by the evaluation stack.  Only
     * possible as initial bases.
     */
    Frame,
    FrameThis,
    EvalStack,

    // TODO: global
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

  static std::string base_string(const folly::Optional<Base>& b) {
    if (!b) return "[none]";
    auto const locStr = [&]() -> const char* {
      switch (b->loc) {
      case BaseLoc::PostElem:      return "PostElem";
      case BaseLoc::PostProp:      return "PostProp";
      case BaseLoc::StaticObjProp: return "StaticObjProp";
      case BaseLoc::Frame:         return "Frame";
      case BaseLoc::FrameThis:     return "FrameThis";
      case BaseLoc::EvalStack:     return "EvalStack";
      }
      not_reached();
    }();
    return folly::format(
      "{: <8}  ({: <14} {: <8} @ {})",
      show(b->type),
      locStr,
      show(b->locTy),
      b->locName ? b->locName->data() : "?"
    ).str();
  }

  struct MInstrState {
    explicit MInstrState(const MInstrInfo* info,
                         const MVector& mvec)
      : info(*info)
      , mvec(mvec)
      , stackIdx(info->valCount() + numVecPops(mvec))
    {}

    // Return the current MElem.  Only valid after the base has been
    // processed.
    const MElem& melem() const {
      assert(mInd < mvec.mcodes.size());
      return mvec.mcodes[mInd];
    }

    // Return the current MemberCode.  Only valid after the base has
    // been processed.
    MemberCode mcode() const { return melem().mcode; }

    const MInstrInfo& info;
    const MVector& mvec;

    // Current base.  folly::none if we know nothing about the current
    // base.
    folly::Optional<Base> base;

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

  bool couldBeThisObj(const folly::Optional<Base>& b) const {
    if (!b) return true;
    auto const thisTy = thisType();
    return b->type.couldBe(thisTy ? *thisTy : TObj);
  }

  bool mustBeThisObj(const folly::Optional<Base>& b) const {
    if (!b) return false;
    if (b->loc == BaseLoc::FrameThis) return true;
    if (auto const ty = thisType())   return b->type.subtypeOf(*ty);
    return false;
  }

  bool couldBeInThis(const folly::Optional<Base>& b) const {
    if (!b) return true;
    if (b->loc == BaseLoc::PostProp) {
      auto const thisTy = thisType();
      if (!thisTy) return true;
      if (!b->locTy.couldBe(*thisTy)) return false;
      if (b->locName) {
        return isTrackedThisProp(b->locName);
      }
      return true;
    }
    return false;
  }

  bool couldBeInSelf(const folly::Optional<Base>& b) const {
    if (!b) return true;
    if (b->loc == BaseLoc::StaticObjProp) {
      auto const selfTy = selfCls();
      return !selfTy || b->locTy.couldBe(*selfTy);
    }
    return false;
  }

  // This helper can probably go away once we have enough coverage to
  // make the base not be Optional<Base>.
  SString baseLocName(const folly::Optional<Base>& b) const {
    return b ? b->locName : nullptr;
  }

  //////////////////////////////////////////////////////////////////////

  void handleInThisPropD(MInstrState& state) {
    if (!couldBeInThis(state.base)) return;

    if (auto const name = baseLocName(state.base)) {
      auto const ty = thisPropAsCell(name);
      if (ty && propCouldPromoteToObj(*ty)) {
        // Note: we could merge Obj=stdClass here, but aren't doing so
        // yet.
        mergeThisProp(name, TObj);
      }
      return;
    }

    mergeEachThisPropRaw([&] (Type t) {
      return propCouldPromoteToObj(t) ? TObj : TBottom;
    });
  }

  void handleInThisElemD(MInstrState& state) {
    if (!couldBeInThis(state.base)) return;

    if (auto const name = baseLocName(state.base)) {
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

  void handleInSelfPropD(MInstrState& state) {
    if (!couldBeInSelf(state.base)) return;

    if (auto const name = baseLocName(state.base)) {
      auto const ty = thisPropAsCell(name);
      if (ty && propCouldPromoteToObj(*ty)) {
        // Note: similar to handleInThisPropD, logically this could be
        // merging Obj=stdClass.
        mergeSelfProp(name, TObj);
      }
      return;
    }

    loseNonRefSelfPropTypes();
  }

  void handleInSelfElemD(MInstrState& state) {
    if (!couldBeInSelf(state.base)) return;

    if (auto const name = baseLocName(state.base)) {
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

    if (auto const name = baseLocName(state.base)) {
      auto const ty = selfPropAsCell(name);
      if (ty) mergeSelfProp(name, loosen_statics(*ty));
    } else {
      mergeEachSelfPropRaw(loosen_statics);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // base ops

  /*
   * Local bases can change the type of the local depending on the
   * mvector, and the next dim.  The current behavior here is very
   * conservative.
   *
   * Basically for now, if we're about to do property dims and it's
   * not an Obj, we give up, and if we're about to do elem dims and
   * it's not an Arr or Obj, we give up.
   *
   * TODO(#3343813): make this more precise.
   */
  Base miBaseLoc(const MInstrState& state) {
    auto& info = state.info;
    auto& mvec = state.mvec;
    bool const isDefine = info.getAttr(mvec.lcode) & MIA_define;

    if (isDefine) ensureInit(mvec.locBase);

    auto const locTy = derefLoc(mvec.locBase);
    if (info.m_instr == MI_UnsetM) {
      // Unsetting can turn static strings and arrays non-static.
      auto const loose = loosen_statics(locTy);
      setLoc(mvec.locBase, loose);
      return Base { loose, BaseLoc::Frame };
    }

    if (!isDefine) {
      return Base { locTy, BaseLoc::Frame };
    }

    auto const firstDim = mvec.mcodes[0].mcode;
    if (mcodeIsProp(firstDim)) {
      if (!locTy.subtypeOf(TObj)) {
        setLoc(mvec.locBase, TInitCell);
      }
    }

    if (mcodeIsElem(firstDim) || firstDim == MemberCode::MW) {
      if (locTy.strictSubtypeOf(TArr)) {
        // We're potentially about to mutate any constant or static
        // array, so raise it to TArr for now.
        setLoc(mvec.locBase, TArr);
      } else if (!locTy.subtypeOfAny(TArr, TObj)) {
        // We're not handling things other than TArr and TObj subtypes
        // so far.
        setLoc(mvec.locBase, TInitCell);
      }
    }

    return Base { locAsCell(mvec.locBase), BaseLoc::Frame };
  }

  folly::Optional<Base> miBase(MInstrState& state) {
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
      // TODO: return global bases so these don't kill object
      // properties.
      return folly::none;
    case LGC:
      topC(--state.stackIdx);
      return folly::none;

    case LNL:
      // TODO: return frame bases so these don't always kill object
      // properties.
      killLocals();
      return folly::none;
    case LNC:
      killLocals();
      topC(--state.stackIdx);
      return folly::none;

    case LSL:
      {
        // TODO: return StaticObjProp bases so these don't always kill
        // object properties.
        UNUSED auto const cls = topA(state.info.valCount());
        return folly::none;
      }
    case LSC:
      {
        auto const cls  = topA(state.info.valCount());
        auto const prop = tv(topC(--state.stackIdx));
        auto const self = selfCls();
        if (self && cls.subtypeOf(*self) &&
            prop && prop->m_type == KindOfStaticString) {
          if (auto const ty = selfPropAsCell(prop->m_data.pstr)) {
            return Base { *ty, BaseLoc::StaticObjProp, cls,
              prop->m_data.pstr };
          }
        }
        return Base { TCell, BaseLoc::StaticObjProp, cls, nullptr };
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

    if (!state.base) return;

    // We know for sure we're going to be in an object property.
    if (state.base->type.subtypeOf(TObj)) {
      state.base = Base { TInitCell,
                          BaseLoc::PostProp,
                          state.base->type,
                          name };
      return;
    }
    // TODO(#3343813): if it must be null, false, or "" we could use a
    // exact PostProp of Obj=stdclass to avoid future dims returning
    // couldBeThisObj.

    /*
     * Otherwise, intermediate props with define can promote a null,
     * false, or "" to stdClass.  Those cases, and others, if it's not
     * MIA_define, will set the base to a null value in tvScratch.
     * The base may also legitimately be an object and our next base
     * is in an object property.
     *
     * We conservatively treat all these cases as "possibly" being
     * inside of an object property with "PostProp" with locType TTop.
     */
    state.base = Base { TInitCell, BaseLoc::PostProp, TTop, name };
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

    if (!state.base) return;

    if (state.base->type.subtypeOf(TArr)) {
      state.base = Base { TInitCell, BaseLoc::PostElem, state.base->type };
      return;
    }
    if (state.base->type.subtypeOf(TStr)) {
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
      // This path is about to fatal.
      state.base = folly::none;
      return;
    }
    handleInThisNewElem(state);
    handleInSelfNewElem(state);

    if (!state.base) return;

    if (state.base->type.subtypeOf(TArr)) {
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
      state.base = Base { TNull, BaseLoc::PostElem, state.base->type };
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

  // "Do" a final op in the dim but throw away anything we know so
  // far.  This is used to allow the SetWithRef final ops to not
  // really be supported for much ...
  void miDiscard(MInstrState& state) {
    killThisProps();
    killSelfProps();
    state.base = folly::none;
    if (state.mcode() != MemberCode::MW) mcodeKey(state);
  }

  void miFinal(MInstrState& state, const bc::SetWithRefLM& op) {
    miDiscard(state);
    miPop(state);
  }

  void miFinal(MInstrState& state, const bc::SetWithRefRM& op) {
    miDiscard(state);
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
  void readLocals()    { m_flags.mayReadLocals = true; }

  void doRet(Type t) {
    readLocals();
    assert(m_state.stack.empty());
    m_flags.returned = t;
  }

  /*
   * It's not entirely clear whether we need to propagate state for a
   * potential OOM, so for now we're annotating the instructions that
   * can only throw OOM separately.
   *
   * TODO(#3367943): we don't throw on oom until a surprise flag
   * check, so this should be removable.
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
    auto const it = m_state.privateProperties.find(name);
    if (it != end(m_state.privateProperties)) {
      return &it->second;
    }
    return nullptr;
  }

  bool isTrackedThisProp(SString name) const { return thisPropRaw(name); }

  void killThisProps() {
    FTRACE(2, "    killThisProps\n");
    for (auto& kv : m_state.privateProperties) kv.second = TGen;
  }

  folly::Optional<Type> thisPropAsCell(SString name) const {
    auto const t = thisPropRaw(name);
    if (!t) return folly::none;
    if (t->subtypeOf(TInitCell)) return *t;
    if (t->subtypeOf(TUninit))   return TInitNull;
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
    for (auto& kv : m_state.privateProperties) {
      mergeThisProp(kv.first, fn(kv.second));
    }
  }

  void unsetThisProp(SString name) { mergeThisProp(name, TUninit); }
  void unsetUnknownThisProp() {
    for (auto& kv : m_state.privateProperties) {
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
    for (auto& kv : m_state.privateProperties) {
      if (kv.second.subtypeOf(TCell)) kv.second = TCell;
    }
  }

private: // properties on self::
  // Similar to $this properties above, we only track control-flow
  // insensitive types for these.

  Type* selfPropRaw(SString name) const {
    auto it = m_state.privateStatics.find(name);
    if (it != end(m_state.privateStatics)) {
      return &it->second;
    }
    return nullptr;
  }

  void killSelfProps() {
    FTRACE(2, "    killSelfProps\n");
    for (auto& kv : m_state.privateStatics) kv.second = TGen;
  }

  void killSelfProp(SString name) const {
    FTRACE(2, "    killSelfProp {}\n", name->data());
    if (auto t = selfPropRaw(name)) *t = TGen;
  }

  folly::Optional<Type> selfPropAsCell(SString name) const {
    auto const t = selfPropRaw(name);
    if (!t) return folly::none;
    if (t->subtypeOf(TInitCell)) return *t;
    if (t->subtypeOf(TUninit))   return TInitNull;
    return TInitCell;
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
    for (auto& kv : m_state.privateStatics) {
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
    for (auto& kv : m_state.privateStatics) {
      if (kv.second.subtypeOf(TInitCell)) kv.second = TCell;
    }
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
  template<class Propagate, class MergeReturn, class MergePrivates>
  void run(Propagate propagate,
           MergeReturn mergeReturn,
           MergePrivates mergePrivates) {
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

    mergePrivates(m_state.privateProperties, m_state.privateStatics);

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
    auto const op = dobj.type == DObj::Exact ? AssertObjOp::Exact
                                             : AssertObjOp::Sub;
    return Bytecode { ObjBC { arg, dobj.cls.name(), op } };
  }
  if (is_opt(t) && t.strictSubtypeOf(TOptObj)) {
    auto const dobj = dobj_of(t);
    auto const op = dobj.type == DObj::Exact ? AssertObjOp::OptExact
                                             : AssertObjOp::OptSub;
    return Bytecode { ObjBC { arg, dobj.cls.name(), op } };
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

State entry_state(const Index& index,
                  Context const ctx,
                  ClassAnalysis* clsAnalysis) {
  State ret;
  ret.initialized = true;
  ret.thisAvailable = false;
  ret.locals.resize(ctx.func->locals.size());

  uint32_t locId = 0;
  for (; locId < ctx.func->params.size(); ++locId) {
    // Parameters may be Uninit (i.e. no InitCell).  Also note that if
    // a function takes a param by ref, it might come in as a Cell
    // still if FPassC was used.
    ret.locals[locId] = ctx.func->params[locId].byRef ? TGen : TCell;
  }

  /*
   * Closures have a hidden local that's always the first local, which
   * stores the closure itself.
   */
  if (ctx.func->isClosureBody) {
    assert(locId < ret.locals.size());
    assert(ctx.func->cls);
    auto const rcls = index.resolve_class(ctx, ctx.func->cls->name);
    assert(rcls && "Closure classes must always be unique and must resolve");
    ret.locals[locId++] = objExact(*rcls);
  }

  for (; locId < ctx.func->locals.size(); ++locId) {
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
      ctx.func->isGeneratorBody || ctx.func->isClosureBody ? TGen : TUninit;
  }

  /*
   * Get private properties into the entry state.  If there is a
   * clsAnalysis object, we should use the state from there.
   * Otherwise use the best known information in the index.
   */
  ret.privateProperties =
    clsAnalysis ? clsAnalysis->privateProperties :
    ctx.cls     ? index.lookup_private_props(ctx.cls)
                : PropState{};
  ret.privateStatics =
    clsAnalysis ? clsAnalysis->privateStatics :
    ctx.cls     ? index.lookup_private_statics(ctx.cls)
                : PropState{};

  return ret;
}

/*
 * Closures inside of classes are analyzed in the context they are
 * created in (this affects accessibility rules, access to privates,
 * etc).
 *
 * Note that in the interpreter code, ctx.func->cls is not
 * necessarily the same as ctx.cls because of closures.
 */
Context adjust_closure_context(Context ctx) {
  if (ctx.cls && ctx.cls->closureContextCls) {
    ctx.cls = ctx.cls->closureContextCls;
  }
  return ctx;
}

FuncAnalysis do_analyze(const Index& index,
                        Context const inputCtx,
                        ClassAnalysis* clsAnalysis) {
  assert(inputCtx.func != inputCtx.unit->pseudomain.get() &&
         "pseudomains not supported");
  FTRACE(2, "{:-^70}\n", "Analyze");

  auto const ctx = adjust_closure_context(inputCtx);
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
    auto const entryState = entry_state(index, ctx, clsAnalysis);
    for (auto& param : ctx.func->params) {
      if (auto const dv = param.dvEntryPoint) {
        ai.bdata[dv->id].stateIn = entryState;
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

    auto mergePrivates = [&] (const PropState& props,
                              const PropState& statics) {
      if (!clsAnalysis) return;
      merge_into(clsAnalysis->privateProperties, props);
      merge_into(clsAnalysis->privateStatics, statics);
    };

    auto stateOut = ai.bdata[blk->id].stateIn;
    Interpreter interp { &index, ctx, blk, stateOut };
    interp.run(propagate, mergeReturn, mergePrivates);
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
      "{}function {}{} ({} block interps):\n{}",
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
    a.thisAvailable == b.thisAvailable &&
    a.locals == b.locals &&
    a.stack == b.stack &&
    a.fpiStack == b.fpiStack &&
    a.privateProperties == b.privateProperties &&
    a.privateStatics == b.privateStatics;
}

bool operator!=(const ActRec& a, const ActRec& b) { return !(a == b); }
bool operator!=(const State& a, const State& b)   { return !(a == b); }

//////////////////////////////////////////////////////////////////////

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

FuncAnalysis analyze_func(const Index& index, Context const ctx) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};
  return do_analyze(index, ctx, nullptr);
}

ClassAnalysis analyze_class(const Index& index, Context const ctx) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};
  assert(ctx.cls && !ctx.func);
  FTRACE(2, "{:#^70}\n", "Class");

  ClassAnalysis clsAnalysis(ctx);
  auto const associatedClosures = index.lookup_closures(ctx.cls);

  /*
   * Initialize inferred private property types to their in-class
   * initializers.
   *
   * We need to loosen_statics and loosen_values on instance
   * properties, because the class could be unserialized, which we
   * don't guarantee preserves those aspects of the type.
   */
  for (auto& prop : ctx.cls->properties) {
    if (!(prop.attrs & AttrPrivate)) continue;

    if (!(prop.attrs & AttrStatic)) {
      clsAnalysis.privateProperties[prop.name] =
        loosen_statics(loosen_values(from_cell(prop.val)));
    } else {
      clsAnalysis.privateStatics[prop.name] = from_cell(prop.val);
    }
  }

  /*
   * Skip trying to do smart things with 86{p,s}init for now.
   *
   * These are special functions that run to initialize static or
   * instance properties that depend on class constants, or have
   * collection literals.
   *
   * We don't handle this yet, so for any class with these types of
   * initializers put the properties up to TInitCell for now.
   *
   * TODO(#3567661, #3562690): we want to analyze these.
   */
  auto const specials = find_special_methods(ctx.cls);
  if (contains(specials, MethodMask::Internal_86pinit)) {
    for (auto& p : clsAnalysis.privateProperties) {
      p.second = union_of(p.second, TInitCell);
    }
  }
  if (contains(specials, MethodMask::Internal_86sinit)) {
    for (auto& p : clsAnalysis.privateStatics) {
      p.second = union_of(p.second, TInitCell);
    }
  }

  for (;;) {
    auto const previousProps   = clsAnalysis.privateProperties;
    auto const previousStatics = clsAnalysis.privateStatics;

    std::vector<FuncAnalysis> methodResults;
    std::vector<FuncAnalysis> closureResults;

    // Analyze every method in the class until we reach a fixed point
    // on the private property states.
    for (auto& f : ctx.cls->methods) {
      methodResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, borrow(f), ctx.cls },
          &clsAnalysis
        )
      );
    }

    for (auto& c : associatedClosures) {
      auto const invoke = borrow(c->methods[0]);
      closureResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, invoke, c },
          &clsAnalysis
        )
      );
    }

    // Check if we've reached a fixed point yet.
    if (previousProps   == clsAnalysis.privateProperties &&
        previousStatics == clsAnalysis.privateStatics) {
      clsAnalysis.methods  = std::move(methodResults);
      clsAnalysis.closures = std::move(closureResults);
      break;
    }
  }

  // For debugging, print the final state of the class analysis.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '+') + "\n";
    auto ret = folly::format(
      "{}class {}:\n{}",
      bsep,
      ctx.cls->name->data(),
      bsep
    ).str();
    for (auto& kv : clsAnalysis.privateProperties) {
      ret += folly::format(
        "private ${: <14} :: {}\n",
        kv.first->data(),
        show(kv.second)
      ).str();
    }
    for (auto& kv : clsAnalysis.privateStatics) {
      ret += folly::format(
        "private static ${: <14} :: {}\n",
        kv.first->data(),
        show(kv.second)
      ).str();
    }
    ret += bsep;
    return ret;
  }());

  return clsAnalysis;
}

void optimize_func(const Index& index, const FuncAnalysis& ainfo) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ainfo.ctx.unit)};
  do_optimize(index, ainfo);
}

void analyze_and_optimize_func(const Index& index, Context ctx) {
  optimize_func(index, analyze_func(index, ctx));
}

//////////////////////////////////////////////////////////////////////

}}
