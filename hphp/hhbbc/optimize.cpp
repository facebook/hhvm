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
#include "hphp/hhbbc/optimize.h"

#include <vector>
#include <string>
#include <set>
#include <utility>
#include <algorithm>

#include <boost/dynamic_bitset.hpp>

#include "folly/Optional.h"
#include "folly/gen/Base.h"
#include "folly/gen/String.h"

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/complex-types.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

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
void insert_assertions_step(const php::Func& func,
                            const Bytecode& bcode,
                            const State& state,
                            std::bitset<64> mayReadLocalSet,
                            bool lastStackOutputObvious,
                            Gen gen) {
  for (size_t i = 0; i < state.locals.size(); ++i) {
    if (options.FilterAssertions) {
      if (i < mayReadLocalSet.size() && !mayReadLocalSet.test(i)) {
        continue;
      }
    }
    auto const realT = state.locals[i];
    auto const op = makeAssert<bc::AssertObjL,bc::AssertTL>(
      borrow(func.locals[i]), realT
    );
    if (op) gen(*op);
  }

  if (!options.InsertStackAssertions) return;

  // Skip asserting the top of the stack if it just came immediately
  // out of an 'obvious' instruction.  (See hasObviousStackOutput.)
  assert(state.stack.size() >= bcode.numPop());
  auto i = size_t{0};
  auto stackIdx = state.stack.size() - 1;
  if (lastStackOutputObvious) {
    ++i, --stackIdx;
  }

  /*
   * This doesn't need to account for ActRecs on the fpiStack, because
   * no instruction in an FPI region can ever consume a stack value
   * from above the pre-live ActRec.
   */
  for (; i < bcode.numPop(); ++i, --stackIdx) {
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

/*
 * When filter assertions is on, we use this to avoid putting stack
 * assertions on some "obvious" instructions.
 *
 * These are instructions that push an output type that is always the
 * same, i.e. where an AssertT is never going to add information.
 * (E.g. "Int 3" obviously pushes an Int, and the JIT has no trouble
 * figuring that out, so there's no reason to assert about it.)
 *
 * TODO(#3676101): It turns out many hhbc opcodes have known output
 * types---there are some super polymorphic ones, but many always do
 * bools or objects, etc.  We might consider making stack flavors have
 * subtypes and adding this to the opcode table.
 */
bool hasObviousStackOutput(Op op) {
  switch (op) {
  case Op::Box:
  case Op::BoxR:
  case Op::Null:
  case Op::NullUninit:
  case Op::True:
  case Op::False:
  case Op::Int:
  case Op::Double:
  case Op::String:
  case Op::Array:
  case Op::NewArray:
  case Op::NewArrayReserve:
  case Op::NewPackedArray:
  case Op::NewStructArray:
  case Op::AddElemC:
  case Op::AddElemV:
  case Op::AddNewElemC:
  case Op::AddNewElemV:
  case Op::NameA:
  case Op::File:
  case Op::Dir:
  case Op::Concat:
  case Op::Not:
  case Op::Xor:
  case Op::Same:
  case Op::NSame:
  case Op::Eq:
  case Op::Neq:
  case Op::Lt:
  case Op::Gt:
  case Op::Lte:
  case Op::Gte:
  case Op::Shl:
  case Op::Shr:
  case Op::CastBool:
  case Op::CastInt:
  case Op::CastDouble:
  case Op::CastString:
  case Op::CastArray:
  case Op::CastObject:
  case Op::InstanceOfD:
  case Op::InstanceOf:
  case Op::Print:
  case Op::Exit:
  case Op::AKExists:
  case Op::IssetL:
  case Op::IssetN:
  case Op::IssetG:
  case Op::IssetS:
  case Op::IssetM:
  case Op::EmptyL:
  case Op::EmptyN:
  case Op::EmptyG:
  case Op::EmptyS:
  case Op::EmptyM:
  case Op::IsTypeC:
  case Op::IsTypeL:
  case Op::ClassExists:
  case Op::InterfaceExists:
  case Op::TraitExists:
  case Op::Floor:
  case Op::Ceil:
    return true;
  default:
    return false;
  }
}

std::vector<Bytecode> insert_assertions(const Index& index,
                                        const Context ctx,
                                        borrowed_ptr<php::Block> const blk,
                                        State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  auto lastStackOutputObvious = false;

  PropertiesInfo props(index, ctx, nullptr);
  Interpreter interp { &index, ctx, props, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(newBCs.back()));

      lastStackOutputObvious =
        newb.numPush() != 0 && hasObviousStackOutput(newb.op);
    };

    auto const preState = state;
    auto const flags    = interp.step(op);

    insert_assertions_step(*ctx.func, op, preState, flags.mayReadLocalSet,
      lastStackOutputObvious, gen);

    gen(op);
  }

  return newBCs;
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
                                     borrowed_ptr<php::Block> const blk,
                                     State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  PropertiesInfo props(index, ctx, nullptr);
  Interpreter interp { &index, ctx, props, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(newBCs.back()));
    };

    auto const flags = interp.step(op);
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

    if (options.RemoveDeadBlocks && flags.tookBranch) {
      switch (op.op) {
      case Op::JmpNZ:  blk->fallthrough = op.JmpNZ.target; break;
      case Op::JmpZ:   blk->fallthrough = op.JmpZ.target;  break;
      default:
        // No support for switch, etc, right now.
        always_assert(0 && "unsupported tookBranch case");
      }
      /*
       * We need to pop the cell that was on the stack for the
       * conditional jump.  Note: this also conceptually needs to
       * execute any side effects a conversion to bool can have.
       * (Currently that is none.)  (TODO: could insert the bool conv
       * and let DCE remove it.)
       */
      gen(bc::PopC {});
      continue;
    }

    if (options.ConstantProp && flags.canConstProp) {
      if (propagate_constants(op, state, gen)) continue;
    }

    if (options.StrengthReduce && flags.strengthReduced) {
      for (auto& hh : *flags.strengthReduced) gen(hh);
      continue;
    }

    gen(op);
  }

  return newBCs;
}

