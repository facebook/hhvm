/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include <iterator>
#include <string>
#include <algorithm>
#include <utility>

#include <folly/Format.h>

#include <boost/dynamic_bitset.hpp>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/loop-analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/mutation.h"

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(hhir_licm);

//////////////////////////////////////////////////////////////////////

bool is_pure_licmable(const IRInstruction* inst) {
  // Note: this list is still not exhaustive; this is just a prototype.
  switch (inst->op()) {
  case AbsDbl:
  case AddInt:
  case SubInt:
  case MulInt:
  case AndInt:
  case AddDbl:
  case SubDbl:
  case MulDbl:
  case DivDbl:
  case Mod:
  case Sqrt:
  case OrInt:
  case XorInt:
  case Shl:
  case Shr:
  case Floor:
  case Ceil:
  case XorBool:
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
  case InterfaceSupportsArr:
  case InterfaceSupportsStr:
  case InterfaceSupportsInt:
  case InterfaceSupportsDbl:
  case LdClsCtor:
  case LdClsName:
  case Mov:
  case LookupClsRDSHandle:
  case LdContActRec:
  case LdAFWHActRec:
  case LdCtx:
  case LdCctx:
  case LdClsCtx:
  case LdClsCctx:
  case LdStkAddr:
  case LdLocAddr:
  case LdPropAddr:
  case LdRDSAddr:
  case IsScalarType:
  case ExtendsClass:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
    return true;

  default:
    return false;
  }
}

//////////////////////////////////////////////////////////////////////

enum class IdomState { Uninit, Valid, Invalid };

struct Env {
  explicit Env(IRUnit& unit, const BlockList& rpoBlocks)
    : unit(unit)
    , ainfo(collect_aliases(unit, rpoBlocks))
    , loops(identify_loops(unit, rpoBlocks))
  {}

  ~Env() {
    switch (idom_state) {
    case IdomState::Uninit:
      break;
    case IdomState::Invalid:
    case IdomState::Valid:
      idoms.~IdomVector();
      break;
    }
  }

  IRUnit& unit;
  AliasAnalysis ainfo;
  LoopAnalysis loops;
  IdomState idom_state{IdomState::Uninit};
  union { IdomVector idoms; };
};

//////////////////////////////////////////////////////////////////////

IdomVector& find_idoms(Env& env) {
  switch (env.idom_state) {
  case IdomState::Valid:
    return env.idoms;
  case IdomState::Invalid:
  case IdomState::Uninit:
    {
      auto const rpo = rpoSortCfg(env.unit);
      auto new_idoms = IdomVector(
        findDominators(env.unit, rpo, numberBlocks(env.unit, rpo))
      );
      if (env.idom_state == IdomState::Uninit) {
        new (&env.idoms) IdomVector(std::move(new_idoms));
      } else {
        env.idoms = std::move(new_idoms);
      }
    }
    env.idom_state = IdomState::Valid;
    break;
  }
  return env.idoms;
}

//////////////////////////////////////////////////////////////////////

/*
 * Information needed during processing of a single loop.  We process an
 * "extended" loop at a time: all the natural loops that share a particular
 * header block.
 */
struct LoopEnv {
  explicit LoopEnv(Env& global, LoopID canonical_loop)
    : global(global)
    , canonical_loop(canonical_loop)
    , invariant_tmps(global.unit.numTmps())
    , expanded_blocks(expanded_loop_blocks(global.loops, canonical_loop))
  {
    FTRACE(2, "expanded_blocks: {}\n",
      [&] () -> std::string {
        auto ret = std::string{};
        for (auto& b : expanded_blocks) folly::format(&ret, "B{} ", b->id());
        return ret;
      }()
    );
  }

  Env& global;
  LoopID canonical_loop;

  /*
   * SSATmps with loop-invariant definitions, either because they are purely
   * loop invariant or because the other definitions they depend on are also
   * loop invariant.  This is populated in a topological order of dependencies
   * between the definitions.
   *
   * These can be moved to the loop header with no change in program semantics,
   * although in the current version of this pass it may cause increases in
   * computation on some paths that exit the loop, because we don't try to make
   * sure the definitions dominate all the Likely-hinted loop exits.
   */
  sparse_idptr_set<SSATmp> invariant_tmps;

  /*
   * Loads of loop invariant memory locations.
   *
   * These definitions are not necessarily loop invariant, because their
   * destination SSATmps may have types that depend on the position in the
   * program.  This means they can't simply be hoisted to a loop preheader, and
   * it also means that computations depending on the destination can't be
   * hoisted.  We can however insert generic loads in the loop pre-header and
   * replace these instructions with AssertTypes.
   */
  jit::vector<IRInstruction*> invariant_loads;

