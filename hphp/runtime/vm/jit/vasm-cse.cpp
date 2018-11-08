/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Intel Corporation                                 |
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


#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <type_traits>
#include <algorithm>

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(vasm_cse);

// We need this for all types that Vreg can be implicitly converted to; for
// all other types, we do nothing
template<class T>
void replace(T &v, Vreg of, Vreg with, typename std::enable_if<
    std::is_convertible<Vreg,T>::value, T>::type* =0) {
  if (v == of) {
    FTRACE(kVasmCSELevel, "vasm-cse: replacing uses of {} with {}\n",
           show(of), show(with));
    v = with;
  }
}

template<class T>
void replace(T &v, Vreg of, Vreg with, typename std::enable_if<
    !std::is_convertible<Vreg,T>::value, T>::type* =0) {}

void replaceUsesOfWith(Vinstr&inst, Vreg of, Vreg with) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      uses \
      break; \
    }
#define U(s)    replace(i.s, of, with);
#define UA(s)   replace(i.s, of, with);
#define UH(s,h) replace(i.s, of, with);
#define Un
    VASM_OPCODES
#undef Un
#undef UH
#undef UA
#undef U
#undef O
  }
}

struct OperandCounter {
  unsigned numOperands = 0;
  unsigned numDefines  = 0;
  void imm(const Immed&) { numOperands++; }
  void imm(const Immed64&) { numOperands++; }
  template<class T> void imm(T) {}
  template<class T> void def(T) { numDefines++; }
  template<class T, class H> void defHint(T, H) { numDefines++; }
  template<class T> void across(T) { numOperands++; }
  template<class T> void use(T) { numOperands++; }
  template<class T, class H> void useHint(T, H) { numOperands++; }
};

struct DefMatcher {
  jit::vector<Vreg> regOp;
  jit::vector<Vptr> ptrOp;
  jit::vector<Vtuple> tupleOp;
  bool definesSF{false};
  template<class T> void use(T) {}
  template<class T, class H> void useHint(T, H) {}
  template<class T> void across(T) {}
  template<class T> void imm(T) {}
  template<class T, class H> void defHint(T r, H) { def(r); }
  void def(Vreg r) { regOp.push_back(r); }
  void def(VregSF r) { definesSF = true; }
  void def(Vptr r) { ptrOp.push_back(r); }
  void def(Vtuple t) { tupleOp.push_back(t); }
  bool operator==(DefMatcher dm) const {
    return regOp == dm.regOp && ptrOp == dm.ptrOp && tupleOp == dm.tupleOp
           && definesSF == dm.definesSF;
  }
  bool defines(Vreg r) {
    return std::find(regOp.begin(), regOp.end(), r) != regOp.end();
  }
  bool defines(Vptr r) {
    return std::find(ptrOp.begin(), ptrOp.end(), r) != ptrOp.end();
  }
  bool defines(Vtuple t) {
    return std::find(tupleOp.begin(), tupleOp.end(), t) != tupleOp.end();
  }
  bool defines(VregSF r) {
    return definesSF;
  }
};

struct UseMatcher {
  jit::vector<Vreg> regOp;
  jit::vector<Vptr> ptrOp;
  jit::vector<Vtuple> tupleOp;
  jit::vector<VcallArgsId> cargsOp;
  jit::vector<RegSet> regSetOp;
  jit::vector<Immed> immOp;
  jit::vector<Immed64> imm64Op;
  unsigned numImms{0};
  template<class T> void def(T) {}
  template<class T, class H> void defHint(T, H) {}
  template<class T> void across(T r) { use(r); }
  template<class T, class H> void useHint(T r, H) { use(r); }
  void use(Vreg r) { regOp.push_back(r); }
  void use(Vptr r) { ptrOp.push_back(r); }
  void use(Vtuple t) { tupleOp.push_back(t); }
  void use(VcallArgsId a) {cargsOp.push_back(a); }
  void use(RegSet regs) { regSetOp.push_back(regs); }
  void imm(Immed imm) { immOp.push_back(imm); }
  void imm(Immed64 imm) { imm64Op.push_back(imm); }
  template<class T> void imm(const T&) { numImms++; }
  bool operator==(UseMatcher um) const {
    return numImms == um.numImms && regOp == um.regOp && ptrOp == um.ptrOp &&
        tupleOp == um.tupleOp && cargsOp == um.cargsOp &&
        regSetOp == um.regSetOp && immOp == um.immOp && imm64Op == um.imm64Op;
  }
  bool uses(Vreg r) {
    return std::find(regOp.begin(), regOp.end(), r) != regOp.end();
  }
  bool uses(Vptr r) {
    return std::find(ptrOp.begin(), ptrOp.end(), r) != ptrOp.end();
  }
  bool uses(Vtuple t) {
    return std::find(tupleOp.begin(), tupleOp.end(), t) != tupleOp.end();
  }
  bool uses(VcallArgsId a) {
    return std::find(cargsOp.begin(), cargsOp.end(), a) != cargsOp.end();
  }
  bool uses(RegSet r) {
    return std::find(regSetOp.begin(), regSetOp.end(), r) != regSetOp.end();
  }
  bool uses(Immed i) {
    return std::find(immOp.begin(), immOp.end(), i) != immOp.end();
  }
  bool uses(Immed64 i) {
    return std::find(imm64Op.begin(), imm64Op.end(), i) != imm64Op.end();
  }
};

