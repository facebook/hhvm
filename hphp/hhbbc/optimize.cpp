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
#include "hphp/hhbbc/optimize.h"

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cassert>
#include <bitset>

#include <boost/dynamic_bitset.hpp>

#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/base/datatype.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/dce.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

VisitContext::VisitContext(const Index& index, const FuncAnalysis& ainfo,
                           CollectedInfo& collect, php::WideFunc& func)
    : index(index), ainfo(ainfo), collect(collect), func(func) {
  assertx(ainfo.ctx.func == func);
}

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * For filtering assertions, some opcodes are considered to have no
 * use for a stack input assertion.
 *
 * For now this is restricted to opcodes that do literally nothing.
 */
bool ignoresStackInput(Op op) {
  switch (op) {
  case Op::UGetCUNop:
  case Op::CGetCUNop:
  case Op::PopU:
    return true;
  default:
    return false;
  }
  not_reached();
}

/*
 * Some ops are marked as "reading" a local, but we don't actually
 * want to emit an AssertRATL for them.
 */
bool ignoresReadLocal(const Bytecode& bcode, LocalId l) {
  auto const key = [&] (auto const& o) {
    switch (o.mkey.mcode) {
      case MEL:
      case MPL:
        return o.mkey.local.id != l;
      default:
        return true;
    }
  };

  switch (bcode.op) {
    // These might "read" a local because they can update a local base
    // type, but they don't actual read the local.
    case Op::Dim:       return key(bcode.Dim);
    case Op::SetM:      return key(bcode.SetM);
    case Op::IncDecM:   return key(bcode.IncDecM);
    case Op::SetOpM:    return key(bcode.SetOpM);
    case Op::UnsetM:    return key(bcode.UnsetM);
    case Op::SetRangeM: return true;
    default:            return false;
  }
}

template<typename TyBC, typename ArgType>
Optional<Bytecode> makeAssert(ArgType arg, Type t) {
  if (t.subtypeOf(BBottom)) return std::nullopt;
  auto const rat = make_repo_type(t);
  using T = RepoAuthType::Tag;
  // Cell and InitCell don't add any useful information, so leave them
  // out entirely.
  if (rat == RepoAuthType{T::Cell})     return std::nullopt;
  if (rat == RepoAuthType{T::InitCell}) return std::nullopt;
  return Bytecode { TyBC { arg, rat } };
}

