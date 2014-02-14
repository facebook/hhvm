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
#include "hphp/hhbbc/dce.h"

#include <vector>
#include <string>
#include <utility>

#include <boost/dynamic_bitset.hpp>

#include "folly/gen/Base.h"
#include "folly/gen/String.h"

#include "hphp/util/trace.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_dce);

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

namespace {

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

}

std::vector<Bytecode> local_dce(const Index& index,
                                const Context ctx,
                                borrowed_ptr<php::Block> const blk,
                                const State& state) {
  Trace::Bump bumper{Trace::hhbbc_dce, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};

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

}}
