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

#include <folly/Optional.h>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/base/datatype.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/dce.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/peephole.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");
const StaticString s_86linit("86linit");

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
  case Op::UGetCUNop:
  case Op::CGetCUNop:
  case Op::PopU:
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
  if (t.subtypeOf(BBottom)) return folly::none;
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
                            std::vector<uint8_t> obviousStackOutputs,
                            Gen gen) {
  if (state.unreachable) return;

  for (LocalId i = 0; i < state.locals.size(); ++i) {
    if (func.locals[i].killed) continue;
    if (options.FilterAssertions) {
      if (i < mayReadLocalSet.size() && !mayReadLocalSet.test(i)) {
        continue;
      }
    }
    auto const realT = state.locals[i];
    auto const op = makeAssert<bc::AssertRATL>(arrTable, i, realT);
    if (op) gen(*op);
  }

  if (!options.InsertStackAssertions) return;

  assert(obviousStackOutputs.size() == state.stack.size());

  auto const assert_stack = [&] (size_t idx) {
    assert(idx < state.stack.size());
    if (obviousStackOutputs[state.stack.size() - idx - 1]) return;
    if (ignoresStackInput(bcode.op)) return;
    auto const realT = state.stack[state.stack.size() - idx - 1].type;
    auto const flav  = stack_flav(realT);

    assert(realT.subtypeOf(BBottom) || !realT.subtypeOf(BCls));
    if (options.FilterAssertions && !realT.strictSubtypeOf(flav)) {
      return;
    }

    auto const op =
      makeAssert<bc::AssertRATStk>(
        arrTable,
        static_cast<uint32_t>(idx),
        realT
      );
    if (op) gen(*op);
  };

  /*
   * This doesn't need to account for ActRecs on the fpiStack, because
   * no instruction in an FPI region can ever consume a stack value
   * from above the pre-live ActRec.
   */
  for (auto i = size_t{0}; i < bcode.numPop(); ++i) assert_stack(i);

  // The base instructions are special in that they don't pop anything, but do
  // read from the stack. We want type assertions on the stack slots they'll
  // read.
  switch (bcode.op) {
    case Op::BaseC:       assert_stack(bcode.BaseC.arg1);       break;
    case Op::BaseNC:      assert_stack(bcode.BaseNC.arg1);      break;
    case Op::BaseGC:      assert_stack(bcode.BaseGC.arg1);      break;
    case Op::BaseSC:      assert_stack(bcode.BaseSC.arg1);      break;
    case Op::BaseR:       assert_stack(bcode.BaseR.arg1);       break;
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
  // Generally consider CGetL obvious because if we knew the type of the local,
  // we'll assert that right before the CGetL.
  auto cgetlObvious = [&] (LocalId l, int idx) {
    return !interp.state.locals[l].couldBe(BRef) ||
      !interp.state.stack[interp.state.stack.size() - idx - 1].
         type.strictSubtypeOf(TInitCell);
  };
  switch (op.op) {
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
  case Op::Dict:
  case Op::Vec:
  case Op::Keyset:
  case Op::NewArray:
  case Op::NewDArray:
  case Op::NewDictArray:
  case Op::NewPackedArray:
  case Op::NewVArray:
  case Op::NewStructArray:
  case Op::NewStructDArray:
  case Op::NewStructDict:
  case Op::NewVecArray:
  case Op::NewKeysetArray:
  case Op::AddNewElemC:
  case Op::AddNewElemV:
  case Op::NewCol:
  case Op::NewPair:
  case Op::ClsRefName:
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
  case Op::Cmp:
  case Op::Shl:
  case Op::Shr:
  case Op::CastBool:
  case Op::CastInt:
  case Op::CastDouble:
  case Op::CastString:
  case Op::CastArray:
  case Op::CastObject:
  case Op::CastDict:
  case Op::CastVec:
  case Op::CastKeyset:
  case Op::CastVArray:
  case Op::CastDArray:
  case Op::DblAsBits:
  case Op::InstanceOfD:
  case Op::IsTypeStruct:
  case Op::AsTypeStruct:
  case Op::CombineAndResolveTypeStruct:
  case Op::InstanceOf:
  case Op::Print:
  case Op::Exit:
  case Op::AKExists:
  case Op::IssetL:
  case Op::IssetN:
  case Op::IssetG:
  case Op::IssetS:
  case Op::EmptyL:
  case Op::EmptyN:
  case Op::EmptyG:
  case Op::EmptyS:
  case Op::IsTypeC:
  case Op::IsTypeL:
  case Op::OODeclExists:
  case Op::AliasCls:
    return true;

  case Op::This:
  case Op::BareThis:
    if (auto tt = thisType(interp)) {
      auto t = interp.state.stack.back().type;
      if (is_opt(t)) t = unopt(std::move(t));
      return !t.strictSubtypeOf(*tt);
    }
    return true;

  case Op::CGetL:
    return cgetlObvious(op.CGetL.loc1, 0);
  case Op::CGetQuietL:
    return cgetlObvious(op.CGetQuietL.loc1, 0);
  case Op::CUGetL:
    return cgetlObvious(op.CUGetL.loc1, 0);
  case Op::CGetL2:
    return cgetlObvious(op.CGetL2.loc1, 1);
  case Op::PushL:
    return cgetlObvious(op.PushL.loc1, 0);

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

void insert_assertions(const Index& index,
                       const FuncAnalysis& ainfo,
                       CollectedInfo& collect,
                       php::Block* const blk,
                       State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  auto& arrTable = *index.array_table_builder();
  auto const ctx = ainfo.ctx;

  std::vector<uint8_t> obviousStackOutputs(state.stack.size(), false);

  auto interp = Interp { index, ctx, collect, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(ctx.func, op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(ctx.func, newBCs.back()));
    };

    if (state.unreachable) {
      blk->fallthrough = NoBlockId;
      blk->unwindExits = {};
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
      arrTable,
      *ctx.func,
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

  blk->hhbcs = std::move(newBCs);
}

bool persistence_check(php::Block* const blk) {
  for (auto& op : blk->hhbcs) {
    switch (op.op) {
      case Op::Nop:
      case Op::DefCls:
      case Op::DefClsNop:
      case Op::DefCns:
      case Op::DefTypeAlias:
      case Op::Null:
      case Op::True:
      case Op::False:
      case Op::Int:
      case Op::Double:
      case Op::String:
      case Op::Vec:
      case Op::Dict:
      case Op::Keyset:
      case Op::Array:
        continue;
      case Op::PopC:
        // Not strictly no-side effects, but as long as the rest of
        // the unit is limited to the above, we're fine (and we expect
        // one following a DefCns).
        continue;
      case Op::RetC:
        continue;
      default:
        return false;
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

template<class Gen>
bool propagate_constants(const Bytecode& op, State& state, Gen gen) {
  auto const numPop  = op.numPop();
  auto const numPush = op.numPush();
  auto const stkSize = state.stack.size();
  constexpr auto numCells = 4;
  Cell constVals[numCells];

  // All outputs of the instruction must have constant types for this
  // to be allowed.
  for (auto i = size_t{0}; i < numPush; ++i) {
    auto const& ty = state.stack[stkSize - i - 1].type;
    if (i < numCells) {
      auto const v = tv(ty);
      if (!v) return false;
      constVals[i] = *v;
    } else if (!is_scalar(ty)) {
      return false;
    }
  }

  auto const slot = visit(op, ReadClsRefSlotVisitor{});
  if (slot != NoClsRefSlotId) gen(bc::DiscardClsRef { slot });

  // Pop the inputs, and push the constants.
  for (auto i = size_t{0}; i < numPop; ++i) {
    switch (op.popFlavor(i)) {
    case Flavor::C:  gen(bc::PopC {}); break;
    case Flavor::V:  gen(bc::PopV {}); break;
    case Flavor::R:
      gen(bc::UnboxRNop {});
      gen(bc::PopC {});
      break;
    case Flavor::U:  not_reached();    break;
    case Flavor::CR: not_reached();    break;
    case Flavor::CU:
      // We only support C's for CU right now.
      gen(bc::PopC {});
      break;
    case Flavor::CV: not_reached();    break;
    case Flavor::CVU:
      // Note that we only support C's for CVU so far (this only comes up with
      // FCallBuiltin)---we'll fail the verifier if something changes to send
      // V's or U's through here.
      gen(bc::PopC {});
      break;
    }
  }

  for (auto i = size_t{0}; i < numPush; ++i) {
    auto const v = i < numCells ?
      constVals[i] : *tv(state.stack[stkSize - i - 1].type);
    gen(gen_constant(v));
    state.stack[stkSize - i - 1].type = from_cell(v);

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

bool propagate_constants(const Bytecode& bc, State& state,
                         std::vector<Bytecode>& out) {
  return propagate_constants(bc, state, [&] (const Bytecode& bc) {
      out.push_back(bc);
    });
}

//////////////////////////////////////////////////////////////////////

/*
 * Create a block similar to another block (but with no bytecode in it yet).
 */
php::Block* make_block(FuncAnalysis& ainfo,
                                    const php::Block* srcBlk,
                                    const State& state) {
  FTRACE(1, " ++ new block {}\n", ainfo.ctx.func->blocks.size());
  assert(ainfo.bdata.size() == ainfo.ctx.func->blocks.size());

  auto newBlk           = std::make_unique<php::Block>();
  newBlk->id            = ainfo.ctx.func->blocks.size();
  newBlk->section       = srcBlk->section;
  newBlk->exnNode       = srcBlk->exnNode;
  newBlk->throwExits    = srcBlk->throwExits;
  newBlk->unwindExits   = srcBlk->unwindExits;
  auto const blk        = newBlk.get();
  ainfo.ctx.func->blocks.push_back(std::move(newBlk));

  ainfo.rpoBlocks.push_back(blk);
  ainfo.bdata.push_back(FuncAnalysis::BlockData {
    static_cast<uint32_t>(ainfo.rpoBlocks.size() - 1),
    state
  });

  return blk;
}

php::Block* make_fatal_block(FuncAnalysis& ainfo,
                                          const php::Block* srcBlk,
                                          const State& state) {
  auto blk = make_block(ainfo, srcBlk, state);
  auto const srcLoc = srcBlk->hhbcs.back().srcLoc;
  blk->hhbcs = {
    bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
    bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
  };
  blk->fallthrough = NoBlockId;
  blk->throwExits = {};
  blk->unwindExits = {};
  blk->exnNode = nullptr;
  return blk;
}

void first_pass(const Index& index,
                FuncAnalysis& ainfo,
                CollectedInfo& collect,
                php::Block* const blk,
                State state) {
  auto const ctx = ainfo.ctx;

  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  auto interp = Interp { index, ctx, collect, blk, state };

  if (options.ConstantProp) collect.propagate_constants = propagate_constants;

  auto peephole = make_peephole(newBCs, index, ctx);
  std::vector<std::pair<Op,bool>> srcStack(state.stack.size(),
                                           {Op::Nop, false});

  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(ctx.func, op));

    auto gen = [&] (const Bytecode& newBC) {
      const_cast<Bytecode&>(newBC).srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(ctx.func, newBC));
      if (options.Peephole) {
        peephole.append(newBC, srcStack);
      } else {
        newBCs.push_back(newBC);
      }
    };

    auto const flags = step(interp, op);

    // The peephole wants the old values of srcStack, so defer the update to the
    // end of the loop.
    SCOPE_EXIT {
      if (op.op == Op::CGetL2) {
        srcStack.emplace(srcStack.end() - 1,
                         op.op, (state.stack.end() - 2)->type.subtypeOf(BStr));
      } else {
        FTRACE(2, "   srcStack: pop {} push {}\n", op.numPop(), op.numPush());
        for (int i = 0; i < op.numPop(); i++) {
          srcStack.pop_back();
        }
        for (int i = 0; i < op.numPush(); i++) {
          srcStack.emplace_back(
            op.op, state.stack[srcStack.size()].type.subtypeOf(BStr));
        }
      }
    };

    auto genOut = [&] (const Bytecode* op) -> Op {
      if (options.ConstantProp && flags.canConstProp) {
        if (propagate_constants(*op, state, gen)) {
          assert(!flags.strengthReduced);
          return Op::Nop;
        }
      }

      if (flags.strengthReduced) {
        for (auto const& bc : *flags.strengthReduced) {
          gen(bc);
        }
        return flags.strengthReduced->back().op;
      }

      gen(*op);
      return op->op;
    };

    if (state.unreachable) {
      // We should still perform the requested transformations; we
      // might be part way through converting an FPush/FCall to an
      // FCallBuiltin, for example
      auto opc = genOut(&op);
      blk->fallthrough = NoBlockId;
      blk->unwindExits = {};
      if (!(instrFlags(opc) & TF)) {
        gen(bc::BreakTraceHint {});
        gen(bc::String { s_unreachable.get() });
        gen(bc::Fatal { FatalOp::Runtime });
      }
      break;
    }

    if (options.RemoveDeadBlocks) {
      if (flags.jmpDest != NoBlockId) {
        switch (op.op) {
          /*
           * For jumps, we need to pop the cell that was on the stack for the
           * conditional jump.  Note: for jumps this also conceptually
           * needs to execute any side effects a conversion to bool can
           * have.  (Currently that is none.)
           */
        case Op::JmpNZ:
        case Op::JmpZ:
        case Op::SSwitch:
        case Op::Switch:
          always_assert(!flags.wasPEI);
          blk->fallthrough = flags.jmpDest;
          gen(bc::PopC {});
          continue;
        case Op::IterInit:
        case Op::LIterInit:
        case Op::IterInitK:
        case Op::LIterInitK:
          if (flags.jmpDest != blk->fallthrough) {
            /*
             * For iterators, if we'll always take the taken branch (which means
             * there's nothing to iterate over), and the op cannot raise an
             * exception, we can just pop the input and set the fall-through to
             * the taken branch. If not, we have to keep the op, but we can make
             * sure we'll fatal if we ever actually take the fall-through.
             */
            if (!flags.wasPEI) {
              blk->fallthrough = flags.jmpDest;
              if (op.op != Op::LIterInit && op.op != Op::LIterInitK) {
                gen(bc::PopC {});
              }
              continue;
            }
            blk->fallthrough = make_fatal_block(ainfo, blk, state)->id;
          } else {
            /*
             * We can't ever optimize away iteration initialization if we know
             * we'll always fall-through (which means we enter the loop) because
             * we need to initialize the iterator. We can ensure, however, that
             * the taken branch is a fatal.
             */
            auto fatal = make_fatal_block(ainfo, blk, state)->id;
            if (op.op == Op::IterInit) {
              op.IterInit.target = fatal;
            } else if (op.op == Op::IterInitK) {
              op.IterInitK.target = fatal;
            } else if (op.op == Op::LIterInit) {
              op.LIterInit.target = fatal;
            } else if (op.op == Op::LIterInitK) {
              op.LIterInitK.target = fatal;
            }
          }
          break;
        case Op::IterNext:
        case Op::IterNextK:
        case Op::LIterNext:
        case Op::LIterNextK:
          assertx(flags.jmpDest == blk->fallthrough);
          /*
           * If we're nexting an iterator and we know we'll always fall-through
           * (which means the iteration is over), and we can't raise an
           * exception when nexting the iterator, we can just free the iterator
           * and let it fall-through. If not, we can at least ensure the taken
           * branch is a fatal.
           */
          if (!flags.wasPEI) {
            if (op.op == Op::IterNext) {
              gen(bc::IterFree { op.IterNext.iter1 });
            } else if (op.op == Op::IterNextK) {
              gen(bc::IterFree { op.IterNextK.iter1 });
            } else if (op.op == Op::LIterNext) {
              gen(bc::LIterFree { op.LIterNext.iter1, op.LIterNext.loc2 });
            } else {
              gen(bc::LIterFree { op.LIterNextK.iter1, op.LIterNextK.loc2 });
            }
            continue;
          } else if (op.op == Op::IterNext) {
            op.IterNext.target = make_fatal_block(ainfo, blk, state)->id;
          } else if (op.op == Op::IterNextK) {
            op.IterNextK.target = make_fatal_block(ainfo, blk, state)->id;
          } else if (op.op == Op::LIterNext) {
            op.LIterNext.target = make_fatal_block(ainfo, blk, state)->id;
          } else {
            op.LIterNextK.target = make_fatal_block(ainfo, blk, state)->id;
          }
          break;
        case Op::MemoGet:
          if (flags.jmpDest != blk->fallthrough) {
            if (!flags.wasPEI) {
              blk->fallthrough = flags.jmpDest;
              continue;
            }
            blk->fallthrough = make_fatal_block(ainfo, blk, state)->id;
          }
          break;
        default:
          always_assert(0 && "unsupported jmpDest");
        }
      }
    }

    genOut(&op);
  }

  if (options.Peephole) {
    peephole.finalize();
  }
  blk->hhbcs = std::move(newBCs);
  auto& fpiStack = ainfo.bdata[blk->id].stateIn.fpiStack;
  auto it = std::remove_if(fpiStack.begin(), fpiStack.end(),
                           [](const ActRec& ar) {
                             return ar.kind == FPIKind::Builtin || ar.foldable;
                           });

  if (it != fpiStack.end()) {
    fpiStack.erase(it, fpiStack.end());
  }
}

//////////////////////////////////////////////////////////////////////

template<class BlockContainer, class AInfo, class Fun>
void visit_blocks_impl(const char* what,
                       const Index& index,
                       AInfo& ainfo,
                       CollectedInfo& collect,
                       const BlockContainer& rpoBlocks,
                       Fun&& fun) {

  BlockId curBlk = NoBlockId;
  SCOPE_ASSERT_DETAIL(what) {
    if (curBlk == NoBlockId) return std::string{"\nNo block processed\n"};
    return folly::sformat(
        "block #{}\nin-{}", curBlk,
        state_string(*ainfo.ctx.func, ainfo.bdata[curBlk].stateIn, collect)
    );
  };

  FTRACE(1, "|---- {}\n", what);
  for (auto& blk : rpoBlocks) {
    curBlk = blk->id;
    FTRACE(2, "block #{}\n", blk->id);
    auto const& state = ainfo.bdata[blk->id].stateIn;
    if (!state.initialized) {
      FTRACE(2, "   unreachable\n");
      continue;
    }
    // TODO(#3732260): this should probably spend an extra interp pass
    // in debug builds to check that no transformation to the bytecode
    // was made that changes the block output state.
    fun(index, ainfo, collect, blk, state);
  }
  assert(check(*ainfo.ctx.func));
}

template<class Fun>
void visit_blocks_mutable(const char* what,
                          const Index& index,
                          FuncAnalysis& ainfo,
                          CollectedInfo& collect,
                          Fun&& fun) {
  // Make a copy of the block list so it can be mutated by the visitor.
  auto const blocksCopy = ainfo.rpoBlocks;
  visit_blocks_impl(what, index, ainfo, collect,
                    blocksCopy, std::forward<Fun>(fun));
}

template<class Fun>
void visit_blocks(const char* what,
                  const Index& index,
                  const FuncAnalysis& ainfo,
                  CollectedInfo& collect,
                  Fun&& fun) {
  visit_blocks_impl(what, index, ainfo, collect,
                    ainfo.rpoBlocks, std::forward<Fun>(fun));
}

//////////////////////////////////////////////////////////////////////

IterId iterFromInit(const php::Func& func, BlockId initBlock) {
  auto const& op = func.blocks[initBlock]->hhbcs.back();
  if (op.op == Op::IterInit)   return op.IterInit.iter1;
  if (op.op == Op::IterInitK)  return op.IterInitK.iter1;
  if (op.op == Op::LIterInit)  return op.LIterInit.iter1;
  if (op.op == Op::LIterInitK) return op.LIterInitK.iter1;
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
  void operator()(const Index& index,
                  const FuncAnalysis& ainfo,
                  CollectedInfo& collect,
                  php::Block* const blk,
                  State state) {
    auto const ctx = ainfo.ctx;
    auto interp = Interp { index, ctx, collect, blk, state };
    for (uint32_t opIdx = 0; opIdx < blk->hhbcs.size(); ++opIdx) {
      // If we've already determined that nothing is eligible, we can just stop.
      if (!eligible.any()) break;

      auto const& op = blk->hhbcs[opIdx];
      FTRACE(2, "  == {}\n", show(ctx.func, op));

      if (state.unreachable) break;

      // At every op, we check the known state of all live iterators and mark it
      // as ineligible as necessary.
      for (IterId it = 0; it < state.iters.size(); ++it) {
        match<void>(
          state.iters[it],
          []  (DeadIter) {},
          [&] (const LiveIter& ti) {
            FTRACE(4, "   iter {: <2}  :: {}\n",
                   it, show(*ctx.func, state.iters[it]));
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
            }
          }
        );
      }

      auto const fixupForInit = [&] {
        auto const base = topStkLocal(state);
        if (base == NoLocalId && eligible[blk->id]) {
          FTRACE(2, "   - blk:{} ineligible\n", blk->id);
          eligible[blk->id] = false;
        }
        fixups.emplace_back(Fixup{blk->id, opIdx, blk->id, base});
        FTRACE(2, "   + fixup ({})\n", fixups.back().show(*ctx.func));
      };

      auto const fixupFromState = [&] (IterId it) {
        match<void>(
          state.iters[it],
          []  (DeadIter) {},
          [&] (const LiveIter& ti) {
            if (ti.initBlock != NoBlockId) {
              assertx(iterFromInit(*ctx.func, ti.initBlock) == it);
              fixups.emplace_back(
                Fixup{blk->id, opIdx, ti.initBlock, ti.baseLocal}
              );
              FTRACE(2, "   + fixup ({})\n", fixups.back().show(*ctx.func));
            }
          }
        );
      };

      // Record a fixup for this iteration op. This iteration loop may not be
      // ultimately eligible, but we'll check that before actually doing the
      // transformation.
      switch (op.op) {
        case Op::IterInit:
        case Op::IterInitK:
          assertx(opIdx == blk->hhbcs.size() - 1);
          fixupForInit();
          break;
        case Op::IterNext:
          fixupFromState(op.IterNext.iter1);
          break;
        case Op::IterNextK:
          fixupFromState(op.IterNextK.iter1);
          break;
        case Op::IterFree:
          fixupFromState(op.IterFree.iter1);
          break;
        case Op::IterBreak:
          for (auto const& it : op.IterBreak.iterTab) {
            if (it.kind == KindOfIter) fixupFromState(it.id);
          }
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
};

void optimize_iterators(const Index& index,
                        const FuncAnalysis& ainfo,
                        CollectedInfo& collect) {
  auto const func = ainfo.ctx.func;
  // Quick exit. If there's no iterators, or if no associated local survives to
  // the end of the iterator, there's nothing to do.
  if (!func->numIters || !ainfo.hasInvariantIterBase) return;

  OptimizeIterState state;
  // Everything starts out as eligible. We'll remove initialization blocks as we
  // go.
  state.eligible.resize(func->blocks.size(), true);

  // Visit all the blocks and build up the fixup state.
  visit_blocks("optimize_iterators", index, ainfo, collect, state);
  if (!state.eligible.any()) return;

  FTRACE(2, "Rewrites:\n");
  for (auto const& fixup : state.fixups) {
    auto const& blk = func->blocks[fixup.block];
    auto const& op = blk->hhbcs[fixup.op];

    if (!state.eligible[fixup.init]) {
      // This iteration loop isn't eligible, so don't apply the fixup
      FTRACE(2, "   * ({}): {}\n", fixup.show(*func), show(func, op));
      continue;
    }

    std::vector<Bytecode> newOps;
    assertx(fixup.base != NoLocalId);

    // Rewrite the iteration op to its liter equivalent:
    switch (op.op) {
      case Op::IterInit:
        newOps = {
          bc_with_loc(op.srcLoc, bc::PopC {}),
          bc_with_loc(
            op.srcLoc,
            bc::LIterInit {
              op.IterInit.iter1,
                fixup.base,
                op.IterInit.target,
                op.IterInit.loc3
            }
          )
        };
        break;
      case Op::IterInitK:
        newOps = {
          bc_with_loc(op.srcLoc, bc::PopC {}),
          bc_with_loc(
            op.srcLoc,
            bc::LIterInitK {
              op.IterInitK.iter1,
                fixup.base,
                op.IterInitK.target,
                op.IterInitK.loc3,
                op.IterInitK.loc4
            }
          )
        };
        break;
      case Op::IterNext:
        newOps = {
          bc_with_loc(
            op.srcLoc,
            bc::LIterNext {
              op.IterNext.iter1,
                fixup.base,
                op.IterNext.target,
                op.IterNext.loc3
            }
          )
        };
        break;
      case Op::IterNextK:
        newOps = {
          bc_with_loc(
            op.srcLoc,
            bc::LIterNextK {
              op.IterNextK.iter1,
                fixup.base,
                op.IterNextK.target,
                op.IterNextK.loc3,
                op.IterNextK.loc4
            }
          )
        };
        break;
      case Op::IterFree:
        newOps = {
          bc_with_loc(
            op.srcLoc,
            bc::LIterFree { op.IterFree.iter1, fixup.base }
          )
        };
        break;
      case Op::IterBreak: {
        auto const iter = iterFromInit(*func, fixup.init);
        newOps = { op };
        for (auto& it : newOps.back().IterBreak.iterTab) {
          if (it.id == iter) {
            assertx(it.kind == KindOfIter);
            it.kind = KindOfLIter;
            it.local = fixup.base;
          }
        }
        break;
      }
      default:
        always_assert(false);
    }

    FTRACE(
      2, "   ({}): {} ==> {}\n",
      fixup.show(*func), show(func, op),
      [&] {
        using namespace folly::gen;
        return from(newOps)
          | map([&] (const Bytecode& bc) { return show(func, bc); })
          | unsplit<std::string>(",");
      }()
    );

    blk->hhbcs.erase(blk->hhbcs.begin() + fixup.op);
    blk->hhbcs.insert(blk->hhbcs.begin() + fixup.op,
                      newOps.begin(), newOps.end());
  }

  FTRACE(10, "{}", show(*func));
}

//////////////////////////////////////////////////////////////////////

/*
 * Use the information in the index to resolve a type-constraint to its
 * underlying type, if possible.
 */
void fixTypeConstraint(Context ctx,
                       const Index& index,
                       TypeConstraint& tc,
                       const Type& candidate) {
  auto t = index.lookup_constraint(ctx, tc, candidate);

  if (is_specialized_obj(t) &&
      !dobj_of(t).cls.couldHaveMockedDerivedClass()) {
    tc.setNoMockObjects();
  }

  if (!tc.isCheckable() || tc.isSoft() || !tc.isObject()) return;

  auto const nullable = is_opt(t);
  if (nullable) t = unopt(std::move(t));

  auto retype = [&] (AnnotType t) {
    tc.resolveType(t, nullable);
    FTRACE(1, "Retype tc {} -> {}\n", tc.typeName(), tc.displayName());
  };

  assertx(!RuntimeOption::EvalHackArrDVArrs ||
          (!t.subtypeOf(BVArr) && !t.subtypeOf(BDArr)));

  if (t.subtypeOf(BInitNull)) return retype(AnnotType::Null);
  if (t.subtypeOf(BBool))     return retype(AnnotType::Bool);
  if (t.subtypeOf(BInt))      return retype(AnnotType::Int);
  if (t.subtypeOf(BDbl))      return retype(AnnotType::Float);
  if (t.subtypeOf(BStr))      return retype(AnnotType::String);
  if (t.subtypeOf(BPArr))     return retype(AnnotType::Array);
  if (t.subtypeOf(BVArr))     return retype(AnnotType::VArray);
  if (t.subtypeOf(BDArr))     return retype(AnnotType::DArray);
  // if (t.subtypeOf(BObj))   return retype(AnnotType::Object);
  if (t.subtypeOf(BRes))      return retype(AnnotType::Resource);
  if (t.subtypeOf(BDict))     return retype(AnnotType::Dict);
  if (t.subtypeOf(BVec))      return retype(AnnotType::Vec);
  if (t.subtypeOf(BKeyset))   return retype(AnnotType::Keyset);
}

//////////////////////////////////////////////////////////////////////

void do_optimize(const Index& index, FuncAnalysis&& ainfo, bool isFinal) {
  FTRACE(2, "{:-^70} {}\n", "Optimize Func", ainfo.ctx.func->name);

  bool again;
  folly::Optional<CollectedInfo> collect;

  collect.emplace(
    index, ainfo.ctx, nullptr, nullptr,
    CollectionOpts::TrackConstantArrays, &ainfo
  );

  optimize_iterators(index, ainfo, *collect);

  do {
    again = false;
    visit_blocks_mutable("first pass", index, ainfo, *collect, first_pass);

    FTRACE(10, "{}", show(*ainfo.ctx.func));
    /*
     * Note: it's useful to do dead block removal before DCE, so it can remove
     * code relating to the branch to the dead block.
     */
    remove_unreachable_blocks(ainfo);

    if (options.LocalDCE) {
      visit_blocks("local DCE", index, ainfo, *collect, local_dce);
    }
    if (options.GlobalDCE) {
      global_dce(index, ainfo);
      again = control_flow_opts(ainfo);
      assert(check(*ainfo.ctx.func));
      /*
       * Global DCE can change types of locals across blocks.  See
       * dce.cpp for an explanation.
       *
       * We need to perform a final type analysis before we do
       * anything else.
       */
      ainfo = analyze_func(index,
                           ainfo.ctx,
                           CollectionOpts::TrackConstantArrays);
      collect.emplace(
        index, ainfo.ctx, nullptr, nullptr,
        CollectionOpts::TrackConstantArrays, &ainfo
      );
    }

    // If we merged blocks, there could be new optimization opportunities
  } while (again);

  if (!isFinal) return;

  auto const func = ainfo.ctx.func;
  if (func->name == s_86pinit.get() ||
      func->name == s_86sinit.get() ||
      func->name == s_86linit.get()) {
    auto const& blk = *func->blocks[func->mainEntry];
    if (blk.hhbcs.size() == 2 &&
        blk.hhbcs[0].op == Op::Null &&
        blk.hhbcs[1].op == Op::RetC) {
      FTRACE(2, "Erasing {}::{}\n", func->cls->name, func->name);
      func->cls->methods.erase(
        std::find_if(func->cls->methods.begin(),
                     func->cls->methods.end(),
                     [&](const std::unique_ptr<php::Func>& f) {
                       return f.get() == func;
                     }));
      return;
    }
  }

  auto pseudomain = is_pseudomain(func);
  func->attrs = (pseudomain ||
                 func->attrs & AttrInterceptable ||
                 ainfo.mayUseVV) ?
    Attr(func->attrs | AttrMayUseVV) : Attr(func->attrs & ~AttrMayUseVV);

  if (pseudomain && func->unit->persistent.load(std::memory_order_relaxed)) {
    auto persistent = true;
    visit_blocks("persistence check", index, ainfo, *collect,
                 [&] (const Index&,
                      const FuncAnalysis&,
                      CollectedInfo&,
                      php::Block* blk,
                      const State&) {
                   if (persistent && !persistence_check(blk)) {
                     persistent = false;
                   }
                 });
    if (!persistent) {
      func->unit->persistent.store(persistent, std::memory_order_relaxed);
    }
  }

  if (options.InsertAssertions) {
    visit_blocks("insert assertions", index, ainfo, *collect,
                 insert_assertions);
  }

  if (RuntimeOption::EvalHardTypeHints) {
    for (auto& p : func->params) {
      fixTypeConstraint(ainfo.ctx, index, p.typeConstraint, TTop);
    }
  }

  if (RuntimeOption::EvalCheckReturnTypeHints >= 3) {
    auto const rtype = [&] {
      if (!func->isAsync) return ainfo.inferredReturn;
      if (!is_specialized_wait_handle(ainfo.inferredReturn)) return TGen;
      return wait_handle_inner(ainfo.inferredReturn);
    }();
    fixTypeConstraint(ainfo.ctx, index, func->retTypeConstraint, rtype);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Bytecode gen_constant(const Cell& cell) {
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
      assert(cell.m_data.pstr->isStatic());
    case KindOfPersistentString:
      return bc::String { cell.m_data.pstr };
    case KindOfVec:
      assert(cell.m_data.parr->isStatic());
    case KindOfPersistentVec:
      assert(cell.m_data.parr->isVecArray());
      return bc::Vec { cell.m_data.parr };
    case KindOfDict:
      assert(cell.m_data.parr->isStatic());
    case KindOfPersistentDict:
      assert(cell.m_data.parr->isDict());
      return bc::Dict { cell.m_data.parr };
    case KindOfKeyset:
      assert(cell.m_data.parr->isStatic());
    case KindOfPersistentKeyset:
      assert(cell.m_data.parr->isKeyset());
      return bc::Keyset { cell.m_data.parr };
    case KindOfShape:
    case KindOfPersistentShape:
      not_implemented();
    case KindOfArray:
      assert(cell.m_data.parr->isStatic());
    case KindOfPersistentArray:
      assert(cell.m_data.parr->isPHPArray());
      return bc::Array { cell.m_data.parr };

    case KindOfRef:
    case KindOfResource:
    case KindOfObject:
    case KindOfFunc:
    case KindOfClass:
      always_assert(0 && "invalid constant in propagate_constants");
  }
  not_reached();
}

void optimize_func(const Index& index, FuncAnalysis&& ainfo, bool isFinal) {
  auto const bump = trace_bump_for(ainfo.ctx.cls, ainfo.ctx.func);

  SCOPE_ASSERT_DETAIL("optimize_func") {
    return "Optimizing:" + show(ainfo.ctx);
  };

  Trace::Bump bumper1{Trace::hhbbc, bump};
  Trace::Bump bumper2{Trace::hhbbc_cfg, bump};
  Trace::Bump bumper3{Trace::hhbbc_dce, bump};
  do_optimize(index, std::move(ainfo), isFinal);
}

//////////////////////////////////////////////////////////////////////

void optimize_class_prop_type_hints(const Index& index, Context ctx) {
  assertx(!ctx.func);
  auto const bump = trace_bump_for(ctx.cls, nullptr);
  Trace::Bump bumper{Trace::hhbbc, bump};
  for (auto& prop : ctx.cls->properties) {
    fixTypeConstraint(
      ctx,
      index,
      const_cast<TypeConstraint&>(prop.typeConstraint),
      TTop
    );
  }
}

//////////////////////////////////////////////////////////////////////

}}