struct Expr {
  Vinstr instr;
  UseMatcher um;
  DefMatcher dm;
};

// Remove any saved expressions that have r as an operand
// This will never happen to virtual registers (because of SSA), but can
// happen to physical registers
template<class T>
void removeUses(Vunit &unit, T r, Vinstr& instr,
                jit::vector<Expr>& exprs) {
  exprs.erase(std::remove_if(exprs.begin(), exprs.end(), [&](Expr e) {
      bool remove{e.um.uses(r)};
      if (remove) {
        FTRACE(kVasmCSELevel,
               "vasm-cse: Removing saved expression:{} ({} has {} as operand)\n"
               , show(unit, e.instr), show(unit, instr), (size_t)r);
      }
      return remove;
  }), exprs.end());
}

struct Remover {
  Remover(Vunit& unit, Vinstr& instr, jit::vector<Expr>& exprs)
      : exprs(exprs), unit(unit), instr(instr) {}
  template<class T> void imm(T) {}
  template<class T> void use(T) {}
  template<class T> void across(T) {}
  template<class T, class H> void useHint(T, H) {}
  template<class T, class H> void defHint(T r, H) { def(r); }
  template<class T> void def(T r) {
    removeUses(unit, r, instr, exprs);
  }
  // Remove any saved expressions which mutate a VregSF. We wouldn't need
  // to do this if vasm-xls would support more than one live SF register at
  // a time
  void def(VregSF r) {
    exprs.erase(std::remove_if(exprs.begin(), exprs.end(), [&](Expr e) {
        bool remove{e.dm.defines(r)};
        if (remove) {
          FTRACE(kVasmCSELevel,
                 "vasm-cse: removing saved expression: {} ({} mutates SF {})\n",
                 show(unit, e.instr), show(unit, instr), (size_t)r);
        }
        return remove;
    }), exprs.end());
  }
  jit::vector<Expr>& exprs;
  Vunit& unit;
  Vinstr& instr;
};

jit::vector<Expr>::iterator match(Vunit& unit, Vinstr& inst,
                                  jit::vector<Expr>& exprs) {
  UseMatcher um;
  visitOperands(inst, um);

  return std::find_if(exprs.begin(), exprs.end(),
    [&](const Expr& e) {
        return e.instr.op == inst.op && e.um == um;
    });
}

bool isBinary(Vunit& unit, Vinstr& inst) {
  OperandCounter oc;
  visitOperands(inst, oc);
  return oc.numOperands == 2 && oc.numDefines >= 1;
}

void addExpression(jit::vector<Expr> &exprs, Vinstr &inst) {
  UseMatcher um;
  visitOperands(inst, um);
  DefMatcher dm;
  visitOperands(inst, dm);
  exprs.emplace_back(Expr{inst, um, dm});
}

unsigned int runOnBlock(Vunit& unit, Vblock& block) {
  jit::vector<Expr> exprs;
  unsigned removed{0};
  unsigned pos{0};

  while (pos < block.code.size()) {
    auto &inst = block.code[pos];
    jit::vector<Vreg> src, dst;

    if (!isBinary(unit, inst)) {
      Remover rm{unit, inst, exprs};
      visitOperands(inst, rm);
      pos++;
      continue;
    }

    auto m = match(unit, inst, exprs);
    if (m != exprs.end()) {
      FTRACE(kVasmCSELevel, "vasm-cse: eliminating binary expression:{}\n",
             show(unit, inst));
      visitDefs(unit, m->instr, [&](Vreg r) { src.emplace_back(r); });
      visitDefs(unit, inst,     [&](Vreg r) { dst.emplace_back(r); });
      unsigned pos2{pos+1};
      while (pos2 < block.code.size()) {
        for (unsigned i = 0; i < src.size(); ++i) {
          FTRACE(kVasmCSELevel, "vasm-cse: trying to replace in {}\n",
                 show(unit, block.code[pos2]));
          replaceUsesOfWith(block.code[pos2], dst[i], src[i]);
        }
        pos2++;
      }
      block.code[pos] = nop{};
      removed++;
    } else {
      FTRACE(kVasmCSELevel, "vasm-cse: saving binary expression:{}\n",
             show(unit, inst));
      Remover rm{unit, inst, exprs};
      visitOperands(inst, rm);
      addExpression(exprs, inst);
    }
    pos++;
  }
  return removed;
}

}

void localCSE(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  unsigned int removed{0};

  for (auto const& label : blocks) {
    removed += runOnBlock(unit, unit.blocks[label]);
  }
  if (removed) {
    FTRACE(kVasmCSELevel, "vasm-cse: transformed {} instruction(s)\n", removed);
  }
  printUnit(kVasmCSELevel, "after vasm-cse", unit);
}

}}
