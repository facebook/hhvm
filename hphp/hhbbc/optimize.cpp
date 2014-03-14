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

#include "folly/Optional.h"

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
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/dce.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

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
                            std::bitset<kMaxTrackedLocals> mayReadLocalSet,
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

void insert_assertions(const Index& index,
                       const Context ctx,
                       borrowed_ptr<php::Block> const blk,
                       State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  auto lastStackOutputObvious = false;

  PropertiesInfo props { index, ctx, nullptr };
  auto interp = Interp { index, ctx, props, blk, state };
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

    insert_assertions_step(*ctx.func, op, preState, flags.mayReadLocalSet,
      lastStackOutputObvious, gen);

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

void first_pass(const Index& index,
                const Context ctx,
                borrowed_ptr<php::Block> const blk,
                State state) {
  std::vector<Bytecode> newBCs;
  newBCs.reserve(blk->hhbcs.size());

  PropertiesInfo props(index, ctx, nullptr);
  auto interp = Interp { index, ctx, props, blk, state };
  for (auto& op : blk->hhbcs) {
    FTRACE(2, "  == {}\n", show(op));

    auto gen = [&] (const Bytecode& newb) {
      newBCs.push_back(newb);
      newBCs.back().srcLoc = op.srcLoc;
      FTRACE(2, "   + {}\n", show(newBCs.back()));
    };

    auto const flags = step(interp, op);
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
      always_assert(!flags.wasPEI);
      switch (op.op) {
      case Op::JmpNZ:     blk->fallthrough = op.JmpNZ.target;     break;
      case Op::JmpZ:      blk->fallthrough = op.JmpZ.target;      break;
      case Op::IterInit:  blk->fallthrough = op.IterInit.target;  break;
      case Op::IterInitK: blk->fallthrough = op.IterInitK.target; break;
      default:
        // No support for switch, etc, right now.
        always_assert(0 && "unsupported tookBranch case");
      }
      /*
       * We need to pop the cell that was on the stack for the
       * conditional jump.  Note: for jumps this also conceptually
       * needs to execute any side effects a conversion to bool can
       * have.  (Currently that is none.)
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

  blk->hhbcs = std::move(newBCs);
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
    fun(index, ainfo.ctx, blk, state);
  }
  assert(check(*ainfo.ctx.func));
}

//////////////////////////////////////////////////////////////////////

void do_optimize(const Index& index, FuncAnalysis ainfo) {
  FTRACE(2, "{:-^70}\n", "Optimize Func");

  visit_blocks("first pass", index, ainfo, first_pass);

  /*
   * Note, it's useful to do dead block removal before DCE, so it can
   * remove code relating to the branch to the dead block.
   */
  remove_unreachable_blocks(index, ainfo);

  if (options.LocalDCE) {
    visit_blocks("local DCE", index, ainfo, local_dce);
  }
  if (options.GlobalDCE) {
    global_dce(index, ainfo);
    assert(check(*ainfo.ctx.func));
    /*
     * Global DCE can change types of locals across blocks.  See
     * dce.cpp for an explanation.
     *
     * We need to perform a final type analysis before we do anything
     * else.
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
