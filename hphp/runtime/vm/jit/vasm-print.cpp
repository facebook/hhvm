/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-print.h"

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"
#include "hphp/util/abi-cxx.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/stack-trace.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
using namespace x64;
using Trace::RingBufferType;
using Trace::ringbufferName;

const char* area_names[] = { "main", "cold", "frozen" };
namespace {

// Visitor class to format the operands of a Vinstr. There are
// imm() overloaded methods for each type of operand used by any Vinstr.
// If we are missing an overload, the templated catch-all prints "?".
struct FormatVisitor {
  FormatVisitor(const Vunit& unit, std::ostringstream& str)
    : unit(unit), str(str)
  {}
  template<class T> void imm(T imm) {
    str << sep() << "?";
  }
  void imm(ConditionCode cc) { str << sep() << cc_names[cc]; }
  void imm(int i) { str << sep() << i; }
  void imm(bool b) { str << sep() << (b ? 'T' : 'F'); }
  void imm(Immed s) { str << sep() << s.l(); }
  void imm(Immed64 s) {
    str << sep();
    if (s.fits(sz::byte)) str << s.l();
    else str << folly::format("0x{:08x}", s.q());
  }
  void imm(TCA addr) {
    str << sep() << getNativeFunctionName(addr);
  }
  void imm(Vpoint p) { str << sep() << (size_t)p; }
  void imm(RingBufferType t) { str << sep() << ringbufferName(t); }
  void imm(SrcKey k) { str << sep() << showShort(k); }
  void imm(Fixup fix) {
    str << sep() << "pc:" << fix.pcOffset << " sp:" << fix.spOffset;
  }
  void imm(Stats::StatCounter c) { str << sep() << Stats::g_counterNames[c]; }
  void imm(Vlabel b) { str << sep() << "B" << size_t(b); }
  void imm(TransID id) { str << sep() << id; }
  void imm(const Func* func) {
    str << sep();
    if (func) {
      str << folly::format("{}(id {:#x})", func->fullName()->data(),
                           func->getFuncId());
    } else {
      str << "nullptr";
    }
  }
  void imm(TransFlags f) {
    if (f.noinlineSingleton) str << sep() << "noinlineSingleton";
  }

  template<class R> void across(R r) { print(r); }
  template<class R> void use(R r) { print(r); }
  void use(Vptr m) { print(m); }
  void use(Vtuple uses) { print(uses); }

  void defSep() {
    if (!seen_def) {
      if (comma) str << " => ";
      else str << "=> ";
      seen_def = true;
      comma = false;
    }
  }
  void def(Vtuple defs) { defSep(); print(defs); }
  template<class R> void def(R r) { defSep(); print(r); }

  void print(Vptr p) {
    str << sep() << show(p);
  }

  void print(Reg64 r) {
    str << sep() << reg::regname(r);
  }

  void print(Vtuple t) {
    for (auto r : unit.tuples[t]) print(r);
  }

  void print(RegSet regs) {
    regs.forEach([&](Vreg r) { print(r); });
  }

  template<class R> void print(R r) {
    str << sep();
    if (!r.isValid()) str << "%?";
    else if (r.isVirt()) str << "%" << size_t(r);
    else str << name(r);
  }
  const char* name(Vreg64 r) { return reg::regname(r.asReg()); }
  const char* name(Vreg32 r) { return reg::regname(r.asReg()); }
  const char* name(Vreg8 r) { return reg::regname(r.asReg()); }
  const char* name(VregXMM r) {
    return reg::regname(r.asReg());
  }
  const char* name(Vreg r) {
    if (r.isGP()) {
      Reg64 tmp = r;
      return reg::regname(tmp);
    }
    RegXMM tmp = r;
    return reg::regname(tmp);
  }
  const char* sep() { return comma ? ", " : (comma = true, ""); }
  const Vunit& unit;

  std::ostringstream& str;
  bool seen_def{false};
  bool comma{false};
};
}

std::string show(Vreg r) {
  if (!r.isValid()) return "%?";
  if (r.isPhys()) {
    if (r.isGP()) {
      Reg64 gpr = r;
      return reg::regname(gpr);
    }
    RegXMM xmm = r;
    return reg::regname(xmm);
  }
  std::ostringstream str;
  str << "%" << size_t(r);
  return str.str();
}

std::string show(Vptr p) {
  // [%fs + %base + disp + %index * scale]
  std::string str = "[";
  auto prefix = false;
  if (p.seg == Vptr::FS) {
    str += "%fs";
    prefix = true;
  }
  if (p.base.isValid()) {
    folly::toAppend(prefix ? " + " : "", show(p.base), &str);
    prefix = true;
  }
  if (p.disp) {
    folly::format(&str, "{}{:#x}",
                  prefix ? p.disp < 0 ? " - " : " + " : "",
                  prefix ? std::abs(p.disp) : p.disp);
    prefix = true;
  }
  if (p.index.isValid()) {
    folly::toAppend(prefix ? " + " : "", show(p.index), &str);
    if (p.scale != 1) folly::toAppend(" * ", p.scale, &str);
  }
  str += ']';
  return str;
}

std::string show(const Vunit& unit, const Vinstr& inst) {
  std::ostringstream out;
  out << folly::format("{: <10} ", vinst_names[inst.op]);
  FormatVisitor pv(unit, out);
  visitOperands(inst, pv);
  auto labels = succs(inst);
  if (labels.size() == 1) {
    out << pv.sep() << folly::format("B{}", size_t(labels[0]));
  } else if (labels.size() == 2) {
    out << pv.sep() << folly::format("B{}, else B{}", size_t(labels[1]),
                                     size_t(labels[0]));
  } else {
    for (auto succ : succs(inst)) {
      out << folly::format("->B{} ", size_t(succ));
    }
  }
  return out.str();
}

void printBlock(std::ostream& out, const Vunit& unit,
                const PredVector& preds, Vlabel b) {
  auto& block = unit.blocks[b];
  out << folly::format("B{: <11} {}", size_t(b), area_names[int(block.area)]);
  for (auto p : preds[b]) out << ", B" << size_t(p);
  out << "\n";
  for (auto& inst : block.code) {
    out << "  " << show(unit, inst) << "\n";
  }
}

void printCfg(std::ostream& out, const Vunit& unit,
              const jit::vector<Vlabel>& blocks) {
  out << "digraph G {\n";
  for (auto b: blocks) {
    auto& block = unit.blocks[b];
    auto succlist = succs(block);
    if (succlist.empty()) continue;
    auto sep = "";
    for (auto s: succlist) {
      out << sep << 'B' << size_t(b) << " -> B" << size_t(s);
      sep = "; ";
    }
    out << "\n";
  }
  out << "}\n";
}

void printCfg(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  std::ostringstream out;
  out << "vunit cfg\n";
  printCfg(out, unit, blocks);
  HPHP::Trace::traceRelease("%s\n", out.str().c_str());
}

std::string show(const Vunit& unit) {
  auto preds = computePreds(unit);
  auto blocks = sortBlocks(unit);

  std::ostringstream out;
  printCfg(out, unit, blocks);
  for (auto b : blocks) {
    printBlock(out, unit, preds, b);
  }
  return out.str();
}

void printUnit(std::string caption, const Vunit& unit) {
  if (!Trace::moduleEnabledRelease(HPHP::Trace::vasm, 1)) return;
  Trace::ftraceRelease("\n{}\n{}\n", caption, show(unit));
}

}}
