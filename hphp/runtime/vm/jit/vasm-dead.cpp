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
#include <boost/dynamic_bitset.hpp>
#include <algorithm>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {
using namespace x64;

namespace {
typedef boost::dynamic_bitset<> LiveSet;
bool effectful(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::copy:
    case Vinstr::copy2:
    case Vinstr::copyargs:
    case Vinstr::ldpoint:
    case Vinstr::load:
    case Vinstr::nop:
    //case Vinstr::andb:
    //case Vinstr::andbi:
    //case Vinstr::andl:
    //case Vinstr::andli:
    //case Vinstr::andq:
    //case Vinstr::andqi:
    //case Vinstr::addq:
    //case Vinstr::addqi:
    case Vinstr::addsd:
    case Vinstr::cloadq:
    case Vinstr::cmovq:
    case Vinstr::cvttsd2siq:
    case Vinstr::cvtsi2sd:
    //case Vinstr::decl:
    //case Vinstr::decq:
    case Vinstr::divsd:
    //case Vinstr::imul:
    //case Vinstr::incl:
    //case Vinstr::incq:
    case Vinstr::lea:
    case Vinstr::loaddqu:
    case Vinstr::loadl:
    case Vinstr::loadq:
    case Vinstr::loadsd:
    case Vinstr::loadzbl:
    case Vinstr::movb:
    case Vinstr::movbi:
    case Vinstr::movdqa:
    case Vinstr::movl:
    case Vinstr::movq:
    case Vinstr::movsbl:
    case Vinstr::movzbl:
    case Vinstr::mulsd:
    //case Vinstr::neg:
    case Vinstr::not: // suprisingly, x86 not doesn't modify flags
    //case Vinstr::orq:
    //case Vinstr::orqi:
    case Vinstr::psllq:
    case Vinstr::psrlq:
    //case Vinstr::rorqi:
    case Vinstr::roundsd:
    //case Vinstr::sarq:
    //case Vinstr::sarqi:
    //case Vinstr::sbbl:
    case Vinstr::setcc: // setcc reads flags but doesn't modify them.
    //case Vinstr::shlli:
    //case Vinstr::shlq:
    //case Vinstr::shlqi:
    //case Vinstr::shrli:
    //case Vinstr::shrqi:
    case Vinstr::sqrtsd:
    //case Vinstr::subli:
    //case Vinstr::subq:
    //case Vinstr::subqi:
    case Vinstr::subsd:
    case Vinstr::unpcklpd:
    //case Vinstr::xorb:
    //case Vinstr::xorbi:
    //case Vinstr::xorq:
    //case Vinstr::xorqi:
      return false;
    case Vinstr::ldimm:
      return !inst.ldimm_.saveflags;
    case Vinstr::incstat:
      return inst.incstat_.force || Stats::enabled();
    default:
      return true;
  }
}
}

// Remove dead instructions by doing a traditional liveness analysis.
// instructions that mutate memory, physical registers, or condition
// codes are considered useful. All branches are considered useful.
//
// If the input is SSA, there's a faster sparse version of this algorithm
// that marks useful instructions in one pass, then transitively marks
// pure instructions that define inputs to useful instructions.
//
// we could remove useless branches by computing the post-dom tree and
// RDF(b) for each block; then a branch is only useful if it controls
// whether or not a useful block executes, and useless branches can
// be forwarded to the nearest useful post-dominator.
void removeDeadCode(Vunit& unit) {
  auto blocks = sortBlocks(unit);
  jit::vector<LiveSet> livein(unit.blocks.size());
  LiveSet live(unit.next_vr);
  auto pass = [&](bool mutate) {
    bool changed = false;
    for (auto blockIt = blocks.end(); blockIt != blocks.begin();) {
      auto b = *--blockIt;
      auto& block = unit.blocks[b];
      live.reset();
      for (auto s : succs(block)) {
        if (!livein[s].empty()) {
          live |= livein[s];
        }
      }
      for (auto i = block.code.end(); i != block.code.begin();) {
        auto& inst = *--i;
        auto useful = effectful(inst);
        visitDefs(unit, inst, [&](Vreg r) {
          if (r.isPhys() || live.test(r)) {
            useful = true;
            live.reset(r);
          }
        });
        if (useful) {
          visitUses(unit, inst, [&](Vreg r) {
            live.set(r);
          });
        } else if (mutate) {
          inst = nop{};
          changed = true;
        }
      }
      if (mutate) {
        assert(live == livein[b]);
      } else {
        if (live != livein[b]) {
          livein[b] = live;
          changed = true;
        }
      }
    }
    return changed;
  };
  // analyze until livein reaches a fixed point
  while (pass(false)) {}
  // nop-out useless instructions
  if (pass(true)) {
    for (auto b : blocks) {
      auto& code = unit.blocks[b].code;
      auto end = std::remove_if(code.begin(), code.end(), [&](Vinstr& inst) {
        return inst.op == Vinstr::nop;
      });
      code.erase(end, code.end());
    }
    printUnit("after vasm-dead", unit);
  }
}

}}
