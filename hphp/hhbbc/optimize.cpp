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
#include "hphp/hhbbc/optimize.h"

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cassert>
#include <bitset>

#include <folly/Optional.h>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/base/datatype.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/dce.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/peephole.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

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
  case Op::UnboxRNop:
  case Op::BoxRNop:
  case Op::FPassVNop:
  case Op::FPassC:
    return true;
  default:
    return false;
  }
  not_reached();
}

template<class TyBC, class ArgType>
folly::Optional<Bytecode> makeAssert(ArrayTypeTable::Builder& arrTable,
                                     ArgType arg,
                                     Type t) {
  auto const rat = make_repo_type(arrTable, t);
  using T = RepoAuthType::Tag;
  if (options.FilterAssertions) {
    // Gen and InitGen don't add any useful information, so leave them
    // out entirely.
    if (rat == RepoAuthType{T::Gen})     return folly::none;
    if (rat == RepoAuthType{T::InitGen}) return folly::none;
  }
  return Bytecode { TyBC { arg, rat } };
}

template<class Gen>
void insert_assertions_step(ArrayTypeTable::Builder& arrTable,
                            const php::Func& func,
                            const Bytecode& bcode,
                            const State& state,
                            std::bitset<kMaxTrackedLocals> mayReadLocalSet,
                            bool lastStackOutputObvious,
                            Gen gen) {
  if (state.unreachable) return;

  for (size_t i = 0; i < state.locals.size(); ++i) {
    if (options.FilterAssertions) {
      if (i < mayReadLocalSet.size() && !mayReadLocalSet.test(i)) {
        continue;
      }
    }
    auto const realT = state.locals[i];
    auto const op = makeAssert<bc::AssertRATL>(
      arrTable,
      borrow(func.locals[i]),
      realT
    );
    if (op) gen(*op);
  }

  if (!options.InsertStackAssertions) return;

  // Skip asserting the top of the stack if it just came immediately
  // out of an 'obvious' instruction (see hasObviousStackOutput), or
  // if this instruction ignoresStackInput.
  assert(state.stack.size() >= bcode.numPop());
  auto i = size_t{0};
  auto stackIdx = state.stack.size() - 1;
  if (options.FilterAssertions) {
    if (lastStackOutputObvious || ignoresStackInput(bcode.op)) {
      ++i, --stackIdx;
    }
  }

  /*
   * This doesn't need to account for ActRecs on the fpiStack, because
   * no instruction in an FPI region can ever consume a stack value
   * from above the pre-live ActRec.
   */
  for (; i < bcode.numPop(); ++i, --stackIdx) {
    auto const realT = state.stack[stackIdx];
    auto const flav  = stack_flav(realT);

    if (flav.subtypeOf(TCls)) continue;
    if (options.FilterAssertions && !realT.strictSubtypeOf(flav)) {
      continue;
    }

    auto const op =
      makeAssert<bc::AssertRATStk>(
        arrTable,
        static_cast<int32_t>(i),
        realT
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
  case Op::OODeclExists:
    return true;

  // Consider CGetL obvious because if we knew the type of the local,
  // we'll assert that right before the CGetL.  Similarly, the output
  // of SetL is obvious if you know what its input is (which we'll
  // assert if we know).
  case Op::CGetL:
  case Op::SetL:
    return true;

  default:
    return false;
  }
}

void insert_assertions(const Index& index,
                       const FuncAnalysis& ainfo,
                       borrowed_ptr<php::Block> const blk,
                       State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  auto& arrTable = *index.array_table_builder();
  auto const ctx = ainfo.ctx;

  auto lastStackOutputObvious = false;

  CollectedInfo collect { index, ctx, nullptr, nullptr };
  auto interp = Interp { index, ctx, collect, blk, state };
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
    auto const flags    = step(interp, op);

    insert_assertions_step(
      arrTable,
      *ctx.func,
      op,
      preState,
      flags.mayReadLocalSet,
      lastStackOutputObvious,
      gen
    );

    gen(op);
  }

  blk->hhbcs = std::move(newBCs);
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
    case Flavor::R:
      gen(bc::UnboxRNop {});
      gen(bc::PopC {});
      break;
    case Flavor::F:  not_reached();    break;
    case Flavor::U:  not_reached();    break;
    case Flavor::CVU:
      // Note that we only support C's for CVU so far (this only comes up with
      // FCallBuiltin)---we'll fail the verifier if something changes to send
      // V's or U's through here.
      gen(bc::PopC {});
      break;
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
      continue;
    }

    // Similar special case for FCallBuiltin.  We need to turn things into R
    // flavors since opcode that followed the call are going to expect that
    // flavor.
    if (op.op == Op::FCallBuiltin) {
      gen(bc::RGetCNop {});
      continue;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

/*
 * Create a block similar to another block (but with no bytecode in it yet).
 */
borrowed_ptr<php::Block> make_block(FuncAnalysis& ainfo,
                                    borrowed_ptr<const php::Block> srcBlk,
                                    const State& state) {
  FTRACE(1, " ++ new block {}\n", ainfo.ctx.func->nextBlockId);
  assert(ainfo.bdata.size() == ainfo.ctx.func->nextBlockId);

  auto newBlk           = folly::make_unique<php::Block>();
  newBlk->id            = ainfo.ctx.func->nextBlockId++;
  newBlk->section       = srcBlk->section;
  newBlk->exnNode       = srcBlk->exnNode;
  newBlk->factoredExits = srcBlk->factoredExits;
  auto const blk        = borrow(newBlk);
  ainfo.ctx.func->blocks.push_back(std::move(newBlk));

  ainfo.rpoBlocks.push_back(blk);
  ainfo.bdata.push_back(FuncAnalysis::BlockData {
    static_cast<uint32_t>(ainfo.rpoBlocks.size() - 1),
    state
  });

  return blk;
}

void first_pass(const Index& index,
                FuncAnalysis& ainfo,
                borrowed_ptr<php::Block> const blk,
                State state) {
  auto const ctx = ainfo.ctx;

  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  CollectedInfo collect { index, ctx, nullptr, nullptr };
  auto interp = Interp { index, ctx, collect, blk, state };

  auto peephole = make_peephole(newBCs);
  std::vector<Op> srcStack(state.stack.size(), Op::LowInvalid);

  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(op));

    auto const stateIn = state; // Peephole expects input eval state.
    auto gen = [&,srcStack] (const Bytecode& newBC) {
      const_cast<Bytecode&>(newBC).srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(newBC));
      if (options.Peephole) {
        peephole.append(newBC, stateIn, srcStack);
      } else {
        newBCs.push_back(newBC);
      }
    };

    auto const flags = step(interp, op);

    if (op.op == Op::CGetL2) {
      srcStack.insert(srcStack.end() - 1, op.op);
    } else if (op.op == Op::CGetL3) {
      srcStack.insert(srcStack.end() - 2, op.op);
    } else {
      for (int i = 0; i < op.numPop(); i++) {
        srcStack.pop_back();
      }
      for (int i = 0; i < op.numPush(); i++) {
        srcStack.push_back(op.op);
      }
    }

    /*
     * We only try to remove mid-block unreachable code if we're not in an FPI
     * region, because it's the easiest way to maintain FPI region invariants
     * in the emitted bytecode.
     */
    if (state.unreachable) {
      gen(op);
      if (!stateIn.unreachable) {
        gen(bc::BreakTraceHint {});
      }
      if (state.fpiStack.empty()) {
        if (!blk->fallthrough ||
            ainfo.bdata[blk->fallthrough->id].stateIn.initialized) {
          auto const fatal = make_block(ainfo, blk, state);
          fatal->hhbcs = {
            bc_with_loc(op.srcLoc, bc::String { s_unreachable.get() }),
            bc_with_loc(op.srcLoc, bc::Fatal { FatalOp::Runtime })
          };
          blk->fallthrough = fatal;
        }
        break;
      }
      continue;
    }

    if (options.RemoveDeadBlocks &&
        flags.jmpFlag != StepFlags::JmpFlags::Either) {
      always_assert(!flags.wasPEI);
      switch (flags.jmpFlag) {
      case StepFlags::JmpFlags::Taken:
        switch (op.op) {
        case Op::JmpNZ:     blk->fallthrough = op.JmpNZ.target;     break;
        case Op::JmpZ:      blk->fallthrough = op.JmpZ.target;      break;
        case Op::IterInit:  blk->fallthrough = op.IterInit.target;  break;
        case Op::IterInitK: blk->fallthrough = op.IterInitK.target; break;
        default:
          // No support for switch, etc, right now.
          always_assert(0 && "unsupported tookBranch case");
        }
        // fall through to PopC
      case StepFlags::JmpFlags::Fallthrough:
        /*
         * We need to pop the cell that was on the stack for the
         * conditional jump.  Note: for jumps this also conceptually
         * needs to execute any side effects a conversion to bool can
         * have.  (Currently that is none.)
         */
        gen(bc::PopC {});
        continue;
      case StepFlags::JmpFlags::Either:
        not_reached();
      }
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

  if (options.Peephole) {
    peephole.finalize();
  }
  blk->hhbcs = std::move(newBCs);
}