template<class Gen>
void insert_assertions_step(const php::Func& func,
                            const Bytecode& bcode,
                            const State& state,
                            std::bitset<kMaxTrackedLocals> mayReadLocalSet,
                            std::vector<uint8_t> obviousStackOutputs,
                            Gen gen) {
  if (state.unreachable) return;

  for (LocalId i = 0; i < state.locals.size(); ++i) {
    if (func.locals[i].killed) continue;
    // Do not emit assertions for untracked locals.
    if (i >= mayReadLocalSet.size()) break;
    if (!mayReadLocalSet.test(i)) continue;
    if (ignoresReadLocal(bcode, i)) continue;
    auto const realT = state.locals[i];
    auto const op = makeAssert<bc::AssertRATL>(i, realT);
    if (op) gen(*op);
  }

  assertx(obviousStackOutputs.size() == state.stack.size());

  auto const assert_stack = [&] (size_t idx) {
    assertx(idx < state.stack.size());
    if (obviousStackOutputs[state.stack.size() - idx - 1]) return;
    if (ignoresStackInput(bcode.op)) return;
    auto const realT = state.stack[state.stack.size() - idx - 1].type;
    auto const flav  = stack_flav(realT);

    if (!realT.strictSubtypeOf(flav)) return;

    auto const op =
      makeAssert<bc::AssertRATStk>(
        static_cast<uint32_t>(idx),
        realT
      );
    if (op) gen(*op);
  };

  for (auto i = size_t{0}; i < bcode.numPop(); ++i) assert_stack(i);

  // The base instructions are special in that they may read from the
  // stack without necessarily popping it. We want type assertions on
  // the stack slots they'll read.
  switch (bcode.op) {
    case Op::BaseC:       assert_stack(bcode.BaseC.arg1);       break;
    case Op::BaseGC:      assert_stack(bcode.BaseGC.arg1);      break;
    case Op::BaseSC:
      assert_stack(bcode.BaseSC.arg1);
      assert_stack(bcode.BaseSC.arg2);
      break;
    case Op::Dim: {
      switch (bcode.Dim.mkey.mcode) {
        case MEC: case MPC:
          assert_stack(bcode.Dim.mkey.idx);
          break;
        case MW:  case MEL: case MPL: case MEI:
        case MET: case MPT: case MQT:
          break;
      }
    }
    default:                                                    break;
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
bool hasObviousStackOutput(const Bytecode& op, const Interp& interp) {
  switch (op.op) {
  case Op::Null:
  case Op::NullUninit:
  case Op::True:
  case Op::False:
  case Op::Int:
  case Op::Double:
  case Op::String:
  case Op::LazyClass:
  case Op::Dict:
  case Op::Vec:
  case Op::Keyset:
  case Op::NewDictArray:
  case Op::NewStructDict:
  case Op::NewVec:
  case Op::NewKeysetArray:
  case Op::AddNewElemC:
  case Op::NewCol:
  case Op::NewPair:
  case Op::ClassName:
  case Op::LazyClassFromClass:
  case Op::File:
  case Op::Dir:
  case Op::Concat:
  case Op::ConcatN:
  case Op::Not:
  case Op::Same:
  case Op::NSame:
  case Op::Eq:
  case Op::Neq:
  case Op::Lt:
  case Op::Gt:
  case Op::Lte:
  case Op::Gte:
  case Op::Cmp:
  case Op::Shl:
  case Op::Shr:
  case Op::CastBool:
  case Op::CastInt:
  case Op::CastDouble:
  case Op::CastString:
  case Op::CastDict:
  case Op::CastVec:
  case Op::CastKeyset:
  case Op::DblAsBits:
  case Op::InstanceOfD:
  case Op::IsLateBoundCls:
  case Op::IsTypeStructC:
  case Op::CombineAndResolveTypeStruct:
  case Op::RecordReifiedGeneric:
  case Op::ClassHasReifiedGenerics:
  case Op::HasReifiedParent:
  case Op::InstanceOf:
  case Op::Print:
  case Op::Exit:
  case Op::AKExists:
  case Op::IssetL:
  case Op::IsUnsetL:
  case Op::IssetG:
  case Op::IssetS:
  case Op::IsTypeC:
  case Op::IsTypeL:
  case Op::OODeclExists:
  case Op::CreateCl:
    return true;

  case Op::This:
  case Op::BareThis:
    if (auto const s = selfCls(interp.index, interp.ctx)) {
      auto t = interp.state.stack.back().type;
      if (t.couldBe(BInitNull) && !t.subtypeOf(BInitNull)) {
        t = unopt(std::move(t));
      }
      return !t.strictSubtypeOf(setctx(toobj(*s)));
    }
    return true;

  case Op::CGetL:
  case Op::CGetQuietL:
  case Op::CUGetL:
  case Op::CGetL2:
  case Op::PushL:
    return true;

  // The output of SetL is obvious if you know what its input is
  // (which we'll assert if we know).
  case Op::SetL:
    return true;

  // The output of SetM isn't quite as obvious as SetL, but the jit
  // can work it out from the input just as well as hhbbc (if not better).
  case Op::SetM:
    return true;

  default:
    return false;
  }
}

void insert_assertions(VisitContext& visit, BlockId bid, State state) {
  BytecodeVec newBCs;
  auto& func = visit.func;
  auto const& cblk = func.blocks()[bid];
  newBCs.reserve(cblk->hhbcs.size());

  auto const& index = visit.index;
  auto const& ainfo = visit.ainfo;
  auto const ctx = AnalysisContext { ainfo.ctx.unit, func, ainfo.ctx.cls };

  std::vector<uint8_t> obviousStackOutputs(state.stack.size(), false);

  auto fallthrough = cblk->fallthrough;
  auto interp = Interp { index, ctx, visit.collect, bid, cblk.get(), state };

  for (auto& op : cblk->hhbcs) {
    FTRACE(2, "  == {}\n", show(*func, op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(*func, newBCs.back()));
    };

    if (state.unreachable) {
      fallthrough = NoBlockId;
      if (!(instrFlags(op.op) & TF)) {
        gen(bc::BreakTraceHint {});
        gen(bc::String { s_unreachable.get() });
        gen(bc::Fatal { FatalOp::Runtime });
      }
      break;
    }

    auto const preState = state;
    auto const flags    = step(interp, op);

    insert_assertions_step(
      *func,
      op,
      preState,
      flags.mayReadLocalSet,
      obviousStackOutputs,
      gen
    );

    if (op.op == Op::CGetL2) {
      obviousStackOutputs.emplace(obviousStackOutputs.end() - 1,
                                  hasObviousStackOutput(op, interp));
    } else {
      for (int i = 0; i < op.numPop(); i++) {
        obviousStackOutputs.pop_back();
      }
      for (auto i = op.numPush(); i--; ) {
        obviousStackOutputs.emplace_back(hasObviousStackOutput(op, interp));
      }
    }

    gen(op);
  }

  if (cblk->fallthrough != fallthrough || cblk->hhbcs != newBCs) {
    auto const blk = func.blocks()[bid].mutate();
    blk->fallthrough = fallthrough;
    blk->hhbcs = std::move(newBCs);
  }
}

//////////////////////////////////////////////////////////////////////

// Create a new fatal error block. Update the given FuncAnalysis if
// it is non-null - specifically, assign the new block an rpoId.
BlockId make_fatal_block(php::WideFunc& func, const php::Block* srcBlk,
                         FuncAnalysis* ainfo) {
  FTRACE(1, " ++ new block {}\n", func.blocks().size());
  auto bid = make_block(func, srcBlk);
  auto const blk = func.blocks()[bid].mutate();
  auto const srcLoc = srcBlk->hhbcs.back().srcLoc;
  blk->hhbcs = {
    bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
    bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
  };
  blk->fallthrough = NoBlockId;
  blk->throwExit = NoBlockId;
  blk->exnNodeId = NoExnNodeId;

  if (ainfo) {
    assertx(ainfo->bdata.size() == bid);
    assertx(bid + 1 == func.blocks().size());
    auto const rpoId = safe_cast<uint32_t>(ainfo->rpoBlocks.size());
    ainfo->rpoBlocks.push_back(bid);
    ainfo->bdata.push_back(FuncAnalysis::BlockData { rpoId, State {} });
  }

  return bid;
}

//////////////////////////////////////////////////////////////////////

template<class Fun>
void visit_blocks(const char* what, VisitContext& visit, Fun&& fun) {
  BlockId curBlk = NoBlockId;
  SCOPE_ASSERT_DETAIL(what) {
    if (curBlk == NoBlockId) return std::string{"\nNo block processed\n"};
    auto const& state = visit.ainfo.bdata[curBlk].stateIn;
    auto const debug = state_string(*visit.func, state, visit.collect);
    return folly::sformat("block #{}\nin-{}", curBlk, debug);
  };

  FTRACE(1, "|---- {}\n", what);
  for (auto const bid : visit.ainfo.rpoBlocks) {
    curBlk = bid;
    FTRACE(2, "block #{}\n", bid);
    auto const& state = visit.ainfo.bdata[bid].stateIn;
    if (!state.initialized) {
      FTRACE(2, "   unreachable\n");
      continue;
    }
    // TODO(#3732260): We should probably do an extra interp pass here
    // in debug builds to check that no transformation to the bytecode
    // was made that changes the block output state.
    fun(visit, bid, state);
  }
  assertx(check(*visit.func));
}

//////////////////////////////////////////////////////////////////////

IterId iterFromInit(const php::WideFunc& func, BlockId initBlock) {
  auto const& op = func.blocks()[initBlock]->hhbcs.back();
  if (op.op == Op::IterInit)  return op.IterInit.ita.iterId;
  if (op.op == Op::LIterInit) return op.LIterInit.ita.iterId;
  always_assert(false);
}

/*
 * Attempt to convert normal iterators into liters. In order for an iterator to
 * be converted to a liter, the following needs to be true:
 *
 * - The iterator is initialized with the value in a local at exactly one block.
 *
 * - That same local is not modified on all possible paths from the
 *   initialization to every usage of that iterator.
 *
 * The first condition is actually more restrictive than necessary, but
 * enforcing that the iterator is initialized at exactly one place simplifies
 * the bookkeeping and is always true with how we currently emit bytecode.
 */

struct OptimizeIterState {
  void operator()(VisitContext& visit, BlockId bid, State state) {
    auto& func = visit.func;
    auto const& ainfo = visit.ainfo;
    auto const blk = func.blocks()[bid].get();
    auto const ctx = AnalysisContext { ainfo.ctx.unit, func, ainfo.ctx.cls };
    auto interp = Interp { visit.index, ctx, visit.collect, bid, blk, state };
    for (uint32_t opIdx = 0; opIdx < blk->hhbcs.size(); ++opIdx) {
      // If we've already determined that nothing is eligible, we can just stop.
      if (!eligible.any()) break;

      auto const& op = blk->hhbcs[opIdx];
      FTRACE(2, "  == {}\n", show(*func, op));

      if (state.unreachable) break;

      // At every op, we check the known state of all live iterators and mark it
      // as ineligible as necessary.
      for (IterId it = 0; it < state.iters.size(); ++it) {
        match<void>(
          state.iters[it],
          []  (DeadIter) {},
          [&] (const LiveIter& ti) {
            FTRACE(4, "   iter {: <2}  :: {}\n",
                   it, show(*func, state.iters[it]));
            // The init block is unknown. This can only happen if there's more
            // than one block where this iterator was initialized. This makes
            // tracking the iteration loop ambiguous, and can't happen with how
            // we currently emit bytecode, so just pessimize everything.
            if (ti.initBlock == NoBlockId) {
              FTRACE(2, "   - pessimize all\n");
              eligible.clear();
              return;
            }
            // Otherwise, if the iterator doesn't have an equivalent local,
            // either it was never initialized with a local to begin with, or
            // that local got changed within the loop. Either way, this
            // iteration loop isn't eligible.
            if (eligible[ti.initBlock] && ti.baseLocal == NoLocalId) {
              FTRACE(2, "   - blk:{} ineligible\n", ti.initBlock);
              eligible[ti.initBlock] = false;
            } else if (ti.baseUpdated) {
              FTRACE(2, "   - blk:{} updated\n", ti.initBlock);
              updated[ti.initBlock] = true;
            }
          }
        );
      }

      auto const fixupForInit = [&] {
        auto const base = topStkLocal(state);
        if (base == NoLocalId && eligible[bid]) {
          FTRACE(2, "   - blk:{} ineligible\n", bid);
          eligible[bid] = false;
        }
        fixups.emplace_back(Fixup{bid, opIdx, bid, base});
        FTRACE(2, "   + fixup ({})\n", fixups.back().show(*func));
      };

      auto const fixupFromState = [&] (IterId it) {
        match<void>(
          state.iters[it],
          []  (DeadIter) {},
          [&] (const LiveIter& ti) {
            if (ti.initBlock != NoBlockId) {
              assertx(iterFromInit(func, ti.initBlock) == it);
              fixups.emplace_back(
                Fixup{bid, opIdx, ti.initBlock, ti.baseLocal}
              );
              FTRACE(2, "   + fixup ({})\n", fixups.back().show(*func));
            }
          }
        );
      };

      // Record a fixup for this iteration op. This iteration loop may not be
      // ultimately eligible, but we'll check that before actually doing the
      // transformation.
      switch (op.op) {
        case Op::IterInit:
          assertx(opIdx == blk->hhbcs.size() - 1);
          fixupForInit();
          break;
        case Op::IterNext:
          fixupFromState(op.IterNext.ita.iterId);
          break;
        case Op::IterFree:
          fixupFromState(op.IterFree.iter1);
          break;
        default:
          break;
      }

      step(interp, op);
    }
  }

  // We identify iteration loops by the block of the initialization op (which we
  // enforce is exactly one block). A fixup describes a transformation to an
  // iteration instruction which must be applied only if its associated loop is
  // eligible.
  struct Fixup {
    BlockId block; // Block of the op
    uint32_t op;   // Index into the block of the op
    BlockId init;  // Block of the loop's initializer
    LocalId base;  // Invariant base of the iterator

    std::string show(const php::Func& f) const {
      return folly::sformat(
        "blk:{},{},blk:{},{}",
        block, op, init,
        base != NoLocalId ? local_string(f, base) : "-"
      );
    }
  };
  std::vector<Fixup> fixups;
  // All of the associated iterator operations within an iterator loop can be
  // optimized to liter if the iterator's initialization block is eligible.
  boost::dynamic_bitset<> eligible;
  // For eligible blocks, the "updated" flag tracks whether there was *any*
  // change to the base initialized in that block (including "safe" changes).
  boost::dynamic_bitset<> updated;
};

void optimize_iterators(VisitContext& visit) {
  // Quick exit. If there's no iterators, or if no associated local survives
  // to the end of the iterator, there's nothing to do.
  auto& func = visit.func;
  auto const& ainfo = visit.ainfo;
  if (!func->numIters || !ainfo.hasInvariantIterBase) return;

  OptimizeIterState state;
  // All blocks starts out eligible. We'll remove initialization blocks as go.
  // Similarly, the iterator bases for all blocks start out not being updated.
  state.eligible.resize(func.blocks().size(), true);
  state.updated.resize(func.blocks().size(), false);

  // Visit all the blocks and build up the fixup state.
  visit_blocks("optimize_iterators", visit, state);
  if (!state.eligible.any()) return;

  FTRACE(2, "Rewrites:\n");
  for (auto const& fixup : state.fixups) {
    auto const& cblk = func.blocks()[fixup.block];
    auto const& op = cblk->hhbcs[fixup.op];

    if (!state.eligible[fixup.init]) {
      // This iteration loop isn't eligible, so don't apply the fixup
      FTRACE(2, "   * ({}): {}\n", fixup.show(*func), show(*func, op));
      continue;
    }

    auto const flags = state.updated[fixup.init]
      ? IterArgs::Flags::None
      : IterArgs::Flags::BaseConst;

    BytecodeVec newOps;
    assertx(fixup.base != NoLocalId);

    // Rewrite the iteration op to its liter equivalent:
    switch (op.op) {
      case Op::IterInit: {
        auto args = op.IterInit.ita;
        auto const target = op.IterInit.target2;
        args.flags = flags;
        newOps = {
          bc_with_loc(op.srcLoc, bc::PopC {}),
          bc_with_loc(op.srcLoc, bc::LIterInit{args, fixup.base, target})
        };
        break;
      }
      case Op::IterNext: {
        auto args = op.IterNext.ita;
        auto const target = op.IterNext.target2;
        args.flags = flags;
        newOps = {
          bc_with_loc(op.srcLoc, bc::LIterNext{args, fixup.base, target}),
        };
        break;
      }
      case Op::IterFree:
        newOps = {
          bc_with_loc(
            op.srcLoc,
            bc::LIterFree { op.IterFree.iter1, fixup.base }
          )
        };
        break;
      default:
        always_assert(false);
    }

    FTRACE(
      2, "   ({}): {} ==> {}\n",
      fixup.show(*func), show(*func, op),
      [&] {
        using namespace folly::gen;
        return from(newOps)
          | map([&] (const Bytecode& bc) { return show(*func, bc); })
          | unsplit<std::string>(",");
      }()
    );

    auto const blk = func.blocks()[fixup.block].mutate();
    blk->hhbcs.erase(blk->hhbcs.begin() + fixup.op);
    blk->hhbcs.insert(blk->hhbcs.begin() + fixup.op,
                      newOps.begin(), newOps.end());
  }

  FTRACE(10, "{}", show(*func));
}

//////////////////////////////////////////////////////////////////////

void do_optimize(const Index& index, FuncAnalysis&& ainfo,
                 php::WideFunc& func) {
  FTRACE(2, "{:-^70} {}\n", "Optimize Func", func->name);

  bool again;
  Optional<CollectedInfo> collect;
  Optional<VisitContext> visit;
  collect.emplace(index, ainfo.ctx, nullptr, CollectionOpts{}, nullptr, &ainfo);
  visit.emplace(index, ainfo, *collect, func);

  update_bytecode(func, std::move(ainfo.blockUpdates), &ainfo);
  optimize_iterators(*visit);

  do {
    again = false;
    FTRACE(10, "{}", show(*func));
    /*
     * Note: it's useful to do dead block removal before DCE, so it can remove
     * code relating to the branch to the dead block.
     */
    remove_unreachable_blocks(ainfo, func);

    visit_blocks("local DCE", *visit, local_dce);
    split_critical_edges(index, ainfo, func);
    if (global_dce(index, ainfo, func)) again = true;
    if (control_flow_opts(ainfo, func)) again = true;
    assertx(check(*func));
    /*
     * Global DCE can change types of locals across blocks.  See
     * dce.cpp for an explanation.
     *
     * We need to perform a final type analysis before we do
     * anything else.
     */
    auto const ctx = AnalysisContext { ainfo.ctx.unit, func, ainfo.ctx.cls };
    ainfo = analyze_func(index, ctx, CollectionOpts{});
    update_bytecode(func, std::move(ainfo.blockUpdates), &ainfo);
    collect.emplace(
      index, ainfo.ctx, nullptr, CollectionOpts{}, nullptr, &ainfo
    );
    visit.emplace(index, ainfo, *collect, func);

    // If we merged blocks, there could be new optimization opportunities
  } while (again);

  if (func->name == s_86pinit.get() ||
      func->name == s_86sinit.get() ||
      func->name == s_86linit.get()) {
    auto const& blk = *func.blocks()[func->mainEntry];
    if (blk.hhbcs.size() == 2 &&
        blk.hhbcs[0].op == Op::Null &&
        blk.hhbcs[1].op == Op::RetC) {
      FTRACE(2, "Erasing {}::{}\n", func->cls->name, func->name);
      auto const it = std::find_if(
        func->cls->methods.begin(),
        func->cls->methods.end(),
        [&](const std::unique_ptr<php::Func>& f) {
          return f.get() == func;
        }
      );
      assertx(it != func->cls->methods.end());
      // Don't actually remove it from the methods table, as that
      // would invalidate any indices in the table. Just null out the
      // entry.
      func.release();
      it->reset();
      return;
    }
  }

  visit_blocks("insert assertions", *visit, insert_assertions);

  // NOTE: We shouldn't duplicate blocks that are shared between two Funcs
  // in this loop. We shrink BytecodeVec at the time we parse the function,
  // so we only shrink when we've already mutated (and COWed) the bytecode.
  for (auto bid : func.blockRange()) {
    auto const& block = func.blocks()[bid];
    assertx(block->hhbcs.size());
    if (block->hhbcs.capacity() == block->hhbcs.size()) continue;
    func.blocks()[bid].mutate()->hhbcs.shrink_to_fit();
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Bytecode gen_constant(const TypedValue& cell) {
  switch (cell.m_type) {
    case KindOfUninit:
      return bc::NullUninit {};
    case KindOfNull:
      return bc::Null {};
    case KindOfBoolean:
      if (cell.m_data.num) {
        return bc::True {};
      } else {
        return bc::False {};
      }
    case KindOfInt64:
      return bc::Int { cell.m_data.num };
    case KindOfDouble:
      return bc::Double { cell.m_data.dbl };
    case KindOfString:
      assertx(cell.m_data.pstr->isStatic());
    case KindOfPersistentString:
      return bc::String { cell.m_data.pstr };
    case KindOfVec:
      assertx(cell.m_data.parr->isStatic());
    case KindOfPersistentVec:
      assertx(cell.m_data.parr->isVecType());
      return bc::Vec { cell.m_data.parr };
    case KindOfDict:
      assertx(cell.m_data.parr->isStatic());
    case KindOfPersistentDict:
      assertx(cell.m_data.parr->isDictType());
      return bc::Dict { cell.m_data.parr };
    case KindOfKeyset:
      assertx(cell.m_data.parr->isStatic());
    case KindOfPersistentKeyset:
      assertx(cell.m_data.parr->isKeysetType());
      return bc::Keyset { cell.m_data.parr };
    case KindOfLazyClass:
      return bc::LazyClass { cell.m_data.plazyclass.name() };

    case KindOfResource:
    case KindOfObject:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfEnumClassLabel: //TODO(T162042839): Implement this
      always_assert(0 && "invalid constant in gen_constant");
  }
  not_reached();
}

void optimize_func(const Index& index, FuncAnalysis&& ainfo,
                   php::WideFunc& func) {
  auto const bump = trace_bump_for(ainfo.ctx.cls, func);

  SCOPE_ASSERT_DETAIL("optimize_func") {
    return "Optimizing:" + show(ainfo.ctx);
  };

  Trace::Bump bumper1{Trace::hhbbc, bump};
  Trace::Bump bumper2{Trace::hhbbc_cfg, bump};
  Trace::Bump bumper3{Trace::hhbbc_dce, bump};
  Trace::Bump bumper4{Trace::hhbbc_index, bump};
  do_optimize(index, std::move(ainfo), func);
}

void update_bytecode(php::WideFunc& func, BlockUpdates&& blockUpdates,
                     FuncAnalysis* ainfo) {
  for (auto& compressed : blockUpdates) {
    std::pair<BlockId, BlockUpdateInfo> ent;
    ent.first = compressed.first;
    compressed.second.expand(ent.second);

    auto blk = func.blocks()[ent.first].mutate();
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    if (!ent.second.unchangedBcs) {
      if (ent.second.replacedBcs.size()) {
        blk->hhbcs = std::move(ent.second.replacedBcs);
      } else {
        blk->hhbcs = { bc_with_loc(blk->hhbcs.front().srcLoc, bc::Nop {}) };
      }
    } else {
      blk->hhbcs.erase(blk->hhbcs.begin() + ent.second.unchangedBcs,
                       blk->hhbcs.end());
      blk->hhbcs.reserve(blk->hhbcs.size() + ent.second.replacedBcs.size());
      for (auto& bc : ent.second.replacedBcs) {
        blk->hhbcs.push_back(std::move(bc));
      }
    }
    if (blk->hhbcs.empty()) {
      blk->hhbcs.push_back(bc_with_loc(srcLoc, bc::Nop {}));
    }
    auto fatal_block = NoBlockId;
    auto fatal = [&] {
      if (fatal_block == NoBlockId) {
        fatal_block = make_fatal_block(func, blk, ainfo);
      }
      return fatal_block;
    };
    blk->fallthrough = ent.second.fallthrough;
    auto hasCf = false;
    forEachTakenEdge(blk->hhbcs.back(),
                     [&] (BlockId& bid) {
                       hasCf = true;
                       if (bid == NoBlockId) bid = fatal();
                     });
    if (blk->fallthrough == NoBlockId &&
        !(instrFlags(blk->hhbcs.back().op) & TF)) {
      if (hasCf) {
        blk->fallthrough = fatal();
      } else {
        blk->hhbcs.push_back(bc::BreakTraceHint {});
        blk->hhbcs.push_back(bc::String { s_unreachable.get() });
        blk->hhbcs.push_back(bc::Fatal { FatalOp::Runtime });
      }
    }
  }
  blockUpdates.clear();
}

//////////////////////////////////////////////////////////////////////

}
