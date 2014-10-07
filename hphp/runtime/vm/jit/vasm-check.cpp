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
#include "hphp/util/assertions.h"
#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

typedef boost::dynamic_bitset<> Bits;
bool checkSSA(Vunit& unit, jit::vector<Vlabel>& blocks) DEBUG_ONLY;
bool checkSSA(Vunit& unit, jit::vector<Vlabel>& blocks) {
  using namespace reg;
  jit::vector<Bits> block_defs(unit.blocks.size()); // index by [Vlabel]
  Bits global_defs(unit.next_vr);
  Bits consts(unit.next_vr);
  for (auto& c : unit.cpool) {
    global_defs.set(c.second);
    consts.set(c.second);
  }
  for (auto b : blocks) {
    Bits local_defs;
    if (block_defs[b].empty()) {
      local_defs.resize(unit.next_vr);
      for (auto& c : unit.cpool) {
        local_defs.set(c.second);
      }
    } else {
      local_defs = block_defs[b];
    }
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg v) {
        assert_flog(v.isValid(), "invalid vreg used in B{}\n{}",
                                 size_t(b), show(unit));
        assert_flog(!v.isVirt() || local_defs[v],
                    "%{} used before def in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
      });
      visitDefs(unit, inst, [&](Vreg v) {
        assert_flog(v.isValid(), "invalid vreg defined in B{}\n{}",
                    size_t(b), show(unit));
        assert_flog(!v.isVirt() || !consts.test(v),
                    "%{} const defined in B{}\n",
                    size_t(v), size_t(b), show(unit));
        assert_flog(!v.isVirt() || !local_defs[v],
                    "%{} locally redefined in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
        assert_flog(!v.isVirt() || !global_defs[v],
                    "%{} redefined in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
        local_defs.set(v);
        global_defs.set(v);
      });
    }
    for (auto s : succs(unit.blocks[b])) {
      if (block_defs[s].empty()) {
        block_defs[s] = local_defs;
      } else {
        block_defs[s] &= local_defs;
      }
    }
  }
  return true;
}

// make sure syncpoint{}, nocatch{}, or unwind{} only appear immediately
// after a call.
bool checkCalls(Vunit& unit, jit::vector<Vlabel>& blocks) DEBUG_ONLY;
bool checkCalls(Vunit& unit, jit::vector<Vlabel>& blocks) {
  for (auto b: blocks) {
    bool unwind_valid = false;
    bool nocatch_valid = false;
    bool sync_valid = false;
    bool hcunwind_valid = false;
    bool hcsync_valid = false;
    bool hcnocatch_valid = false;
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::call:
        case Vinstr::callm:
        case Vinstr::callr:
        case Vinstr::mccall:
          sync_valid = unwind_valid = nocatch_valid = true;
          break;
        case Vinstr::syncpoint:
          assert(sync_valid);
          sync_valid = false;
          break;
        case Vinstr::unwind:
          assert(unwind_valid);
          unwind_valid = nocatch_valid = false;
          break;
        case Vinstr::nocatch:
          assert(nocatch_valid);
          unwind_valid = nocatch_valid = false;
          break;
        case Vinstr::hostcall:
          hcsync_valid = hcunwind_valid = hcnocatch_valid = true;
          break;
        case Vinstr::hcsync:
          assert(hcsync_valid);
          hcsync_valid = false;
          break;
        case Vinstr::hcunwind:
          assert(hcunwind_valid);
          hcunwind_valid = hcnocatch_valid = false;
          break;
        case Vinstr::hcnocatch:
          assert(hcnocatch_valid);
          hcunwind_valid = hcnocatch_valid = false;
          break;
        default:
          unwind_valid = nocatch_valid = sync_valid = false;
          hcunwind_valid = hcnocatch_valid = hcsync_valid = false;
          break;
      }
    }
  }
  return true;
}

}

bool isBlockEnd(Vinstr& inst) {
  switch (inst.op) {
    // service request-y things
    case Vinstr::bindaddr:
    case Vinstr::bindjcc1:
    case Vinstr::bindjmp:
    case Vinstr::fallback:
    case Vinstr::retransopt:
    // control flow
    case Vinstr::jcc:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::phijmp:
    // terminal
    case Vinstr::resume:
    case Vinstr::ud2:
    case Vinstr::unwind:
    case Vinstr::ret:
    case Vinstr::end:
    // arm specific
    case Vinstr::hcunwind:
    case Vinstr::cbcc:
    case Vinstr::tbcc:
    case Vinstr::brk:
      return true;
    default:
      return false;
  }
}

// check that each block has exactly one terminal instruction at the end.
bool checkBlockEnd(Vunit& unit, Vlabel b) {
  assert(!unit.blocks[b].code.empty());
  auto& block = unit.blocks[b];
  auto n = block.code.size();
  for (size_t i = 0; i < n - 1; ++i) {
    assert(!isBlockEnd(block.code[i]));
  }
  assert(isBlockEnd(block.code[n - 1]));
  return true;
}

bool check(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  assert(checkSSA(unit, blocks));
  assert(checkCalls(unit, blocks));
  return true;
}

}}
