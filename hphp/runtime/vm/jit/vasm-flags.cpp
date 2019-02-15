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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/arch.h"
#include "hphp/util/dataflow-worklist.h"

#include <boost/range/adaptor/reversed.hpp>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

///////////////////////////////////////////////////////////////////////////////

struct Env {
  struct DefFlags {
    Vinstr* def;        // The single def of a VregSF
    Vflags flags{0x00}; // The flags required by all of the uses of VregSF
  };

  Vunit& unit;

  // Map of VregSF to its required flags.
  jit::hash_map<size_t, DefFlags> sf_def_flags;
};

///////////////////////////////////////////////////////////////////////////////

struct FlagVisitor {
  template<class I> void imm(I&) {}
  template<class R> void def(R) {}
  template<class D, class H> void defHint(D, H) {}
  template <class T>
  void across(T /*r*/) {}
  template<class R> void use(R) {}
  template<class S, class H> void useHint(S, H) {}
  void imm(Vflags& fl) {
    assertx(!m_flags);
    m_flags = &fl;
  }

  Vflags getFlags() {
    assertx(m_flags);
    return *m_flags;
  }
  void setFlags(Vflags fl) {
    assertx(m_flags);
    *m_flags = fl;
  }

  Vflags* m_flags{nullptr};
};

struct CCVisitor {
  template<class I> void imm(I&) {}
  template<class R> void def(R) {}
  template<class D, class H> void defHint(D, H) {}
  template <class T>
  void across(T /*r*/) {}
  template<class R> void use(R) {}
  template<class S, class H> void useHint(S, H) {}
  void imm(ConditionCode& cc) {
    assertx(!m_cc);
    m_cc = &cc;
  }

  ConditionCode getCC() {
    assertx(m_cc);
    return *m_cc;
  }

