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

#include "hphp/runtime/vm/jit/vasm-prof.h"

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/struct-log.h"

#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <string>
#include <vector>

namespace HPHP { namespace jit {

TRACE_SET_MOD(prof_branch);

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Thread-local RDS branch-sampling counter.
 */
rds::Link<uint32_t> s_counter{rds::kInvalidHandle};

/*
 * Reset `s_counter'.
 */
void reset_counter() {
  assertx(s_counter.bound());
  *s_counter = RuntimeOption::EvalProfBranchSampleFreq;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Compact identifier for an instruction.
 */
struct VasmID {
  VasmID(Offset bcoff, uint16_t iroff, uint16_t voff,
         Op bc, Opcode ir, AreaIndex area_idx, StringTag tag)
    : bcoff(bcoff)
    , iroff(iroff)
    , voff(voff)
    , bc_op(bc)
    , ir_op(ir)
    , tag(tag)
    , aidx(static_cast<uint8_t>(area_idx))
    , taken(false)
    , imm(false)
  {}

  AreaIndex area_idx() const { return static_cast<AreaIndex>(aidx); }

public:
  union {
    struct {
      Offset bcoff;
      uint16_t iroff;
      uint16_t voff;
    };
    uint64_t lo;
  };
  union {
    struct {
      Op bc_op;
      Opcode ir_op;
      StringTag tag;
      uint8_t aidx : 2;
      bool taken : 1;
      bool imm : 1;
    };
    uint64_t hi;
  };
};

static_assert(sizeof(VasmID) == 16, "VasmID layout error");

/*
 * Identifier for a branch.
 */
struct BranchID {
  BranchID(VasmID f, VasmID n, VasmID t) : from(f), next(n), taken(t) {}
  BranchID(VasmID f, VasmID n, TCA t) : from(f), next(n), taken(t) {
    from.imm = true;
  }

  BranchID& take(bool taken) {
    from.taken = taken;
    return *this;
  }

public:
  VasmID from;
  VasmID next;
  union U {
    U(VasmID v) : vasm_id(v) {}
    U(TCA t) : tca(t) {}

    VasmID vasm_id;
    TCA tca;
  } taken;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Version number for the entries.
 *
 * Bump this whenever the log format changes, so that it's easy to filter out
 * old, incompatible results.
 */
constexpr auto kVersion = 1;

}

/*
 * Record a hit to `branch'.
 *
 * This is defined outside the anonymous namespace so that they can be
 * symbolized in TRACE output.
 *
 * In most cases, `func' will be the same as `fp->func()'---i.e., the Func*
 * belonging to the branch that we took.  The exception is for inlined
 * functions where the inlined callee's frame has been partially elided, so we
 * just always pass `func' explicitly.
 */
void record_branch_hit(const BranchID* branch,
                       const Func* func, const ActRec* fp) {
  reset_counter();

  auto const& b = *branch;

  auto record = StructuredLogEntry{};
  record.setInt("version", kVersion);
  record.setStr("side", b.from.taken ? "taken" : "next");

  record.setStr("func", func->fullName()->data());

  auto const record_vasm_id = [&] (std::string pref, VasmID id) {
    record.setStr(pref + "_bc", opcodeToName(id.bc_op));
    record.setStr(pref + "_ir", opcodeName(id.ir_op));
    record.setInt(pref + "_bcoff", id.bcoff);
    record.setInt(pref + "_iroff", id.iroff);
    record.setInt(pref + "_voff", id.voff);
    record.setStr(pref + "_area", areaAsString(id.area_idx()));

    if (auto const str = string_from_tag(id.tag)) {
      record.setStr(pref + "_tag", str);
    }
  };

  record_vasm_id("from", b.from);
  record_vasm_id("next", b.next);

  if (b.from.imm) {
    record.setInt("taken_addr", reinterpret_cast<uintptr_t>(b.taken.tca));
  } else {
    record_vasm_id("taken", b.taken.vasm_id);
  }

  if (auto const cls = func->cls()) {
    record.setStr("cls", cls->name()->data());

    if (fp == nullptr || fp->func() != func || instrCanHalt(b.from.bc_op)) {
      // We can't access the late-bound class if:
      //  - rvmfp() was invalidated, e.g., by FreeActRec;
      //  - we're in an inlined function but are not pointing at the callee
      //    frame; or
      //  - we're in a Ret-like instruction which might have decref'd the
      //    frame's this/cls.
      //
      // Note that if we've inlined a function into itself (because of a
      // recursive call), we'll get a false-positive here, and our logging
      // could be wrong if the caller and callee happen to have different
      // late-bound classes.
      record.setStr("late_bound_cls", "(inaccessible)");
    } else {
      if (fp->hasClass()) {
        record.setStr("late_bound_cls", fp->getClass()->name()->data());
        record.setStr("call_type", "static");
      } else {
        assertx(fp->hasThis());
        record.setStr("late_bound_cls",
                      fp->getThis()->getVMClass()->name()->data());
        record.setStr("call_type", "instance");
      }
    }
  }

  FTRACE(1, "prof-branch: {}\n", show(record).c_str());
  StructuredLog::log("hhvm_tc_branches", record);
}

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Create a new block logically associated with the same IR as `s'.
 *
 * Used primarily to create a logical header (and predecessor) of `s'.
 */
Vout vheader(Vunit& unit, Vlabel s, AreaIndex area_cap = AreaIndex::Main) {
  auto const aidx = std::max(
    unit.blocks[s].area_idx, area_cap,
    [](AreaIndex l, AreaIndex r) {
      return static_cast<unsigned>(l) < static_cast<unsigned>(r);
    }
  );
  auto const header = unit.makeBlock(aidx, unit.blocks[s].weight);

  auto const& code = unit.blocks[s].code;
  assertx(!code.empty());

  return Vout{unit, header, code.front().irctx()};
}

/*
 * Decrement the branch sampling counter, and return the resultant SF register.
 */
Vreg check_counter(Vout& v) {
  s_counter.bind(rds::Mode::Local);

  auto const handle = s_counter.handle();
  auto const sf = v.makeReg();
  v << declm{rvmtl()[handle], sf};
  return sf;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Branch profiling context.
 */
struct Env {
  Vunit& unit;
  /*
   * Which flags register (if any) is live at the start of each block.
   */
  std::vector<Vreg> sf_livein;
  /*
   * VregSF uses that need to be renamed after inserting profiling code.
   *
   * This is needed whenever we profile an edge through which the flags
   * register is live.  In such cases, we push and pop the flags around the
   * profiling code, but to preserve SSA, we have to rewrite the subsequent
   * uses.
   *
   * Actually doing that can be hard (we might need to add phidef{}s, in case
   * we def'd a VregSF, branched, re-joined, and then used the VregSF after the
   * phi), so instead we just rename everything to RegSF{0}.
   */
  std::unordered_set<unsigned> sf_renames;
  /*
   * Set of Vlabels for which rvmfp() is not valid.
   */
  boost::dynamic_bitset<> fp_invalid;
};

/*
 * Get the StringTag for a Vinstr, if it has one.
 */
StringTag string_tag_for(const Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::jcc:
      return inst.jcc_.tag;
    default:
      break;
  }
  return StringTag{};
}

/*
 * Pack the VasmID for `inst' in block `b'.
 */
VasmID vasm_id_for(Env& env, const Vinstr& inst, Vlabel b) {
  auto const origin = inst.origin;
  assertx(origin != nullptr);

  return VasmID {
    origin->marker().bcOff(),
    origin->iroff(),
    inst.voff,
    origin->marker().sk().op(),
    origin->op(),
    env.unit.blocks[b].area_idx,
    string_tag_for(inst)
  };
};

/*
 * Pack the BranchID for the branch instruction that terminates `b'.
 */
template<typename Inst>
BranchID branch_id_for(Env& env, const Inst& from, Vlabel b) {
  auto& blocks = env.unit.blocks;
  auto const next  = from.targets[0];
  auto const taken = from.targets[1];

  return BranchID {
    vasm_id_for(env, blocks[b].code.back(), b),
    vasm_id_for(env, blocks[next].code.front(), next),
    vasm_id_for(env, blocks[taken].code.front(), taken)
  };
}

BranchID branch_id_for(Env& env, const jcci& from, Vlabel b) {
  auto& blocks = env.unit.blocks;
  auto const next  = from.target;
  auto const taken = from.taken;

  return BranchID {
    vasm_id_for(env, blocks[b].code.back(), b),
    vasm_id_for(env, blocks[next].code.front(), next),
    taken
  };
}

/*
 * Call the profiling routine to log `branch' belonging to `func'.
 *
 * `b' should be either the from-block or the to-block for `branch'.
 */
void sample_branch(Vout& v, Env& env, const BranchID& branch,
                   const Func* func, Vlabel b) {
  auto const push_val = [&] (Vout& v, uint64_t val) {
    // This wacky nonsense is to try to force XLS to use the same register for
    // each immediate we want to push.
    v << copy{v.cns(val), rret()};
    v << push{rret()};
  };

  auto const rbranch = v.makeReg();
  push_val(v, branch.taken.vasm_id.hi);
  push_val(v, branch.taken.vasm_id.lo);
  push_val(v, branch.next.hi);
  push_val(v, branch.next.lo);
  push_val(v, branch.from.hi);
  push_val(v, branch.from.lo);
  v << copy{rsp(), rbranch};

  auto const rfunc = v.cns(func);
  auto const rfp = env.fp_invalid[b] ? v.cns(0) : Vreg(rvmfp());

  v << vcall{
    CallSpec::direct(record_branch_hit),
    v.makeVcallArgs({{rbranch, rfunc, rfp}}),
    v.makeTuple({})
  };

  v << lea{rsp()[6 * sizeof(uint64_t)], rsp()};
}

/*
 * Create a branch profiling header for successor `s' of branch instruction
 * `from', and optionally link it into the CFG via `pre' and `post'.
 *
 * Calls `pre' before any code is emitted, and `post' after, both with the
 * header's Vout and the header's start Vlabel as arguments.
 */
template<class Pre, class Post>
void create_profiling_header(Env& env, BranchID branch, Vlabel to,
                             Pre pre, Post post) {
  auto const& inst = env.unit.blocks[to].code.front();
  if (!inst.origin) return; // we only want to profile IR control-flow

  auto v = vheader(env.unit, to);
  auto vc = vheader(env.unit, to, AreaIndex::Cold);
  auto const header = Vlabel(v);

  auto const live = env.sf_livein[to];

  pre(v, header);

  if (live.isValid()) {
    // Save the flags register if it's live.
    v << lea{rsp()[-8], rsp()};
    v << pushf{live};
  }

  // Check the profiling counter, and log a sample if it overflows.
  auto const sf = check_counter(v);
  unlikelyIfThen(v, vc, CC_LE, sf, [&] (Vout& v) {
    sample_branch(v, env, branch, inst.origin->func(), to);
  });

  if (live.isValid()) {
    // Restore flags, and register that we need to rename the uses.
    v << popf{RegSF{0}};
    v << lea{rsp()[8], rsp()};
    env.sf_renames.insert(live);
  }

  post(v, header);
}

/*
 * Convenience no-op function for create_profiling_header().
 */
void noop(const Vout&, Vlabel) {}

/*
 * Call create_profiling_header(), and link the profiling blocks into the CFB
 * by modifying the target `s' of the branch, and falling through to `s' after
 * profiling.
 */
void insert_profiling_header(Env& env, BranchID branch, Vlabel& to) {
  create_profiling_header(
    env, branch, to, noop,
    [&] (Vout& v, Vlabel header) {
      v << jmp{to};
      to = header;
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

template <typename Inst>
void profile(Env& /*env*/, Inst& /*inst*/, Vlabel /*b*/) {}

void profile(Env& env, jcc& inst, Vlabel b) {
  auto branch = branch_id_for(env, inst, b);
  auto const taken = inst.targets[1];

  for (auto& s : inst.targets) {
    insert_profiling_header(env, branch.take(s == taken), s);
  };
}

void profile(Env& env, jcci& inst, Vlabel b) {
  auto& unit = env.unit;

  auto branch = branch_id_for(env, inst, b);

  // Profile the `next' branch.
  insert_profiling_header(env, branch.take(false), inst.target);

  // Profile the `taken' branch...
  auto v = vheader(unit, b);
  auto vc = vheader(unit, b, AreaIndex::Cold);
  auto const header = Vlabel(v);

  auto const sf = check_counter(v);
  unlikelyIfThen(v, vc, CC_LE, sf, [&] (Vout& v) {
    auto const& vinstr = env.unit.blocks[b].code.back();
    sample_branch(v, env, branch.take(true), vinstr.origin->func(), b);
  });
  v << jmpi{inst.taken};

  // ...then replace the jcci{} with a jcc{}.
  auto& from = unit.blocks[b].code.back();
  from = jcc{inst.cc, inst.sf, {inst.target, header}};
}

void profile(Env& env, phijcc& inst, Vlabel b) {
  auto& unit = env.unit;

  auto const fresh_tuple = [&] {
    auto copy = unit.tuples[inst.uses];
    for (auto& r : copy) r = unit.makeReg();
    return unit.makeTuple(copy);
  };

  auto branch = branch_id_for(env, inst, b);
  auto const taken = inst.targets[1];

  for (auto& s : inst.targets) {
    DEBUG_ONLY auto const& to = unit.blocks[s].code.front();
    assertx(to.op == Vinstr::phidef);
    assertx(unit.tuples[inst.uses].size() ==
            unit.tuples[to.phidef_.defs].size());

    auto const middlemen = fresh_tuple();

    create_profiling_header(
      env, branch.take(s == taken), s,
      [&] (Vout& v, Vlabel) {
        v << phidef{middlemen};
      },
      [&] (Vout& v, Vlabel header) {
        v << phijmp{s, middlemen};
        s = header;
      }
    );
  };
}

///////////////////////////////////////////////////////////////////////////////

const Abi sf_abi {
  RegSet{}, RegSet{}, RegSet{}, RegSet{}, RegSet{},
  RegSet{RegSF{0}}
};

/*
 * Determine whether any VregSF is live at the beginning of each block.
 */
std::vector<Vreg> compute_sf_livein(const Vunit& unit,
                                    const jit::vector<Vlabel>& rpo,
                                    const PredVector& preds) {
  auto livein = std::vector<Vreg>(unit.blocks.size());

  auto workQ = dataflow_worklist<uint32_t>(unit.blocks.size());

  auto const po_to_block = [&] {
    auto blocks = rpo;
    std::reverse(blocks.begin(), blocks.end());
    return blocks;
  }();
  auto const block_to_po = [&] {
    auto order = std::vector<uint32_t>(unit.blocks.size());

    for (size_t po = 0; po < po_to_block.size(); ++po) {
      workQ.push(po);
      order[po_to_block[po]] = po;
    }
    return order;
  }();

  while (!workQ.empty()) {
    auto const b = po_to_block[workQ.pop()];
    auto const& block = unit.blocks[b];

    auto live = Vreg{};
    for (auto const s : succs(block)) {
      auto const other = livein[s];
      if (!other.isValid()) continue;

      assertx(!live.isValid() || live == other);
      live = other;
    }

    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      if (inst.op == Vinstr::phidef) {
        // Skip phidef{}---if the def-tuple includes a VregSF, then it's
        // actually live on the incoming edge.
        continue;
      }

      RegSet implicit_uses, implicit_across, implicit_defs;
      if (inst.op == Vinstr::vcall ||
          inst.op == Vinstr::vinvoke ||
          inst.op == Vinstr::vcallarray) {
        // getEffects() would assert since these haven't been lowered yet.
        implicit_defs |= RegSF{0};
      } else {
        getEffects(sf_abi, inst, implicit_uses, implicit_across, implicit_defs);
        assertx(implicit_across.empty());
      }

      auto const visit_def = [&] (Vreg, Width w) {
        if (w == Width::Flags) live = Vreg{};
      };
      auto const visit_use = [&] (Vreg r, Width w) {
        if (w == Width::Flags) live = r;
      };

      // Determine liveness at `inst'.  We rely on the assumption that VregSF
      // lifetimes can never overlap, which the checkSF() pass provides.
      visitDefs(unit, inst, visit_def);
      visit(unit, implicit_defs, visit_def);
      visitUses(unit, inst, visit_use);
      visit(unit, implicit_uses, visit_use);
    }

    if (live.isValid()) {
      if (livein[b].isValid()) {
        assertx(live == livein[b]);
      } else {
        livein[b] = live;
        for (auto p : preds[b]) workQ.push(block_to_po[p]);
      }
    }
  }

  return livein;
}

/*
 * Determine which blocks include, or are dominated by, an operation which
 * logically invalidates rvmfp().
 *
 * For the most part, we only change rvmfp() via calls and returns, which we
 * consider "safe" for our purposes here, since they are "atomic" sequences.
 * The one exception is in the async return path, when we emit a FreeActRec
 * instruction before doing some more work and eventually returning to the
 * scheduler.
 *
 * This analysis just searches for all blocks that contain or are dominated by
 * a FreeActRec instruction.
 */
boost::dynamic_bitset<> analyze_fp_validity(const Vunit& unit,
                                            const jit::vector<Vlabel>& rpo,
                                            const PredVector& preds) {
  auto fp_invalid = boost::dynamic_bitset<>(unit.blocks.size());

  auto workQ = dataflow_worklist<uint32_t>(unit.blocks.size());

  auto const block_to_rpo = [&] {
    auto order = std::vector<uint32_t>(unit.blocks.size());

    for (size_t i = 0; i < rpo.size(); ++i) {
      workQ.push(i);
      order[rpo[i]] = i;
    }
    return order;
  }();

  while (!workQ.empty()) {
    auto const b = rpo[workQ.pop()];
    auto const& block = unit.blocks[b];

    auto const invalid = !!fp_invalid[b];

    // `b' is fp-invalidated if any of its predecessors are...
    for (auto p : preds[b]) {
      fp_invalid[b] |= fp_invalid[p];
    }
    // ...or if it contains Vinstrs belonging to a FreeActRec.
    if (!fp_invalid[b]) {
      for (auto const& inst : block.code) {
        if (inst.origin && inst.origin->is(FreeActRec)) {
          fp_invalid[b] = true;
          break;
        }
      }
    }

    if (invalid != fp_invalid[b]) {
      for (auto const s : succs(block)) workQ.push(block_to_rpo[s]);
    }
  }

  return fp_invalid;
}

/*
 * VregSF-renaming visitor.
 */
struct FlagsVisitor {
  explicit FlagsVisitor(Env& env) : m_env(env) {}

  template<class F> void imm(const F&) {}
  template<class R> void across(R&) {}
  void across(VregSF&) = delete;

  template<class R> void def(R&) {}
  template<class R, class H> void defHint(R& r, H) { def(r); }
  template<class R> void use(R&) {}
  template<class R, class H> void useHint(R& r, H) { use(r); }

  void use(VregSF& r) {
    // Just rename everything to the physical flags register.  See the
    // documentation for `sf_renames' for the justification.
    if (m_env.sf_renames.count(r)) r = RegSF{0};
  }
  void def(VregSF& r) { use(r); }

 private:
  Env& m_env;
};

}

///////////////////////////////////////////////////////////////////////////////

void profile_branches(Vunit& unit) {
  if (!unit.profiling) return;

  auto const rpo = sortBlocks(unit);
  auto const preds = computePreds(unit);

  auto env = Env {
    unit,
    compute_sf_livein(unit, rpo, preds),
    decltype(Env::sf_renames){},
    analyze_fp_validity(unit, rpo, preds)
  };

  PostorderWalker{unit}.dfs([&] (Vlabel b) {
    assertx(!unit.blocks[b].code.empty());
    auto& inst = unit.blocks[b].code.back();

    switch (inst.op) {
#define O(name, ...)                                    \
      case Vinstr::name:                                \
        if (inst.origin) profile(env, inst.name##_, b); \
        break;
      VASM_OPCODES
#undef O
    }
  });

  auto visitor = FlagsVisitor(env);

  // Rename any VregSF's that had to be spilled.
  for (auto& blk : unit.blocks) {
    for (auto& inst : blk.code) {
      visitOperands(inst, visitor);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