//////////////////////////////////////////////////////////////////////

template<class BlockContainer, class AInfo, class Fun>
void visit_blocks_impl(const char* what,
                       const Index& index,
                       AInfo& ainfo,
                       const BlockContainer& rpoBlocks,
                       Fun fun) {
  FTRACE(1, "|---- {}\n", what);
  for (auto& blk : rpoBlocks) {
    FTRACE(2, "block #{}\n", blk->id);
    auto const& state = ainfo.bdata[blk->id].stateIn;
    if (!state.initialized) {
      FTRACE(2, "   unreachable\n");
      continue;
    }
    // TODO(#3732260): this should probably spend an extra interp pass
    // in debug builds to check that no transformation to the bytecode
    // was made that changes the block output state.
    fun(index, ainfo, blk, state);
  }
  assert(check(*ainfo.ctx.func));
}

template<class Fun>
void visit_blocks_mutable(const char* what,
                          const Index& index,
                          FuncAnalysis& ainfo,
                          Fun fun) {
  // Make a copy of the block list so it can be mutated by the visitor.
  auto const blocksCopy = ainfo.rpoBlocks;
  visit_blocks_impl(what, index, ainfo, blocksCopy, fun);
}

template<class Fun>
void visit_blocks(const char* what,
                  const Index& index,
                  const FuncAnalysis& ainfo,
                  Fun fun) {
  visit_blocks_impl(what, index, ainfo, ainfo.rpoBlocks, fun);
}

//////////////////////////////////////////////////////////////////////

void do_optimize(const Index& index, FuncAnalysis ainfo) {
  FTRACE(2, "{:-^70}\n", "Optimize Func");

  visit_blocks_mutable("first pass", index, ainfo, first_pass);

  /*
   * Note: it's useful to do dead block removal before DCE, so it can remove
   * code relating to the branch to the dead block.
   */
  remove_unreachable_blocks(index, ainfo);

  if (options.LocalDCE) {
    visit_blocks("local DCE", index, ainfo, local_dce);
  }
  if (options.GlobalDCE) {
    global_dce(index, ainfo);
    assert(check(*ainfo.ctx.func));
    /*
     * Global DCE can change types of locals across blocks.  See dce.cpp for an
     * explanation.
     *
     * We need to perform a final type analysis before we do anything else.
     */
    ainfo = analyze_func(index, ainfo.ctx);
  }

  if (options.InsertAssertions) {
    visit_blocks("insert assertions", index, ainfo, insert_assertions);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void optimize_func(const Index& index, FuncAnalysis ainfo) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ainfo.ctx.unit)};
  do_optimize(index, std::move(ainfo));
}

//////////////////////////////////////////////////////////////////////

}}
