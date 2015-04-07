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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/assertions.h"

#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

namespace {
///////////////////////////////////////////////////////////////////////////////

bool checkSSA(Vunit& unit, jit::vector<Vlabel>& blocks) DEBUG_ONLY;
bool checkSSA(Vunit& unit, jit::vector<Vlabel>& blocks) {
  using namespace reg;
  using Bits = boost::dynamic_bitset<>;

  jit::vector<Bits> block_defs(unit.blocks.size()); // index by [Vlabel]
  Bits global_defs(unit.next_vr);
  Bits consts(unit.next_vr);

  for (auto& c : unit.constants) {
    global_defs.set(c.second);
    consts.set(c.second);
  }
  for (auto b : blocks) {
    Bits local_defs;
    if (block_defs[b].empty()) {
      local_defs.resize(unit.next_vr);
      for (auto& c : unit.constants) {
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
                    "%{} const defined in B{}\n{}",
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
    auto& block = unit.blocks[b];
    auto lastOp = block.code.back().op;
    if (lastOp == Vinstr::phijmp || lastOp == Vinstr::phijcc) {
      for (DEBUG_ONLY auto s : succs(block)) {
        assert_flog(!unit.blocks[s].code.empty()
          && unit.blocks[s].code.front().op == Vinstr::phidef,
          "B{} ends in {} but successor B{} doesn't begin with phidef\n",
          size_t(b), vinst_names[lastOp], size_t(s));
      }
    }
    for (auto s : succs(block)) {
      if (block_defs[s].empty()) {
        block_defs[s] = local_defs;
      } else {
        block_defs[s] &= local_defs;
      }
    }
  }
  return true;
}

/*
 * Make sure syncpoint{}, nothrow{}, or unwind{} only appear immediately after
 * a call.
 */
bool checkCalls(Vunit& unit, jit::vector<Vlabel>& blocks) DEBUG_ONLY;
bool checkCalls(Vunit& unit, jit::vector<Vlabel>& blocks) {
  for (auto b: blocks) {
    bool unwind_valid = false;
    bool nothrow_valid = false;
    bool sync_valid = false;
    bool hcunwind_valid = false;
    bool hcsync_valid = false;
    bool hcnocatch_valid = false;
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::call:
        case Vinstr::callm:
        case Vinstr::callr:
        case Vinstr::callstub:
        case Vinstr::mccall:
        case Vinstr::contenter:
          sync_valid = unwind_valid = nothrow_valid = true;
          break;
        case Vinstr::syncpoint:
          assertx(sync_valid);
          sync_valid = false;
          break;
        case Vinstr::unwind:
          assertx(unwind_valid);
          unwind_valid = nothrow_valid = false;
          break;
        case Vinstr::nothrow:
          assertx(nothrow_valid);
          unwind_valid = nothrow_valid = false;
          break;
        case Vinstr::hostcall:
          hcsync_valid = hcunwind_valid = hcnocatch_valid = true;
          break;
        case Vinstr::hcsync:
          assertx(hcsync_valid);
          hcsync_valid = false;
          break;
        case Vinstr::hcunwind:
          assertx(hcunwind_valid);
          hcunwind_valid = hcnocatch_valid = false;
          break;
        case Vinstr::hcnocatch:
          assertx(hcnocatch_valid);
          hcunwind_valid = hcnocatch_valid = false;
          break;
        default:
          unwind_valid = nothrow_valid = sync_valid = false;
          hcunwind_valid = hcnocatch_valid = hcsync_valid = false;
          break;
      }
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

bool check(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  assertx(checkSSA(unit, blocks));
  assertx(checkCalls(unit, blocks));
  return true;
}

bool checkBlockEnd(Vunit& unit, Vlabel b) {
  assertx(!unit.blocks[b].code.empty());
  auto& block = unit.blocks[b];
  auto n = block.code.size();
  for (size_t i = 0; i < n - 1; ++i) {
    assertx(!isBlockEnd(block.code[i]));
  }
  assertx(isBlockEnd(block.code[n - 1]));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