  ConditionCode* m_cc{nullptr};
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Check the particular SF uses and change the CC if the platform doesn't have
 * a simple CC mapping for the given def and use.
 *
 * This is to be done in the specializations below as general mappings are
 * usually correct.
 */
template <typename Inst>
void fixSFUses(const Env& /*env*/, Inst& /*inst*/) {}

void fixSFUses(const Env& env, jcc& inst) {
  if (!arch_any(Arch::ARM)) return;

  if (env.sf_def_flags.at(inst.sf).def->op == Vinstr::ucomisd) {
    switch (inst.cc) {
    case CC_AE: // CC_NB
      inst.cc = CC_GE;
      break;
    default:
      break;
    }
  }
}

/*
 * For each use of an SF, return the StatusFlags bits needed by the use's CC.
 *
 * Optionally, check the uses, updating the CC as required by the platform.
 */
Vflags annotateSFUses(const Env& env, Vinstr* inst) {
  switch (inst->op) {
#define O(name, imms, ...) \
    case Vinstr::name: { \
      fixSFUses(env, inst->name##_); \
      break; \
    }
    VASM_OPCODES
#undef O
  }
  CCVisitor ccv;
  visitOperands(*inst, ccv);
  return required_flags(ccv.getCC());
}

///////////////////////////////////////////////////////////////////////////////

}

void annotateSFUses(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  Env env { unit };

  auto const labels = sortBlocks(unit);

  // Set up Env, only visiting reachable blocks.
  // 1) Track each VregSF to the instruction which def'd it.
  // 2) Track the required flags for the def based on the uses.
  for (auto const b : labels) {
    assertx(!blocks[b].code.empty());
    for (auto& inst : blocks[b].code) {
      visitDefs(unit, inst, [&] (Vreg r, Width w) {
        if (w == Width::Flags) {
          env.sf_def_flags[r].def = &inst;
        }
      });
      visitUses(unit, inst, [&] (Vreg r, Width w) {
        if (w == Width::Flags) {
          env.sf_def_flags[r].flags |= annotateSFUses(env, &inst);
        }
      });
    }
  };

  // Update each def of VregSF with the required flags.
  for (auto& kv : env.sf_def_flags) {
    auto& df = kv.second;

    // Set the flags for the instruction that defs the sf.
    FlagVisitor fv;
    visitOperands(*df.def, fv);
    fv.setFlags(df.flags);

    FTRACE(kVasmAnnotateSFLevel, "VregSF: {} -- flags: {}\n",
           kv.first, df.flags);
  }

  printUnit(kVasmAnnotateSFLevel, "after map SF", unit);
}

///////////////////////////////////////////////////////////////////////////////

// Perform a few peephole optimizations that are only safe if the flags register
// is dead.
void sfPeepholes(Vunit& unit, const Abi& abi) {
  // Currently, all our optimizations are only relevant on x64.
  assertx(arch() == Arch::X64);

  Timer timer(Timer::vasm_sf_peepholes, unit.log_entry);

  auto const rpo = sortBlocks(unit);
  jit::vector<size_t> rpoOrder(unit.blocks.size());
  for (size_t i = 0; i < rpo.size(); ++i) rpoOrder[rpo[i]] = i;

  auto const preds = computePreds(unit);

  // Calculate per-block flag liveness using dataflow
  boost::dynamic_bitset<> livenessIn;
  livenessIn.resize(unit.blocks.size());

  dataflow_worklist<size_t, std::less<size_t>> worklist(rpo.size());
  for (size_t i = 0; i < rpo.size(); ++i) worklist.push(i);

  while (!worklist.empty()) {
    auto const b = rpo[worklist.pop()];
    auto const& block = unit.blocks[b];

    auto const liveIn = [&]{
      for (auto const& inst : block.code) {
        auto uses = false;
        visitUses(
          unit, inst,
          [&] (Vreg r) { if (r.isSF()) uses = true; }
        );
        if (uses) return true;

        auto kills = false;
        visitDefs(
          unit, inst,
          [&] (Vreg r) { if (r.isSF()) kills = true; }
        );
        if (kills) return false;
      }

      auto liveOut = false;
      for (auto const s : succs(block)) liveOut |= livenessIn[s];
      return liveOut;
    }();

    if (liveIn != livenessIn[b]) {
      assertx(liveIn);
      assertx(!livenessIn[b]);
      for (auto const pred : preds[b]) {
        worklist.push(rpoOrder[pred]);
      }
      livenessIn[b] = liveIn;
    }
  }

  // Now perform the peepholes
  for (auto const b : rpo) {
    auto& block = unit.blocks[b];

    auto live = false;
    for (auto const s : succs(block)) live |= livenessIn[s];

    // We need to update the flag register's liveness as we go, walk backwards.
    for (auto& inst : boost::adaptors::reverse(block.code)) {
      visitDefs(
        unit, inst,
        [&] (Vreg r) { if (r.isSF()) live = false; }
      );

      visitUses(
        unit, inst,
        [&] (Vreg r) { if (r.isSF()) live = true; }
      );

      // The peepholes are only sound if the flag register is dead. If its not
      // then keep going until it is.
      if (live) continue;

      auto const sf = abi.sf.choose();

      // An lea manipulating the stack pointer can be changed to a simple add if
      // we don't mind clobbering the flags. An immediate load of zero can be
      // simplified into an equivalent xor if we don't mind clobbering the
      // flags.
      switch (inst.op) {
        case Vinstr::lea: {
          auto const& lea = inst.lea_;
          if (lea.d == rsp()) {
            assertx(lea.s.base == lea.d);
            assertx(!lea.s.index.isValid());
            inst.addqi_ = addqi{lea.s.disp, lea.d, lea.d, sf};
            inst.op = Vinstr::addqi;
          }
          break;
        }
        case Vinstr::ldimmb: {
          auto const& ldimm = inst.ldimmb_;
          if (ldimm.s.q() == 0 && ldimm.d.isGP()) {
            inst.xorb_ = xorb{ldimm.d, ldimm.d, ldimm.d, sf};
            inst.op = Vinstr::xorb;
          }
          break;
        }
        case Vinstr::ldimml: {
          auto const& ldimm = inst.ldimml_;
          if (ldimm.s.q() == 0 && ldimm.d.isGP()) {
            inst.xorl_ = xorl{ldimm.d, ldimm.d, ldimm.d, sf};
            inst.op = Vinstr::xorl;
          }
          break;
        }
        case Vinstr::ldimmq: {
          auto const& ldimm = inst.ldimmq_;
          if (ldimm.s.q() == 0 && ldimm.d.isGP()) {
            inst.xorq_ = xorq{ldimm.d, ldimm.d, ldimm.d, sf};
            inst.op = Vinstr::xorq;
          }
          break;
        }
        default:
          break;
      }
    }

    // Our tracking of the flag register liveness should match what the dataflow
    // calculated.
    assertx(live == livenessIn[b]);
  }

  assertx(check(unit));

  printUnit(kVasmAnnotateSFLevel, "after sf-peepholes", unit);
}

///////////////////////////////////////////////////////////////////////////////

}}