  /*
   * Hoistable check instructions.
   *
   * These are instructions that check the type of either loop invariant memory
   * locations, or loop invariant definitions, and occur in the loop body
   * before any code with side effects (including any branches out of the
   * loop).  They can be moved to the loop pre header without changing the
   * program's semantics.
   */
  jit::vector<IRInstruction*> invariant_checks;

  /*
   * Hoistable checks if we're willing to side-exit.
   *
   * These instructions check the type of either loop invariant memory
   * locations, or loop invariant definitions, and if the check fails they
   * definitely leave the loop, and we think they probably also leave the
   * entire region.
   *
   * We can rewrite the code to check the condition before entering the loop,
   * and side exit if it fails.  This transformation is only appropriate if we
   * believe the check failing is very unlikely, because it can cause new
   * regions to be generated that duplicate portions of the loop body.
   */
  jit::vector<IRInstruction*> hoistable_as_side_exits;

  /*
   * All member blocks of any natural loop with the same header as our
   * canonical loop.
   */
  jit::flat_set<Block*> expanded_blocks;

  /*
   * Whether any block in the loop contains a php call.  If this is true,
   * hoisting any loop invariant definition will create uses of SSATmps that
   * span a call on some path to the back edge.  Right now we don't try to
   * optimize in this situation.
   */
  bool contains_call{false};

  /*
   * Memory locations the loop provably cannot modify on any route to a back
   * edge.  (Note that the program still may modify these memory locations in
   * blocks that are reachable from the loop header, after exiting the loop.)
   */
  ALocBits invariant_memory;

  /*
   * Currently unused except for debug printing:
   *
   * Memory locations that we saw PureLoad instructions for in the loop.  And
   * memory locations that we saw check instructions for in the loop.
   */
  ALocBits pure_load_memory;
  ALocBits dbg_checked;
};

//////////////////////////////////////////////////////////////////////

NaturalLoopInfo& linfo(LoopEnv& env) {
  return env.global.loops.naturals[env.canonical_loop];
}

Block* header(LoopEnv& env)     { return linfo(env).header; }
Block* pre_header(LoopEnv& env) { return linfo(env).pre_header; }

template<class Seen, class F>
void visit_loop_post_order(LoopEnv& env, Seen& seen, Block* b, F f) {
  if (seen[b->id()]) return;
  seen.set(b->id());
  auto go = [&] (Block* b) {
    if (!b || !env.expanded_blocks.count(b)) return;
    visit_loop_post_order(env, seen, b, f);
  };
  go(b->next());
  go(b->taken());
  f(b);
}

BlockList rpo_sort_loop(LoopEnv& env) {
  auto seen = boost::dynamic_bitset<>(env.global.unit.numBlocks());
  auto ret = BlockList{};
  visit_loop_post_order(
    env, seen, header(env),
    [&] (Block* b) { ret.push_back(b); }
  );
  std::reverse(begin(ret), end(ret));
  return ret;
}

//////////////////////////////////////////////////////////////////////

void analyze_block(LoopEnv& env, Block* blk) {
  auto kill = [&] (AliasClass acls) {
    auto const canon = canonicalize(acls);
    auto const may_write = env.global.ainfo.may_alias(canon);
    FTRACE(6, "    kill: {}\n", show(may_write));
    env.invariant_memory &= ~may_write;
  };

  FTRACE(6, "analyze_block B{}\n", blk->id());
  for (auto& inst : blk->instrs()) {
    auto const effects = canonicalize(memory_effects(inst));
    FTRACE(6, "  {}\n    mem: {}\n", inst.toString(), show(effects));
    match<void>(
      effects,
      [&] (UnknownEffects)   { kill(AUnknown); },

      [&] (CallEffects x)    { env.contains_call = true;
                               if (x.destroys_locals) kill(AFrameAny);
                               kill(AHeapAny); },
      [&] (PureStore x)      { kill(x.dst); },
      [&] (PureSpillFrame x) { kill(x.stk); },

      [&] (GeneralEffects x) {
        kill(x.stores);

        switch (inst.op()) {
        case CheckLoc:
        case CheckStk:
          if (auto const meta = env.global.ainfo.find(x.loads)) {
            env.dbg_checked.set(meta->index);
          }
        default:
          break;
        }
      },

      [&] (PureLoad x) {
        if (auto const meta = env.global.ainfo.find(x.src)) {
          env.pure_load_memory.set(meta->index);
        }
      },

      [&] (IrrelevantEffects) {},

      [&] (ReturnEffects)     { assertx(inst.is(InlineReturn)); },
      [&] (ExitEffects)       { assertx(!"tried to exit in a loop"); }
    );
  }
}

