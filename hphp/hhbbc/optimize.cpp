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

#include <folly/Optional.h>

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
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/peephole.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");

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
  case Op::FPassVNop:
  case Op::FPassC:
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

  for (LocalId i = 0; i < state.locals.size(); ++i) {
    if (func.locals[i].killed) continue;
    if (options.FilterAssertions) {
      // MemoGet and MemoSet read from a range of locals, but don't gain any
      // benefit from knowing their types.
      if (bcode.op == Op::MemoGet || bcode.op == Op::MemoSet) continue;
      if (i < mayReadLocalSet.size() && !mayReadLocalSet.test(i)) {
        continue;
      }
    }
    auto const realT = state.locals[i];
    auto const op = makeAssert<bc::AssertRATL>(arrTable, i, realT);
    if (op) gen(*op);
  }

  if (!options.InsertStackAssertions) return;

  auto const assert_stack = [&] (size_t idx) {
    assert(idx < state.stack.size());
    auto const realT = state.stack[state.stack.size() - idx - 1].type;
    auto const flav  = stack_flav(realT);

    assert(!realT.subtypeOf(TCls));
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

  // Skip asserting the top of the stack if it just came immediately
  // out of an 'obvious' instruction (see hasObviousStackOutput), or
  // if this instruction ignoresStackInput.
  assert(state.stack.size() >= bcode.numPop());
  auto i = size_t{0};
  if (options.FilterAssertions) {
    if (lastStackOutputObvious || ignoresStackInput(bcode.op)) {
      ++i;
    }
  }

  /*
   * This doesn't need to account for ActRecs on the fpiStack, because
   * no instruction in an FPI region can ever consume a stack value
   * from above the pre-live ActRec.
   */
  for (; i < bcode.numPop(); ++i) assert_stack(i);

  // The base instructions are special in that they don't pop anything, but do
  // read from the stack. We want type assertions on the stack slots they'll
  // read.
  switch (bcode.op) {
    case Op::BaseC:       assert_stack(bcode.BaseC.arg1);       break;
    case Op::BaseNC:      assert_stack(bcode.BaseNC.arg1);      break;
    case Op::BaseGC:      assert_stack(bcode.BaseGC.arg1);      break;
    case Op::BaseSC:      assert_stack(bcode.BaseSC.arg1);      break;
    case Op::BaseR:       assert_stack(bcode.BaseR.arg1);       break;
    case Op::FPassBaseNC: assert_stack(bcode.FPassBaseNC.arg2); break;
    case Op::FPassBaseGC: assert_stack(bcode.FPassBaseGC.arg2); break;
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
bool hasObviousStackOutput(const Bytecode& op, const State& state) {
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
  case Op::NewDictArray:
  case Op::NewPackedArray:
  case Op::NewStructArray:
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
  case Op::InstanceOfD:
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
  case Op::IsUninit:
  case Op::OODeclExists:
  case Op::AliasCls:
    return true;

  // Consider CGetL obvious because if we knew the type of the local,
  // we'll assert that right before the CGetL.  Similarly, the output
  // of SetL is obvious if you know what its input is (which we'll
  // assert if we know).
  case Op::CGetL:
    if (state.locals[op.CGetL.loc1].couldBe(TRef) &&
        state.stack.back().type.strictSubtypeOf(TInitCell)) {
      // In certain cases (local static, for example) we can have
      // information about the unboxed value of the local which isn't
      // obvious from the local itself (which will be TRef or TGen).
      return false;
    }
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

  CollectedInfo collect { index, ctx, nullptr, nullptr, true, &ainfo };
  auto interp = Interp { index, ctx, collect, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(ctx.func, op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(ctx.func, newBCs.back()));

      lastStackOutputObvious =
        newb.numPush() != 0 && hasObviousStackOutput(newb, state);
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

bool persistence_check(borrowed_ptr<php::Block> const blk) {
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
bool propagate_constants(const Bytecode& op, const State& state, Gen gen) {
  auto const numPop  = op.numPop();
  auto const numPush = op.numPush();
  auto const stkSize = state.stack.size();

  // All outputs of the instruction must have constant types for this
  // to be allowed.
  for (auto i = size_t{0}; i < numPush; ++i) {
    if (!tv(state.stack[stkSize - i - 1].type)) return false;
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
    case Flavor::F:  not_reached();    break;
    case Flavor::U:  not_reached();    break;
    case Flavor::CR: not_reached();    break;
    case Flavor::CUV:
      // We only support C's for CUV right now.
      gen(bc::PopC {});
      break;
    case Flavor::CVU:
      // Note that we only support C's for CVU so far (this only comes up with
      // FCallBuiltin)---we'll fail the verifier if something changes to send
      // V's or U's through here.
      gen(bc::PopC {});
      break;
    }
  }

  for (auto i = size_t{0}; i < numPush; ++i) {
    auto const v = tv(state.stack[stkSize - i - 1].type);
    gen(gen_constant(*v));

    // Special case for FPass* instructions.  We just put a C on the
    // stack, so we need to get it to be an F.
    if (isFPassStar(op.op)) {
      if (state.fpiStack.back().kind != FPIKind::Builtin) {
        // We should only ever const prop for FPassL right now.
        always_assert(numPush == 1 && op.op == Op::FPassL);
        gen(bc::FPassC { op.FPassL.arg1 });
      }
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

bool propagate_constants(const Bytecode& bc, const State& state,
                         std::vector<Bytecode>& out) {
  return propagate_constants(bc, state, [&] (const Bytecode& bc) {
      out.push_back(bc);
    });
}

//////////////////////////////////////////////////////////////////////

/*
 * Create a block similar to another block (but with no bytecode in it yet).
 */
borrowed_ptr<php::Block> make_block(FuncAnalysis& ainfo,
                                    borrowed_ptr<const php::Block> srcBlk,
                                    const State& state) {
  FTRACE(1, " ++ new block {}\n", ainfo.ctx.func->blocks.size());
  assert(ainfo.bdata.size() == ainfo.ctx.func->blocks.size());

  auto newBlk           = std::make_unique<php::Block>();
  newBlk->id            = ainfo.ctx.func->blocks.size();
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

  CollectedInfo collect { index, ctx, nullptr, nullptr, true, &ainfo };
  auto interp = Interp { index, ctx, collect, blk, state };

  if (options.ConstantProp) collect.propagate_constants = propagate_constants;

  auto peephole = make_peephole(newBCs, index, ctx);
  std::vector<Op> srcStack(state.stack.size(), Op::Nop);

  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(ctx.func, op));

    auto const stateIn = state; // Peephole expects input eval state.
    auto gen = [&,srcStack] (const Bytecode& newBC) {
      const_cast<Bytecode&>(newBC).srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(ctx.func, newBC));
      if (options.Peephole) {
        peephole.append(newBC, stateIn, srcStack);
      } else {
        newBCs.push_back(newBC);
      }
    };

    auto const flags = step(interp, op);

    if (op.op == Op::CGetL2) {
      srcStack.insert(srcStack.end() - 1, op.op);
    } else {
      FTRACE(2, "   srcStack: pop {} push {}\n", op.numPop(), op.numPush());
      for (int i = 0; i < op.numPop(); i++) {
        srcStack.pop_back();
      }
      for (int i = 0; i < op.numPush(); i++) {
        srcStack.push_back(op.op);
      }
    }

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
      if (!(instrFlags(opc) & TF)) {
        gen(bc::BreakTraceHint {});
        gen(bc::String { s_unreachable.get() });
        gen(bc::Fatal { FatalOp::Runtime });
      }
      break;
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

    genOut(&op);
  }

  if (options.Peephole) {
    peephole.finalize();
  }
  blk->hhbcs = std::move(newBCs);
  auto& fpiStack = ainfo.bdata[blk->id].stateIn.fpiStack;
  auto it = std::remove_if(fpiStack.begin(), fpiStack.end(),
                           [](const ActRec& ar) {
                             return ar.kind == FPIKind::Builtin;
                           });

  if (it != fpiStack.end()) {
    fpiStack.erase(it, fpiStack.end());
    ainfo.builtinsRemoved = true;
  }
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

void do_optimize(const Index& index, FuncAnalysis&& ainfo) {
  FTRACE(2, "{:-^70} {}\n", "Optimize Func", ainfo.ctx.func->name);

  bool again;
  do {
    again = false;
    visit_blocks_mutable("first pass", index, ainfo, first_pass);
    if (ainfo.builtinsRemoved) {
      again = true;
      ainfo.builtinsRemoved = false;
    }

    FTRACE(10, "{}", show(*ainfo.ctx.func));
    /*
     * Note: it's useful to do dead block removal before DCE, so it can remove
     * code relating to the branch to the dead block.
     */
    remove_unreachable_blocks(ainfo);

    if (options.LocalDCE) {
      visit_blocks("local DCE", index, ainfo, local_dce);
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
      ainfo = analyze_func(index, ainfo.ctx, true);
    }

    // If we merged blocks, there could be new optimization opportunities
  } while (again);

  auto const func = ainfo.ctx.func;
  if (index.frozen() &&
      (func->name == s_86pinit.get() || func->name == s_86sinit.get())) {
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
    visit_blocks("persistence check", index, ainfo,
                 [&] (const Index&,
                      const FuncAnalysis&,
                      borrowed_ptr<php::Block> const blk,
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
    visit_blocks("insert assertions", index, ainfo, insert_assertions);
  }

  auto fixTypeConstraint = [&] (TypeConstraint& tc, const Type& candidate) {
    if (!tc.hasConstraint() ||
        tc.isSoft() ||
        tc.isTypeVar() ||
        tc.isTypeConstant() ||
        (tc.isThis() && !options.CheckThisTypeHints) ||
        tc.type() != AnnotType::Object) {
      return;
    }

    auto t = index.lookup_constraint(ainfo.ctx, tc, candidate);
    auto const nullable = is_opt(t);
    if (nullable) t = unopt(std::move(t));

    auto retype = [&] (AnnotType t) {
      tc.resolveType(t, nullable);
      FTRACE(1, "Retype tc {} -> {}\n",
             tc.typeName(), tc.displayName());
    };

    if (t.subtypeOf(TInitNull)) return retype(AnnotType::Null);
    if (t.subtypeOf(TBool))     return retype(AnnotType::Bool);
    if (t.subtypeOf(TInt))      return retype(AnnotType::Int);
    if (t.subtypeOf(TDbl))      return retype(AnnotType::Float);
    if (t.subtypeOf(TStr))      return retype(AnnotType::String);
    if (t.subtypeOf(TArr))      return retype(AnnotType::Array);
    // if (t.subtypeOf(TObj))   return retype(AnnotType::Object);
    if (t.subtypeOf(TRes))      return retype(AnnotType::Resource);
    if (t.subtypeOf(TDict))     return retype(AnnotType::Dict);
    if (t.subtypeOf(TVec))      return retype(AnnotType::Vec);
    if (t.subtypeOf(TKeyset))   return retype(AnnotType::Keyset);
  };

  if (options.HardTypeHints) {
    for (auto& p : func->params) {
      fixTypeConstraint(p.typeConstraint, TTop);
    }
  }

  if (options.HardReturnTypeHints) {
    auto const rtype = [&] {
      if (!func->isAsync) return ainfo.inferredReturn;
      if (!is_specialized_wait_handle(ainfo.inferredReturn)) return TGen;
      return wait_handle_inner(ainfo.inferredReturn);
    }();
    fixTypeConstraint(func->retTypeConstraint, rtype);
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
    case KindOfArray:
      assert(cell.m_data.parr->isStatic());
    case KindOfPersistentArray:
      assert(cell.m_data.parr->isPHPArray());
      return bc::Array { cell.m_data.parr };

    case KindOfRef:
    case KindOfResource:
    case KindOfObject:
      always_assert(0 && "invalid constant in propagate_constants");
  }
  not_reached();
}

void optimize_func(const Index& index, FuncAnalysis&& ainfo) {
  auto const bump = trace_bump_for(ainfo.ctx.cls, ainfo.ctx.func);

  Trace::Bump bumper1{Trace::hhbbc, bump};
  Trace::Bump bumper2{Trace::hhbbc_cfg, bump};
  Trace::Bump bumper3{Trace::hhbbc_dce, bump};
  do_optimize(index, std::move(ainfo));
}

//////////////////////////////////////////////////////////////////////

}}
