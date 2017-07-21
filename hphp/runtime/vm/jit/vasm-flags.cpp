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
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/arch.h"

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

}}