void analyze_loop(LoopEnv& env) {
  env.invariant_memory.set();
  analyze_block(env, header(env));
  for (auto& blk : env.expanded_blocks) analyze_block(env, blk);
  FTRACE(2, "invariant: {}\n"
            "pure-load: {}\n"
            "  checked: {}\n"
            "    calls: {}\n",
         show(env.invariant_memory),
         show(env.pure_load_memory),
         show(env.dbg_checked),
         env.contains_call);
  FTRACE(6, "--\n");
}

bool is_invariant_memory(LoopEnv& env, AliasClass acls) {
  if (auto const meta = env.global.ainfo.find(canonicalize(acls))) {
    return env.invariant_memory[meta->index] == true;
  }
  return false;
}

bool has_all_loop_invariant_args(LoopEnv& env, const IRInstruction& inst) {
  for (auto& src : inst.srcs()) {
    if (env.invariant_tmps.contains(src)) continue;
    if (is_tmp_usable(find_idoms(env.global), src, pre_header(env))) continue;
    return false;
  }
  return true;
}

bool try_pure_licmable(LoopEnv& env, const IRInstruction& inst) {
  if (env.contains_call) return false;
  if (!is_pure_licmable(&inst)) return false;
  if (!has_all_loop_invariant_args(env, inst)) return false;
  FTRACE(2, "      invariant: {} = {}\n", inst.dst()->toString(),
    inst.toString());
  env.invariant_tmps.insert(inst.dst());
  return true;
}

bool check_is_loop_exit(LoopEnv& env, const IRInstruction& inst) {
  auto const t = inst.taken();
  return t != header(env) && !env.expanded_blocks.count(t);
}

bool impl_hoistable_check(LoopEnv& env, IRInstruction& inst) {
  if (!check_is_loop_exit(env, inst)) return false;
  if (!has_all_loop_invariant_args(env, inst)) return false;
  FTRACE(2, "      hoistable check: {}\n", inst.toString());
  env.invariant_checks.push_back(&inst);
  return true;
}

/*
 * NB. This function doesn't actually prove that the taken branch is a side
 * exit.  We can't really know this for a fact: even if it started as a
 * "side-exit", real computation could've been moved into the branch.
 */
bool check_is_probable_side_exit(LoopEnv& env, const IRInstruction& inst) {
  return check_is_loop_exit(env, inst) &&
    inst.taken()->isExit() &&
    inst.taken()->hint() == Block::Hint::Unlikely;
}

bool try_hoistable_as_side_exit(LoopEnv& env, IRInstruction& inst) {
  if (!check_is_probable_side_exit(env, inst)) return false;
  if (!has_all_loop_invariant_args(env, inst)) return false;
  FTRACE(2, "      hoistable as side exit: {}\n", inst.toString());
  env.hoistable_as_side_exits.push_back(&inst);
  return true;
}

bool try_hoistable_check(LoopEnv& env, IRInstruction& inst) {
  switch (inst.op()) {
  case CheckCtxThis:
    return impl_hoistable_check(env, inst);
  default:
    return false;
  }
}

bool try_invariant_memory(LoopEnv& env,
                          IRInstruction& inst,
                          bool may_still_hoist_checks,
                          const MemEffects& effects) {
  return match<bool>(
    effects,
    [&] (UnknownEffects)    { return false; },
    [&] (CallEffects)       { return false; },
    [&] (PureStore)         { return false; },
    [&] (PureSpillFrame)    { return false; },
    [&] (IrrelevantEffects) { return false; },
    [&] (ReturnEffects)     { return false; },
    [&] (ExitEffects)       { return false; },

    [&] (GeneralEffects mem) {
      switch (inst.op()) {
      case CheckLoc:
        if (!is_invariant_memory(env, mem.loads)) return false;
        if (may_still_hoist_checks) {
          if (impl_hoistable_check(env, inst)) return true;
        }
        return try_hoistable_as_side_exit(env, inst);

      default:
        break;
      }
      return false;
    },

    [&] (PureLoad mem) {
      if (env.contains_call) return false;
      if (!is_invariant_memory(env, mem.src)) return false;
      if (!has_all_loop_invariant_args(env, inst)) return false;
      FTRACE(2, "      invariant load: {} = {}\n",
             inst.dst()->toString(),
             inst.toString());
      env.invariant_loads.push_back(&inst);
      return true;
    }
  );
}