//////////////////////////////////////////////////////////////////////

/*
 * Propagate a block input State to each instruction in the block.
 *
 * Returns a State array that is parallel to the instruction array in
 * blk->hhbcs.
 */
std::vector<State>
locally_propagated_states(const Index& index,
                          const Context ctx,
                          borrowed_ptr<const php::Block> blk,
                          State state) {
  Trace::Bump bumper{Trace::hhbbc, 10};

  std::vector<State> ret;
  ret.reserve(blk->hhbcs.size());

  PropertiesInfo props(index, ctx, nullptr);
  Interpreter interp { &index, ctx, props, blk, state };
  for (auto& op : blk->hhbcs) {
    ret.push_back(state);
    interp.step(op);
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

/*
 * Local DCE tries to eliminate unnecessary instructions within a
 * single block.  It only looks at instructions that affect eval stack
 * locations (to know whether accesses to locals are dead requires a
 * global view).  It also can't handle eval stack locations that are
 * live across a block boundary.
 *
 * This works in the following way:
 *
 *   - First, we forward-propagate the block entry state to each
 *     instruction, so we can conditionally decide instructions are
 *     unnecessary based on types.
 *
 *   - Then, we traverse the block backwards, maintaining a stack that
 *     indicates whether eval stack slots are going to be required or
 *     not.
 *
 *     During this traversal, each instruction that "pops" when going
 *     forward instead pushes information about whether that input is
 *     needed.  If it is not needed, it also pushes an accumulating
 *     set of instruction ids that must be removed if the instruction
 *     which produces the stack slot is removed.
 *
 *     Similarly, each instruction that would "push" a value going
 *     forward instead pops the information about whether its stack
 *     output is going to be needed.  If not, the instruction can mark
 *     itself (and all downstream instructions that depended on it) as
 *     removable.
 *
 *  - Finally, remove all the instructions that were marked dead.
 *
 * This doesn't support very many opcodes yet, but we can add more
 * later if we start producing more dead code.  (Currently not very
 * many bytecode changes produce any, and local DCE doesn't remove
 * very much.)
 */

enum class Use { Not, Used };
using InstrId    = size_t;
using InstrIdSet = std::set<InstrId>;
using UseInfo    = std::pair<Use,InstrIdSet>;

struct DceState {
  boost::dynamic_bitset<> marks;
  std::vector<UseInfo> stack;
};

const char* show(Use u) {
  switch (u) {
  case Use::Not:   return "0";
  case Use::Used:  return "U";
  }
  not_reached();
}

std::string show(const InstrIdSet& set) {
  using namespace folly::gen;
  return from(set)
    | eachTo<std::string>()
    | unsplit<std::string>(";")
    ;
}

struct DceVisitor : boost::static_visitor<void> {
  DceVisitor(DceState& dceState, InstrId id, const State& stateBefore)
    : m_dceState(dceState)
    , m_id(id)
    , m_stateBefore(stateBefore)
  {}

  void operator()(const bc::PopC&)       { discardNonDtors(); }
  // For PopV and PopR currently we never know if can't run a
  // destructor.
  void operator()(const bc::PopA&)       { discard(); }
  void operator()(const bc::Int&)        { pushRemovable(); }
  void operator()(const bc::String&)     { pushRemovable(); }
  void operator()(const bc::Array&)      { pushRemovable(); }
  void operator()(const bc::Double&)     { pushRemovable(); }
  void operator()(const bc::True&)       { pushRemovable(); }
  void operator()(const bc::False&)      { pushRemovable(); }
  void operator()(const bc::Null&)       { pushRemovable(); }
  void operator()(const bc::NullUninit&) { pushRemovable(); }
  void operator()(const bc::File&)       { pushRemovable(); }
  void operator()(const bc::Dir&)        { pushRemovable(); }
  void operator()(const bc::NameA&)      { pushRemovable(); }
  void operator()(const bc::CreateCl&)   { pushRemovable(); }

  void operator()(const bc::NewArray&)        { pushRemovable(); }
  void operator()(const bc::NewArrayReserve&) { pushRemovable(); }
  void operator()(const bc::NewStructArray&)  { pushRemovable(); }
  void operator()(const bc::NewCol&)          { pushRemovable(); }

  void operator()(const bc::Dup&) {
    auto const u1 = push();
    auto const u2 = push();
    popCond(u1, u2);
  }

  void operator()(const bc::CGetL&) {
    auto const u = push();
    popCond(u);
  }

  void operator()(const bc::CGetL2&) {
    auto const u1 = push();
    auto const u2 = push();
    popCond(u1, u2);
  }

  void operator()(const bc::CGetL3&) {
    auto const u1 = push();
    auto const u2 = push();
    auto const u3 = push();
    popCond(u1, u2, u3);
  }

  // Default implementation is conservative: assume we use all of our
  // inputs, and can't be removed even if our output is unused.
  template<class Op>
  void operator()(const Op& op) {
    for (auto i = uint32_t{0}; i < op.numPush(); ++i) {
      push();
    }
    for (auto i = uint32_t{0}; i < op.numPop(); ++i) {
      pop(Use::Used, InstrIdSet{});
    }
  }

private:
  void pop(Use u, InstrIdSet set) {
    FTRACE(2, "      pop({})\n", show(u));
    m_dceState.stack.emplace_back(u, std::move(set));
  }

  void discard() {
    pop(Use::Not, InstrIdSet{m_id});
  }

  bool allUnused() { return true; }
  template<class... Args>
  bool allUnused(const UseInfo& ui, Args&&... args) {
    return ui.first == Use::Not &&
      allUnused(std::forward<Args>(args)...);
  }

  void combineSets(InstrIdSet&) {}
  template<class... Args>
  void combineSets(InstrIdSet& accum, const UseInfo& ui, Args&&... args) {
    accum.insert(begin(ui.second), end(ui.second));
    combineSets(accum, std::forward<Args>(args)...);
  }

  // If all the supplied UseInfos represent unused stack slots, make a
  // pop that is considered unused.  Otherwise pop as a Use::Used.
  template<class... Args>
  void popCond(Args&&... args) {
    bool unused = allUnused(std::forward<Args>(args)...);
    if (!unused) return pop(Use::Used, InstrIdSet{});
    auto accum = InstrIdSet{m_id};
    combineSets(accum, std::forward<Args>(args)...);
    pop(Use::Not, accum);
  }

  /*
   * It may be ok to remove pops on objects with destructors in some
   * scenarios (where it won't change the observable point at which a
   * destructor runs).  We could also look at the object type and see
   * if it is known that it can't have a user-defined destructor.
   *
   * For now we're not trying though, since at the time this was
   * tested there were only two additional "dead" PopC's in all of www
   * if you remove the couldBe checks below.
   */
  void discardNonDtors() {
    auto const t = topC();
    if (t.couldBe(TObj) || t.couldBe(TArr)) {
      return pop(Use::Used, InstrIdSet{});
    }
    discard();
  }

  UseInfo push() {
    // If the stack is empty, it means the value is consumed in a
    // different block (so we didn't see the instruction that popped
    // it).  For local DCE, we have to assume it is used.
    if (m_dceState.stack.empty()) {
      FTRACE(2, "      <non-local> = push()\n");
      return { Use::Used, InstrIdSet{} };
    }
    auto ret = m_dceState.stack.back();
    m_dceState.stack.pop_back();
    FTRACE(2, "      {}@{} = push()\n", show(ret.first), show(ret.second));
    return ret;
  }

  void pushRemovable() {
    auto const ui = push();
    switch (ui.first) {
    case Use::Not:
      markWithSet(ui.second);
      break;
    case Use::Used:
      break;
    }
  }

  Type topT(uint32_t idx = 0) {
    assert(idx < m_stateBefore.stack.size());
    return m_stateBefore.stack[m_stateBefore.stack.size() - idx - 1];
  }

  Type topC(uint32_t idx = 0) {
    auto const t = topT(idx);
    assert(t.subtypeOf(TInitCell));
    return t;
  }

private:
  void markWithSet(const InstrIdSet& set) {
    m_dceState.marks[m_id] = 1;
    FTRACE(2, "    marking {} {}\n", m_id, show(set));
    for (auto& i : set) {
      m_dceState.marks[i] = 1;
    }
  }

private:
  DceState& m_dceState;
  InstrId m_id;
  const State& m_stateBefore;
};

std::vector<Bytecode> local_dce(const Index& index,
                                const Context ctx,
                                borrowed_ptr<php::Block> const blk,
                                const State& state) {
  auto const states = locally_propagated_states(index, ctx, blk, state);

  DceState dceState;
  dceState.marks.resize(blk->hhbcs.size());

  for (auto idx = blk->hhbcs.size(); idx-- > 0;) {
    auto const& op = blk->hhbcs[idx];

    FTRACE(2, "  == #{} {}\n", idx, show(op));
    FTRACE(4, "    dce stack: {}\n",
      [&]() -> std::string {
        auto ret = std::string{};
        for (auto& u : dceState.stack) {
          ret += folly::format("{}@{} ", show(u.first), show(u.second)).str();
        }
        return ret;
      }()
    );
    FTRACE(4, "    interp stack: {}\n",
      [&]() -> std::string {
        auto ret = std::string{};
        for (auto& t : states[idx].stack) ret += show(t) + " ";
        return ret;
      }()
    );

    auto visitor = DceVisitor { dceState, idx, states[idx] };
    visit(op, visitor);
  }

  // Remove all instructions that were marked.
  for (auto idx = blk->hhbcs.size(); idx-- > 0;) {
    if (!dceState.marks.test(idx)) continue;
    blk->hhbcs.erase(begin(blk->hhbcs) + idx);
  }

  return blk->hhbcs;
}

//////////////////////////////////////////////////////////////////////

template<class Fun>
void visit_blocks(const char* what,
                  const Index& index,
                  const FuncAnalysis& ainfo,
                  Fun fun) {
  FTRACE(1, "|---- {}\n", what);
  for (auto& blk : ainfo.rpoBlocks) {
    FTRACE(2, "block #{}\n", blk->id);
    auto const& state = ainfo.bdata[blk->id].stateIn;
    if (!state.initialized) {
      FTRACE(2, "   unreachable\n");
      continue;
    }
    // TODO(#3732260): this should probably spend an extra interp pass
    // in debug builds to check that no transformation to the bytecode
    // was made that changes the block output state.
    blk->hhbcs = fun(index, ainfo.ctx, blk, state);
  }
}

void do_optimize(const Index& index, const FuncAnalysis& ainfo) {
  FTRACE(2, "{:-^70}\n", "Optimize Func");

  visit_blocks("first pass", index, ainfo, optimize_block);
  if (options.LocalDCE) {
    visit_blocks("local DCE", index, ainfo, local_dce);
  }
  if (options.InsertAssertions) {
    visit_blocks("insert assertions", index, ainfo, insert_assertions);
  }

  // If we didn't remove jumps to dead blocks, we replace all
  // supposedly unreachable blocks with fatal instructions.
  if (!options.RemoveDeadBlocks) {
    for (auto& blk : ainfo.rpoBlocks) {
      auto const& state = ainfo.bdata[blk->id].stateIn;
      if (state.initialized) continue;
      auto const srcLoc = blk->hhbcs.front().srcLoc;
      blk->hhbcs = {
        bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
        bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
      };
      blk->fallthrough = nullptr;
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void optimize_func(const Index& index, const FuncAnalysis& ainfo) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ainfo.ctx.unit)};
  do_optimize(index, ainfo);
}

//////////////////////////////////////////////////////////////////////

}}
