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
#include "hphp/util/assertions.h"
#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit { namespace x64 {

namespace {

typedef boost::dynamic_bitset<> Bits;
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
        assert_flog(v.isValid(), "invalid vreg used in B{}", size_t(b));
        assert_flog(!v.isVirt() || local_defs[v],
                    "%{} used before def in B{}", size_t(v), size_t(b));
      });
      visitDefs(unit, inst, [&](Vreg v) {
        assert_flog(v.isValid(), "invalid vreg defined in B{}", size_t(b));
        // TODO: t4779057: require SSA
        assert_flog(!v.isVirt() || !consts.test(v),
                    "%{} const defined in B{}", size_t(v), size_t(b));
        //assert_flog(!v.isVirt() || !local_defs[v],
        //            "%{} locally redefined in B{}", size_t(v), size_t(b));
        //assert_flog(!v.isVirt() || !global_defs[v],
        //            "%{} redefined in B{}", size_t(v), size_t(b));
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
}

bool check(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  assert(checkSSA(unit, blocks));
  return true;
}

}}}