bool may_have_side_effects(const IRInstruction& inst) {
  switch (inst.op()) {
  case Jmp:
  case ExitPlaceholder:
    return false;

  /*
   * Note: we could potentially extend this to allow various load instructions
   * or pure computations, but it would only be legal to hoist checks after
   * them if we first verify that the SSATmps they defined are not used under
   * the taken block.
   */

  default:
    return true;
  }
}

/*
 * Unlike a usual LICM algorithm, we don't need to iterate to identify the
 * invariant code.  If we walk the loop blocks in reverse post order, so we see
 * successors after predecessors, we see enough information for each of the
 * types of invariant code we're trying to identify:
 *
 *   o For "pure licm"able instructions, if they are only licmable after some
 *     of their sources are licm'd, we're guaranteed to see those dependencies
 *     in order because of SSA.  All of these have a single definition, and the
 *     definitions dominate uses.
 *
 *   o For loads of invariant locations, we're using information from the full
 *     flow-insensitive pass over the loop in analyze_loop that identified
 *     which locations are invariant.
 *
 *   o For hoisting Check instructions: we only can do this for Checks that
 *     occur before any side-effects.  We also only /want/ to do it if the
 *     Check is guaranteed to be executed in the loop body (assuming another
 *     hoistable Check doesn't fail).  So, for correctness in the presence of
 *     nested loops (natural or not), we need to start to refuse to hoist
 *     checks if we see a block where we haven't visited all of its
 *     predecessors yet.  The restriction on when we want to do it is handled
 *     by considering conditional jumps (and any instruction with multiple
 *     successor blocks other than ExitPlaceholder) to be instructions with
 *     side-effects.
 *
 */
void find_invariant_code(LoopEnv& env) {
  auto may_still_hoist_checks = true;
  auto visited_block = boost::dynamic_bitset<>(env.global.unit.numBlocks());

  for (auto& b : rpo_sort_loop(env)) {
    FTRACE(2, "  B{}:\n", b->id());
    if (may_still_hoist_checks && b != header(env)) {
      b->forEachPred([&] (Block* pred) {
        if (!visited_block[pred->id()]) {
          FTRACE(5, "    may_still_hoist_checks = false (pred: B{})\n",
            pred->id());
          may_still_hoist_checks = false;
        }
      });
    }

    for (auto& inst : b->instrs()) {
      FTRACE(4, "    {}\n", inst.toString());
      if (try_pure_licmable(env, inst)) continue;

      auto const effects = memory_effects(inst);
      if (try_invariant_memory(env, inst, may_still_hoist_checks, effects)) {
        continue;
      }

      if (may_still_hoist_checks) {
        if (try_hoistable_check(env, inst)) continue;
        if (may_have_side_effects(inst)) {
          FTRACE(5, "      may_still_hoist_checks = false\n");
          may_still_hoist_checks = false;
        }
      }
    }

    visited_block.set(b->id());
  }
}

void hoist_invariant(LoopEnv& env) {
  /*
   * Iterating invariant_tmps goes in insertion order, which means it is
   * already guaranteed to be topologically sorted by any dependencies between
   * the instructions.
   */
  for (auto& invID : env.invariant_tmps) {
    auto const dst = env.global.unit.findSSATmp(invID);
    auto const inst = dst->inst();
    auto const preh = pre_header(env);
    FTRACE(1, "moving {} to B{}\n", inst->toString(), preh->id());
    inst->block()->erase(inst);
    assert(!inst->taken() && !inst->next());
    preh->insert(std::prev(preh->end()), inst);
  }
}

/*
 * Loads that have invariant memory locations still don't define SSATmps that
 * are loop invariant (without further analysis).  The reason is that the type
 * may depend on some other instruction that is still in the loop.
 *
 * So we can insert a load in the pre-header that defines a Gen, and replace
 * the load at the position in the loop with an AssertType.
 */
void hoist_invariant_loads(LoopEnv& env) {
  for (auto& old_load : env.invariant_loads) {
    if (!old_load->is(LdLoc, LdStk, LdMem)) continue;

    auto const preh     = pre_header(env);
    auto const new_load = env.global.unit.clone(old_load);
    new_load->setTypeParam(TGen);
    retypeDests(new_load, &env.global.unit);
    preh->insert(std::prev(preh->end()), new_load);
    env.global.unit.replace(
      old_load,
      AssertType,
      old_load->typeParam(),
      new_load->dst()
    );
    FTRACE(1, "inserted {} in B{}\n         {} -> {}\n",
           new_load->toString(),
           preh->id(),
           old_load->dst()->toString(),
           old_load->toString());
  }
}

