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

#include "hphp/runtime/vm/jit/vasm-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {
using namespace x64;

namespace {
struct Folder {
  Vunit& unit;
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  // helpers
  bool match_byte(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::byte)) return false;
    val = imm64;
    return true;
  }
  bool match_int(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::dword)) return false;
    val = imm64;
    return true;
  }
  // folders
  template<class Inst> void fold(Inst&, Vinstr& out) {}
  void fold(addq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = addqi{val, in.s1, in.d}; }
    else if (match_int(in.s1, val)) { out = addqi{val, in.s0, in.d}; }
  }
  void fold(andq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = andqi{val, in.s1, in.d}; }
    else if (match_int(in.s1, val)) { out = andqi{val, in.s0, in.d}; }
  }
  void fold(cmpb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) { out = cmpbi{val, in.s1}; }
  }
  void fold(cmpq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqi{val, in.s1}; }
  }
  void fold(cmpqm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqim{val, in.s1}; }
  }
  void fold(cmplm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmplim{val, in.s1}; }
  }
  void fold(orq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = orqi{val, in.s1, in.d}; }
    else if (match_int(in.s1, val)) { out = orqi{val, in.s0, in.d}; }
  }
  void fold(storeb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s, val)) { out = storebim{val, in.m}; }
  }
  void fold(storel& in, Vinstr& out) {
    int val;
    if (match_int(in.s, val)) { out = storelim{val, in.m}; }
  }
  void fold(storeq& in, Vinstr& out) {
    int val;
    if (match_int(in.s, val)) { out = storeqim{val, in.m}; }
  }
  void fold(store& in, Vinstr& out) {
    int val;
    if (match_int(in.s, val)) { out = storeqim{val, in.d}; }
  }
  void fold(subq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = subqi{val, in.s1, in.d}; }
  }
  void fold(xorb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) { out = xorbi{val, in.s1, in.d}; }
    else if (match_byte(in.s1, val)) { out = xorbi{val, in.s0, in.d}; }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = xorqi{val, in.s1, in.d}; }
    else if (match_int(in.s1, val)) { out = xorqi{val, in.s0, in.d}; }
  }
};
}

// Immediate-folding. If an instruction takes a register operand defined
// as a constant, and there is valid immediate-form of that instruction,
// then change the instruction and embed the immediate.
void foldImms(Vunit& unit) {
  assert(check(unit)); // especially, SSA
  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);
  Folder folder{unit};
  folder.vals.resize(unit.next_vr);
  folder.valid.resize(unit.next_vr);
  // figure out which Vregs are constants and stash their values.
  for (auto& entry : unit.cpool) {
    folder.valid.set(entry.second);
    folder.vals[entry.second] = entry.first;
  }
  // now mutate instructions
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
#define O(name, imms, uses, defs)\
        case Vinstr::name: folder.fold(inst.name##_, inst); break;
        X64_OPCODES
#undef O
      }
    }
  }
  printUnit(kVasmImmsLevel, "after foldImms", unit);
}

}}