void hoist_check_instruction(LoopEnv& env,
                             IRInstruction* old_check,
                             Block* new_taken,
                             const char* why) {
  auto const preh      = pre_header(env);
  auto const new_check = env.global.unit.clone(old_check);

  new_check->setTaken(new_taken);

  // Note that the current pre_header jump may have arguments.  We need to
  // preserve them in the new pre_header, so we have to keep the same
  // instruction.
  assert(preh->back().is(Jmp));
  auto const jmp       = &preh->back();
  auto const new_preh  = env.global.unit.defBlock();
  preh->erase(jmp);
  new_preh->prepend(jmp);
  new_check->setNext(new_preh);
  preh->insert(preh->end(), new_check);

  FTRACE(1, "[{}] inserted {} in B{}, adding B{} as the new pre_header\n",
         why,
         new_check->toString(),
         preh->id(),
         new_preh->id());

  env.global.unit.replace(
    old_check,
    Jmp,
    old_check->next()
  );

  // We've changed the pre-header and invalidated the dominator tree.
  env.global.idom_state = IdomState::Invalid;
  update_pre_header(env.global.loops, env.canonical_loop, new_preh);
}

void hoist_invariant_checks(LoopEnv& env) {
  for (auto& check : env.invariant_checks) {
    hoist_check_instruction(env, check, check->taken(), "invariant");
  }
}

/*
 * If we have an ExitPlaceholder instruction that dominates the entire loop, we
 * can use its target as a side exit target.  Otherwise we can't hoist anything
 * as a side exit, since we don't know how to side exit here safely.  The first
 * instruction in the loop header definitely dominates every block in the loop,
 * so look for it there.
 */
Block* find_exit_placeholder(LoopEnv& env) {
  auto const h = header(env);
  auto const skip = h->skipHeader();
  return skip->is(ExitPlaceholder) ? skip->taken() : nullptr;
}

void hoist_side_exits(LoopEnv& env) {
  auto const side_exit = find_exit_placeholder(env);
  if (!side_exit) return;
  for (auto& check : env.hoistable_as_side_exits) {
    hoist_check_instruction(env, check, side_exit, "side-exit");
  }
}

void process_loop(LoopEnv& env) {
  analyze_loop(env);
  find_invariant_code(env);
  hoist_invariant(env);
  hoist_invariant_loads(env);
  hoist_invariant_checks(env);
  hoist_side_exits(env);
}

//////////////////////////////////////////////////////////////////////

void insert_pre_headers(IRUnit& unit, LoopAnalysis& loops) {
  auto added_pre_headers = false;
  for (auto& linfo : loops.naturals) {
    if (!linfo.pre_header) {
      insert_loop_pre_header(unit, loops, linfo.id);
      added_pre_headers = true;
    }
  }
  if (added_pre_headers) {
    FTRACE(1, "Loops after adding pre-headers:\n{}\n", show(loops));
  }
}

//////////////////////////////////////////////////////////////////////

}

void optimizeLoopInvariantCode(IRUnit& unit) {
  PassTracer tracer { &unit, Trace::hhir_licm, "LICM" };
  Env env { unit, rpoSortCfg(unit) };
  if (env.loops.naturals.empty()) {
    FTRACE(1, "no loops\n");
    return;
  }
  FTRACE(2, "Locations:\n{}\n", show(env.ainfo));
  FTRACE(1, "Loops:\n{}\n", show(env.loops));
  insert_pre_headers(env.unit, env.loops);

  /*
   * Note: currently this is visiting inner loops first, but not for any strong
   * reason for the types of optimizations it currently performs.
   */

  auto workQ = jit::queue<LoopID>{};
  auto seen = boost::dynamic_bitset<>(env.loops.naturals.size());

  auto enqueue = [&] (LoopID id) {
    if (id == kInvalidLoopID) return;
    id = env.loops.headers[env.loops.naturals[id].header];
    if (seen[id]) return;
    seen[id] = true;
    workQ.push(id);
  };

  for (auto& id : env.loops.inner_loops) enqueue(id);

  do {
    auto const id = workQ.front();
    workQ.pop();
    enqueue(env.loops.naturals[id].canonical_outer);

    FTRACE(1, "L{}:\n", id);
    auto lenv = LoopEnv { env, id };
    process_loop(lenv);

  } while (!workQ.empty());
}

//////////////////////////////////////////////////////////////////////

}}

/*
 * LICM is a work in progress, disabled for now.  Here's some things still to
 * look at:
 *
 *   o More is_pure_licmable.  Some things that dereference certain kinds of
 *     pointers (e.g. LdObjClass) can probably be in the list too.
 *
 *   o More kinds of checks for hoisting (CheckType, CheckTypeMem).
 *
 *   o Maybe support for only hoisting things that dominate all the loop exits.
 */
